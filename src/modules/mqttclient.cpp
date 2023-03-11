/**
 * @file mqttclient.cpp
 * @author Dirk Broßwick ( dirk.brosswick@uni-jena.de )
 * @brief 
 * @version 0.1
 * @date 2021-11-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <soc/rtc.h>
#include <esp_task_wdt.h>

#include "config/mqtt_config.h"

#include "config.h"
#include "utils/alloc.h"
#include "mqttclient.h"
#include "wificlient.h"
#include "webserver.h"
#include "ntp.h"
/**
 * local variables
 */
AsyncMqttClient mqttClient;                     /** @brief mqtt client */
TaskHandle_t _ASYNCMQTT_Task;                   /** @brief handle for mqtt task */
mqtt_config_t mqtt_config;                      /** @brief MQTT config */
callback_t *mqttclient_callback = NULL;         /** @brief callback function for mqtt */
static bool mqttclient_initialized = false;     /** @brief flag for mqtt client initialized */
/**
 * local mqtt client string and topics
 */
char ClientID[128] = "";                        /** @brief ClientID for mqtt */
char mqtt_stats_topic[128] = "";                /** @brief mqtt stats topic */
char mqtt_cmnd_topic[128] = "";                 /** @brief mqtt cmnd topic */
char mqtt_tele_topic[128] = "";                 /** @brief mqtt tele topic */
char reset_state[64]="";                        /** @brief reset state */
/**
 * local static funtions
 */
static bool mqtt_client_webserver_cb( EventBits_t event, void *arg );
static void mqtt_client_Task( void * pvParameters );

void onMqttConnect(bool sessionPresent) {
    time_t now;
    struct tm info;
    char time_str[32] = "";

    snprintf( mqtt_stats_topic, sizeof( mqtt_stats_topic ), MQTT_STAT, mqtt_config.topic, wificlient_get_hostname() );
    snprintf( mqtt_cmnd_topic, sizeof( mqtt_cmnd_topic ), MQTT_CMND, mqtt_config.topic, wificlient_get_hostname() );
    snprintf( mqtt_tele_topic, sizeof( mqtt_tele_topic ), MQTT_TELE, mqtt_config.topic, wificlient_get_hostname() );

    log_i( "MQTT-Client: connected to [%s]", mqtt_config.server );
    mqttClient.subscribe( mqtt_stats_topic, 0 );
    log_i( "MQTT-Client: subscribe [%s]:%s", mqtt_config.server, mqtt_stats_topic );
    mqttClient.subscribe( mqtt_cmnd_topic, 0 );
    log_i( "MQTT-Client: subscribe [%s]:%s", mqtt_config.server, mqtt_cmnd_topic );
    mqttClient.subscribe( mqtt_tele_topic, 0 );
    log_i( "MQTT-Client: subscribe [%s]:%s", mqtt_config.server, mqtt_tele_topic );

    String json;
    String ip;
    DynamicJsonDocument doc(4096);

    ip = WiFi.localIP().toString();
    time( &now );
    localtime_r( &now, &info );
    strftime( time_str, sizeof( time_str ), "%Y-%m-%d %H:%M.%S", &info );

    doc["id"] = wificlient_get_hostname();
    doc["ip"] = ip.c_str();
    doc["time"] = time_str;
    doc["uptime"] = millis() / 1000;
    doc["reset_state"] = reset_state;
    doc["mac"] = WiFi.macAddress();
    doc["channel"] = WiFi.channel();
    doc["ssid"] = WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();

    serializeJson( doc, json );
    mqtt_client_tele_publish( (char*)json.c_str() );
}

/**
 * @brief mqtt client callback function on disconnect
 * 
 * @param reason 
 */
static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    log_i( "MQTT-Client: Disconnected from MQTT" );
}

/**
 * @brief mqtt client callback function on subscribe
 * 
 * @param packetId 
 * @param qos 
 */
static void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
    log_d( "MQTT-Client: Subscribe acknowledged [%d]", packetId );
}

/**
 * @brief mqtt client callback function on unsubscribe
 * 
 * @param packetId 
 */
static void onMqttUnsubscribe(uint16_t packetId) {
    log_d( "MQTT-Client: Unsubscribe acknowledged [%d]", packetId );
}

