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

#include "iowa_prv_objects_internals.h"

#ifdef LWM2M_CLIENT_MODE

#define PRV_RSC_NUMBER 19

/*************************************************************************************
** Private functions
*************************************************************************************/

static bool prv_listFindCallback(void *nodeP,
                                 void *criteriaP)
{
    return ((lwm2m_server_t *)nodeP)->srvObjInstId == *((uint16_t *)criteriaP);
}

static iowa_status_t prv_serverObjectCallback(iowa_dm_operation_t operation,
                                              iowa_lwm2m_data_t *dataP,
                                              size_t numData,
                                              void *userData,
                                              iowa_context_t contextP)
{
    lwm2m_server_t *targetP;
    size_t i;
    iowa_status_t result;

    (void)userData;

    CRIT_SECTION_ENTER(contextP);

    result = IOWA_COAP_NO_ERROR;

    switch (operation)
    {
    case IOWA_DM_READ:
        for (i = 0; i < numData && result == IOWA_COAP_NO_ERROR; i++)
        {
            if (i == 0
                || dataP[i].instanceID != dataP[i - 1].instanceID)
            {
                {
                    targetP = (lwm2m_server_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->serverList, prv_listFindCallback, &dataP[i].instanceID);
                }

                if (targetP == NULL)
                {
                    CRIT_SECTION_LEAVE(contextP);
                    IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Target server not found.");
                    return IOWA_COAP_404_NOT_FOUND;
                }
            }

            switch (dataP[i].resourceID)
            {
            case IOWA_LWM2M_SERVER_ID_SHORT_ID:
                dataP[i].value.asInteger = targetP->shortId;
                break;

            case IOWA_LWM2M_SERVER_ID_LIFETIME:
                dataP[i].value.asInteger = targetP->lifetime;
                break;

#ifdef IOWA_SERVER_SUPPORT_RSC_DEFAULT_PERIODS
            case IOWA_LWM2M_SERVER_ID_MIN_PERIOD:
                dataP[i].value.asInteger = targetP->defaultPmin;
                break;

            case IOWA_LWM2M_SERVER_ID_MAX_PERIOD:
                dataP[i].value.asInteger = targetP->defaultPmax;
                break;
#endif

#ifdef IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
            case IOWA_LWM2M_SERVER_ID_TIMEOUT:
                dataP[i].value.asInteger = targetP->disableTimeout;
                break;
#endif

            case IOWA_LWM2M_SERVER_ID_STORING:
                dataP[i].value.asBoolean = targetP->notifStoring;
                break;

            case IOWA_LWM2M_SERVER_ID_BINDING:
                // In LwM2M 1.0: Server transports + Server Queue mode
                dataP[i].value.asBuffer.length = utils_bindingToString(targetP->binding, (targetP->binding & BINDING_Q) != 0, &dataP[i].value.asBuffer.buffer);
                break;

            default:
                // Should not happen
                break;
            }
        }
        break;

    case IOWA_DM_FREE:
        for (i = 0 ; i < numData ; i++)
        {
            switch (dataP[i].resourceID)
            {
            case IOWA_LWM2M_SERVER_ID_BINDING:
                iowa_system_free(dataP[i].value.asBuffer.buffer);
                break;

            default:
                // Do nothing
                break;
            }
        }
        break;

    case IOWA_DM_WRITE:
        for (i = 0; i < numData && result == IOWA_COAP_NO_ERROR; i++)
        {
            if (i == 0
                || dataP[i].instanceID != dataP[i - 1].instanceID)
            {
                {
                    targetP = (lwm2m_server_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->serverList, prv_listFindCallback, &dataP[i].instanceID);
                }

                if (targetP == NULL)
                {
                    CRIT_SECTION_LEAVE(contextP);
                    IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Target server not found.");
                    return IOWA_COAP_404_NOT_FOUND;
                }
            }

            switch (dataP[i].resourceID)
            {

            case IOWA_LWM2M_SERVER_ID_LIFETIME:
                if (dataP[i].value.asInteger < 0
                    || dataP[i].value.asInteger > INT_MAX)
                {
                    IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Lifetime value is outside the range [0; %d].", INT_MAX);
                    result = IOWA_COAP_400_BAD_REQUEST;
                    break;
                }

                if (targetP->lifetime != dataP[i].value.asInteger)
                {
                    targetP->lifetime = dataP[i].value.asInteger;

                    {
                        coreServerEventCallback(contextP, targetP, IOWA_EVENT_SERVER_SETTING_CHANGED, false, IOWA_SERVER_SETTING_LIFETIME);
                        lwm2mUpdateRegistration(contextP, targetP, LWM2M_UPDATE_FLAG_LIFETIME);

                        CRIT_SECTION_LEAVE(contextP);
                        INTERRUPT_SELECT(contextP);
                        CRIT_SECTION_ENTER(contextP);
                    }
                }
                break;
#ifdef IOWA_SERVER_SUPPORT_RSC_DEFAULT_PERIODS
            case IOWA_LWM2M_SERVER_ID_MIN_PERIOD:
                if (dataP[i].value.asInteger < 0
                    || dataP[i].value.asInteger > UINT32_MAX)
                {
                    IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Minimum period value is outside the range [0; %d].", UINT32_MAX);
                    result = IOWA_COAP_400_BAD_REQUEST;
                    break;
                }
                targetP->defaultPmin = dataP[i].value.asInteger;
                coreServerEventCallback(contextP, targetP, IOWA_EVENT_SERVER_SETTING_CHANGED, false, IOWA_SERVER_SETTING_DEFAULT_MIN_PERIOD);
                break;

            case IOWA_LWM2M_SERVER_ID_MAX_PERIOD:
                if (dataP[i].value.asInteger < 0
                    || dataP[i].value.asInteger > UINT32_MAX)
                {
                    IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Maximum period value is outside the range [0; %d].", UINT32_MAX);
                    result = IOWA_COAP_400_BAD_REQUEST;
                    break;
                }
                    targetP->defaultPmax = dataP[i].value.asInteger;
                    coreServerEventCallback(contextP, targetP, IOWA_EVENT_SERVER_SETTING_CHANGED, false, IOWA_SERVER_SETTING_DEFAULT_MAX_PERIOD);
                break;
#endif // IOWA_SERVER_SUPPORT_RSC_DEFAULT_PERIODS

#ifdef IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
            case IOWA_LWM2M_SERVER_ID_TIMEOUT:
                if (dataP[i].value.asInteger < 0
                    || dataP[i].value.asInteger > INT32_MAX)
                {
                    IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Timeout value is outside the range [0; %d].", INT32_MAX);
                    result = IOWA_COAP_400_BAD_REQUEST;
                    break;
                }
                targetP->disableTimeout = dataP[i].value.asInteger;
                coreServerEventCallback(contextP, targetP, IOWA_EVENT_SERVER_SETTING_CHANGED, false, IOWA_SERVER_SETTING_DISABLE_TIMEOUT);
                break;
#endif

            case IOWA_LWM2M_SERVER_ID_STORING:
                {
                }

                targetP->notifStoring = dataP[i].value.asBoolean;
                coreServerEventCallback(contextP, targetP, IOWA_EVENT_SERVER_SETTING_CHANGED, false, IOWA_SERVER_SETTING_NOTIF_STORING);
                break;

            case IOWA_LWM2M_SERVER_ID_BINDING:
                IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Cannot write the resource Binding.");
                result = IOWA_COAP_400_BAD_REQUEST;
                break;

            default:
                // Should not happen
                break;
            }
        }
        break;

    case IOWA_DM_EXECUTE:
        for (i = 0; i < numData && result == IOWA_COAP_NO_ERROR; i++)
        {
            if (dataP[i].type != IOWA_LWM2M_TYPE_UNDEFINED)
            {
                // dataP[i].type can only be IOWA_LWM2M_TYPE_UNDEFINED or IOWA_LWM2M_TYPE_STRING with an Execute operation
                IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "No argument should be provided. Found: \"%.*s\".", dataP[i].value.asBuffer.length, dataP[i].value.asBuffer.buffer);
                result = IOWA_COAP_400_BAD_REQUEST;
                break;
            }

            if (i == 0
                || dataP[i].instanceID != dataP[i - 1].instanceID)
            {
                targetP = (lwm2m_server_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->serverList, prv_listFindCallback, &dataP[i].instanceID);
                if (targetP == NULL)
                {
                    CRIT_SECTION_LEAVE(contextP);
                    IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Target server not found.");
                    return IOWA_COAP_404_NOT_FOUND;
                }
            }

            switch (dataP[i].resourceID)
            {
            case IOWA_LWM2M_SERVER_ID_UPDATE:
                lwm2mUpdateRegistration(contextP, targetP, LWM2M_UPDATE_FLAG_NONE);
                result = IOWA_COAP_NO_ERROR;
                CRIT_SECTION_LEAVE(contextP);
                INTERRUPT_SELECT(contextP);
                CRIT_SECTION_ENTER(contextP);
                break;

#ifdef IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
            case IOWA_LWM2M_SERVER_ID_DISABLE:
                if (targetP->runtime.status == STATE_DISCONNECTED
                    || targetP->runtime.status == STATE_REG_REGISTERING
                    || targetP->runtime.status == STATE_REG_FAILED)
                {
                    IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Try to disable a server which is not yet registered.");
                    result = IOWA_COAP_400_BAD_REQUEST;
                    break;
                }

                targetP->runtime.flags |= LWM2M_SERVER_FLAG_DISABLE;
                break;
#endif

            default:
                // Should not happen
                break;
            }
        }
        break;

    default:
        break;
    }

    CRIT_SECTION_LEAVE(contextP);

    return result;
}

