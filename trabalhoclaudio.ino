#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// ===================== ALTERE AS CONFIGURAÇÕES ABAIXO
const char* ssid = "epg_rede";
const char* password = "estevao123";
const char* mqtt_server = "6eda8dcbba3643ed8116e7e45658e1e7.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "estevaopgalvao";
const char* mqtt_pass = "123abc--";

const char* topic_cmd = "sala/led/cmd";   // Tópico de comando
const char* topic_state = "sala/led/state"; // Tópico de status

#define LED_PIN 5 // Pino GPIO 5 na ESP32

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando ao WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (message == "1" || message == "ON") {
    digitalWrite(LED_PIN, HIGH);
    client.publish(topic_state, "ON");
    Serial.println("LED ligado");
  } else if (message == "0" || message == "OFF") {
    digitalWrite(LED_PIN, LOW);
    client.publish(topic_state, "OFF");
    Serial.println("LED desligado");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao Broker MQTT...");
    String clientId = "ESP32-LED-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("conectado");
      client.subscribe(topic_cmd);
      client.publish(topic_state, "ONLINE");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" — nova tentativa em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(115200);

  setup_wifi();

  espClient.setInsecure(); // Simplifica a validação TLS em ambiente didático
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}