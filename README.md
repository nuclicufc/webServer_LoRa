# webServer_LoRa

  Código adapatado de https://randomnerdtutorials.com/esp32-lora-sensor-web-server/ para o minicurso SBRT2020 "Novas plataformas de comunicação para Internet das Coisas - experiências e práticas".

  A prática propõe a construção de um sistema de monitoramento de sensores com um servidor Web. O sistema pode ser dividido em duas partes: o transmissor, composto por uma placa Heltec WiFi LoRa 32(V2) e o módulo com sensor DHT11, utilizado para obter valores de temperatura e umidade do ambiente que serão enviados através do protocolo LoRaWAN para o receptor; e o receptor, a outra placa, que recebe essa mensagem e os exibe em uma página Web através de um servidor hospedado localmente na placa.
