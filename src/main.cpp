#include <Arduino.h>
#include "IRremote.h"
#include "Ultrasonic.h"
#include "Stepper.h"

// Modes ////////////////////////
#define MODE_SLEEP         0
#define MODE_WAKE          1
/////////////////////////////////

//  IR  //////////////////////////
#define IRpin               11
#define IRClk               5000
//////////////////////////////////

//  Ultrasonic  //////////////////
#define UltrasonicPins      12, 13
#define UltrasonicTimeout   40000UL
#define UltrasonicClk       1000
#define UltrasonicDistance  10
/////////////////////////////////

// Stepper ///////////////////////////////////
#define StepperPins1        10, 8, 9, 7
#define StepperPins2        6, 4, 5, 3      
#define StepperSteps        2048
#define StepperSpeed        (int)12
#define SteppsForClk        (int)4

#define StepperModeGo       0
#define StepperModeBack     1
#define StepperModeTurn     2

#define RuedasAtrasTimer      3000
#define RuedasGirarTimer      7000
//////////////////////////////////////////////

/********* IR Variables **********/
extern IRrecv IrReceiver;

IRData IRCode;

#define IRCode_On_OFF        69

unsigned long millis_IR();

// IR CLK ---------------------
unsigned long IRThisMillis = 0;
unsigned long IRLastMillis = 0;
//-----------------------------
/**********************************/

/********* Ultrasonic Variables **********/
Ultrasonic ultrasonic(UltrasonicPins);

// Ultrasonic CLK ---------------------
unsigned long UltrasonicThisMillis = 0;
unsigned long UltrasonicLastMillis = 0;
//-------------------------------------
/*****************************************/

/********* Steeper Variables ******************************************/
Stepper stepper1(StepperSteps, StepperPins1);
Stepper stepper2(StepperSteps, StepperPins2); // Cambiar el StepperPins

int SteeperMode = 0;
// Ruedas CLK ---------------------
unsigned long RuedasThisMillis = 0;
unsigned long RuedasLastMillis = 0;
//---------------------------------
/**********************************************************************/

/******** TASKS ********/
void IR_task();
void Ultrasonic_task();
void Stepper_task();

void RuedasAdelante();
void RuedasAtras();
void RuedasGirar();
/***********************/

uint8_t mode; 

void setup() {
  mode = MODE_SLEEP;
  Serial.begin(9600);
  IrReceiver.begin(IRpin);
  ultrasonic.setTimeout(UltrasonicTimeout);
  stepper1.setSpeed(StepperSpeed);
  stepper2.setSpeed(StepperSpeed);
  ////Debug IR
  // Serial.println("-IR info: ");
  // Serial.println("      millis_IR(): " + String(millis_IR()) + " mSeg"); 
}

void loop() {
  switch (mode)
  {
  case (int)MODE_SLEEP:
    IR_task();
    for (int pin = 3; pin <= 10; pin++) {
      digitalWrite(pin, LOW);
    }
    break;
  
  case (int)MODE_WAKE:
    IR_task();
    Ultrasonic_task();
    Stepper_task();

  default:
    IR_task();
    break;
  }
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
    IrReceiver.resume();
    //Serial.println("\r\n************** DEBUG IR *************");
    //Serial.print("\r\nComando IR: "); Serial.println(IRCode.command);
    
    switch (IRCode.command)
    {
    case (int)IRCode_On_OFF:
      if(mode == MODE_SLEEP){
        //Serial.println("Mode: WAKE");
        mode = MODE_WAKE;
      } else
      if(mode == MODE_WAKE){
        //Serial.println("Mode: SLEEP");
        mode = MODE_SLEEP;
      } 
      break;

    default:
      mode = MODE_SLEEP;
      break;
    }
  }
}

void Stepper_task(){
  //Serial.println("\r\n************** DEBUG STEPPER *************");
  //Serial.print("\r\nSteeperMode: "); 
  switch (SteeperMode)
  {
  case (int)StepperModeGo:
    //Serial.println("StepperModeGo");
    RuedasAdelante();
    break;
  
  case (int)StepperModeBack:
    //Serial.println("StepperModeBack");
    RuedasAtras();

  case (int)StepperModeTurn:
    //Serial.println("StepperModeTurn ::");Serial.println(SteeperMode);
    RuedasGirar();
    break;

  default:
    break;
  }
}

void Ultrasonic_task(){
  UltrasonicThisMillis = millis();
  if(UltrasonicThisMillis - UltrasonicLastMillis >= UltrasonicClk){
    int cm = ultrasonic.read(CM);
    if(cm < UltrasonicDistance){
      Serial.println("UltrasonicSignal [OK]");
      SteeperMode = (int)StepperModeBack;
    } else {
    Serial.print("Distance in CM: ");Serial.println(cm);
    UltrasonicLastMillis = UltrasonicThisMillis;
    }
  }
}


void RuedasAdelante(){
  stepper1.step(-SteppsForClk);
  stepper2.step(SteppsForClk);
  RuedasLastMillis = millis();
}

void RuedasAtras(){
  RuedasThisMillis = millis();
  //Serial.print("Operacion: ");Serial.print(RuedasThisMillis);Serial.print(", ");Serial.println(RuedasLastMillis);
  if(((int)RuedasThisMillis - (int)RuedasLastMillis) < (int)RuedasAtrasTimer){
    stepper1.step(SteppsForClk);  
    stepper2.step(-SteppsForClk);
  } else if(((int)RuedasThisMillis - (int)RuedasLastMillis) >= (int)RuedasAtrasTimer){
    SteeperMode = StepperModeTurn;
    RuedasLastMillis = RuedasThisMillis;
  }
}

void RuedasGirar(){
  RuedasThisMillis = millis();
  //Serial.print("Operacion: ");Serial.print(RuedasThisMillis);Serial.print(", ");Serial.println(RuedasLastMillis);
  if((int)RuedasThisMillis - (int)RuedasLastMillis < (int)RuedasGirarTimer){
    //stepper.step(SteppsForClk);  

    stepper1.step(SteppsForClk);
  } else {
    SteeperMode = StepperModeGo;
    RuedasLastMillis = RuedasThisMillis;
  }
}