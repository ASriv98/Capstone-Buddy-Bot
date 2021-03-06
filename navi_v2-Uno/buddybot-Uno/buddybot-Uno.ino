#include "FRCmotor.h"

int gamemode = 1; // Enables the FRCmotor library

const int rightEncA = 2;
const int rightEncB = 4;
const int leftEncA = 3;
const int leftEncB = 5;

volatile int leftVel = 0;
volatile int rightVel = 0;
int lasttime;

int softstop;

//SerialRead/Write
#define BUFSIZE 256
#define SPEED_LIMIT 100


const int safesize = BUFSIZE / 2;
char buf[BUFSIZE];
char write_buffer[BUFSIZE];
int available_bytes = 0;

// Target and previous velocity arrays
int target_vel[] = {0 , 0};


//Motor Control
FRCmotor leftMotor; //DECLARE LEFT MOTOR CONTROLLER
FRCmotor rightMotor; //DECLARE RIGHT MOTOR CONTROLLER


//encoder count variables
volatile long leftCount = 0, rightCount = 0;

//Interrupt events for encoders
void rightEncoderEvent() {
  if (digitalRead(rightEncA) == HIGH) {
    if (digitalRead(rightEncB) == LOW) {
      //Serial.write("HIGH LOW");
      rightCount++;
    } else {
      //Serial.write("HIGH HIGH");
      rightCount--;
    }
  } else {
    if (digitalRead(rightEncB) == HIGH) {
      //Serial.write("LOW HIGH");
      rightCount++;
    } else {
      //Serial.write("LOW LOW");
      rightCount--;
    }
  }
  //printSerial();
}

void leftEncoderEvent() {
  if (digitalRead(leftEncA) == HIGH) {
    if (digitalRead(leftEncB) == LOW) {
      leftCount--;
    } else {
      leftCount++;
    }
  } else {
    if (digitalRead(leftEncB) == HIGH) {
      leftCount--;
    } else {
      leftCount++;
    }
  }
}


void setup() {
  leftMotor.SetPort(11); //DECLARE ARDUINO PORT FOR MOTOR CONTROLLER SIGNAL
  rightMotor.SetPort(10);
  leftMotor.Set(0); //SET INITIAL MOTOR VALUES TO ZERO
  rightMotor.Set(0); //(100 MAX FORWARD, -100 MAX BACK)

  attachInterrupt(digitalPinToInterrupt(rightEncA),rightEncoderEvent, CHANGE);
  attachInterrupt(digitalPinToInterrupt(leftEncA),leftEncoderEvent, CHANGE);

  Serial.begin(57600);
  lasttime = millis();
  softstop = millis();
  //Serial.print("Ready!");
}

void loop()
{
  readSerial();
  ramp();
  moveMotor();
  printSerial();
  //char printer[100];
  //int len = sprintf(printer,"A: %d\tb:%d\n",digitalRead(rightEncA), digitalRead(rightEncB));
  //Serial.write(printer, len);
  delay(50);
}

void readSerial()
{
  if ((available_bytes = Serial.available()))
  {
    // Read + attach null byte to read string
    int obytes = strlen(buf);
    Serial.readBytes(&buf[obytes], available_bytes);
    buf[available_bytes + obytes] = '\0';
    if (strlen(buf) > safesize) {
      memmove(buf, &buf[strlen(buf) - safesize], safesize);
      buf[safesize] = '\0';
    }
    char *s, *e;
    if ((e = strchr(buf, '\n')))
    {
      e[0] = '\0';
      if ((s = strrchr(buf, '[')))
      {
        sscanf(s, "[%d,%d]\n", &target_vel[0], &target_vel[1]);
        target_vel[0] = constrain(target_vel[0], -SPEED_LIMIT, SPEED_LIMIT);
        target_vel[1] = constrain(target_vel[1], -SPEED_LIMIT, SPEED_LIMIT);
      }
      memmove(buf, &e[1], strlen(&e[1]) + sizeof(char));
    }
    //softstop = millis();
  }
  //if(millis() - softstop > 5000){
  //Serial.print("SOFTSTOP");
  //target_vel[0] = 0;
  //target_vel[1] = 0;
  //}
}


void ramp() {
  if (millis() - lasttime > 50) {
    if (leftVel < target_vel[0]) {
      leftVel++;
    }
    if (leftVel > target_vel[0]) {
      leftVel--;
    }
    if (rightVel < target_vel[1]) {
      rightVel++;
    }
    if (rightVel > target_vel[1]) {
      rightVel--;
    }
    lasttime = millis();
  }
}

void moveMotor()
{
  leftMotor.Set(-1 * leftVel);
  rightMotor.Set(rightVel);
}

void printSerial() {
  int len = sprintf(write_buffer, "[%ld,%ld]\n", leftCount, rightCount);
  write_buffer[len + 1] = '\0';
  Serial.write(write_buffer, len + 1);
}
