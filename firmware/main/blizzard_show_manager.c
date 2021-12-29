/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Show Manager -> blizzard_show_manager.c
 *
 */

#include "blizzard_show_manager.h"
#include "blizzard_global_defines.h"
#include "blizzard_nvs_manager.h"
#include "blizzard_helpers.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "blizzard_dmx_manager.h"
#include "blizzard_recorder_manager.h"

#include "math.h"

static const char *Tag = "SHOW MANAGER";

/*----------------- Globals --------------------------------------------------*/
uint8_t Show_State = SHOW_STATE_STOP;
uint8_t Show_Internal_State = SHOW_INTERNAL_EMPTY;
uint8_t Show_On_Start = DISABLE;
uint8_t Show_On_Loop = DISABLE;
uint8_t Show_Present = DISABLE;
uint32_t Show_Write_Pointer = 0;
uint32_t Show_Read_Pointer = 0;
uint32_t Current_Frame = ~0;
uint32_t Total_Frames = 0;
uint32_t Current_Delay = 0;
uint32_t Delay_Tick = 0;
uint32_t Current_Timestamp = 0;
uint32_t Total_Time = 0;
uint32_t Current_Pointer = 0;

BlizzardPlaybackFileHeader Show_Header;
BlizzardPlaybackFileFrame Frame_Header;


char Show_Name_Buffer[MAX_FILENAME_LENGTH];
uint8_t Diffs_Buffer[512];
uint8_t Show_DMX_Buffer[512];
const esp_partition_t* Show_Partition = NULL;

static uint8_t Head_Magic[] = BPF_HEAD_MAGIC;
static uint8_t Frame_Magic[] = BPF_FRAME_MAGIC;
static uint8_t Tail_Magic[] = BPF_TAIL_STOP_CODE;

/*----------------- Functions ------------------------------------------------*/
/*
 * Function: init_show_manager(void)
 * ----------------------------
 *  initalizes the show manager
 *
 *  returns: void
 */
uint8_t init_show_manager(void){
  uint8_t retVal;

  retVal = get_nvs_config(SHOW_ON_START_KEY, DATA_U8, &Show_On_Start);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting show on start - init show: %d", retVal);
    return retVal;
  }

  retVal = get_nvs_config(SHOW_ON_LOOP_KEY, DATA_U8, &Show_On_Loop);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting show on loop - init show: %d", retVal);
    return retVal;
  }

  Show_Partition = esp_partition_find_first(
      ESP_PARTITION_TYPE_DATA, 
      ESP_PARTITION_SUBTYPE_ANY,
      "show"
  );
  if(Show_Partition == NULL){
    ESP_LOGE(Tag, "Could not find show partition");
    return SHOW_PARTITION_ERROR;
  }


  retVal = check_show();
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error checking show - init show: %d", retVal);
    return retVal;
  }

  if(Show_On_Start == ENABLE && Show_Internal_State == SHOW_INTERNAL_OK){
    play_show();
  }

  ESP_LOGI(Tag, "Show is making cardio a little less hardio");

  return SUCCESS;
}

/*
 * Function: tick_show(void)
 * ----------------------------
 *  Main Show Loop
 *
 *  returns: void
 */
// uint8_t Test_Change_Show = 0;
void tick_show(){

  if(Show_Internal_State == SHOW_INTERNAL_OK &&
    get_recorder_state() == RECORDER_STATE_IDLE
  ){
    if(Show_State == SHOW_STATE_PLAY){
      if(--Delay_Tick == 0){
        if(Current_Frame >= Total_Frames){
          if(Show_On_Loop == ENABLE){
            seekFrame(0);
          } else {
            stop_show();
          }
          
        } else {
          nextFrame();
        }

        copy_to_dmx(Show_DMX_Buffer);
      }
    }
  } else if(Show_State == SHOW_STATE_PLAY) {
    stop_show();
  }
}



/*
 * Function: play_show(void)
 * ----------------------------
 *  Starts playback of the Show 
 *
 *  returns: void
 */
void play_show(){
  Delay_Tick = Current_Delay + 1;
  ESP_LOGE(Tag, "Play Show");
  Show_State = SHOW_STATE_PLAY;
}

/*
 * Function: pause_show(void)
 * ----------------------------
 *  Pauses the show
 *
 *  returns: void
 */
void pause_show(){
  ESP_LOGE(Tag, "Pause Show");
  Show_State = SHOW_STATE_PAUSE;
}

/*
 * Function: stop_show(void)
 * ----------------------------
 *  Stops the show
 *
 *  returns: void
 */
void stop_show(){
  ESP_LOGE(Tag, "Stop Show");
  Show_State = SHOW_STATE_STOP;
  blackout();
}

/*
 * Function: toggle_show(void)
 * ----------------------------
 *  Play/Stop
 *
 *  returns: void
 */
void toggle_show(){
  ESP_LOGE(Tag, "Toggle Show");
  switch (Show_State)
  {
  case SHOW_STATE_PLAY:
    Show_State = SHOW_STATE_STOP;
    break;
  case SHOW_STATE_PAUSE:
  case SHOW_STATE_STOP:
  default:
    Show_State = SHOW_STATE_PLAY;
    break;
  }
}

/*
 * Function: seek_show(uint32_t frame)
 * ----------------------------
 *  Seek specific frame in the show
 *  Restarts the timer and follows either play or pause
 *
 *  returns: void
 */
void seek_show(uint32_t frame){
  seekFrame(frame);
}

/*
 * Function: uint8_t get_show_state(void)
 * ----------------------------
 *  Returns the state of the show
 */
uint8_t get_show_state(){
  return Show_State;
}

/*
 * Function: uint8_t get_show_internal_state(void)
 * ----------------------------
 *  Returns the internal state of the show (how the file is doing)
 */
