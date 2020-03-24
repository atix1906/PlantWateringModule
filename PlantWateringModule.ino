/*
  PlantWateringModule.ino
  This sketch uses a ESP8266 Module and has a moisture sensor and a waterpump connected to it.
  So whenever the moisture sensor registers a lack of water, the pump will water it again.
*/

#include "EspMQTTClient.h"
#include "Constants.h"

int cnt = 0;

String moduleName = "PlantWatering"; //ToDo: replace text with this variable
float moistureValue = 0;
double moisPercent = 0;
int maxValue = 600;

int programStatus = 0;

const int waterPump = 5;
const int sensorInput = 0;
const int sensorOnOff = 4;

const int oneMinute = 60e6;
const int tenMinutes = oneMinute * 10;
const int oneHour = oneMinute * 60;  

bool debug = false;

String statusMsg = "";

EspMQTTClient client(
  GlobalConstants::ssid,
  GlobalConstants::password,
  GlobalConstants::ipAddress,  // MQTT Broker server ip
  "MQTTUsername",   // Can be omitted if not needed
  "MQTTPassword",   // Can be omitted if not needed
  "PlantWatering",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void getStatus(const String & payload) {
  if (payload == "getStatus") {
    client.publish("PlantWatering/response", "PlantWatering is alive");
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(waterPump,OUTPUT);
  pinMode(sensorInput,INPUT);
  pinMode(sensorOnOff,OUTPUT);

  if(debug){
    client.enableDebuggingMessages(); //Enable debugging messages sent to serial output
  }
  client.enableLastWillMessage("PlantWatering/status", "I am going offline",true);  // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Subscribe to "PlantWatering/test" and display received message to Serial
  client.subscribe("PlantWatering/test", [](const String & payload) {
    Serial.println(payload);
  });

  client.subscribe("PlantWatering/request", [](const String & payload) {
    getStatus(payload);
    Serial.println(payload);
  });
     
  // Publish a message to "Briefkasten/test"
  client.publish("PlantWatering/status", "PlantWatering is connected",true); // You can activate the retain flag by setting the third parameter to true
}

void goingToSleep(int sleepTime){
  yield();
  if(debug){
    Serial.println("going to deepsleep");  
  }
  delay(100);
  ESP.deepSleep(sleepTime);
  yield();
}

void startTheWaterFlow(){
  digitalWrite(waterPump,HIGH);
  if(debug){
    Serial.println("Water is flowing!!!");
  }
}

void stopTheWaterFlow(){
  digitalWrite(waterPump,LOW);
  if(debug){
    Serial.println("Pump is shut off");
  }
}

void moistureSensorOn(){
  digitalWrite(sensorOnOff,HIGH);
}

void moistureSensorOff(){
  digitalWrite(sensorOnOff,LOW);
}

void sendData(double data){
  String output = "";
  output = String(data,2);
  client.publish("PlantWatering/moisturePercentage", output); // You can activate the retain flag by setting the third parameter to true
}

void sendStatus(String data){
  client.publish("PlantWatering/waterStatus", data); // You can activate the retain flag by setting the third parameter to true
}


void loop() {
  client.loop();
  if(client.isConnected()){
    cnt = 0;
    switch(programStatus){
      case 0:   // "measure"
        moistureSensorOn();
        for(int i = 0; i<20; i++){
          moistureValue = analogRead(sensorInput);
          if (moistureValue > maxValue){
            maxValue = moistureValue;
          }  
          delay(100);
        }
        
        moistureSensorOff();      
        if(debug){
          Serial.print("\nmoisture Value: ");
          Serial.print(moistureValue);
          Serial.print("\tmax Value: ");
          Serial.println(maxValue);
        }
        programStatus = 10;
        break;
      case 10:    //"evaluate"
        moisPercent = (moistureValue/maxValue)*100.0;
        if(debug){
          Serial.print("\tProzent: ");
          Serial.print(moisPercent);
          Serial.println();
        }
        sendData(moisPercent);
        if(moisPercent < 20){
          programStatus = 40;
        } 
        else if (moisPercent >=20 && moisPercent <40){
          programStatus = 30;
        }
        else{
          if(debug){
            Serial.println("I'm good, thanks for asking!");
          }
          programStatus = 20;
        }
        break;
      case 20:    //"waterLevelHigh"
        statusMsg = "I'm good, thanks for asking!";
        sendStatus(statusMsg);
        if(debug){
          Serial.println(statusMsg);
        }
        programStatus = 70;
        break;
      case 30:    //"waterLevelMedium"
        statusMsg = "I will be thirsty soon.";
        sendStatus(statusMsg);
        if(debug){
          Serial.println(statusMsg);
        }
        programStatus = 70;
        break;
      case 40:    //"waterLevelLow"
        statusMsg = "Water...I need water....";
        sendStatus(statusMsg);
        if(debug){
          Serial.println(statusMsg);
        }
        programStatus = 50;
        break;
      case 50:    //"waterPumpStart"
          startTheWaterFlow();
          delay(10000); // 10 seconds
          programStatus = 60;
        break;
      case 60:    //"waterPumpStop"
        stopTheWaterFlow();
        programStatus = 70;
        break;
      case 70:  //"sleep"
        goingToSleep(oneHour);
        break;
      default:
      break;
    }
    
  }
  else if(cnt > 100){
    goingToSleep(tenMinutes);
  }
  if(debug){
    Serial.println(cnt);  
  }
  cnt++;
  delay(100);
}
