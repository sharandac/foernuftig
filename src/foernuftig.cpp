/**
 * @file foernuftig.cpp
 * @author Dirk Broßwick (dirk.brosswick@googlemail.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <Arduino.h>

#include "config.h"
#include "config/foernuftig_config.h"
#include "foernuftig.h"

#include "utils/alloc.h"
#include "modules/button.h"
#include "modules/mqttclient.h"
#include "modules/webserver.h"
/**
 * local variables
 */
foernuftig_config_t foernuftig_config;              /** @brief foernuftig config struct */
bool foernuftig_initialized = false;                /** @brief flag if foernuftig is initialized */
volatile int rpm_counter = 0;                       /** @brief rpm counter */
volatile int current_rpm = 0;                       /** @brief current rpm */ 
volatile uint64_t rpm_counter_time_stamp = 0;       /** @brief time stamp for rpm counter */
/**
 * local static funtions
 */
static bool foernuftig_mqtt_cb( EventBits_t event, void *arg );
static bool foernuftig_webserver_cb( EventBits_t event, void *arg );
/**
 * @brief foernuftig rpm pulse counter
 */
void IRAM_ATTR foernuftig_IRQ( void ) {
    rpm_counter++;
}

void foernuftig_init( void ) {
    /**
     * check if already init
     */
    ASSERT( !foernuftig_initialized, "foernuftig already initialized" );
    /**
     * load config
     */
    foernuftig_config.load();
    /**
     * setup pins
     */
    pinMode( foernuftig_config.supply_pin, OUTPUT );
    digitalWrite( foernuftig_config.supply_pin, LOW );

    ledcSetup( 0, 100, 8 );
    ledcAttachPin( foernuftig_config.clk_pin, 0 );
    ledcWrite( 0, 128 );
    pinMode( foernuftig_config.clk_pin, OUTPUT );

    pinMode( foernuftig_config.fg_pin, INPUT_PULLDOWN );
    attachInterrupt( foernuftig_config.fg_pin, &foernuftig_IRQ, GPIO_INTR_POSEDGE );
    rpm_counter_time_stamp = millis();

    pinMode( foernuftig_config.led_pin, OUTPUT );
    digitalWrite( foernuftig_config.led_pin, LOW );

    mqtt_client_register_cb( MQTTSTAT_JSON_DATA | MQTTCMND_JSON_DATA, foernuftig_mqtt_cb, "foernuftig" );
    asyncwebserver_register_cb( WS_DATA | WEB_DATA | WEB_MENU | SAVE_CONFIG | RESET_CONFIG, foernuftig_webserver_cb, "/foernuftig.htm" );

    foernuftig_initialized = true;
}

/**
 * @brief foernuftig mqttclient callback function
 * 
 * @param event         event mask
 * @param arg           pointer to optional data
 * @return true 
 * @return false 
 */
static bool foernuftig_mqtt_cb( EventBits_t event, void *arg ) {
    bool retval = false;
    mqttData_t *mqttData = (mqttData_t*)arg;
    DynamicJsonDocument& doc = *mqttData->doc;

    switch( event ) {
        case MQTTSTAT_JSON_DATA:
            /**
             * check json data
             */
            ASSERT( mqttData->doc, "foernuftig: mqttstat json data is NULL");
            /**
             * insert foernuftig data
             */
            doc["rpm"] = current_rpm;
            retval = true;
            break;
        case MQTTCMND_JSON_DATA:
            /**
             * check json data
             */
            ASSERT( mqttData->doc, "foernuftig: mqttstat json data is NULL");
            /**
             * insert foernuftig data
             */
            if( doc.containsKey( "rpm" ) ) {
                foernuftig_set_rpm( doc["rpm"] );
            }
            if( doc.containsKey( "set_level" ) ) {
                foernuftig_set_level( doc["set_level"] );
            }
            retval = true;
            break;
    }
    return( retval );
}
/**
 * @brief foernuftig webserver callback function
 * 
 * @param event         event mask
 * @param arg           pointer to optional data
 * @return true 
 * @return false 
 */
