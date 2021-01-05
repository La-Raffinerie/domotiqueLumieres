/******************************************************************************
  Basé sur
  MQTT_Light_Example.ino
  Example for controlling a light using MQTT
  by: Alex Wende, SparkFun Electronics

  Ce croquis connecte l'ESP32 au broker MQTT et souscrit à un topic pour
  écouter les commandes de lumière :
  quand il recoit "on" il met le relay à LOW, 
  quand il recoit "off" il met le relais à HIGH.
  (et pas l'inverse, pour que si le relais n'est pas alimenté,
  la lumière s'allume)
  Il publie en retour sur le topic d'état pour prévenir du changement effectué
******************************************************************************/

// *****************************************
// Parametres
// *****************************************
// MQTT Network
IPAddress broker(192, 168, 1, 15); // IP address of your MQTT broker eg. 192.168.1.50
const int port = 1883;
const char *ID = "relaisPréau";  // Name of our device, must be unique
const char *TOPIC = "preau/lumiere/1";  // Topic to subcribe to
const char *STATE_TOPIC = "preau/lumiere/1/state";  // Topic to publish the light state to

// pin arduino
const byte SWITCH_PIN = 0; // bouton sur la carte esp32 (utile pour actionner un relais directement, à des fins de test)
const byte RELAY1 = 1; // pour actionner le relais

#include "esp32_MQTT_SECRETS.h"

// *****************************************

#include <WiFi.h>
#include <PubSubClient.h>

// Utilisation de la led RGB de l'esp32 comme témoin lumineux pour du debug
#define USE_RGB
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

WiFiClient wclient;
PubSubClient client(wclient); // Setup MQTT client

bool state = 0; // état du relais


// *****************************************
// SETUP
// *****************************************
void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud
  pinMode(RELAY1, OUTPUT);
  pixels.begin();
  pinMode(SWITCH_PIN, INPUT); // Configure SWITCH_Pin as an input
  digitalWrite(SWITCH_PIN, HIGH); // enable pull-up resistor (active low)
  delay(100);
  setup_wifi(); // Connect to network
  client.setServer(broker, 1883);
  client.setCallback(callback);// Initialize the callback routine
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
    pixels.setPixelColor(0, pixels.Color(0, 20, 0));
    pixels.show();
    digitalWrite(RELAY1, HIGH);
  } else {
    pixels.setPixelColor(0, pixels.Color(20, 0, 0));
    pixels.show();
    digitalWrite(RELAY1, LOW);
  }

  // if the switch is being pressed
  if (digitalRead(SWITCH_PIN) == 0)
  {
    state = !state; //toggle state
    if (state == 1) // ON
    {
      client.publish(TOPIC, "on");
      Serial.println((String)TOPIC + " => on");
    }
    else // OFF
    {
      client.publish(TOPIC, "off");
      Serial.println((String)TOPIC + " => off");
    }

    while (digitalRead(SWITCH_PIN) == 0) // Wait for switch to be released
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
    client.publish(STATE_TOPIC, "on");
    state = 1;
  }
  else if (response == "off") // Turn the light off
  {
    client.publish(STATE_TOPIC, "off");
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
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


// *****************************************
// Reconnect to client MQTT
// *****************************************
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID, HA_USER, HA_PASS)) {
      client.subscribe(TOPIC);
      Serial.println("connected");
      Serial.print("Subcribed to: ");
      Serial.println(TOPIC);
      Serial.println('\n');

    } else {
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
