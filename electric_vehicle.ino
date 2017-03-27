#include <SparkFun_TB6612.h>

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
const int offsetA = 1;
const int offsetB = -1;
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

int sigPin = 10; // For sensor signal
int val_old, val_new; // Holds the returned values

int btnPin = 12; //pushbutton
int val = 0; 

//Enter distance in centimeters required to travel here: 
int DIST = 980; 
double wheelCirc = 7.1 * PI; 
int numTicks = int(8 * (DIST/wheelCirc));  
int t = 0; 
  
void setup(){
  pinMode(sigPin, INPUT); // signal pin as input
  pinMode(btnPin, INPUT); 
}
  
void loop(){
  //start when button is pressed
  while(digitalRead(btnPin) == LOW) {
  }
  delay(200);

  //start motors running 
  motor1.drive(-135);
  motor2.drive(-120);

  //count wheel encoder until value is reached 
  val_new = digitalRead(sigPin);
  val_old = val_new;
  
  while(t <= numTicks) {
    val_new = digitalRead(sigPin);
    if(val_new == LOW && val_old == HIGH) {
      t++; 
    }
    val_old = val_new; 
  }

  //stop motors at finish line 
  motor2.brake();
  delay(150); 
  motor1.brake(); 

  t = 0;
}
