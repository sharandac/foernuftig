/**
 * @file callback.h
 * @author Dirk Broßwick (dirk.brosswick@googlemail.com)
 * @brief 
 * @version 0.1
 * @date 2023-03-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _CALLBACK_H
    #define _CALLBACK_H

    #include <stdint.h>

    typedef uint32_t EventBits_t;

    /**
     * @brief prio type def
     */
    typedef enum {
        CALL_CB_NUM_START = 0,          /** @brief enum only for enum counting */
        CALL_CB_FIRST,                  /** @brief highest prio level */
        CALL_CB_MIDDLE,                 /** @brief middle prio level */
        CALL_CB_LAST,                   /** @brief lowestt prio level */
        CALL_CB_NUM                     /** @brief enum only for enum counting */
    } callback_prio_t;
    /**
     * @brief typedef for the callback function call
     * 
     * @param event     event mask
     * @param arg       void pointer to an argument
     * 
     * @return          true if success or false if failed
     */
    typedef bool ( * CALLBACK_FUNC ) ( EventBits_t event, void *arg );
    /**
     * @brief callback table entry structure
     */
    typedef struct {
        EventBits_t event;                  /** @brief event mask */
        CALLBACK_FUNC callback_func;        /** @brief pointer to a callback function */
        const char *id;                     /** @brief id for the callback */
        bool active;                        /** @brief true if callback function is activated, false is deactivated */
        callback_prio_t prio;               /** @brief order to call cb functions, CALL_CB_FIRST means first */
        uint64_t counter;                   /** @brief callback function call counter thair returned true */
    } callback_table_t;
    /**
     * @brief callback head structure
     */
    typedef struct callback_t {
        uint32_t entrys;                    /** @brief count callback entrys */
        bool debug;                         /** @brief debug flag, if TRUE to get debug messages */
        callback_table_t *table;            /** @brief pointer to an callback table */
        const char *name;                   /** @brief id for the callback structure */
        callback_t *next_callback_t;        
    } callback_t;
    /**
     * @brief init the callback structure
     * 
     * @param   name        pointer to an string thats contains the name for the callback table
     * 
     * @return  pointer to a callback_t structure if success, NULL if failed
     */
    callback_t *callback_init( const char *name );
    /**
     * @brief   register an callback function
     * 
     * @param   callback        pointer to a callback_t structure
     * @param   event           event filter mask
     * @param   callback_func   pointer to a callbackfunc
     * @param   id              pointer to an string thats contains the id aka name for the callback function
     * 
     * @note    prio is set to CALL_CB_MIDDLE as default
     * 
     * @return  true if success, false if failed
     */
    bool callback_register( callback_t *callback, EventBits_t event, CALLBACK_FUNC callback_func, const char *id );
    /**
     * @brief   register an callback function and their prio
     * 
     * @param   callback        pointer to a callback_t structure
     * @param   event           event filter mask
     * @param   callback_func   pointer to a callbackfunc
     * @param   id              pointer to an string thats contains the id aka name for the callback function
     * @param   prio            prio ( CALL_CB_FIRST, CALL_CB_MIDDLE, CALL_CB_LAST )
     * 
     * @return  true if success, false if failed
     */
    bool callback_register_with_prio( callback_t *callback, EventBits_t event, CALLBACK_FUNC callback_func, const char *id, callback_prio_t prio );
    /**
     * @brief   call all callback function thats match with the event filter mask and their id
     * 
     * @param   callback        pointer to a callback_t structure
     * @param   event           event filter mask
     * @param   arg             argument for the called callback function
     * @param   id              pointer to an string thats contains the id aka name for the callback function
     * 
     * @return  true if success, false if failed
     */
    bool callback_send_to_id( callback_t *callback, EventBits_t event, void *arg, const char *id );
    /**
     * @brief   call all callback function thats match with the event filter mask
     * 
     * @param   callback        pointer to a callback_t structure
     * @param   event           event filter mask
     * @param   arg             argument for the called callback function
     * 
     * @return  true if success, false if failed
     */
    bool callback_send( callback_t *callback, EventBits_t event, void *arg );
    /**
     * @brief   call all callback function thats match with the event filter mask in reverse order
     * 
     * @param   callback        pointer to a callback_t structure
     * @param   event           event filter mask
     * @param   arg             argument for the called callback function
     * 
     * @return  true if success, false if failed
     */
    bool callback_send_reverse( callback_t *callback, EventBits_t event, void *arg );
    /**
     * @brief   call all callback function thats match with the event filter mask without logging
     * 
     * @param   callback        pointer to a callback_t structure
     * @param   event           event filter mask
     * @param   arg             argument for the called callback function
     * 
     * @return  true if success, false if failed
     */
    bool callback_send_no_log( callback_t *callback, EventBits_t event, void *arg );
    /**
     * @brief prints out the complete callback table and their entrys
     */
    void callback_print( void );

#endif // _CALLBACK_H