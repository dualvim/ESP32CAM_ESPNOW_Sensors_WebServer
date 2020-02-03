/*
* Constants_structs_and_functions.h
*      
* --> This header file contains the structs and functions used in both ESP32 modules;
*            
*/


/********************************
** Simbolic constants          **
********************************/
// --> Pins from the OV2640 camera module (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1 // RESET pin is not available
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26 //SDA pin - 'GPIO 26'
#define SIOC_GPIO_NUM     27 //SCL pin - 'GPIO 26'
#define Y9_GPIO_NUM       35 //D7 pin - 'GPIO 35'
#define Y8_GPIO_NUM       34 //D6 pin - 'GPIO 34'
#define Y7_GPIO_NUM       39 //D5 pin - 'GPIO 39'
#define Y6_GPIO_NUM       36 //D4 pin - 'GPIO 36'
#define Y5_GPIO_NUM       21 //D3 pin - 'GPIO 21'
#define Y4_GPIO_NUM       19 //D2 pin - 'GPIO 19'
#define Y3_GPIO_NUM       18 //D1 pin - 'GPIO 18'
#define Y2_GPIO_NUM        5 //D0 pin - 'GPIO 5'
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


// --> EEPROM constants: picture number saved in card
#define EEPROM_SIZE 1
#define EEPROM_ADDRESS 0






/********************************
** struct Struct_RTC_Env_Data  **
********************************/
struct struct_rtc_env_data {
      uint16_t dy;
      uint8_t dm;
      uint8_t dd;
      uint8_t hh;
      uint8_t hm;
      uint8_t hs;
      float rtc_tmp;
      String rtc_date;
      String rtc_hour;
      float bmp_temp;
      float bmp_pres;
      float dht_temp;
      float dht_hum;
      uint16_t adc_ldr;
      uint16_t lux;
};


// Type Struct_RTC_Env_Data
typedef struct struct_rtc_env_data Struct_RTC_Env_Data;






/*************************
** Function prototypes  **
*************************/
String get_formatted_date( Struct_RTC_Env_Data *p_str_data );
String get_formatted_hour( Struct_RTC_Env_Data *p_str_data);
double get_mean_temperature(  Struct_RTC_Env_Data *p_str_data );
String get_file_name_sd_card( uint8_t pic_num );






/*************************
** Auxiliary Functions  **
*************************/
String get_formatted_date( Struct_RTC_Env_Data *p_str_data ){
      String str_rtc_date = "";

      str_rtc_date += String(p_str_data->dy) + "-";

      if( p_str_data->dm < 10 ){  str_rtc_date += "0";  }
      str_rtc_date += String(p_str_data->dm) + "-";

      if( p_str_data->dd < 10 ){  str_rtc_date += "0";  }
      str_rtc_date += String(p_str_data->dd);

      return  str_rtc_date;
}


String get_formatted_hour( Struct_RTC_Env_Data *p_str_data){
      String str_rtc_hour = "";

      if( p_str_data->hh < 10 ){  str_rtc_hour += "0";  }
      str_rtc_hour += String(p_str_data->hh) + ":";

      if( p_str_data->hm < 10 ){  str_rtc_hour += "0";  }
      str_rtc_hour += String(p_str_data->hm) + ":";

      if( p_str_data->hs < 10 ){  str_rtc_hour += "0";  }
      str_rtc_hour += String(p_str_data->hs);

      // Update the 'p_str_data->str_hour' field
      return str_rtc_hour;
}


double get_mean_temperature(  Struct_RTC_Env_Data *p_str_data ){
      double mean_temp = p_str_data->rtc_tmp;
      mean_temp += (double) p_str_data->bmp_temp;
      mean_temp += (double) p_str_data->dht_temp;
      
      return mean_temp / (double)3.0;
}


String get_file_name_sd_card( uint8_t pic_num ){
      String path = (pic_num < 100) ? ((pic_num < 10) ? "/pic_00" : "/pic_0") : "/pic_"; 
      path = path + String(pic_num) + ".jpg";
      return path;
}

   
