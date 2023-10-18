#include <WiFi.h>
#include <PubSubClient.h>

const char * WIFI_SSID = "-------";
const char * WIFI_PASS = "*********";

const char * MQTT_BROKER = "broker.hivemq.com";
const int MQTT_BROKER_PORT = 1883;

const char * MQTT_CLIENT_ID = "osber.rioja@ucb.edu.bo";
const char * SUBSCRIBE_TOPIC = "ucbcbaiot9/in";
const char * PUBLISH_TOPIC = "ucbcbaiot9/in";

WiFiClient wiFiClient;
PubSubClient mqttClient(wiFiClient);


int Red_LED=32;
int Blue_LED=33;
int White_LED=26;


void callback(const char * topic, byte * payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += String((char) payload[i]);
  }

  if (String(topic) == SUBSCRIBE_TOPIC) {
    Serial.println("Message from topic " + String(topic) + ":" + message);

    if (message == "RedLed_ON") {
      digitalWrite(Red_LED, HIGH);
    } else if (message == "BlueLed_ON") {
      digitalWrite(Blue_LED, HIGH);
    } else if (message == "RedLed_OFF") {
      digitalWrite(Red_LED, LOW);
    } else if (message == "BlueLed_OFF") {
      digitalWrite(Blue_LED, LOW);
    } else if (message == "ON") {
      digitalWrite(White_LED, HIGH);
    } else if (message == "OFF") {
      digitalWrite(White_LED, LOW);
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
  pinMode(Red_LED, OUTPUT);
  pinMode(Blue_LED, OUTPUT);
  pinMode(White_LED, OUTPUT);
  //pinMode(LED_BUILTIN, OUTPUT);

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
      // publish
      String message = "Hello from ESP32! " + String(counter++);
      mqttClient.publish(PUBLISH_TOPIC, message.c_str());
    }
  }
}
