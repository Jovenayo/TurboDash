#include <Arduino.h>
#include "IRremote.h"
#include "Ultrasonic.h"

/******* DEFINES *******/
//  IR  //////////////////////////
#define IRpin               11
#define IRClk               5000
//////////////////////////////////

//  Ultrasonic  //////////////////
#define UltrasonicPins      12, 13
#define UltrasonicTimeout   40000UL
#define UltrasonicClk       5000
/////////////////////////////////
/***********************/

/********* IR Variables **********/
extern IRrecv IrReceiver;

IRData IRCode;
unsigned long millis_IR();

//  CLK ///////////////////////
unsigned long IRThisMillis = 0;
unsigned long IRLastMillis = 0;
///////////////////////////////
/*****************************************/

/********* Ultrasonic Variables **********/
Ultrasonic ultrasonic(UltrasonicPins);

//  CLK ///////////////////////////////
unsigned long UltrasonicThisMillis = 0;
unsigned long UltrasonicLastMillis = 0;
///////////////////////////////////////
/*****************************************/

/******** TASKS ********/
void IR_task();
void Ultrasonic_task();
/***********************/

void setup() {



  Serial.begin(9600);
  IrReceiver.begin(IRpin);
  ultrasonic.setTimeout(UltrasonicTimeout);

  ////Debug IR
  // Serial.println("-IR info: ");
  // Serial.println("      millis_IR(): " + String(millis_IR()) + " mSeg"); 
}

void loop() {
  //while (1);
  IR_task();
  Ultrasonic_task();
}

unsigned long millis_IR(){
  unsigned long  startMillis = millis();
  while(!IrReceiver.decode());
  unsigned long finishMillis = millis();
  return (finishMillis-startMillis);
}

void IR_task(){
  if(IrReceiver.decode()){
    IRCode = IrReceiver.decodedIRData;
    if(IRCode.protocol == UNKNOWN){
      IrReceiver.resume();
      return;
    }
    Serial.print("IRcode: ");
    Serial.println(IRCode.command, HEX);
    IrReceiver.resume();
  }
}

void Ultrasonic_task(){
  UltrasonicThisMillis = millis();
  if(UltrasonicThisMillis - UltrasonicLastMillis >= UltrasonicClk){
  Serial.print("Distance in CM: ");
  Serial.println(ultrasonic.read(CM));
  UltrasonicLastMillis = UltrasonicThisMillis;
  }
}

