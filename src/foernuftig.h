/**
 * @file foernuftig.h
 * @author Dirk Broßwick (dirk.brosswick@googlemail.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _FOERNUFTIG_H
    #define _FOERNUFTIG_H
    /**
     * @brief init foernuftig
     */
    void foernuftig_init( void );
    /**
     * @brief foernuftig loop function 
     */
    void foernuftig_loop( void );
    /**
     * @brief get the rpm
     * 
     * @return int
     */
    int foernuftig_get_rpm( void );
    /**
     * @brief set the led
     * 
     * @param enable 
     */
    void foernuftig_set_led( uint8_t enable );
    /**
     * @brief set the rpm
     * 
     * @param rpm 
     */
    void foernuftig_set_rpm( int rpm );
    /**
     * @brief set the fan level from 0..3
     * 
     * @param level 
     */
    void foernuftig_set_level( int level );
    /**
     * @brief store foernuftig config
     */
    void foernuftig_save_settings( void );
    /**
     * @brief  foernuftig config page
     */
    static const char foernuftig_config_page[] =
        "<h2>foernuftig hardware settings</h2>\n"
        "<script>set_settings_namespace(\"get_foernuftig_settings\");setInterval(function () {sendCMD(\"get_foernuftig_data\");}, 1000);</script>"
        "<div id=\"foernuftig_settings\" class=\"hbox\">\n"
        "  <div class=\"vbox\">\n"
        "    <label>power supply pin</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='foernuftig_supply_pin'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>clk pin</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='foernuftig_clk_pin'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>fg pin</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='foernuftig_fg_pin'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>led pin</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='foernuftig_led_pin'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>rpm factor</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='foernuftig_rpm_factor'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <h2>foernuftig fan limits</h2>\n"
        "  <div class=\"vbox\">\n"
        "    <label>min rpm</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='foernuftig_min_rpm'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>mid rpm</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='foernuftig_mid_rpm'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>max rpm</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='foernuftig_max_rpm'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <h2>foernuftig hardware monitor</h2>\n"
        "  <div class=\"vbox\">\n"
        "    <div class=\"box\">\n"
        "      <button type='button' onclick='sendCMD(\"foernuftig_set_level\\\\0\");' class=\"smallbutton\">OFF</button>"
        "      <button type='button' onclick='sendCMD(\"foernuftig_set_level\\\\1\");' class=\"smallbutton\">I</button>"
        "      <button type='button' onclick='sendCMD(\"foernuftig_set_level\\\\2\");' class=\"smallbutton\">II</button>"
        "      <button type='button' onclick='sendCMD(\"foernuftig_set_level\\\\3\");' class=\"smallbutton\">III</button>"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>current rpm</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='foernuftig_current_rpm' disabled>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>\n"
        "<br>\n"
        "<br>\n"
        "<button type='button' onclick='SendSetting(\"foernuftig_led_pin\");SendSetting(\"foernuftig_fg_pin\");SendSetting(\"foernuftig_clk_pin\");SendSetting(\"foernuftig_supply_pin\");SendSetting(\"foernuftig_rpm_factor\");SendSetting(\"foernuftig_min_rpm\");SendSetting(\"foernuftig_mid_rpm\");SendSetting(\"foernuftig_max_rpm\");SaveSettings();get_settings();' class='button'>save</button>";

#endif // _FOERNUFTIG_H