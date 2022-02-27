// Physical Electronic Musical Instrument
// Naor Zaharia 312423841, Harel Zahari 305494452

/*The instrument we choose as our inspiration is the maracas.
We based our implementation on 3 sensors:
The accelerometer was used to simulate the shake intensity of the maracas.
The left and right buttons were used to offer different “shake sound” length variation. (the default is 2 ms).
The slide button was used for turning on and off combined with an LED effect.
*/

#include <Adafruit_CircuitPlayground.h>

// Init global var
int soundArray [3] = {4274, 4588, 4920};
bool isInstrumentOn;
int ledRedColor = 0;
int ledGreenColor = 0;
int ledBlueColor = 0;
bool leftButtonPressed;
bool rightButtonPressed;
bool needInit = true;
float Z, ZPrev;

// Entry point
void setup()
{  
  Serial.begin(9600);
  CircuitPlayground.begin();  
}

// Main board loop
void loop() 
{    
  updateZLocation();    
  float value = min(abs(ZPrev-Z),5);  
  leftButtonPressed = CircuitPlayground.leftButton();
  rightButtonPressed = CircuitPlayground.rightButton();
  
  if(checkIfOn())
  {
    updateLedDelay(leftButtonPressed,rightButtonPressed);
    
    if (value > 2)
    {    
      int note = map(value, 2, 5, 0, 2);
      ledShakeGauge((note+1)*2);
            
      if (leftButtonPressed)
      {    
        CircuitPlayground.playTone(soundArray[note], 4);      
      }
      else
      {
        if (rightButtonPressed) 
        {
          CircuitPlayground.playTone(soundArray[note], 3);       
        }
        else 
        {
          CircuitPlayground.playTone(soundArray[note], 2);        
        }
      }
      
      CircuitPlayground.clearPixels();     
    }
  }  
}

// Getter for board shake delay
int getCurrentDelay()
{
    if(leftButtonPressed)
    {
      return 3;
    }
    else
    {
    if(rightButtonPressed)
    {
      return 4;
    }
    else
    {
      return 2;
    }
  }
}

// Setter board location
void setCurrentZLoaction() 
{
  int currentDelay = getCurrentDelay();
  delay(currentDelay);  
  Z = CircuitPlayground.motionZ();  
}

// Setter prev board location
void setPrevZLoaction()
{  
  ZPrev = Z; 
}

// The method init board location
void initZLoaction()
{
  setCurrentZLoaction();
  setPrevZLoaction();
  needInit=false; 
}

// The method update new board location
void updateZLocation()
{
  if(needInit)
  {
    initZLoaction();
    isInstrumentOn = CircuitPlayground.slideSwitch();
  }
  else
  {
    setPrevZLoaction(); 
    setCurrentZLoaction();
  }
}

// The method create the On/Off led effect
void ledOnOffSwitch(bool isOn)
{
  if(!isOn)
  {
    ledRedColor = 255;
    ledGreenColor = 0;
  }
  else
  {
    ledRedColor = 0;
    ledGreenColor = 255;
  }

  for(int i = 0; i < 10 ;i++)
  {  
    delay(200);
    CircuitPlayground.setPixelColor(i,ledRedColor , ledGreenColor, 0);
  }
  
  CircuitPlayground.clearPixels();
}

// The method creates the shake leds gauge effect
void ledShakeGauge(int shakeValue)
{
  for(int i = 0;i < shakeValue ;i++)
  {
    CircuitPlayground.setPixelColor(i,0x8B2FBB);
  } 
}

// The method creates the delay leds gague effect
void ledDelayGauge(int delayValue)
{
  for(int i = 0; i < delayValue ;i++)
  {
    CircuitPlayground.setPixelColor(9 - i,0xF5F557);
  } 
}

// The method checks if the instrument is On or Off, and activates the On/Off led effect 
bool checkIfOn()
{
  if(isInstrumentOn != CircuitPlayground.slideSwitch())
  {
    isInstrumentOn = CircuitPlayground.slideSwitch();
    
    if(isInstrumentOn)
    {
      ledOnOffSwitch(true);
    }
    else
    {
      ledOnOffSwitch(false);
    }
  }
    
  return isInstrumentOn;
}

// The method generates delay leds effects
void updateLedDelay(bool leftButtonPressed, bool rightButtonPressed)
{
  if(leftButtonPressed)
  {
    ledDelayGauge(4);
  }
  else
  {
    if(rightButtonPressed)
    {
      ledDelayGauge(3);
    }
    else
    {
      ledDelayGauge(2);
    }
  }
}
