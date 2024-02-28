// Copyright (c) 2020 Tomas Vanagas, Eimantas Rebzdys
// Copyright (c) 2020 Vilnius university, Kaunas faculty
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ESP8266ZabbixSender.h"
#include <Base64.h>
#include <OneWire.h>
#include <DallasTemperature.h>

ESP8266ZabbixSender zSender;

/* WiFi settings */
String ssid = "KOTI";
String pass = "emilyisnottheone";

/* Zabbix server setting */
#define SERVERADDR 10, 169, 3, 140
#define ZABBIXPORT 10051			
#define ZABBIXAGHOST "ESP"
#define ZABBIX_KEY1 "Probe1"
#define ZABBIX_KEY2 "Probe2"


// DS18B20
OneWire oneWire(2); // GPIO2 or D4 pin on ESP device
DallasTemperature DS18B20(&oneWire);


// WiFi connectivity checker
boolean checkConnection() {
  Serial.print("Checking Wifi");
  while (true) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Wifi connected!");
      return (true);
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println("Timed out.");
  return false;
}

void setup() {

  // Initialize terminal
  Serial.begin(115200);
  Serial.println();

  // Configure WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid.c_str(), pass.c_str());
  
  // Wait for connectivity
  while (!checkConnection());

  // Initialize Zabbix sender
  zSender.Init(IPAddress(SERVERADDR), ZABBIXPORT, ZABBIXAGHOST);
}

void loop() {
  
  // Get temperature from DS18B20
  DS18B20.requestTemperatures(); 
  float temp1 = DS18B20.getTempCByIndex(0);
  float temp2 = DS18B20.getTempCByIndex(1);

  Serial.print("Temperature: ");
  Serial.print(temp1);
  Serial.print(" : ");
  Serial.println(temp2);

  // Check connectivity
  checkConnection();

  // If temperature is good - send it to Zabbix
  if (! (temp1 == 85.0 || temp1 == (-127.0))) {
    zSender.ClearItem();              
    zSender.AddItem(ZABBIX_KEY1, temp1);
    zSender.AddItem(ZABBIX_KEY2, temp2);

    if (zSender.Send() == EXIT_SUCCESS) {
      Serial.println("ZABBIX SEND: OK");
    } else {
      Serial.println("ZABBIX SEND: Not Good");
    }
  }

  
  delay(1000); 
}
