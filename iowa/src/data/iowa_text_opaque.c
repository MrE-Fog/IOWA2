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
* Copyright (c) 2019-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
*
**********************************************/

#include "iowa_prv_data_internals.h"

#define PRV_STR_LENGTH                 32
#define PRV_OBJECT_LINK_TEXT_MAX_LEN   (size_t)11 // 65535:65535

/*************************************************************************************
** Public functions
*************************************************************************************/

iowa_status_t textSerialize(iowa_lwm2m_data_t *dataP,
                            uint8_t **bufferP,
                            size_t *bufferLengthP)
{
    assert(dataP != NULL);
    assert(bufferP != NULL);
    assert(bufferLengthP != NULL);

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Entering with data type: %s.", STR_LWM2M_TYPE(dataP->type));

    // dataP array length is assumed to be 1.
    switch (dataP->type)
    {
    case IOWA_LWM2M_TYPE_STRING:
    case IOWA_LWM2M_TYPE_CORE_LINK:
        if (dataP->value.asBuffer.length == 0)
        {
            *bufferP = NULL;
        }
        else
        {
            *bufferP = (uint8_t *)iowa_system_malloc(dataP->value.asBuffer.length);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (*bufferP == NULL)
            {
                IOWA_LOG_ERROR_MALLOC(dataP->value.asBuffer.length);
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
            memcpy(*bufferP, dataP->value.asBuffer.buffer, dataP->value.asBuffer.length);
        }

        *bufferLengthP = dataP->value.asBuffer.length;
        break;

    case IOWA_LWM2M_TYPE_OPAQUE:
        if (dataP->value.asBuffer.length == 0)
        {
            *bufferP = NULL;
            *bufferLengthP = 0;
        }
        else
        {
            // Only encode in Base64 if buffer is not nil
            size_t bufferLength;

            bufferLength = iowa_utils_base64_get_encoded_size(dataP->value.asBuffer.length);

            // 'bufferLength' cannot be equal to zero
            *bufferP = (uint8_t *)iowa_system_malloc(bufferLength);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (*bufferP == NULL)
            {
                IOWA_LOG_ERROR_MALLOC(bufferLength);
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif

            *bufferLengthP = bufferLength;
            utils_b64Encode(dataP->value.asBuffer.buffer, dataP->value.asBuffer.length, *bufferP, bufferLengthP, BASE64_MODE_CLASSIC);

            if (*bufferLengthP == 0)
            {
                IOWA_LOG_WARNING(IOWA_PART_DATA, "Opaque to Base64 conversion failed");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
        }
        break;

    case IOWA_LWM2M_TYPE_UNSIGNED_INTEGER:
        if (dataP->value.asInteger < 0)
        {
            IOWA_LOG_ARG_WARNING(IOWA_PART_DATA, "Unsigned integer value has a negative value: %d", dataP->value.asInteger);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
        // Fall through
    case IOWA_LWM2M_TYPE_INTEGER:
    case IOWA_LWM2M_TYPE_TIME:
    {
        uint8_t intString[PRV_STR_LENGTH];

        *bufferLengthP = dataUtilsIntToBuffer(dataP->value.asInteger, intString, PRV_STR_LENGTH, false);
        if (*bufferLengthP == 0)
        {
            IOWA_LOG_WARNING(IOWA_PART_DATA, "Integer to text conversion failed");
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }

        *bufferP = (uint8_t *)iowa_system_malloc(*bufferLengthP);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*bufferP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(*bufferLengthP);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memcpy(*bufferP, intString, *bufferLengthP);
        break;
    }

    case IOWA_LWM2M_TYPE_FLOAT:
    {
        uint8_t floatString[PRV_STR_LENGTH * 2];

        *bufferLengthP = dataUtilsFloatToBuffer(dataP->value.asFloat, floatString, PRV_STR_LENGTH * 2, false);
        if (*bufferLengthP == 0)
        {
            IOWA_LOG_WARNING(IOWA_PART_DATA, "Float to text conversion failed");
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }

        *bufferP = (uint8_t *)iowa_system_malloc(*bufferLengthP);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (NULL == *bufferP)
        {
            IOWA_LOG_ERROR_MALLOC(*bufferLengthP);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memcpy(*bufferP, floatString, *bufferLengthP);
        break;
    }

    case IOWA_LWM2M_TYPE_BOOLEAN:
        *bufferP = (uint8_t *)iowa_system_malloc(1);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (NULL == *bufferP)
        {
            IOWA_LOG_ERROR_MALLOC(1);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        if (dataP->value.asBoolean == true)
        {
            *bufferP[0] = '1';
        }
        else
        {
            *bufferP[0] = '0';
        }

        *bufferLengthP = 1;
        break;

    case IOWA_LWM2M_TYPE_OBJECT_LINK:
    {
        char stringBuffer[PRV_OBJECT_LINK_TEXT_MAX_LEN];

        *bufferLengthP = dataUtilsObjectLinkToBuffer(dataP, (uint8_t *)stringBuffer, PRV_OBJECT_LINK_TEXT_MAX_LEN);
        if (*bufferLengthP == 0)
        {
            IOWA_LOG_WARNING(IOWA_PART_DATA, "Object Link to text conversion failed");
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }

        *bufferP = (uint8_t *)iowa_system_malloc(*bufferLengthP);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (NULL == *bufferP)
        {
            IOWA_LOG_ERROR_MALLOC(*bufferLengthP);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memcpy(*bufferP, stringBuffer, *bufferLengthP);
        break;
    }

    default:
        return IOWA_COAP_400_BAD_REQUEST;
    }

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_commonDeserialize(iowa_lwm2m_uri_t *baseUriP,
                                           uint8_t *bufferP,
                                           size_t bufferLength,
                                           iowa_lwm2m_data_t **dataP,
                                           size_t *dataCountP)
{
    IOWA_LOG_TRACE(IOWA_PART_DATA, "Entering.");

    *dataCountP = 0;
    *dataP = NULL;

    // Check arguments
    if (baseUriP == NULL)
    {
        IOWA_LOG_INFO(IOWA_PART_DATA, "No base URI provided.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }
#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (baseUriP->resourceId == IOWA_LWM2M_ID_ALL)
    {
        IOWA_LOG_INFO(IOWA_PART_DATA, "This format does only support single resource.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }
#endif

    *dataP = (iowa_lwm2m_data_t *)iowa_system_malloc(sizeof(iowa_lwm2m_data_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*dataP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(iowa_lwm2m_data_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(*dataP, 0, sizeof(iowa_lwm2m_data_t));

    if (bufferLength != 0)
    {
        if (IOWA_COAP_NO_ERROR != dataUtilsSetBuffer(bufferP, bufferLength, *dataP, IOWA_LWM2M_TYPE_UNDEFINED))
        {
            iowa_system_free(dataP);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
    }

    (*dataP)->objectID = baseUriP->objectId;
    (*dataP)->instanceID = baseUriP->instanceId;
    (*dataP)->resourceID = baseUriP->resourceId;
    (*dataP)->resInstanceID = baseUriP->resInstanceId;

    *dataCountP = 1;

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t textDeserialize(iowa_lwm2m_uri_t *baseUriP,
                              uint8_t *bufferP,
                              size_t bufferLength,
                              iowa_lwm2m_data_t **dataP,
                              size_t *dataCountP)
{
    iowa_status_t result;

    IOWA_LOG_TRACE(IOWA_PART_DATA, "Entering.");

    *dataCountP = 0;
    *dataP = NULL;

    result = prv_commonDeserialize(baseUriP, bufferP, bufferLength, dataP, dataCountP);

    if (IOWA_COAP_NO_ERROR == result)
    {
        (*dataP)->type = IOWA_LWM2M_TYPE_STRING;
    }

    return result;
}

iowa_status_t opaqueSerialize(iowa_lwm2m_data_t *dataP,
                              uint8_t **bufferP,
                              size_t *bufferLengthP)
{
    assert(dataP != NULL);
    assert(bufferP != NULL);
    assert(bufferLengthP != NULL);

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Entering with data type: %s.", STR_LWM2M_TYPE(dataP->type));

    switch (dataP->type)
    {
    case IOWA_LWM2M_TYPE_UNDEFINED:
    case IOWA_LWM2M_TYPE_OPAQUE:
        break;

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_DATA, "Cannot serialize in opaque the data type: %s.", STR_LWM2M_TYPE(dataP->type));
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }

    if (dataP->value.asBuffer.length == 0)
    {
        *bufferP = NULL;
    }
    else
    {
        // dataP array length is assumed to be 1.
        *bufferP = (uint8_t *)iowa_system_malloc(dataP->value.asBuffer.length);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*bufferP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(dataP->value.asBuffer.length);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memcpy(*bufferP, dataP->value.asBuffer.buffer, dataP->value.asBuffer.length);
    }

    *bufferLengthP = dataP->value.asBuffer.length;

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t opaqueDeserialize(iowa_lwm2m_uri_t *baseUriP,
                                uint8_t *bufferP,
                                size_t bufferLength,
                                iowa_lwm2m_data_t **dataP,
                                size_t *dataCountP)
{
    iowa_status_t result;

    IOWA_LOG_TRACE(IOWA_PART_DATA, "Entering.");

    *dataCountP = 0;
    *dataP = NULL;

    result = prv_commonDeserialize(baseUriP, bufferP, bufferLength, dataP, dataCountP);

    if (IOWA_COAP_NO_ERROR == result)
    {
        (*dataP)->type = IOWA_LWM2M_TYPE_OPAQUE;
    }

    return result;
}