static iowa_status_t prv_serverResInstanceCallback(uint16_t objectID,
                                                   uint16_t instanceID,
                                                   uint16_t resourceID,
                                                   uint16_t *nbResInstanceP,
                                                   uint16_t **resInstanceArrayP,
                                                   void *userData,
                                                   iowa_context_t contextP)
{
    (void)objectID;
    (void)instanceID;
    (void)resourceID;
    (void)userData;
    (void)contextP;

    *nbResInstanceP = 0;
    *resInstanceArrayP = NULL;

    return IOWA_COAP_NO_ERROR;
}

/*************************************************************************************
** Internal functions
*************************************************************************************/

iowa_status_t objectServerInit(iowa_context_t contextP)
{
    iowa_status_t result;
    iowa_lwm2m_resource_desc_t resources[PRV_RSC_NUMBER];
    int currentPt;
    const uint16_t nbrRes = PRV_RSC_NUMBER
#ifndef IOWA_SERVER_SUPPORT_RSC_DEFAULT_PERIODS
                            - 2
#endif
#ifndef IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
                            - 2
#endif
#ifndef IOWA_SERVER_SUPPORT_RSC_BOOTSTRAP_TRIGGER
                            - 1
#endif
#ifndef IOWA_SERVER_SUPPORT_RSC_REGISTRATION_BEHAVIOUR
                            - 4
#endif
#ifndef IOWA_SERVER_SUPPORT_RSC_COMMUNICATION_ATTEMPTS
                            - 4
#endif
#ifndef IOWA_SERVER_SUPPORT_RSC_MUTE_SEND
                            - 1
#endif
                            ;

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Entering.");

    // Get the resource list
    currentPt = 0;

    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, SHORT_ID, resources, currentPt);
