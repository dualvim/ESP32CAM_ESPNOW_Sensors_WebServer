/*
* Script_03_ESP32CAM_ESP-NOW_WebServer.ino     
*
* --> Sketch sent to the ESP32-CAM.
* --> Activities:   
*     - Receives the data from the ESP32 module
*     - Take a picture and saves it
*/



/*************************
** Include header files **
*************************/
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <FS.h>
#include "SD_MMC.h"            // SD Card ESP32
#include <SPIFFS.h> // SPIFFS of the ESP32-CAM
#include <EEPROM.h>            // read and write from flash memory
#include <esp_now.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h> // --> Async web server
#include <StringArray.h>
#include "Constants_structs_and_functions.h"
#include "WebServer_ESP32CAM.h"





/*****************************************
** Global scope constants and variables **
*****************************************/
// Pin of the ESP32-CAM which is connected to the camera's flash 
#define FLASH_PIN 4

// Soft AP constants
const char* SSID_AP = "Soft-Access-Point";
const char* PASSWORD_AP = "123456789";

// Wi-Fi constants
#define NETWORK_SSID "YOUR SSID"
#define NETWORK_PASS "YOUR PASSWORD"
#define CHAN_AP 2 // Wi-Fi Channel (1 to 13)

// Camera constants
#define FILE_PHOTO "/photo.jpg"

// Time interval to take a new picture if no picture have been taken
#define INTERVAL 10*60000 // 10 minutes


// Global scope variables
camera_config_t CONFIG_CAM;  // OV2640 camera configuration
Struct_RTC_Env_Data data_esp32_1; // Struct Struct_RTC_Env_Data
unsigned long lastMillis = 0; // Milliseconds counter:
double mean_temp; // Average temperature in the 3 sensors
boolean takeNewPhoto = false; //Boolean indicating if it should take another photo




// --> Object 'AsyncWebServer'
AsyncWebServer server(8888); // Asynchronous web server in port 8888





/******************************
** Function prototypes       **
******************************/
// --> Callback functions 
void fcalback_data_received(const uint8_t * mac, const uint8_t *received_data, int len);
void print_received_data( void );
String processor( const String& var );
// --> ESP32-CAM devices configuration
void ov2640_camera_module_configurations( void );
void initialize_spiffs( void );
void init_microSD_card( void );
void connect_esp32_wifi_network( char* ssid, char* password );
// --> EEPROM data manipulation
void set_value_in_eeprom_to_0( void );
uint8_t get_current_value_stored_in_eeprom( void );
void increment_current_value_stored_in_eeprom( void );
// --> Functions to take pictures
bool check_photo( fs::FS &fs, String str_file );
void take_picture_save_spiffs( void );
String copy_picture_spiffs_sdmmc( void ); 





