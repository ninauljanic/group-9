/**
 * This sketch was created to control the robot car, initialize the serials attached on the car as the raspberry pi, sensors and bluetooth module.
 * There are data exchange between the pi and the mobile application (Andriod code). 
 * 
 * @author - Nina (Version 1), Laiz (Version 1, 2 and 4) and Rema (Version 2, 3 and 4)
 * @editor - Isak: Serial3 connection with the application when the car faces an obstacle in order to prompt the user for a new command.
 * @editor - Kosara: Serial connection with the raspberry pi and the car in order to send and receive data for the Identify red object feature.
 * 
**/

#include<Smartcar.h>

/*===============================================
              Hardware initialization
  ===============================================
*/
SR04 sensorFront;
SR04 sensorBack;
Gyroscope gyro(6);
Odometer encoderLeft;
Odometer encoderRight;
Car car;

/*===============================================
            Pin numbers initialization
  ===============================================
*/
const int encoderPinL = 2; // <---- the number of the left odometers pin
const int encoderPinR = 3; // <---- the number of the right odometers pin
const int TRIGGER_PIN_F = 51; // <---- the number of the ultrasound sensor pin for the front
const int ECHO_PIN_F = 50; // <---- the number of the ultrasound sensor pin for the front
const int TRIGGER_PIN_B = 45; // <---- the number of the ultrasound sensor pin for the back
const int ECHO_PIN_B = 44; // <---- the number of the ultrasound sensor pin for the back
const int ledRight = 48; // <---- the number of the LED pin
const int ledLeft = 49; // <---- the number of the LED pin

/*===============================================
            Variables initialization
  ===============================================
*/
char input = 0; // <---- for the bluetooth connection
char output = 0; // <---- for the bluetooth connection
unsigned int tempSpeed = 0; // <---- for setting the velocity
int ledStateLeft = LOW; // <---- led state used to set the LED
int ledStateRight = LOW; // <---- led state used to set the LED
const long intervalLeft = 1000; // <---- interval to blink (milliseconds)
const long intervalRight = 1000; // <---- interval to blink (milliseconds)
const int blinkDuration = 500; // <---- number of millisecs that Led's are on - all three leds use this
unsigned long currentMillis = 0; // <---- stores the value of millis() in each iteration of loop()
unsigned long previousMillisL = 0; // <---- to store last time LED at the left side was updated
unsigned long previousMillisR = 0; // <---- to store last time LED at the right side was updated
unsigned int distanceObF = 0; // <---- 
unsigned int distanceObB = 0; // <---- 
boolean goAuto1 = false; // <---- 
boolean canDriveForward = true; // <---- to make sure the car is allowed to go forward
boolean canDriveBackward = true; // <---- 

/*===============================================
                    SETUP
  ===============================================
*/
void setup() {
  Serial3.begin(9600);
  sensorFront.attach(TRIGGER_PIN_F, ECHO_PIN_F);
  sensorBack.attach(TRIGGER_PIN_B, ECHO_PIN_B);
  gyro.attach();
  encoderLeft.attach(encoderPinL);
  encoderRight.attach(encoderPinR);
  car.begin(gyro);
  gyro.begin();
  encoderLeft.begin();
  encoderRight.begin();

  // initialize the digital pin as an output.
  pinMode(ledLeft, OUTPUT);
  pinMode(ledRight, OUTPUT);

}

/*===============================================
                    STATE
  ===============================================
*/
void loop() {

  currentMillis = millis();

  checkSerialInput(); // <---- Get input from blutooth

  modeSelection();    // <---- Get autonmous mode change (Also from bluetooth)

  /* Enter this section when in autonomous mode */
  if (goAuto1 == true) {

    if (ObstacleFront()) { //< -- Always check for obstacle and act accordingly
      turnRight();
    }

    moveCar(70, 70); // <-- Car is always moving unless the autonmous mode is off

    checkSerialInput();

  } else {

    /* Enter this section when in manual mode */
    // The car proccess the commands from user but stops incase of obstacle
    delay(1000);
    ObstacleF(); // <----- check allways

    ObstacleB();
    //Serial.print('b');
    //Serial.println(canDriveBackward);
    //Serial.print('f');
    //Serial.println(canDriveForward);
  }

}

/*===============================================
                    LIGHTS
  ===============================================
*/

/* Method to turnOn the right light */
void blinkRight() {

  if (ledStateRight == LOW) {
    if (currentMillis - previousMillisR >= intervalRight) {
      ledStateRight = HIGH;
      previousMillisR += intervalRight;
    }
  } else {
    if (currentMillis - previousMillisR >= blinkDuration) {
      ledStateRight = LOW;
      previousMillisR += blinkDuration;
    }
  }
  digitalWrite(ledRight, ledStateRight);
}


/* Method to turnOn the left light */
void blinkLeft() {

  if (ledStateLeft == LOW) {
    if (currentMillis - previousMillisL >= intervalLeft) {
      ledStateLeft = HIGH;
      previousMillisL += intervalLeft;
    }
  } else {
    if (currentMillis - previousMillisL >= blinkDuration) {
      ledStateLeft = LOW;
      previousMillisL += blinkDuration;
    }
  }
  digitalWrite(ledLeft, ledStateLeft);
}

