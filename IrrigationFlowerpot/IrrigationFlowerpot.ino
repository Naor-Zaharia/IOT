/*
The device we made hold`s 10 different types of plants.
We based our implementation on 2 inputs:
The daily amount of rain (acquired using a whether website Json string).
The amount of water a plant need's.
Combining the two we make sure the plants get the daily amount of water they need, while considering the amount of rain they received in that given day.
*/

#include <Adafruit_CircuitPlayground.h>
#include <ArduinoJson.h>
#include <string.h>

#define NUMOFLEDS 10

#define TINY_GSM_MODEM_ESP8266

#define SerialMon Serial
#define SerialAT Serial1

#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 2048
#endif

#define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Your WiFi connection credentials, if applicable
const char wifiSSID[]  = "ONEPLUS";
const char wifiPass[] = "Zahari@123";

// Time in MS
unsigned long hourInMS = 3600000; // 3600000 milliseconds in an hour
unsigned long previousMillis = 0;
unsigned long totalMillis = 0;
unsigned int hoursCountInDay=0;

// Server details
const char server[] = "api.openweathermap.org";
const char resource[] = "/data/2.5/find?lat=32.164860&lon=34.844170&cnt=1&units=metric&appid=68b98bef6fdea93288ce81dfa4046c52";

float rainDailyBuffer =0;
float plantsDailyMMWaterRequiredArray[10]={5.0,7.0,9.0,11.0,13.0,20.0,30.0,40.0,50.0,100.0};

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);
const int  port = 80;

void setup() {
  CircuitPlayground.begin();
  SerialMon.begin(115200);
  delay(1000);
  SerialAT.begin(115200);  
  SerialMon.println("Initializing modem...");
  modem.restart();
  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);
}

void loop() {
  // Wifi connection parameters must be set before waiting for the network
  SerialMon.print(F("Setting SSID/password..."));
  if (!modem.networkConnect(wifiSSID, wifiPass)) {
    SerialMon.println(" fail");
    delay(5000);
    return;
  }
  SerialMon.println(" success");
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(5000);
    return;
  }
  SerialMon.println("Success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  SerialMon.print("Connecting to ");
  SerialMon.println(server);
  if (!client.connect(server, port)) {
    SerialMon.println("Failed");
    delay(10000);
    return;
  }
  SerialMon.println("Success");  
  smapleNextRainData();

  // Shutdown

  client.stop();
  SerialMon.println(F("Server disconnected"));

  modem.networkDisconnect();
  SerialMon.println(F("WiFi disconnected"));

  while (!isHourPass()) {
    // Wait for next sampling and update led gauge
    ledGaugeForPlantsIrrigation();
  }
  
}

void smapleNextRainData(){
  makeHttpRequest();
  String jsonHttpResponse = getHttpResponse();  
  parseJSONResponse(jsonHttpResponse);
}

void parseJSONResponse(String json) {
  SerialMon.println(String("parsing response JSON:\n") + json);
  SerialMon.println("******************");
  
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, json);
  JsonObject obj = doc.as<JsonObject>();

  if (error) {
    SerialMon.print(F("deserializeJson() failed: "));
    SerialMon.println(error.f_str());
  }
  else {      
    const char* cityName = obj["list"][0]["name"];
    float feelsLike = obj["list"][0]["main"]["feels_like"];
    float tempMin = obj["list"][0]["main"]["temp_min"];
    float tempMax = obj["list"][0]["main"]["temp_max"];    
    float rainInMM = obj["list"][0]["rain"]["1h"];
    SerialMon.println("Extracted via JSON:");
    SerialMon.println(String("cityName:  ") + cityName);
    SerialMon.println(String("feelsLike: ") + feelsLike);    
    SerialMon.println(String("tempMin:  ") + tempMin);
    SerialMon.println(String("tempMax: ") + tempMax);    
    SerialMon.println(String("rain: ") + rainInMM);
    SerialMon.println("******************");
    ledGetRainData();
    updateRainDataInArray(rainInMM);
  }
  
}

void updateRainDataInArray(float rainInMM){  
  rainDailyBuffer += rainInMM;
}

void makeHttpRequest(){
  // Make a HTTP GET request:
  SerialMon.println("Performing HTTP GET request...");
  client.print(String("GET ") + resource + " HTTP/1.1\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.println();  
}

String getHttpResponse(){
  //Get HTTP GET Response
  uint32_t timeout = millis();
  String httpResponse = "";
  int nestingDepth = 0;
  while (client.connected() && millis() - timeout < 1000L) {
    while (client.available()) {
      char c = client.read();
#ifdef LOG_HTTP_RESPONSE
      SerialMon.print(c);
#endif
      if (c == '{') {
        nestingDepth++;
      }
      if (nestingDepth){
        httpResponse += c;
      }
      if (c == '}') {
        nestingDepth--;
      }
      timeout = millis();
    }
  }
    
  return httpResponse;
}

bool isHourPass(){
  bool isHourPass=false;
  unsigned long currentMillis=millis();
  totalMillis += currentMillis - previousMillis;
  previousMillis = currentMillis;
  if(totalMillis >= hourInMS){
    isHourPass=true;
    hoursCountInDay++;
    
    if(hoursCountInDay==24){
      hoursCountInDay=0;
    }
    
    totalMillis=0;
  }
  
  return isHourPass;
}

void ledGetRainData()
{
    for (int i = 0; i < NUMOFLEDS; i++)
    {
        delay(200);
        CircuitPlayground.setPixelColor(i, 127,127 , 127);
    }

    CircuitPlayground.clearPixels();
}

void ledGaugeForPlantsIrrigation(){  
  for(int i=0; i<NUMOFLEDS; i++){
    if(rainDailyBuffer == 0){ // Full Plants Irrigation
       CircuitPlayground.setPixelColor(i, 07,255 , 0);
    }else{
      if(rainDailyBuffer - plantsDailyMMWaterRequiredArray[i] >= 0 ){ // No Plants Irrigation
          CircuitPlayground.setPixelColor(i, 255,0 , 0);
      }else{
          CircuitPlayground.setPixelColor(i, 255,153 , 0); // Half Plants Irrigation
      }
    }
  }
}
