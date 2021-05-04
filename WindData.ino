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
 
 
 void setup(){
 Serial.begin(9600); 
 pinMode(2, INPUT_PULLUP); 
 attachInterrupt(0, rpm_fan, FALLING);
 }
 
 void loop(){
  if (millis() - lastmillis == 1000){ //Update every one second, this will be equal to reading frequency (Hz).
   detachInterrupt(0);//Disable interrupt when calculating
   rpm = half_revolutions * 30; // Convert frequency to RPM, note: 60 works for one interruption per full rotation. For two interrupts per full rotation use half_revolutions * 30.
   Serial.print("RPM ="); //print the word "RPM" and tab.
   Serial.print(rpm); // print the rpm value.
   Serial.print(" Hz="); //print the word "Hz".
   Serial.print(half_revolutions/2); //print revolutions per second or Hz. And print new line or enter. divide by 2 if 2 interrupts per revolution
   half_revolutions = 0; // Restart the RPM counter
   lastmillis = millis(); // Update lastmillis
   attachInterrupt(0, rpm_fan, FALLING); //enable interrupt
   mph = diameter * 3.14 * rpm * 60 / 63360; //convert to mph
   //mph = mph * 3.5; // calibration factor for anemometer accuracy, adjust as necessary
   Serial.print(" MPH="); //print the word "MPH".
   Serial.println(mph);
   
  // Wind Direction
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
  Serial.print("Wind Direction (°): ");
  Serial.println(direction);
  }
 }
 // this code will be executed every time the interrupt 0 (pin2) gets low.
 void rpm_fan(){
  half_revolutions++;
 }
