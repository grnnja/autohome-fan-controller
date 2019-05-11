#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "env.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// wifi
WiFiClient espClient;
PubSubClient client(espClient);

static long timeout = 0;
char msg[50];
float temperature = 0;
volatile byte interruptCounter = 0;
bool relayState = 0;

// temperature sensor
int resolution = 9;
long temperatureWaitTime = (750/ (1 << (12-resolution)));
long temperatureRequestedTime = 0;

// pins
const int led = 13;
const int relay = 12;
const int button = 0;
const int temperatureSensorPin = 1; // 1

// function prototypes
void setupWifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();
void handleInterrupt();

OneWire oneWire(temperatureSensorPin);
DallasTemperature temperatureSensor(&oneWire);

void setup(void){
  pinMode(led, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button), handleInterrupt, RISING);
  //Serial.begin(115200);
  setupWifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  temperatureSensor.begin();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
  temperatureSensor.setWaitForConversion(false);
  temperatureSensor.setResolution(resolution);
  //Serial.println(sensors.getResolution(tempSensor), DEC);
  temperatureSensor.requestTemperatures();
  timeout = millis() + 10000;
}

void loop(void){
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();
  long now = millis();
  if ((long)(now - timeout) >= 0) {
    temperatureSensor.requestTemperatures();
    temperatureRequestedTime = millis();
    timeout = now + 60000;
    
    // delay the remaining time that the sensor takes to finish converting
    delay(MAX(temperatureRequestedTime - millis(), 0));
    //Serial.println(temperatureWaitTime);
    //Serial.print("temperature = ");
    //Serial.println(temperature);
    //Serial.print("devices: ");
    //Serial.println(temperatureSensor.getDeviceCount());
    temperature = temperatureSensor.getTempCByIndex(0);
    temperatureRequestedTime = 0;
    //snprintf (msg, 50, "hello world #%ld", value);
    //Serial.print("Publish message: ");
    //Serial.println(temperature);
    dtostrf(temperature, 3, 1, msg);
    client.publish("heater/temperature", msg); 
    //Serial.print("wifi level: ");
    //Serial.println(WiFi.RSSI());
  }
  if (interruptCounter > 0){
    // Change state only if interruptCounter has changed by odd amount
    if (interruptCounter % 2){
      //Serial.println("relay triggered");
      digitalWrite(led, relayState ? HIGH : LOW);
      client.publish("heater/fan/status", relayState ? "1" : "0");
    }
    interruptCounter = 0;
  }
}

void setupWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led, LOW);
    delay(500);
    digitalWrite(led, HIGH);
    delay(500);
  }
  //Serial.println("Connected to wifi");
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");
  // Serial.println(length);
  // for (int i = 0; i < length; i++) {
  //   Serial.print((char)payload[i]);
  // }
  //Serial.println();

  // If we get a message on heater/fan/control, update the relay
  if (strcmp(topic,"heater/fan/control") == 0) {
    //Serial.println("topic is /heater/fan");
    if ((char)payload[0] == '1'){
      digitalWrite(led, LOW);
      digitalWrite(relay, HIGH);
      relayState = 1;
      //Serial.println("relay on");
    } else if ((char)payload[0] == '0') {
      digitalWrite(led, HIGH);
      digitalWrite(relay, LOW);
      relayState = 0;
      //Serial.println("relay off");
    } else {
      client.publish("debug", "unrecognized fan command");
      }
  }
}

void reconnectMQTT() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    String clientId = "Heater Fan Controller";
    if (client.connect(clientId.c_str())) {
      //Serial.println("connected");
      client.subscribe("heater/fan/control");
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Turn relay on when button is pressed, debounce, and increment interruptCounter
// This is not done in the main loop because we can be searching for wifi 
// but still want to be able to turn on the relay with the button
void handleInterrupt() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > 200) {
    interruptCounter++;
    relayState =! relayState;
    digitalWrite(relay, relayState ? LOW : HIGH);
  }
  lastInterruptTime = interruptTime;
}
