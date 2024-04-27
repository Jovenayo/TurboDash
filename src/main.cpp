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
/////////////////////////////////

// Stepper ///////////////////////////////////
#define StepperPins         10, 8, 9, 7 
#define StepperSteps        2048 // a boleo tu
#define StepperSpeed        5
#define SteppsForClk        5

#define StepperModeGo       0
#define StepperModeBack     1
#define StepeprModeTurn     2

#define RuedasAtrasTimer      5000
#define RuedasGirarTimer      5000
//////////////////////////////////////////////


/********* IR Variables **********/
extern IRrecv IrReceiver;

IRData IRCode;

#define IRCode_On_OFF        69

unsigned long millis_IR();



//  CLK -----------------------
unsigned long IRThisMillis = 0;
unsigned long IRLastMillis = 0;
//-----------------------------
/*****************************************/

/********* Ultrasonic Variables **********/
Ultrasonic ultrasonic(UltrasonicPins);

//  CLK ------------------------------
unsigned long UltrasonicThisMillis = 0;
unsigned long UltrasonicLastMillis = 0;
//-------------------------------------
/*****************************************/

/********* Steeper Variables **************/
Stepper stepper(StepperSteps, StepperPins);

int SteeperMode = 0;
// Ruedas CLK ----------------------------
unsigned long RuedasThisMillis = 0;
unsigned long RuedasLastMillis = 0;
//----------------------------------------
/******************************************/




/******** TASKS ********/
void IR_task();
void Ultrasonic_task();
void Stepper_task();

void RuedasAdelante();
void RuedasAtras();
void RuedasGirar();
/***********************/

uint8_t mode = MODE_SLEEP;

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IRpin);
  ultrasonic.setTimeout(UltrasonicTimeout);
  stepper.setSpeed(StepperSpeed);
  ////Debug IR
  // Serial.println("-IR info: ");
  // Serial.println("      millis_IR(): " + String(millis_IR()) + " mSeg"); 
}

void loop() {
  switch (mode)
  {
  case MODE_SLEEP:
    IR_task();
    break;
  
  case MODE_WAKE:
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
    Serial.println("\r\n************** DEBUG IR *************");
    Serial.print("\r\nComando IR: "); Serial.println(IRCode.command);
    
    switch (IRCode.command)
    {
    case IRCode_On_OFF:
      if(mode == MODE_SLEEP){
        Serial.println("Mode: WAKE");
        mode = MODE_WAKE;
      } else
      if(mode == MODE_WAKE){
        Serial.println("Mode: SLEEP");
        mode = MODE_SLEEP;
      } 
      break;

    default:
      mode = MODE_SLEEP;
      break;
    }
  }
}

void Ultrasonic_task(){
  UltrasonicThisMillis = millis();
  if(UltrasonicThisMillis - UltrasonicLastMillis >= UltrasonicClk){
    int cm = ultrasonic.read(CM);
    if(cm < 5){
      Serial.println("UltrasonicSignal [OK]");
      SteeperMode = (int)StepperModeBack;
    } else {
    Serial.print("Distance in CM: ");Serial.println(cm);
    UltrasonicLastMillis = UltrasonicThisMillis;
    }
  }
}

void Stepper_task(){
  Serial.println("\r\n************** DEBUG STEPPER *************");
  Serial.print("\r\nSteeperMode: "); 
  switch (SteeperMode)
  {
  case (int)StepperModeGo:
    Serial.println("StepperModeGo");
    RuedasAdelante();
    break;
  
  case (int)StepperModeBack:
    Serial.println("StepperModeBack");
    RuedasAtras();

  case (int)StepeprModeTurn:
    Serial.println("StepperModeTurn ::");Serial.println(SteeperMode);
    RuedasGirar();
    break;

  default:
    break;
  }
}

void RuedasAdelante(){
  stepper.step(SteppsForClk);
  RuedasLastMillis = millis();
}

void RuedasAtras(){
  RuedasThisMillis = millis();
  Serial.print("Operacion: ");Serial.print(RuedasThisMillis);Serial.print(", ");Serial.println(RuedasLastMillis);
  if(((int)RuedasThisMillis - (int)RuedasLastMillis) < RuedasAtrasTimer){
    stepper.step(-SteppsForClk);  
  } else if(((int)RuedasThisMillis - (int)RuedasLastMillis) >= RuedasAtrasTimer){
    SteeperMode = StepeprModeTurn;
    RuedasLastMillis = RuedasThisMillis;
  }
}

void RuedasGirar(){
  RuedasThisMillis = millis();
    Serial.print("Operacion: ");Serial.print(RuedasThisMillis);Serial.print(", ");Serial.println(RuedasLastMillis);
  if((int)RuedasThisMillis - (int)RuedasLastMillis < RuedasGirarTimer){
    //stepper.step(SteppsForClk);  
  } else {
    SteeperMode = StepperModeGo;
    RuedasLastMillis = RuedasThisMillis;
  }
}