// App used: Joystick Bluetooth Commander by kas_dev 
// Code based on AndroTest_V2.ino demo sketch
// Source: http://forum.arduino.cc/index.php?topic=173246.0 
// By Mary Xu (github.com/rlxu) and Rain Sun
// Robotics SIDE Project 2016-17 (last updated 3/29/17) 

// Buttons setup:
// Button #1 controls pin D13 to take photo
// Button #2 controls pin D13 to start/stop video
// Button #3 triggers emergency braking of motors 
// Button #4 toggles datafield display rate

// Arduino pin#1 to TX BlueTooth module
// Arduino pin#0 to RX BlueTooth module
// HC05 BT board baud rate set to @9600 bps

#include "SoftwareSerial.h"
#include <SparkFun_TB6612.h> 

#define    STX          0x02
#define    ETX          0x03
#define    SLOW         750                             // Datafields refresh rate (ms)
#define    FAST         250                             // Datafields refresh rate (ms)

// Set up Bluetooth connection 
SoftwareSerial mySerial(0, 1);

//setup camera trigger pin D13
const int cameraPin = 13; 

// Set up IR Sensor for speedometer pin D10 
int sigPin = 10; 
int val_old, val_new; // Holds the returned values

// motor driver setup
#define AIN1 2
#define BIN1 7
#define AIN2 4
#define BIN2 8
#define PWMA 5
#define PWMB 6
#define STBY 9

// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int offsetA = -1;
const int offsetB = 1;
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

byte cmd[8] = {0, 0, 0, 0, 0, 0, 0, 0};                 // bytes received
byte buttonStatus = 0;                                  // first Byte sent to Android device
long previousMillis = 0;                                // will store last time Buttons status was updated
long sendInterval = SLOW;                               // interval between Buttons status transmission (milliseconds)
String displayStatus = "xxxx";                          // message to Android device

void setup()  {
  Serial.begin(9600);
  mySerial.begin(9600);                                // 57600 = max value for softserial
  pinMode(cameraPin, OUTPUT);   
  pinMode(sigPin, INPUT);   
  while(mySerial.available())  mySerial.read();         // empty RX buffer
}

void loop() {
  if(mySerial.available())  {                           // data received from smartphone
    delay(2);
    cmd[0] =  mySerial.read();  
    if(cmd[0] == STX)  {
      int i=1;      
      while(mySerial.available())  {
        delay(1);
        cmd[i] = mySerial.read();
        if(cmd[i]>127 || i>7)                 break;     // Communication error
        if((cmd[i]==ETX) && (i==2 || i==7))   break;     // Button or Joystick data
        i++;
      }
      if     (i==2)          getButtonState(cmd[1]);    // 3 Bytes  ex: < STX "C" ETX >
      else if(i==7)          getJoystickState(cmd);     // 6 Bytes  ex: < STX "200" "180" ETX >
    }
  } 
  sendBlueToothData(); 
}

void sendBlueToothData()  {
  static long previousMillis = 0;                             
  long currentMillis = millis();
  if(currentMillis - previousMillis > sendInterval) {   // send data back to smartphone
    previousMillis = currentMillis; 

// Data frame transmitted back from Arduino to Android device:
// < 0X02   Buttons state   0X01   DataField#1   0x04   DataField#2   0x05   DataField#3    0x03 >  
// < 0X02      "01011"      0X01     "120.00"    0x04     "-4500"     0x05  "Motor enabled" 0x03 >    // example

    mySerial.print((char)STX);                                             // Start of Transmission
    mySerial.print(getButtonStatusString());  mySerial.print((char)0x1);   // buttons status feedback
    mySerial.print(GetdataInt1());            mySerial.print((char)0x4);   // datafield #1
    mySerial.print(displayStatus);            mySerial.print((char)0x5);   // datafield #2
    // mySerial.print(GetdataFloat2());                                       // datafield #3
    mySerial.print((char)ETX);                                             // End of Transmission
  }  
}

String getButtonStatusString()  {
  String bStatus = "";
  for(int i=0; i<6; i++)  {
    if(buttonStatus & (B100000 >>i))      bStatus += "1";
    else                                  bStatus += "0";
  }
  return bStatus;
}

// function for the speedometer (rpm) using IR Sensor on wheel
// displays in data field 1, updates every 5 revs  
int GetdataInt1()  {           
  int numTicks = 0; 
  int rev = 0; 
  long currTime = millis(); 
  val_new = digitalRead(sigPin);
  val_old = val_new;
  
  while(rev < 5) {
    val_new = digitalRead(sigPin);
    if(val_new == LOW && val_old == HIGH) {
      numTicks++; 
      rev = numTicks/8; 
    }
    val_old = val_new; 
  }

  double t = (millis() - currTime)/60000; 
  int rpm = rev/t; 
  
  return rpm; 
}

