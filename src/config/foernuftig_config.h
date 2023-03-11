/****************************************************************************
 *   Tu May 22 21:23:51 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef _FOERNUFTIG_CONFIG_H
    #define _FOERNUFTIG_CONFIG_H

    #include "utils/basejsonconfig.h"

    #define     FOERNUFTIG_JSON_CONFIG_FILE    "/foernuftig.json" /** @brief defines json config file name */
    /**
     * @brief button config structure
     */
    class foernuftig_config_t : public BaseJsonConfig {
        public:
            foernuftig_config_t();
            int supply_pin = 19;
            int clk_pin = 18;
            int fg_pin = 33;
            int led_pin = 35;
            int min_rpm = 50;
            int mid_rpm = 150;
            int max_rpm = 300;
            float rpm_factor = 2.0;

        protected:
            ////////////// Available for overloading: //////////////
            virtual bool onLoad(JsonDocument& document);
            virtual bool onSave(JsonDocument& document);
            virtual bool onDefault( void );
            virtual size_t getJsonBufferSize() { return 2048; }
    };
#endif // _FOERNUFTIG_CONFIG_H
