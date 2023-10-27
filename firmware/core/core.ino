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

//
// » Callback Functions
//
// »» Temperature & Humidity 
//
void read_temperature_humidity_CB();
Task read_temperature_humidity(1000, TASK_FOREVER, &read_temperature_humidity_CB);

//
// » Macros & Values (See https://doc.bccnsoft.com/docs/cppreference_en/preprocessor/all.htm)
//
//
// »» DHT22 - Pin and Type
//
#define DHTPIN 4        // GPIO4 <-> D4
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

//
// » Objects
//
DHT dht(DHTPIN, DHTTYPE);
Scheduler ts;

//
// » Setup
//
void setup(){
  // Configure serial
  Serial.begin(115200); 

  // Start
  dht.begin();    // DHT22
  ts.init();      // Task Scheduler

  // Add tasks to scheduler
  ts.addTask(read_temperature_humidity);

  // Enable tasks
   read_temperature_humidity.enable();
}

//
// » Main Loop
//
void loop() {
  // Execute tasks
  ts.execute();
}
