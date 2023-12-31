/**********************************************
*
*  _________ _________ ___________ _________
* |         |         |   |   |   |         |
* |_________|         |   |   |   |    _    |
* |         |    |    |   |   |   |         |
* |         |    |    |           |         |
* |         |    |    |           |    |    |
* |         |         |           |    |    |
* |_________|_________|___________|____|____|
*
* Copyright (c) 2016-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_CLIENT_INCLUDE_
#define _IOWA_CLIENT_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa.h"
#include "iowa_security.h"
#include "iowa_coap.h"

/**************************************************************
* Constants
**************************************************************/

#define IOWA_INVALID_SENSOR_ID     0xFFFFFFFF
#define IOWA_DEVICE_TIME_SENSOR_ID 0x0003F00D

#define IOWA_RESOURCE_FLAG_NONE         0x00
#define IOWA_RESOURCE_FLAG_OPTIONAL     0x00   // Default
#define IOWA_RESOURCE_FLAG_MULTIPLE     0x01
#define IOWA_RESOURCE_FLAG_MANDATORY    0x04
#define IOWA_RESOURCE_FLAG_ASYNCHRONOUS 0x08
#define IOWA_RESOURCE_FLAG_STREAMABLE   0x10

#define IOWA_OBJECT_MODE_DEFAULT      0x00
#define IOWA_OBJECT_MODE_ASYNCHRONOUS 0x01

#define IOWA_OPERATION_NONE    0x00
#define IOWA_OPERATION_READ    0x01
#define IOWA_OPERATION_WRITE   0x02
#define IOWA_OPERATION_EXECUTE 0x04

// Defines used when configuring the client.
#define IOWA_DEVICE_RSC_BATTERY             (1<<0)
#define IOWA_DEVICE_RSC_POWER_SOURCE        (1<<1)
#define IOWA_DEVICE_RSC_CURRENT_TIME        (1<<2)
#define IOWA_DEVICE_RSC_UTC_OFFSET          (1<<3)
#define IOWA_DEVICE_RSC_TIMEZONE            (1<<4)
#define IOWA_DEVICE_RSC_RESET_ERROR         (1<<5)
#define IOWA_DEVICE_RSC_MEMORY_FREE         (1<<6)
#define IOWA_DEVICE_RSC_EXTERNAL_INFO       (1<<7)

// Defines used when updating LwM2M Server parameters.
#define IOWA_SERVER_SETTING_QUEUE_MODE          1       // bool
#define IOWA_SERVER_SETTING_LIFETIME            2       // int32_t
#define IOWA_SERVER_SETTING_BINDING             3       // iowa_lwm2m_binding_t
#define IOWA_SERVER_SETTING_NOTIF_STORING       4       // bool
#define IOWA_SERVER_SETTING_DISABLE_TIMEOUT     5       // int32_t
#define IOWA_SERVER_SETTING_DEFAULT_MIN_PERIOD  6       // uint32_t
#define IOWA_SERVER_SETTING_DEFAULT_MAX_PERIOD  7       // uint32_t
#define IOWA_SERVER_SETTING_COAP_ACK_TIMEOUT    8       // uint8_t
#define IOWA_SERVER_SETTING_COAP_MAX_RETRANSMIT 9       // uint8_t

typedef uint8_t iowa_server_setting_id_t;

// Defines used when updating LwM2M Object parameters.
#define IOWA_OBJECT_SETTING_VERSION          0x00

typedef uint8_t iowa_object_setting_id_t;

// Defines used for binding.
#define IOWA_LWM2M_BINDING_UNKNOWN  0x00
#define IOWA_LWM2M_BINDING_UDP      0x01 // UDP
#define IOWA_LWM2M_BINDING_TCP      0x02 // TCP
#define IOWA_LWM2M_BINDING_SMS      0x04 // SMS
#define IOWA_LWM2M_BINDING_NON_IP   0x08 // Non-IP

/**************************************************************
 * Types and Data Structures
 **************************************************************/

typedef uint8_t iowa_lwm2m_binding_t;

typedef uint32_t iowa_sensor_t;

