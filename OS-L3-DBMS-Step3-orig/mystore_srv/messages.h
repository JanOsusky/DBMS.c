/* 
 * File:   messages.h
 * Author: Guillermo Pérez Trabado
 *
 * This file defines private data types used for messages between client and server.
 * 
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

  /* This is the default to get a unique message queue key for each user. */
#define MYSTORE_API_KEY ((key_t)getuid())
  /* This is the type to identify each client with a unique type. */
#define MYSTORE_API_CLIENT ((long)getpid())

  typedef enum
  {
    /* Request from client to server. */
    MYSAPMT_REQUEST = 1,
    /* Request a client identifier for threads. */
    MYSPMT_GETCLID = 2,
    /* Send to any client using this destination tag.
     * Only the first waiting will get the mesage. */
    MYSAPMT_ANYCLIENT = 3
  } MYSTORE_API_MTYPES;

  /**
   * We can define an enum instead of an integer to give unique numbers to
   * protocol operations.
   */
  typedef enum
  {
    MYSCOP_READ = 0,
    MYSCOP_WRITE
    /* Any other operation will have its own number here. */
  } MYSTORE_CLI_OP;

  /**
   * Message for a request from the client.
   */
  typedef struct
  {
    long mtype; /* This type distinguishes messages to server from messages to clients. */
    MYSTORE_CLI_OP requested_op; /* This is the selected operation to perform. */
    long return_to; /* The client sends a type to address the reply to because we may have several clients. */
    MYRECORD_RECORD_t data; /* This field contains a record only when writing. */
    int index; /* Record index to read or write */

    /* Did you forget some other field? Add it to the message. */
  } request_message_t;

  /**
   * Message for an answer from the server.
   */
  typedef struct
  {
    long mtype; /* This type distinguishes messages to server from messages to clients. */
    int status; /* This status passes back the result of each operation. */
    MYRECORD_RECORD_t data; /* This field contains a record only when reading. */
    /* Did you forget some other field? Add it to the message. */
  } answer_message_t;

#ifdef __cplusplus
}
#endif

#endif /* MESSAGES_H */