/**
 * @brief mqtt client callback function on message
 * 
 * @param topic             topic of the message
 * @param payload           payload of the message
 * @param properties        properties of the message
 * @param len               length of the message
 * @param index             index of the message
 * @param total             total length of the message
 */
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    
    char *msg = (char *)CALLOC_ASSERT( 1, len + 1, "mqtt client msg buffer alloc failed" );
    memcpy( (void*)msg, payload, len );

    log_d("MQTT-Client: Publish received, topic: [%s] , payload: [%s]", topic, msg );

    if( !strcmp( topic, mqtt_cmnd_topic ) ) {
        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, msg );
        if ( !error) {
            /**
             * mqttclient callback structure
             */
            mqttData_t mqttData;
            mqttData.doc = &doc;
            mqttData.topic = topic;
            mqttData.msg = msg;
            callback_send( mqttclient_callback, MQTTCMND_JSON_DATA, (void*)&mqttData );
        }
        else {
            /**
             * mqttclient callback structure
             */
            mqttData_t mqttData;
            mqttData.topic = topic;
            mqttData.msg = msg;
            callback_send( mqttclient_callback, MQTTCMND_RAW_DATA, (void*)&mqttData );
        }
    }

    free( msg );
}

/**
 * @brief mqtt client publish function
 * 
 * @param topic 
 * @param payload 
 */
void mqtt_client_publish( char * topic, const char * fmt, ... ) {
    va_list args;
    va_start(args, fmt);

    char *msg = NULL;
    int size = vasprintf( &msg, fmt, args );
    va_end(args);

    if( size )
        mqttClient.publish( topic , 1, false, fmt, strlen( fmt ), false, 0 );

    if( msg )
        free( msg );
}

/**
 * @brief send a mqtt message via stat topic
 * 
 * @param fmt       pointer to the message
 * @param ... 
 */
void mqtt_client_stat_publish( const char * fmt, ... ) {
    va_list args;
    va_start(args, fmt);

    char *msg = NULL;
    int size = vasprintf( &msg, fmt, args );
    va_end(args);

    if( size )
        mqttClient.publish( mqtt_stats_topic , 1, false, fmt, strlen( fmt ), false, 0 );

    if( msg )
        free( msg );
}

/**
 * @brief send a mqtt message via tele topic
 * 
 * @param fmt       pointer to the message
 * @param ... 
 */
void mqtt_client_tele_publish( const char * fmt, ... ) {
    va_list args;
    va_start(args, fmt);

    char *msg = NULL;
    int size = vasprintf( &msg, fmt, args );
    va_end(args);

    if( size )
        mqttClient.publish( mqtt_tele_topic , 1, false, fmt, strlen( fmt ), false, 0 );

    if( msg )
        free( msg );
}

/**
 * @brief send a mqtt message via cmnd topic
 * 
 * @param fmt       pointer to the message
 * @param ... 
 */
