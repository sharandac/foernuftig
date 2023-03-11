/****
 * Copyright  2020  Skurydin Alexey under the MIT License.
 * http://github.com/anakod
 ****/

#ifndef BASEJSONCONFIG_H_
#define BASEJSONCONFIG_H_

#define MAX_CONFIG_FILE_NAME_LENGTH 32

#include "ArduinoJson.h"

/**
 * @brief JSON configuration storage with bindings for variables and UI widgets
 */
class BaseJsonConfig {
public:
  BaseJsonConfig(const char* configFileName);
  /**
   * @brief Load settings from file
   */
  bool load();
  /**
   * @brief Load settings from file with a custom json size
   */
  bool load( uint32_t size );
  /**
   * @brief Save settings to file
   */
  bool save();
  /**
   * @brief Save settings to file with a custom json size
   */
  bool save( uint32_t size );
  /**
   * @brief Save settings to file with a custom json size
   */
  bool resetToDefault( void );
  /**
   * @brief print out json
   */
  void debugPrint();
  
protected:
  ////////////// Available for overloading: //////////////
  virtual bool onSave(JsonDocument& document) = 0;
  virtual bool onLoad(JsonDocument& document) = 0;
  virtual bool onDefault( void ) = 0;

  virtual size_t getJsonBufferSize() { return 8192; }
protected:
  char fileName[MAX_CONFIG_FILE_NAME_LENGTH];
  bool prettyJson = true;
};

#endif