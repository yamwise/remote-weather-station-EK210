
//Temp Sensor ---------------------------------------------------------------------------------------------
#include <SHT1x.h>
#define dataPin 5
#define clockPin 6
SHT1x sht1x(dataPin, clockPin);


// SD ----------------------------------------------------------------------------------------
#include <SPI.h>
#include <SD.h>

const int chipSelect = 10;
//Radio -----------------------------------------------------------------------------------------

 #include <RH_RF95.h>
     
 #define RFM95_CS 4
 #define RFM95_RST 7
 #define RFM95_INT 3
     
 // Change to 434.0 or other frequency, must match RX's freq!
 #define RF95_FREQ 433.0
     
 // Singleton instance of the radio driver
 RH_RF95 rf95(RFM95_CS, RFM95_INT);

 String message;

//Windy Stuff-----------------------------------------------------------------------------------------

  // diameter of anemometer
 float radius= 2.75; //inches from center pin to middle of cup
 float diameter = radius * 2; //inches from center pin to middle of cup
 float mph;
 
 // read RPM
 int half_revolutions = 0;
 int rpm = 0;
 unsigned long lastmillis = 0;

 //Wind Direction
 float direction;
 int sensorValue;
 float voltage;


void setup() {
  // Wind Speed------------------------------------------------------------------------
  pinMode(2, INPUT_PULLUP); 
 attachInterrupt(0, rpm_fan, FALLING);
  
  
  //SD------------------------------------------------------------------------
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  //Radio ------------------------------------------------------
      
      //pinMode(RFM95_RST, OUTPUT);
      digitalWrite(RFM95_RST, HIGH);
     
      while (!Serial);
      Serial.begin(9600);
      delay(100);
     
      Serial.println("Arduino LoRa TX Test!");
     
      // manual reset
      digitalWrite(RFM95_RST, LOW);
      delay(10);
      digitalWrite(RFM95_RST, HIGH);
      delay(10);
     
      while (!rf95.init()) {
        Serial.println("LoRa radio init failed");
        while (1);
      }
      Serial.println("LoRa radio init OK!");
     
      // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
      if (!rf95.setFrequency(RF95_FREQ)) {
        Serial.println("setFrequency failed");
        while (1);
      }
      Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
      
      // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
     
      // The default transmitter power is 13dBm, using PA_BOOST.
      // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
      // you can set transmitter powers from 5 to 23 dBm:
      rf95.setTxPower(23, false);
    
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

void loop() {

// Measuring Wind Speed and Direction
//Wind Speed-------------------------------------

  if (millis() - lastmillis == 5000){ //Update every one second, this will be equal to reading frequency (Hz).
   detachInterrupt(0);//Disable interrupt when calculating
   rpm = half_revolutions * 6; // Convert frequency to RPM, note: 60 works for one interruption per full rotation. For two interrupts per full rotation use half_revolutions * 30.
  /* Serial.print("RPM ="); //print the word "RPM" and tab.
   Serial.print(rpm); // print the rpm value.
   Serial.print(" Hz="); //print the word "Hz".
   Serial.print(half_revolutions/2); //print revolutions per second or Hz. And print new line or enter. divide by 2 if 2 interrupts per revolution
  */ half_revolutions = 0; // Restart the RPM counter
   lastmillis = millis(); // Update lastmillis
   attachInterrupt(0, rpm_fan, FALLING); //enable interrupt
   mph = diameter * 3.14 * rpm * 60 / 63360;
   //mph = mph * 3.5; // calibration factor for anemometer accuracy, adjust as necessary
   //Serial.print(" MPH="); //print the word "MPH".
   //Serial.println(mph);
   float windSpeed = mph;
   
  
  
  // Wind Direction------------------------------------------
  sensorValue = analogRead(A0);
  voltage = sensorValue * (5.0 / 1023.0);
  if(voltage <= 0.36){
    direction = 112.5;
  }
  else if (voltage < 0.43){
    direction = 67.5;
  }
  else if (voltage < 0.54){
    direction = 90;
  }
  else if (voltage < 0.77){
    direction = 157.5;
  }
  else if (voltage < 1.05){
    direction = 135;
  }
  else if (voltage < 1.30){
    direction = 202.5;
  }
  else if (voltage < 1.70){
    direction = 180;
  }
  else if (voltage < 2.12){
    direction = 22.5;
  }
  else if (voltage < 2.60){
    direction = 45;
  }
  else if (voltage < 3.01){
    direction = 247.5;
  }
  else if (voltage < 3.25){
    direction = 225;
  }
  else if (voltage < 3.64){
    direction = 337.5;
  }
  else if (voltage < 3.94){
    direction = 0;
  }
  else if (voltage < 4.20){
    direction = 292.5;
  }
  else if (voltage < 4.48){
    direction = 315;
  }
  else{
    direction = 270;
  }

  

// Measuring and printing the temperature and humidity vaules -------------------------
  float tempC = sht1x.readTemperatureC();
  float humidity = sht1x.readHumidity();
  
 /* Serial.print("humidity    = ");
  Serial.println(humidity);
  Serial.print("temperature = ");
  Serial.println(tempC);
  Serial.println();
*/

  
// Sending Temperature and Humidity via Radio---------------------------------------------------------
      Serial.println("Sending to rf95_server");
      // Send a message to rf95_server
      
      //char radiopacket[20] = "Hello World #      ";
      //itoa(packetnum++, radiopacket+13, 10);
      //char radiopacket[10];
      
      //Serial.print("Sending "); Serial.println(radiopacket);
      //radiopacket[10] = 0;

    //Temp String -------------------------------------------------------------------------
      char radiopacket[20] = "Temp in °C:        ";
      Serial.print("Sending "); Serial.println(radiopacket);
      radiopacket[19] = 0;
      
      Serial.println("Sending..."); delay(10);
      rf95.send((uint8_t *)radiopacket, 20);
     
      Serial.println("Waiting for packet to complete..."); delay(10);
      rf95.waitPacketSent();
     //Temp Value -------------------------------------------------------------------------
      radiopacket[20] = "                   ";
      //tempString.toCharArray(radiopacket+13, 10);
      //itoa(tempC, radiopacket, 10);
      dtostrf(tempC, 6, 2, radiopacket);
      Serial.print("Sending "); Serial.println(radiopacket);
      radiopacket[19] = 0;
      
      Serial.println("Sending..."); delay(10);
      rf95.send((uint8_t *)radiopacket, 20);
     
      Serial.println("Waiting for packet to complete..."); delay(10);
      rf95.waitPacketSent();
     
      //Humid String -------------------------------------------------------------------------
      radiopacket[20] = "Humidity in %:     ";
      message = "Humidity in %:     ";
      message.toCharArray(radiopacket, 20);
      Serial.print("Sending "); Serial.println(radiopacket);
      radiopacket[19] = 0;
      
      Serial.println("Sending..."); delay(10);
      rf95.send((uint8_t *)radiopacket, 20);
     
      Serial.println("Waiting for packet to complete..."); delay(10);
      rf95.waitPacketSent();
     //Humid Value -------------------------------------------------------------------------
      radiopacket[20] = "                   ";
      //tempString.toCharArray(radiopacket+13, 10);
      //itoa(tempC, radiopacket, 10);
      dtostrf(humidity, 6, 2, radiopacket);
      Serial.print("Sending "); Serial.println(radiopacket);
      radiopacket[19] = 0;
      
      Serial.println("Sending..."); delay(10);
      rf95.send((uint8_t *)radiopacket, 20);
     
      Serial.println("Waiting for packet to complete..."); delay(10);
      rf95.waitPacketSent();
      //WD String -------------------------------------------------------------------------
      radiopacket[20] = "Wind Direction (°):";
      message = "Wind Direction (°):";
      message.toCharArray(radiopacket, 20);
      Serial.print("Sending "); Serial.println(radiopacket);
      radiopacket[19] = 0;
      
      Serial.println("Sending..."); delay(10);
      rf95.send((uint8_t *)radiopacket, 20);
     
      Serial.println("Waiting for packet to complete..."); delay(10);
      rf95.waitPacketSent();
     //WD Value -------------------------------------------------------------------------
      radiopacket[20] = "                   ";
      //tempString.toCharArray(radiopacket+13, 10);
      //itoa(tempC, radiopacket, 10);
      dtostrf(direction, 6, 2, radiopacket);
      Serial.print("Sending "); Serial.println(radiopacket);
      radiopacket[19] = 0;
      
      Serial.println("Sending..."); delay(10);
      rf95.send((uint8_t *)radiopacket, 20);
     
      Serial.println("Waiting for packet to complete..."); delay(10);
      rf95.waitPacketSent();
      //WS String -------------------------------------------------------------------------
      radiopacket[20] = "Wind Speed in MPH: ";
      message = "Wind Speed in MPH: ";
      message.toCharArray(radiopacket, 20);
      Serial.print("Sending "); Serial.println(radiopacket);
      radiopacket[19] = 0;
      
      Serial.println("Sending..."); delay(10);
      rf95.send((uint8_t *)radiopacket, 20);
     
      Serial.println("Waiting for packet to complete..."); delay(10);
      rf95.waitPacketSent();
     //WS Value -------------------------------------------------------------------------
      radiopacket[20] = "                   ";
      //tempString.toCharArray(radiopacket+13, 10);
      //itoa(tempC, radiopacket, 10);
      dtostrf(windSpeed, 6, 2, radiopacket);
      Serial.print("Sending "); Serial.println(radiopacket);
      radiopacket[19] = 0;
      
      Serial.println("Sending..."); delay(10);
      rf95.send((uint8_t *)radiopacket, 20);
     
      Serial.println("Waiting for packet to complete..."); delay(10);
      rf95.waitPacketSent();
     /*
      // Now wait for a reply
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
     
      Serial.println("Waiting for reply..."); delay(10);
      if (rf95.waitAvailableTimeout(1000))
      { 
        // Should be a reply message for us now   
        if (rf95.recv(buf, &len))
       {
          Serial.print("Got reply: ");
          Serial.println((char*)buf);
          Serial.print("RSSI: ");
          Serial.println(rf95.lastRssi(), DEC);    
        }
        else
        {
          Serial.println("Receive failed");
        }
      }
      else
      {
        Serial.println("No reply, is there a listener around?");
      }
      delay(1000);
*/

// Storing temperature values for fancy sensor ------------------------
  
  SD.begin(chipSelect);
  File tempFile = SD.open("Data.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (tempFile) {
    tempFile.print("Temp in °C: ");
    tempFile.println(tempC);
    tempFile.print("Humidity in %: ");
    tempFile.println(humidity);
    tempFile.print("Wind Direction (°): ");
    tempFile.println(direction);
    tempFile.print("Wind Speed in MPH: ");
    tempFile.println(windSpeed);
    tempFile.println();
    tempFile.close();
    // print to the serial port too:
    Serial.println("SD Data:");
    Serial.print("Temp in °C: ");
    Serial.println(tempC);
    Serial.print("Humidity in %: ");
    Serial.println(humidity);
    Serial.print("Wind Direction (°): ");
     Serial.println(direction);
    Serial.print("Wind Speed in MPH: ");
     Serial.println(windSpeed);
     Serial.println();
     Serial.println();
     Serial.println();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening tempData.txt");
  }


// -----------------------------------------------------------------------


}
}
void rpm_fan(){
  half_revolutions++;
 }
