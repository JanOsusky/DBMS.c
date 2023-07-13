/*
 * File:   test_store_server
 * Author: Guillermo Pérez Trabado
 *
 * This file implements a primitive server implementing store server of the
 * DBMS.
 *
 * This test uses the cache library to read and write records to disk.
 * It also uses the server side store API to read request and send back answers.
 */

#include <stdio.h>
#include <stdlib.h>

#include <mycache.h>
#include <mystore_srv.h>
#include <signal.h>
#include <sys/types.h>
#include <stdbool.h>

#include "debug.h"

/* Debug level for messages */
static int debug_level = DEBUG_INIT;

// the booleans
volatile static int prog_end_requested = 0;
volatile static int sigusr1_requested = 0;
volatile static int sigusr2_reuqested = 0;
volatile static int alarm_requested = 0;

void s_handler(int sig_num)
{
  switch (sig_num)
  {
  case SIGINT:
    signal(SIGINT, s_handler); // calling signal again
    prog_end_requested = 1;    // setting boolean to 1
    break;

  case SIGTERM:
    signal(SIGTERM, s_handler); // calling signal again
    prog_end_requested = 1;     // setting boolean to 1
    break;

  case SIGUSR1:
    signal(SIGUSR1, s_handler); // calling signal again
    sigusr1_requested = 1;      // setting boolean to 1
    break;

  case SIGUSR2:
    signal(SIGUSR2, s_handler); // calling signal again
    sigusr2_reuqested = 1;      // setting boolean to 1
    break;

  case SIGALRM:
    alarm_requested = 1;        // setting boolean to 1
    alarm(15);                  // calling after 15 second
    signal(SIGALRM, s_handler); // calling signal again
    break;

  default:
    break;
  }
}

// stats
static int numberR;
static int numberW;
static int numberReq;

/* This is the main loop of the server */
int main(int argc, char **argv)
{
  // called to not kill the program after detaching
  // ignoring the signal
  signal(SIGHUP, SIG_IGN);

  bool detaching = true;
  // parsing cmd arguments -v or -f
  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      // Process option
      if (argv[i][1] == 'v')
      {
        // Process -v option
        MYC_debuglevel_rotate();
        debuglevel_rotate();
        STORS_debuglevel_rotate();
      }
      else if (argv[i][1] == 'f')
      {
        // Process -f option
        detaching = false;
      }
      else
      {
        fprintf(stderr, "NOT VALID ARGS");
        exit(1);
      }
    }
    else
    {
      fprintf(stderr, "NOT VALID ARGS");
      exit(1); // Invalid option found
    }
  }

  // setting the signals to the handler
  signal(SIGTERM, s_handler);
  signal(SIGINT, s_handler);
  signal(SIGALRM, s_handler);
  signal(SIGUSR1, s_handler);
  signal(SIGUSR2, s_handler);

  // closing stdin
  fclose(stdin);
  // closing stdouts
  fclose(stdout);

  // reopening file
  FILE *logFile = freopen("store_server.log", "w", stderr);

  if (logFile == NULL) // error handling
  {
    fprintf(stderr, "Failed to open the log file.\n");
    return 1;
  }

  /* This function initializes the cache. */
  if (MYC_initCache() != 0)
  {
    debug_error("Error initializing cache.");
    exit(1);
  }

  if (STORS_init() != 0)
  {
    debug_error("Error initializing server side API.");
    /* Close cache as we end here. */
    MYC_closeCache();
    exit(1);
  }

  debug_info("Test store server started OK.");

  // flushing
  fflush(stderr);

  if (detaching)
  {
    int childPid = fork();
    if (childPid)
    {
      // parent of a running daemon
      debug_info("Deamon inicialized");
      return 0;
    }
    // detaching from terminal
    setsid();
  }

  // ignoring this signal
  signal(SIGHUP, SIG_IGN);

  // setting timer for 15
  alarm(15);

  while (!prog_end_requested)
  {

    if (alarm_requested)
    {
      alarm_requested = 0;
      debug_info("Flushing");
      MYC_flushAll();
      fflush(stderr);
    }

    if (sigusr1_requested)
    {
      sigusr1_requested = 0;
      debug_info("Read  Requests: %d", numberR);
      debug_info("Write Requests: %d\n", numberW);
      debug_info("Total Requests: %d", numberReq);
      fflush(stderr);
    }

    if (sigusr2_reuqested)
    {
      sigusr2_reuqested = 0;
      debuglevel_rotate();
      MYC_debuglevel_rotate();
      STORS_debuglevel_rotate();
      debug_info("Set debug level to %d", debug_level);
      fflush(stderr);
    }

    /* We need a request and an answer. */
    request_message_t req;
    answer_message_t answer;

    /* Wait for a request from a client. */
    int status = STORS_readrequest(&req);
    /* Check status and possible errors. */
    /* A signal interrupted reception. Start loop again to check termination. */
    if (status == -2)
      continue;
    if (status == -1)
    {
      debug_error("Problems receiving a request.");
      /* Exit from main loop. */
      break;
    }

    /* Prepare an answer to our client. */
    /* Fill the answer type with the identity of the client sending the request. */
    answer.mtype = req.return_to;
    numberReq++;

    /* Decode operation. */
    switch (req.requested_op)
    {
    case MYSCOP_READ:
      /* Implement read operation with cache library. */
      /* The index is provided in the request. */
      /* The record content must be stored in the answer. */
      status = MYC_readEntry(req.index, &answer.data);
      answer.status = status; /* Fill status with the result of the operation. */
      debug_debug("Read operation (client=%ld, idx=%d) ret %d.", req.return_to, req.index, status);
      debug_verbose("id: %u, age: %d, gender: %d, name: %s", answer.data.registerid, answer.data.age, answer.data.gender, answer.data.name);
      numberR++; // stats
      break;

    case MYSCOP_WRITE:
      /* Implement write operation with cache library. */
      /* The record to write and the index are provided in the request. */
      status = MYC_writeEntry(req.index, &req.data);
      answer.status = status; /* Fill status with the result of the operation. */
      debug_debug("Write operation (client=%ld, idx=%d) ret %d.", req.return_to, req.index, status);
      numberW++; // stats
      break;

    default:
      /* Remark unknown operations to stderr!!!
       Maybe we are using a more advanced client who uses more
       operations than an older server. */
      debug_error("Unknown operation received from client.");
      answer.status = -1; /* You should have an special error for "Unknown operation" */
      break;
    }

    /* Send back the answer */
    status = STORS_sendanswer(&answer);
    /* Check status and possible errors. */
    if (status != 0)
    {
      debug_error("Problems sending back an answer.");
      /* Exit from main loop. */
      break;
    }
  }

  /* This server never ends (by now). But one day, it will be able to end. */

  /* Close the server side API. */
  if (STORS_close() != 0)
  {
    debug_error("Error closing server API.");
    /* Do not exit without closing the cache!!! */
  }

  /* This function closes the cache. It flushes all the information inside the
   * cache that is not written to the file yet. */
  if (MYC_closeCache() != 0)
  {
    debug_error("Error closing cache.");
    exit(1);
  }

  debug_info("Test store server ended OK.");
  return (EXIT_SUCCESS);
}
