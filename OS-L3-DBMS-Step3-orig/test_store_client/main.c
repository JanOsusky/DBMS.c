/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Guillermo Pérez Trabado
 *
 * Created on 06 de may de 2021, 15:01
 */

#include <stdio.h>
#include <stdlib.h>

#include <mycache.h>
#include <mystore_cli.h>

#include "debug.h"

/* Debug level for messages */
static int debug_level = DEBUG_INIT;

/* Number of registers to use in the test. */
#define TEST_LENGTH 68
#define NUMBER_CACHE_ENTRIES 64

/*
 * This test uses the mystore client library to create some registers and then reads them again.
 * The requests need the store_engine server to be running to receive the requests.
 */
int
main (int argc, char** argv)
{

  /************************************************************/
  /* WRITE TEST */
  /************************************************************/

  /* This function initializes the client API. */
  if (STORC_init () != 0)
    {
      debug_error ("Error initializing client API.");
      exit (1);
    }

  debug_info ("Test store client started OK.");
  debug_info ("Write test started...");

  /* This is a variable to hold one record. */
  MYRECORD_RECORD_t record;

  /* Fill register n times and create or update records. */
  for (int k = 0; k < 100; k++)
    for (int j = 1; j < TEST_LENGTH - NUMBER_CACHE_ENTRIES; j++)
      for (int i = j; i < j + NUMBER_CACHE_ENTRIES; i++)
        {
          /* Id of the record. */
          record.registerid = i;
          record.age = i;
          record.gender = -1;
          /* This is a fixed length string. Print into it directly. */
          snprintf (record.name, sizeof (record.name), "reg #%d", i);

          /* Write record into storage. */
          if (STORC_write (i, &record) != 0)
            {
              debug_error ("Error writing to the storage.");
              exit (1);
            }
          debug_debug ("idx: %d write OK", i);
        }

  /* This function ends the API. Not the server. */
  if (STORC_close () != 0)
    {
      debug_error ("Error closing API.");
      exit (1);
    }

  debug_info ("Write test ended OK.");

  /************************************************************/
  /* READ TEST */
  /************************************************************/
  debug_info ("Read test started...");

  /* Open API again. */
  if (STORC_init () != 0)
    {
      debug_error ("Error initializing client API.");
      exit (1);
    }

  /* Read register from storage. */
  for (int k = 0; k < 100; k++)
    for (int j = 1; j < TEST_LENGTH - NUMBER_CACHE_ENTRIES; j++)
      for (int i = j; i < j + NUMBER_CACHE_ENTRIES; i++)
        {
          /* Read from store server. */
          if (STORC_read (i, &record) != 0)
            {
              debug_error ("Error reading from server.");
              exit (1);
            }
          /* Check that the index from the record matches the position. */
          if (record.registerid != i)
            {
              debug_error ("Register at %d contains id %d.", i, record.registerid);
            }
          else
            {
              debug_debug ("idx: %d read OK", i);
              debug_verbose ("id: %u, age: %d, gender: %d, name: %s", record.registerid, record.age, record.gender, record.name);
            }
        }

  /* This function closes the API. */
  if (STORC_close () != 0)
    {
      debug_error ("Error closing client.");
      exit (1);
    }

  debug_info ("Read test ended OK.");

  debug_info ("Test store client ended OK.");

  return (EXIT_SUCCESS);
}

