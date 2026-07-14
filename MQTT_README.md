## MQTT Integration Example

To enable MQTT publishing of KNX telegrams, add the following to your `linknx.xml` configuration file within the `<services>` section:

```xml
<config>
  <services>
    <knxconnection url="ip:127.0.0.1:6720"/>
    
    <!-- MQTT connection configuration -->
    <mqttconnection 
      url="mqtt.example.com" 
      port="1883" 
      username="linknx" 
      password="your_password" 
      prefix="knx"/>
    
    <xmlserver type="inet" port="1028"/>
  </services>
  
  <objects>
    <object id="light_kitchen" gad="1/1/11">Kitchen Light</object>
    <object id="temp_living" gad="1/2/10" type="EIS5">Living Room Temperature</object>
  </objects>
</config>
```

## How it works

When LinKNX receives a KNX telegram on the bus:
- **Group Address**: 1/1/11
- **Telegram Type**: Write
- **Data**: 0x01 (on)

It will publish to MQTT:
- **Topic**: `knx/1/1/11`
- **Payload**: `00 81` (hex representation of the raw KNX telegram)

All three telegram types are published:
- **Write** telegrams (0x80): Group value updates
- **Read** telegrams (0x00): Read requests
- **Response** telegrams (0x40): Read responses

## Build Requirements

To enable MQTT support, LinKNX must be built with:

```bash
./configure --with-mqtt --with-pth=/usr/local
make
```

This requires libmosquitto >= 2.1 to be installed on your system.