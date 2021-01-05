# domotiqueLumieres

Cette série de sketch Arduino permet d'allumer/éteindre des lumières à distance, en utilisant des boutons d'arcade (par l'intérmédiaire d'un broker MQTT). 
Le microcontroleur est un ESP32-S2

## esp32_MQTT_relays
Les lumières sont actionnées par des relais. 

## esp32_MQTT_interrupteurs
Ce croquis connecte l'ESP32 au broker MQTT et envoie une commande quand 
  un bouton est pressé. Il souscrit au topic d'état correspondant pour
  être mis à jour des changements d'état venant d'ailleurs, et
  quand il recoit "on" il allume la led du bouton, 
  quand il recoit "off" il éteint la led du bouton.
  L'état lumineux des boutons peut varier aussi selon l'état de l'application,
  par exemple la lumière serpentera entre les boutons pendant la connection
  wifi
  

## Configuration du Mosquitto
```
switch:
  - platform: mqtt
    name: "Example_Switch"
    state_topic: "room/light"
    payload_on: "on"
    payload_off: "off"
    #retain: true
    
light:
  - platform: mqtt
    name: "Example_Light"
    command_topic: "room/light"
    state_topic: "room/light/state"
    payload_on: "on"
    payload_off: "off"
    retain: true
    qos: 2
```


## Références
### Esp32-s2
Le support sur Arduino de l'ESP32-S2 est encore en développement, l'installation est donc un peu fastidieuse, ce site résume bien :
https://www.mischianti.org/2020/12/01/esp32-s2-pinout-specs-and-arduino-ide-configuration-1/

### Le tuto de départ sur Mqtt / esp32 / home-assistant
**Merci Sparkfun !**

https://learn.sparkfun.com/tutorials/using-home-assistant-to-expand-your-home-automations/example-1-mqtt--esp32

