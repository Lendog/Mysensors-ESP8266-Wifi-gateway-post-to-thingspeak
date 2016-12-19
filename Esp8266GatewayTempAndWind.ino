/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik EKblad
 * Contribution by a-lurker and Anticimex,
 * Contribution by Norbert Truchsess <norbert.truchsess@t-online.de>
 * Contribution by Ivo Pullens (ESP8266 support)
 *
 * DESCRIPTION
 * The EthernetGateway sends data received from sensors to the WiFi link.
 * The gateway also accepts input on ethernet interface, which is then sent out to the radio network.
 *
 * VERA CONFIGURATION:
 * Enter "ip-number:port" in the ip-field of the Arduino GW device. This will temporarily override any serial configuration for the Vera plugin.
 * E.g. If you want to use the defualt values in this sketch enter: 192.168.178.66:5003
 *
 * LED purposes:
 * - To use the feature, uncomment any of the MY_DEFAULT_xx_LED_PINs in your sketch, only the LEDs that is defined is used.
 * - RX (green) - blink fast on radio message recieved. In inclusion mode will blink fast only on presentation recieved
 * - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
 * - ERR (red) - fast blink on error during transmission error or recieve crc error
 *
 * See http://www.mysensors.org/build/esp8266_gateway for wiring instructions.
 * nRF24L01+  ESP8266
 * VCC        VCC
 * CE         GPIO4
 * CSN/CS     GPIO15
 * SCK        GPIO14
 * MISO       GPIO12
 * MOSI       GPIO13
 * GND        GND
 *
 * Not all ESP8266 modules have all pins available on their external interface.
 * This code has been tested on an ESP-12 module.
 * The ESP8266 requires a certain pin configuration to download code, and another one to run code:
 * - Connect REST (reset) via 10K pullup resistor to VCC, and via switch to GND ('reset switch')
 * - Connect GPIO15 via 10K pulldown resistor to GND
 * - Connect CH_PD via 10K resistor to VCC
 * - Connect GPIO2 via 10K resistor to VCC
 * - Connect GPIO0 via 10K resistor to VCC, and via switch to GND ('bootload switch')
 *
  * Inclusion mode button:
 * - Connect GPIO5 via switch to GND ('inclusion switch')
 *
 * Hardware SHA204 signing is currently not supported!
 *
 * Make sure to fill in your ssid and WiFi password below for ssid & pass.
 */

#include <EEPROM.h>
#include <SPI.h>

// Enable debug prints to serial monitor
#define MY_DEBUG

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 9600

// Enables and select radio type (if attached)
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#define MY_GATEWAY_ESP8266

#define MY_ESP8266_SSID "MySSID"
#define MY_ESP8266_PASSWORD "MyVerySecretPassword"

// Enable UDP communication
//#define MY_USE_UDP

// Set the hostname for the WiFi Client. This is the hostname
// it will pass to the DHCP server if not static.
// #define MY_ESP8266_HOSTNAME "sensor-gateway"

// Enable MY_IP_ADDRESS here if you want a static ip address (no DHCP)
//#define MY_IP_ADDRESS 192,168,178,87

// If using static ip you need to define Gateway and Subnet address as well
#define MY_IP_GATEWAY_ADDRESS 192,168,178,1
#define MY_IP_SUBNET_ADDRESS 255,255,255,0

// The port to keep open on node server mode
#define MY_PORT 5003

// How many clients should be able to connect to this gateway (default 1)
#define MY_GATEWAY_MAX_CLIENTS 4

// Controller ip address. Enables client mode (default is "server" mode).
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere.
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 68

// Enable inclusion mode
#define MY_INCLUSION_MODE_FEATURE

// Enable Inclusion mode button on gateway
// #define MY_INCLUSION_BUTTON_FEATURE
// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 60
// Digital pin used for inclusion mode button
#define MY_INCLUSION_MODE_BUTTON_PIN  3


// Set blinking period
// #define MY_DEFAULT_LED_BLINK_PERIOD 300

// Flash leds on rx/tx/err
// Led pins used if blinking feature is enabled above
#define MY_DEFAULT_ERR_LED_PIN 16  // Error led pin
#define MY_DEFAULT_RX_LED_PIN  16  // Receive led pin
#define MY_DEFAULT_TX_LED_PIN  16  // the PCB, on board LED

#if defined(MY_USE_UDP)
  #include <WiFiUdp.h>
#endif

#include <ESP8266WiFi.h>
#include <MySensors.h>

// replace with your channel's thingspeak API key, 
const char* host = "api.thingspeak.com";   // thingspeak host address
//const char* thingspeak_key = "ThingspeakAPIWriteKey";   //    was used for just one single thingspeak channel.. but i now have multiple and handled below
const char *ssid =  "GatewayName";    //  Wifi gateway name -  cannot be longer than 32 characters!  might not be needed as handled above
const char *pass =  "GatewayPassword"; // Wifi gateway password     - might not be needed as handled above


void setup() {
  Serial.println(); Serial.println();
  Serial.println("ESP8266 MySensors Gateway");
  Serial.print("Connecting to "); Serial.println(ssid);

  checkwifi();
}

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("REST-api gateway to Thingspeak", "1.1");
  // Present locally attached sensors here
}


void loop() {
  // Send locally attached sensors data here
}

void receive(const MyMessage &message) {
 char convBuf[MAX_PAYLOAD*2+1];
//  if (mGetCommand(message) == C_PRESENTATION && inclusionMode) {
//  gw.rxBlink(3);
//   } else {
//  gw.rxBlink(1);
//   }
   // Pass along the message from sensors to serial line
 // removed for my sensor 2 test...  serial(PSTR("%d;%d;%d;%d;%d;%s\n"),message.sender, message.sensor, mGetCommand(message), mGetAck(message), message.type, message.getString(convBuf));
    
  if (message.type == V_TEMP) {   //V_TEMP, // S_TEMP. Temperature S_TEMP, S_HEATER, S_HVAC
    thingspeak(message.sensor, message.getString(convBuf),"#######");    //thingspeak channel 1 API write key
  }
  if (message.type == V_WIND) {             //V_WIND, // S_WIND. Wind speed
    thingspeak(message.sensor, message.getString(convBuf),"#######");   //thingspeak channel 2 API write key
    //Serial.print("Vwind");Serial.print(message.sensor);Serial.println(message.getString(convBuf));
  }

} 


//-------------- ThingSpeak -----------------------------

void thingspeak(int sensorN, String Dtemp, String thingspeak_key) {   //void thingspeak( sensor, temp, thingspeak_key) {

  //Serial.print("connecting to ");
  //Serial.println(host);
  
  //check if connected to wifi - if not connect
  checkwifi();

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, 80)) {
    Serial.println("connection failed");
    return;
  }


  String url = "/update?key="+thingspeak_key;
  if(sensorN==0){
    url += "&field1="+String(Dtemp);
  }
  if(sensorN==1){
    url += "&field2="+String(Dtemp);
  }
  if(sensorN==2){
    url += "&field3="+String(Dtemp);
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  //while (client.available()) {
  //  String line = client.readStringUntil('\r');
  //  Serial.print(line);
  //}
  client.stop();
  //Serial.println("Finished posting to Thingspeak...");
}

void checkwifi(){
  //check if connected to wifi - if not connect
  if (WiFi.status() != WL_CONNECTED){
    (void)WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }
  Serial.println("Connected!");  Serial.print("IP: "); Serial.println(WiFi.localIP());  Serial.flush();
  }
}


