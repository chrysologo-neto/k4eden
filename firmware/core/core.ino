//
// »» K4Eden - Smart Hydroponic Cannabis' Cultivation System ««
//
//  Authors:
//  - Chrysologo Neto (chrysologo-neto)
//  - Alexandre Colauto (AlexandreColauto)
//

//
// » Libraries
//
#include "DHT.h"
#include <TaskScheduler.h> // https://github.com/arkhipenko/TaskScheduler
#include "EspMQTTClient.h"

//
// » Callback Functions
//
// »» Temperature & Humidity
//
void read_temperature_humidity_CB();
Task read_temperature_humidity(1000, TASK_FOREVER, &read_temperature_humidity_CB);
//
// »» pH
//
void read_ph_CB();
Task read_ph(1000, TASK_FOREVER, &read_ph_CB);
//
// »» TDS
//
void read_tds_CB();
Task read_tds(1000, TASK_FOREVER, &read_tds_CB);

//
// » Macros & Values (See https://doc.bccnsoft.com/docs/cppreference_en/preprocessor/all.htm)
//
//
// »» Pin and Type
//
#define DHTPIN  4       // GPIO4  <-> D4
#define DHTTYPE DHT22   // DHT 22 <-> (AM2302), AM2321
#define PHPIN   35      // GPI35  <-> D35
#define TDSPIN  34      // GPIO36 <-> D36
#define VREF    3.3     // Analog reference voltage(Volt) of the ADC
#define SCOUNT  30      // Sum of sample point

//
// » Objects
//
DHT dht(DHTPIN, DHTTYPE);
Scheduler ts;
EspMQTTClient client(
  "Vodafone-DAC08B_EXT",
  "DVbwCkw5sT",
  "192.168.1.138",  // MQTT Broker server ip
  "k4eden",   // Can be omitted if not needed
  "eden",   // Can be omitted if not needed
  "TestClient",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void onConnectionEstablished()
{
  // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("testTopic", [](const String & payload) {
    Serial.println(payload);
  });


  // Publish a message to "mytopic/test"
  client.publish("testTopic", "This is a message from mosquitto on esp32"); // You can activate the retain flag by setting the third parameter to true


}
//
// » Setup
//
void setup() {
  // Configure serial
  Serial.begin(115200);

  // Start
  dht.begin();    // DHT22
  ts.init();
  //MQTT client config
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true


  // Add tasks to scheduler
  ts.addTask(read_temperature_humidity);
  ts.addTask(read_ph);
  ts.addTask(read_tds);

  // Enable tasks
  read_temperature_humidity.enable();
  read_ph.enable();
  read_tds.enable();
}

//
// » Main Loop
//
void loop() {
  client.loop();
  // Execute tasks
  ts.execute();
  delay(2000);
}
