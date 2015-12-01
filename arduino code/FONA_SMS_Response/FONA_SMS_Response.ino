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


#include "Adafruit_FONA.h"

//sleep lib
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

//timer definition, dont change as i dont understand what this means
volatile int f_wdt=1;

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

int countdown;
int maxCountdown = 24; //this number times 10 is how many second it will do something;

int keyPin = 5;
int pwStatPin = 6;
int ledPin = 13;


//again, dont change this as i dont know how this works
/***************************************************
 *  Name:        ISR(WDT_vect)
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Watchdog Interrupt Service. This
 *               is executed when watchdog timed out.
 *
 ***************************************************/
ISR(WDT_vect)
{
  if(f_wdt == 0)
  {
    f_wdt=1;
  }
  else
  {
    Serial.println("WDT Overrun!!!");
  }
}


/***************************************************
 *  Name:        enterSleep
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Enters the arduino into sleep mode.
 *
 ***************************************************/
void enterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
  sleep_enable();
  
  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the WDT timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}


void setup() {
  while (!Serial);

  pinMode(keyPin, OUTPUT);
  pinMode(pwStatPin, INPUT);
  pinMode(ledPin, OUTPUT);
  delay(1000);
  Serial.begin(115200);
  Serial.println(F("FONA SMS caller ID test"));
  Serial.println(F("Initializing....(May take 3 seconds)"));
  delay(3000);
  turnOnGPS();
  // make it slow so its easy to read!
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
//    while(1);
  }
  Serial.println(F("FONA is OK"));
//
//  // Print SIM card IMEI number.
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: "); Serial.println(imei);
  }
  
  Serial.println("FONA Ready");
  turnOffGPS();

  delay(500);
  
  /*** Setup the WDT ***/
  
  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);
  
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);
}

  
char fonaInBuffer[64];          //for notifications from the FONA

void loop() {
  
  if(f_wdt == 1)
  {
    if(countdown >= 0){
      countdown--;
    }else{
      //do something
//      wdt_disable()
      Serial.println("wake!");
      digitalWrite(ledPin, HIGH);
      wdt_reset();
      turnOnGPS();
      Serial.println("poop");
      wdt_reset();
      delay(5000);//give it some time to start
      wdt_reset();
      sendGPS();
      wdt_reset();
      delay(5000);
      wdt_reset();
      turnOffGPS();
      wdt_reset();
      //reset counter
      countdown = maxCountdown;

      //give it time to do stuff
      delay(500);
      Serial.println("back to sleep");
      delay(500);      
      digitalWrite(ledPin, LOW);
//      wdt_enable(WDTO_8S);
    }
    /* Don't forget to clear the flag. */
    f_wdt = 0;
    
    /* Re-enter sleep mode. */
    enterSleep();
  }
  else
  {
    /* Do nothing. */
  }
  
}




void turnOnGPS(){
  if(digitalRead(pwStatPin) != HIGH){
    Serial.println("Off, trying to turn on");
    digitalWrite(keyPin,HIGH);
    delay(1000);
    digitalWrite(keyPin,LOW);
    delay(1000);
  }else{
    Serial.println("it's already on");
  }
}
void turnOffGPS(){
  if(digitalRead(pwStatPin) != LOW){
    Serial.println("on, trying to turn off");
    digitalWrite(keyPin,HIGH);
    delay(1000);
    digitalWrite(keyPin,LOW);
    delay(1000);
  }else{
    Serial.println("it's already off");
  }
}

void sendGPS(){
  wdt_reset();
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
//    while(1);
  } 
  Serial.println(F("FONA is OK"));
  wdt_reset();
  
  if (!fona.enableGPRS(true))
    Serial.println(F("Failed to turn on"));
 wdt_reset();
  // check for GSMLOC (requires GPRS)
  uint16_t returncode;
  
  if (!fona.getGSMLoc(&returncode, replybuffer, 250))
    Serial.println(F("Failed!"));
  if (returncode == 0) {
    Serial.println(replybuffer);
  } else {
    Serial.print(F("Fail code #")); Serial.println(returncode);
  }
//  String tmpData = String(replybuffer);
  //        tmpData += "gps";
  
//  tmpData.toCharArray(replybuffer,255);
//  if (!fona.sendSMS(callerIDbuffer, replybuffer)) {
//    Serial.println(F("Failed"));
//  } else {
//    Serial.println(F("Sent!"));
//  }
  wdt_reset();
  // read the battery voltage and percentage
  uint16_t vbat;  
  if (! fona.getBattPercent(&vbat)) {
    Serial.println(F("Failed to read Batt"));
  } else {
    Serial.print(F("VPct = ")); Serial.print(vbat); Serial.println(F("%"));
  }

  wdt_reset();
  char url[160];
  uint16_t statuscode;
  int16_t length;
  // sprintf (url, "http://data.sparkfun.com/input/%s?private_key=%s&latitude=%s&longitude=%s&hdop=%s&altitude=%s",
  //   SPARKFUN_PUBLIC_KEY, SPARKFUN_PRIVATE_KEY, current_location.latitude_c, current_location.longitude_c, current_location.hdop_c, current_location.altitude_c);
  // Make the FONA listen, we kinda need that...
//  String urlStr = "http://shortcircuit.netau.net/index.php?coord=34,112,251,167,345";
  sprintf(url,"http://shortcircuit.netau.net/index.php?sql=%s,%u",
    replybuffer, vbat);
//  urlStr.toCharArray(url,urlStr.length()); 
  Serial.print(F("Sending: ")); Serial.println(url); 
  // fonaSerial.listen();
  wdt_reset();
  uint8_t rssi = fona.getRSSI();
  if (rssi > 5) {
   // Make an attempt to turn GPRS mode on.  Sometimes the FONA gets freaked out and GPRS doesn't turn off.
   // When this happens you can't turn it on aagin, but you don't need to because it's on.  So don't sweat
   // the error case here -- GPRS could already be on -- just keep on keeping on and let HTTP_GET_start()
   // error if there's a problem with GPRS.
   if (!fona.enableGPRS(true)) {
     Serial.println(F("Failed to turn GPRS on!"));
   }
   if (fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
     while (length > 0) {
       while (fona.available()) {
         char c = fona.read();
         // Serial.write is too slow, we'll write directly to Serial register!
         loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
         UDR0 = c;
         length--;
         if (! length) break;
       }
     }
     fona.HTTP_GET_end();
   } else {
     Serial.println(F("Failed to send GPRS data!"));
   }
   if (!fona.enableGPRS(false)) {
     Serial.println(F("Failed to turn GPRS off!"));
   }
   wdt_reset();
  } else {
   Serial.println(F("Can't transmit, network signal strength is crap!"));
  }
}

