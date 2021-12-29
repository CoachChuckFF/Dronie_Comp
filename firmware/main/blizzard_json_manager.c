/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * JSON Manager -> blizzard_json_manager.c
 *
 */

#include "blizzard_json_manager.h"
#include "blizzard_nvs_manager.h"
#include "blizzard_global_defines.h"
#include "blizzard_show_manager.h"
#include "blizzard_recorder_manager.h"
#include "blizzard_protocol_manager.h"
#include "blizzard_simple_ui_manager.h"

#include <inttypes.h>
#include "cJSON.h"

static const char *Tag = "JSON";

/*
* Function: init_json_manager()
* ----------------------------
*   Clears all of the change flags
*
*   returns: SUCCESS
*/
uint8_t init_json_manager(){

    ESP_LOGI(Tag, "Json Stations!");
    return SUCCESS;
}

/*------------------- Handlers -------------------------------------------------------*/
/*
* Function: parse_json()
* ----------------------------
*   parses a json command and deligates the appropriate command
*   it then returns the return values int the ret_json param
*
*   returns: SUCCESS ERRORS
*/
uint8_t parse_json(char* json, char* ret_json){
    uint8_t action, retEnable, tempBuffer[MAX_STRING_LENGTH];
    uint8_t retVal = SUCCESS;
	cJSON *jsons[MAX_JSON_ITEMS];

    if(json == NULL){
        ESP_LOGE(Tag, "Null json - root action json");
        return ACTION_NULL_JSON_ERROR;
    }

    memset(jsons, 0, (sizeof(cJSON*) * MAX_JSON_ITEMS));

    retEnable = (ret_json != NULL);

    ESP_LOGI(Tag, "Command: %s", json);

    jsons[JSON_ROOT_INDEX] = cJSON_Parse(json);
    if(jsons[JSON_ROOT_INDEX] == NULL){
        retVal = ACTION_JSON_PARSE_ERROR;
        ESP_LOGE(Tag, "Error parsing json - action json: %d", ACTION_JSON_PARSE_ERROR);
        setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Error parsing json", ACTION_JSON_PARSE_ERROR, 0);
        goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
    }

    jsons[0] = cJSON_GetObjectItem(jsons[JSON_ROOT_INDEX], "Action");
    if(!cJSON_IsNumber(jsons[0])){
        retVal = ACTION_BAD_ACTION_ERROR;
        ESP_LOGE(Tag, "Unkown action - action json: %d", ACTION_BAD_ACTION_ERROR);
        setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Unkown action", ACTION_BAD_ACTION_ERROR, 0);
        goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
    }

    action = jsons[0]->valueint;

    jsons[JSON_TEMP_INDEX] = cJSON_CreateObject();
    if(jsons[JSON_TEMP_INDEX] == NULL){
        retVal = ACTION_JSON_CREATE_ERROR;
        ESP_LOGE(Tag, "Error creating json: action %d", action);
        setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Error creating json", ACTION_JSON_CREATE_ERROR, action);
        goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;    
    } else {
        setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "SUCCESS", SUCCESS, action);
    }

    cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Action", cJSON_CreateNumber(action));
    //cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], JSON_ODYSSEY_KEY, cJSON_CreateNumber(JSON_ODYSSEY_VALUE));

    switch(action){

        /*------------------------------------ Get General Config ----------------------------*/

        case ACTION_SET_GENERAL_CONFIG:
            jsons[1] = cJSON_GetObjectItem(jsons[JSON_ROOT_INDEX], "Key");
            if(!cJSON_IsString(jsons[1])){
                retVal = ACTION_JSON_ITEM_ERROR;
                ESP_LOGE(Tag, "Invalid key - action json - set general config");
                setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Invalid key - set general config", ACTION_JSON_ITEM_ERROR, action);
                goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
            }

            jsons[2] = cJSON_GetObjectItem(jsons[JSON_ROOT_INDEX], "Type");
            if(!cJSON_IsNumber(jsons[2])){
                retVal = ACTION_JSON_ITEM_ERROR;
                ESP_LOGE(Tag, "Invalid type - action json - set general config");
                setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Invalid type - set general config", ACTION_JSON_ITEM_ERROR, action);
                goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
            }

            jsons[3] = cJSON_GetObjectItem(jsons[JSON_ROOT_INDEX], "Data");

            if((uint8_t)jsons[2]->valueint == DATA_STRING){

                if(!cJSON_IsString(jsons[3])){
                    retVal = ACTION_JSON_ITEM_ERROR;
                    ESP_LOGE(Tag, "Invalid data string - action json - set general config");
                    setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Invalid data string - set general config", ACTION_JSON_ITEM_ERROR, action);
                    goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
                }

                retVal = set_nvs_config((const char*)jsons[1]->valuestring,
                                                    (uint8_t) jsons[2]->valueint,
                                                    (void *) jsons[3]->valuestring);


            } else {

                if(!cJSON_IsNumber(jsons[3])){
                    retVal = ACTION_JSON_ITEM_ERROR;
                    ESP_LOGE(Tag, "Invalid data number - action json - set general config");
                    setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Invalid data number - set general config", ACTION_JSON_ITEM_ERROR, action);
                    goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
                }

                retVal = set_nvs_config((const char*)jsons[1]->valuestring,
                                                    (uint8_t) jsons[2]->valueint,
                                                    (void *) &(jsons[5]->valueint));
            }

            if(retVal != SUCCESS){
                ESP_LOGE(Tag, "Error setting general config - action json: %d", retVal);
                setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Error setting general config", retVal, action);
                goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;    
            }

            cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Key", cJSON_CreateString(jsons[1]->valuestring));
            cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Type", cJSON_CreateNumber(jsons[2]->valueint));

            switch((uint8_t)jsons[2]->valueint){
                case DATA_U8:
                    cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Data", cJSON_CreateNumber(((uint8_t*)tempBuffer)[0]));
                    break;
                case DATA_U16:
                    cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Data", cJSON_CreateNumber(((uint16_t*)tempBuffer)[0]));
                    break;
                case DATA_U32:
                    cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Data", cJSON_CreateNumber(((uint32_t*)tempBuffer)[0]));
                    break;
                case DATA_STRING:
                    cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Data", cJSON_CreateString((char*)tempBuffer));
                    break;
                default:
                    retVal = NVS_TYPE_ERROR;
                    ESP_LOGE(Tag, "Invalid type - action json - get general config: type %d", (uint8_t)jsons[2]->valueint);
                    setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Invalid type - get general config", NVS_TYPE_ERROR, action);
                    goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
            }
        break;

        /*------------------------------------ Set General Config ----------------------------*/

        case ACTION_GET_GENERAL_CONFIG:

            jsons[1] = cJSON_GetObjectItem(jsons[JSON_ROOT_INDEX], "Key");
            if(!cJSON_IsString(jsons[1])){
                retVal = ACTION_JSON_ITEM_ERROR;
                ESP_LOGE(Tag, "Invalid key - action json - get general config");
                setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Invalid key - get general config", ACTION_JSON_ITEM_ERROR, action);
                goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
            }

            jsons[2] = cJSON_GetObjectItem(jsons[JSON_ROOT_INDEX], "Type");
            if(!cJSON_IsNumber(jsons[2])){
                retVal = ACTION_JSON_ITEM_ERROR;
                ESP_LOGE(Tag, "Invalid type - action json - get general config");
                setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Invalid type - get general config", ACTION_JSON_ITEM_ERROR, action);
                goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
            }

            retVal = get_nvs_config((const char*)jsons[1]->valuestring,
                                    (uint8_t)jsons[2]->valueint,
                                    (void*)tempBuffer);

            if(retVal != SUCCESS){
                ESP_LOGE(Tag, "Error getting general config - action json - get general config : %d", retVal);
                setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Error getting general config", retVal, action);
                goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;    
            }

            cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Key", cJSON_CreateString(jsons[1]->valuestring));
            cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Type", cJSON_CreateNumber(jsons[2]->valueint));

            switch((uint8_t)jsons[2]->valueint){
                case DATA_U8:
                    cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Data", cJSON_CreateNumber(((uint8_t*)tempBuffer)[0]));
                    break;
                case DATA_U16:
                    cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Data", cJSON_CreateNumber(((uint16_t*)tempBuffer)[0]));
                    break;
                case DATA_U32:
                    cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Data", cJSON_CreateNumber(((uint32_t*)tempBuffer)[0]));
                    break;
                case DATA_STRING:
                    cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Data", cJSON_CreateString((char*)tempBuffer));
                    break;
                default:
                    retVal = NVS_TYPE_ERROR;
                    ESP_LOGE(Tag, "Invalid type - action json - get general config: type %d", (uint8_t)jsons[2]->valueint);
                    setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Invalid type - get general config", NVS_TYPE_ERROR, action);
                    goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
            }
            break;

        /*------------------------------------ Start Playback ----------------------------*/

        case ACTION_START_PLAYBACK:
            play_show();
            break;

        /*------------------------------------ Pause Playback ----------------------------*/

        case ACTION_PAUSE_PLAYBACK:
            pause_show();
            break;

        /*------------------------------------ Stop Playback ----------------------------*/

        case ACTION_STOP_PLAYBACK:
            stop_show();
            break;

        /*------------------------------------ Seek ----------------------------*/

        case ACTION_SEEK:

            jsons[1] = cJSON_GetObjectItem(jsons[JSON_ROOT_INDEX], "Frame");
            if(!cJSON_IsNumber(jsons[1])){
                retVal = ACTION_JSON_ITEM_ERROR;
                ESP_LOGE(Tag, "Could not find 'Frame' in JSON seek");
                setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Could not find 'Frame' in JSON seek", ACTION_JSON_ITEM_ERROR, action);
                goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
            }

            seek_show((uint32_t) jsons[1]->valueint);
            break;


        /*------------------------------------ Get Playback Name ----------------------------*/

        case ACTION_GET_PLAYBACK_NAME:


            get_show_name((char*)tempBuffer, 0);
            tempBuffer[(MAX_STRING_LENGTH-1)] = '\0'; //sanity null terminate
            cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Name", cJSON_CreateString((char*)tempBuffer));

            break;

        /*------------------------------------ Get Current Frame ----------------------------*/

        case ACTION_GET_CURRENT_FRAME:

            cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "CurrentFrame", cJSON_CreateNumber(get_current_frame()));

            break;

        /*------------------------------------ Get Total Frames ----------------------------*/

        case ACTION_GET_TOTAL_FRAMES:

            cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "TotalFrames", cJSON_CreateNumber(get_total_frames()));

            break;

        /*------------------------------------ Get Current Timestamp ----------------------------*/

        case ACTION_GET_CURRENT_TIMESTAMP:

            cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "Timestamp", cJSON_CreateNumber(get_current_timestamp()));

            break;

        /*------------------------------------ Get Total Time ----------------------------*/

        case ACTION_GET_TOTAL_TIME:

            cJSON_AddItemToObject(jsons[JSON_TEMP_INDEX], "TotalTime", cJSON_CreateNumber(get_total_time()));

            break;

        /*------------------------------------ Set Show on Loop ----------------------------*/

        case ACTION_SET_SHOW_ON_LOOP:

            jsons[1] = cJSON_GetObjectItem(jsons[JSON_ROOT_INDEX], "Value");
            if(!cJSON_IsNumber(jsons[1])){
                retVal = ACTION_JSON_ITEM_ERROR;
                ESP_LOGE(Tag, "Could not find 'Value' in JSON set show on loop");
                setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Could not find 'Frame' in JSON seek", ACTION_JSON_ITEM_ERROR, action);
                goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
            }

            change_show_on_loop((uint32_t) jsons[1]->valueint);
            break;

        /*------------------------------------ Set Show on Start ----------------------------*/

        case ACTION_SET_SHOW_ON_START:

            jsons[1] = cJSON_GetObjectItem(jsons[JSON_ROOT_INDEX], "Value");
            if(!cJSON_IsNumber(jsons[1])){
                retVal = ACTION_JSON_ITEM_ERROR;
                ESP_LOGE(Tag, "Could not find 'Value' in JSON set show on start");
                setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Could not find 'Frame' in JSON seek", ACTION_JSON_ITEM_ERROR, action);
                goto JAIL_AND_DO_NOT_COLLECT_200_DOLLARS;
            }

            change_show_on_start((uint32_t) jsons[1]->valueint);
            break;

        /*------------------------------------ Check Show ----------------------------*/

        case ACTION_CHECK_SHOW:
            check_show();
            break;

        /*------------------------------------ Start Record ----------------------------*/

        case ACTION_START_RECORD:
            start_recording();
            break;

        /*------------------------------------ Stop Record ----------------------------*/

        case ACTION_STOP_RECORD:
            stop_recording();
            break;

        /*------------------------------------ Start Listening ----------------------------*/

        case ACTION_START_LISTENING:
            if(
                get_current_ui_state() != UI_USER_STATE_RECORDING && 
                get_current_ui_state() != UI_USER_STATE_LISTENING
            ){
                start_listening();
                set_listening_ui_state();
            }
            break;

        /*------------------------------------ Stop Listening ----------------------------*/

        case ACTION_STOP_LISTENING:
            if(get_current_ui_state() == UI_USER_STATE_LISTENING){
                stop_listening();
                set_idle_ui_state();
            }
            break;

        /*------------------------------------ Reboot ----------------------------*/

        case ACTION_REBOOT:
            esp_restart();
            break;

        /*------------------------------------ Factory Reset ----------------------------*/

        case ACTION_FACTORY_RESET:
            reset_esp32();
            break;

        /*------------------------------------ Error ----------------------------*/

        default:
            retVal = ACTION_BAD_ACTION_ERROR;
            ESP_LOGE(Tag, "Unkown action: %d json: %s", action, json);
            setJSONError(retEnable, &jsons[JSON_ERROR_INDEX], "Unkown action", retVal, action);
        
    }


