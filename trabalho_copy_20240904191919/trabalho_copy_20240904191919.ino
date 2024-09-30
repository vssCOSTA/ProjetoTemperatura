#include <ESP8266WiFi.h>

#include <PubSubClient.h>

#include <ArduinoJson.h>
 
// Configurações WiFi

const char* ssid = "SENAC";

const char* password = "x1y2z3@snc";
 
// Configurações MQTT

const char* mqtt_server = "zfeemqtt.eastus.cloudapp.azure.com";

const int mqtt_port = 41883;

const char* mqtt_user = "Senac";

const char* mqtt_password = "Senac";

const char* clientID = "Senac12";

const char* topic = "Senac/Gptrutas/Saida"; // Tópico para receber o JSON
 
// Pinos dos LEDs (GPIO 14 e GPIO 12 no ESP8266)

const int redLedPin = D5; // LED vermelho

const int blueLedPin = D6; // LED azul
 
WiFiClient espClient;

PubSubClient client(espClient);
 
// Função para conectar ao WiFi

void setup_wifi() {

    delay(10);

    Serial.println();

    Serial.print("Conectando a ");

    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {

        delay(500);

        Serial.print(".");

    }

    Serial.println("");

    Serial.println("WiFi conectado");

    Serial.print("Endereço IP: ");

    Serial.println(WiFi.localIP());

}
 
// Função chamada quando uma mensagem é recebida

void callback(char* topic, byte* payload, unsigned int length) {

    Serial.print("Mensagem recebida [");

    Serial.print(topic);

    Serial.print("]: ");

    // Converte o payload para string

    String message;

    for (int i = 0; i < length; i++) {

        message += (char)payload[i];

    }

    Serial.println(message);
 
    // Cria um objeto JSON para tratar os dados recebidos

    StaticJsonDocument<200> doc;

    DeserializationError error = deserializeJson(doc, message);
 
    if (error) {

        Serial.print("Erro ao desserializar JSON: ");

        Serial.println(error.c_str());

        return;

    }
 
    // Lê o status do JSON

    const char* status = doc["status"]; // "Frio" ou "Quente"
 
    // Verifica o status e aciona os LEDs

    if (strcmp(status, "Frio") == 0) {

        digitalWrite(redLedPin, LOW);   // Desliga o LED vermelho

        digitalWrite(blueLedPin, HIGH); // Liga o LED azul

        Serial.println("Status: Frio, LED azul aceso.");

    } else if (strcmp(status, "Quente") == 0) {

        digitalWrite(redLedPin, HIGH);  // Liga o LED vermelho

        digitalWrite(blueLedPin, LOW);   // Desliga o LED azul

        Serial.println("Status: Quente, LED vermelho aceso.");

    }

}
 
// Função para conectar ao broker MQTT

void reconnect() {

    while (!client.connected()) {

        Serial.print("Conectando ao MQTT...");

        if (client.connect(clientID, mqtt_user, mqtt_password)) {

            Serial.println("conectado");

            client.subscribe(topic); // Inscreve no tópico para receber dados de temperatura

        } else {

            Serial.print("falha, rc=");

            Serial.print(client.state());

            Serial.println(" Tentando novamente em 5 segundos");

            delay(5000);

        }

    }

}
 
void setup() {

    // Inicializa os pinos dos LEDs como saída

    pinMode(redLedPin, OUTPUT);

    pinMode(blueLedPin, OUTPUT);

    // Configura o monitor serial

    Serial.begin(115200);
 
    // Conecta ao WiFi

    setup_wifi();
 
    // Configura o servidor MQTT

    client.setServer(mqtt_server, mqtt_port);

    client.setCallback(callback);

}
 
void loop() {

    // Reconecta ao MQTT se desconectado

    if (!client.connected()) {

        reconnect();

    }

    client.loop(); // Mantém a conexão com o broker

}

 
