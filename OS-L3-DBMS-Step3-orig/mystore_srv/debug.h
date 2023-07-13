/* 
 * File:   debug.h
 * Author: Guillermo Pérez Trabado
 *
 * We define here the some macros for managing debug messages and debug levels.
 */

#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <errno.h>
#include <string.h>

  /* Debug levels */
#define DEBUG_ERROR 0
#define DEBUG_INFO 1
#define DEBUG_DEBUG 2
#define DEBUG_VERBOSE 3
#define DEBUG_INIT DEBUG_INFO

  /* Define several macros for debug messages */
#define debuglevel_increase() {if(debug_level<DEBUG_VERBOSE) debug_level++;}
#define debuglevel_decrease() {if(debug_level>DEBUG_ERROR) debug_level--;}
#define debuglevel_rotate() {if(debug_level<DEBUG_VERBOSE) debug_level++;else debug_level=DEBUG_ERROR;}

#define debug_error(...) {if(debug_level>=DEBUG_ERROR){fprintf(stderr,"%s:%s()::ERROR ",__FILE__,__func__);fprintf(stderr, __VA_ARGS__);fputc('\n',stderr);}}
#define debug_perror(...) {if(debug_level>=DEBUG_ERROR){fprintf(stderr,"%s:%s()::ERROR ",__FILE__,__func__);fprintf(stderr, __VA_ARGS__);fputs(strerror(errno),stderr);fputc('\n',stderr);}}
#define debug_info(...) {if(debug_level>=DEBUG_INFO){fprintf(stderr,"%s:%s()::INFO ",__FILE__,__func__);fprintf(stderr, __VA_ARGS__);fputc('\n',stderr);}}
  /* Use conditional compilation to remove this code from program. */
#ifdef DEBUG_LIB
#define debug_debug(...) {if(debug_level>=DEBUG_DEBUG){fprintf(stderr,"%s:%s()::DEBUG ",__FILE__,__func__);fprintf(stderr, __VA_ARGS__);fputc('\n',stderr);}}
#define debug_verbose(...) {if(debug_level>=DEBUG_VERBOSE){fprintf(stderr,"%s:%s()::VERBOSE ",__FILE__,__func__);fprintf(stderr, __VA_ARGS__);fputc('\n',stderr);}}
#else
#define debug_debug(...) {}
#define debug_verbose(...) {}
#endif


#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H */

