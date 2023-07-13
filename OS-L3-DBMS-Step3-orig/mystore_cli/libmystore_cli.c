/* 
 * File:   libmystore_cli.h
 * Author: Guillermo Pérez Trabado
 *
 * This file implements the library to communicate with the store server.
 * The communication uses System V IPC message queues.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mystore_cli.h"
#include "messages.h"
#include "debug.h"

/************************************************************
 PRIVATE VARIABLES
 ************************************************************/

/* Use global variables to keep values between functions. */
/* Add the keyword "static" to hide them so that a global variable can't be seen outside
 * this module. */

/* This will be the descriptor for the message queue. */
static int message_queue = -1;

/* Debug level for messages */
static int debug_level = DEBUG_INIT;

/************************************************************
 PRIVATE FUNCTIONS
 ************************************************************/


/* Write any private function you need in this part of the file. */

/* Use the keyword "static" before a function which is only used inside this file */


/************************************************************
 PUBLIC FUNCTIONS
 ************************************************************/

/* Functions without "static" will be visible from any other C file. */

/**
 * Initialize the client API: open message queue, etc.
 * @return -1 in case of error during initialization. 0 means OK.
 */
int
STORC_init ()
{
  /* Open message queue. Do not create it in the client if it does not exist yet.
   *  If the server is the only one creating the queue, the client could
   *  detect when the server is not running. */

  /* Select the key for our message queue. Using UID is better to guarantee that
   your queue is unique if other users are running a similar daemon. */
  key_t key = MYSTORE_API_KEY;

  debug_verbose ("Opening message queue in client API. (key=0x%08x)", key);

  /* Open an existing message queue. An error will be returned if it does not
   * exist. That indicates that the server is not running. */
  message_queue = msgget (key, 0);

  if (message_queue == -1)
    {
      debug_perror ("Error opening message queue in client API.");
      return -1;
    }
  /* Don't forget to check status of system calls. */
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

  debug_info ("Message queue opened in client API. (key=0x%08x)", key);
  /* Everything is OK */
  return 0;
}

/**
 * This function finishes the client API. You should not remove the queue in 
 * the client as thre may be more clients.
 * @return -1 in case of error during cleaning. 0 means OK.
 */
int
STORC_close ()
{

  /* Close the message queue. Do not remove it! */
  /*    ==> No need to do anything in the client. Queue can't be closed. */
  debug_info ("Message queue closed in client API.");

  /* Set the message queue descriptor to -1 to indicate it is not open. */
  message_queue = -1;
  return 0;
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
}

/**
 * This function reads a record from the store server.
 * @param fileIndex This is the index of the record to read.
 * @param record This is a pointer to a record allocated by the user.
 * @return Return the status from the server. 0 is OK. -1 means some error
 * using the queue. -2 means that the queue was removed (server is not running).
 */
int
STORC_read (int fileIndex, MYRECORD_RECORD_t *record)
{
  /* Remember to create a unique number for each client and add it to the request.
   * The PID of the process is OK as identifier by now. */

  /* Send a request message to the server indicating the index to read. */
  request_message_t request;
  /* The server will be receiving only on this type. */
  request.mtype = MYSAPMT_REQUEST;
  /* This is the default method to get a unique client identifier.
   This is not valid if we used several threads inside a process. */
  request.return_to = MYSTORE_API_CLIENT;
  /* This is the operation code. */
  request.requested_op = MYSCOP_READ;
  /* This is the argument to the read operation. */
  request.index = fileIndex;

  debug_verbose ("Sending request to server (idx=%d).", fileIndex);

  /* Send the request to the server. */
  int status = msgsnd (message_queue, &request, sizeof (request), 0);
  if (status == -1)
    {
      debug_perror ("Error sending message.");
      return -1;
    }

  /* Wait for an answer message from the server. This client should wait using its unique
   number to avoid that other clients steal the answer to this client. */
  answer_message_t answer;

  debug_verbose ("Receiving answer from server (client id=%d).", request.return_to);
  /* Receive the answer using the message client identifier. */
  status = msgrcv (message_queue, &answer, sizeof (answer), request.return_to, 0);
  if (status == -1)
    {
      debug_perror ("Error receiving answer.");
      return -1;
    }
  debug_debug ("Answer received from server (status=%d).", answer.status);
  /* Check return status and copy the record from the message if OK. */
  if (answer.status == 0)
    {
      /* Copy the contents of the answer, not the pointer!!!! */
      *record = answer.data;
    }
  /* Copy return status from server. */
  return answer.status;
}

/**
 * This function writes a record to the store server.
 * @param fileIndex This is the index of the record to write.
 * @param record This is a pointer to a record allocated by the user.
 * @return Return the status from the server. 0 is OK. -1 means some error
 * using the queue. -2 means that the queue was removed (server is not running).
 */
int
STORC_write (int fileIndex, MYRECORD_RECORD_t *record)
{
  /* Send a request message to the server indicating the index to write and
   * include the record data to write. */

  /* Remember to create a unique number for each client and add it to the request.
   * The PID of the process is OK as identifier by now. */

  /* Send a request message to the server indicating the index to read. */
  request_message_t request;
  /* The server will be receiving only on this type. */
  request.mtype = MYSAPMT_REQUEST;
  /* This is the default method to get a unique client identifier.
   This is not valid if we used several threads inside a process. */
  request.return_to = MYSTORE_API_CLIENT;
  /* This is the operation code. */
  request.requested_op = MYSCOP_WRITE;
  /* This are the arguments to the write operation. */
  request.index = fileIndex;
  request.data = *record;

  debug_verbose ("Sending request to server (idx=%d).", fileIndex);

  /* Send the request to the server. */
  int status = msgsnd (message_queue, &request, sizeof (request), 0);
  if (status == -1)
    {
      debug_perror ("Error sending message.");
      return -1;
    }

  /* Wait for an answer message from the server. This client should wait using its unique
   number to avoid that other clients steal the answer to this client. */
  answer_message_t answer;

  debug_verbose ("Receiving answer from server (client id=%d).", request.return_to);
  /* Receive the answer using the message client identifier. */
  status = msgrcv (message_queue, &answer, sizeof (answer), request.return_to, 0);
  if (status == -1)
    {
      debug_perror ("Error receiving answer.");
      return -1;
    }
  debug_debug ("Answer received from server (status=%d).", answer.status);
  /* Copy return status from server. */
  return answer.status;
}