// The enumeration defining the server status
typedef enum
{
    IOWA_SERVER_STATUS_UNREGISTERED = 0,
    IOWA_SERVER_STATUS_REGISTERING,
    IOWA_SERVER_STATUS_REGISTERED,
    IOWA_SERVER_STATUS_REGISTRATION_FAILED,
    IOWA_SERVER_STATUS_BOOTSTRAPPING
} iowa_server_status_t;

typedef struct
{
    uint16_t                shortId;
    const char             *uriP;
    int32_t                 lifetime;
    iowa_lwm2m_binding_t    binding;
    bool                    queueMode;
    iowa_server_status_t    status;
    uint16_t                securityObjectInstanceId;
    uint16_t                serverObjectInstanceId;
    uint16_t                oscoreObjectInstanceId;
    iowa_lwm2m_protocol_version_t lwm2mVersion;
} iowa_server_info_t;

typedef struct
{
    uint16_t               id;
    iowa_lwm2m_data_type_t type;
    uint8_t                operations;
    uint8_t                flags;
} iowa_lwm2m_resource_desc_t;

typedef struct
{
    uint16_t    flags;
    int32_t     currentTime;
    const char *utcOffsetP;
    const char *timezoneP;
} iowa_device_time_info_t;

// The device writing time information callback.
// timeInfoP: Current device time information.
// userDataP: the data passed to iowa_client_configure in 'infoP.callbackUserDataP'.
// contextP: the IOWA context on which iowa_client_configure was called.
typedef void(*iowa_client_time_update_callback_t) (iowa_device_time_info_t *timeInfoP,
                                                   void *userDataP,
                                                   iowa_context_t contextP);

// The device factory reset callback.
// userDataP: the data passed to iowa_client_configure in 'infoP.callbackUserDataP'.
// contextP: the IOWA context on which iowa_client_configure was called.
typedef void(*iowa_client_factory_reset_callback_t) (void *userDataP,
                                                     iowa_context_t contextP);

// Optional device information
// manufacturer: human readable manufacturer name (can be nil)
// deviceType: type of the device (manufacturer specified string, can be nil)
// modelNumber: model identifier (manufacturer specified string, can be nil)
// serialNumber: device serial number (can be nil)
// hardwareVersion: current hardware version of the device (can be nil)
// firmwareVersion: current firmware version of the device (can be nil)
// softwareVersion: current software version of the device (can be nil)
// msisdn: The phone number of the device (can be nil)
// optFlags: flags used to enable optional features
// utcOffsetP: current UTC offset currently in effect for this LwM2M Device. UTC+X [ISO 8601] (can be nil)
// timezoneP: Indicates in which time zone the LwM2M Device is located, in IANA Timezone (TZ) database format (can be nil)
// memoryTotal: Total amount of storage space which can store data and software in the LwM2M Device (in kilobytes).
// memoryFree: Estimated current available amount of storage space which can store data and software in the LwM2M Device (in kilobytes).
// dataTimeUpdateCallback: The callback called to update the time (can be nil)
// factoryResetCallback: The callback called on a Factory Reset (can be nil)
// callbackUserDataP: Past as argument to the callback (can be nil)
typedef struct
{
    const char *manufacturer;
    const char *deviceType;
    const char *modelNumber;
    const char *serialNumber;
    const char *hardwareVersion;
    const char *firmwareVersion;
    const char *softwareVersion;
    const char *msisdn;
    const char *altPath;
    uint16_t    optFlags;
    const char *utcOffsetP;
    const char *timezoneP;
    uint32_t    memoryTotal;
    uint32_t    memoryFree;
    iowa_client_time_update_callback_t    dataTimeUpdateCallback;
    iowa_client_factory_reset_callback_t  factoryResetCallback;
    void                                 *callbackUserDataP;
} iowa_device_info_t;

/**************************************************************
 * Client Role APIs
 **************************************************************/

/****************************
 * Configuration APIs
 */

