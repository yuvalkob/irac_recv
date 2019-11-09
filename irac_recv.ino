

/*
 * Copyright 2009 Ken Shirriff, http://arcfn.com
 * Copyright 2017-2019 David Conran
 * Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009,
 * Copyright YuvalK 2019
 */

#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>

// ==================== USER PROPERTIES =================================

const char* ssid = "WIFI SSID";
const char* password = "WIFI PASSWORD";
const char* mqtt_server = "MQTT SERVER";
const char* mqtt_username = "MQTT USER NAME";
const char* mqtt_password = "MQTT PASSWORD";
std::string ac_name = "AC NAME";
const uint16_t kRecvPin = D2;

// ==================== start of TUNEABLE PARAMETERS ====================
std::string mqtt_prefix="/irac_recv/";

std::string mqtt_topic = (mqtt_prefix + ac_name);

const uint32_t kBaudRate = 115200;

const uint16_t kCaptureBufferSize = 1024;

#if DECODE_AC

const uint8_t kTimeout = 50;
#else   // DECODE_AC

const uint8_t kTimeout = 15;
#endif  // DECODE_AC
const uint16_t kMinUnknownSize = 12;


#define LEGACY_TIMING_INFO false
// ==================== end of TUNEABLE PARAMETERS ====================


WiFiClient espClient;
PubSubClient client(espClient);
IRrecv * irrecv = new IRrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results

stdAc::state_t prev_state;

long lastMsg = 0;
char msg[50];
int value = 0;

int last_message = 0;

void setup_wifi() {
   delay(100);
  // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  
} //end callback

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command if any
      client.subscribe("OsoyooCommand");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} //end reconnect()



void setup() {
#if defined(ESP8266)
  Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
#else  // ESP8266
  Serial.begin(kBaudRate, SERIAL_8N1);
#endif  // ESP8266

#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv->setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv->enableIRIn();  // Start the receiver

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 500) {
     lastMsg = now;
    if (irrecv->decode(&results)) 
    {
      String description = IRAcUtils::resultAcToString(&results);
      if (description.length())
      { 
        Serial.println(mqtt_topic.c_str());
        Serial.println(description);
        client.publish(mqtt_topic.c_str(), description.c_str());
     
      }
      
     }
  } 
}
