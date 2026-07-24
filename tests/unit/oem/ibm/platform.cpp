#include <endian.h>
#include <libpldm/base.h>
#include <libpldm/oem/ibm/platform.h>
#include <libpldm/platform.h>

#include <array>
#include <cstdint>
#include <cstring>

#include <gtest/gtest.h>

TEST(EncodeBiosAttributeUpdateEventReq, testGoodEncode)
{
    constexpr uint8_t instanceId = 0;
    constexpr uint8_t formatVersion = 1;
    constexpr uint8_t tid = 1;
    constexpr uint8_t numHandles = 2;
    uint16_t handles[] = {htole16(0x100), htole16(0x200)};

    constexpr size_t payloadLen = PLDM_PLATFORM_EVENT_MESSAGE_MIN_REQ_BYTES +
                                  sizeof(numHandles) +
                                  numHandles * sizeof(uint16_t);
    std::array<uint8_t, sizeof(pldm_msg_hdr) + payloadLen> requestMsg{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto request = reinterpret_cast<pldm_msg*>(requestMsg.data());

    auto rc = encode_bios_attribute_update_event_req(
        instanceId, formatVersion, tid, numHandles,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<uint8_t*>(handles), payloadLen, request);
    EXPECT_EQ(rc, PLDM_SUCCESS);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* req = reinterpret_cast<pldm_bios_attribute_update_event_req*>(
        request->payload);
    EXPECT_EQ(req->format_version, formatVersion);
    EXPECT_EQ(req->tid, tid);
    EXPECT_EQ(req->event_class,
              PLDM_EVENT_TYPE_OEM_EVENT_BIOS_ATTRIBUTE_UPDATE);
    EXPECT_EQ(req->num_handles, numHandles);
}

TEST(EncodeBiosAttributeUpdateEventReq, testBadFormatVersion)
{
    std::array<uint8_t, sizeof(pldm_msg_hdr) + 32> requestMsg{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto request = reinterpret_cast<pldm_msg*>(requestMsg.data());
    uint16_t handle = 1;

    auto rc = encode_bios_attribute_update_event_req(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        0, 0, 1, 1, reinterpret_cast<uint8_t*>(&handle), 32, request);
    EXPECT_EQ(rc, PLDM_ERROR_INVALID_DATA);
}

TEST(EncodeBiosAttributeUpdateEventReq, testBadNullArgs)
{
    uint16_t handle = 1;

    EXPECT_EQ(encode_bios_attribute_update_event_req(
                  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                  0, 1, 1, 1, reinterpret_cast<uint8_t*>(&handle), 32, nullptr),
              PLDM_ERROR_INVALID_DATA);

    std::array<uint8_t, sizeof(pldm_msg_hdr) + 32> requestMsg{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto request = reinterpret_cast<pldm_msg*>(requestMsg.data());

    EXPECT_EQ(encode_bios_attribute_update_event_req(0, 1, 1, 1, nullptr, 32,
                                                     request),
              PLDM_ERROR_INVALID_DATA);
}

TEST(EncodeBiosAttributeUpdateEventReq, testBadZeroHandles)
{
    std::array<uint8_t, sizeof(pldm_msg_hdr) + 32> requestMsg{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto request = reinterpret_cast<pldm_msg*>(requestMsg.data());
    uint16_t handle = 1;

    auto rc = encode_bios_attribute_update_event_req(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        0, 1, 1, 0, reinterpret_cast<uint8_t*>(&handle), 32, request);
    EXPECT_EQ(rc, PLDM_ERROR_INVALID_DATA);
}

TEST(EncodeBiosAttributeUpdateEventReq, testBadPayloadTooSmall)
{
    std::array<uint8_t, sizeof(pldm_msg_hdr) + 4> requestMsg{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto request = reinterpret_cast<pldm_msg*>(requestMsg.data());
    uint16_t handle = 1;

    auto rc = encode_bios_attribute_update_event_req(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        0, 1, 1, 1, reinterpret_cast<uint8_t*>(&handle), 1, request);
    EXPECT_EQ(rc, PLDM_ERROR_INVALID_LENGTH);
}
