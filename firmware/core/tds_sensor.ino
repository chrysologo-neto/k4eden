// Original source code: https://wiki.keyestudio.com/KS0429_keyestudio_TDS_Meter_V1.0#Test_Code
// Project details: https://RandomNerdTutorials.com/esp32-tds-water-quality-sensor/
//
// » Variables
//
int analogBuffer[SCOUNT];     // Store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;
float tds = 0;
float temperature = 25;       // Current temperature for compensation

//
// » Auxiliar Functions
//
// Median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

//
// » Read TDS sensor
//
void read_tds_CB() {
  for (int i = 0; i < SCOUNT; i++) {
    analogBuffer[i] = analogRead(TDSPIN);
    delay(30);
  }

  for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
    analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
  }
  // Read the analog value more stable by the median filtering algorithm, and convert to voltage value
  averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4096.0;

  // Temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
  // Temperature compensation
  float compensationVoltage = averageVoltage / compensationCoefficient;

  // Convert voltage value to tds value
  tds = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage) * 0.5;

  // @TODO - Create function to organize all prints on serial output

  Serial.print("TDS:");
  Serial.print(tds);
  Serial.print("mg/L   ");

  // @TODO - Add function to value Publish to MQTT topic
  client.publish("testTopic/tds", String(tds));


}