// The enumeration defining the event types
typedef enum
{
    IOWA_EVENT_UNDEFINED = 0,
    IOWA_EVENT_REG_UNREGISTERED,
    IOWA_EVENT_REG_REGISTERING,
    IOWA_EVENT_REG_REGISTERED,
    IOWA_EVENT_REG_UPDATING,
    IOWA_EVENT_REG_FAILED,
    IOWA_EVENT_REG_UPDATE_FAILED,
    IOWA_EVENT_BS_PENDING,
    IOWA_EVENT_BS_FINISHED,
    IOWA_EVENT_BS_FAILED,
    IOWA_EVENT_OBSERVATION_STARTED,
    IOWA_EVENT_OBSERVATION_NOTIFICATION,
    IOWA_EVENT_OBSERVATION_NOTIFICATION_ACKED,
    IOWA_EVENT_OBSERVATION_NOTIFICATION_FAILED,
    IOWA_EVENT_OBSERVATION_CANCELED,
    IOWA_EVENT_OBJECT_INSTANCE_CREATED,
    IOWA_EVENT_OBJECT_INSTANCE_DELETED,
    IOWA_EVENT_EVALUATION_PERIOD,
    IOWA_EVENT_READ,
    IOWA_EVENT_SERVER_SETTING_CHANGED
} iowa_event_type_t;

// The structure used by the event callback
typedef struct
{
    iowa_event_type_t eventType;
    uint16_t          serverShortId;
    union
    {
        struct
        {
            uint32_t lifetime;
            bool internalError;
            iowa_status_t errorCode;
        } registration;
        struct
        {
            iowa_sensor_t sensorId;
            uint32_t notificationNumber;
            uint16_t resourceId;
            uint32_t minPeriod;
            uint32_t maxPeriod;
            uint32_t minEvalPeriod; // Minimal sample time
            uint32_t maxEvalPeriod; // Maximal sample time
        } observation;
        struct
        {
            iowa_lwm2m_uri_t * uriP;
        } objectInstance;
        struct
        {
            iowa_lwm2m_uri_t * uriP;
            uint32_t minEvalPeriod;
            uint32_t maxEvalPeriod;
        } evalPeriod;
        struct
        {
            iowa_sensor_t sensorId;
        } sensor;
        struct
        {
            iowa_server_setting_id_t id;
            union
            {
                bool asBoolean;
                uint32_t asUInt32;
                int32_t asInt32;
                iowa_lwm2m_binding_t asBinding;
            } value;
        } serverSetting;
    } details;
} iowa_event_t;

// The event callback.
// eventP: the object the resource belongs to.
// userData: the data passed to iowa_init.
// contextP: the IOWA context on which iowa_client_configure was called.
typedef void(*iowa_event_callback_t) (iowa_event_t * eventP,
                                      void * userData,
                                      iowa_context_t contextP);

// Set the information of the LwM2M Client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - identity: unique name of the device.
// - infoP: optional information of the LwM2M Client.
// - eventCb: callback called when an event occurred.
iowa_status_t iowa_client_configure(iowa_context_t contextP,
                                    const char *identity,
                                    iowa_device_info_t *infoP,
                                    iowa_event_callback_t eventCb);

// Inform the stack of a new incoming connection.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - type: the type of the connection.
// - connP: the connection of the same opaque type as returned by iowa_system_connection_open().
// - isSecure: set to true if the security must be enabled on this connection.
iowa_status_t iowa_client_new_incoming_connection(iowa_context_t contextP,
                                                  iowa_connection_type_t type,
                                                  void *connP,
                                                  bool isSecure);

// Enumeration used by the battery status resource.
typedef enum
{
    IOWA_DEVICE_BATTERY_STATUS_NORMAL           = 0,
    IOWA_DEVICE_BATTERY_STATUS_CHARGING         = 1,
    IOWA_DEVICE_BATTERY_STATUS_CHARGE_COMPLETE  = 2,
    IOWA_DEVICE_BATTERY_STATUS_DAMAGED          = 3,
    IOWA_DEVICE_BATTERY_STATUS_LOW_BATTERY      = 4,
    IOWA_DEVICE_BATTERY_STATUS_NOT_INSTALLED    = 5,
    IOWA_DEVICE_BATTERY_STATUS_UNKNOWN          = 6
} iowa_device_battery_status_t;

// The battery update callback.
// Parameters:
// - contextP: returned by iowa_init().
// - batteryLevel: battery level in %
// - batteryStatus: battery status
// Notes:
// - IOWA_DEVICE_SUPPORT_RSC_BATTERY flag must be defined
// - Operating only if battery information is set in `iowa_client_configure()`
iowa_status_t iowa_client_device_update_battery(iowa_context_t contextP,
                                                uint8_t batteryLevel,
                                                iowa_device_battery_status_t batteryStatus);

