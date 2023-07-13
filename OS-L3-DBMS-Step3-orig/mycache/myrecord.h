/* 
 * File:   myrecord.h
 * Author: Guillermo Pérez Trabado
 *
 * This is the definition of one single record for my simple database.
 * We only have one table with a record with fixed fields.
 * Records are managed on RAM and preserved on DISK.
 * 
 * You can change the fields of the record to personalize your database table.
 */

#ifndef MYRECORD_H
#define MYRECORD_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MYRECORD_NAMELENGTH 16

  /* This is the definition of one record in my database. */
  typedef struct
  {
    unsigned int registerid;
    int age;
    int gender;
    char name[MYRECORD_NAMELENGTH];
  } MYRECORD_RECORD_t;

  /* We define some useful macros to help writing code. */

  /* This MACRO allocates space for a record. */
#define myb_newrecord() ((MYRECORD_RECORD_t *)calloc(1,sizeof(MYRECORD_RECORD_t)))
  /* You don't need a special function to free but this macro is nicer. */
#define myb_freerecord(r) free(r)


#ifdef __cplusplus
}
#endif

#endif /* MYRECORD_H */