static bool foernuftig_webserver_cb( EventBits_t event, void *arg ) {
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
    ASSERT( foernuftig_initialized, "foernuftig not initialized, check your code" );
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
                foernuftig_config.save();
            }
            else if ( !strcmp("get_foernuftig_settings", cmd ) ) {
                asyncwebserver_send_websocket_msg("foernuftig_fg_pin\\%d", foernuftig_config.fg_pin );
                asyncwebserver_send_websocket_msg("foernuftig_led_pin\\%d", foernuftig_config.led_pin );
                asyncwebserver_send_websocket_msg("foernuftig_clk_pin\\%d", foernuftig_config.clk_pin );
                asyncwebserver_send_websocket_msg("foernuftig_supply_pin\\%d", foernuftig_config.supply_pin );
                asyncwebserver_send_websocket_msg("foernuftig_rpm_factor\\%f", foernuftig_config.rpm_factor );
                asyncwebserver_send_websocket_msg("foernuftig_min_rpm\\%d", foernuftig_config.min_rpm );
                asyncwebserver_send_websocket_msg("foernuftig_mid_rpm\\%d", foernuftig_config.mid_rpm );
                asyncwebserver_send_websocket_msg("foernuftig_max_rpm\\%d", foernuftig_config.max_rpm );
                asyncwebserver_send_websocket_msg("foernuftig_current_rpm\\%d", current_rpm );
            }
            else if ( !strcmp("foernuftig_fg_pin", cmd ) ) {
                foernuftig_config.fg_pin = atoi( value );
                asyncwebserver_send_websocket_msg( "foernuftig_fg_pin\\%d", foernuftig_config.fg_pin );
            }
            else if ( !strcmp("foernuftig_led_pin", cmd ) ) {
                foernuftig_config.led_pin = atoi( value );
                asyncwebserver_send_websocket_msg( "foernuftig_led_pin\\%d", foernuftig_config.led_pin );
            }
            else if ( !strcmp("foernuftig_clk_pin", cmd ) ) {
                foernuftig_config.clk_pin = atoi( value );
                asyncwebserver_send_websocket_msg( "foernuftig_clk_pin\\%d", foernuftig_config.clk_pin );
            }
            else if( !strcmp( "foernuftig_supply_pin", cmd ) ) {
                foernuftig_config.supply_pin = atoi( value );
                asyncwebserver_send_websocket_msg( "foernuftig_supply_pin\\%d", foernuftig_config.supply_pin );
            }
            else if( !strcmp( "foernuftig_rpm_factor", cmd ) ) {
                foernuftig_config.rpm_factor = atof( value );
                asyncwebserver_send_websocket_msg( "foernuftig_rpm_factor\\%f", foernuftig_config.rpm_factor );
            }
            else if( !strcmp( "foernuftig_min_rpm", cmd ) ) {
                foernuftig_config.min_rpm = atoi( value );
                asyncwebserver_send_websocket_msg( "foernuftig_min_rpm\\%d", foernuftig_config.min_rpm );
            }
            else if( !strcmp( "foernuftig_mid_rpm", cmd ) ) {
                foernuftig_config.mid_rpm = atoi( value );
                asyncwebserver_send_websocket_msg( "foernuftig_mid_rpm\\%d", foernuftig_config.mid_rpm );
            }
            else if( !strcmp( "foernuftig_max_rpm", cmd ) ) {
                foernuftig_config.max_rpm = atoi( value );
                asyncwebserver_send_websocket_msg( "foernuftig_max_rpm\\%d", foernuftig_config.max_rpm );
            }
            else if( !strcmp( "foernuftig_set_level", cmd ) ) {
                foernuftig_set_level( atoi( value ) );
            }
            else if ( !strcmp( "get_foernuftig_data", cmd ) ) {
                asyncwebserver_send_websocket_msg( "foernuftig_current_rpm\\%d", current_rpm );
            }
            retval = true;
            break;
        case WEB_DATA:
            ASSERT( request, "request is NULL, check your code!" );
            /**
             * html page and send it
             */
            html = html_header;
            html += asyncwebserver_get_menu();
            html += foernuftig_config_page;
            html += html_footer;
            request->send(200, "text/html", html);
            retval = true;
            break;
        case WEB_MENU:
            asyncwebserver_set_menu_entry( "/foernuftig.htm", "Foernuftig" );
            retval = true;
            break;
        case SAVE_CONFIG:
            foernuftig_config.save();
            retval = true;
            break;
        case RESET_CONFIG:
            foernuftig_config.resetToDefault();
            retval = true;
            break;
    }
    return( retval );
}

/**
 * @brief foernuftig loop function
 * 
 */
void foernuftig_loop( void ) {
    static uint64_t NextFoernuftigMillis = 0;
    static uint64_t NextFoernuftigRpmMillis = 0;
    /**
     * check if already init
     */
    ASSERT( foernuftig_initialized, "foernuftig not initialized, check your code" );
    /**
     * first run init all things
     */
    if( NextFoernuftigMillis == 0 ) {
        NextFoernuftigMillis = millis();
        NextFoernuftigRpmMillis = millis();
    }
    
    if( NextFoernuftigMillis < millis() ) {
        NextFoernuftigMillis += 50l;

        switch( button_get() ) {
            case BUTTON_OFF:
                foernuftig_set_rpm( 0 );
                log_i("foernuftig off");
                break;
            case BUTTON_I:
                foernuftig_set_rpm( foernuftig_config.min_rpm );
                log_i("foernuftig min");
                break;
            case BUTTON_II:
                foernuftig_set_rpm( foernuftig_config.mid_rpm );
                log_i("foernuftig mid");
                break;
            case BUTTON_III:
                foernuftig_set_rpm( foernuftig_config.max_rpm );
                log_i("foernuftig max");
                break;
            default:
                break;
        }
    }

    if( NextFoernuftigRpmMillis < millis() ) {
        NextFoernuftigRpmMillis += 1000l;

        if( rpm_counter_time_stamp != 0 ) {
            current_rpm = ( rpm_counter * 1000 / ( millis() - rpm_counter_time_stamp ) ) * foernuftig_config.rpm_factor;
            rpm_counter = 0;
            rpm_counter_time_stamp = millis();
        }
    }
}

int foernuftig_get_rpm( void ) {
    return( current_rpm );
}

void foernuftig_set_led( uint8_t enable ) {
    digitalWrite( foernuftig_config.led_pin, enable );
}

void foernuftig_set_level( int level ) {
    if( !foernuftig_initialized )
        return;

    switch( level ) {
        case 0:
            foernuftig_set_rpm( 0 );
            break;
        case 1:
            foernuftig_set_rpm( foernuftig_config.min_rpm );
            break;
        case 2:
            foernuftig_set_rpm( foernuftig_config.mid_rpm );
            break;
        case 3:
            foernuftig_set_rpm( foernuftig_config.max_rpm );
            break;
    }
}

void foernuftig_set_rpm( int rpm ) {
    if( !foernuftig_initialized )
        return;

    if( rpm >= foernuftig_config.min_rpm && rpm <= foernuftig_config.max_rpm ) {
        ledcSetup( 0, rpm, 8 );
        ledcWrite( 0, 128 );
        digitalWrite( foernuftig_config.supply_pin, HIGH );
    }
    else if( rpm == 0 )
        digitalWrite( foernuftig_config.supply_pin, LOW );
}

void foernuftig_save_settings( void ) {
    foernuftig_config.save();
}