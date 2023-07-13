/*
 * File:   libmycache.h
 * Author: Guillermo Pérez Trabado
 *
 * This file implements the library to handle my RAM cache.
 *
 * The RAM cache is a RAM buffer between a program using records and the DB file.
 * When we want to use a record from the DB file, this library must read it from disk to RAM.
 *
 * The RAM cache is a table of BUCKETS. Each bucket may contain one register of
 * the DB file. As it is a cache, the record contained on each bucket may change.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mycache.h"
#include "debug.h"

/************************************************************
 PRIVATE VARIABLES
 ************************************************************/

/* Use global variables to keep values between functions. */
/* Add the keyword "static" to hide them so that a global variable can't be seen outside
 * this module. */

/* This will be the descriptor for the file returned by open() */
static int dbFile = -1;

/* You also need a array of buckets to use them as a RAM cache. */
static MYBUCKET_BUCKET_t *CacheEntries = NULL;

/* We also need another array of booleans to know if an entry has been written or not to disk.  */
static int *CacheDirty = NULL;

/* Debug level for messages */
static int debug_level = DEBUG_INIT;

/************************************************************
 PRIVATE FUNCTIONS
 ************************************************************/

/* Use the keyword "static" before a functions which is only used inside this file */

/**
 * Allocate memory for the cache.
 * @param n Number of buckets of the cache.
 * @return A pointer to a table with the required number of buckets.
 * NULL means a problem allocating memory.
 */
static MYBUCKET_BUCKET_t *
allocateCache(int n)
{
  return (MYBUCKET_BUCKET_t *)calloc(n, sizeof(MYBUCKET_BUCKET_t));
}

/**
 * Allocate memory for the array of booleans for the dirty flag.
 * @param n Number of buckets of the cache.
 * @return A pointer to a table with the required number of integers.
 * NULL means a problem allocating memory.
 */
static int *
allocateDirty(int n)
{
  return (int *)calloc(n, sizeof(int));
}

/**
 * Search for an unused entry in the table.
 * If there's no unused one, just one clean entry.
 * If there's no clean one, return -1.
 * @return The index of the selected entry. -1 means that no entry was unused or clean.
 */
static int
searchUnusedOrClean()
{
  for (int i = 0; i < MYC_NUMENTRIES; i++)
  {
    /* Any entry with id==0 is free. */
    if (0 == CacheEntries[i].id)
    {
      debug_verbose("returns %d.", i);
      return i;
    }
  }

  /* No unused entry in the cache. We have to reuse one clean entry. */
  for (int i = 0; i < MYC_NUMENTRIES; i++)
  {
    /* Any entry with Dirty==0 is free. */
    if (0 == CacheDirty[i])
    {
      debug_verbose("returns %d.", i);
      return i;
    }
  }
  /* No unused or clean entry */
  debug_verbose("returns %d.", -1);
  return -1;
}

/**
 * Get a random entry from the cache if all entries are dirty.
 * @return A random index.
 */
static int
searchAny()
{
  int i = rand() % MYC_NUMENTRIES;
  debug_verbose("returns %d.", i);
  return i;
}

/**
 * Search for an entry already associated with an offset of the file.
 * If there's no such entry, return -1.
 * @return The index of the entry already containing fileIndex. -1 means that no entry was found.
 */
static int
searchRecord(int fileIndex)
{
  for (int i = 0; i < MYC_NUMENTRIES; i++)
  {
    /* Check if entry contains record at fileIndex. */
    if (fileIndex == CacheEntries[i].id)
    {
      debug_verbose("returns %d.", i);
      return i;
    }
  }
  /* Not found. */
  return -1;
}

/**
 * This function reads one entry from the file into the cache.
 * The entry CachesEntries[cacheIndex] of the cache is read from the position
 * number "CacheEntries[cacheIndex].id" of the file.
 * @param cacheIndex The index of the entry in the cache.
 * @return -1 indicates an error reading the entry. 0 success.
 */
static int
readEntry(int cacheIndex)
{
  /* The memory address of the entry can be obtained with this.*/
  void *src_addr = &(CacheEntries[cacheIndex]);
  int fileIndex = CacheEntries[cacheIndex].id;

  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  /* The file contains a table of MYBUCKET_BUCKET_t. */
  /* Calculate the offset in bytes of the source on this variable. */
  int offset = fileIndex * sizeof(MYBUCKET_BUCKET_t);

  if (lseek(dbFile, offset, SEEK_SET) != offset)
  {
    debug_error("Error seeking into DB file. %s", strerror(errno));
    return -1;
  }

  /* Read the bucket containing the record from the file.
   * You use read() to read the memory contents from file. */
  // modified read for possible interruptions
  int res;
  do
  {
    res = read(dbFile, src_addr, sizeof(MYBUCKET_BUCKET_t)) != sizeof(MYBUCKET_BUCKET_t);
  } while (res == -1 && errno == EINTR);

  if (res == -1)
  {
    debug_error("Error reading from DB file. %s", strerror(errno));
    return -1;
  }
  /* Check status and return -1 in case of error. */
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  CacheDirty[cacheIndex] = 0;
  return 0;
}

