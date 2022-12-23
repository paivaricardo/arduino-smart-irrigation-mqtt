#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
#include <iostream>
#include <string>
#include <stdio.h>

using namespace std;

//WiFi
const char* SSID = "CafofoRic";                // SSID / nome da rede WiFi que deseja se conectar
const char* PASSWORD = "novaVida@2022@2G";   // Senha da rede WiFi que deseja se conectar
WiFiClient wifiClient;                        
 
//MQTT Server
const char* BROKER_MQTT = "mqtt.eclipseprojects.io"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883;                      // Porta do Broker MQTT

#define ID_MQTT  "ESP8266NODE1"            //Informe um ID unico e seu. Caso sejam usados IDs repetidos a ultima conexão irá sobrepor a anterior. 
#define TOPIC_PUBLISH "irrigacao_acionada"    //Informe um Tópico único. Caso sejam usados tópicos em duplicidade, o último irá eliminar o anterior.
PubSubClient MQTT(wifiClient);        // Instancia o Cliente MQTT passando o objeto espClient

//  Declaração das Funções
void mantemConexoes();  // Garante que as conexoes com WiFi e MQTT Broker se mantenham ativas
void conectaWiFi();     // Faz conexão com WiFi
void conectaMQTT();     // Faz conexão com Broker MQTT
void enviaPacote();     // Envia mensagem ao broker MQTT

// Tempo inicial da aplicação
int initMillis;

// Tempo da última requisição
int lastMillis;

// Contador de envio de mensagens
int counterMsgIrrigacao;

// Pino de entrada do sinal digital oriundo do Arduino, indicando o funcionamento do sistema de irrigação
const int leituraPinoDigitalIrrigacao = 4;

// Variável booleana para controlar se o sistema de irrigação foi acionado ou não
bool sistemaIrrigacaoAcionado = false;

// Variável booleana para controlar se a informação já foi dada acerca do acionamento do sistema ou não
bool infSistemaIrrigacaoDesligado = false;

void setup() {
  // Inicia o stream serial com 115200 baud rate
  Serial.begin(115200);

  conectaWiFi();
  MQTT.setServer(BROKER_MQTT, BROKER_PORT); 

  // Definir timer inicial da aplicação
  initMillis =  millis();

  // Definir o último tempo da requisição como o tempo inicial
  lastMillis = initMillis;

  // Definir o LED da placa como OUTPUT, que irá piscar toda a vez que enviar uma mensagem
  pinMode(LED_BUILTIN, OUTPUT);

  // Definir o pino digital que controla os sinais de irrigação como INPUT
  pinMode(leituraPinoDigitalIrrigacao, INPUT);
  
  // Definir o contador de envio de mensagens para o sistema de irrigação
  counterMsgIrrigacao = 0;
}

void loop() {
  mantemConexoes();
  enviaValores();
  MQTT.loop();
}

void mantemConexoes() {
    if (!MQTT.connected()) {
       conectaMQTT(); 
    }
    
    conectaWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

void conectaWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
     return;
  }
        
  Serial.print("Conectando-se na rede: ");
  Serial.print(SSID);
  Serial.println("  Aguarde!");

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI  
  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Conectado com sucesso, na rede: ");
  Serial.print(SSID);  
  Serial.print("  IP obtido: ");
  Serial.println(WiFi.localIP()); 
}

void conectaMQTT() { 
    while (!MQTT.connected()) {
        Serial.print("Conectando ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado ao Broker com sucesso!");
        } 
        else {
            Serial.println("Não foi possivel se conectar ao broker.");
            Serial.println("Nova tentatica de conexao em 10s");
            delay(10000);
        }
    }
}

void enviaValores() {
  // Obter o tempo atual em milissegundos
  int currentMillis = millis();

  // Ler o pino digital conectado ao Arduino (já reduzida a tensão com módulo step-down de 5V para 3.3V, para não danificar a ESP 8266). Se o pino digital estiver no estado HIGH, setar a booleana como verdadeira.
  sistemaIrrigacaoAcionado = digitalRead(leituraPinoDigitalIrrigacao);

  // A cada meio segundo, e se o sistema de irrigação estiver acionado, publicar uma mensagem para o servidor MQTT
  if (currentMillis % 500 == 0 && sistemaIrrigacaoAcionado) {
      // Ler o pino digital oriundo do Arduino. Se estiver no estado alto, publicar a mensagem de irrigação
      counterMsgIrrigacao++;

      char bufferPublishedMessageIrrigacaoAcionada[1024];
      const char* boolStringIrrigacaoAcionada = (sistemaIrrigacaoAcionado == true) ? "true" : "false";

      sprintf(bufferPublishedMessageIrrigacaoAcionada, "{ 'sequencial': %d, 'idCultura': '%s', 'tipoCultura': '%s', 'irrigacaoAcionada': %s, 'dataHoraMsg': %d }", counterMsgIrrigacao, "cult01-000001", "Tomate Holandês Rama", boolStringIrrigacaoAcionada, currentMillis);

      MQTT.publish(TOPIC_PUBLISH, bufferPublishedMessageIrrigacaoAcionada);
      Serial.println(bufferPublishedMessageIrrigacaoAcionada);

      // Atualizar o tempo da última requisição
      lastMillis = currentMillis;

      // Piscar LED da placa NodeMCU para mostrar que a mensagem foi publicada
      // Atenção: no NodeMCU, o LED é ligado no estado LOW
      digitalWrite(LED_BUILTIN, LOW);

      // Determinar que não foi dada a informação de que o sistema de irrigação foi desligado
      infSistemaIrrigacaoDesligado = false;
  }

  // Desligar o LED após milissegundos
  if (currentMillis == lastMillis + 250) {
      // Apagar o LED interno da placa NodeMCU
      // Atenção: no NodeMCU, o LED é ligado no estado HIGH
      digitalWrite(LED_BUILTIN, HIGH);

      if (!sistemaIrrigacaoAcionado && !infSistemaIrrigacaoDesligado) {
        counterMsgIrrigacao++;

        char bufferPublishedMessageIrrigacaoNaoAcionada[1024];
        const char* boolStringIrrigacaoAcionada = (sistemaIrrigacaoAcionado == true) ? "true" : "false";

        sprintf(bufferPublishedMessageIrrigacaoNaoAcionada, "{ 'sequencial': %d, 'idCultura': '%s', 'tipoCultura': '%s', 'irrigacaoAcionada': %s, 'dataHoraMsg': %d }", counterMsgIrrigacao, "cult01-000001", "Tomate Holandês Rama", boolStringIrrigacaoAcionada, currentMillis);

        MQTT.publish(TOPIC_PUBLISH, bufferPublishedMessageIrrigacaoNaoAcionada);
        Serial.println(bufferPublishedMessageIrrigacaoNaoAcionada);

        infSistemaIrrigacaoDesligado = true;
      }

  }

}