uint8_t get_show_internal_state(){
  return Show_Internal_State;
}

/*
 * Function: bool get_show_ok(void)
 * ----------------------------
 *  Returns if the file integrety is ok
 */
uint8_t get_show_ok(){
  return Show_Internal_State == SHOW_INTERNAL_OK;
}

/*
 * Function: uint8_t get_show_on_start(void)
 * ----------------------------
 *  Returns Show_On_Start
 *
 */
uint8_t get_show_on_start(){
  return Show_On_Start;
}

/*
 * Function: change_show_on_start(uint8_t show_on_start)
 * ----------------------------
 *  Changes show on start
 *
 *  returns: void
 */
void change_show_on_start(uint8_t show_on_start){
  switch(show_on_start){
    case ENABLE:
      show_on_start = ENABLE;
    break;
    default: //DISABLE
      show_on_start = DISABLE;
    break;
  }

  Show_On_Start = show_on_start;

  set_nvs_config(SHOW_ON_START_KEY, DATA_U8, &show_on_start);
}

/*
 * Function: uint8_t get_show_on_loop(void)
 * ----------------------------
 *  Returns Show_On_Loop
 *
 */
uint8_t get_show_on_loop(){
  return Show_On_Loop;
}

/*
 * Function: change_show_on_loop(uint8_t show_on_loop)
 * ----------------------------
 *  Returns Show_On_Loop
 *
 *  returns: void
 */
void change_show_on_loop(uint8_t show_on_loop){
  switch(show_on_loop){
    case ENABLE:
      show_on_loop= ENABLE;
    break;
    default: //DISABLE
      show_on_loop = DISABLE;
    break;
  }

  Show_On_Loop = show_on_loop;

  set_nvs_config(SHOW_ON_LOOP_KEY, DATA_U8, &show_on_loop);
}


/*
 * Function: void get_show_name(char* name)
 * ----------------------------
 *  Returns the name of the show in the provided buffer
 *
 */
void get_show_name(char* name, uint8_t cap){
  
  uint8_t len = 0;
  switch (get_show_internal_state())
  {
  case SHOW_INTERNAL_OK:
    len = strlen(Show_Name_Buffer);
    strcpy(name, Show_Name_Buffer);
    break;
  case SHOW_INTERNAL_EMPTY:
    len = strlen(BPF_EMPTY_NAME);
    strcpy(name, BPF_EMPTY_NAME);
    break;
  default:
    len = strlen(BPF_ERROR_NAME);
    strcpy(name, BPF_ERROR_NAME);
    break;
  }

  //Null terminate just in case!
  name[blz_min_u8(len, cap)] = '\0';
}


/*
 * Function: uint32_t get_current_frame(void)
 * ----------------------------
 *  Returns get_current_frame
 *
 */
uint32_t get_current_frame(){
  return Current_Frame;
}

/*
 * Function: uint32_t get_total_frames(void)
 * ----------------------------
 *  Returns get_total_frames
 *
 */
uint32_t get_total_frames(){
  return Total_Frames;
}

/*
 * Function: uint32_t get_current_timestamp(void)
 * ----------------------------
 *  Returns get_current_timestamp
 *
 */
uint32_t get_current_timestamp(){
  return Current_Timestamp + (Current_Delay - Delay_Tick);
}

/*
 * Function: uint32_t get_total_time(void)
 * ----------------------------
 *  Returns get_total_time
 *
 */
uint32_t get_total_time(){
  return Total_Time;
}

/*
 * Function: Magic Getters
 * ----------------------------
 *
 */
uint8_t *get_head_magic(){return (uint8_t *) Head_Magic;}
uint8_t *get_frame_magic(){return (uint8_t *) Frame_Magic;}
uint8_t *get_tail_magic(){return (uint8_t *) Tail_Magic;}
uint8_t get_head_magic_size(){return sizeof(Head_Magic);}
uint8_t get_frame_magic_size(){return sizeof(Frame_Magic);}
uint8_t get_tail_magic_size(){return sizeof(Tail_Magic);}
char *get_show_name_pointer(){return (char *) Show_Name_Buffer;}

/*----------------- Under the Hood -------------------------------------------*/

/*
 * Function: void check_show(void)
 * ----------------------------
 *  Inits global flags and info
 *
 */
uint8_t check_show(){
  uint8_t retVal;

  //always check for a valid show
  retVal = readHeadIntoBuffer();
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Could not read into head - Check Show");
    return retVal;
  }

  ESP_LOGI(Tag, "Name:          %s", Show_Name_Buffer);
  ESP_LOGI(Tag, "Total Frames:  %d", Show_Header._total_frames);
  ESP_LOGI(Tag, "Total Time:    %d", Show_Header._total_time);
  ESP_LOGI(Tag, "Delay:         %d", Show_Header._delay);

  setShowOK();

  return retVal;
}

/*
 * Function: void setShowOK(void)
 * ----------------------------
 *  Sets the OK Code to
 *
 */
uint8_t setShowOK(){
  uint8_t retVal, ok = SHOW_OK;

  retVal = set_nvs_config(SHOW_STATUS_KEY, DATA_U8, &ok);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting shwo error - setShowOk: %d", retVal);
    return retVal;
  }

  Show_Internal_State = SHOW_INTERNAL_OK;
  return SUCCESS;
}

/*
 * Function: void setShowError(void)
 * ----------------------------
 *  Sets the Error Code to
 *
 */
uint8_t setShowError(){
  uint8_t retVal, error = SHOW_ERROR;

  retVal = set_nvs_config(SHOW_STATUS_KEY, DATA_U8, &error);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting show error - setShowError: %d", retVal);
    return retVal;
  }

  Show_Internal_State = SHOW_INTERNAL_ERROR;
  Show_State = SHOW_STATE_STOP;
  return SUCCESS;
}