// Defines used by the error code resource.
#define IOWA_ERROR_CODE_NO_ERROR                        0
#define IOWA_ERROR_CODE_LOW_BATTERY_POWER               1
#define IOWA_ERROR_CODE_EXTERNAL_POWER_SUPPLY_OFF       2
#define IOWA_ERROR_CODE_GPS_MODULE_FAILURE              3
#define IOWA_ERROR_CODE_LOW_RECEIVED_SIGNAL_STRENGTH    4
#define IOWA_ERROR_CODE_OUT_OF_MEMORY                   5
#define IOWA_ERROR_CODE_SMS_FAILURE                     6
#define IOWA_ERROR_CODE_IP_CONNECTIVITY_FAILURE         7
#define IOWA_ERROR_CODE_PERIPHERAL_MALFUNCTION          8

// Set an error code on Device object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - errorCode: the error code to add. If errorCode is IOWA_ERROR_CODE_NO_ERROR, all other error codes will be cleared.
iowa_status_t iowa_client_set_device_error_code(iowa_context_t contextP,
                                                uint8_t errorCode);

// Clear an error code on Device object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - errorCode: the error code to remove. It can't be IOWA_ERROR_CODE_NO_ERROR.
iowa_status_t iowa_client_clear_device_error_code(iowa_context_t contextP,
                                                  uint8_t errorCode);

typedef uint8_t iowa_power_source_type_t;

#define IOWA_POWER_SOURCE_DC_POWER            0
#define IOWA_POWER_SOURCE_INTERNAL_BATTERY    1
#define IOWA_POWER_SOURCE_EXTERNAL_BATTERY    2
#define IOWA_POWER_SOURCE_FUEL_CELL           3
#define IOWA_POWER_SOURCE_POWER_OVER_ETHERNET 4
#define IOWA_POWER_SOURCE_USB                 5
#define IOWA_POWER_SOURCE_AC_POWER            6
#define IOWA_POWER_SOURCE_SOLAR               7

// Add a power source to Device object with initial value of voltage and current.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - type: power source type.
// - voltageValue: initial voltage value (mV).
// - currentValue: initial current value (mA).
// - idP: OUT. Used to store the ID of the object.
// Notes:
// - IOWA_DEVICE_SUPPORT_RSC_POWER_SOURCE flag must be defined
// - Operating only if power source information is set in `iowa_client_configure()`
iowa_status_t iowa_client_add_device_power_source(iowa_context_t context,
                                                  iowa_power_source_type_t type,
                                                  int voltageValue,
                                                  int currentValue,
                                                  iowa_sensor_t *idP);

// Remove a power source to Device object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the object.
// Notes:
// - IOWA_DEVICE_SUPPORT_RSC_POWER_SOURCE flag must be defined
// - Operating only if power source information is set in `iowa_client_configure()`
iowa_status_t iowa_client_remove_device_power_source(iowa_context_t context,
                                                     iowa_sensor_t id);

// Update the config information of the LwM2M Client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - deviceInfoP: optional information of the LwM2M Client.
iowa_status_t iowa_client_update_device_information(iowa_context_t contextP,
                                                    iowa_device_info_t *deviceInfoP);

// Update a power source values of Device object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the object.
// - voltageValue: new voltage value.
// - currentValue: new current value.
// Notes:
// - IOWA_DEVICE_SUPPORT_RSC_POWER_SOURCE flag must be defined
// - Operating only if power source information is set in `iowa_client_configure()`
iowa_status_t iowa_client_update_device_power_source(iowa_context_t context,
                                                     iowa_sensor_t id,
                                                     int voltageValue,
                                                     int currentValue);

// Update time information of Device object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - timeInfoP: Current device time information
// Notes:
// - at least one of the following flags must be defined: IOWA_DEVICE_SUPPORT_RSC_CURRENT_TIME, IOWA_DEVICE_SUPPORT_RSC_UTC_OFFSET, IOWA_DEVICE_SUPPORT_RSC_TIMEZONE
// - Operating only if time information is set in `iowa_client_configure()`
iowa_status_t iowa_client_update_device_time_information(iowa_context_t contextP,
                                                         iowa_device_time_info_t *timeInfoP);

