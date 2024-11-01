#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "Flow";     // WiFi name
const char* password = "jensss889";    // WiFi password

const char* mqtt_server = "192.168.80.105";
const char* topic_water_level = "sensor/ultrasonic";

const int trigPin = 32;   // Ultrasonic sensor trigger pin
const int echoPin = 33;   // Ultrasonic sensor echo pin
const int max_water_level = 100;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

float readWaterLevel() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;
  float relative_distance = max_water_level - distance;
  return relative_distance;
}

void loop() {
  float water_level = readWaterLevel();

  Serial.print("Water Level: ");
  Serial.print(water_level);
  Serial.println(" cm");

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int relative_water_level_integer = (int)water_level; // We use water_level for both Serial Monitor and MQTT

  DynamicJsonDocument jsonDoc(200);    
  jsonDoc["sensor_id"] = "sensor_ultrasonic";
  jsonDoc["relative_distance_cm"] = relative_water_level_integer;
  jsonDoc["relative_distance"] = relative_water_level_integer;
  char buffer[512];
  serializeJson(jsonDoc, buffer);

  if (client.publish(topic_water_level, buffer)) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Data sending failed");
  }

  delay(2000);
}
