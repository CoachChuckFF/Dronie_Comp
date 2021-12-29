/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Storage Manager -> blizzard_nvs_manager.c
 *
 * The NVS (Non Viotile Storage) hold all of the configurations plus some
 * 
 */

#include "blizzard_nvs_manager.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_partition.h"
#include "nvs_flash.h"

#include "blizzard_global_defines.h"

static const char *Tag = "NVS";

nvs_handle NVS_Handle;

/*---------------------------- Initializers ----------------------------------*/
/*
 * Function: init_nvs_manager
 * ----------------------------
 *   inits and starts nvs
 *
 *   returns: void - on failure it reboots
 */
uint8_t init_nvs_manager(void){
  esp_err_t retVal;
  uint8_t reset = 0;

  ESP_LOGI(Tag, "Starting NVS engines...");

  retVal = nvs_flash_init();
  if(retVal != ESP_OK)
  {
    ESP_LOGE(Tag, "NVS init failed %d", retVal);
    ESP_LOGE(Tag, "%d", ESP_ERR_NVS_NO_FREE_PAGES);
    dd_esp32();
  }

  retVal = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &NVS_Handle);
  if(retVal != ESP_OK) {
    ESP_LOGE(Tag, "NVS open failed %d", retVal);
    dd_esp32();
  }
  

  /* If reset flag is set - repopulate the nvs */
  retVal = get_nvs_config(RESET_KEY, DATA_U8, &reset);
  if(retVal == NVS_DNE_ERROR || reset == ENABLE){
    retVal = populateNVS(retVal == NVS_DNE_ERROR);
    if(retVal != SUCCESS){
      return retVal;
    }
  }


  ESP_LOGI(Tag, "NVS blastoff!");
  return SUCCESS;
}

void dd_esp32(){
  ESP_LOGE(Tag, "dd = Disk Destroy");
  nvs_flash_erase();
  esp_restart();
}

void reset_esp32(){
  uint8_t enable = ENABLE;
  ESP_LOGE(Tag, "Factory Reset...");
  assert(set_nvs_config(RESET_KEY, DATA_U8, &enable) == SUCCESS);
  esp_restart();
}

/*-------------------------- Generic Getters/Setters -------------------------*/

/*
 * Function: get_nvs_config(const char* key, uint8_t data_type, void* data)
 * ----------------------------
 *
 * This is the generic getter for all configurations
 * depending on the data_type it will return data in
 * in the void* array
 *
 * returns: NVS_PARAMETER_ERROR NVS_TYPE_ERROR read* return
 */
uint8_t get_nvs_config(const char* key, uint8_t data_type, void* data){

  if(key == NULL || data == NULL){
    ESP_LOGE(Tag, "Null parameter - get: %s %s", 
      (key == NULL) ? "Null key" : "",
      (data == NULL) ? "Null data" : "");
    return NVS_PARAMETER_ERROR;
  }

  switch(data_type){
    case DATA_U8:
      // return 0;
      return readU8(key, (uint8_t *)data);
    case DATA_U16:
      // return 0;
      return readU16(key, (uint16_t *)data);
    case DATA_U32:
      // return 0;
      return readU32(key, (uint32_t *)data);
    case DATA_STRING:
      // return 0;
      return readString(key, (char *)data);
    default:
      ESP_LOGE(Tag, "Unkown data type - get: %s:%d", key, data_type);
      return NVS_TYPE_ERROR;
  }
  //should never get here
  return NVS_ERROR;

}

/*
 * Function: set_nvs_config(const char* key, uint8_t data_type, void* data)
 * ----------------------------
 *
 * This is the generic setter for all configurations
 * depending on the data_type it will return data in
 * in the void* array
 *
 * returns: NVS_PARAMETER_ERROR NVS_TYPE_ERROR write* return
 */