/**
 * This function writes one entry of the cache to the file.
 * The entry CachesEntries[cacheIndex] of the cache is written on the position
 * number "CachesEntries[cacheIndex].id" of the file.
 * @param cacheIndex The index of the entry in the cache.
 * @return -1 indicates an error writing the entry. 0 success.
 */
static int
writeEntry(int cacheIndex)
{
  /* The memory address of the entry can be obtained with this.*/
  void *src_addr = &(CacheEntries[cacheIndex]);
  int fileIndex = CacheEntries[cacheIndex].id;

  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  /* The file contains a table of MYBUCKET_BUCKET_t. */
  /* Calculate the offset in bytes of the destination on this variable. */
  int offset = fileIndex * sizeof(MYBUCKET_BUCKET_t);

  if (lseek(dbFile, offset, SEEK_SET) != offset)
  {
    debug_error("Error seeking into DB file. %s", strerror(errno));
    return -1;
  }

  /* Write the bucket containing the record to the file.
   * You use write() to write the memory contents to file. */
  // EDITED WRITE FOR POSSIBLE INTERRUPS
  int res;
  do
  {
    res = write(dbFile, src_addr, sizeof(MYBUCKET_BUCKET_t)) != sizeof(MYBUCKET_BUCKET_t);
  } while (res == -1 && errno == EINTR);
  if (res == -1)
  {
    debug_error("Error writing to DB file. %s", strerror(errno));
    return -1;
  }
  /* Check status and return -1 in case of error. */
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  CacheDirty[cacheIndex] = 0;
  return 0;
}

/************************************************************
 PUBLIC FUNCTIONS
 ************************************************************/

/* Functions without "static" will be visible from any other C file. */

/* This function initializes the cache. */

/**
 * Initialize the cache: allocate RAM, open file, etc.
 * @return -1 in case of error during initialization. 0 means OK.
 */
int MYC_initCache()
{
  /* Allocate memory for the table of buckets. */
  CacheEntries = allocateCache(MYC_NUMENTRIES);
  /* Always check everything, warn and return an error. */
  if (CacheEntries == NULL)
  {
    debug_error("Not enough memory for the entry table.");
    return -1;
  }

  /* Allocate memory for the table of flags. */
  CacheDirty = allocateDirty(MYC_NUMENTRIES);
  /* Always check everything, warn and return an error. */
  if (CacheDirty == NULL)
  {
    debug_error("Not enough memory for the flags table.");
    return -1;
  }

  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  /* Open the DB file below. */
  /* Insert here the code to open your DB file and leave it open. */
  /* Add the flags to: READ/WRITE the file and CREATE it if it does not
   * exists before.
   * Add the permission flags to set the flags in case of creation.
   */

  dbFile = open(MYC_FILENAME, O_SYNC | O_RDWR | O_CREAT, S_IRWXU);
  if (dbFile == -1)
  {
    debug_error("Error opening DB file. %s", strerror(errno));
    return -1;
  }

  debug_info("DB file opened. (%s)", MYC_FILENAME);
  /* Don't forget to check that the open() has succeded. */
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

  /* Everything is OK */
  return 0;
}

/**
 * This function finishes the cache. It flushes all the information inside the
 * cache that is not written to the file yet and closes the file.
 * @return
 */
int MYC_closeCache()
{
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  /* Flush all dirty entries in the cache to the file. */
  MYC_flushAll();

  /* Free memory of the cache and NULLify pointers. */
  free(CacheDirty);
  CacheDirty = NULL;
  free(CacheEntries);
  CacheEntries = NULL;

  /* Close the DB file here. */
  if (close(dbFile) == -1)
  {
    debug_error("Error closing DB file. %s", strerror(errno));
    dbFile = -1;
    return -1;
  }
  /* Set the file descriptor to -1 to indicate a closed file. */
  dbFile = -1;
  debug_info("DB file closed. (%s)", MYC_FILENAME);
  return 0;
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
}

/**
 * This function copies into a record passed as argument from the cache.
 * The cache will be read from the given index of the file if not on the cache.
 * The record structure is property of the user, so we have to copy the content
 * of the cache entry onto it.
 *
 * @param fileIndex This is the index of the record in the file.
 * @param record This is a pointer to a record allocated by the user.
 * @return -1 in case of any error like I/O error when reading. 0 is OK.
 */
