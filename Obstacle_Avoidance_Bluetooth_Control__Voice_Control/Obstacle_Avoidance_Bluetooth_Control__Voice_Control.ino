#include <AFMotor.h> // Adafruit Motor Shield Library
#include <Servo.h> //Servo library
#define Echo A0 // Defining Echo pin of the ultrasonic as Analog 0
#define Trig A1 // Defining Trig pin of the ultrasonic as Analog 1
#define servoPin 10 // Defining servo pin 10
#define startingPoint 90  // Starting point for servo

Servo servo; // Servo object
AF_DCMotor M1(1); //Motor objects
AF_DCMotor M2(2);
AF_DCMotor M3(3);
AF_DCMotor M4(4);

int distance;
int Left;
int Right;
int RightWay;
int LeftWay;
char value; 


void setup() {

  Serial.begin(9600);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  servo.attach(servoPin);

  M1.setSpeed(180);  // Speed between 0-255
  M2.setSpeed(180);  // Speed between 0-255
  M3.setSpeed(180);  // Speed between 0-255
  M4.setSpeed(180);  // Speed between 0-255

}

void loop() {
  Obstacle();
  BluetoothControl();
  VoiceControl();

}
void BluetoothControl(){
  if(Serial.available()>0){ // if the value is not 0, then if stamement is true ; if the value is zero, if statement is false 
    value = Serial.read(); // It is used to read data from the serial port. Put the incoming data to the "value" variable.
    Serial.println(value); 
    switch(value){
      case 'F':
        Forward();
        break;
      case 'B':
        Backward();
        break;
      case 'L':
        TurnLeft();
        break;
      case 'R':
        TurnRight();
        break;
      case 'S':
        Stop();
        break;
      default:
        Serial.println("There is a problem in bluetooth control function");
    }
  }
}


 void VoiceControl(){
  if ( Serial.available() > 0 ) {
    value = Serial.read();
    Serial.println(value);

    if (value == '^'){
      Forward();
    }
    else if (value == '-'){
      Backward();
    }
    else if (value == '<'){
      Left = LookingLeft();
      servo.write(startingPoint);
      if(Left >= 10){
        TurnLeft();
        delay(500);
        Stop();
      }
      else if(Left < 10){
        Stop();
      }
    }
    else if (value == '>'){
      Right = LookingRight();
      servo.write(startingPoint);
      if(Right >= 10){
        TurnRight();
        delay(500);
        Stop();
      }
      else if(Right < 10){
        Stop();
      }
    }
    else if (value == '*'){
      Stop();
    }

    
  }
 }

void Obstacle(){
  distance = distCalc();
  if (distance <= 13){ 
    Stop();
    Backward();
    delay(200);
    Stop();
    Left = LookingLeft();
    servo.write(startingPoint);
    delay(500);
    Right = LookingRight();
    servo.write(startingPoint);
    if(Left < Right){
      TurnRight();
      delay(500);
      Stop();
      delay(300);
    }
    else if(Right < Left){
      TurnLeft();
      delay(500);
      Stop();
      delay(300);
    }
  }
  else{
    Forward();
  }
}

int distCalc(){

  digitalWrite(Trig,LOW); // Initialize the Trig pin 
  delayMicroseconds(5); // Microsecond is smaller than milisecond (delay)
  digitalWrite(Trig,HIGH); // Command for starting to generate sound waves or pulses
  delayMicroseconds(10);
  digitalWrite(Trig,LOW); // Not to generate new pulses we make the Trig pin passive
  long duration = pulseIn(Echo,HIGH); // pulseIn -> Reads a pulse on a pin.
  long cm = duration / 29 / 2; // Convert the duration to distance in the terms of centimer
  return cm;
}

void Stop(){
  M1.run(RELEASE);
  M2.run(RELEASE);
  M3.run(RELEASE);
  M4.run(RELEASE);
}

void Backward(){
  M1.run(BACKWARD);
  M2.run(BACKWARD);
  M3.run(BACKWARD);
  M4.run(BACKWARD);
}
void Forward(){
  M1.run(FORWARD);
  M2.run(FORWARD);
  M3.run(FORWARD);
  M4.run(FORWARD);
}

int LookingLeft(){
  servo.write(180);
  delay(500);
  RightWay = distCalc();
  return RightWay;
}

int LookingRight(){
  servo.write(20);  
  delay(500);
  LeftWay = distCalc();
  return LeftWay;
}

void TurnRight(){
  M1.run(BACKWARD);
  M2.run(BACKWARD);
  M3.run(FORWARD);
  M4.run(FORWARD);
}

void TurnLeft(){
  M1.run(FORWARD);
  M2.run(FORWARD);
  M3.run(BACKWARD);
  M4.run(BACKWARD);
}
