# Autohome: Fan controller
* Part of the autohome project<br />
* This code runs on a [sonoff](https://www.itead.cc/sonoff-wifi-wireless-switch.html) which allows me to turn on a fan over WiFi to circulate air and easily add a temperature sensor to send data back to the server<br />
* Works with [server](https://github.com/grnnja/autohome-server) and [client](https://github.com/grnnja/autohome-client)
## Features
* Turn fan on and off through MQTT
* Use button on sonoff to turn relay on
  * send fan status back through MQTT
* send temperature from DS18B20 throught MQTT
## Installation and Usage
* clone this repository `https://github.com/grnnja/autohome-fan-controller.git`
* Build and upload code to sonoff
