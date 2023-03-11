/**
 * @file button.h
 * @author Dirk Broßwick ( dirk.brosswick@uni-jena.de )
 * @brief 
 * @version 0.1
 * @date 2022-01-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _BUTTON_H
    #define _BUTTON_H
    /**
     * @brief button states
     */
    enum {
        BUTTON_NONE = 0,
        BUTTON_OFF = 1,
        BUTTON_I = 2,
        BUTTON_II = 4,
        BUTTON_III = 8,
    };
    /**
     * @brief init button
     */
    void button_init( void );
    /**
     * @brief button loop function
     */
    void button_loop( void );
    /**
     * @brief inject a button event
     * 
     * @param key 
     */
    void button_inject( int key );
    /**
     * @brief get the last pressed button
     * 
     * @return int 
     */
    int button_get( void );
    /**
     * @brief store button config
     * 
     * @return const char* 
     */
    void button_save_settings( void );
    /**
     * @brief button config page
     */
    static const char button_config_page[] =
        "<h2>button hardware settings</h2>\n"
        "<script>set_settings_namespace(\"get_button_settings\");</script>\n"
        "<div id=\"button_settings\" class=\"hbox\">\n"
        "  <div class=\"vbox\">\n"
        "    <label>off pin</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='button_off_pin'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>I pin</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='button_i_pin'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>II pin</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='button_ii_pin'>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"vbox\">\n"
        "    <label>III pin</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='button_iii_pin'>\n"
        "    </div>\n"
        "  </div>\n"
        "<h2>button hardware monitor</h2>\n"
        "  <div class=\"vbox\">\n"
        "    <label>button_state</label><br>\n"
        "    <div class=\"box\">\n"
        "      <input type='text' size='32' id='button_state' disabled>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>\n"
        "<br>\n"
        "<br>\n"
        "<button type='button' onclick='SendSetting(\"button_off_pin\");SendSetting(\"button_i_pin\");SendSetting(\"button_ii_pin\");SendSetting(\"button_iii_pin\");SaveSettings();get_settings();' class='button'>save</button>";

#endif // _BUTTON_H