/*
 * Function: void setShowEmpty(void)
 * ----------------------------
 *  Sets the Empty Code to
 *
 */
uint8_t setShowEmpty(){
  uint8_t retVal, empty = SHOW_EMPTY;

  retVal = set_nvs_config(SHOW_STATUS_KEY, DATA_U8, &empty);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting shwo empty - setShowEmpty: %d", retVal);
    return retVal;
  }

  Show_Internal_State = SHOW_INTERNAL_EMPTY;
  Show_State = SHOW_STATE_STOP;
  return SUCCESS;
}

/*
 * Function: void nextFrame(void)
 * ----------------------------
 *  Loads the next frame in the DMX Buffer and increments the pointer,
 *  on error it will call set error. If the next frame is the tail, then
 *  it does nothing.
 *
 */
uint8_t nextFrame(){
  uint32_t offset = sizeof(BlizzardPlaybackFileFrame);
  uint16_t diffCount = Frame_Header._diff_count;

  if(Current_Frame >= Total_Frames){ //Tail
    return SUCCESS; //nothing to do here
  } else if(Current_Frame == 0 || Current_Pointer == 0){ //Header
    Current_Frame = 0;
    Current_Pointer = 0;
    offset = sizeof(BlizzardPlaybackFileHeader);
  } else { //Frame
    if(diffCount == BPF_FULL_FRAME){
      offset += sizeof(BlizzardPlaybackFileFrameFull);
    } else {
      offset += sizeof(BlizzardPlaybackFileFrameDiff) * diffCount;
    }
  }

  if(readFrameIntoBuffer(Current_Pointer + offset) == SUCCESS){
    Current_Pointer += offset;
    Current_Frame++;
    Current_Timestamp += Current_Delay;
    return SUCCESS;
  } else {
    ESP_LOGE(Tag, "Could not read frame into buffer");
    return SHOW_BAD_SEEK_ERROR;
  }
}

/*
 * Function: void prevFrame(void)
 * ----------------------------
 *  Loads the prev frame in the DMX Buffer and decrements the pointer,
 *  on error it will call set error. The the prev frame is the header then
 *  it will do nothing.
 *
 */
uint8_t prevFrame(){
  uint32_t pointer;
  uint16_t i = 0;
  uint8_t retVal = 0, foundMagic = FALSE;

  if(Current_Frame == 0 || Current_Pointer == 0){ //IS Header
    Current_Frame = 0;
    Current_Pointer = 0;
    return SUCCESS; //nothing to do here
  } else if(Current_Frame == 1){ //Prev is header
    pointer = 0;
  } else {
    for(i = 0; i < BPF_MAX_FRAME_SIZE; i++){
      pointer = Current_Pointer - i;

      if(pointer == 0){
        foundMagic = TRUE;
        break;
      }

      //dumb and slow
      retVal = readShow(pointer, Diffs_Buffer, sizeof(Frame_Magic));
      if(retVal != SUCCESS){
        ESP_LOGE(Tag, "Could not read test buffer - prev frame");
        setShowError();
        return SHOW_BAD_SEEK_ERROR;
      }

      if(Diffs_Buffer[0] == Frame_Magic[0]){
        //We use this instead of check bytes because on fail, check bytes will 
        //raise the show error flag
        if(memcmp(Diffs_Buffer, Frame_Magic, sizeof(Frame_Magic)) == SUCCESS){
          foundMagic = TRUE;
          break;
        }
      }
    }


    if(!foundMagic){
      ESP_LOGE(Tag, "Exhasted fram search - prev frame");
      setShowError();
      return SHOW_BAD_SEEK_ERROR;
    }
  }

  //read into buffer
  if(pointer == 0){
    retVal = readHeadIntoBuffer();
    if(retVal != SUCCESS){
      ESP_LOGE(Tag, "Could not find head magic - prev frame");
      return SHOW_BAD_SEEK_ERROR;
    }
  } else {
    if(readFrameIntoBuffer(pointer) == SUCCESS){
      Current_Pointer = pointer;
      Current_Frame--;
      Current_Timestamp -= Current_Delay;
    } else {
      ESP_LOGE(Tag, "Could not find frame magic - prev frame");
      return SHOW_BAD_SEEK_ERROR;
    }
  }

  return SUCCESS;
}

/*
 * Function: void seekFrame(uint32_t frame)
 * ----------------------------
 *  Seeks the frame specified by iterating through next or prev until desired
 *  frame is reached.
 *
 */
void seekFrame(uint32_t frame){
  uint32_t i = 0, currentFrame = Current_Frame;

  // ------------- Error Checking ----------------------
  if(frame >= Total_Frames){
    ESP_LOGE(Tag, "Frame out of bounds - seek frame");
    return;
  }

  if(frame == Current_Frame){
    ESP_LOGE(Tag, "Requested frame = current frame - seek frame");
    return;
  }

  // ------------- First Frame Seek ----------------------
  if(frame == 0){
    if(readHeadIntoBuffer() != SUCCESS){
      ESP_LOGE(Tag, "Error getting header - seek");
    }
    return;
  }

  // ------------- Keep Reading in frames until we get there ----------------------
  for(i = 0; i < abs((int)frame - (int)currentFrame); i++){
    if(frame > Current_Frame){
      if(nextFrame() != SUCCESS){
        ESP_LOGE(Tag, "Error seeking forward");
        return;
      }
    } else {
      if(prevFrame() != SUCCESS){
        ESP_LOGE(Tag, "Error seeking backwards");
        return;
      }
    }
  }
}

