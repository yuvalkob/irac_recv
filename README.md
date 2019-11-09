# irac_recv
Component that receive AC remote control signals, and update climate components

In order to start with this component you have to make your own ir reciever.
to do so, make sure you have NodeMCU, and IR Reciever (also, few jumper wires)
I used the following:
* NodeMCU https://www.amazon.com/HiLetgo-Internet-Development-Wireless-Micropython/dp/B010O1G1ES
* Ir Reciever https://www.amazon.com/gp/product/B00X77KXV4/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1

To connct those two, follow this chart:
![sketch](/sketch/sketchjpg.jpg)

Then, use your arduio IDE to upload the irac_recv.ino file to your NodeMCU
Before you do so, make sure you include the following libraries:
* IRremoteESP8266
* PubSubClient

And changing the configuration line at the start of the file:
```c++
const char* ssid = "WIFI SSID";
const char* password = "WIFI PASSWORD";
const char* mqtt_server = "MQTT SERVER";
const char* mqtt_username = "MQTT USER NAME";
const char* mqtt_password = "MQTT PASSWORD";
std::string ac_name = "AC NAME";
```
Complile and flush your nodemcu

Now, on HA, add the irac_recv component into the custom_components dir.
Add the following to your configuration.yml file:
```yaml
irac_recv:
  - topic: /irac_recv/[ac_name]   
    ac_entity: climate.living_room_ac
```
replace [ac_name] with the name you entered at the file you flush your NodeMCU (e.g if you entered "living_room", you topic will be /irac_recv/living_room)
for ac_entity use you climate home assistant entity
