/**
 * @file wifi_config.cpp
 * @author Dirk Broßwick (dirk.brosswick@googlemail.com)
 * @brief 
 * @version 0.1
 * @date 2023-03-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <WiFi.h>
#include "wifi_config.h"

wificlient_config_t::wificlient_config_t() : BaseJsonConfig( WIFICLIENT_JSON_CONFIG_FILE ) {
}

bool wificlient_config_t::onSave(JsonDocument& doc) {

    doc["hostname"] = hostname;
    doc["ssid"] = ssid;
    doc["password"] = password;
    doc["enable_softap"] = enable_softap;
    doc["softap_ssid"] = softap_ssid;
    doc["softap_password"] = softap_password;
    doc["timeout"] = timeout;
    doc["low_bandwidth"] = low_bandwidth;
    doc["low_power"] = low_power;
    doc["enable_led"] = enable_led;
    doc["led_pin"] = led_pin;

    return true;
}

bool wificlient_config_t::onLoad(JsonDocument& doc) {
    /*
     * make an uniqe Hostname for the SoftAp SSID
     */
    uint8_t mac[6];
    char tmp_hostname[ WIFICLIENT_MAX_TEXT_SIZE] = "";
    WiFi.macAddress( mac );
    snprintf( tmp_hostname, sizeof( tmp_hostname ), DEVICE_NAME "_%02x%02x%02x", mac[3], mac[4], mac[5] );
    
    strlcpy( hostname, doc["hostname"] | tmp_hostname, sizeof( hostname ) );
    strlcpy( ssid, doc["ssid"] | "", sizeof( ssid ) );
    strlcpy( password, doc["password"] | "", sizeof( password ) );
    enable_softap = doc["enable_softap"] | true;
    strlcpy( softap_ssid, doc["softap_ssid"] | tmp_hostname, sizeof( softap_ssid ) );
    strlcpy( softap_password, doc["softap_password"] | DEVICE_NAME, sizeof( softap_password ) );
    timeout = doc["timeout"] | 15;
    low_bandwidth = doc["low_bandwidth"] | false;
    low_power = doc["low_power"] | false;
    enable_led = doc["enable_led"] | false;
    led_pin = doc["led_pin"] | -1;
    
    return true;
}

bool wificlient_config_t::onDefault( void ) {
    /*
     * make an uniqe Hostname for the SoftAp SSID
     */
    uint8_t mac[6];
    WiFi.macAddress( mac );
    snprintf( hostname, sizeof( hostname ), DEVICE_NAME "_%02x%02x%02x", mac[3], mac[4], mac[5] );
    snprintf( softap_ssid, sizeof( softap_ssid ), DEVICE_NAME "_%02x%02x%02x", mac[3], mac[4], mac[5] );

    return true;
}
