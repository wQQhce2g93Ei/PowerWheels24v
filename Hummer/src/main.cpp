// =======================================================================================
//                       PowerWheels Conversion: Hummer
// =======================================================================================
//                            Written By: PatchBOTS
//                           Updated By: wQQhce2g93Ei
// =======================================================================================
//                     microcontroller  ---  Sparkfun Pro Micro (5v)
// --------------------------------------------------------------------------------------
//
//                              Arduino Pro Micro
//                            +-------------------+
//                            | [ ]TX      RAW[ ] | POWER IN
//                            | [ ]RX      GND[ ] | GND
//                        GND | [ ]GND     RST[ ] |
//                        GND | [ ]GND     VCC[ ] | MotorControllers
//             Speed 1 Jumper | [ ]2        A3[ ] |
//             Speed 2 Jumper | [ ]3~       A2[ ] |
//             Speed 3 Jumper | [ ]4        A1[ ] |
//                            | [ ]5~       A0[ ] | Accelerator
//                   NeoPixel | [ ]6~       15[ ] |
//              MCEnable_R_EN | [ ]7        14[ ] |
//              MCEnable_L_EN | [ ]8        16[ ] |
//                       RPWM | [ ]9~      ~10[ ] | LPWM
//                            +-------------------+
//
//                                   NOTES
// Forked from https://github.com/PatchBOTS/PowerWheels24v, see that for more informationa and context

#include "Arduino.h"

#define SPEED_1_PIN     2
#define SPEED_2_PIN     3
#define SPEED_3_PIN     4
#define ACCELERATOR_PIN A0
#define MOTOR_R_EN_PIN  7
#define MOTOR_L_EN_PIN  8
#define MOTOR_RPWM_PIN  9

int speed = 0;
bool acceleratorActive = false;
bool acceleratorActivePrev = false;

int currentDutyCycle = 0;

// starting duty cycle after accelerator released
int startingDutyCycle = 100;

// target power (0-255)
int targetDutyCycle = 0;

// how fast the ramp up occurs
int dutyCycleIncrement = 5;

void setup()
{
  Serial.begin(9600);

  pinMode(SPEED_1_PIN, INPUT_PULLUP);
  pinMode(SPEED_2_PIN, INPUT_PULLUP);
  pinMode(SPEED_3_PIN, INPUT_PULLUP);
  pinMode(MOTOR_R_EN_PIN, OUTPUT);
  pinMode(MOTOR_L_EN_PIN, OUTPUT);
  pinMode(MOTOR_RPWM_PIN, OUTPUT);

  // Enable the motor controller. This will need to remain high in order to work.
  digitalWrite(MOTOR_R_EN_PIN, HIGH);
  digitalWrite(MOTOR_L_EN_PIN, HIGH);
}

int getSpeed()
{
  if (digitalRead(SPEED_3_PIN) == 0)
  {
    speed = 3;
  }
  else if (digitalRead(SPEED_2_PIN) == 0)
  {
    speed = 2;
  }
  else if (digitalRead(SPEED_1_PIN) == 0)
  {
    speed = 1;
  }
  else
  {
    speed = 0;
  }

  return speed;
}

void loop()
{
  int speed = getSpeed();

  // check to see if accelerator is engaged
  // using voltage divider with 470 and 100 ohm resistors
  // this should work with 12-24 volts in the 5v operating
  // range of the pro micro
  int acceleratorValue = analogRead(ACCELERATOR_PIN);
  float acceleratorVoltage = acceleratorValue * (5.0 / 1023.0);
  acceleratorActive = acceleratorVoltage > 0.5;
  bool resetAcceleration = acceleratorActive && !acceleratorActivePrev;
  acceleratorActivePrev = acceleratorActive;

  if (!acceleratorActive)
  {
    Serial.println("Accelerator Inactive...");
    currentDutyCycle = 0;
  }
  else
  {
    if (resetAcceleration)
    {
      currentDutyCycle = startingDutyCycle;
    }

    // Set the target duty cycle based on current speed. The number is out of 255.
    // 175/255=68%
    // 215/255=85%
    // 255/255=100%
    if (speed == 0)
    {
      targetDutyCycle = 0;
    }
    else if (speed == 1)
    {
      targetDutyCycle = 175;
    }
    else if (speed == 2)
    {
      targetDutyCycle = 215;
    }
    else if (speed == 3)
    {
      targetDutyCycle = 255;
    }

    if (currentDutyCycle < targetDutyCycle)
    {
      currentDutyCycle += dutyCycleIncrement;
    }
    else if (currentDutyCycle > targetDutyCycle)
    {
      currentDutyCycle = targetDutyCycle;
    }

    Serial.print("Accelerator Voltage: ");
    Serial.print(acceleratorVoltage);
    Serial.print(", Speed: ");
    Serial.print(speed);
    Serial.print(", Accelerating: ");
    Serial.print(acceleratorActive);
    Serial.print(", Duty Cycle: ");
    Serial.println(currentDutyCycle);
  }

  analogWrite(MOTOR_RPWM_PIN, currentDutyCycle);
  delay(20);
}
