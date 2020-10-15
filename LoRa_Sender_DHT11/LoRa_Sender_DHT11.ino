/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-lora-sensor-web-server/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

//Bibliotecas para o LoRa
#include <SPI.h>
#include <LoRa.h>

//Bibliotecas para o display OLED
#include <Wire.h>
#include <Adafruit_SSD1306.h>

//Biblioteca para o sensor DHT11
#include <DHT.h>

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

//Definição DHT11
#define DHTPIN 13
#define DHTTYPE DHT11

//Instância para o DHT11 com o pino e o tipo
DHT dht(DHTPIN, DHTTYPE);

//Contador de pacotes
int readingID = 0;

int counter = 0;
String LoRaMessage = "";

float temperature = 0;
float humidity = 0;

//Instância para o display com as dimensões como parâmetro
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

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
  display.print("LORA SENDER");
}

//Inicialização do módulo LoRa
void startLoRa(){
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
    readingID++;
    Serial.println("Starting LoRa failed!"); 
  }
  Serial.println("LoRa Initialization OK!");
  display.setCursor(0,10);
  display.clearDisplay();
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);
}

//Obtenção dos valores de temperatura e umidade do DHT11
void getReadings(){
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

//Envio dos dados e exibição no display OLED
void sendReadings() {
  LoRaMessage = String(readingID) + "/" + String(temperature) + "&" + String(humidity);// + "#" + String(pressure);
  //Enviar o pacote LoRa para o receptor
  LoRa.beginPacket();
  LoRa.print(LoRaMessage);
  LoRa.endPacket();
  
  //Exibição dos dados no display OLED
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.print("LoRa packet sent!");
  display.setCursor(0,20);
  display.print("Temperature:");
  display.setCursor(72,20);
  display.print(temperature);
  display.setCursor(0,30);
  display.print("Humidity:");
  display.setCursor(54,30);
  display.print(humidity);
  display.setCursor(0,40);
  display.print("Reading ID:");
  display.setCursor(66,50);
  display.print(readingID);
  display.display();
  Serial.print("Sending packet: ");
  Serial.println(readingID);
  readingID++;
}

void setup() {
  //Inicialização do Serial Monitor
  Serial.begin(115200);
  dht.begin();
  startOLED();
  startLoRa();
}
void loop() {
  getReadings();
  sendReadings();
  delay(10000);
}