uint8_t set_nvs_config(const char* key, uint8_t data_type, void* data){

  if(key == NULL || data == NULL){
    ESP_LOGE(Tag, "Null parameter - set: %s %s", 
                                            (key == NULL) ? "Null key" : "",
                                            (data == NULL) ? "Null data" : "");
    return NVS_PARAMETER_ERROR;
  }

  switch(data_type){
    case DATA_U8:
      return writeU8(key, ((uint8_t *)data)[0]);
      break;
    case DATA_U16:
      return writeU16(key, ((uint16_t *)data)[0]);
      break;
    case DATA_U32:
      return writeU32(key, ((uint32_t *)data)[0]);
    case DATA_STRING:
      return writeString(key, (char *)data);
      break;
    default:
      ESP_LOGE(Tag, "Unkown data type - set: %s:%d", key, data_type);
      return NVS_TYPE_ERROR;
  }
  //should never get here
  return NVS_ERROR;
}

/*---------------------------- Helper Functions ------------------------------*/
/*
 * Function: read*(const char* key, type* value)
 * ----------------------------
 *   Reads value of given key and returns it in the value parameter.
 *
 *   returns: SUCCESS on success, NVS_ERROR if something went wrong and
 *   NVS_DNE_ERROR, if the key is not recognized
 */

