#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include "EspMQTTClient.h"
#include "DHTesp.h"
#define DHT_UPDATE_PERIOD 3000
unsigned long time_now1 = 0;   //dht loop period

DHTesp dht;
float humidity = 0;
float temperature = 0;
EspMQTTClient client(
  "ssid",
  "passwd",
  "192.168.0.248",  // MQTT Broker server ip
  "mqttusername",   // Can be omitted if not needed
  "mqttpasswd",   // Can be omitted if not needed
  "smart-light",     // Client name that uniquely identify your device
  1883     
);
byte uart_cmd[7] = {};   //reveived cmd data's lenth is 7  eg aa 55 01 55 aa

byte send_cmd[7] = {};   //sent cmd data's lenth is 9

SoftwareSerial ESPserial(4, 5); // RX/TX
void setup() {
  send_cmd[0] = 0xaa;
  send_cmd[1] = 0x55;
  send_cmd[5] = 0x55;
  send_cmd[6] = 0xaa;

  dht.setup(14, DHTesp::DHT11); 
  // put your setup code here, to run once:
  Serial.begin(115200);
  ESPserial.begin(9600); 
//  WiFi.mode(WIFI_STA);
//  WiFiManager wm;
//  bool res;
//    res = wm.autoConnect("AutoConnectAP"); // anonymous ap
//    //res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
//
//    if(!res) {
//        Serial.println("Failed to connect");
////        ESP.restart();
//    } 
//    else {
//        //if you get here you have connected to the WiFi    
//        Serial.println("connected...yeey :)");
//    }
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
    
}

void onConnectionEstablished(){
  Serial.print("MQTT connected!\n");
}

void loop() {
  client.loop();
  if(ESPserial.available()==5){
    for(int i=0 ; i<=4; i++){
      uart_cmd[i] =  char(ESPserial.read());
      Serial.print(uart_cmd[i],HEX);
      Serial.print('\n');
    }
  if(uart_cmd[0] == 0xaa && uart_cmd[1] == 0x55 && uart_cmd[3] == 0x55 && uart_cmd[4] == 0xaa){
    Serial.print("check!");
    Serial.print(uart_cmd[2],HEX);
    client.publish("/smart-light/cmd",String(uart_cmd[2]));
    }
    if(uart_cmd[2] == 0x01){       //0x01 cmd to get temp
      temperature = dht.getTemperature();
      if (temperature != NAN){
//        int temp_tmp= int(temperature*10);
        send_cmd[2] = 0x01;  // cmd 01 means temperature
        send_cmd[3] = ((int)temperature)%256;  // tmp_H
        send_cmd[4] = ((int)(temperature*10))%10;   //tmp_L
        ESPserial.write(send_cmd,7);
        }
    }
    if(uart_cmd[2] == 0x02){       //0x01 cmd to get temp
      humidity = dht.getHumidity();
      if (humidity != NAN){
        send_cmd[2] = 0x02;  // cmd 02 means humidity
        send_cmd[3] = ((int)humidity)%256;  // humi
        send_cmd[4] = 0;
        ESPserial.write(send_cmd,7);
        }
    }
    
  }
//  if(millis() > time_now1 + DHT_UPDATE_PERIOD ){
//    time_now1 = millis();
//    float humidity = dht.getHumidity();
//    float temperature = dht.getTemperature();
//    Serial.print(temperature*10);
//    Serial.print('\n');
//    Serial.print(humidity);
//    Serial.print('\n\n');
//  }

}
