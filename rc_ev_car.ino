#include <SparkFun_TB6612.h>
#include <SoftwareSerial.h>  

//set up bluetooth connection 
const int bluetoothTx = 0;
const int bluetoothRx = 1;
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

//setup camera trigger pin
const int cameraPin = 13; 

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

int btnPin = 12; //pushbutton (not used here) 
int val = 0; 
  
void setup(){
  //establish pins used 
  pinMode(cameraPin, OUTPUT); 
  pinMode(btnPin, INPUT); 

  //Start BT and serial connections
  Serial.begin(9600); 
  bluetooth.begin(9600);
  delay(50);
  Serial.println("Bluetooth connected.");
}
  
void loop(){
  //allows car to communicate with bluetooth
  if (bluetooth.available()) {
    char call = bluetooth.read();
    Serial.println(call);
    //checks for calls to trigger the camera for photo
    if (call == 'p') {
      digitalWrite(cameraPin, HIGH);
      delay(20); 
      digitalWrite(cameraPin, LOW); 
    }
    //checks for calls to start/stop video 
    if (call == 'v') {
      digitalWrite(cameraPin, HIGH);
      delay(600); 
      digitalWrite(cameraPin, LOW); 
    }

    //checks for calls to move motors back/forth
    //basic 2 motor tank drive 
    //left motor forward: 'l'
    //left motor backward: 'm' 
    //right motor forward: 'r'
    //right motor backward: 's' 
    if (call == 'l') {
      motor1.drive(80);
    } else {
      motor1.drive(0);
    }
    if (call == 'm') {
      motor1.drive(-80);
    } else {
      motor1.drive(0);
    }
    if (call == 'r') {
      motor2.drive(75);
    } else {
      motor2.drive(0);
    }
    if (call == 's') {
      motor2.drive(-75);
    } else {
      motor2.drive(0);
    }
     
    //checks for call to brake all motors
    if (call == 'b') {
      motor1.brake(); 
      motor2.brake();
    }
  }
}