#ifdef IOWA_SERVER_SUPPORT_RSC_DEFAULT_PERIODS
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, MIN_PERIOD, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, MAX_PERIOD, resources, currentPt);
#endif
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, LIFETIME, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, STORING, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, BINDING, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, UPDATE, resources, currentPt);
#ifdef IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, DISABLE, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, TIMEOUT, resources, currentPt);
#endif

    // Inform the stack
    result = customObjectAdd(contextP,
                             IOWA_LWM2M_SERVER_OBJECT_ID,
                             OBJECT_MULTIPLE,
                             0, NULL,
                             nbrRes, resources,
                             prv_serverObjectCallback,
                             NULL,
                             prv_serverResInstanceCallback,
                             NULL);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectServerCreate(iowa_context_t contextP,
                                 uint16_t id)
{
    iowa_status_t result;

    assert(id != IOWA_LWM2M_ID_ALL);

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Adding new server object.");

    result = objectAddInstance(contextP,
                               IOWA_LWM2M_SERVER_OBJECT_ID,
                               id,
                               0, NULL);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectServerRemove(iowa_context_t contextP,
                                 uint16_t id)
{
    iowa_status_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Removing server object (instance: %d).", id);

    result = objectRemoveInstance(contextP,
                                  IOWA_LWM2M_SERVER_OBJECT_ID,
                                  id);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectServerClose(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    iowa_status_t result;

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Closing server object.");

    result = customObjectRemove(contextP, IOWA_LWM2M_SERVER_OBJECT_ID);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

#endif
