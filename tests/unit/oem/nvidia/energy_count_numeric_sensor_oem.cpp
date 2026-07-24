/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
#include <endian.h>
#include <libpldm/base.h>
#include <libpldm/oem/nvidia/energy_count_numeric_sensor_oem.h>

#include <array>
#include <cstdint>
#include <cstring>

#include <gtest/gtest.h>

constexpr auto hdrSize = sizeof(pldm_msg_hdr);

TEST(EncodeGetOemEnergyCountSensorReadingReq, testGoodEncodeRequest)
{
    std::array<uint8_t, sizeof(pldm_msg_hdr) +
                            PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_REQ_BYTES>
        requestMsg{};

    uint16_t sensorId = 0x1234;
    uint16_t sensorIdLe = htole16(sensorId);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto request = reinterpret_cast<pldm_msg*>(requestMsg.data());

    auto rc =
        encode_get_oem_energy_count_sensor_reading_req(0, sensorId, request);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(request->hdr.request, PLDM_REQUEST);
    ASSERT_EQ(request->hdr.instance_id, 0);
    ASSERT_EQ(request->hdr.type, PLDM_OEM);
    ASSERT_EQ(request->hdr.command, PLDM_OEM_GET_ENERGYCOUNT_SENSOR_READING);
    ASSERT_EQ(0, memcmp(request->payload, &sensorIdLe, sizeof(sensorIdLe)));
}

TEST(EncodeGetOemEnergyCountSensorReadingReq, testGoodEncodeRequestMaxSensor)
{
    std::array<uint8_t, sizeof(pldm_msg_hdr) +
                            PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_REQ_BYTES>
        requestMsg{};

    uint16_t sensorId = 0xffff;
    uint16_t sensorIdLe = htole16(sensorId);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto request = reinterpret_cast<pldm_msg*>(requestMsg.data());

    auto rc =
        encode_get_oem_energy_count_sensor_reading_req(31, sensorId, request);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(request->hdr.request, PLDM_REQUEST);
    ASSERT_EQ(request->hdr.instance_id, 31);
    ASSERT_EQ(request->hdr.type, PLDM_OEM);
    ASSERT_EQ(request->hdr.command, PLDM_OEM_GET_ENERGYCOUNT_SENSOR_READING);
    ASSERT_EQ(0, memcmp(request->payload, &sensorIdLe, sizeof(sensorIdLe)));
}

