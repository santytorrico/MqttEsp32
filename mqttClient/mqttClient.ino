#include <WiFi.h>
#include <PubSubClient.h>
#include <NewPing.h>
#include <DHT.h>

const char * WIFI_SSID = "COMTECO-N3783084";
const char * WIFI_PASS = "ZKTQD32168";

const char * MQTT_BROKER = "broker.hivemq.com";
const int MQTT_BROKER_PORT = 1883;

const char * MQTT_CLIENT_ID = "santy.torrico@ucb.edu.bo";
const char *TEMP_TOPIC = "ucbcbaiot9/temperature";
const char *HUM_TOPIC = "ucbcbaiot9/humidity";
const char *DISTANCE_TOPIC = "ucbcbaiot9/distance";
const char *LED_TOPIC = "ucbcbaiot9/led";
const char *SUBSCRIBE_TOPIC = "ucbcbaiot9/led";

#define TRIGGER_PIN 12  // Pin connected to the sensor's trigger
#define ECHO_PIN 13     // Pin connected to the sensor's echo
#define MAX_DISTANCE 200 // Maximum distance to measure (adjust as needed)

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

#define DHT_PIN 4   
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

WiFiClient wiFiClient;
PubSubClient mqttClient(wiFiClient);

void callback(const char * topic, byte * payload, unsigned int lenght) {
  String message;
  for (int i = 0; i < lenght; i++) {
    message += String((char) payload[i]);
  }
  if (String(topic) == SUBSCRIBE_TOPIC) {
    Serial.println("Message from topic " + String(topic) + ":" + message);

    if (message == "LED_ON") {
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      if (message == "LED_OFF"){
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
  }
}

boolean mqttClientConnect() {
  Serial.print("Connecting to " + String(MQTT_BROKER));
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println(" DONE!");

    mqttClient.subscribe(SUBSCRIBE_TOPIC);
    Serial.println("Subscribed to " + String(SUBSCRIBE_TOPIC));
  } else {
    Serial.println("Can't connect to " + String(MQTT_BROKER));
  }
  return mqttClient.connected();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.print("Connecting to " + String(WIFI_SSID));

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println(" DONE!");

  mqttClient.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  mqttClient.setCallback(callback);

  Serial.println("MQTT client set up.");

  dht.begin();
}

unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;

unsigned char counter = 0;

void loop() {
  unsigned long now = millis();

  if (!mqttClient.connected()) {
    if (now - previousConnectMillis >= 2000) {
      previousConnectMillis = now;
      if (mqttClientConnect()) previousConnectMillis = 0;
      else delay(1000);
    }
  } else { // Connected to the MQTT Broker
    mqttClient.loop();
    delay(20);
    
    if (now - previousPublishMillis >= 10000) {
      previousPublishMillis = now;
      
      float humidity = dht.readHumidity();
      float temperature = dht.readTemperature();
      unsigned int distance = sonar.ping_cm();

      if (!isnan(humidity) && !isnan(temperature)) {
      // Publish temperature and humidity to the MQTT topic
        String tempMessage = String(temperature);
        mqttClient.publish(TEMP_TOPIC, tempMessage.c_str());
        // Serial.println("Published message: " + tempMessage);

        String humidityMessage = String(humidity) ;
        mqttClient.publish(HUM_TOPIC, humidityMessage.c_str());
        Serial.println("Published message: " + humidityMessage);
      } else {
        Serial.println("Failed to read from DHT sensor");
      }
      // String message = "Distance: " + String(distance) + " cm";
      String message = String(distance);
      mqttClient.publish(DISTANCE_TOPIC, message.c_str());
      // Serial.println("Published message: " + message);
    }
  }
}