uint8_t readHeadIntoBuffer(void){
  uint8_t retVal = SUCCESS;
  
  retVal = readShow(0, (uint8_t *) &Show_Header, sizeof(BlizzardPlaybackFileHeader));
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Bad read - readHeadIntoBuffer");
    return SHOW_ERROR;
  }

  retVal = checkBytes((uint8_t *) &Show_Header, Head_Magic, sizeof(Head_Magic));
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Bad magic - readHeadIntoBuffer");
    return SHOW_BAD_MAGIC_ERROR;
  }

  Total_Frames = Show_Header._total_frames;
  Total_Time = Show_Header._total_time;
  Current_Delay = Delay_Tick = Show_Header._delay + 1;

  memcpy(Show_Name_Buffer, Show_Header._name, sizeof(Show_Name_Buffer));

  Current_Frame = 0;
  Current_Pointer = 0;
  Current_Timestamp = 0;

  memcpy(Show_DMX_Buffer, Show_Header._starting_dmx, 512);

  return SUCCESS;
}

uint8_t readFrameIntoBuffer(uint32_t pointer){
  uint8_t retVal;
  uint16_t i = 0;

  retVal = readShow(pointer, (uint8_t *) &Frame_Header, sizeof(BlizzardPlaybackFileFrame));
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Could not read frame head - readFrameIntoBuffer");
    return SHOW_ERROR;
  }

  retVal = checkBytes((uint8_t *) &Frame_Header, Frame_Magic, sizeof(Frame_Magic));
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Could not read frame head - readFrameIntoBuffer");
    return SHOW_BAD_MAGIC_ERROR;
  }

  Delay_Tick = Frame_Header._delay;
  Current_Delay = Frame_Header._delay;

  if(Frame_Header._diff_count == BPF_FULL_FRAME){
    retVal = readShow(pointer + sizeof(BlizzardPlaybackFileFrame), Show_DMX_Buffer, sizeof(BlizzardPlaybackFileFrameFull));
    if(retVal != SUCCESS){
      ESP_LOGE(Tag, "Could not read full frame - readFrameIntoBuffer");
      return SHOW_ERROR;
    }
  } else {

    //This is to make sure, even if there are more diffs, we read on!
    if(Frame_Header._diff_count > BPF_MAX_DIFFS){
      ESP_LOGE(Tag, "Too many diffs - readFrameIntoBuffer");
      return SHOW_ERROR;
    }


    retVal = readShow(pointer + sizeof(BlizzardPlaybackFileFrame), Diffs_Buffer, sizeof(BlizzardPlaybackFileFrameDiff) * (Frame_Header._diff_count));
    if(retVal != SUCCESS){
      ESP_LOGE(Tag, "Could not read diff frames - readFrameIntoBuffer");
      return SHOW_ERROR;
    }

    for(i = 0; i < Frame_Header._diff_count; i++){
      Show_DMX_Buffer[((BlizzardPlaybackFileFrameDiff *) Diffs_Buffer)[i]._address-1] = (uint8_t)(((BlizzardPlaybackFileFrameDiff *) Diffs_Buffer)[i]._value & 0x00FF);
    }
  }

  return SUCCESS;
}

/*
 * Function: uint8_t checkBytes(uint8_t* buffer, uint8_t* check, uint8_t length)
 * ----------------------------
 *  Checks matching bytes in flash given a buffer to check against. Starts at offset
 *
 */
uint8_t checkBytes(uint8_t* buffer, uint8_t* check, uint8_t length){
  uint8_t retVal;

  if(buffer == NULL || check == NULL){
    ESP_LOGE(Tag, "Null Buffer in check bytes");
    setShowError();
    return FALSE;
  }

  retVal = memcmp(buffer, check, length);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Bad check bytes wanted");
    setShowError();
  }

  return retVal;
}

/*
 * Function: readShow(uint32_t offset, uint8_t* buffer, uint32_t length)
 * ----------------------------
 *  Read Flash at offset bytes until length into buffer
 *
 *  returns: SUCCESS or SHOW ERROR
 */
uint8_t readShow(uint32_t offset, uint8_t* buffer, uint32_t length){
  if(buffer == NULL){
    ESP_LOGE(Tag, "Null buffer in reading show memory");
    setShowError();
    return SHOW_READ_ERROR;
  }

  // //SD Card Code
  // if(read_sd(offset, buffer, length) != SUCCESS){
  //   ESP_LOGE(Tag, "SD card read fail");
  //   setShowError();
  //   return SHOW_READ_ERROR;
  // }

  // Internal Storage Code
  if(esp_partition_read(Show_Partition, offset, buffer, length) != ESP_OK){
    ESP_LOGE(Tag, "Error reading to show memory");
    return SHOW_READ_ERROR;
  }

  return SUCCESS;
}

#define DUMP_SHOW_STEPS 16
void dumpShow(uint32_t start, uint8_t reps){
  uint32_t i = 0;
  uint8_t j = 0;
  uint8_t buffer[DUMP_SHOW_STEPS];

  printf("--------- Show Dump --------\n");

  for(i = start; i < (reps * DUMP_SHOW_STEPS); i+=DUMP_SHOW_STEPS){
    readShow(i, buffer, DUMP_SHOW_STEPS);
    for(j = 0; j < DUMP_SHOW_STEPS; j++){
      printf("%02X ", buffer[j]);
    }
    printf("... ");
    for(j = 0; j < DUMP_SHOW_STEPS; j++){
      printf("%c ", buffer[j]);
    }
    printf("\n");
  }
}














// /*----------------- Functions ------------------------------------------------*/
// /*
//  * Function: init_show_manager(void)
//  * ----------------------------
//  *  initalizes the show manager
//  *
//  *  returns: void
//  */
// uint8_t init_show_manager(void){
//   uint8_t retVal;

//   Show_Partition = esp_partition_find_first(
//     ESP_PARTITION_TYPE_DATA, 
//     ESP_PARTITION_SUBTYPE_ANY,
//     "show"
//   );

//   if(Show_Partition == NULL){
//     ESP_LOGE(Tag, "Could not find show partition");
//     return SHOW_PARTITION_ERROR;
//   }

