/*  Node-RED and ESP32 Make a Terrarium Controller
 * 
 * This code is part of a course from Tech Explorations.
 * For information about this course, please see
 * 
 * https://techexplorations.com/so/nodered/
 * 
 * For information on hardware components and the wiring needed to 
 * run this sketch, please see the relevant lecture in the course.
 * 
 *  
 *  Created by Peter Dalmaris
 * 
 */


// This version introduced the Task scheduler and the pump safety cuttoff.

#include "EspMQTTClient.h"
#include "esp32_secrets.h"
#include <TaskScheduler.h>  // See https://github.com/arkhipenko/TaskScheduler

void report_soil_humidity_CB();
Task report_soil_humidity(1000, TASK_FOREVER, &report_soil_humidity_CB);

void pump_off_CB();
Task turn_pump_off(5000, TASK_FOREVER, &pump_off_CB);

void report_voltages_CB();
Task report_voltages(2500, TASK_FOREVER, &report_voltages_CB);

Scheduler runner;

const float   input_voltage_max               = 9.0; // In Volts, max voltage 
const int     R1                              = 302; // Ohms, in kOhm
const int     R2                              = 197; // Ohms, in kOhm
const float   correction_factor               = 0.95; //1.62; // Helps improve the accuracy of the voltage calculation
const float   max_voltage_on_analog_input     = 3.6; //2.49;
const byte    motor_voltage_sensor_pin        = 34;

const float   mcu_battery_max                 = 4.7;
const int     R1_mcu                          = 219; // Ohms, in kOhm
const int     R2_mcu                          = 415; // Ohms, in kOhm
const float   correction_factor_mcu           = 1.0; //1.62; // Helps improve the accuracy of the voltage calculation
const float   max_voltage_on_analog_input_mcu = 3.1; // calculated using the voltage divider calculator
const byte    mcu_voltage_sensor_pin          = 33;

int           soil_humidity_threshold         = 400; // This is the initial value for the threshold. Actual value set via MQTT.

EspMQTTClient client(
  SECRET_SSID,
  SECRET_PASS,
  BROKER_IP,        // MQTT Broker server ip
  BROKER_USERNAME,  // Can be omitted if not needed
  BROKER_PASSWORD,  // Can be omitted if not needed
  CLIENT_NAME,      // Client name that uniquely identify your device
  BROKER_PORT       // The MQTT port, default to 1883. this line can be omitted
);

const byte  soil_humidity_pin           = 35; // GPIO
const byte  pump_pin                    = 32; // GPIO

String      soil_humidity_topic         = "hydroroot/soil-humidity";
String      pump_control_topic          = "hydroroot/pump-control";
String      motor_voltage_topic         = "hydroroot/motor-voltage";
String      mcu_voltage_topic           = "hydroroot/mcu-voltage";
String      get_soil_humidity_threshold = "hydroroot/soil_humidity_threshold";

int         soil_humidity_error_range   = 10; // Updates to the broker will be sent only
// if new soil humidity differs from last
// soil humidity by soil_humidity_error_range.
int         current_soil_humidity;
int         last_soil_humidity          = -1;        // The initial value.

void setup()
{
  analogReadResolution(10);
  pinMode(soil_humidity_pin, INPUT);
  pinMode(pump_pin, OUTPUT);
  Serial.begin(115200);

  // Optionnal functionnalities of EspMQTTClient :
  //client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  //client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").

  runner.init();
  runner.addTask(report_soil_humidity);
  report_soil_humidity.enable();
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  client.subscribe(pump_control_topic, [](const String & pump_state) {
    pump_control(pump_state);
  });

  client.subscribe(get_soil_humidity_threshold, [](const String & threshold) {
    set_threshold(threshold);
  });
}

void loop()
{
  client.loop(); // must be called once per loop.
  runner.execute();
  //  report_soil_humidity();
  //  delay(1000);    // No longer needed as we are using the scheduler.
}

void report_soil_humidity_CB()  // This function formerly named 'report_soil_humidity'. 'CB' stands for CallBack
{
  current_soil_humidity = analogRead(soil_humidity_pin);
  Serial.print("Current: ");
  Serial.print(current_soil_humidity);
  Serial.print(", last: ");
  Serial.print(last_soil_humidity);
  Serial.print(", diff: ");
  Serial.print(abs(current_soil_humidity - last_soil_humidity));

  // Is the current soil humidity different enough from the last one to do an update?
  if ( abs(current_soil_humidity - last_soil_humidity) > soil_humidity_error_range  || current_soil_humidity > soil_humidity_threshold)
  {
    Serial.print(", Sending... ");
    Serial.println(analogRead(soil_humidity_pin));

    client.publish(soil_humidity_topic, String(analogRead(soil_humidity_pin)) + "," + CLIENT_NAME);
    last_soil_humidity = current_soil_humidity;
    Serial.println();
  }
  Serial.println();
}

void set_threshold(String threshold)
{
  soil_humidity_threshold = threshold.toInt();
  Serial.print("Threshold: ");
  Serial.println(soil_humidity_threshold);
}

void pump_control(String pump_state)
{
  Serial.println(pump_state);
  if (pump_state[0] == '0')
  {
    Serial.println("Stop pump");
    digitalWrite(pump_pin, HIGH);
  } else if (pump_state[0] == '1')
  {
    Serial.println("Start pump");
    digitalWrite(pump_pin, LOW);
    turn_pump_off.enableDelayed(5000);
  }
}

void report_voltages_CB()   // Formerly titled 'report_voltages'
{
  float motor_voltage = motor_battery_voltage();
  float mcu_voltage   = mcu_battery_voltage();

  Serial.print("Motor voltage: ");
  Serial.print(motor_voltage);
  Serial.print(", MCU voltage: ");
  Serial.println(mcu_voltage);

  client.publish(motor_voltage_topic, String(motor_voltage, 1));
  client.publish(mcu_voltage_topic,   String(mcu_voltage, 1));
}

float motor_battery_voltage()
{
  // read the input on analog pin 0:
  int sensorValue = analogRead(motor_voltage_sensor_pin);
  // print out the value you read:
  Serial.print("Motor Voltage sensor raw: ");
  Serial.print(sensorValue);
  float battery_voltage = (sensorValue * max_voltage_on_analog_input * correction_factor / 1023) * (R1 + R2) / R2;
  Serial.print(", voltage = ");
  Serial.println(battery_voltage);
  return battery_voltage;
}

float mcu_battery_voltage()
{
  // read the input on analog pin 0:
  int sensorValue_mcu = analogRead(mcu_voltage_sensor_pin);
  // print out the value you read:
  Serial.print("MCU Voltage sensor raw: ");
  Serial.print(sensorValue_mcu);
  float mcu_battery_voltage = (sensorValue_mcu * max_voltage_on_analog_input_mcu * correction_factor_mcu / 1023) * (R1_mcu + R2_mcu) / R2_mcu;
  Serial.print(", voltage = ");
  Serial.println(mcu_battery_voltage);
  return mcu_battery_voltage;
}

void pump_off_CB()
{
  Serial.println("Stop pump CB");
  digitalWrite(pump_pin, LOW);
  turn_pump_off.disable();
}
