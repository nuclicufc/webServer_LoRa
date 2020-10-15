/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-lora-sensor-web-server/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

//Bibliotecas para o WiFi
#include <WiFi.h>
#include "ESPAsyncWebServer.h"

#include <SPIFFS.h>

//Bibliotecas para o LoRa
#include <SPI.h>
#include <LoRa.h>

//Bibliotecas para o display OLED
#include <Wire.h>
#include <Adafruit_SSD1306.h>

//Bibliotecas para obter o tempo do NTP Server
#include <NTPClient.h>
#include <WiFiUdp.h>

//Definição de pinos padrão para o módulo LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//Frequência
//433E6 para a Ásia
//866E6 para a Europa
//915E6 para a América
#define BAND 915E6

//Pinos OLED
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 //Largura da tela OLED, em pixels
#define SCREEN_HEIGHT 64 //Altura da tela OLED, em pixels

//Substitua por suas credenciais de rede
const char* ssid     = "ssid-rede";
const char* password = "senha-rede";

//Definição do NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//Variáveis para o armazenamento de data e hora
String formattedDate;
String day;
String hour;
String timestamp;

//Variáveis para o armazenamento dos dados recebidos
int rssi;
String loRaMessage;
String temperature;
String humidity;
String readingID;

//Criação de um objeto AsyncWebServer na porta 80
AsyncWebServer server(80);

//Instância para o display com as dimensões como parâmetro
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//Substituição dos espaços reservados pelos valores recebidos
String processor(const String& var){
  if(var == "TEMPERATURE"){
    return temperature;
  }
  else if(var == "HUMIDITY"){
    return humidity;
  }
  else if(var == "TIMESTAMP"){
    return timestamp;
  }
  else if (var == "RRSI"){
    return String(rssi);
  }
  return String();
}

//Inicialização do display OLED
void startOLED(){
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //inicializar OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA RECEIVER");
}

//Inicialização do módulo LoRa
void startLoRa(){
  int counter;
  //Pinos SPI LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  while (!LoRa.begin(BAND) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 10) {
    // Incremento de readingID a cada nova leitura
    Serial.println("Starting LoRa failed!"); 
  }
  Serial.println("LoRa Initialization OK!");
  display.setCursor(0,10);
  display.clearDisplay();
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);
}

//Uso do SSID e senha para conectar a placa à rede local
void connectWiFi(){
  //Conexão à rede WiFi com SSID e senha
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Imprime o endereço de IP local e inicia o servidor web
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  display.setCursor(0,20);
  display.print("Access web server at: ");
  display.setCursor(0,30);
  display.print(WiFi.localIP());
  display.display();
}

//Lê e organiza o pacote recebido
void getLoRaData() {
  Serial.print("Lora packet received: ");
  //Leitura do pacote
  while (LoRa.available()) {
    String LoRaData = LoRa.readString();
    //Formato LoRaData: readingID/temperature&humidity
    //Exemplo de String: 1/27.43&65.4
    Serial.print(LoRaData); 
    
    //Armazenamento de readingID, temperatura e umidade
    int pos1 = LoRaData.indexOf('/');
    int pos2 = LoRaData.indexOf('&');
    readingID = LoRaData.substring(0, pos1);
    temperature = LoRaData.substring(pos1 +1, pos2);
    humidity = LoRaData.substring(pos2+1, LoRaData.length());   
  }
  //Obter RSSI
  rssi = LoRa.packetRssi();
  Serial.print(" with RSSI ");    
  Serial.println(rssi);
}

//Obtenção de data e hora do NTPClient
void getTimeStamp() {
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  //O formattedDate tem o seguinte formato:
  //2018-05-28T16:00:13Z
  //E deve ser estraído data e hora
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  //Data
  int splitT = formattedDate.indexOf("T");
  day = formattedDate.substring(0, splitT);
  Serial.println(day);
  
  //Hora
  hour = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.println(hour);
  timestamp = day + " " + hour;
}

void setup() { 
  //Inicialização do Serial Monitor
  Serial.begin(115200);
  startOLED();
  startLoRa();
  connectWiFi();
  
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  //Rota para root / página web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperature.c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", humidity.c_str());
  });
  server.on("/timestamp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", timestamp.c_str());
  });
  server.on("/rssi", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(rssi).c_str());
  });
  server.on("/winter", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/winter.jpg", "image/jpg");
  });
  //Inicialização do servidor
  server.begin();
  
  //Inicialização de um NTPClient
  timeClient.begin();
  //Defina o tempo de deslocamento em segundos para ajustar para o seu fuso horário, por exemplo:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(-3); //Brasil
}

void loop() {
  //Verifica se existem pacotes LoRa disponíveis
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    getLoRaData();
    getTimeStamp();
  }
}