//   //Get user configs
//   retVal = get_nvs_config(SHOW_ON_START_KEY, DATA_U8, &Show_On_Start);
//   if(retVal != SUCCESS){
//     ESP_LOGE(Tag, "Error getting show on start - init show: %d", retVal);
//     return retVal;
//   }

//   retVal = get_nvs_config(SHOW_ON_LOOP_KEY, DATA_U8, &Show_On_Loop);
//   if(retVal != SUCCESS){
//     ESP_LOGE(Tag, "Error getting show on loop - init show: %d", retVal);
//     return retVal;
//   }

//   //Init Globals
//   retVal = check_show();
//   if(retVal != SUCCESS){
//     ESP_LOGE(Tag, "Error checking show - init show: %d", retVal);
//     return retVal;
//   }

//   ESP_LOGI(Tag, "Show is making cardio a little less hardio");

//   if(Show_On_Start == ENABLE){
//     Show_State = SHOW_STATE_PLAY;
//   }

//   return SUCCESS;
// }

// /*
//  * Function: tick_show(void)
//  * ----------------------------
//  *  Main Show Loop
//  *
//  *  returns: void
//  */
// void tick_show(){

//   if(Show_Internal_State == SHOW_INTERNAL_OK &&
//     get_recorder_state() == RECORDER_STATE_IDLE
//   ){
//     if(Show_State == SHOW_STATE_PLAY){
//       if(--Delay_Tick == 0){
//         if(Current_Frame == Total_Frames - 1){
//           if(Show_On_Loop == ENABLE){
//             seekFrame(0);
//           } else {
//             stop_show();
//           }
//         } else {
//           nextFrame();
//         }
//         copy_to_dmx(Show_DMX_Buffer);
//       }
//     }
//   } else {
//     stop_show();
//   }
// }

// /*
//  * Function: play_show(void)
//  * ----------------------------
//  *  Starts playback of the Show 
//  *
//  *  returns: void
//  */
// void play_show(){
//   ESP_LOGE(Tag, "Play Show");
    
//   if(Show_Internal_State == SHOW_INTERNAL_OK){
//     Show_State = SHOW_STATE_PLAY;
//   }
// }

// /*
//  * Function: pause_show(void)
//  * ----------------------------
//  *  Pauses the show
//  *
//  *  returns: void
//  */
// void pause_show(){
//   ESP_LOGE(Tag, "Pause Show");

//   if(Show_Internal_State == SHOW_INTERNAL_OK){
//     Show_State = SHOW_STATE_PAUSE;
//   } else {
//     stop_show();
//   }
// }

// /*
//  * Function: stop_show(void)
//  * ----------------------------
//  *  Stops the show
//  *
//  *  returns: void
//  */
// void stop_show(){
//   if(Show_Internal_State == SHOW_INTERNAL_OK &&
//     get_recorder_state() == RECORDER_STATE_IDLE
//   ){
//     seekFrame(0);
//     blackout();
//   }

//   Show_State = SHOW_STATE_STOP;
// }

// /*
//  * Function: toggle_show(void)
//  * ----------------------------
//  *  Play/Stop
//  *
//  *  returns: void
//  */
// void toggle_show(){

//   switch (Show_State)
//   {
//   case SHOW_STATE_PLAY:
//     Show_State = SHOW_STATE_STOP;
//     break;
//   case SHOW_STATE_PAUSE:
//   case SHOW_STATE_STOP:
//   default:
//     if(get_show_ok()){
//       Show_State = SHOW_STATE_PLAY;
//     }
//     break;
//   }
// }

// /*
//  * Function: seek_show(uint32_t frame)
//  * ----------------------------
//  *  Seek specific frame in the show
//  *  Restarts the timer and follows either play or pause
//  *
//  *  returns: void
//  */
// void seek_show(uint32_t frame){
//   if(Show_Internal_State == SHOW_INTERNAL_OK){
//     seekFrame(frame);
//   }
// }

// /*
//  * Function: uint8_t get_show_state(void)
//  * ----------------------------
//  *  Returns the state of the show
//  */
// uint8_t get_show_state(){
//   return Show_State;
// }

// /*
//  * Function: uint8_t get_show_internal_state(void)
//  * ----------------------------
//  *  Returns the internal state of the show (how the file is doing)
//  */
// uint8_t get_show_internal_state(){
//   return Show_Internal_State;
// }

// /*
//  * Function: bool get_show_ok(void)
//  * ----------------------------
//  *  Returns if the file integrety is ok
//  */
// uint8_t get_show_ok(){
//   return Show_Internal_State == SHOW_INTERNAL_OK;
// }

// /*
//  * Function: uint8_t get_show_on_start(void)
//  * ----------------------------
//  *  Returns Show_On_Start
//  *
//  */
// uint8_t get_show_on_start(){
//   return Show_On_Start;
// }

// /*
//  * Function: change_show_on_start(uint8_t show_on_start)
//  * ----------------------------
//  *  Changes show on start
//  *
//  *  returns: void
//  */
// void change_show_on_start(uint8_t show_on_start){
//   switch(show_on_start){
//     case ENABLE:
//       show_on_start = ENABLE;
//     break;
//     default: //DISABLE
//       show_on_start = DISABLE;
//     break;
//   }

//   Show_On_Start = show_on_start;

//   set_nvs_config(SHOW_ON_START_KEY, DATA_U8, &show_on_start);
// }

// /*
//  * Function: uint8_t get_show_on_loop(void)
//  * ----------------------------
//  *  Returns Show_On_Loop
//  *
//  */
// uint8_t get_show_on_loop(){
//   return Show_On_Loop;
// }

