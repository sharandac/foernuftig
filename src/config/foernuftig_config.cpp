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
#include "foernuftig_config.h"

foernuftig_config_t::foernuftig_config_t() : BaseJsonConfig(FOERNUFTIG_JSON_CONFIG_FILE) {}

bool foernuftig_config_t::onSave(JsonDocument& doc) {

    doc["fan"]["hardware"]["supply_pin"] = supply_pin;
    doc["fan"]["hardware"]["clk_pin"] = clk_pin;
    doc["fan"]["hardware"]["fg_pin"] = fg_pin;
    doc["fan"]["hardware"]["led_pin"] = led_pin;
    doc["fan"]["min_rpm"] = min_rpm;
    doc["fan"]["mid_rpm"] = mid_rpm;
    doc["fan"]["max_rpm"] = max_rpm;
    doc["fan"]["rpm_factor"] = rpm_factor;

    return true;
}

bool foernuftig_config_t::onLoad(JsonDocument& doc) {

    supply_pin = doc["fan"]["hardware"]["supply_pin"] | 19;
    clk_pin= doc["fan"]["hardware"]["clk_pin"] | 18;
    fg_pin= doc["fan"]["hardware"]["fg_pin"] | 33;
    led_pin = doc["fan"]["hardware"]["led_pin"] | 35;
    min_rpm= doc["fan"]["min_rpm"] | 50;
    mid_rpm= doc["fan"]["mid_rpm"] | 150;
    max_rpm= doc["fan"]["max_rpm"] | 300; 
    rpm_factor= doc["fan"]["rpm_factor"] | 2.0;

    return true;
}

bool foernuftig_config_t::onDefault( void ) {
    return true;
}
