/*
* SenderModule_Constants_Structs.h
*      
* --> This header file contains the structs and function prototypes of the functions commonly used in both ESP32 modules;
*		
*/


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
