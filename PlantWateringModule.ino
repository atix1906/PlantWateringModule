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

const int waterPump = 5;
const int sensorInput = 0;
const int sensorOnOff = 4;

bool waterGo = false;
bool debug = true;

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
  
  // Execute delayed instructions
  /*client.executeDelayed(5 * 1000, []() {
    client.publish("PlantWatering/status", "Not sure what to say yet!");
    counter = 0;
  });*/

   //client.publish("PlantWatering/test", "loop",true); // You can activate the retain flag by setting the third parameter to true
}

void goingToSleep(){
  yield();
  //setOutputPins();
  
  Serial.println("going to deepsleep");
  delay(100);
  ESP.deepSleep(20e6);
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
  client.publish("PlantWatering/data", output); // You can activate the retain flag by setting the third parameter to true
}


void loop() {
  client.loop();
  if(debug){
    Serial.println(cnt);
  }
  if(cnt < 30){
    moistureSensorOn();
    moistureValue = analogRead(sensorInput);
    if (moistureValue > maxValue){
      maxValue = moistureValue;
    }
    if(debug){
      Serial.print("\nmoisture Value: ");
      Serial.print(moistureValue);
      Serial.print("\tmax Value: ");
      Serial.println(maxValue);
    }
  }
  else{
    moistureSensorOff();
    moisPercent = (moistureValue/maxValue)*100.0;
    if(debug){
      Serial.print("\tProzent: ");
      Serial.print(moisPercent);
      Serial.println();
    }
    sendData(moisPercent);
    if(moisPercent < 20){
      if(debug){
        Serial.println("Water...I need water....");
      }
      waterGo = true;
    } 
    else if (moisPercent >=20 && moisPercent <40){
      if(debug){
        Serial.println("I will be thirsty soon.");
      }
    }
    else{
      if(debug){
        Serial.println("I'm good, thanks for asking!");
      }
    }
  }

  if(waterGo){
    if(cnt < 80){
      startTheWaterFlow();
    }
    else{
      stopTheWaterFlow();
    }
  }

  if(cnt > 110){
    goingToSleep();
  }
  cnt++;
  delay(100);
}