// Update external information of Device object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - extLinkCount: The number of links in extLinkArray.
// - extLinkArray: An array of object links referencing object instance containing "external device" information.
// Note:
// - Operating only if the flag IOWA_DEVICE_RSC_EXTERNAL_INFO is set in `iowa_client_configure()`
iowa_status_t iowa_client_update_device_external_information(iowa_context_t contextP,
                                                             size_t extLinkCount,
                                                             iowa_lwm2m_object_link_t *extLinkArray);

// Declare a LwM2M Bootstrap Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - uri: the URI to reach this bootstrap server e.g. "coap://127.0.0.1:5683", "coaps://[::1]:5684", "sms://+33102030405"
// - securityMode: the security mode to use when connecting to.
// Note: LWM2M_BOOTSTRAP must be defined
iowa_status_t iowa_client_add_bootstrap_server(iowa_context_t contextP,
                                               const char *uri,
                                               iowa_security_mode_t securityMode);

// Remove a LwM2M Bootstrap Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t iowa_client_remove_bootstrap_server(iowa_context_t contextP);

// Set the Bootstrap Hold Off time.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - holdOff: Hold off time.
iowa_status_t iowa_client_set_bootstrap_server_hold_off(iowa_context_t contextP,
                                                        int32_t holdOff);

// Retrieve the CoAP peer of a LwM2M Bootstrap Server.
// Returned value: The CoAP peer. This may be nil.
// Parameters:
// - contextP: returned by iowa_init().
iowa_coap_peer_t *iowa_client_get_bootstrap_server_coap_peer(iowa_context_t contextP);

// To enable the LwM2M queue mode
#define IOWA_LWM2M_QUEUE_MODE 0x0001

// Declare a LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to this Server.
// - uri: the URI to reach this Server e.g. "coap://127.0.0.1:5683", "coaps://[::1]:5684", "sms://+33102030405"
// - lifetime: the lifetime in seconds of the registration.
// - configFlags: enable advanced feature.
// - securityMode: the security mode to use when connecting to this Server.
iowa_status_t iowa_client_add_server(iowa_context_t contextP,
                                     uint16_t shortID,
                                     const char * uri,
                                     uint32_t lifetime,
                                     uint16_t configFlags,
                                     iowa_security_mode_t securityMode);

// Remove a LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to the Server.
iowa_status_t iowa_client_remove_server(iowa_context_t contextP,
                                        uint16_t shortID);

// Disable a LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to the Server.
// - timeout: period in seconds to disable the Server. A negative value means forever.
iowa_status_t iowa_client_disable_server(iowa_context_t contextP,
                                         uint16_t shortID,
                                         int32_t timeout);

// Enable a previously disabled LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to the Server.
iowa_status_t iowa_client_enable_server(iowa_context_t contextP,
                                        uint16_t shortID);

// Update a LwM2M Server setting.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to a LwM2M Server.
// - settingId: the setting to set.
// - argP: pointer to the setting value. Dependent on the setting Id.
iowa_status_t iowa_client_set_server_configuration(iowa_context_t contextP,
                                                   uint16_t shortId,
                                                   iowa_server_setting_id_t settingId,
                                                   void *argP);

// Retrieve a LwM2M Server setting.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to a LwM2M Server.
// - settingId: the setting to get.
// - argP: pointer to store the setting value. Dependent on the setting Id.
iowa_status_t iowa_client_get_server_configuration(iowa_context_t contextP,
                                                   uint16_t shortId,
                                                   iowa_server_setting_id_t settingId,
                                                   void *argP);

// Set the MSISDN of a previously added LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to a LwM2M Server.
// - msisdn: the MSISDN to reach this Server e.g. "0102030405" or "+33102030405". This can be nil.
iowa_status_t iowa_client_set_server_msisdn(iowa_context_t contextP,
                                            uint16_t shortID,
                                            const char * msisdn);

#define IOWA_SERVER_ACCOUNT_DEFAULT_PRIORITY_VALUE           0
#define IOWA_SERVER_ACCOUNT_DEFAULT_INITIAL_DELAY_VALUE      0
#define IOWA_SERVER_ACCOUNT_DEFAULT_REG_FAIL_BLOCK_VALUE     false
#define IOWA_SERVER_ACCOUNT_DEFAULT_BOOTSTRAP_REG_FAIL_VALUE true