/* float GetdataFloat2()  {           // Data dummy values sent to Android device for demo purpose
  static float i=50;               // Replace with your own code
  i-=.5;
  if(i <-50)    i = 50;
  return i;  
} */

void getJoystickState(byte data[8])    {
  int joyX = (data[1]-48)*100 + (data[2]-48)*10 + (data[3]-48);       // obtain the Int from the ASCII representation
  int joyY = (data[4]-48)*100 + (data[5]-48)*10 + (data[6]-48);
  joyX = joyX - 200;                                                  // Offset to avoid
  joyY = joyY - 200;                                                  // transmitting negative numbers

  if(joyX<-100 || joyX>100 || joyY<-100 || joyY>100)     return;      // commmunication error
  
  // Joystick motor controls: 2 motor rear drive, no front steering, controlled in arcade mode
  int leftMotor = joyY + joyX; 
  int rightMotor = joyY - joyX; 
  motor1.drive(leftMotor); 
  motor2.drive(rightMotor); 
  
  Serial.print("Joystick position:  ");
  Serial.print(joyX);  
  Serial.print(", ");  
  Serial.println(joyY); 
}

void getButtonState(int bStatus)  {
  switch (bStatus) {
// -----------------  BUTTON #1: Picture  -----------------------
    case 'A':          // configured as momentary button
      // buttonStatus |= B000001;        // ON
      Serial.println("\n** Button_1: ++ pushed ++ **");    
      displayStatus = "Take Picture";
      Serial.println(displayStatus);
      digitalWrite(cameraPin, HIGH);
      delay(2); 
      digitalWrite(cameraPin, LOW); 
      delay(500); 
      break;
    /* case 'B':
      buttonStatus &= B111110;        // OFF
      Serial.println("\n** Button_1: OFF **");
      // your code...      
      displayStatus = "LED <OFF>";
      Serial.println(displayStatus);
      digitalWrite(ledPin, LOW);
      break; */

// -----------------  BUTTON #2: Video  -----------------------
    case 'C':
      buttonStatus |= B000010;        // ON
      Serial.println("\n** Button_2: ON **");   
      displayStatus = "Video Started";
      Serial.println(displayStatus);
      digitalWrite(cameraPin, HIGH);
      delay(500); 
      digitalWrite(cameraPin, LOW); 
      delay(500);
      break;
    case 'D':
      buttonStatus &= B111101;        // OFF
      Serial.println("\n** Button_2: OFF **");    
      displayStatus = "Video Stopped";
      Serial.println(displayStatus);
      digitalWrite(cameraPin, HIGH);
      delay(500); 
      digitalWrite(cameraPin, LOW); 
      delay(500); 
      break;

// -----------------  BUTTON #3: Brake  -----------------------
    case 'E':          // configured as momentary button
      // buttonStatus |= B000100;        // ON
      Serial.println("\n** Button_3: ++ pushed ++ **");   
      displayStatus = "Brake initiated";
      Serial.println(displayStatus);
      motor1.brake(); 
      motor2.brake();
      break;
    /* case 'F':
      buttonStatus &= B111011;      // OFF
      Serial.println("\n** Button_3: OFF **");
      // your code...      
      displayStatus = "Motor #1 stopped";
      Serial.println(displayStatus);
      break; */

// -----------------  BUTTON #4  -----------------------
    case 'G':
      buttonStatus |= B001000;       // ON
      Serial.println("\n** Button_4: ON **");   
      displayStatus = "Datafield update <FAST>";
      Serial.println(displayStatus);
      sendInterval = FAST;
      break;
    case 'H':
      buttonStatus &= B110111;    // OFF
      Serial.println("\n** Button_4: OFF **");      
      displayStatus = "Datafield update <SLOW>";
      Serial.println(displayStatus);
      sendInterval = SLOW;
     break;

/* -----------------  BUTTON #5  -----------------------
    case 'I':           // configured as momentary button
//      buttonStatus |= B010000;        // ON
      Serial.println("\n** Button_5: ++ pushed ++ **");
      // your code...      
      displayStatus = "Button5: <pushed>";
      break;
//   case 'J':
//     buttonStatus &= B101111;        // OFF
//     // your code...      
//     break;

// -----------------  BUTTON #6  -----------------------
    case 'K':
      buttonStatus |= B100000;        // ON
      Serial.println("\n** Button_6: ON **");
      // your code...      
       displayStatus = "Button6 <ON>"; // Demo text message
     break;
    case 'L':
      buttonStatus &= B011111;        // OFF
      Serial.println("\n** Button_6: OFF **");
      // your code...      
      displayStatus = "Button6 <OFF>";
      break;
// ---------------------------------------------------------------*/
  }
}

