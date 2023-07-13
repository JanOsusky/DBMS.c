/* 
 * File:   libmystore_srv.h
 * Author: Guillermo Pérez Trabado
 *
 * This file implements the library to implement communications at the
 * store server side.
 * The communication uses System V IPC message queues.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "mystore_srv.h"
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
 * Initialize the server library: open message queue, etc.
 * @return -1 in case of error during initialization. 0 means OK.
 */
int
STORS_init ()
{

  /* Open message queue. Create it in the server side exclusively.
   * It should failt if it already exists (see flag EXCL in man page).
   * The server should detect that another instance is running with
   * this method.
   */

  /* Select the key for our message queue. Using UID is better to guarantee that
   your queue is unique if other users are running a similar daemon. */
  key_t key = MYSTORE_API_KEY;

  debug_verbose ("Opening message queue in server API... (key=0x%08x)", key);

  /* Create a non existing message queue. An error will be returned if it does
   * exist. That indicates that another server is running. */
  message_queue = msgget (key, IPC_CREAT | IPC_EXCL | S_IRWXU);

  if (message_queue == -1)
    {
      debug_perror ("Error creating message queue in server API (key=0x%08x).", key);
      return -1;
    }

  /* Don't forget to check status of system calls. */
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

  debug_info ("Message queue opened in server API. (key=0x%08x)", key);
  /* Everything is OK */
  return 0;
}

/**
 * This function finishes the server side of the library. It removes the message
 * queue to avoid clients sending further messages.
 * @return -1 if some error happened during removal of the queue. 0 means OK.
 */
int
STORS_close ()
{
  /* Close the message queue. Remove it! */
  int status = msgctl (message_queue, IPC_RMID, NULL);
  if (status != 0)
    {
      debug_perror ("Error removing message queue in server API (key=0x%08x).", MYSTORE_API_KEY);
    }

  debug_info ("Message queue removed in server API. (key=0x%08x)", MYSTORE_API_KEY);

  /* Set the message queue descriptor to -1 to indicate it is not open. */
  message_queue = -1;
  return 0;
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
}

/**
 * This function reads a request from the message queue.
 * This function will wait blocked until it receives a request.
 * When returning, the request passed by reference as pameter will contain the
 * data of the request received from the message queue.
 * The request will be processes outside this library.
 * @param request Is a pointer to a request structure to return a request
 * received from the client.
 * @return Return 0 if OK. -1 in case of some error receiving.
 * -2 indicates that a signal interrupted the reception of a message.
 */
int
STORS_readrequest (request_message_t *request)
{
  debug_verbose ("Receiving request from client (type=%d).", MYSAPMT_REQUEST);

  /* Wait for a request received from a client through the message queue.
   */
  int status = msgrcv (message_queue, request, sizeof (request_message_t), MYSAPMT_REQUEST, 0);
  if (status == -1)
    {
      if (errno == EINTR)
        {
          debug_info ("Signal received while reading a request.");
          return -2;
        }
      debug_perror ("Error receiving request from message queue. %s");
      return -1;
    }
  debug_debug ("Request received from client (cliend id=%d, op=%d, idx=%d).", request->return_to, request->requested_op, request->index);
  /* If no error, return 0 and the request contains the received one. */
  return 0;
}

/**
 * This function sends an answer structure to a client through a message queue.
 * @param answer This structure is already initialized and ready to be sent.
 * @return Return 0 if OK. -1 in case of some error sending the answer.
 */
int
STORS_sendanswer (answer_message_t *answer)
{
  /* Send an answer message to some client. */
  /* The answer already contains the type field with the identity of the client
   * which will receive the answer. This is came in the request structure.
   */
  debug_verbose ("Sending answer to client (cliend id=%d, status=%d).", answer->mtype, answer->status);

  /* Remember to create a unique number for each client and add it to the request in the client side.
   */
  int status = msgsnd (message_queue, answer, sizeof (answer_message_t), 0);
  if (status == -1)
    {
      debug_perror ("Error sending answer message. %s");
      return -1;
    }
  debug_debug ("Answer sent to client (cliend id=%d, status=%d).", answer->mtype, answer->status);
  /* If no error, return 0 and the request contains the received one. */
  return 0;
}

/* Increases current debug level or reset to 0 if maximum is reached. */
void
STORS_debuglevel_rotate ()
{
  debuglevel_rotate ();
  debug_info ("Rotating debug level. Current level=%d.", debug_level);
}