JAIL_AND_DO_NOT_COLLECT_200_DOLLARS:

    if(jsons[JSON_TEMP_INDEX] != NULL){
        if(retVal == SUCCESS && retEnable){
            sprintf(ret_json, "%.512s", cJSON_Print(jsons[JSON_TEMP_INDEX]));
        }
        cJSON_Delete(jsons[JSON_TEMP_INDEX]);
    }

    if(jsons[JSON_ERROR_INDEX] != NULL){
        if(retVal != SUCCESS && retEnable){ 
            sprintf(ret_json, "%.512s", cJSON_Print(jsons[JSON_ERROR_INDEX]));
        }
        cJSON_Delete(jsons[JSON_ERROR_INDEX]);
    }

    cJSON_Delete(jsons[JSON_ROOT_INDEX]);

    return retVal;
}

/*
* Function: setJSONError()
* ----------------------------
*   adds JSON objects to the error return
*
*   returns: ERRORS SUCCESS
*/
uint8_t setJSONError(uint8_t enable, cJSON **json, const char* message, uint8_t reason, uint8_t action){
    if(enable == DISABLE){
        return SUCCESS;
    }

    if(json == NULL){
        ESP_LOGE(Tag, "Null json - set error");
        return ACTION_NULL_DATA_ERROR; 
    }

    if(message == NULL){
        ESP_LOGE(Tag, "Null message - set error");
        return ACTION_NULL_DATA_ERROR;
    }

    *json = cJSON_CreateObject();
    if(*json == NULL){
        ESP_LOGE(Tag, "Error creating json - set error");
        return ACTION_JSON_CREATE_ERROR;
    }

    cJSON_AddItemToObject(*json, "Error", cJSON_CreateString(message));
    cJSON_AddItemToObject(*json, "Ermac", cJSON_CreateNumber(reason));
    cJSON_AddItemToObject(*json, "Action", cJSON_CreateNumber(action));

    return SUCCESS;
}