void mqtt_client_cmnd_publish( const char * fmt, ... ) {
    va_list args;
    va_start(args, fmt);

    char *msg = NULL;
    int size = vasprintf( &msg, fmt, args );
    va_end(args);

    if( size )
        mqttClient.publish( mqtt_stats_topic , 1, false, fmt, strlen( fmt ), false, 0 );

    if( msg )
        free( msg );
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
bool mqtt_client_register_cb( EventBits_t event, CALLBACK_FUNC callback_func, const char *id ) {
    if ( mqttclient_callback == NULL ) {
        mqttclient_callback = callback_init( "mqtt client" );
        ASSERT( mqttclient_callback, "mqtt client callback alloc failed" );
    }    
    return( callback_register( mqttclient_callback, event, callback_func, id ) );
}

void mqtt_client_Task( void * pvParameters ) {
    uint64_t NextWifiMillis = 0;        /** @brief next wifi connect try */
    uint64_t NextMqttMillis = 0;        /** @brief next mqtt connect try */
    time_t now;                         /** @brief current time */
    struct tm info;                     /** @brief current time info */
    char time_str[32] = "";             /** @brief current time string */
    
    log_i( "Start MQTT-Client on Core: %d", xPortGetCoreID() );
    /**
     * get reset reason
     */
    esp_reset_reason_t why = esp_reset_reason();
    switch ( why ) {
        case (ESP_RST_POWERON):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_POWERON" );
            break;
        case (ESP_RST_UNKNOWN):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_UNKNOWN" );
            break;
        case (ESP_RST_EXT):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_EXT" );
            break;
        case (ESP_RST_SW):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_SW" );
            break;
        case (ESP_RST_PANIC):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_PANIC" );
            break;
        case (ESP_RST_INT_WDT):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_INT_WDT" );
            break;
        case (ESP_RST_TASK_WDT):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_TASK_WDT" );
            break;
        case (ESP_RST_WDT):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_WDT" );
            break;
        case (ESP_RST_DEEPSLEEP):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_DEEPSLEEP" );
            break;
        case (ESP_RST_BROWNOUT):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_BROWNOUT" );
            break;
        case (ESP_RST_SDIO):
            snprintf( reset_state, sizeof( reset_state ), "ESP_RST_SDIO" );
            break;
        default:
            break;
    }
    /**
     * load config
     */
    mqtt_config.load();
    /**
     * set call back functions
     */
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.setKeepAlive( 10 );
    /**
     * set flag for mqtt client init
     */
    mqttclient_initialized = true;
    /**
     * delay task
     */
    vTaskDelay( 1000 );
    /**
     * wait for wifi
     */
    while( !WiFi.isConnected() ){};
    NextWifiMillis = millis();
    NextMqttMillis = millis();
    /**
     * add watchdog
     */
    esp_task_wdt_add( NULL );
    /**
     * loop forever
     */
    while( true ) {
        /**
         * check mqtt connection state
         */
        if ( NextWifiMillis < millis() ) {
            NextWifiMillis += 1000l;
            if ( WiFi.status() == WL_CONNECTED && !mqttClient.connected() ) {
                mqttClient.setServer( mqtt_config.server , mqtt_config.port );
                mqttClient.setClientId( wificlient_get_hostname() );
                mqttClient.setCredentials( mqtt_config.username, mqtt_config.password );
                mqttClient.connect();
            }
        }
        /**
         * mqtt work interval
         */
        if ( NextMqttMillis < millis() ) {
            NextMqttMillis += mqtt_config.interval * 1000l;

            if ( mqttClient.connected() ) {
                String json;
                String ip;
                DynamicJsonDocument doc(4096);
                /**
                 * get ip and local time
                 */
                ip = WiFi.localIP().toString();
                time( &now );
                localtime_r( &now, &info );
                strftime( time_str, sizeof( time_str ), "%Y-%m-%d %H:%M.%S", &info );
                /**
                 * add stat some data
                 */
                doc["id"] = wificlient_get_hostname();
                doc["ip"] = ip.c_str();
                doc["time"] = time_str;
                doc["uptime"] = millis() / 1000;
                doc["reset_state"] = reset_state;
                doc["interval"] = mqtt_config.interval;
                /**
                 * call all registered callback functions
                 */
                mqttData_t mqttData;
                mqttData.doc = &doc;
                callback_send( mqttclient_callback, MQTTSTAT_JSON_DATA, (void*)&mqttData );
                /**
                 * send json to mqtt server
                 */
                serializeJson( doc, json );
                mqtt_client_stat_publish( (char*)json.c_str() );
            }
        }
        /**
         * wait and reset watchdog
         */
        esp_task_wdt_reset();
        vTaskDelay( 1 );
    }
}

void mqtt_client_StartTask( void ) {
    /**
     * create mqtt client task
     */
    xTaskCreatePinnedToCore(    mqtt_client_Task,       /* Function to implement the task */
                                "mqttclient Task",      /* Name of the task */
                                16000,                  /* Stack size in words */
                                NULL,                   /* Task input parameter */
                                1,                      /* Priority of the task */
                                &_ASYNCMQTT_Task,       /* Task handle. */
                                1 );                    /* Core where the task should run */
    /**
     * register callback function
     */
    asyncwebserver_register_cb( WS_DATA | WEB_DATA | WEB_MENU | SAVE_CONFIG | RESET_CONFIG, mqtt_client_webserver_cb, "/mqttclient.htm" );
}

/**
 * @brief mqtt client webserver callback
 * 
 * @param event         event mask
 * @param arg           pointer to wsData_t
 * @return true 
 * @return false 
 */
static bool mqtt_client_webserver_cb( EventBits_t event, void *arg ) {
    bool retval = false;
    wsData_t *ws_data = (wsData_t *)arg;
    AsyncWebSocketClient * client = ws_data->client;
    AsyncWebServerRequest * request = ws_data->request;
    const char *cmd = ws_data->cmd ? ws_data->cmd : "";
    const char *value = ws_data->value ? ws_data->value : "";
    String html = "";
    /**
     * check if mqtt client is initialized
     */
    ASSERT( mqttclient_initialized, "mqtt client not initialized, check your code!");
    /**
     * check event
     */
    switch( event ) {
        case WS_DATA:
            ASSERT( client, "client is NULL, check your code!" );
            /**
             * check all commands
             */
            if ( !strcmp( "SAV", cmd ) ) {
                mqtt_config.save();
                mqttClient.disconnect();
            }
            else if ( !strcmp("get_mqtt_settings", cmd ) ) {
                asyncwebserver_send_websocket_msg("mqtt_server\\%s", mqtt_config.server );
                asyncwebserver_send_websocket_msg("mqtt_port\\%d", mqtt_config.port );
                asyncwebserver_send_websocket_msg("mqtt_username\\%s", mqtt_config.username );
                asyncwebserver_send_websocket_msg("mqtt_password\\%s", "********" );
                asyncwebserver_send_websocket_msg("mqtt_topic\\%s", mqtt_config.topic );
                asyncwebserver_send_websocket_msg("mqtt_interval\\%d", mqtt_config.interval );
                asyncwebserver_send_websocket_msg("checkbox\\mqtt_realtimestats\\%s", mqtt_config.realtimestats ? "true" : "false" );
            }
            else if ( !strcmp("mqtt_server", cmd ) ) {
                strncpy( mqtt_config.server, value, sizeof( mqtt_config.server ) );
                asyncwebserver_send_websocket_msg( "mqtt_server\\%s", mqtt_config.server );
            }
            else if ( !strcmp("mqtt_port", cmd ) ) {
                mqtt_config.port = atoi( value );
                asyncwebserver_send_websocket_msg( "mqtt_port\\%d", mqtt_config.port );
            }
            else if ( !strcmp("mqtt_username", cmd ) ) {
                strncpy( mqtt_config.username, value, sizeof( mqtt_config.username ) );
                asyncwebserver_send_websocket_msg( "mqtt_username\\%s", mqtt_config.username );
            }
            else if ( !strcmp("mqtt_password", cmd ) ) {
                if ( strcmp( "********", value ) )
                    strncpy( mqtt_config.password, value, sizeof( mqtt_config.password ) );
            }
            else if ( !strcmp("mqtt_topic", cmd ) ) {
                strncpy( mqtt_config.topic, value, sizeof( mqtt_config.topic ) );
                asyncwebserver_send_websocket_msg( "mqtt_topic\\%s", mqtt_config.topic );
            }
            else if ( !strcmp("mqtt_interval", cmd ) ) {
                mqtt_config.interval = atoi( value );
                asyncwebserver_send_websocket_msg( "mqtt_interval\\%d", mqtt_config.interval );
            }
            else if ( !strcmp("mqtt_realtimestats", cmd ) ) {
                mqtt_config.realtimestats = atoi( value ) ? true : false;
                asyncwebserver_send_websocket_msg( "checkbox\\mqtt_realtimestats\\%s", mqtt_config.realtimestats ? "true" : "false" );
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
            html += mqtt_config_page;
            html += html_footer;
            request->send(200, "text/html", html);
            retval = true;
            break;
        case WEB_MENU:
            asyncwebserver_set_menu_entry( "/mqttclient.htm", "mqtt settings" );
            retval = true;
            break;
        case SAVE_CONFIG:
            mqtt_config.save();
            retval = true;
            break;
        case RESET_CONFIG:
            mqtt_config.resetToDefault();
            retval = true;
            break;
    }
    return( retval );
}

void mqtt_client_save_settings( void ) {
    mqtt_config.save();
    mqttClient.disconnect();
}