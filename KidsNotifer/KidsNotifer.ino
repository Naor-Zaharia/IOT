//Something for someone, Blynk and Integromat
// Naor Zaharia 312423841, Harel Zahari 305494452
/*
 * We set out to build this device for our hasidic family that don't want to buy a phone for their kids but, wants to keep an eye on them. 
 * Using the device they'll be able to receive notifications about their kids. 
 * The device uses the electric charge to assign a massage. 
 * The massage is forwarded using the blink application to integromat, 
 * which sends an email to the parents with the disierd information combind with the name of the child that sent it.
*/

#define BLYNK_PRINT SerialUSB
#define BLYNK_DEBUG SerialUSB

// Consts
#define KEYTHRESHOLD 700
#define AMOUNTOFACTIVEKYES 5
#define NUMOFLEDS 10
#define AMOUNTOFACTIVECHILDRENS 5

#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <Adafruit_CircuitPlayground.h>

String msgs[]={"is home", "ate", "is taking the dog for a walk", "is going to friends", "S.O.S"};
String childrenNames[]={"Avi","Moshe","Ben","Tzvika","Kobi"};
String currentMsg;
int currentChildIndex=0;
byte capsensePins[AMOUNTOFACTIVEKYES] = {A3, A4, A1, A5, A2};

char auth[] = "CpnpDX6o4sR9SqI4Wk4StSSsLhSMeJIC";
char ssid[] = "ONEPLUS";
char pass[] = "Zahari@123";

#define EspSerial Serial1
#define ESP8266_BAUD 115200

ESP8266 wifi(&EspSerial);

long last_sent = 0;

// Entry point
void setup() {
  CircuitPlayground.begin();
  SerialUSB.begin(115200);
  EspSerial.begin(ESP8266_BAUD);
  delay(10);
  Blynk.begin(auth, wifi, ssid, pass);
  updateChildrenLed();
}

// Main board loop
void loop() {
    updateKeys();    
    checkForSwitchChild();  
}

// Switch child index if needed
void checkForSwitchChild(){
  
bool leftButtonPressed = CircuitPlayground.leftButton();
bool rightButtonPressed = CircuitPlayground.rightButton();
    
if((millis() - last_sent) >  400) {
    if (rightButtonPressed == true)
    {
      currentChildIndex = (currentChildIndex+1) % AMOUNTOFACTIVECHILDRENS;
      updateChildrenLed();
    }

    if (leftButtonPressed == true)
    {
      if(currentChildIndex==0){
        currentChildIndex = AMOUNTOFACTIVECHILDRENS - 1;
        updateChildrenLed();
      }else{
        currentChildIndex=currentChildIndex-1;
        updateChildrenLed();
      }
    }
    last_sent = millis();    
  }
}

// Update all keys
void updateKeys(){
  for(int i=0; i<AMOUNTOFACTIVEKYES;i++){
    updateKey(i);
  }
}

// The method update the relevant key
void updateKey(int index)
{
    int currentKey = CircuitPlayground.readCap(capsensePins[index]);
    if (currentKey > KEYTHRESHOLD)
    {
      currentMsg=msgs[index];
      Blynk.run();   
      Blynk.virtualWrite(V1, childrenNames[currentChildIndex]+", "+currentMsg);  
      SerialUSB.println("Email Sent");
      SerialUSB.println(childrenNames[currentChildIndex]+", "+currentMsg);
      msgSentBlink();    
    }
}

// Leds msg sent
void msgSentBlink(){
    for (int i = 0; i < NUMOFLEDS; i++)
    {
        delay(200);
        CircuitPlayground.setPixelColor(i, 0, 255, 0);
    }

    CircuitPlayground.clearPixels();
    updateChildrenLed();
}

// Set led for current child
void updateChildrenLed(){
      CircuitPlayground.clearPixels();
      CircuitPlayground.setPixelColor(currentChildIndex, 255, 0, 0);
}
