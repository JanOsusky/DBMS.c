/* 
 * File:   mystore_cli.h
 * Author: Guillermo Pérez Trabado
 *
 * This file defines the headers of a library to communicate with the store server.
 * 
 */

#ifndef MYSTORE_CLI_H
#define MYSTORE_CLI_H

#include <stdint.h>
#include <sys/types.h>

#include <myrecord.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Initialize the client API: open message queue.
   * @return -1 in case of error during initialization. 0 means OK.
   */
  int STORC_init ();

  /**
   *This function closes only the client API. Not the storage server.
   * @return -1 in case of error during cleaning. 0 means OK.
   */
  int STORC_close ();

  /**
   * This function reads a record from the store server.
   * @param fileIndex This is the index of the record to read.
   * @param record This is a pointer to a record allocated by the user.
   * @return Return the status from the server. 0 is OK. -1 means some error
   * using the queue. -2 means that the queue was removed (server is not running).
   */
  int STORC_read (int fileIndex, MYRECORD_RECORD_t *record);

  /**
   * This function writes a record to the store server.
   * @param fileIndex This is the index of the record to write.
   * @param record This is a pointer to a record allocated by the user.
   * @return Return the status from the server. 0 is OK. -1 means some error
   * using the queue. -2 means that the queue was removed (server is not running).
   */
  int STORC_write (int fileIndex, MYRECORD_RECORD_t *record);

  /* This function flushes this record index inside the storage server. */
  int STORC_flush (int fileIndex);

  /* This function flushes all the entries in the storage server. */
  int STORC_flushAll ();

#ifdef __cplusplus
}
#endif

#endif /* MYSTORE_CLI_H */

