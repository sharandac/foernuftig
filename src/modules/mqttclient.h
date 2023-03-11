/**
 * @file mqttclient.h
 * @author Dirk Broßwick ( dirk.brosswick@uni-jena.de )
 * @brief 
 * @version 0.1
 * @date 2021-11-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _MQTTCLIENT_H
    #define _MQTTCLIENT_H

    #include <utils/callback.h>
    #include <AsyncMqttClient.h>
    #include <ArduinoJson.h>
    /**
     * @brief topic channel format
     */
    #define MQTT_STAT      "stat/%s/%s"             /** @brief stat topic channel format */
    #define MQTT_CMND      "cmnd/%s/%s"             /** @brief cmnd topic channel format */
    #define MQTT_TELE      "tele/%s/%s"             /** @brief tele topic channel format */
    /**
     * @brief webserver callback event mask
     */
    #define MQTTSTAT_JSON_DATA           _BV(0)      /** @brief event mask for powermgm standby */
    #define MQTTCMND_JSON_DATA           _BV(1)      /** @brief event mask for powermgm wakeup */  
    #define MQTTTELE_JSON_DATA           _BV(2)      /** @brief event mask for powermgm wakeup */  
    #define MQTTSTAT_RAW_DATA            _BV(3)      /** @brief event mask for powermgm standby */
    #define MQTTCMND_RAW_DATA            _BV(4)      /** @brief event mask for powermgm wakeup */  
    #define MQTTTELE_RAW_DATA            _BV(5)      /** @brief event mask for powermgm wakeup */  
    /**
     * @brief webserver callback data structure
     */
    typedef struct {
        DynamicJsonDocument *doc = NULL;            /** @brief pointer to the json document */
        const char *topic = "";                     /** @brief pointer to the topic */
        const char *msg = "";                       /** @brief pointer to the argument */
    } mqttData_t;
    /**
     * @brief start mqtt client task
     */
    void mqtt_client_StartTask( void );
    /**
     * @brief store all mqtt client settings
     */
    void mqtt_client_save_settings( void );
    /**
     * @brief send a mqtt message to a given topic
     * 
     * @param topic     pointer to the topic
     * @param fmt       pointer to the message
     * @param ... 
     */
    void mqtt_client_publish( char * topic, const char * fmt, ... );
    /**
     * @brief send a mqtt message via stat topic
     * 
     * @param fmt       pointer to the message
     * @param ... 
     */
    void mqtt_client_stat_publish( const char * fmt, ... );
    /**
     * @brief send a mqtt message via cmnd topic
     * 
     * @param fmt       pointer to the message
     * @param ... 
     */
    void mqtt_client_cmnd_publish( const char * fmt, ... );
    /**
     * @brief send a mqtt message via tele topic
     * 
     * @param fmt       pointer to the message
     * @param ... 
     */
    void mqtt_client_tele_publish( const char * fmt, ... );
    /**
     * @brief register a callback function for a event
     * 
     * @param event             event mask
     * @param callback_func     pointer to the callback function
     * @param id                id of the callback function
     * @return true 
     * @return false 
     */
    bool mqtt_client_register_cb( EventBits_t event, CALLBACK_FUNC callback_func, const char *id );
    /**
     * @brief mqqt client config page
     */
    static const char mqtt_config_page[] =
        "<h1>mqtt settings</h1>\n"
        "<script>set_settings_namespace(\"get_mqtt_settings\");</script>"
        "<div id=\"mqtt_settings\" class=\"hbox\">\n"
        "  <div class=\"vbox\">\n"
        "    <label>MQTT server</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='mqtt_server'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>MQTT port</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='mqtt_port'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>MQTT user</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='mqtt_username'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>MQTT password</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='mqtt_password'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>MQTT topic ( stat/%topic%/... and cmnd/%topic%/... )</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='mqtt_topic'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>MQTT update interval</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='mqtt_interval'>\n"
        "    </div>\n"
        "  </div>\n"
//        "  <div class=\"vbox\">\n"
//        "    <input type=\"checkbox\" id=\"mqtt_realtimestats\" checked><label for=\"mqtt_realtimestats\"> additional stats every second</label>\n"
//        "  </div class=\"vbox\">\n"
        "</div>\n"
        "<br>\n"
        "<br>\n"
        "<button type='button' onclick='SendSetting(\"mqtt_server\");SendSetting(\"mqtt_port\");SendSetting(\"mqtt_username\");SendSetting(\"mqtt_password\");SendSetting(\"mqtt_topic\");SendSetting(\"mqtt_interval\");SendCheckboxSetting(\"mqtt_realtimestats\");SaveSettings();get_settings();' class='button'>save</button>";

#endif // _MQTTCLIENT_H