// Set the registration behaviour of a LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortId: the Short ID assigned to a LwM2M Server.
// - priorityOrder: the Server priority order for the registration sequence.
// - initialDelayTimer: the initial delay to wait before to send the registration.
// - blockOnFailure: if registration fails and true is set, the registration sequence is interrupted.
// - bootstrapOnFailure: if registration fails and true is set, a bootstrap sequence is initiated.
iowa_status_t iowa_client_set_server_registration_behaviour(iowa_context_t contextP,
                                                            uint16_t shortId,
                                                            uint16_t priorityOrder,
                                                            int32_t initialDelayTimer,
                                                            bool blockOnFailure,
                                                            bool bootstrapOnFailure);

#define IOWA_SERVER_ACCOUNT_DEFAULT_COMM_RETRY_COUNT_VALUE    5
#define IOWA_SERVER_ACCOUNT_DEFAULT_COMM_RETRY_TIMER_VALUE    60
#define IOWA_SERVER_ACCOUNT_DEFAULT_COMM_SEQUENCE_COUNT_VALUE 1
#define IOWA_SERVER_ACCOUNT_DEFAULT_COMM_SEQUENCE_DELAY_VALUE 86400

// Set the communication attempts of a LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortId: the Short ID assigned to a LwM2M Server.
// - retryCount: the number of successive registration attempts before which a registration sequence is considered as failed.
// - retryDelayTimer: the number to wait between each registration sequence. The value is multiplied by two to the power of the registration retry attempt minus one (2**(retry attempt-1)) to create an exponential back-off.
// - sequenceRetryCount: the number of successive registration sequences before which a registration attempt is considered as failed.
// - sequenceDelayTimer: the number to wait between each successive registration sequences. The value is multiplied by two to the power of the registration retry attempt minus one (2**(retry attempt-1)) to create an exponential back-off.
iowa_status_t iowa_client_set_server_communication_attempts(iowa_context_t contextP,
                                                            uint16_t shortId,
                                                            uint8_t retryCount,
                                                            int32_t retryDelayTimer,
                                                            uint8_t sequenceRetryCount,
                                                            int32_t sequenceDelayTimer);

// Get the count of configured LwM2M Servers.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - serversCountP: OUT. the count of LwM2M servers.
iowa_status_t iowa_client_get_server_count(iowa_context_t contextP,
                                           size_t *serversCountP);

// Get the list of configured LwM2M Servers.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - serversCount: the number of LwM2M servers.
// - serverArrayP: OUT. the LwM2M server list.
iowa_status_t iowa_client_get_server_array(iowa_context_t contextP,
                                           size_t serversCount,
                                           iowa_server_info_t *serverArrayP);

// Retrieve the CoAP peer of a LwM2M Server.
// Returned value: The CoAP peer. This may be nil.
// Parameters:
// - contextP: returned by iowa_init().
// - shortId: the Short ID assigned to a LwM2M Server.
iowa_coap_peer_t *iowa_client_get_server_coap_peer(iowa_context_t contextP,
                                                   uint16_t shortId);

// Configure the default periods for notifications sent to a LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to the Server.
// - minPeriod: the default minimum time in seconds between two notifications.
// - maxPeriod: the default maximum time in seconds between two notifications.
iowa_status_t iowa_client_set_notification_default_periods(iowa_context_t contextP,
                                                           uint16_t shortID,
                                                           uint32_t minPeriod,
                                                           uint32_t maxPeriod);

// Ensure or not that notifications are received by the LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortId: the Short ID assigned to the Server.
// - enable: if true, activate the reliable notification.
iowa_status_t iowa_client_use_reliable_notifications(iowa_context_t contextP,
                                                     uint16_t shortId,
                                                     bool enable);

/****************************
 * Object APIs
 */