int MYC_readEntry(int fileIndex, MYRECORD_RECORD_t *record)
{
  /* This variable will be the index of the entry of the cache to use. */
  int cacheIndex = 0;

  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  /* REMEMBER TO USE THE AUXILIARY FUNCTIONS ABOVE. */
  /* Search the cache to guess if there's already an entry for "fileIndex". */
  /* If the record was already in the cache copy that entry. */
  cacheIndex = searchRecord(fileIndex);
  if (cacheIndex == -1)
  {
    /* If not, get an unused or clean entry to read from the file. */
    cacheIndex = searchUnusedOrClean();
    if (cacheIndex == -1)
    {
      /* If not, get a dirty entry, and flush its contents before reading from the file. */
      cacheIndex = searchAny();
      if (MYC_flushEntry(CacheEntries[cacheIndex].id) == -1)
      {
        debug_error("Error flushing entry to cache.");
        return -1;
      }
    }
    /* Remember to update the entry with the index of the file that it contains now. */
    /* Set the new entry to the current record. */
    CacheEntries[cacheIndex].id = fileIndex;
    /* Read from the file to the cache if needed. */
    if (readEntry(cacheIndex) == -1)
    {
      debug_error("Error reading entry from cache.");
      return -1;
    }
  }

  /* Copy from the record inside the cache entry to the record passed as argument.
     Remember to use the macros at mybucket.h. */
  /* Be careful with pointers: record is already a pointer (don't use & again). */
  myb_bucket2record(&CacheEntries[cacheIndex], record);
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  debug_debug("Entry %d read from cache.", fileIndex);
  return 0;
}

/*
 * This function copies a record passed as argument into the cache.
 * The record will be written at the given index of the file LATER.
 * This funtions does not write the cache entry to the file inmediately.
 * The record structure is property of the user, so we have to copy its
 * content to the entry as the record can be deallocated by the user.
 *
 * @param fileIndex This is the index of the record in the file.
 * @param record This is a pointer to a record allocated by the user.
 * @return -1 in case of any error like I/O error when writing. 0 is OK.
 */
int MYC_writeEntry(int fileIndex, MYRECORD_RECORD_t *record)
{
  /* This variable will be the index of the entry of the cache. */
  int cacheIndex = 0;

  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  /* REMEMBER TO USE THE AUXILIARY FUNCTIONS ABOVE. */
  /* Search the cache to guess if there's already an entry for "fileIndex". */
  /* If the record was already in the cache use that entry. */
  cacheIndex = searchRecord(fileIndex);
  if (cacheIndex == -1)
  {
    /* If not, get an unused or clean entry. */
    cacheIndex = searchUnusedOrClean();
    if (cacheIndex == -1)
    {
      /* If not, get a dirty entry, and flush it before writing on it. */
      cacheIndex = searchAny();
      if (writeEntry(cacheIndex) == -1)
      {
        debug_error("Error flushing entry to cache.");
        return -1;
      }
    }
    /* Set the new entry to the current record. */
    CacheEntries[cacheIndex].id = fileIndex;
  }

  /* Overwrite = copy from the record passed as argument to the record inside the bucket.
     Remember to use the macros at mybucket.h. */
  /* Be careful with pointers: record is already a pointer (don't use & again). */
  myb_record2bucket(record, &CacheEntries[cacheIndex]);
  CacheDirty[cacheIndex] = 1;
  CacheEntries[cacheIndex].id = fileIndex;
  /* Remember to update the entry with the index of the file that contains. */
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  debug_debug("Entry %d written to cache.", fileIndex);
  return 0;
}

/**
 * Forces the cache to write the contents of the entry containing the record at
 * "fileIndex" in the file.
 * @param fileIndex This is the index of the entry of the file to be flushed.
 * @return -1 in case of I/O error. 0 is OK.
 */
int MYC_flushEntry(int fileIndex)
{
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  /* Go through the cache and search the entry holding the record
   * number "fileIndex" of the file. */
  int cacheIndex = searchRecord(fileIndex);
  /* If the entry is dirty, write it to disk. */
  if (CacheDirty[cacheIndex])
  {
    if (writeEntry(cacheIndex) == -1)
    {
      debug_error("Error flushing entry to cache.");
      return -1;
    }
  }
  /* Always check errors*/
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  debug_debug("Entry %d flushed to disk.", fileIndex);
  return 0;
}

/**
 * Flush any dirty entry in the cache inmediately.
 * @return
 */
int MYC_flushAll()
{
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  /* Go through the cache and write all dirty entries to the file. */
  for (int cacheIndex = 0; cacheIndex < MYC_NUMENTRIES; cacheIndex++)
  {
    if (CacheDirty[cacheIndex])
    {
      if (writeEntry(cacheIndex) == -1)
      {
        debug_error("Error flushing entry to cache.");
        return -1;
      }
    }
  }
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  debug_debug("All entries flushed to disk.");
  return 0;
}

/* Increases current debug level or reset to 0 if maximum is reached. */
void MYC_debuglevel_rotate()
{
  debuglevel_rotate();
  debug_info("Rotating debug level. Current level=%d.", debug_level);
}