/******************
** setup()       ** 
******************/
void setup() {
      // Serial ccnnection
      Serial.begin(115200);
      delay(3000);

      // Turn off the flash:
      pinMode(FLASH_PIN, OUTPUT);
      digitalWrite(FLASH_PIN, LOW);
      
      // Set the EEPROM value in 0 (just in the first time):
      set_value_in_eeprom_to_0();
      
      // Initialize devices attached to the ESP32-CAM
      ov2640_camera_module_configurations(); //Intialize the OV2640 camera module
      initialize_spiffs(); // Initialize the SPIFFS
      init_microSD_card(); // Initialize the SD Card module



      
      /**********************************************************************************
      ** IMPORTANT: Create the Soft Access Point;
      **    --> The ESP32 module running the web server must be an access point  
      **    --> The sender(s) module(s) must connect to this soft AP.    
      **    --> The ESP32 board with the soft AP then connects to the Wi-Fi network      
      **********************************************************************************/
      // Connect to the Wi-Fi Network
      WiFi.mode(WIFI_AP_STA);
      Serial.print("Creating the soft-AP...");
      WiFi.softAP(SSID_AP, PASSWORD_AP, CHAN_AP, 1);
      IPAddress IP = WiFi.softAPIP();
      Serial.print("IP do AP ");
      Serial.print(SSID_AP);
      Serial.print(": ");
      Serial.println(IP);

      
      // --> Connect the ESP32-CAM to the Wi-Fi Network
      connect_esp32_wifi_network(NETWORK_SSID, NETWORK_PASS);

      // --> Initialize the ESP-NOW:
      if( esp_now_init() != ESP_OK ){
            Serial.println("Error initializing ESP-NOW");
            return;
      }

      // --> Regiter the receive callback function:
      esp_now_register_recv_cb(fcalback_data_received); //Function called whenever the ESP32 receives data



      
      // --> Configure the asynchronous web server
      // Route for root / web page
      server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/html", index_html); } );

      // Date of the last data:
      server.on("/date_data", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/plain", data_esp32_1.rtc_date.c_str()); });

      // Hour of the last data:
      server.on("/hour_data", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/plain", data_esp32_1.rtc_hour.c_str()); });

      // Mean temperature:
      server.on("/temp_data", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/plain", String(mean_temp).c_str()); });

      // Air Pressuree:
      server.on("/press_data", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/plain", String(data_esp32_1.bmp_pres).c_str()); });

      // Humidity:
      server.on("/humid_data", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/plain", String(data_esp32_1.dht_hum).c_str()); });

      // ADC LDR:
      server.on("/ldr_data", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/plain", String(data_esp32_1.adc_ldr).c_str()); });

      // Lux:
      server.on("/lux_data", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/plain", String(data_esp32_1.lux).c_str()); });
      
      // Take a new picture
      server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
            request->send_P(200, "text/plain", "Tirando uma foto...Aguarde!"); 
      });

      // Last picture taken
      server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) { request->send(SPIFFS, FILE_PHOTO, "image/jpg"); });


      // --> Start the asynchronous web server
      Serial.println("Starting the Web Server");
      server.begin();
}



/****************
** loop()      **  
****************/
void loop() {
      // --> If asked to take a new photo:
      if( takeNewPhoto ){
            // Average temperature in the 3 sensors read:
            mean_temp = get_mean_temperature( &data_esp32_1 );
            
            // Call the function to take a photo with the camera and save it on the SPIFFS
            take_picture_save_spiffs();

            // Copy the picture saved in the SPIFFS to the SD card
            copy_picture_spiffs_sdmmc();//, &last_picture);

            // Update the value of 'takeNewPhoto'
            takeNewPhoto = false;
      }

      
      // --> Small delay to fix some issues with Wi-Fi stability
      delay(150);


      
      // --> If passed INTERVAL miliseconds without a new picture, change the value of 'takeNewPhoto'
      if( (millis() - lastMillis) > INTERVAL ){
            lastMillis = millis();
            takeNewPhoto = true;
      }

}




/*************************
** Callback Functions   **
*************************/
// --> Callback function executed whern the module receives data
void fcalback_data_received(const uint8_t * mac, const uint8_t *received_data, int len){
      // Copy 'received_data' content to 'last_picture'
      memcpy(&data_esp32_1, received_data, sizeof(data_esp32_1));
      // Number o received bytes:
      Serial.print("Bytes received: "); Serial.println(len);
      //Print the received data
      print_received_data();
}


void print_received_data( void ){
      // Print the date, time and temperature
      Serial.print(data_esp32_1.rtc_date); Serial.print(" - ");
      Serial.print(data_esp32_1.rtc_hour); Serial.print(" - ");
      Serial.print(data_esp32_1.rtc_tmp); Serial.println(" ºC");

      // Print the other fields of the 'weather_data' module
      Serial.print("sizeof(data_esp32_1) = "); Serial.print(sizeof(data_esp32_1)); Serial.println(" bytes");
      Serial.print("ADC LDR = "); Serial.print(data_esp32_1.adc_ldr); Serial.print(";  "); Serial.print(data_esp32_1.lux); Serial.println(" lux");
      Serial.print("Temp. BMP280 = "); Serial.print(data_esp32_1.bmp_temp); Serial.print(" ºC; Press. BMP280 = "); Serial.print(data_esp32_1.bmp_pres); Serial.println(" hPa");
      Serial.print("Temp. DHT22 = "); Serial.print(data_esp32_1.dht_temp); Serial.print(" ºC; Humid. DHT22 = "); Serial.print(data_esp32_1.dht_hum); Serial.println("%");
      Serial.print("Mean temperature = "); Serial.print(get_mean_temperature( &data_esp32_1 ) ); Serial.println(" ºC");
      Serial.println("\n");
}


// --> processor():
String processor( const String& var ) {
       //-->  If 'var' is HOUR_DATA:
      if (var == "HOUR_DATA") { return data_esp32_1.rtc_hour; }
      
      //--> If 'var' is HOUR_PHOTO:
      else if (var == "HOUR_PHOTO") { return data_esp32_1.rtc_date; }
      

      //--> If 'var' is TEMP_DATA:
      else if (var == "TEMP_DATA") { return String(mean_temp); }

      //--> If 'var' is PRESS_DATA:
      else if (var == "PRESS_DATA") { return String(data_esp32_1.bmp_pres); }

      //--> If 'var' is HUMID_DATA:
      else if (var == "HUMID_DATA") { return String(data_esp32_1.dht_hum); }

      //--> If 'var' is LDR_DATA:
      else if (var == "LDR_DATA") { return String(data_esp32_1.adc_ldr); }

      //--> If 'var' is LUX_DATA:
      else if (var == "LUX_DATA") { return String(data_esp32_1.lux); }
      
      // --> Else, return empty string
      return String();
}




/*********************************************
** Auxiliary Functions for the web server   **
*********************************************/
void ov2640_camera_module_configurations( void ) {
      Serial.print("Inicializing the OV2640 camera module...");
      // --> OV2640 camera configuration
      CONFIG_CAM.ledc_channel = LEDC_CHANNEL_0;
      CONFIG_CAM.ledc_timer = LEDC_TIMER_0;
      CONFIG_CAM.pin_d0 = Y2_GPIO_NUM;
      CONFIG_CAM.pin_d1 = Y3_GPIO_NUM;
      CONFIG_CAM.pin_d2 = Y4_GPIO_NUM;
      CONFIG_CAM.pin_d3 = Y5_GPIO_NUM;
      CONFIG_CAM.pin_d4 = Y6_GPIO_NUM;
      CONFIG_CAM.pin_d5 = Y7_GPIO_NUM;
      CONFIG_CAM.pin_d6 = Y8_GPIO_NUM;
      CONFIG_CAM.pin_d7 = Y9_GPIO_NUM;
      CONFIG_CAM.pin_xclk = XCLK_GPIO_NUM;
      CONFIG_CAM.pin_pclk = PCLK_GPIO_NUM;
      CONFIG_CAM.pin_vsync = VSYNC_GPIO_NUM;
      CONFIG_CAM.pin_href = HREF_GPIO_NUM;
      CONFIG_CAM.pin_sscb_sda = SIOD_GPIO_NUM;
      CONFIG_CAM.pin_sscb_scl = SIOC_GPIO_NUM;
      CONFIG_CAM.pin_pwdn = PWDN_GPIO_NUM;
      CONFIG_CAM.pin_reset = RESET_GPIO_NUM;
      CONFIG_CAM.xclk_freq_hz = 20000000;
      CONFIG_CAM.pixel_format = PIXFORMAT_JPEG;
      // --> Frame size:
      if (psramFound()) {
            CONFIG_CAM.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
            CONFIG_CAM.jpeg_quality = 10;
            CONFIG_CAM.fb_count = 2;
      } 
      else {
            CONFIG_CAM.frame_size = FRAMESIZE_SVGA;
            CONFIG_CAM.jpeg_quality = 12;
            CONFIG_CAM.fb_count = 1;
      }
      // --> Initialize the camera module
      esp_err_t err = esp_camera_init(&CONFIG_CAM);
      if (err != ESP_OK) {
            Serial.printf("Camera init failed with error 0x%x", err);
            return;
      }
      Serial.println("Ok!");
}



/********************************************************
** Connect the ESP32-CAM to the Wi-Fi Network          **
********************************************************/
void connect_esp32_wifi_network( char* ssid, char* password ) {
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





/********************************************************
** Initialize the ESP32-CAM SPIFFS and MicroSD module  **
********************************************************/
void initialize_spiffs( void ) {
      // Initialize the SPIFFS file system
      Serial.print("Starting the SPIFFS file system...");
      if( !SPIFFS.begin( true ) ){
            Serial.println("ERROR");
            return;
      } 
      else { delay(500); Serial.println("Ok!"); }
}




void init_microSD_card( void ){
      Serial.println("Starting SD Card...");
      if(!SD_MMC.begin()){
            Serial.println("Error! --> SD Card Mount Failed");
            return;
      }
      uint8_t cardType = SD_MMC.cardType();
      if(cardType == CARD_NONE){
            Serial.println("Error! --> No SD Card attached");
            return;
      }
      Serial.print("Ok! --> cardType = ");
      Serial.println(cardType);
}





/*************************************
** Read/Update EEPROM value         **
*************************************/
void set_value_in_eeprom_to_0( void ){
      EEPROM.begin(EEPROM_SIZE);
      Serial.print("EEPROM  Address: "); Serial.println(EEPROM.read( EEPROM_ADDRESS ));
      EEPROM.write(EEPROM_ADDRESS, 0);
      Serial.print("EEPROM = "); Serial.println(EEPROM.read(EEPROM_ADDRESS));
      EEPROM.commit();
}


uint8_t get_current_value_stored_in_eeprom( void ){
      EEPROM.begin(EEPROM_SIZE);
      uint8_t curr = EEPROM.read(EEPROM_ADDRESS);
      return curr;
}


void increment_current_value_stored_in_eeprom( void ){
      EEPROM.begin(EEPROM_SIZE);

      // Valor atual salvo na EEPROM
      uint8_t curr = EEPROM.read(EEPROM_ADDRESS );

      // Incrementar o valor atual
      if( curr < 255 ){ ++curr; }
      else{ curr = 0;}

      // Salvar o valor atual na EEPROM
      EEPROM.write(EEPROM_ADDRESS, curr);
      EEPROM.commit();
}




/**************************************************
** Functions using the camera and saving files   **
**************************************************/
// --> Check if the picture file saved in SPIFFS is greater than 100 bytes
bool check_photo( fs::FS &fs, String str_file ) {
        String path = str_file;
        File f_pic = fs.open( path.c_str() );
        unsigned int pic_sz = f_pic.size();
        f_pic.close();
        return ( pic_sz > 100 );
}



// --> Take a picture and save it in SPIFFS
void take_picture_save_spiffs( void ){
      // --> Turn on the flash
      digitalWrite(FLASH_PIN, HIGH); 


      // Pointer to the photo data and booleans indicanting if the picture have been saved
      camera_fb_t * fb = NULL; // pointer
      bool ok = 0; // Boolean indicating if the picture file is correctly saved in the SPIFFS


      // --> Repeat the loop untill the files of the photo be correct
      do {
            // Take a picture with the camera
            Serial.print("Taking a picture with the OV2640 camera...");
            fb = esp_camera_fb_get();
            if (!fb) {
                  Serial.println("ERROR! Camera capture failed");
                  return;
            }
            Serial.println("OK!");
            
            // --> Save the picture in the SPIFFS
            Serial.println("Saving the photo in the SPIFFS");
            File spiffs_file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
            if( !spiffs_file ) { 
                  Serial.println("Failed to open file in writing mode"); 
            }
            else{
                  spiffs_file.write(fb->buf, fb->len); // payload (image), payload length
                  Serial.print("The picture has been saved in the SPIFFS with the name ");
                  Serial.print(FILE_PHOTO);
                  Serial.print(" - Size: ");
                  Serial.print(spiffs_file.size());
                  Serial.println(" bytes");
            }
            spiffs_file.close(); // Close the file
            ok = check_photo( SPIFFS, FILE_PHOTO ); //Check if its ok

            esp_camera_fb_return(fb);
      } while ( !ok );


      // --> Turn off the flash
      digitalWrite(FLASH_PIN, LOW); 
}



// --> Copy to the SD card the picture saved in SPIFFS
String copy_picture_spiffs_sdmmc( void ){//, Picture_Name *p_nome ){
      // Get current value saved in EEPROM
      uint8_t pic_num = get_current_value_stored_in_eeprom();

      // Filename in the SD card
      String filename_sd_card = get_file_name_sd_card( pic_num );

      // Copy the filename to 'p_nome->pic_name'
      //p_nome->pic_name = filename_sd_card;
      
      // File handler from the image saved in SPIFFS
      File spiffs_file = SPIFFS.open( FILE_PHOTO );

      // File with a copy of the SPIFFS image
      File mmc_file = SD_MMC.open( filename_sd_card.c_str(), FILE_WRITE);

      // Copy the content of 'spiffs_file' to 'mmc_file'
      static uint8_t buf[512];
      while( spiffs_file.read(buf, 512) ){
            mmc_file.write( buf, 512 );
      }
      
      // Close the files
      spiffs_file.close();
      mmc_file.close();

      // Check and print the size of the file copyed to the MicroSD card
      File f_pic = SD_MMC.open( filename_sd_card.c_str() );
      unsigned int pic_sz = f_pic.size();
      f_pic.close();
      Serial.print("Filename: "); Serial.print(filename_sd_card);
      Serial.print("\tSize: "); Serial.print(pic_sz);
      Serial.println(" bytes");

      // Update the value saved in EEPROM
      increment_current_value_stored_in_eeprom();

      // Return the string 'filename_sd_card'
      return filename_sd_card;
}