// /*
//  * Function: change_show_on_loop(uint8_t show_on_loop)
//  * ----------------------------
//  *  Returns Show_On_Loop
//  *
//  *  returns: void
//  */
// void change_show_on_loop(uint8_t show_on_loop){
//   switch(show_on_loop){
//     case ENABLE:
//       show_on_loop= ENABLE;
//     break;
//     default: //DISABLE
//       show_on_loop = DISABLE;
//     break;
//   }

//   Show_On_Loop = show_on_loop;

//   set_nvs_config(SHOW_ON_LOOP_KEY, DATA_U8, &show_on_loop);
// }


// /*
//  * Function: void get_show_name(char* name)
//  * ----------------------------
//  *  Returns the name of the show in the provided buffer
//  *
//  */
// void get_show_name(char* name, uint8_t cap){
//   if(name == NULL){
//     ESP_LOGE(Tag, "Null Name Buffer - get_show_name");
//     return;
//   }

//   if(cap == 0){
//     cap = BPF_NAME_SIZE;
//   }

//   switch (get_show_internal_state())
//   {
//   case SHOW_INTERNAL_OK:
//     readShow(BPF_NAME_OFFSET, (uint8_t*) name, BPF_NAME_SIZE);
//     break;
//   case SHOW_INTERNAL_EMPTY:
//     strcpy(name, BPF_EMPTY_NAME);
//     break;
//   default:
//     strcpy(name, BPF_ERROR_NAME);
//     break;
//   }

//   //Null terminate just in case!
//   name[cap-1] = '\0';

// }


// /*
//  * Function: uint32_t get_current_frame(void)
//  * ----------------------------
//  *  Returns get_current_frame
//  *
//  */
// uint32_t get_current_frame(){
//   return Current_Frame;
// }

// /*
//  * Function: uint32_t get_total_frames(void)
//  * ----------------------------
//  *  Returns get_total_frames
//  *
//  */
// uint32_t get_total_frames(){
//   return Total_Frames;
// }

// /*
//  * Function: uint32_t get_current_timestamp(void)
//  * ----------------------------
//  *  Returns get_current_timestamp
//  *
//  */
// uint32_t get_current_timestamp(){
//   return Current_Timestamp + (Current_Delay - Delay_Tick);
// }

// /*
//  * Function: uint32_t get_total_time(void)
//  * ----------------------------
//  *  Returns get_total_time
//  *
//  */
// uint32_t get_total_time(){
//   return Total_Time;
// }

// /*----------------- Under the Hood -------------------------------------------*/

// /*
//  * Function: void check_show(void)
//  * ----------------------------
//  *  Inits global flags and info
//  *
//  */
// uint8_t check_show(){
//   uint8_t retVal, status;

//   retVal = get_nvs_config(SHOW_STATUS_KEY, DATA_U8, &status);
//   if(retVal != SUCCESS){
//     ESP_LOGE(Tag, "Cannont get show status - Check Show");
//     return retVal;
//   }

//   //keeps the current status
//   switch (status)
//   {
//   case SHOW_ERROR:
//     Show_Internal_State = SHOW_INTERNAL_ERROR;
//     break;
//   case SHOW_EMPTY: //checks to see if a wild show has appeared
//     Show_Internal_State = SHOW_INTERNAL_EMPTY;
//     break;
//   }

//   //always check for a valid show
//   if(checkBytes(0, Head_Magic, sizeof(Head_Magic))){
//     setShowOK();
//     readHeadIntoBuffer();
//   }

//   if(Show_Internal_State == SHOW_INTERNAL_OK){
//     char name[64];
//     get_show_name(name, 0);
//     name[63] = '\0';

//     ESP_LOGI(Tag, "Name:         %s", name);
//     ESP_LOGI(Tag, "Total Frames: %d", Total_Frames);
//     ESP_LOGI(Tag, "Total Time:   %d", Total_Time);
//     ESP_LOGI(Tag, "Delay:        %d", Current_Delay);
//   }

//   return retVal;
// }

// /*
//  * Function: void setShowOK(void)
//  * ----------------------------
//  *  Sets the OK Code to
//  *
//  */
// uint8_t setShowOK(){
//   uint8_t retVal, ok = SHOW_OK;

//   retVal = set_nvs_config(SHOW_STATUS_KEY, DATA_U8, &ok);
//   if(retVal != SUCCESS){
//     ESP_LOGE(Tag, "Error setting shwo error - setShowOk: %d", retVal);
//     return retVal;
//   }

//   Show_Internal_State = SHOW_INTERNAL_OK;
//   return SUCCESS;
// }

// /*
//  * Function: void setShowError(void)
//  * ----------------------------
//  *  Sets the Error Code to
//  *
//  */
// uint8_t setShowError(){
//   uint8_t retVal, error = SHOW_ERROR;

//   retVal = set_nvs_config(SHOW_STATUS_KEY, DATA_U8, &error);
//   if(retVal != SUCCESS){
//     ESP_LOGE(Tag, "Error setting shwo error - setShowError: %d", retVal);
//     return retVal;
//   }

//   Show_Internal_State = SHOW_INTERNAL_ERROR;
//   Show_State = SHOW_STATE_STOP;
//   return SUCCESS;
// }

// /*
//  * Function: void setShowEmpty(void)
//  * ----------------------------
//  *  Sets the Empty Code to
//  *
//  */
// uint8_t setShowEmpty(){
//   uint8_t retVal, empty = SHOW_EMPTY;

//   retVal = set_nvs_config(SHOW_STATUS_KEY, DATA_U8, &empty);
//   if(retVal != SUCCESS){
//     ESP_LOGE(Tag, "Error setting shwo empty - setShowEmpty: %d", retVal);
//     return retVal;
//   }

//   Show_Internal_State = SHOW_INTERNAL_EMPTY;
//   Show_State = SHOW_STATE_STOP;
//   return SUCCESS;
// }