// The Read, Write and Execute operations callback.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// operation: the type of the operation among IOWA_DM_READ, IOWA_DM_WRITE and IOWA_DM_EXECUTE.
// dataP: an array of resources targeted by the operation along with their values.
// numData: the number of elements in dataP.
// userData: the iowa_client_IPSO_add_object() parameter.
// contextP: the IOWA context on which iowa_client_IPSO_add_object() was called.
typedef iowa_status_t(*iowa_RWE_callback_t) (iowa_dm_operation_t operation,
                                             iowa_lwm2m_data_t *dataP,
                                             size_t numData,
                                             void *userData,
                                             iowa_context_t contextP);

// The Create and Delete operations callback.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// operation: the type of the operation among IOWA_DM_CREATE and IOWA_DM_DELETE.
// objectID: the object targeted by the operation.
// instanceID: the instance targeted by the operation.
// userData: the iowa_client_IPSO_add_object() parameter.
// contextP: the IOWA context on which iowa_client_IPSO_add_object() was called.
typedef iowa_status_t(*iowa_CD_callback_t) (iowa_dm_operation_t operation,
                                            uint16_t objectID,
                                            uint16_t instanceID,
                                            void * userData,
                                            iowa_context_t contextP);

// The resource instance list callback.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// objectID: the object the resource belongs to.
// instanceID: the object instance the resource belongs to.
// resourceID: the resource.
// nbResInstanceP: OUT. The number of elements in resInstanceArrayP.
// resInstanceArrayP: OUT. An array containing the resource instances IDs.
// userData: the iowa_client_IPSO_add_object() parameter.
// contextP: the IOWA context on which iowa_client_IPSO_add_object() was called.
typedef iowa_status_t(*iowa_RI_callback_t) (uint16_t objectID,
                                            uint16_t instanceID,
                                            uint16_t resourceID,
                                            uint16_t * nbResInstanceP,
                                            uint16_t ** resInstanceArrayP,
                                            void * userData,
                                            iowa_context_t contextP);

// Add a LwM2M Object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - objectID: ID of the Object.
// - instanceCount: the number of elements in instanceIDs. This can be 0.
// - instanceIDs: the IDs of the instances of the Object. This can be nil.
// - resourceCount: the number of elements in resourceArray.
// - resourceArray: an array of iowa_lwm2m_resource_desc_t composing the Object.
// - dataCallback: the callback to perform Read, Write and Execute operations on the resources.
// - instanceCallback: the callback to perform Create and Delete operations. This can be nil.
// - resInstanceCallback: the callback to retrieve the list of resource instances. This can be nil.
// - userData: past as argument to the callbacks.
iowa_status_t iowa_client_add_custom_object(iowa_context_t contextP,
                                            uint16_t objectID,
                                            uint16_t instanceCount,
                                            uint16_t * instanceIDs,
                                            uint16_t resourceCount,
                                            iowa_lwm2m_resource_desc_t * resourceArray,
                                            iowa_RWE_callback_t dataCallback,
                                            iowa_CD_callback_t instanceCallback,
                                            iowa_RI_callback_t resInstanceCallback,
                                            void * userData);

// Remove a LwM2M Object created with iowa_client_add_custom_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - objectID: ID of the Object.
iowa_status_t iowa_client_remove_custom_object(iowa_context_t contextP,
                                               uint16_t objectID);

// Inform the stack that a resource value changed.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - objectID: ID of the Object.
// - instanceID: ID of the Instance. This can be IOWA_LWM2M_ID_ALL.
// - resourceID: ID of the Resource. This can be IOWA_LWM2M_ID_ALL.
iowa_status_t iowa_client_object_resource_changed(iowa_context_t contextP,
                                                  uint16_t objectID,
                                                  uint16_t instanceID,
                                                  uint16_t resourceID);

// Inform the stack that an instance was created or deleted.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - objectID: ID of the Object.
// - instanceID: ID of the Instance.
// - operation: IOWA_DM_CREATE if it is a new instance. IOWA_DM_DELETE if the instance was removed.
iowa_status_t iowa_client_object_instance_changed(iowa_context_t contextP,
                                                  uint16_t objectID,
                                                  uint16_t instanceID,
                                                  iowa_dm_operation_t operation);

// Prevent or allow the stack to send notifications. This is useful when several calls to iowa_client_object_resource_changed() are needed.
// WARNING: Do not call this function from any custom object callback.
// Returned value: none.
// Parameters:
// - contextP: returned by iowa_init().
// - enter: true to stop the notifications, false to resume them.
void iowa_client_notification_lock(iowa_context_t contextP,
                                   bool enter);

