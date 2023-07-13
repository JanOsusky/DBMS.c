/* 
 * File:   regcache.h
 * Author: Guillermo Pérez Trabado
 *
 * This file defines the headers of a library to handle my RAM cache.
 * 
 * The RAM cache is a RAM buffer between a program using records and the DB file.
 * When we want to use a record from the DB file, this library must read it from disk to RAM.
 * 
 * The RAM cache is a table of BUCKETS. Each bucket may contain one register of
 * the DB file. As it is a cache, the record contained on each bucket may change.
 */

#ifndef MYCACHE_H
#define MYCACHE_H

#include <stdint.h>
#include <sys/types.h>

#include "mybucket.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* This is the size of our cache in buckets.  */
#define MYC_NUMENTRIES 64

  /* This is the default name of the DB file. */
#define MYC_FILENAME "myDBtable.dat"

  /* This function initializes the cache. */
  int MYC_initCache ();
  /* This function closes the cache. It flushes all the information inside the
     cache that is not written to the file yet. */
  int MYC_closeCache ();

  /* This function reads a record from the file (at given index)
   * inside the record passed as argument. */
  int MYC_readEntry (int fileIndex, MYRECORD_RECORD_t *record);

  /* This function writes a record into the cache from the record passed as argument.
   * The record will be written at the given index of the file later.
   * This funtions does not write the cache entry to the file inmediately. */
  int MYC_writeEntry (int fileIndex, MYRECORD_RECORD_t *record);
  /* This function flushes one cache entry containing one record to be written
   * at the given index. */
  int MYC_flushEntry (int fileIndex);
  /* This function flushes all the entries of the cache to the file. */
  int MYC_flushAll ();

  /* Increases current debug level or reset to 0 if maximum is reached. */
  void MYC_debuglevel_rotate ();

#ifdef __cplusplus
}
#endif

#endif /* MYCACHE_H */

