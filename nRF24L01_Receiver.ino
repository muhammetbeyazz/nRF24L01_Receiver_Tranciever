#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "String.h";
#include "stdio.h";
#include "stdlib.h";
#include "stdbool.h";
//nRF24 kütüphaneleri
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const char* mqttUser = "";
const char* mqttPassword = "";

WiFiClient espClient;
PubSubClient client(espClient);

String UART_DATA = "";
boolean completeReading = false;

RF24 nrf24(2, 4); // use this for NodeMCU Amica/AdaFruit Huzzah ESP8266 Feather
// RH_NRF24 nrf24(8, 7); // use this with Arduino UNO/Nano
 
const byte address[6] = "00001";
unsigned long timer = 0;
byte num = 0;

//****************************************************************
//WiFi Ağına Bağlanıyoruz...
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.println("Bağlantı Kuruluyor... ");
  Serial.print("Bağlantı Sağlanacak ssid : ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi'e Bağlantı Başarılı Bir Şekilde Kurulmuştur...");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic);
  Serial.print("'ten Gelen Mesaj: ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

// Bağalntı Kesilirse Tekrar Bağlanması için gereken kodlar
void reconnect() {
  while (!client.connected()) {
    Serial.print("Mqtt Bağlantısı Deneniyor...");

    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Bağlantı Tekrar Sağlandı...");

      // Bağlandıktan Sonra Bir Duyuru Yayınlama İşemi
      client.publish("outTopic", "hello world");

      // Yeniden Abone Olma İşlemi
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("5 saniye bekleyiniz...");

      delay(5000);
    }
  }
}

//***************************************************************
char * muhammet = "1111111";
void setup()
{
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("MQTT'e Bağlanıyor...");

    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("Mqtt Bağlantısı Başarılı...");
    } else {
      Serial.print("Bağlantı Başarısız...");
      Serial.print(client.state());
      delay(2000);
    }
  }
  
  //client.subscribe("RX");
  Serial.println("nRF24L01 Baslatiliyor");
  boolean state = nrf24.begin();
  nrf24.openReadingPipe(0, address);
  nrf24.setPALevel(RF24_PA_MIN);
  nrf24.setDataRate(RF24_250KBPS);
  nrf24.startListening();
  
  if(state == true){
    Serial.println("nRF24-SPI Communication Started.");
  }
  else{
    Serial.println("nRF24-SPI Communication Failled!");
    //while(1);
  }
  Serial.println("Alici Başlatıldı...");
}
 
void loop() 
  {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  while(Serial.available() > 0){
    char ch = Serial.read();
    if(ch != '\n'){
      UART_DATA += ch;
    }
    else{
      completeReading = true;
      break;
    }
  }
  
  if(completeReading == true){
      Serial.println("Publish Mesaj : " + String(UART_DATA));
      completeReading = false;

      String str = String(UART_DATA);
      int str_len = str.length() + 1;
      char txBuffer[str_len];
      str.toCharArray(txBuffer, str_len);
      client.publish("TX",txBuffer);
      delay(50);
      
      completeReading = false;
      UART_DATA = "";
   }
  if (nrf24.available()) {              // is there a payload? get the pipe number that recieved it
      //uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
      char rxBuffer[10];
      nrf24.read(&rxBuffer, sizeof(rxBuffer));             // fetch payload from FIFO
      Serial.println("nRF24L01 Receiving Data Is: " + String(rxBuffer));
    }
  }
