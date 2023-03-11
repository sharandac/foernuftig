/**
 * @file alloc.h
 * @author Dirk Broßwick (dirk.brosswick@googlemail.com)
 * @brief 
 * @version 0.1
 * @date 2023-03-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _ALLOC_H
    #define _ALLOC_H

    #include <stdio.h>
    #include <stdlib.h>

    #include <stddef.h>
    #include <stdbool.h>
    #include <esp32-hal-psram.h>
    #include <Arduino.h>

    #if defined ( BOARD_HAS_PSRAM )
        #define MALLOC         ps_malloc            /** @brief malloac from PSRAM */
        #define CALLOC         ps_calloc            /** @brief calloc from PSRAM */
        #define REALLOC        ps_realloc           /** @brief realloc from PSRAM */
    #else
        #define MALLOC         malloc               /** @brief malloac from normal heap */
        #define CALLOC         calloc               /** @brief calloc from normal heap */
        #define REALLOC        realloc              /** @brief realloc from normal heap */
    #endif

    #define     ASSERT( test, message, ... ) do { if( !(test) ) { log_e( message, ##__VA_ARGS__); while( true ); } } while ( 0 )
    #define     MALLOC_ASSERT( size, message, ... ) ( { void *p = (void*)MALLOC( size ); ASSERT( p, message, ##__VA_ARGS__ ); p; } ) 
    #define     CALLOC_ASSERT( nmemb, size, message, ... ) ( { void *p = (void*)CALLOC( nmemb, size ); ASSERT( p, message, ##__VA_ARGS__ ); p; } ) 
    #define     REALLOC_ASSERT( ptr, size, message, ... ) ( { void *p = (void*)REALLOC( ptr, size ); ASSERT( p, message, ##__VA_ARGS__ ); p; } ) 

#endif // _ALLOC_H
