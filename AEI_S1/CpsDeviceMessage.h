#ifndef _CpsDeviceMessage_H
#define _CpsDeviceMessage_H 

#include "cJSON.h"
#include "s2j.h"

/*
{
    "deviceInfo":{
        "mag1Resistance":1726,
        "mag2Resistance":1726,
        "mag3Resistance":1726,
        "mag4Resistance":1726,
        "rfStatus":12,
        "antennaConnection":1
    },
    "cpsSettings":{
        "cpsConnectionStatus": 0
    }
}
 */
typedef struct {
    int mag1Resistance;
    int mag2Resistance;
    int mag3Resistance;
    int mag4Resistance;
    int rfStatus;
    int antennaConnection;
    char deviceDateTime[22];
} DeviceInfo;

typedef struct {
    int cpsConnectionStatus;
} CpsSettings;

typedef struct {
   DeviceInfo    deviceInfo;
   CpsSettings   cpsSettings;
} DeviceMessage;

static cJSON  *DeviceMessage_to_json(void* struct_obj) {
    DeviceMessage *struct_deviceMessage = (DeviceMessage *)struct_obj;

    s2j_create_json_obj(json_deviceMessage);

    s2j_json_set_struct_element(json_deviceInfo, json_deviceMessage, struct_deviceInfo, struct_deviceMessage, DeviceInfo, deviceInfo);
    s2j_json_set_basic_element(json_deviceInfo, struct_deviceInfo, int, mag1Resistance);
    s2j_json_set_basic_element(json_deviceInfo, struct_deviceInfo, int, mag2Resistance);
    s2j_json_set_basic_element(json_deviceInfo, struct_deviceInfo, int, mag3Resistance);
    s2j_json_set_basic_element(json_deviceInfo, struct_deviceInfo, int, mag4Resistance);
    s2j_json_set_basic_element(json_deviceInfo, struct_deviceInfo, int, rfStatus);
    s2j_json_set_basic_element(json_deviceInfo, struct_deviceInfo, int, antennaConnection);
    s2j_json_set_basic_element(json_deviceInfo, struct_deviceInfo, string, deviceDateTime);

    s2j_json_set_struct_element(json_cpsSettings, json_deviceMessage, struct_cpsSettings, struct_deviceMessage, CpsSettings, cpsSettings);
    s2j_json_set_basic_element(json_cpsSettings, struct_cpsSettings, int, cpsConnectionStatus);
    
    return json_deviceMessage;
}


#endif