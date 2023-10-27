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
#define TDSPIN  36      // GPIO36 <-> D36
#define VREF    3.3     // Analog reference voltage(Volt) of the ADC
#define SCOUNT  30      // Sum of sample point

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
  ts.init();

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
  // Execute tasks
  ts.execute();
}
