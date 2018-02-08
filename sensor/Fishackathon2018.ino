
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

Adafruit_BME280 bme; // I2C

SoftwareSerial mySerial(5, 4); // RX, TX for UnaBiz V2S

SoftwareSerial gps(8, 9); // TRX for GPS
TinyGPSPlus solve;

SoftwareSerial color(10, 11);
#define ASKCLR  6

static float Lat = 25.0500643;
static float Lng = 121.5312909;
static unsigned char clr[3];
static float temp; 
static char buff[24];
static int ptr;
static char ans;

void setup() {
  // put your setup code here, to run once:
  pinMode(ASKCLR, OUTPUT);
  digitalWrite(ASKCLR, HIGH);

  Serial.begin(9600); 

  if (!bme.begin(0x76)) {  ////  NOTE: Must use 0x76 for UnaShield V2S
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  mySerial.begin(9600);
  gps.begin(9600);
  color.begin(9600);

  mySerial.write("AT"); //wake the module up
  mySerial.write(10); //send chr(10) as the finish of current command
  smartDelay(200);

  clr[0] = 0x70;
  clr[1] = 0x70;
  clr[2] = 0x30;
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  
  // Always GPS first
  gps.listen();
  do
  {
    while (gps.available() > 0)
      solve.encode(gps.read());
  } while (millis() - start < ms);
}

void loop() {
  // put your main code here, to run repeatedly:
  mySerial.listen();
  while (mySerial.available() > 0)
  {
    ans = mySerial.read();
  }
  
  if(solve.location.isValid())
  {
    Lat = solve.location.lat();
    Lng = solve.location.lng();
  }

  if( Serial.available() > 0 )
    ans = Serial.read();

  if(ans == 'W')
  {
    temp = bme.readTemperature();

    ptr = 0;
    digitalWrite(ASKCLR, LOW);
    smartDelay(5);
    color.listen();
    while( color.available() > 0 )
    {
      color.listen();
      clr[ptr++] = color.read();
      smartDelay(1);
    }
    digitalWrite(ASKCLR, HIGH);

    String msg = "";
    
    dtostrf(Lat * 100000, 8, 0, buff);
    msg = String(buff);
    msg.replace(" ", "0");
    msg.replace("-", "ff");
    dtostrf(Lng * 100000, 8, 0, buff);
    msg += String(buff);
    msg.replace(" ", "0");
    msg.replace("-", "ff");

    msg += String((int) round(temp), HEX);
    msg.replace(" ", "0");

    msg += String(clr[0], HEX);
    msg.replace(" ", "0");
    msg += String(clr[1], HEX);
    msg.replace(" ", "0");
    msg += String(clr[2], HEX);
    msg.replace(" ", "0");

    Serial.print(Lat, 5); Serial.print(",");
    Serial.print(Lng, 5); Serial.print(",");
    Serial.print( (int) round(temp) ); Serial.print(",");
    Serial.print(clr[0]); Serial.print(",");
    Serial.print(clr[1]); Serial.print(",");
    Serial.println(clr[2]);
    
    SendSigfoxMessage(msg);
  }

  smartDelay(2000);
}

//void SendSigfoxMessage(long intToSigfox)
void SendSigfoxMessage(String srcIntStr)
{
  if (srcIntStr.length() > 24)
    srcIntStr = srcIntStr.substring(0, 23);

  smartDelay(100);
  mySerial.listen();
  mySerial.write("AT$P=0");
  mySerial.write(10);
  smartDelay(100);
  mySerial.listen();
  mySerial.write("AT$GI?");
  mySerial.write(10);
  smartDelay(100);
  mySerial.listen();
  mySerial.write("AT$RC");
  mySerial.write(10);

  smartDelay(100);
  mySerial.listen();
  mySerial.write("AT$SF=");
  mySerial.print(srcIntStr);
  mySerial.write(10);
}