// Set the object mode.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the object.
// - mode: flags used to enable modes.
iowa_status_t iowa_client_object_set_mode(iowa_context_t contextP,
                                          iowa_sensor_t id,
                                          uint8_t mode);

// Set the object configuration.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - objectId: ID of the object.
// - settingId: ID of the setting.
// - argP: pointer to the parameter.
iowa_status_t iowa_client_object_set_configuration(iowa_context_t contextP,
                                                   uint16_t objectId,
                                                   iowa_object_setting_id_t settingId,
                                                   void *argP);

// Retrieve values of readable Resources.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - uriP: the URI of the data to retrieve.
// - dataCountP, dataArrayP: OUT. the retrieved values.
iowa_status_t iowa_client_object_get_values(iowa_context_t contextP,
                                            iowa_lwm2m_uri_t *uriP,
                                            size_t *dataCountP,
                                            iowa_lwm2m_data_t **dataArrayP);

// Free the memory allocated during a call to iowa_client_object_get_values().
// Parameters:
// - contextP: returned by iowa_init().
// - dataCount, dataArray: the retrieved values to free.
void iowa_client_object_release_values(iowa_context_t contextP,
                                       size_t dataCount,
                                       iowa_lwm2m_data_t *dataArray);

/****************************
 * Helper APIs
 */

// Transform an iowa_lwm2m_uri_t into an iowa_sensor_t.
// Returned value: the sensor id or IOWA_INVALID_SENSOR_ID in case of error.
// Parameters:
// - uriP: uri to transform.
iowa_sensor_t iowa_utils_uri_to_sensor(iowa_lwm2m_uri_t *uriP);

// Transform an iowa_sensor_t into an iowa_lwm2m_uri_t.
// Returned value: an uri.
// Parameters:
// - id: sensorId to transform.
iowa_lwm2m_uri_t iowa_utils_sensor_to_uri(iowa_sensor_t id);

/****************************
 * Other APIs
 */

// Macro to check if a CoAP message is an heartbeat message.
// Returned value: true if the message is an heartbeat, false otherwise.
#define IOWA_IS_HEARTBEAT_MESSAGE(buffer, length) (length == 4           \
                                                   && buffer[0] == 0x50  \
                                                   && buffer[1] == 0x00)

// Send an heartbeat message to a server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to this Server.
iowa_status_t iowa_client_send_heartbeat(iowa_context_t contextP,
                                         uint16_t shortID);


/****************************
 * Data push APIs
 * LWM2M_DATA_PUSH_SUPPORT must be defined
 */
typedef struct
{
    iowa_sensor_t id;
    uint16_t resourceId;
} iowa_sensor_uri_t;

// Send data from iowa_sensor_t to server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to the Server.
// - sensorUriP, sensorUriCount: the sensor uri passed for the operation.
// - responseCb, userDataP: the data push operation result callback.
iowa_status_t iowa_client_send_sensor_data(iowa_context_t contextP,
                                           uint16_t shortId,
                                           iowa_sensor_uri_t *sensorUriP,
                                           size_t sensorUriCount,
                                           iowa_response_callback_t responseCb,
                                           void *userDataP);

// Send data to server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to the Server.
// - dataArrayP, dataCount: the data passed for the operation.
// - responseCb, userDataP: the data push operation result callback.
iowa_status_t iowa_client_send_data(iowa_context_t contextP,
                                    uint16_t shortId,
                                    iowa_lwm2m_data_t *dataArrayP,
                                    size_t dataCount,
                                    iowa_response_callback_t responseCb,
                                    void *userDataP);

/**************************************************************
 * Deprecated
 **************************************************************/

// Please use IOWA_INVALID_SENSOR_ID instead
#define INVALID_SENSOR_ID 0xFFFFFFFF

// Update a LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - shortID: the Short ID assigned to a LwM2M Server.
// - settingId: the setting to set.
// - argP: pointer to the setting value. Dependent on the setting Id.
iowa_status_t iowa_server_configuration_set(iowa_context_t contextP,
                                            uint16_t shortId,
                                            iowa_server_setting_id_t settingId,
                                            void *argP);

#ifdef __cplusplus
}
#endif


#endif
