/**
 * @file button.c
 * @author Dirk Broßwick ( dirk.brosswick@uni-jena.de )
 * @brief 
 * @version 0.1
 * @date 2022-01-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <Arduino.h>
#include "config.h"
#include "button.h"
#include "config/button_config.h"
#include "modules/webserver.h"
#include "utils/alloc.h"

button_config_t button_config;
QueueHandle_t button_queue;
static int button_state = 0;
static bool button_initialized = false;

static bool button_webserver_cb( EventBits_t event, void *arg );

void button_init( void ) {
    /**
     * check if already init
     */
    if( button_initialized ) {
        log_e("buttons already initialized");
        return;
    }
    /**
     * load config
     */
    button_config.load();
    /**
     * setup button
     */
    pinMode( button_config.button_0, INPUT );
    pinMode( button_config.button_1, INPUT );
    pinMode( button_config.button_2, INPUT );
    pinMode( button_config.button_3, INPUT );
    /**
     * create button queue
     */
    button_queue = xQueueCreate( 10, sizeof( int ) );
    /**
     * register webserver callback
     */
    asyncwebserver_register_cb( WS_DATA | WEB_DATA | WEB_MENU | SAVE_CONFIG | RESET_CONFIG, button_webserver_cb, "/button.htm" );
    /**
     * set init flag
     */
    button_initialized = true;
}

/**
 * @brief webserver button callback function
 * 
 * @param event         event bitmask
 * @param arg           pointer to optional data
 * @return true 
 * @return false 
 */
static bool button_webserver_cb( EventBits_t event, void *arg ) {
    bool retval = false;
    wsData_t *ws_data = (wsData_t *)arg;
    AsyncWebSocketClient * client = ws_data->client;
    AsyncWebServerRequest * request = ws_data->request;
    const char *cmd = ws_data->cmd ? ws_data->cmd : "";
    const char *value = ws_data->value ? ws_data->value : "";
    String html = "";
    /**
     * check if already init
     */
    ASSERT( button_initialized, "button are not in initialized, check your code" );
    /**
     * check event
     */
    switch( event ) {
        case WS_DATA:
            ASSERT( client, "client is NULL, check your code" );
            /**
             * check all commands
             */
            if ( !strcmp( "SAV", cmd ) ) {
                button_config.save();
                log_i("save button config");
            }
            else if ( !strcmp("get_button_settings", cmd ) ) {
                asyncwebserver_send_websocket_msg("button_off_pin\\%d", button_config.button_0 );
                asyncwebserver_send_websocket_msg("button_i_pin\\%d", button_config.button_1 );
                asyncwebserver_send_websocket_msg("button_ii_pin\\%d", button_config.button_2 );
                asyncwebserver_send_websocket_msg("button_iii_pin\\%d", button_config.button_3 );
                asyncwebserver_send_websocket_msg("button_state\\%d", button_state );
            }
            else if ( !strcmp("button_off_pin", cmd ) ) {
                button_config.button_0 = atoi( value );
                asyncwebserver_send_websocket_msg("button_off_pin\\%d", button_config.button_0 );
            }
            else if ( !strcmp("button_i_pin", cmd ) ) {
                button_config.button_1 = atoi( value );
                asyncwebserver_send_websocket_msg("button_i_pin\\%d", button_config.button_1 );
            }
            else if ( !strcmp("button_ii_pin", cmd ) ) {
                button_config.button_2 = atoi( value );
                asyncwebserver_send_websocket_msg("button_ii_pin\\%d", button_config.button_2 );
            }
            else if ( !strcmp("button_iii_pin", cmd ) ) {
                button_config.button_3 = atoi( value );
                asyncwebserver_send_websocket_msg("button_iii_pin\\%d", button_config.button_3 );
            }
            retval = true;
            break;
        case WEB_DATA:
            ASSERT( request, "request is NULL, check your code" );
            /**
             * html page and send it
             */
            html = html_header;
            html += asyncwebserver_get_menu();
            html += button_config_page;
            html += html_footer;
            request->send(200, "text/html", html);
            retval = true;
            break;
        case WEB_MENU:
            asyncwebserver_set_menu_entry( "/button.htm", "button settings" );
            retval = true;
            break;
        case SAVE_CONFIG:
            button_config.save();
            retval = true;
            break;
        case RESET_CONFIG:
            button_config.resetToDefault();
            retval = true;
            break;
    }
    return( retval );
}

void button_loop( void ) {
    static uint64_t NextButtonMillis = 0;
    /**
     * check if already init
     */
    ASSERT( button_initialized, "button are not in initialized, check your code" );
    /**
     * first run init all things
     */
    if( NextButtonMillis == 0 )
        NextButtonMillis = millis();
    /**
     * work in buttons
     */
    if( NextButtonMillis < millis() ) {
        NextButtonMillis += 50l;
        int button_state_tmp = 0;
        int button = BUTTON_NONE;
        /**
         * read button state into one byte
         */
        button_state_tmp = digitalRead( button_config.button_0 ) ? 0 : 1;
        button_state_tmp |= digitalRead( button_config.button_1 ) ? 0 : 2;
        button_state_tmp |= digitalRead( button_config.button_2 ) ? 0 : 4;
        button_state_tmp |= digitalRead( button_config.button_3 ) ? 0 : 8;
        /**
         * check if button state changed
         */
        if( button_state_tmp != button_state ) {
            button_state = button_state_tmp;
            /**
             * check button state
             */
            switch( button_state ) {
                case 0b0001:
                    button = BUTTON_OFF;
                    log_i("button off");
                    break;
                case 0b0010:
                    button = BUTTON_I;
                    log_i("button I");
                    break;
                case 0b0100:
                    button = BUTTON_II;
                    log_i("button II");
                    break;
                case 0b1000:
                    button = BUTTON_III;
                    log_i("button III");
                    break;
            }
        }
        /**
         * send button event to queue
         */
        if( button != BUTTON_NONE ) {
            xQueueSend( button_queue, &button, 0 );
            asyncwebserver_send_websocket_msg( "button_state\\%d", button );
        }
    }
}

void button_inject( int key ) {
    if( button_initialized )
        xQueueSend( button_queue, &key, 0 );  
}

int button_get( void ) {
    int retval = BUTTON_NONE;
    
    if( button_initialized )
        xQueueReceive( button_queue, &retval, 0 );        

    return( retval );
}

void button_save_settings( void ) {
    button_config.save();
}