/*https://github.com/maarten-pennings/CCS811
 * + https://github.com/JoaoLopesF/RemoteDebug
 * 
 * 
 * ccs811basic.ino - Demo sketch printing results of the CCS811 digital gas sensor for monitoring indoor air quality from ams.
 * 
 * 
 * Pinout: 
 * For the NodeMCU (ESP8266), connect as follows (I did not use pull-ups, presumably they are inside the MCU)
 * CCS811    ESP8266
 *  VDD        3V3
 *  GND        GND
 *  SDA        D2
 *  SCL        D1
 *  nWAKE      D3 (or GND)
 * 
 * 
 * 
 * 
*/


#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library

// Wifi 
#include <ESP8266WiFi.h>

//OTA
//#include <WiFiUdp.h>
//#include <ArduinoOTA.h>

// for mDNS
#include <DNSServer.h>
#include <ESP8266mDNS.h>

// Remote debug over telnet - not recommended for production, only for development
#include "RemoteDebug.h"  
RemoteDebug Debug;

// SSID and password
const char* ssid = "...";
const char* password = "....";

// Host mDNS
#define HOST_NAME "remotedebug-sample"


// Wiring for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)
CCS811 ccs811(D3); // nWAKE on D3


void setup() {
  // Enable serial
  Serial.begin(115200);
  
  // get Serial.print in serial monitor nn
  setup_wifi();   
  setup_remoteDebug();
  
  Serial.println("");
  Serial.println("Starting CCS811 basic demo");

  // Enable I2C
  Wire.begin(); 
  
  // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("init: CCS811 begin FAILED");
  if (!ok) rdebugDln("init: CCS811 begin  FAILED");
  
  // Print CCS811 versions
  Serial.print("init: hardware    version: "); Serial.println(ccs811.hardware_version(),HEX);
  Serial.print("init: bootloader  version: "); Serial.println(ccs811.bootloader_version(),HEX);
  Serial.print("init: application version: "); Serial.println(ccs811.application_version(),HEX);

  // Start measuring
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) Serial.println("init: CCS811 start measuring FAILED");
  if (!ok) rdebugDln("init: CCS811 start measuring FAILED");



  

}


void loop() {
  // *** ouput from startup
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("init: CCS811 begin FAILED");
  if (!ok) rdebugDln("init: CCS811 begin  FAILED");
  
  // Print CCS811 versions
  int hardware  = ccs811.hardware_version();
  int bootloader  = ccs811.bootloader_version();
  int applic  = ccs811.application_version();
  rdebugDln("init: hardware    version: %u ", hardware); 
  rdebugDln("init: bootloader  version: %u ", bootloader); 
  rdebugDln("init: application version: %u ", applic); 
  Serial.print("init: hardware    version: "); Serial.println(ccs811.hardware_version(),HEX);
  Serial.print("init: bootloader  version: "); Serial.println(ccs811.bootloader_version(),HEX);
  Serial.print("init: application version: "); Serial.println(ccs811.application_version(),HEX);

  // Start measuring
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) Serial.println("init: CCS811 start measuring FAILED");
  if (!ok) rdebugDln("init: CCS811 start measuring FAILED");

  delay(1000);

  // *** end



  
  // Read
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw); 




  // *** get the readings
  rdebugDln("start debug");
  Serial.print("eco2=");  Serial.print(eco2);     Serial.print(" ppm  ");
  rdebugDln("eco2=  %u  ppm" ,eco2 );
  Serial.print("etvoc="); Serial.print(etvoc);    Serial.print(" ppb  ");
  rdebugDln("etvoc=  %u ppb ",etvoc );
  rdebugDln("raw %u ",raw );
  rdebugDln("erstat %u ",errstat );
  rdebugDln("end debug");
  // *** end


  
  
  // Print measurement results based on status
  if( errstat==CCS811_ERRSTAT_OK ) { 
    Serial.print("CCS811: ");
    Serial.print("eco2=");  Serial.print(eco2);     Serial.print(" ppm  ");
    rdebugDln("eco2=  %u  ppm",eco2 );
    Serial.print("etvoc="); Serial.print(etvoc);    Serial.print(" ppb  ");
    rdebugDln("etvoc=  %u ppb ",etvoc );
    //Serial.print("raw6=");  Serial.print(raw/1024); Serial.print(" uA  "); 
    //Serial.print("raw10="); Serial.print(raw%1024); Serial.print(" ADC  ");
    //Serial.print("R="); Serial.print((1650*1000L/1023)*(raw%1024)/(raw/1024)); Serial.print(" ohm");
    Serial.println();
  } else if( errstat==CCS811_ERRSTAT_OK_NODATA ) {
    Serial.println("CCS811: waiting for (new) data");
    rdebugDln("CCS811: waiting for (new) data");
  } else if( errstat & CCS811_ERRSTAT_I2CFAIL ) { 
    Serial.println("CCS811: I2C error");
    rdebugDln("CCS811: I2C error");
    rdebugDln("CCS811: errstat=  %u",errstat );
    rdebugDln("CCS811: decode errstat=   %u",ccs811.errstat_str(errstat) );
 
    
    
  } else {
    Serial.print("CCS811: errstat="); Serial.print(errstat,HEX); 
    Serial.print("="); Serial.println( ccs811.errstat_str(errstat) ); 
    rdebugDln("CCS811: errstat=  %u",errstat );
    rdebugDln(" =  %u",ccs811.errstat_str(errstat) );
  }
  
  // Wait
  delay(1000); 

  // RemoteDebug
  // Remote debug over telnet
  Debug.handle();

  // Give a time for ESP8266
  yield();
}



void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connexion OK ");
  Serial.print("=> Addresse IP : ");
  Serial.print(WiFi.localIP());
}

void setup_remoteDebug() {
    // Register host name in WiFi and mDNS
    String hostNameWifi = HOST_NAME;
    hostNameWifi.concat(".local");
    WiFi.hostname(hostNameWifi);


    // start mDNS
    if (MDNS.begin(HOST_NAME)) {
        Serial.print("* MDNS responder started. Hostname -> ");
        Serial.println(HOST_NAME);
    }

    MDNS.addService("telnet", "tcp", 23);


    // Initialize the telnet server of RemoteDebug
    Debug.setSerialEnabled(true); // If we want Serial.print() in Putty

    Debug.begin(HOST_NAME); // Initiaze the telnet server

    Debug.setResetCmdEnabled(true); // Enable the reset command

    //Debug.showTime(true); // To show time

    // Debug.showProfiler(true); // To show profiler - time between messages of Debug
                              // Good to "begin ...." and "end ...." messages

    // This sample (serial -> educattional use only, not need in production)
    Serial.println("* Arduino RemoteDebug Library");
    Serial.println("*");
    Serial.print("* WiFI connected. IP address: ");
    Serial.println(WiFi.localIP());
}

