# PlantWateringModule
This sketch uses a ESP8266 Module and has a moisture sensor and a waterpump connected to it.
So whenever the moisture sensor registers a lack of water, the pump will water it again.

Note:
Using <b>mosquitto_sub -h [broker ip] -t PlantWatering/# -F "%I %t %p"</b> on pi
