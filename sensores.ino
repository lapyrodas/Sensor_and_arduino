#include <SimpleDHT.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

int UVOUT = A0; //Output from the sensor
int REF_3V3 = A1;
int pinDHT22 = 2;
SimpleDHT22 dht22(pinDHT22);
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

//funciones uv sensor
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int averageAnalogRead(int pinToRead){
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;

  for(int x = 0 ; x < numberOfReadings ; x++)
  runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return(runningValue);
}
void uv_sensor(){
  int uvLevel = averageAnalogRead(UVOUT);
  int refLevel = averageAnalogRead(REF_3V3);
  float outputVoltage = 3.3 * uvLevel/ refLevel;
  float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
  Serial.print("      ");
  Serial.print(uvLevel);
  Serial.print("        ");
}
// void create_csv(){
  // fstream fout;
  
  // fout.open('E:/PREGRADO/9/Tecnologias_geomaticas/laboratorios/3/report_sensor.csv', ios::out | ios::app);
  // fout <<'Uv_intensidad'<<'\n';
  // fout<<"mW/cm^2"<<'\n';
// }
//funciones gps

static void printFloat(float val, bool valid, int len, int prec){
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}
static void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void printInt(unsigned long val, bool valid, int len){
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t){
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}
//funciones temp-humedad
void th_sensor(){
  float temperature = 0;
  float humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 failed, err="); Serial.println(err);delay(2000);
    return;
  }
  Serial.print((float)temperature);
  Serial.print("  ");
  Serial.println((float)humidity);
  
}
//configuracion
void setup(){
  Serial.begin(115200);
  ss.begin(GPSBaud);
  pinMode(UVOUT, INPUT);
  pinMode(REF_3V3, INPUT);
  Serial.println(F(" Fecha      Hora    date   Latitud   Longitud   Altitud   uv_intensity     Temp.  Hum."));
  Serial.println(F("            UTC +0          ---*---    ---*---   --(m)--    -(mW/cm^2)--     *C     %"));
  Serial.println(F("--------------------------------------------------------------------------------------"));
  // create_csv();
}

void loop(){
  printDateTime(gps.date, gps.time);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  uv_sensor();
  th_sensor();
  Serial.println();
  smartDelay(2500);
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
  delay(2500);
  
}
