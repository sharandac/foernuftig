/**
 * @file ntp.cpp
 * @author Dirk Broßwick (dirk.brosswick@googlemail.com)
 * @brief 
 * @version 0.1
 * @date 2023-03-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <Arduino.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <time.h>

#include "ntp.h"
#include "webserver.h"
#include "config.h"
#include "config/ntp_config.h"
#include "utils/alloc.h"
#include "utils/callback.h"
/**
 * local variables
 */
TaskHandle_t _NTP_Task;                 /** @brief handle for ntp task */
ntp_config_t ntp_config;                /** @brief ntp config */
callback_t *ntp_callback = NULL;        /** @brief callback function for ntp */
/**
 * local static funtions
 */
static void ntp_Task( void * pvParameters );
static bool ntp_webserver_cb( EventBits_t event, void *arg );

void ntp_StartTask( void ) {  
    /**
     * load ntp config
     */
    ntp_config.load();
    /**
     * start ntp task
     */
    xTaskCreatePinnedToCore(    ntp_Task,           /* Function to implement the task */
                                "ntp Task",         /* Name of the task */
                                2500,               /* Stack size in words */
                                NULL,               /* Task input parameter */
                                1,                  /* Priority of the task */
                                &_NTP_Task,         /* Task handle. */
                                1 );                /* Core where the task should run */   
    /**
     * register webserver callback function
     */
    asyncwebserver_register_cb_with_prio( WS_DATA | WEB_DATA | WEB_MENU | SAVE_CONFIG | RESET_CONFIG, ntp_webserver_cb, "/ntp.htm", CALL_CB_MIDDLE ); 
}

/**
 * @brief ntp task
 * 
 * @param pvParameters 
 */
static void ntp_Task( void * pvParameters ) {
    struct tm info;                         /** @brief current time */
    char time_str[32] = "";                 /** @brief current time string */
    static uint64_t NextMillis = millis();  /** @brief next update time */
    
    log_i("Start NTP Task on Core: %d", xPortGetCoreID() );
    /**
     * wait for wifi
     */
    while( !WiFi.isConnected() ){};
    /**
     * send ntp started event
     */
    callback_send( ntp_callback, NTP_START, NULL );
    /**
     * add watchdog
     */
    esp_task_wdt_add( NULL );
    /**
     * set timezone
     */
    while ( true ) {
        /**
         * update needed?
         */
        if ( NextMillis < millis() ) {
            /**
             * set next update
             */
            NextMillis += 15 * 1000l;
            /**
             * check if wifi is connected
             */
            if ( WiFi.isConnected() && ntp_config.enable ) {
                log_i( "NTP-client: renew time" );
                /**
                 * set timezone and server
                 */
                configTzTime( ntp_config.timezone, ntp_config.server );
                /**
                 * get time
                 */
                if( getLocalTime( &info ) ) {
                    NextMillis = millis() + NTP_RENEW_INTERVAL * 1000l;
                    callback_send( ntp_callback, NTP_SYNC, NULL );
                    strftime( time_str, sizeof( time_str ), "%Y-%m-%d %H:%M.%S", &info );
                    log_i("NTP-client: time is %s", time_str );
                }
                else
                    log_e( "NTP-client: Failed to obtain time" );
            }
        }
        /**
         * wait and reset watchdog
         */
        esp_task_wdt_reset();
        vTaskDelay( 5000 );
    }
}

/**
 * @brief register a callback function for a event
 * 
 * @param event             event mask
 * @param callback_func     pointer to the callback function
 * @param id                id of the callback function
 * @return true 
 * @return false 
 */
bool ntp_register_cb( EventBits_t event, CALLBACK_FUNC callback_func, const char *id ) {
    if ( ntp_callback == NULL ) {
        ntp_callback = callback_init( "ntp client" );
        ASSERT( ntp_callback, "ntp client callback alloc failed" );
    }    
    return( callback_register( ntp_callback, event, callback_func, id ) );
}

/**
 * @brief webserver callback for index.htm
 * 
 * @param event     event bitmask
 * @param arg       pointer to additional data
 * @return true 
 * @return false 
 */
static bool ntp_webserver_cb( EventBits_t event, void *arg ) {
    bool retval = false;
    wsData_t *ws_data = (wsData_t *)arg;
    AsyncWebSocketClient * client = ws_data->client;
    AsyncWebServerRequest * request = ws_data->request;
    const char *cmd = ws_data->cmd ? ws_data->cmd : "";
    const char *value = ws_data->value ? ws_data->value : "";
    String html = "";
    /**
     * check event
     */
    switch( event ) {
        case WS_DATA:
            ASSERT( client, "client is NULL, check your code!");
            /**
             * check all commands
             */
            if ( !strcmp( "SAV", cmd ) ) {
                ntp_config.save();
                callback_send( ntp_callback, NTP_CONFIG_CHANGE, NULL );
            }
            else if ( !strcmp("get_ntp_settings", cmd ) ) {
                asyncwebserver_send_websocket_msg( "ntp_server\\%s", ntp_config.server );
                asyncwebserver_send_websocket_msg( "ntp_timezone\\%s", ntp_config.timezone );
                asyncwebserver_send_websocket_msg( "ntp_interval\\%d", ntp_config.interval );
                asyncwebserver_send_websocket_msg( "checkbox\\ntp_enable\\%s", ntp_config.enable ? "true" : "false ");
            }
            else if ( !strcmp( "ntp_server", cmd ) ) {
                strncpy( ntp_config.server, value, sizeof( ntp_config.server ) );
                asyncwebserver_send_websocket_msg( "ntp_server\\%s", ntp_config.server );
            }
            else if ( !strcmp( "ntp_timezone", cmd ) ) {
                strncpy( ntp_config.timezone, value, sizeof( ntp_config.timezone ) );
                asyncwebserver_send_websocket_msg( "ntp_timezone\\%s", ntp_config.timezone );
            }
            else if ( !strcmp( "ntp_interval", cmd ) ) {
                ntp_config.interval = atoi( value );
                asyncwebserver_send_websocket_msg( "ntp_interval\\%d", ntp_config.interval );
            }
            else if ( !strcmp( "ntp_enable", cmd ) ) {
                ntp_config.enable = atoi( value ) ? true : false;
                asyncwebserver_send_websocket_msg( "checkbox\\ntp_enable\\%s", ntp_config.enable ? "true" : "false " );
            }
            retval = true;
            break;
        case WEB_DATA:
            ASSERT( request, "request is NULL, check your code!");
            /**
             * html page and send it
             */
            html = html_header;
            html += asyncwebserver_get_menu();
            html += ntp_config_page;
            html += html_footer;
            request->send(200, "text/html", html);
            retval = true;
            break;
        case WEB_MENU:
            asyncwebserver_set_menu_entry( "/ntp.htm", "ntp" );
            retval = true;
            break;
        case SAVE_CONFIG:
            ntp_config.save();
            retval = true;
            break;
        case RESET_CONFIG:
            ntp_config.resetToDefault();
            retval = true;
            break;
    }
    return( retval );
}