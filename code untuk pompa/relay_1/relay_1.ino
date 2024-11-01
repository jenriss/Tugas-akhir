#include <WiFi.h>
#include <PubSubClient.h>

#define RELAY_PIN1 32
#define RELAY_PIN2 33

// WiFi Configuration
const char* ssid = "Flow";
const char* password = "jensss889";

// MQTT Configuration
const char* mqtt_server = "192.168.80.105";
const char* mqtt_username = "";
const char* mqtt_password = "";
const char* mqtt_topic1 = "topic/perangkat1";
const char* mqtt_topic3 = "topic/perangkat3";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Start Serial
  Serial.begin(9600);
  while (!Serial);

  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  digitalWrite(RELAY_PIN1, LOW);  // Ensure relays are off initially
  digitalWrite(RELAY_PIN2, LOW);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");

  // Connect to MQTT Broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  reconnect();  // Initial MQTT connection
}

void loop() {
  // Check WiFi and MQTT connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// Function to reconnect to the MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic1);
      client.subscribe(mqtt_topic3);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Function to handle incoming messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Convert payload to string
  String data = "";
  for (unsigned int i = 0; i < length; i++) {
    data += (char)payload[i];
  }
  Serial.println(data);

  if (String(topic) == mqtt_topic1) {
    if (data == "ON1") {
      Serial.println("Relay 1 turned ON");
      digitalWrite(RELAY_PIN1, HIGH);
    } else if (data == "OFF1") {
      Serial.println("Relay 1 turned OFF");
      digitalWrite(RELAY_PIN1, LOW);
    }
  } else if (String(topic) == mqtt_topic3) {
    if (data == "ON3") {
      Serial.println("Relay 2 turned ON");
      digitalWrite(RELAY_PIN2, HIGH);
    } else if (data == "OFF3") {
      Serial.println("Relay 2 turned OFF");
      digitalWrite(RELAY_PIN2, LOW);
    } else if (data == "ON4") {
      Serial.println("Relay 2 turned ON");
      digitalWrite(RELAY_PIN2, HIGH);
    } else if (data == "OFF4") {
      Serial.println("Relay 2 turned OFF");
      digitalWrite(RELAY_PIN2, LOW);
    } else {
      Serial.println("Unknown command");
    }
  }
}
