//
// » Read Temperature & Humidity
//
void read_temperature_humidity_CB() {
  // Wait a few seconds between measurements.
  

  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.print(F("Temperature/Humidity read error"));
    // @TODO - Add function to value Publish to MQTT topic
    return;
  }

  // @TODO - Create function to organize all prints on serial output

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));

  // @TODO - Add function to value Publish to MQTT topic
   client.publish("testTopic/temperature", String(t));
}
