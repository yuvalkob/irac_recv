import homeassistant.loader as loader

import logging




DOMAIN = 'irac_recv'

# List of integration names (string) your integration depends upon.
DEPENDENCIES = ['mqtt']


CONF_TOPIC = 'topic'
CONF_AC_ENTITY = 'ac_entity'
DEFAULT_TOPIC = '/irac_recv/living_room'

_LOGGER = logging.getLogger(__name__)

def setup(hass, config):
    """Set up the Hello MQTT component."""
    mqtt = hass.components.mqtt
    topic_ac_map = {}
    for items in config[DOMAIN]:
        topic_ac_map[items[CONF_TOPIC]] = items[CONF_AC_ENTITY]
    entity_id = 'irac_rec.last_message'

    # Listener to be called when we receive a message.
    # The msg parameter is a Message object with the following members:
    # - topic, payload, qos, retain
    def message_received(msg):
        current_ac_entity = topic_ac_map.get(msg.topic)
        if current_ac_entity:
            # get current state
            current_state = hass.states.get(current_ac_entity)
            current_state_dict = current_state.as_dict().get('attributes')
            sp = msg.payload.split(',')
            msg_dic = {}
            for i in sp:
                t = i.split(':')
                msg_dic[t[0].strip()] = t[1].strip()
            """Handle new MQTT messages."""
            # handle temperature
            temp_from_msg = msg_dic.get("Temp")
            if temp_from_msg:
                new_temp = int(temp_from_msg.replace("C",""))
                if new_temp != current_state_dict['temperature']:
                    hass.services.call('climate', 'set_temperature', 
                    {"entity_id": current_ac_entity,"temperature":new_temp}, False)
           
            # # handle mode
            mode_from_msg = msg_dic.get("Mode")
            mode = "off"
            if mode_from_msg:
                swich = int(mode_from_msg[0])
                if swich == 0: #cool
                    mode = "cool"
                elif swich == 1: #dry
                    mode = "dry"
                elif swich == 2: #auto
                    mode = "auto"
                elif swich == 3: #heat
                    mode = "heat"
                elif swich == 4: #fan
                    mode = "fan_only"
                if mode != current_state_dict['operation_mode']:
                    hass.services.call('climate', 'set_operation_mode', 
                    {"entity_id": current_ac_entity,"operation_mode":mode}, False)    
            
            # #handle power off
            power_from_msg = msg_dic.get("Power")
            if power_from_msg:
                if power_from_msg == "Off" and current_state_dict['operation_mode'] != "off":
                    hass.services.call('climate', 'set_operation_mode', 
                    {"entity_id": current_ac_entity,"operation_mode":"off"}, False)
            
            #handle fan
            fan_from_msg = msg_dic.get("Fan")
            fan_to_send = None
            if fan_from_msg:
                new_fan = int(fan_from_msg[0])
                if new_fan == 1:
                    fan_to_send = "level3"
                elif 2 < new_fan < 4:
                    fan_to_send = "level2"
                elif new_fan == 4:
                    fan_to_send = "level1"
                if current_state_dict["fan_mode"] != fan_to_send:
                    hass.services.call('climate', 'set_fan_mode', 
                    {"entity_id": current_ac_entity,"fan_mode":fan_to_send}, False)

    # Subscribe our listener to a topic.
    for topic in topic_ac_map:
        mqtt.subscribe(topic, message_received)

    # Set the initial state.
    hass.states.set(entity_id, 'No messages')

    # Service to publish a message on MQTT.
    def set_state_service(call):
        """Service to send a message."""
        mqtt.publish(topic, call.data.get('new_state'))

    # Register our service with Home Assistant.
    hass.services.register(DOMAIN, 'set_state', set_state_service)

    # Return boolean to indicate that initialization was successfully.
    return True
