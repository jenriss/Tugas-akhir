#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Konfigurasi WiFi
const char ssid[] = "Flow";
const char password[] = "jensss889";

// Konfigurasi MQTT Broker
const char* mqtt_server = "192.168.80.105";
const int mqttPort = 1883;
const int basahValue = 205;  // Ubah sesuai dengan nilai Anda saat basah
const int keringValue = 1024;
// Pin yang terhubung ke sensor kelembaban tanah
const int soilMoisturePin = A0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, mqttPort);
}

int readSoilMoisture() {
  // Baca nilai sensor soil moisture
  int soilMoistureValue = analogRead(soilMoisturePin);

  // Lakukan kalibrasi
  int kalibrasiValue = map(soilMoistureValue, keringValue, basahValue, 0, 100);
  kalibrasiValue = constrain(kalibrasiValue, 0, 100);

  return kalibrasiValue;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int soilMoisture = readSoilMoisture(); // readSoilMoisture();
  Serial.print("Kelembaban Tanah: ");
  Serial.print(soilMoisture); // Mencetak nilai kelembaban tanah di Serial Monitor
  Serial.println("%");

  StaticJsonDocument<100> jsonDocument;
  jsonDocument["sensor_id"] = "sensor3";  // Ganti dengan ID sensor yang sesuai
  jsonDocument["soil_moisture"] = soilMoisture;

  char payload[100];
  serializeJson(jsonDocument, payload);

  client.publish("sensor/soilmoisture", payload);
  delay(5000); // Kirim data setiap 5 detik
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
    Serial.print("Menghubungkan ke MQTT Broker...");
    if (client.connect("NODE3")) {
      Serial.println("Terhubung");
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}