// /*
//  * Function: void nextFrame(void)
//  * ----------------------------
//  *  Loads the next frame in the DMX Buffer and increments the pointer,
//  *  on error it will call set error. If the next frame is the tail, then
//  *  it does nothing.
//  *
//  */
// uint8_t nextFrame(){
//   BlizzardPlaybackFileFrame frame;
//   uint32_t offset = sizeof(BlizzardPlaybackFileFrame);
//   uint16_t diff_overflow = 0;

//   if(Current_Frame >= Total_Frames - 1){ //Tail
//     return SUCCESS; //nothing to do here
//   } else if(Current_Frame == 0 || Current_Pointer == 0){ //Header
//     offset = sizeof(BlizzardPlaybackFileHeader);
//   } else { //Frame
//     readShow(Current_Pointer, (uint8_t *) &frame, sizeof(BlizzardPlaybackFileFrame));
//     if(frame._diff_count == BPF_FULL_FRAME){
//       offset += sizeof(BlizzardPlaybackFileFrameFull);
//     } else {
//       offset += sizeof(BlizzardPlaybackFileFrameDiff) * frame._diff_count;
//     }
//   }

//   if(readFrameIntoBuffer(Current_Pointer + offset, &diff_overflow, ENABLE) == SUCCESS){
//     Current_Pointer += offset + (diff_overflow * sizeof(BlizzardPlaybackFileFrameDiff));
//     Current_Frame++;
//     Current_Timestamp += Current_Delay;
//     return SUCCESS;
//   } else {
//     ESP_LOGE(Tag, "Could not fine frame magic - next frame");
//     setShowError();
//     return SHOW_BAD_SEEK_ERROR;
//   }
// }

// /*
//  * Function: void prevFrame(void)
//  * ----------------------------
//  *  Loads the prev frame in the DMX Buffer and decrements the pointer,
//  *  on error it will call set error. The the prev frame is the header then
//  *  it will do nothing.
//  *
//  */
// uint8_t prevFrame(){
//   uint32_t pointer;
//   uint16_t i = 0;
//   uint8_t retVal = 0, test[1];

//   if(Current_Frame == 0 || Current_Pointer == 0){ //IS Header
//     return SUCCESS; //nothing to do here
//   } else if(Current_Frame == 1){ //Prev is header
//     if(!checkBytes(BPF_MAGIC_OFFSET, Head_Magic, sizeof(Head_Magic))){
//       ESP_LOGE(Tag, "Could not find head magic - prev frame");
//       setShowError();
//       return SHOW_BAD_SEEK_ERROR;
//     } else {
//       pointer = 0;
//     }
//   } else if(Current_Frame == 2){ //Prev is 1+ header
//     pointer = sizeof(BlizzardPlaybackFileHeader);

//     if(!checkBytes(pointer, Frame_Magic, sizeof(Frame_Magic))){
//       ESP_LOGE(Tag, "Could not find frame magic f=2 - prev frame");
//       setShowError();
//       return SHOW_BAD_SEEK_ERROR;
//     }
//   } else {

//     for(i = 1; i < BPF_MAX_FRAME_SIZE; i++){
//       pointer = Current_Pointer - i;

//       if(pointer == 0){
//         pointer = 0;
//         goto FOUND_FRAM;
//       }

//       //dumb and slow
//       readShow(pointer, test, 1);
//       if(test[0] == Frame_Magic[0]){
//         if(checkBytes(pointer, Frame_Magic, sizeof(Frame_Magic))){
//           goto FOUND_FRAM;
//         }
//       }
//     }

//     ESP_LOGE(Tag, "Exhasted fram search - prev frame");
//     setShowError();
//     return SHOW_BAD_SEEK_ERROR;
//     //did not find anything
//   }

// FOUND_FRAM:

//   //read into buffer
//   if(pointer == 0){
//     retVal = readHeadIntoBuffer();
//     if(retVal != SUCCESS){
//       ESP_LOGE(Tag, "Could not find head magic - prev frame");
//       setShowError();
//       return SHOW_BAD_SEEK_ERROR;
//     }
//   } else {
//     retVal = readFrameIntoBuffer(pointer, NULL, DISABLE);

//     if(retVal == SUCCESS){
//       Current_Pointer = pointer;
//       Current_Frame--;
//       Current_Timestamp -= Current_Delay;
//     } else {
//       ESP_LOGE(Tag, "Could not find frame magic - prev frame");
//       setShowError();
//       return SHOW_BAD_SEEK_ERROR;
//     }
//   }

//   return SUCCESS;
// }

// /*
//  * Function: void seekFrame(uint32_t frame)
//  * ----------------------------
//  *  Seeks the frame specified by iterating through next or prev until desired
//  *  frame is reached.
//  *
//  */
// void seekFrame(uint32_t frame){
//   uint32_t i = 0, currentFrame = Current_Frame;

//   if(frame >= Total_Frames){
//     ESP_LOGE(Tag, "Frame out of bounds - seek frame");
//     return;
//   }

//   if(frame == Current_Frame){
//     ESP_LOGE(Tag, "Requested frame = current frame - seek frame");
//     return;
//   }

//   if(frame == 0){
//     if(readHeadIntoBuffer() != SUCCESS){
//       ESP_LOGE(Tag, "Error getting header - seek");
//       setShowError();
//     }
//     return;
//   }

//   for(i = 0; i < abs((int)frame - (int)currentFrame); i++){
//     if(frame > Current_Frame){
//       if(nextFrame() != SUCCESS){
//         ESP_LOGE(Tag, "Error seeking forward");
//         return;
//       }
//     } else {
//       if(Current_Frame == 0){
//         seekFrame(0);
//         break;
//       } else if(prevFrame() != SUCCESS){
//         ESP_LOGE(Tag, "Error seeking backwards");
//         return;
//       }
//     }
//   }
// }

