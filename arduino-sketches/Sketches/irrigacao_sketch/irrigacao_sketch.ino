// Sketch para irrigação automatizada com IoT, por meio do Arduino
// Autores:
// Ricardo Corrêa Leal Paiva
// Ulisses
// Moisés
// Madhu
// Ivan
// Sander

const int sensorUmidadeSolo = 2;
const int rele = 3;
const int ledVermelho = 5;
const int ledAmarelo = 6;
const int ledVerde = 7;
const int buzzer = 10;
const int informacaoAcionamentoIrrigacao = 12;
bool irrigar = false;

// Variável para registrar o tempo atual da aplicação
int currentMillis;

// Variável para registar o tempo de acionamento do buzzer
int lastMillis;

// Variável para armazenar a leitura acerca da umidade do solo
int nivelUmidadoSolo;

void setup() {
  pinMode(sensorUmidadeSolo, INPUT);
  pinMode(rele, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(informacaoAcionamentoIrrigacao, OUTPUT);

  currentMillis = millis();
  lastMillis = currentMillis;

  Serial.begin(9600);
}

void loop() {
  // Ler o pino digital oriundo do sensor de umidade do solo. Se estiver no estado HIGH, o solo estará seco, então o sistema de irrigação deverá ser acionado.
  irrigar = digitalRead(sensorUmidadeSolo);
  nivelUmidadoSolo = analogRead(A0);

  // Obter o tempo atual em milissegundos
  int currentMillis = millis();

  if (currentMillis % 2000 == 0) {
    char bufferLeituraUmidadeSolo[1024];
    sprintf(bufferLeituraUmidadeSolo, "{ 'umidadeSolo': %d, 'tempoLeitura': %d }", nivelUmidadoSolo, currentMillis);
    
    // Obter a leitura analógica da umidade a cada 2 segundos
    Serial.println(bufferLeituraUmidadeSolo);
  }
      


  if (irrigar) {
    digitalWrite(rele, HIGH);
    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledAmarelo, HIGH);
    digitalWrite(ledVerde, LOW);
    digitalWrite(informacaoAcionamentoIrrigacao, HIGH);

    if (currentMillis % 200 == 0) {
      tone(buzzer, 600);
      lastMillis = currentMillis;
    }

    if (currentMillis == lastMillis + 100) {
      noTone(buzzer);
    }

  } else {
    digitalWrite(rele, LOW);
    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVerde, HIGH);
    digitalWrite(informacaoAcionamentoIrrigacao, LOW);
    noTone(buzzer);
  }
}
