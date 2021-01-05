/******************************************************************************
  Basé sur
  MQTT_Light_Example.ino
  Example for controlling a light using MQTT
  by: Alex Wende, SparkFun Electronics

  Ce croquis connecte l'ESP32 au broker MQTT et envoie une commande quand 
  un bouton est pressé. Il souscrit au topic d'état correspondant pour
  être mis à jour des changements d'état venant d'ailleurs, et
  quand il recoit "on" il allume la led du bouton, 
  quand il recoit "off" il éteint la led du bouton.
  L'état lumineux des boutons peut varier aussi selon l'état de l'application,
  par exemple la lumière serpentera entre les boutons pendant la connection
  wifi
******************************************************************************/


// *****************************************
// Parametres
// *****************************************
// MQTT Network
IPAddress broker(192, 168, 1, 15); // IP address of your MQTT broker eg. 192.168.1.50
const int port = 1883;
const char *ID = "interrupteurs1";  // Name of our device, must be unique
const char *TOPIC = "preau/lumiere/1";  // Topic to subcribe to
const char *STATE_TOPIC = "preau/lumiere/1/state";  // Topic to publish the light state to

// INTERRUPTEURS et voyants témoins
const byte SWITCH1_PIN = 19;           // Pin to control the light with
const byte LIGHT1_PIN = 20;           // Pin to control the light with
const byte SWITCH_PINS[] = {19};
const byte LIGHT_PINS[] = {20};

#include "esp32_MQTT_SECRETS.h"

// *****************************************

#include <WiFi.h>
//#include <WiFiClientSecure.h> // essai de SSL non concluant, n'arrive pas à se connecter au serveur MQTT (erreur -2)
#include <PubSubClient.h>

// Utilisation de la led RGB de l'esp32 comme témoin lumineux pour du debug
// #define USE_RGB
#ifdef USE_RGB
  #include <Adafruit_NeoPixel.h>
  #define PIN 18 // led RGB
  #define NUMPIXELS 1
  Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#endif

// WiFi Network Credentials
const char *ssid = SECRET_WIFI_SSID;   // name of your WiFi network
const char *password = SECRET_WIFI_PASSWORD; // password of the WiFi network

// Home Assistant Credentials
const char *HA_USER = SECRET_HA_USER;
const char *HA_PASS = SECRET_HA_PASSWORD;

// Etats possibles de l'appli
enum { CONNECTING_WIFI, CONNECTING_MQTT, INCOMING_MESSAGE, ERROR_PUBLISH };

//WiFiClientSecure wclient;
WiFiClient wclient;
PubSubClient client(wclient); // Setup MQTT client

bool state = 0; // etat reçu par l'inscription au topic STATE
bool blinkstate = false; // pour le clignotement de toutes les led


// *****************************************
// SETUP
// *****************************************
void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud
  pinMode(SWITCH1_PIN, INPUT_PULLUP); // Configure SWITCH_Pin as an input
  pinMode(LIGHT1_PIN, OUTPUT),
  delay(100);
  setup_wifi(); // Connect to network
  client.setServer(broker, port);
  client.setCallback(callback);// Initialize the callback routine

  #ifdef USE_RGB
    pixels.begin();
    pixels.setPixelColor(0, pixels.Color(20, 0, 0));
    pixels.show();
  #endif
}

// *****************************************
// LOOP
// *****************************************
void loop() {
  if (!client.connected())  // Reconnect if connection is lost
  {
    reconnect();
  }
  client.loop();

  if (state == 1) {
    digitalWrite(LIGHT1_PIN, HIGH);
  } else {
    digitalWrite(LIGHT1_PIN, LOW);
  }

  // if the switch is being pressed
  if (digitalRead(SWITCH1_PIN) == 0)
  {
    //state = !state; //toggle state
    if (state == 0) // OFF -> ON
    {
      client.publish(TOPIC, "on");
      Serial.println((String)TOPIC + " => on");
    }
    else // ON -> OFF
    {
      client.publish(TOPIC, "off");
      Serial.println((String)TOPIC + " => off");
    }

    while (digitalRead(SWITCH1_PIN) == 0) // Wait for switch to be released
    {
      delay(20);
    }
  }
}


// *****************************************
// Handle incomming messages from the broker
// *****************************************
void callback(char* topic, byte* payload, unsigned int length) {
  String response;

  for (int i = 0; i < length; i++) {
    response += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(response);
  if (response == "on") // Turn the light on
  {
    //client.publish(STATE_TOPIC,"on");
    state = 1;
  }
  else if (response == "off") // Turn the light off
  {
    //client.publish(STATE_TOPIC,"off");
    state = 0;
  }
}

// *****************************************
// Connect to WiFi network
// *****************************************
void setup_wifi() {
  Serial.print(ID);
  Serial.print("\n---------");
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); // Connect to network

  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    Serial.print(".");
    blinkstate = !blinkstate;
    digitalWrite(LIGHT1_PIN, blinkstate ? HIGH : LOW);
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

//  wclient.setCACert(ca_cert);
//  wclient.setCertificate (cert);
//  wclient.setPrivateKey(private_key);
//  Serial.println("Starting connection to server...");

}

// *****************************************
// Reconnect to client MQTT
// *****************************************
void reconnect() {
  Serial.print("connect status code = ");
  Serial.println(client.state());
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID, HA_USER, HA_PASS)) {
      client.subscribe(STATE_TOPIC);
      Serial.println("connected");
      Serial.print("Subcribed to: ");
      Serial.println(STATE_TOPIC);
      Serial.println('\n');

    } else {
      Serial.print("failed, status code = ");
      Serial.print(client.state());
      Serial.println(", try again in 5 seconds");
      // Wait 5 seconds before retrying
      blinkstate = !blinkstate;
      for (int i = 0; i < 6; i++)
      {
        digitalWrite(LIGHT1_PIN, (i%2) ? HIGH : LOW); // clignote selon si le compteur est pair/impair
        delay(1000);
      }
    }
  }
}
