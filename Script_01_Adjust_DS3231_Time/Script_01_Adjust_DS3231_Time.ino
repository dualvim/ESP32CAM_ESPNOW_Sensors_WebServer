/*
* Script_01_Adjust_DS3231_Time.ino     
*
*--> Adjust the RTC module DS3231 time
*/

#include "RTClib.h"




/*************************
** struct RTC_Data      **
*************************/
struct rtc_data {
      uint8_t hh;
      uint8_t hm;
      uint8_t hs;
      uint16_t dy;
      uint8_t dm;
      uint8_t dd;
      float tmp;
      String str_date;
      String str_hour;
      
};

// Type 'RTC_Data'
typedef struct rtc_data RTC_Data;


//--> Global variable of type 'RTC_Data'
RTC_Data ds3231_data;


// --> Object to control the DS3231 module
RTC_DS3231 rtc;



// --> Function prototypes
void get_formatted_hour( RTC_Data *p_rtc_data );
void get_formatted_date( RTC_Data *p_rtc_data );
void update_fields_struct_rtc_data( RTC_Data *p_rtc_data );



/******************
** setup()       ** 
******************/
void setup() {
      Serial.begin(115200);
     rtc.begin();

      Serial.println(F(__DATE__));
      Serial.println(F(__TIME__));
     // Set the date and time in the RTC module
     //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

      
}



/****************
** loop()      **  
****************/
void loop() {
      // Get the date and time in the RTC module
      update_fields_struct_rtc_data( &ds3231_data );

      // Print the date, time and temperature
      Serial.print(ds3231_data.str_date); Serial.print(" - ");
      Serial.print(ds3231_data.str_hour); Serial.print(" - ");
      Serial.print(ds3231_data.tmp); Serial.println(" ºC");
      delay(5000);

}


/*************************
** Auxiliary functions  **
*************************/
void get_formatted_date( RTC_Data *p_rtc_data ){
      String str_rtc_date = "";

      str_rtc_date += String(p_rtc_data->dy) + "-";

      if( p_rtc_data->dm < 10 ){  str_rtc_date += "0";  }
      str_rtc_date += String(p_rtc_data->dm) + "-";

      if( p_rtc_data->dd < 10 ){  str_rtc_date += "0";  }
      str_rtc_date += String(p_rtc_data->dd);

      p_rtc_data->str_date = str_rtc_date;
}


void get_formatted_hour( RTC_Data *p_rtc_data ){
      String str_rtc_hour = "";

      if( p_rtc_data->hh < 10 ){  str_rtc_hour += "0";  }
      str_rtc_hour += String(p_rtc_data->hh) + ":";

      if( p_rtc_data->hm < 10 ){  str_rtc_hour += "0";  }
      str_rtc_hour += String(p_rtc_data->hm) + ":";

      if( p_rtc_data->hs < 10 ){  str_rtc_hour += "0";  }
      str_rtc_hour += String(p_rtc_data->hs);

      // Update the 'p_rtc_data->str_hour' field
      p_rtc_data->str_hour = str_rtc_hour;
}


void update_fields_struct_rtc_data( RTC_Data *p_rtc_data ){
      DateTime time_now = rtc.now();
      
      p_rtc_data->hh = (uint8_t) time_now.hour();
      p_rtc_data->hm = (uint8_t) time_now.minute();
      p_rtc_data->hs = (uint8_t) time_now.second();
      p_rtc_data->dy = (uint16_t) time_now.year();
      p_rtc_data->dm = (uint8_t) time_now.month();
      p_rtc_data->dd = (uint8_t) time_now.day();
      p_rtc_data->tmp = (float) rtc.getTemperature();

      // Formatted strings:
      get_formatted_date( p_rtc_data );
      get_formatted_hour( p_rtc_data );
}
