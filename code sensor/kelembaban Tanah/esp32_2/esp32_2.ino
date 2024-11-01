#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char ssid[] = "Flow";
const char password[] = "jensss889";

// Konfigurasi MQTT Broker
const char* mqtt_server = "192.168.80.105";
const int basahValue = 1365;  // Sesuaikan dengan nilai basah Anda
const int keringValue = 4095; // Sesuaikan dengan nilai kering Anda
const int soilMoisturePin = 33; // Ganti dengan pin yang sesuai pada ESP32
const char* topic = "sensor/soilmoisture";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
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

    if (client.connect("NODE2")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

int readSoilMoisture() {
  // Baca nilai sensor soil moisture
  int soilMoistureValue = analogRead(soilMoisturePin);

  // Lakukan kalibrasi dan sesuaikan dengan rentang 0-100
  int kalibrasiValue = map(soilMoistureValue, keringValue, basahValue, 0, 100);
  kalibrasiValue = constrain(kalibrasiValue, 0, 100);

  Serial.print("Mapped Soil Moisture Value: ");
  Serial.println(kalibrasiValue);

  return kalibrasiValue;
}


void loop() {
  // Baca nilai langsung dari sensor tanah
  int rawSoilMoistureValue = analogRead(soilMoisturePin);
  Serial.print("Raw Soil Moisture Value: ");
  Serial.println(rawSoilMoistureValue);

  // Tunggu sebentar sebelum membaca ulang
  delay(1000); // Atur sesuai kebutuhan Anda

  // Sisanya kode tetap sama
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Baca nilai sensor soil moisture
  int soilMoistureValue = readSoilMoisture();

  // Print nilai kelembaban tanah ke Serial Monitor
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoistureValue);

  //Kirim data kelembaban tanah ke topik MQTT
  StaticJsonDocument<200> jsonDoc;    
  jsonDoc["sensor_id"] = "sensor2";
  jsonDoc["soil_moisture"] = soilMoistureValue;

  String jsonString;
  serializeJson(jsonDoc, jsonString);

  if (client.publish(topic, jsonString.c_str())) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Data sending failed");
  }

  delay(5000); // Kirim data setiap 5 detik (sesuaikan dengan kebutuhan Anda)
}
