/***************************************************
  This is an example for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963
  ----> http://www.adafruit.com/products/2468
  ----> http://www.adafruit.com/products/2542

  These cellular modules use TTL Serial to communicate, 2 pins are
  required to interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

/*
THIS CODE IS STILL IN PROGRESS!

Open up the serial console on the Arduino at 115200 baud to interact with FONA


This code will receive an SMS, identify the sender's phone number, and automatically send a response

*/

//adding base64 lbr
#include <Base64.h>

#include "Adafruit_FONA.h"

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// this is a large buffer for replies
char replybuffer[255];
char replybuffer2[255];

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Hardware serial is also possible!
//  HardwareSerial *fonaSerial = &Serial1;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

void setup() {
  while (!Serial);

  Serial.begin(115200);
  Serial.println(F("FONA SMS caller ID test"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  // make it slow so its easy to read!
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while(1);
  }
  Serial.println(F("FONA is OK"));

  // Print SIM card IMEI number.
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: "); Serial.println(imei);
  }
  
  Serial.println("FONA Ready");
}

  
char fonaInBuffer[64];          //for notifications from the FONA

void loop() {
  
  char* bufPtr = fonaInBuffer;    //handy buffer pointer
  
  if (fona.available())      //any data available from the FONA?
  {
    int slot = 0;            //this will be the slot number of the SMS
    int charCount = 0;
    //Read the notification into fonaInBuffer
    do  {
      *bufPtr = fona.read();
      Serial.write(*bufPtr);
      delay(1);
    } while ((*bufPtr++ != '\n') && (fona.available()) && (++charCount < (sizeof(fonaInBuffer)-1)));
    
    //Add a terminal NULL to the notification string
    *bufPtr = 0;
    
    //Scan the notification string for an SMS received notification.
    //  If it's an SMS message, we'll get the slot number in 'slot'
    if (1 == sscanf(fonaInBuffer, "+CMTI: \"SM\",%d", &slot)) {
      Serial.print("slot: "); Serial.println(slot);
      
      char callerIDbuffer[32];  //we'll store the SMS sender number in here
      
      // Retrieve SMS sender address/phone number.
      if (! fona.getSMSSender(slot, callerIDbuffer, 31)) {
        Serial.println("Didn't find SMS message in slot!");
      }
      Serial.print(F("FROM: ")); Serial.println(callerIDbuffer);
      //check content
      
      Serial.print("slot: "); Serial.println(slot);
      // Retrieve SMS value.
      uint16_t smslen;
      if (! fona.readSMS(slot, replybuffer, 250, &smslen)) { // pass in buffer and max len!
        Serial.println("Failed!");
//        break;
      }
      String whereCommand = "Where";
      String statusCommand = "Status";
      String msg = String(replybuffer);
      if(msg.equalsIgnoreCase(whereCommand)){
        
        // turn GPRS on
        if (!fona.enableGPRS(true))
          Serial.println(F("Failed to turn on"));
          
        // check for GSMLOC (requires GPRS)
        uint16_t returncode;

        if (!fona.getGSMLoc(&returncode, replybuffer, 250))
          Serial.println(F("Failed!"));
        if (returncode == 0) {
          Serial.println(replybuffer);
        } else {
          Serial.print(F("Fail code #")); Serial.println(returncode);
        }
        String tmpData = String(replybuffer);
        tmpData += "gps ";
        
        
        // encoding
//String data = "" + String(millis());
        char input[tmpData.length()];
        tmpData.toCharArray(input, tmpData.length());
        int inputLen = tmpData.length();
        Serial.println(input);
        int encodedLen = base64_enc_len(inputLen);
        
      //  Serial.print(input); Serial.print(" = ");
        
        char encoded[encodedLen];
        // note input is consumed in this step: it will be empty afterwards
        base64_encode(encoded, input, inputLen); 
        
        Serial.println(encoded);
        delay(1000);

//        strncpy(encoded, replybuffer, inputLen);
//        tmpData.toCharArray(replybuffer,255);
        
//        replybuffer += "gps";
        
        //Send back an automatic response
//        Serial.println("Sending reponse...where");
//          char* p = strtok(replybuffer, ",");
//          replybuffer2 = strdup(p) + r  eplybuffer2;
//          p = strtok(NULL,",");
//          replybuffer2 = strdup(p) + "," + replybuffer2;
//          replybuffer2 = "http://maps.google.com/maps?q=" + replybuffer2;
          
        if (!fona.sendSMS(callerIDbuffer, encoded)) {
//        if (!fona.sendSMS(callerIDbuffer, "http://maps.google.com/maps?q=49.188087,-122.848480")) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Sent!"));
        }
      }else if(msg.equalsIgnoreCase(statusCommand)){
        
        // read the battery voltage and percentage
        uint16_t vbat;
        String batStat = "Battery Voltage is ";
        
        if (! fona.getBattVoltage(&vbat)) {
          Serial.println(F("Failed to read Batt"));
        } else {
          Serial.print(F("VBat = ")); Serial.print(vbat); Serial.println(F(" mV"));
          batStat += String(vbat);
          batStat += " mV. Battery has ";
        }


        if (! fona.getBattPercent(&vbat)) {
          Serial.println(F("Failed to read Batt"));
        } else {
          Serial.print(F("VPct = ")); Serial.print(vbat); Serial.println(F("%"));
          batStat += String(vbat);
          batStat += "% left.";
        }
        
        //Send back an automatic response
        Serial.println("Sending reponse...status");
        batStat.toCharArray(replybuffer,255);
        if (!fona.sendSMS(callerIDbuffer, replybuffer)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Sent!"));
        }
      }else{
        
        //Send back an automatic response
        Serial.println("Sending reponse...dont know");
        if (!fona.sendSMS(callerIDbuffer, "Whaaaaaattt.... i dont get it, sorry")) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Sent!"));
        }
      }
//      char whereCommand[] = "where";
//      char statusCommand[] = "status";
//      Serial.println(whereCommand.length());
//      // cases: 0, where; 1, status; 2, do nothing
//      int case = 0;
//      for(int i = 0; i < whereCommand.length(); i ++){
//         if(whereCommand[i] != replybuffer[i]){
//          case = 1;
//          break;
//         }
//      }
//      if(case == 0){
//        Serial.println("address URL");
//      }else{
//        for(int i = 0; i < statusCommand.length(); i ++){
//         if(statusCommand[i] != replybuffer[i]){
//          case = 2;
//          break;
//         }
//      }

     
        
      
//        flushSerial();
        Serial.print(F("Delete All"));
//        uint8_t smsn = readnumber();
        for(int i = 0; i < 10; i++){
            
          Serial.print(F("\n\rDeleting SMS #")); Serial.println(i);
          if (fona.deleteSMS(i)) {
            Serial.println(F("OK!"));
          } else {
            Serial.println(F("Couldn't delete"));
          }
        }
    }
  }
}
