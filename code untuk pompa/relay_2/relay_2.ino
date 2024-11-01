#include <WiFi.h>
#include <PubSubClient.h>

#define RELAY_PIN1 33
#define RELAY_PIN2 32
#define RELAY_PIN3 26
#define RELAY_PIN4 27
#define RELAY_PIN5 25

// Konfigurasi WiFi
const char* ssid = "Flow";
const char* password = "jensss889";

// Konfigurasi MQTT
const char* mqtt_server = "192.168.80.105";
const char* mqtt_username = "";
const char* mqtt_password = "";
const char* mqtt_topic = "topic/perangkat3";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Mulai Serial
  Serial.begin(115200);
  while (!Serial);

  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  pinMode(RELAY_PIN3, OUTPUT);
  pinMode(RELAY_PIN4, OUTPUT);
  pinMode(RELAY_PIN5, OUTPUT);

  // Hubungkan ke jaringan WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("WiFi connected");

  // Hubungkan ke broker MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Periksa koneksi WiFi dan MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// Fungsi untuk menghubungkan kembali ke broker MQTT
void reconnect() {
  while (!client.connected()) {
    if (client.connect("node1", mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Fungsi untuk menangani pesan yang diterima
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Konversi payload menjadi string
  String data = "";
  for (int i = 0; i < length; i++) {
    data += (char)payload[i];
  }

  if (String(topic) == mqtt_topic) {
    // Cek nilai pesan yang diterima dan sesuaikan status relay
    if (data == "OFF3") {
      Serial.println("Relay 3 turned ON");
      digitalWrite(RELAY_PIN1, HIGH);
      digitalWrite(RELAY_PIN2, LOW);
    } else if (data == "ON3") {
      Serial.println("Relay 3 turned OFF");
      digitalWrite(RELAY_PIN1, LOW);
      digitalWrite(RELAY_PIN2, HIGH);
    } else if (data == "OFF4") {
      Serial.println("Relay 4 turned ON");
      digitalWrite(RELAY_PIN3, HIGH);
      digitalWrite(RELAY_PIN4, LOW);
    } else if (data == "ON4") {
      Serial.println("Relay 4 turned OFF");
      digitalWrite(RELAY_PIN3, LOW);
      digitalWrite(RELAY_PIN4, HIGH);
    } else if (data == "ALL_ON") {
      Serial.println("All relays turned ON");
      digitalWrite(RELAY_PIN1, HIGH);
      digitalWrite(RELAY_PIN2, HIGH);
      digitalWrite(RELAY_PIN3, HIGH);
      digitalWrite(RELAY_PIN4, HIGH);
    } else if (data == "ALL_OFF") {
      Serial.println("All relays turned OFF");
      digitalWrite(RELAY_PIN1, LOW);
      digitalWrite(RELAY_PIN2, LOW);
      digitalWrite(RELAY_PIN3, LOW);
      digitalWrite(RELAY_PIN4, LOW);
    } else {
      Serial.println("Unknown command");
    }
    // Update relay 5 based on the status of other relays
    updateRelay5();
  }
}

// Fungsi untuk mengupdate status relay 5 berdasarkan kondisi relay lain
void updateRelay5() {
  bool relay1State = digitalRead(RELAY_PIN1) == HIGH;
  bool relay2State = digitalRead(RELAY_PIN2) == HIGH;
  bool relay3State = digitalRead(RELAY_PIN3) == HIGH;
  bool relay4State = digitalRead(RELAY_PIN4) == HIGH;

  // Kondisi: Relay 2 dan 4 menyala, dan relay 1 dan 3 menyala
  if (relay1State && relay3State && relay2State && relay4State) {
    digitalWrite(RELAY_PIN5, HIGH);
    Serial.println("Relay 5 turned ON because relay 1, 3, 2, and 4 are ON");
  }
  // Kondisi lainnya
  else {
    digitalWrite(RELAY_PIN5, LOW);
    Serial.println("Relay 5 turned OFF");
  }
}

// Fungsi untuk memproses pesan dari server Flask
void processFlaskData(float avgData) {
  // Proses data dari Flask di sini
  if (avgData > 50) {
    // Kirim pesan MQTT untuk mematikan perangkat
    client.publish(mqtt_topic, "OFF3");
  } else {
    // Kirim pesan MQTT untuk menyalakan perangkat selama 1 menit
    client.publish(mqtt_topic, "ON3");
    delay(5000); // Tunggu 1 menit
    client.publish(mqtt_topic, "OFF3");
  }
}
