
#define TdsSensorPin 35
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(TdsSensorPin,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  float ec = read_ec_sensor();
  float ph = read_ph_sensor();
  Serial.print("TDS Value:");
  Serial.println(ec);
  Serial.print("TDS Value:");
  Serial.println(ph);

}                  