/* Method to turn Off both lights */
void blinkOff() {
  ledStateLeft = LOW;
  ledStateRight = LOW;
  digitalWrite(ledLeft, ledStateLeft);
  digitalWrite(ledRight, ledStateRight);
}

/* Method to make both lights blink 4 times */
void blinkAlert() {
  blinkLeft();
  blinkRight();
}

/*===============================================
                    TURNS
  ===============================================
*/

/* Method to make the car turn right + blink the right light */
void turnRight() {
  blinkRight(); //First blink
  car.rotate(84); //Rotate
  blinkOff();
  delay(100);
  stopCar();
}

/* Method to make the car turn right + blink the right light */
void turnRightM() {

  blinkRight(); //First blink
  car.rotate(84); //Rotate
  blinkOff();
  delay(1000);
  stopCar();

}

/* Method to make the car turn left + blink the left light */
void turnLeftM() {

  blinkLeft(); //First blink
  car.rotate(-84); //Rotate
  blinkOff();
  delay(1000);
  stopCar();

}
/*===============================================
                    MOVEMENT
  ===============================================
*/

/* Method to make the car move given a speed */
void moveCar(int tempSpeedL, int tempSpeedR) {
  car.setMotorSpeed(tempSpeedR, tempSpeedL);
}

/* Method to make the car stop */
void stopCar() {
  car.stop();
  input = 0;      // <-- Dont listen to blutooth input anymore
  delay(100);     // < -- The API documentation requires a 100 ms delay (Thats what i understood :P )
}

/* Method to make the car go backwards */
void goBack(int tempSpeedL, int tempSpeedR) {
  car.setMotorSpeed(-(tempSpeedR), -(tempSpeedL));  //<-- Just set the speed but in reverse
}
/*===============================================
                    OBSTACLES
  ===============================================
*/

/* Check for front obstacles */
void ObstacleF() {

  if (ObstacleFront()) {
    canDriveForward = false;  // < -- If there is an obstacle in the front of the car, don't allow the car to moves forward

  } else {
    canDriveForward = true;  // < -- If there isn't an obstacle in the front of the car, allow the car to moves forward

  }
}

/* Check for back obstacles */
void ObstacleB() {

  if (ObstacleBack()) {
    canDriveBackward = false; // < -- If there is an obstacle in the back of the car, don't allow the car to moves backward

  } else {
    canDriveBackward = true; // < -- If there isn't an obstacle in the back of the car, allow the car to moves backward

  }
}

/* Checks the front sensor readings for obstacles, blink the light and stop the car */
boolean ObstacleFront() {
  distanceObF = sensorFront.getDistance();
  if (distanceObF > 0 && distanceObF < 30) {
    blinkAlert();   // <-- Make the lights blink
    stopCar();      // <-- Stop the car

    output = 'r';
    Serial3.println(output);
    
    blinkOff();     // <-- Stop blinking
    return true;
  }
  return false;
}

/* Checks the back sensor readings for obstacles, blink the light and stop the car */
boolean ObstacleBack() {
  distanceObB = sensorBack.getDistance();
  if (distanceObB > 0 && distanceObB < 30) {
    blinkAlert();   // <-- Make the lights blink
    stopCar();      // <-- Stop the car

    output = 't';
    Serial3.println(output);
    
    blinkOff();     // <-- Stop blinking
    return true;
  }
  return false;
}

/*===============================================
                    MANUAL CONTROL
  ===============================================
*/

/* Proccess the input from the bluetooth */
void goManual() {

  if (input == 'q') {       // <---- Stop
    stopCar();
  }

  if (input == 'f') {       // <----  Drive forwards
    // Perform an obstacle check before driving
    if (canDriveForward) {
      moveCar(70, 70);
    }
  }

  if (input == 'b') {     // <---- Drive backwards
    // Perform an obstacle check before driving
    if (canDriveBackward) {
      goBack(70, 70);
    }
  }
  if (input == 'l') {     // <---- Turn left (its acctualy driving left because it moves then stop :P)
    turnLeftM();
  }
  if (input == 'r') {     // <---- Turn right
    turnRightM();
  }
  if (input == 'j') {     // <---- Blink the lights for fun
    blinkAlert();
  }
}

/*===============================================
                    MODE SELECTION
  ===============================================
*/

/* Proccess the blutooth input for autonmous mode switching */
void modeSelection() {
  switch (input) {
    case 'a':         // <---- Robots will invade us :D
      goAuto1 = true;
      break;
    case 's':         // <---- STATIC no movement (Toggles the autonmous mode off)
      stopCar();
      goAuto1 = false;
      break;
//    case 'o':         // <---- Blink the alert lights if an red object is faced
//      blinkAlert();
//      break;
//    case 'w':         // <---- Stopping blink the alert lights if an red object is gone
//      blinkOff();
//      break;
    default:
      goManual();

  }
}

/*===============================================
                    BLUETOOTH
  ===============================================
*/
void checkSerialInput() {
  if (Serial3.available() > 0) { // <---- Get data only when bluetooth available
    input = Serial3.read();
  }
}

/*===============================================
                PI CONNECTION
  ===============================================
*/
void readSerial() {
  if (Serial.available() > 0) { // <---- Get data only when Serial port is available
    input = Serial.read();
  }
}