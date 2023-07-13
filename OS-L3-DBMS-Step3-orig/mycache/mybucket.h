/* 
 * File:   mybucket.h
 * Author: Guillermo Pérez Trabado
 *
 * We define here the BUCKET structure. Bucket contains one record plus some
 * control fields to control the space on which a record is stored.
 * We use buckets both on disk and on RAM to control what record is stored
 * on each bucket on RAM or DISK.
 */

#ifndef MYBUCKET_H
#define MYBUCKET_H

#include <string.h>
#include "myrecord.h"

#ifdef __cplusplus
extern "C" {
#endif


    /* We define this macro to avoid writing this expression many times. */
#define MYBUCKET_RECORDSIZE (sizeof(MYRECORD_RECORD_t))

    /* BUCKET: This structure contains one single record and additional control fields. */
    typedef struct {
        /* This is the space in bytes needed to store one record inside this bucket.
         * The unsigned char type is the most similar type to a byte. */
        unsigned char record[MYBUCKET_RECORDSIZE];
        /* This is the number of the record: 0 means that these bucket is empty. */
        unsigned int id;
    } MYBUCKET_BUCKET_t;


    /* We define some useful macros to help writing code.
     * All this macros need pointers as arguments.
     * Be careful passing pointers, not structs.
     */

    /* This function copies the memory contents of a record from a record to a bucket. */
#define myb_record2bucket(r, b) memcpy(&((b)->record), (r), MYBUCKET_RECORDSIZE);
    /* This function copies the memory contents of a record from a bucket to a record. */
#define myb_bucket2record(b, r) memcpy((r), &((b)->record), MYBUCKET_RECORDSIZE);

#ifdef __cplusplus
}
#endif

#endif /* MYBUCKET_H */

