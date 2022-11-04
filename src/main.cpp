#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQ135.h>
#include <MQTT.h>
#include "secrets.h"

#define PIN_MQ135 A0 // MQ135 Analog Input Pin

const char *ssid = SECRET_WIFI_SSID;
const char *password = SECRET_WIFI_PASS;

WiFiClient net;
MQTTClient mqttClient;

MQ135 mq135_sensor = MQ135(PIN_MQ135);

float temperature = 21.0;
float humidity = 25.0;

float current_ppm = 0;

//function declarations
void connectMQTT();
void publishValue(float newValue);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password); // begin WiFi connection

    // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Connecting to MQTT broker...");
  mqttClient.begin(MQTT_BROKER_ADDR, net);
  mqttClient.setWill("air-condition-sensor/status", "offline");
  connectMQTT();
  Serial.println("Connection to MQTT brocker established");

  Serial.println("Device started!");
}

void loop() {
  if(!mqttClient.connected()){
    Serial.println("Lost connection to MQTT broker, reconnecting...");
    connectMQTT();
  }

  float ppm = mq135_sensor.getCorrectedPPM(temperature, humidity);
  Serial.print("\nMeasurment ...");
  publishValue(ppm);
  delay(1000);
}

void connectMQTT(){
  while (!mqttClient.connect("air-condition-sensor", "public", "public"))
  {
    Serial.print(".");
    delay(1000);
  }
  mqttClient.publish("air-condition-sensor/status", "online");
  mqttClient.publish("air-condition-sensor/value", String(current_ppm));
}

void publishValue(float new_ppm){
  if(current_ppm != new_ppm){
    current_ppm = new_ppm;
    Serial.print("\nCurrent PPM: ");
    Serial.print(new_ppm);
    mqttClient.publish("air-condition-sensor/value", String(new_ppm));
  }
}