uint8_t readU8(const char* key, uint8_t* value){
  esp_err_t retVal;

  if(key == NULL){
    ESP_LOGE(Tag, "Error reading U8: null key");
    return NVS_ERROR;
  }

  if(value == NULL){
    ESP_LOGE(Tag, "Error reading U8: null value");
    return NVS_ERROR;
  }

  retVal = nvs_get_u8(NVS_Handle, key, value);
  if(retVal != ESP_OK){
      if(retVal == ESP_ERR_NVS_NOT_FOUND){
        ESP_LOGE(Tag, "Key does not exist - read u8: %s", key);
        return NVS_DNE_ERROR;
      }
    ESP_LOGE(Tag, "Error reading u8: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  return SUCCESS;
}

uint8_t readU16(const char* key, uint16_t* value){
  esp_err_t retVal;

  if(key == NULL){
    ESP_LOGE(Tag, "Error reading U16: null key");
    return NVS_ERROR;
  }

  if(value == NULL){
    ESP_LOGE(Tag, "Error reading U16: null value");
    return NVS_ERROR;
  }

  retVal = nvs_get_u16(NVS_Handle, key, value);
  if(retVal != ESP_OK){
      if(retVal == ESP_ERR_NVS_NOT_FOUND){
        ESP_LOGE(Tag, "Key does not exist - read u16: %s", key);
        return NVS_DNE_ERROR;
      }
    ESP_LOGI(Tag, "Error reading u16: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  return SUCCESS;
}

uint8_t readU32(const char* key, uint32_t* value){
  esp_err_t retVal;

  if(key == NULL){
    ESP_LOGE(Tag, "Error reading U32: null key");
    return NVS_ERROR;
  }

  if(value == NULL){
    ESP_LOGE(Tag, "Error reading U32: null value");
    return NVS_ERROR;
  }

  retVal = nvs_get_u32(NVS_Handle, key, value);
  if(retVal != ESP_OK){
      if(retVal == ESP_ERR_NVS_NOT_FOUND){
        ESP_LOGE(Tag, "Key does not exist - read u32: %s", key);
        return NVS_DNE_ERROR;
      }
    ESP_LOGI(Tag, "Error reading u32: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  return SUCCESS;
}

uint8_t readString(const char* key, char* value){
  esp_err_t retVal;
  size_t tempLength = MAX_STRING_LENGTH;

  if(key == NULL){
    ESP_LOGE(Tag, "Error reading string: null key");
    return NVS_ERROR;
  }

  if(value == NULL){
    ESP_LOGE(Tag, "Error reading string: null value");
    return NVS_ERROR;
  }

  retVal = nvs_get_str(NVS_Handle, key, value, &tempLength);
  if(retVal != ESP_OK){
      if(retVal == ESP_ERR_NVS_NOT_FOUND){
        ESP_LOGE(Tag, "Key does not exist - read string: %s", key);
        return NVS_DNE_ERROR;
      }
    ESP_LOGI(Tag, "Error getting string: %s:%s length %d", key, errorToString(retVal), tempLength);
    return NVS_ERROR;
  }

  return SUCCESS;
}

/*
 * Function: write*(const char* key, type value)
 * ----------------------------
 *   Attempts to write value to the corresponding key
 *
 *   returns: SUCCESS on success, NVS_ERROR if something went wrong
 */

uint8_t writeU8(const char* key, uint8_t value){
  esp_err_t retVal;

  if(key == NULL){
    ESP_LOGE(Tag, "Error writing u8: null key");
    return NVS_ERROR;
  }

  retVal = nvs_set_u8(NVS_Handle, key, value);
  if(retVal != ESP_OK){
    ESP_LOGI(Tag, "Error writing u8: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  retVal = nvs_commit(NVS_Handle);
  if(retVal != ESP_OK){
    ESP_LOGI(Tag, "Error commiting u8: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  ESP_LOGI(Tag, "Wrote u8 %s: %d", key, value);
  return SUCCESS;
}

uint8_t writeU16(const char* key, uint16_t value){
  esp_err_t retVal;

  if(key == NULL){
    ESP_LOGE(Tag, "Error writing u16: null key");
    return NVS_ERROR;
  }

  retVal = nvs_set_u16(NVS_Handle, key, value);
  if(retVal != ESP_OK){
    ESP_LOGI(Tag, "Error writing u16: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  retVal = nvs_commit(NVS_Handle);
  if(retVal != ESP_OK){
    ESP_LOGI(Tag, "Error commiting u16: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  ESP_LOGI(Tag, "Wrote u16 %s: %d", key, value);
  return SUCCESS;
}

uint8_t writeU32(const char* key, uint32_t value){
  esp_err_t retVal;

  if(key == NULL){
    ESP_LOGE(Tag, "Error writing u32: null key");
    return NVS_ERROR;
  }

  retVal = nvs_set_u32(NVS_Handle, key, value);
  if(retVal != ESP_OK){
    ESP_LOGI(Tag, "Error writing u32: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  retVal = nvs_commit(NVS_Handle);
  if(retVal != ESP_OK){
    ESP_LOGI(Tag, "Error commiting u32: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  ESP_LOGI(Tag, "Wrote u32 %s: %d", key, value);
  return SUCCESS;
}

uint8_t writeString(const char* key, char* value){

  esp_err_t retVal;

  if(key == NULL){
    ESP_LOGE(Tag, "Error writing string: null key");
    return NVS_ERROR;
  }

  if(value == NULL){
    ESP_LOGE(Tag, "Error writing string: null value");
    return NVS_ERROR;
  }

  //Checks for NULL termination
  retVal = checkString(value);
  if(retVal != SUCCESS) {
    return retVal;
  }

  retVal = nvs_set_str(NVS_Handle, key, (const char*) value);
  if(retVal != ESP_OK){
    ESP_LOGI(Tag, "Error writing string: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  retVal = nvs_commit(NVS_Handle);
  if(retVal != ESP_OK){
    ESP_LOGI(Tag, "Error commiting string: %s:%s", key, errorToString(retVal));
    return NVS_ERROR;
  }

  ESP_LOGI(Tag, "Wrote String %s: %s", key, (const char*) value);
  return SUCCESS;
}

/*
 * Function: uint8_t populateNVS(uint8_t should_qc)
 * ----------------------------
 *   populates the nvs with default values, if nvs was empty before
 *   set with SSID and PASS of Blizzard
 *
 *   returns: NVS errors SUCCESS
 */
uint8_t populateNVS(uint8_t should_qc){
  char* keys[] = DEFAULT_KEYS;
  uint8_t types[] = DEFAULT_TYPES;
  char* strings[] = DEFAULT_STRINGS;
  uint32_t u32s[] = DEFAULT_U32;
  uint16_t u16s[] = DEFAULT_U16;
  uint8_t u8s[] = DEFAULT_U8;

  uint8_t masterIndex, stringIndex = 0, u32Index = 0, u16Index = 0, u8Index = 0, retVal = SUCCESS;

  for(masterIndex = 0; masterIndex < sizeof(types); masterIndex++){
    switch(types[masterIndex]){
      case DATA_STRING:
        retVal = set_nvs_config(keys[masterIndex], DATA_STRING, strings[stringIndex++]);
        break;
      case DATA_U32:
        retVal = set_nvs_config(keys[masterIndex], DATA_U32, &(u32s[u32Index++]));
        break;
      case DATA_U16:
        retVal = set_nvs_config(keys[masterIndex], DATA_U16, &(u16s[u16Index++]));
        break;
      case DATA_U8:
        retVal = set_nvs_config(keys[masterIndex], DATA_U8, &(u8s[u8Index++]));
        break;
    }

    if(retVal != SUCCESS){
      ESP_LOGE(Tag, "Error populating the nvs");
      return retVal;
    }
    ESP_LOGE(Tag, "NVS Populated");
  }

  if(should_qc){
    u8Index = 0;
    assert(set_nvs_config(SSID_KEY, DATA_STRING, QC_SSID) == SUCCESS);
    assert(set_nvs_config(PASS_KEY, DATA_STRING, QC_PASS) == SUCCESS);
    assert(set_nvs_config(DEVICE_NAME_KEY, DATA_STRING, QC_NAME) == SUCCESS);
    assert(set_nvs_config(FIRMWARE_VERSION_KEY, DATA_U8, &u8Index) == SUCCESS);
    assert(set_nvs_config(FIRMWARE_SUB_VERSION_KEY, DATA_U8, &u8Index) == SUCCESS);
  } else {
    u8Index = DISABLE;
    assert(set_nvs_config(QC_KEY, DATA_U8, &u8Index) == SUCCESS);
  }

  //disable reset (Yes, I'm reusing the u8Index)
  u8Index = DISABLE;
  set_nvs_config(RESET_KEY, DATA_U8, &u8Index);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error disabling reset");
    return retVal;
  }

  return retVal;
}

/*
 * Function: checkString(char* string)
 * ----------------------------
 *   Checks to see if a string is valid
 *
 *   returns: NVS_STRING_ERROR SUCCESS
 */
uint8_t checkString(char* string){
  uint16_t i;

  if(string == NULL){
    ESP_LOGE(Tag, "Null string - check string");
    return NVS_STRING_ERROR;
  }

  for(i = 0; i < MAX_STRING_LENGTH; i++){
    if(string[i] == 0){
      return SUCCESS;
    }
  }

  ESP_LOGE(Tag, "String not null terminated or too long - check string");
  return NVS_STRING_ERROR;
}

/*
 * Function: const char* errorToString(esp_err_t retVal)
 * ----------------------------
 *   Turns esp_err_t into a string
 *
 *   returns: string version of esp_err_t
 */
const char* errorToString(esp_err_t retVal){
  switch(retVal){
    case ESP_ERR_NVS_INVALID_HANDLE:
      return "invalid handle";
    case ESP_ERR_NVS_READ_ONLY:
      return "read only";
    case ESP_ERR_NVS_INVALID_NAME:
      return "invalid name";
    case ESP_ERR_NVS_INVALID_LENGTH:
      return "invalid length";
    case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
      return "insufficient space";
    case ESP_ERR_NVS_REMOVE_FAILED:
      return "remove failed";
    case ESP_ERR_NVS_KEY_TOO_LONG:
      return "key name too long";
    default:
      return "unkown error";
  }
}