TEST(EncodeGetOemEnergyCountSensorReadingReq, testBadEncodeRequestNullMsg)
{
    auto rc = encode_get_oem_energy_count_sensor_reading_req(0, 0x1234, NULL);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testGoodDecodeRespUint8)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_UINT8;
    resp->sensor_operational_state = 0x01;
    resp->present_reading[0] = 0xab;

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT8;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(retCompletionCode, PLDM_SUCCESS);
    ASSERT_EQ(retSensorDataSize, PLDM_SENSOR_OEM_DATA_SIZE_UINT8);
    ASSERT_EQ(retSensorOperationalState, 0x01);
    ASSERT_EQ(retPresentReading[0], 0xab);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testGoodDecodeRespUint16)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES + 1;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    uint16_t reading = 0x1234;
    uint16_t readingLe = htole16(reading);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_UINT16;
    resp->sensor_operational_state = 0x02;
    memcpy(resp->present_reading, &readingLe, sizeof(readingLe));

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT16;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(retCompletionCode, PLDM_SUCCESS);
    ASSERT_EQ(retSensorDataSize, PLDM_SENSOR_OEM_DATA_SIZE_UINT16);
    ASSERT_EQ(retSensorOperationalState, 0x02);

    uint16_t retVal;
    memcpy(&retVal, retPresentReading, sizeof(retVal));
    ASSERT_EQ(retVal, reading);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testGoodDecodeRespUint32)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES + 3;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    uint32_t reading = 0x12345678;
    uint32_t readingLe = htole32(reading);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_UINT32;
    resp->sensor_operational_state = 0x03;
    memcpy(resp->present_reading, &readingLe, sizeof(readingLe));

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT32;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(retCompletionCode, PLDM_SUCCESS);
    ASSERT_EQ(retSensorDataSize, PLDM_SENSOR_OEM_DATA_SIZE_UINT32);
    ASSERT_EQ(retSensorOperationalState, 0x03);

    uint32_t retVal;
    memcpy(&retVal, retPresentReading, sizeof(retVal));
    ASSERT_EQ(retVal, reading);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testGoodDecodeRespUint64)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES + 7;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    uint64_t reading = 0x123456789abcdef0;
    uint64_t readingLe = htole64(reading);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_UINT64;
    resp->sensor_operational_state = 0x04;
    memcpy(resp->present_reading, &readingLe, sizeof(readingLe));

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT64;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(retCompletionCode, PLDM_SUCCESS);
    ASSERT_EQ(retSensorDataSize, PLDM_SENSOR_OEM_DATA_SIZE_UINT64);
    ASSERT_EQ(retSensorOperationalState, 0x04);

    uint64_t retVal;
    memcpy(&retVal, retPresentReading, sizeof(retVal));
    ASSERT_EQ(retVal, reading);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testGoodDecodeRespSint8)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_SINT8;
    resp->sensor_operational_state = 0x00;
    resp->present_reading[0] = 0x80;

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_SINT8;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(retCompletionCode, PLDM_SUCCESS);
    ASSERT_EQ(retSensorDataSize, PLDM_SENSOR_OEM_DATA_SIZE_SINT8);
    ASSERT_EQ(retSensorOperationalState, 0x00);
    ASSERT_EQ(retPresentReading[0], 0x80);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testGoodDecodeRespSint16)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES + 1;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    int16_t reading = -1234;
    uint16_t readingLe = htole16(static_cast<uint16_t>(reading));

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_SINT16;
    resp->sensor_operational_state = 0x01;
    memcpy(resp->present_reading, &readingLe, sizeof(readingLe));

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_SINT16;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(retCompletionCode, PLDM_SUCCESS);
    ASSERT_EQ(retSensorDataSize, PLDM_SENSOR_OEM_DATA_SIZE_SINT16);
    ASSERT_EQ(retSensorOperationalState, 0x01);

    int16_t retVal;
    memcpy(&retVal, retPresentReading, sizeof(retVal));
    ASSERT_EQ(retVal, reading);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testGoodDecodeRespSint32)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES + 3;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    int32_t reading = -123456;
    uint32_t readingLe = htole32(static_cast<uint32_t>(reading));

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_SINT32;
    resp->sensor_operational_state = 0x02;
    memcpy(resp->present_reading, &readingLe, sizeof(readingLe));

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_SINT32;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(retCompletionCode, PLDM_SUCCESS);
    ASSERT_EQ(retSensorDataSize, PLDM_SENSOR_OEM_DATA_SIZE_SINT32);
    ASSERT_EQ(retSensorOperationalState, 0x02);

    int32_t retVal;
    memcpy(&retVal, retPresentReading, sizeof(retVal));
    ASSERT_EQ(retVal, reading);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testGoodDecodeRespSint64)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES + 7;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    int64_t reading = -123456789012345;
    uint64_t readingLe = htole64(static_cast<uint64_t>(reading));

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_SINT64;
    resp->sensor_operational_state = 0x03;
    memcpy(resp->present_reading, &readingLe, sizeof(readingLe));

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_SINT64;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(retCompletionCode, PLDM_SUCCESS);
    ASSERT_EQ(retSensorDataSize, PLDM_SENSOR_OEM_DATA_SIZE_SINT64);
    ASSERT_EQ(retSensorOperationalState, 0x03);

    int64_t retVal;
    memcpy(&retVal, retPresentReading, sizeof(retVal));
    ASSERT_EQ(retVal, reading);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp,
     testGoodDecodeRespCompletionCodeFailure)
{
    constexpr size_t payloadLen = 1;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    response->payload[0] = PLDM_ERROR;

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT8;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_SUCCESS);
    ASSERT_EQ(retCompletionCode, PLDM_ERROR);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testBadDecodeRespNullMsg)
{
    uint8_t completionCode = 0;
    uint8_t sensorDataSize = 0;
    uint8_t sensorOperationalState = 0;
    uint8_t presentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        NULL, 0, &completionCode, &sensorDataSize, &sensorOperationalState,
        presentReading);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testBadDecodeRespNullParams)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    uint8_t completionCode = 0;
    uint8_t sensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT8;
    uint8_t sensorOperationalState = 0;
    uint8_t presentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, NULL, &sensorDataSize, &sensorOperationalState,
        presentReading);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);

    rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &completionCode, NULL, &sensorOperationalState,
        presentReading);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);

    rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &completionCode, &sensorDataSize, NULL,
        presentReading);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);

    rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &completionCode, &sensorDataSize,
        &sensorOperationalState, NULL);
    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testBadDecodeRespInvalidLength)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES - 1;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    response->payload[0] = PLDM_SUCCESS;

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT8;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp, testBadDecodeRespInvalidDataSize)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_SINT64 + 1;
    resp->sensor_operational_state = 0x00;

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_SINT64 + 1;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_DATA);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp,
     testBadDecodeRespSensorDataSizeTooLargeForCaller)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_UINT16;
    resp->sensor_operational_state = 0x00;

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT8;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp,
     testBadDecodeRespUint8WrongPayloadLength)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES + 1;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_UINT8;
    resp->sensor_operational_state = 0x00;

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT8;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp,
     testBadDecodeRespUint16WrongPayloadLength)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_UINT16;
    resp->sensor_operational_state = 0x00;

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT16;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp,
     testBadDecodeRespUint32WrongPayloadLength)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_UINT32;
    resp->sensor_operational_state = 0x00;

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT32;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}

TEST(DecodeGetOemEnergyCountSensorReadingResp,
     testBadDecodeRespUint64WrongPayloadLength)
{
    constexpr size_t payloadLen =
        PLDM_GET_OEM_ENERGYCOUNT_SENSOR_READING_MIN_RESP_BYTES;
    std::array<uint8_t, hdrSize + payloadLen> responseMsg{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto response = reinterpret_cast<pldm_msg*>(responseMsg.data());
    auto* resp =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<pldm_get_oem_energycount_sensor_reading_resp*>(
            response->payload);

    resp->completion_code = PLDM_SUCCESS;
    resp->sensor_data_size = PLDM_SENSOR_OEM_DATA_SIZE_UINT64;
    resp->sensor_operational_state = 0x00;

    uint8_t retCompletionCode = 0;
    uint8_t retSensorDataSize = PLDM_SENSOR_OEM_DATA_SIZE_UINT64;
    uint8_t retSensorOperationalState = 0;
    uint8_t retPresentReading[8] = {};

    auto rc = decode_get_oem_energy_count_sensor_reading_resp(
        response, payloadLen, &retCompletionCode, &retSensorDataSize,
        &retSensorOperationalState, retPresentReading);

    ASSERT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}
