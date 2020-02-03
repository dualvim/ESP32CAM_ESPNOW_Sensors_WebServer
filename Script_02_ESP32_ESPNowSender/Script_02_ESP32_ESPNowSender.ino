/*
* Script_02_ESP32_ESPNowSender.ino     
*
*--> Sketch which read the sensors and uses the ESP-NOW to send the data to the ESP32-CAM:
*/
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h> //I2C
#include <Adafruit_BMP280.h> //Biblioteca para usar o BMP280
#include "RTClib.h"
#include <BH1750FVI.h>
#include "SenderModule_Constants_Structs.h"




/*****************************************
** Constants and global-scope variables **
*****************************************/
#define LDR_PIN  27 //Pin connected to the LDR 
#define DHT_PIN 33 //Pin connected to the DHT22 module
#define I2C_BMP280 0x76 //I2C address of the BMP280 module

// Wi-Fi parameters
#define CHAN_AP 2 // Wi-Fi Channel (1 to 13)

// MAC address of the receiver module
const uint8_t MAC_ESP32CAM[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// Struct used to send the data to the receiver
Struct_RTC_Env_Data data_esp32_1;
String status_msg = "ERROR";

// SSID and password of the Soft Access Point created in the ESP32-CAM
const char* SSID_AP = "Soft-Access-Point";
const char*  PASSWORD_AP = "123456789";




/*****************************************************************
** Objects to control the devices connected to the ESP32 board  **
*****************************************************************/
RTC_DS3231 rtc; // Object to control the DS3231 module
Adafruit_BMP280 bmp; //Object to control the BMP280 module
BH1750FVI luximeter(BH1750FVI::k_DevModeContLowRes); // Object to control the BH1750FVI module
DHT dht(DHT_PIN, DHT22);// Object to control the DHT22 module




/*************************
** Function prototypes  **
*************************/
void fcalback_data_sent( const uint8_t *mac_addr, esp_now_send_status_t deliv_status );
void connect_esp32_wifi_network( void );
void update_fields_struct_rtc_data( Struct_RTC_Env_Data *p_str_data );
void update_fields_bmp280_data( Struct_RTC_Env_Data *p_str_data );
void update_fields_dht22_data( Struct_RTC_Env_Data *p_str_data );
void update_fields_illuminationt_data( Struct_RTC_Env_Data *p_str_data );




/******************
** setup()       ** 
******************/
void setup() {
      Serial.begin(115200);
      delay(3000);
      
      // Configure the pin connected to the LDR
      pinMode(LDR_PIN, INPUT);
      
      // Intialize the DS3231 module
      rtc.begin();
      
      // Initialize the BMP280 module
      bmp.begin( I2C_BMP280 );
      
      // Initialize the BH1750 module
      luximeter.begin();
      
      // Initialize the DHT22 sensor:
      dht.begin();

      
      /****************************************************************************************************
      ** IMPORTANT: You must connect this module to the soft Access point where the web server is hosted **
      ****************************************************************************************************/
      WiFi.mode(WIFI_STA); // No problem in use the station mode in this module!
      connect_esp32_wifi_network(SSID_AP, PASSWORD_AP );


      // --> Initialize the ESP-NOW:
      if( esp_now_init() != ESP_OK ){
            Serial.println("Error initializing ESP-NOW");
            return;
      }

      // --> Register the send callback function:
      esp_now_register_send_cb( fcalback_data_sent ); //Function called whenever the ESP32 sends data
      
      // Register new ESP-NOW peer:
      esp_now_peer_info_t peerInfo;
      memcpy(peerInfo.peer_addr, MAC_ESP32CAM, 6);
      peerInfo.channel = CHAN_AP;  
      peerInfo.encrypt = false;

      // Add peer        
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
            Serial.println("Failed to add peer");
            return;
      }
}






/****************
** loop()      **  
****************/
void loop() {
      // Get the date and time in the RTC module
      update_fields_struct_rtc_data( &data_esp32_1 );
      
      // Update the BMP280 data
      update_fields_bmp280_data( &data_esp32_1 );
      
      // Update the DHT22 data
      update_fields_dht22_data( &data_esp32_1 );
      
      // Update the illumination data
      update_fields_illuminationt_data( &data_esp32_1 );

      // Send the data to the ESP32-CAM module
      esp_err_t result = esp_now_send(MAC_ESP32CAM, (uint8_t *) &data_esp32_1, sizeof(data_esp32_1));

      
      if( result == ESP_OK ){ 
            Serial.println("Delivery Success :)");
      }
      else{ 
            Serial.println("Delivery Fail :("); 
      }
      
      // Wait 10s
      delay(10000);

}




/*************************
** Callback Function    **
*************************/
// --> Callback function executed when a data is sent
void fcalback_data_sent( const uint8_t *mac_addr, esp_now_send_status_t deliv_status ){
      Serial.print("\r\nLast Packet Send Status:\t");
      
      // Update the global variable 'status_msg'
      if( deliv_status == ESP_NOW_SEND_SUCCESS ){
            Serial.println("Delivery Success");
            status_msg = "OK";
            
      }
      else {
            Serial.println("Delivery Fail");
            status_msg = "ERROR";
      }
}




/*********************************************************************************
** Function which connects the sender to the soft Acces Point in the ESP32-CAM  **
*********************************************************************************/
void connect_esp32_wifi_network( const char* ssid, const char* password ) {
      // --> Connect the ESP32 to the Wi-Fi Network:
      Serial.println("------------------------------------------------------------");
      // Connect to the Wi-Fi network
      WiFi.begin(ssid, password);
      Serial.print("Connecting to ");
      Serial.print(ssid);
      while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(500);
      }
      Serial.println("Ok\nConected!");
      
      // Show IP and MAC Address:
      Serial.print("IP Address: ");
      Serial.print(WiFi.localIP());
      Serial.print("; MAC Address: ");
      Serial.println(WiFi.macAddress());
      Serial.println("------------------------------------------------------------\n");
}




/*************************
** Auxiliary functions  **
*************************/
void update_fields_struct_rtc_data( Struct_RTC_Env_Data *p_str_data ){
      DateTime time_now = rtc.now();
      
      p_str_data->hh = (uint8_t) time_now.hour();
      p_str_data->hm = (uint8_t) time_now.minute();
      p_str_data->hs = (uint8_t) time_now.second();
      p_str_data->dy = (uint16_t) time_now.year();
      p_str_data->dm = (uint8_t) time_now.month();
      p_str_data->dd = (uint8_t) time_now.day();

      // temperature
      p_str_data->rtc_tmp = (float) rtc.getTemperature();

      // Formated strings
      p_str_data->rtc_date = get_formatted_date( p_str_data );
      p_str_data->rtc_hour = get_formatted_hour( p_str_data );
}


void update_fields_bmp280_data( Struct_RTC_Env_Data *p_str_data ){
      // Data from the BMP280 module
      p_str_data->bmp_temp = (float) bmp.readTemperature();
      p_str_data->bmp_pres = (float) bmp.readPressure() / 100.0F;
}


void update_fields_dht22_data( Struct_RTC_Env_Data *p_str_data ){
      p_str_data->dht_temp = dht.readTemperature(false); //Ler a temperatura
      p_str_data->dht_hum = dht.readHumidity();
}


void update_fields_illuminationt_data( Struct_RTC_Env_Data *p_str_data ){
      // Update field with the ADC value read in LDR pin
      p_str_data->adc_ldr = analogRead(LDR_PIN);
      delay(500);
      
      // Update field of the light intensity
      p_str_data->lux = luximeter.GetLightIntensity();
      delay(500);
}
