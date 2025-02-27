/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
#include <endian.h>
#include <libpldm/base.h>
#include <libpldm/oem/ibm/host.h>
#include <stdint.h>
#include <string.h>

LIBPLDM_ABI_STABLE
int encode_get_alert_status_req(uint8_t instance_id, uint8_t version_id,
                                struct pldm_msg* msg, size_t payload_length)
{
    if (msg == NULL)
    {
        return PLDM_ERROR_INVALID_LENGTH;
    }

    if (payload_length != PLDM_GET_ALERT_STATUS_REQ_BYTES)
    {
        return PLDM_ERROR_INVALID_LENGTH;
    }

    struct pldm_header_info header = {0};
    header.msg_type = PLDM_REQUEST;
    header.instance = instance_id;
    header.pldm_type = PLDM_OEM;
    header.command = PLDM_HOST_GET_ALERT_STATUS;
    uint8_t rc = pack_pldm_header(&header, &(msg->hdr));
    if (rc != PLDM_SUCCESS)
    {
        return rc;
    }

    msg->payload[0] = version_id;

    return PLDM_SUCCESS;
}

LIBPLDM_ABI_STABLE
int decode_get_alert_status_resp(const struct pldm_msg* msg,
                                 size_t payload_length,
                                 uint8_t* completion_code, uint32_t* rack_entry,
                                 uint32_t* pri_cec_node)
{
    if (msg == NULL || completion_code == NULL || rack_entry == NULL ||
        pri_cec_node == NULL)
    {
        return PLDM_ERROR_INVALID_DATA;
    }

    *completion_code = msg->payload[0];
    if (PLDM_SUCCESS != *completion_code)
    {
        return PLDM_SUCCESS;
    }

    if (payload_length != PLDM_GET_ALERT_STATUS_RESP_BYTES)
    {
        return PLDM_ERROR_INVALID_LENGTH;
    }

    struct pldm_get_alert_status_resp* response =
        (struct pldm_get_alert_status_resp*)msg->payload;

    *rack_entry = le32toh(response->rack_entry);
    *pri_cec_node = le32toh(response->pri_cec_node);

    return PLDM_SUCCESS;
}

LIBPLDM_ABI_STABLE
int decode_get_alert_status_req(const struct pldm_msg* msg,
                                size_t payload_length, uint8_t* version_id)
{
    if (msg == NULL || version_id == NULL)
    {
        return PLDM_ERROR_INVALID_DATA;
    }

    if (payload_length != PLDM_GET_ALERT_STATUS_REQ_BYTES)
    {
        return PLDM_ERROR_INVALID_LENGTH;
    }

    *version_id = msg->payload[0];

    return PLDM_SUCCESS;
}

LIBPLDM_ABI_STABLE
int encode_get_alert_status_resp(uint8_t instance_id, uint8_t completion_code,
                                 uint32_t rack_entry, uint32_t pri_cec_node,
                                 struct pldm_msg* msg, size_t payload_length)
{
    if (msg == NULL)
    {
        return PLDM_ERROR_INVALID_LENGTH;
    }

    if (payload_length != PLDM_GET_ALERT_STATUS_RESP_BYTES)
    {
        return PLDM_ERROR_INVALID_DATA;
    }

    struct pldm_header_info header = {0};
    header.msg_type = PLDM_RESPONSE;
    header.instance = instance_id;
    header.pldm_type = PLDM_OEM;
    header.command = PLDM_HOST_GET_ALERT_STATUS;
    uint8_t rc = pack_pldm_header(&header, &(msg->hdr));
    if (rc != PLDM_SUCCESS)
    {
        return rc;
    }

    struct pldm_get_alert_status_resp* response =
        (struct pldm_get_alert_status_resp*)msg->payload;

    response->completion_code = completion_code;
    response->rack_entry = htole32(rack_entry);
    response->pri_cec_node = htole32(pri_cec_node);

    return PLDM_SUCCESS;
}
