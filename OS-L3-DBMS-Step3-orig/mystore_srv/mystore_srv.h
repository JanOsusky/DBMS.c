/* 
 * File:   mystore_srv.h
 * Author: Guillermo Pérez Trabado
 *
 * This file defines the headers of a library to implement communications at the
 * store server side. The library allows to receive request messages and to send
 * back answers.
 * 
 */

#ifndef MYSTORE_CLI_H
#define MYSTORE_CLI_H

#include <stdint.h>
#include <sys/types.h>

#include <myrecord.h>
#include <messages.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Initialize the server library: create message queue, etc.
   * 
   * The queue is created exclusively to guarantee that no other instance of
   * the server is running with the same queue.
   * @return -1 in case of error during initialization. 0 means OK.
   */
  int STORS_init ();

  /**
   * This function finishes the server side of the library. It removes the message
   * queue to avoid clients sending further messages.
   * 
   * @return -1 if some error happened during removal of the queue. 0 means OK.
   */
  int STORS_close ();

  /**
   * This function reads a request from the message queue.
   * This function will wait blocked until it receives a request.
   * When returning, the request passed by reference as pameter will contain the
   * data of the request received from the message queue.
   * The request will be processes outside this library.
   * @param request Is a pointer to a request structure to return a request
   * received from the client.
   * @return Return 0 if OK. -1 in case of some error receiving.
   */
  int STORS_readrequest (request_message_t *request);

  /**
   * This function sends an answer structure to a client through a message queue.
   * @param answer This structure is already initialized and ready to be sent.
   * @return Return 0 if OK. -1 in case of some error sending the answer.
   */
  int STORS_sendanswer (answer_message_t *answer);

  /* Increases current debug level or reset to 0 if maximum is reached. */
  void STORS_debuglevel_rotate ();

#ifdef __cplusplus
}
#endif

#endif /* MYSTORE_CLI_H */

