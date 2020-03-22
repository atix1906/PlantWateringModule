/*
  SimpleMQTTClient.ino
  The purpose of this exemple is to illustrate a simple handling of MQTT and Wifi connection.
  Once it connects successfully to a Wifi network and a MQTT broker, it subscribe to a topic and send a message to it.
  It will also send a message delayed 5 seconds later.
*/

#include "EspMQTTClient.h"
#include "Constants.h"

int counter = 0;
unsigned int counter2 = 0;

EspMQTTClient client(
  GlobalConstants::ssid,
  GlobalConstants::password,
  GlobalConstants::ipAddress,  // MQTT Broker server ip
  "MQTTUsername",   // Can be omitted if not needed
  "MQTTPassword",   // Can be omitted if not needed
  "PostBox",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void getStatus(const String & payload) {
  if (payload == "getStatus") {
    client.publish("Briefkasten/response", "Briefkasten is alive");
    
    //client.publish("Briefkasten/response", (unsigned char)sensorValue);
  }
}

void setup()
{
  Serial.begin(115200);

 // noInterrupts();
 // timer0_isr_init();
 // timer0_attachInterrupt(servoISR)
  // pinMode(D6  , OUTPUT);
  setOutputPins();
  //pinMode(interruptPin, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING);
  // Optionnal functionnalities of EspMQTTClient :
  //client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  //client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Subscribe to "Briefkasten/test" and display received message to Serial
  client.subscribe("Briefkasten/test", [](const String & payload) {
    Serial.println(payload);
  });

  client.subscribe("Briefkasten/request", [](const String & payload) {
    getStatus(payload);
    Serial.println(payload);
  });
     
  // Publish a message to "Briefkasten/test"
  client.publish("Briefkasten/inbox", "Briefkasten is connected"); // You can activate the retain flag by setting the third parameter to true
  
  // Execute delayed instructions
  client.executeDelayed(5 * 1000, []() {
    client.publish("Briefkasten/inbox", "Check your letterbox!");
    counter = 0;
  });

   //client.publish("Briefkasten/test", "loop",true); // You can activate the retain flag by setting the third parameter to true
}

void goingToSleep(){
  yield();
  //setOutputPins();
  setPinsLow();
  Serial.println("going to deepsleep");
  delay(100);
  ESP.deepSleep(0);
  yield();
}

void loop() {
  /*float moistureValue = analogRead(analogInput);
  if (moistureValue > maxValue){
    maxValue = moistureValue;
  }
  Serial.print("\nmoisture Value: ");
  Serial.print(moistureValue);
  Serial.print("\tmax Value: ");
  Serial.print(maxValue);
  
  double moisPercent = (moistureValue/maxValue)*100.0;
  Serial.print("\tProzent: ");
  Serial.print(moisPercent);
  if(moisPercent < 20){
    digitalWrite(ledGreen,LOW);
    digitalWrite(ledYellow,LOW);
    digitalWrite(ledRed,HIGH);
  } 
  else if (moisPercent >=20 && moisPercent <40){
    digitalWrite(ledGreen,LOW);
    digitalWrite(ledYellow,HIGH);
    digitalWrite(ledRed,LOW);
  }
  else{
    digitalWrite(ledGreen,HIGH);
    digitalWrite(ledYellow,LOW);
    digitalWrite(ledRed,LOW);
  }
  delay(1000);*/
}
