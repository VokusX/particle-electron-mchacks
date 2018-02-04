// This #include statement was automatically added by the Particle IDE.
#include <HttpClient.h>

// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_DHT.h>

#include <string>

#include "cellular_hal.h"

#define DHTPIN D2
#define DHTTYPE DHT22

STARTUP(cellular_credentials_set("isp.telus.com", "", "", NULL))

string domain = '"http://mchacks-env.3pmuemxzmu.us-east-1.elasticbeanstalk.com';

int gas_digital = D0; // Digital pin reading for Gas Sensor
int gas_analog = A0; // Analog pin reading for gas sensor
int dht_5v = D7;
int gas_5v = D3;
int loudness_digital = A1;
int speaker_analog = A2;

int motion_digital = D1;

DHT dht(DHTPIN, DHTTYPE);

FuelGauge fuel;

HttpClient http;

http_hearders_t headers[] = {
    {"Content-Type", "application/json"},
    {NULL, NULL}
};

http_request_t request;
http_response_t response;

string formatJson() {
    return sprintf(
        "["
        "{'motion_detector': %d},"
        "{'gas_detector': %f},"
        "{'temperature_detector': %f},"
        "{'humidity_detector': %f},"
        "{'loudness_detector': %f},"
        "{'flood_detector': %d},"
        "{'power_detector': %d},"
        "]",
        digitalRead(motion_digital),
        analogRead(gas_analog),
        dht.getTempCelcius(),
        dht.getHumidity(),
        analogRead(loudness_digital),
        0,
        0,
        );
}


void setup() {
  Particle.keepAlive(30);
  pinMode(gas_digital, INPUT);
  pinMode(gas_analog, INPUT);
  pinMode(motion_digital, INPUT);
  pinMode(loudness_digital, INPUT);
  pinMode(dht_5v, OUTPUT);
  digitalWrite(dht_5v, HIGH);
  pinMode(gas_5v, OUTPUT);
  digitalWrite(gas_5v, HIGH);
  dht.begin();
  pinMode(speaker_analog, OUTPUT);
}

void loop() {
/*
  Serial.printlnf("testing gas %f", analogRead(gas_analog));
  Serial.printlnf("testing digital %d", digitalRead(gas_digital));
  Serial.printlnf("testing motion %d", digitalRead(motion_digital));

  delay(100);
  float h = dht.getHumidity();
  float t = dht.getTempCelcius();


  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.printlnf("Temperature %f", t);
  Serial.printlnf("Humdity %f", h);


  Serial.printlnf("testing loudness %f", analogRead(loudness_digital));
  */
  // Serial.println(fuel.getVCell());
}
