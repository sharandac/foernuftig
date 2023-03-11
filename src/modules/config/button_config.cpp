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
#include "button_config.h"

button_config_t::button_config_t() : BaseJsonConfig(BUTTON_JSON_CONFIG_FILE) {
}

bool button_config_t::onSave(JsonDocument& doc) {

    doc["pin"]["button_0"] = button_0;
    doc["pin"]["button_1"] = button_1;
    doc["pin"]["button_2"] = button_2;
    doc["pin"]["button_3"] = button_3;

    return true;
}

bool button_config_t::onLoad(JsonDocument& doc) {

    button_0 = doc["pin"]["button_0"] | 21;
    button_1 = doc["pin"]["button_1"] | 22;
    button_2 = doc["pin"]["button_2"] | 32;
    button_3 = doc["pin"]["button_3"] | 25;

    return true;
}

bool button_config_t::onDefault( void ) {
    return true;
}