// uint8_t readHeadIntoBuffer(void){
//   BlizzardPlaybackFileHeader header;
  
//   if(!checkBytes(0, Head_Magic, sizeof(Head_Magic))){
//     ESP_LOGE(Tag, "Could not find head magic - readHeadIntoBuffer");
//     return SHOW_BAD_MAGIC_ERROR;
//   }
  
//   readShow(0, (uint8_t *) &header, sizeof(BlizzardPlaybackFileHeader));

//   Total_Frames = header._total_frames;
//   Total_Time = header._total_time;
//   Current_Delay = Delay_Tick = header._delay;

//   Current_Frame = 0;
//   Current_Pointer = 0;
//   Current_Timestamp = 0;

//   memcpy(Show_DMX_Buffer, header._starting_dmx, 512);

//   return SUCCESS;
// }

// uint8_t readFrameIntoBuffer(uint32_t pointer, uint16_t *diff_overflow_ret, uint8_t needsCheck){
//   BlizzardPlaybackFileFrame frame;
//   uint16_t i = 0, diff_overflow = 0;
  
//   if(needsCheck == ENABLE){
//     if(!checkBytes(pointer, Frame_Magic, sizeof(Frame_Magic))){
//       ESP_LOGE(Tag, "Could not find frame magic - readFrameIntoBuffer");
//       return SHOW_BAD_MAGIC_ERROR;
//     }
//   }

//   readShow(pointer, (uint8_t *) &frame, sizeof(BlizzardPlaybackFileFrame));

//   Delay_Tick = frame._delay;
//   Current_Delay = frame._delay;

//   if(frame._diff_count == BPF_FULL_FRAME){
//     readShow(pointer + sizeof(BlizzardPlaybackFileFrame), Show_DMX_Buffer, sizeof(BlizzardPlaybackFileFrameFull));
//   } else {

//     if(frame._diff_count > BPF_MAX_DIFFS){
//       diff_overflow = frame._diff_count - BPF_MAX_DIFFS; //these will be ignored
//     }

//     readShow(pointer + sizeof(BlizzardPlaybackFileFrame), Diffs_Buffer, sizeof(BlizzardPlaybackFileFrameDiff) * (frame._diff_count - diff_overflow));

//     for(i = 0; i < (frame._diff_count - diff_overflow); i++){
//       Show_DMX_Buffer[((BlizzardPlaybackFileFrameDiff *) Diffs_Buffer)[i]._address-1] = (uint8_t)(((BlizzardPlaybackFileFrameDiff *) Diffs_Buffer)[i]._value & 0x00FF);
//     }
//   }

//   if(diff_overflow_ret != NULL){
//     diff_overflow_ret[0] = diff_overflow;
//   }

//   return SUCCESS;
// }

// /*
//  * Function: uint8_t checkBytes(uint32_t offset, uint8_t* buffer, uint8_t length)
//  * ----------------------------
//  *  Checks matching bytes in flash given a buffer to check against. Starts at offset
//  *
//  */
// uint8_t checkBytes(uint32_t offset, uint8_t* buffer, uint8_t length){

//   if(buffer == NULL){
//     ESP_LOGE(Tag, "Null Buffer in check bytes");
//     return FALSE;
//   }


//   readShow(offset, Diffs_Buffer, length);

//   return memcmp(Diffs_Buffer, buffer, length) == 0;
// }

// /*
//  * Function: readShow(uint32_t offest, uint8_t* buffer, uint32_t length)
//  * ----------------------------
//  *  Read Flash at offset bytes until length into buffer
//  *
//  *  returns: SUCCESS or SHOW ERROR
//  */
// uint8_t readShow(uint32_t offest, uint8_t* buffer, uint32_t length){
//   esp_err_t retVal;

//   if(buffer == NULL){
//     ESP_LOGE(Tag, "Null buffer in reading show memory");
//     return SHOW_READ_ERROR;
//   }

//   retVal = esp_partition_read(Show_Partition, offest, buffer, length);
//   if(retVal != ESP_OK){
//     ESP_LOGE(Tag, "Error reading to show memory:%s", errorToString(retVal));
//     return SHOW_READ_ERROR;
//   }

//   return SUCCESS;
// }

// #define DUMP_SHOW_STEPS 16
// void dumpShow(uint32_t start, uint8_t reps){
//   uint32_t i = 0;
//   uint8_t j = 0;
//   uint8_t buffer[DUMP_SHOW_STEPS];

//   printf("--------- Show Dump --------\n");

//   for(i = start; i < (reps * DUMP_SHOW_STEPS); i+=DUMP_SHOW_STEPS){
//     readShow(i, buffer, DUMP_SHOW_STEPS);
//     for(j = 0; j < DUMP_SHOW_STEPS; j++){
//       printf("%02X ", buffer[j]);
//     }
//     printf("... ");
//     for(j = 0; j < DUMP_SHOW_STEPS; j++){
//       printf("%c ", buffer[j]);
//     }
//     printf("\n");
//   }
// }

// //How to play in reverse

//   // if(Show_Internal_State == SHOW_INTERNAL_OK){
//   //   if(Show_State == SHOW_STATE_PLAY){
//   //     if(--Delay_Tick == 0){
//   //       if(Current_Frame == 0){
//   //         if(Show_On_Loop == ENABLE){
//   //           seekFrame(Total_Frames-1);
//   //         } else {
//   //           stop_show();
//   //         }
//   //       } else {
//   //         prevFrame();
//   //       }
//   //       printf("%d:%d [0] = %d\n", Current_Pointer, Current_Frame, Show_DMX_Buffer[0]);
//   //       copy_to_dmx(Show_DMX_Buffer);
//   //     }
//   //   } else {
//   //     ESP_LOGE(Tag, "Show State %d", Show_State);
//   //   }
//   // }