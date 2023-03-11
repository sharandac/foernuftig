/****
 * Copyright  2020  Skurydin Alexey under the MIT License.
 * http://github.com/anakod
 ****/
#include <SPIFFS.h>
#include <FS.h>

#include "basejsonconfig.h"

BaseJsonConfig::BaseJsonConfig(const char* configFileName) {
  if (configFileName[0] == '/')
    strlcpy(fileName, configFileName, MAX_CONFIG_FILE_NAME_LENGTH);
  else
  {
    fileName[0] = '/';
    strlcpy(fileName+1, configFileName, MAX_CONFIG_FILE_NAME_LENGTH);
  }
}

bool BaseJsonConfig::load() {
    bool result = false;
    /*
     * load config if exsits
     */
    if ( SPIFFS.exists(fileName) ) {
        /*
         * open file
         */
        fs::File file = SPIFFS.open(fileName, FILE_READ);
        /*
         * check if open was success
         */
        if (!file) {
            log_e("Can't open file: %s!", fileName);
        }
        else {
            /*
             * get filesize
             */
            int filesize = file.size();
            /*
             * create json structure
             */
            DynamicJsonDocument doc( filesize*4 );
            DeserializationError error = deserializeJson( doc, file );
            /*
             * check if create json structure was successfull
             */
            if ( error || filesize == 0 ) {
                log_e("json config deserializeJson() failed: %s, file: %s", error.c_str(), fileName );
            }
            else {
                log_d("json config deserializeJson() success: %s, file: %s", error.c_str(), fileName );
                result = onLoad(doc);
            }
            doc.clear();
        }
        file.close();
    }
    /*
     * check if read from json is failed
     */
    if ( !result ) {
        log_d("reading json failed, call defaults, file: %s", fileName );
        result = onDefault();
    }

    return result;
}

bool BaseJsonConfig::load( uint32_t size ) {
    bool result = false;
    /*
     * load config if exsits
     */
    if ( SPIFFS.exists(fileName) ) {
        /*
         * open file
         */
        fs::File file = SPIFFS.open(fileName, FILE_READ);
        /*
         * check if open was success
         */
        if (!file) {
            log_e("Can't open file: %s!", fileName);
        }
        else {
            /*
             * create json structure
             */
            DynamicJsonDocument doc( size );
            DeserializationError error = deserializeJson( doc, file );
            /*
             * check if create json structure was successfull
             */
            if ( error || size == 0 ) {
                log_e("json config deserializeJson() failed: %s, file: %s", error.c_str(), fileName );
            }
            else {
                log_d("json config deserializeJson() success: %s, file: %s", error.c_str(), fileName );
                result = onLoad(doc);
            }
            doc.clear();
        }
        file.close();
    }
    /*
     * check if read from json is failed
     */
    if ( !result ) {
        log_d("reading json failed, call defaults, file: %s", fileName );
        result = onDefault();
    }

    return result;
}

bool BaseJsonConfig::save( uint32_t size ) {
    bool result = false;
    fs::File file = SPIFFS.open(fileName, FILE_WRITE );

    if (!file) {
        log_e("Can't open file: %s!", fileName);
    }
    else {
        DynamicJsonDocument doc( size );
        result = onSave(doc);

        if ( doc.overflowed() ) {
            log_e("json to large, some value are missing. use another size");
        }
        
        size_t outSize = 0;
        if (prettyJson)
            outSize = serializeJsonPretty(doc, file);
        else
            outSize = serializeJson(doc, file);

        if (result == true && outSize == 0) {
            log_e("Failed to write config file %s", fileName);
            result = false;
        }
        else {
            log_d("json config serializeJson() success: %s", fileName );
        }
        
        doc.clear();
    }
    file.close();

    return result;
}

bool BaseJsonConfig::save() {
    bool result = false;
    fs::File file = SPIFFS.open(fileName, FILE_WRITE );

    if (!file) {
        log_e("Can't open file: %s!", fileName);
    }
    else {
        auto size = getJsonBufferSize();
        DynamicJsonDocument doc( size );
        result = onSave(doc);

        if ( doc.overflowed() ) {
            log_e("json to large, some value are missing. use doc.save( uint32_t size )");
        }
        
        size_t outSize = 0;
        if (prettyJson)
            outSize = serializeJsonPretty(doc, file);
        else
            outSize = serializeJson(doc, file);

        if (result == true && outSize == 0) {
            log_e("Failed to write config file %s", fileName);
            result = false;
        }
        else {
            log_d("json config serializeJson() success: %s", fileName );            
        }
        
        doc.clear();
    }
    file.close();

    return result;
}

bool BaseJsonConfig::resetToDefault( void ) {
    bool result = false;
    /*
     * check if file exists
     */
    if ( SPIFFS.exists(fileName) ) {
        /*
         * remove file
         */
        result = SPIFFS.remove(fileName);
        /*
         * check if remove was success
         */
        if ( !result ) {
            log_e("Can't remove file: %s!", fileName);
        }
    }
    /*
     * check if file was removed
     */
    if ( result ) {
        /*
         * call default values
         */
        result = onDefault();
    }
    return result;
}

void BaseJsonConfig::debugPrint() {
    auto size = getJsonBufferSize();
    DynamicJsonDocument doc(size);
    bool result = onSave(doc);
    if ( result ) {
        serializeJsonPretty(doc, Serial);
    }
}
