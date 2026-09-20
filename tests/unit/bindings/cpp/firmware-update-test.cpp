#include "testpkg-util.hpp"

#include <libpldm/api.h>

#include <expected>
#include <libpldm++/firmware_update.hpp>
#include <libpldm++/types.hpp>
#include <span>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace pldm::fw_update;

namespace pldm
{
namespace fw_update
{

TEST(PackageParserTest, ValidPkgSingleDescriptorSingleComponent)
{
    std::vector<uint8_t> pkg;

    appendPackageHeaderIdentifier(pkg, pldm::fw_update::PackagePin::v1);

    // pkg header size
    appendLE16(pkg, 0x8b);

    // pkg release date time (13 bytes, timestamp104)
    appendTimestamp104(pkg);

    // component bitmap bit length
    appendLE16(pkg, 0x08);

    // package version string
    appendTypeLengthString(
        pkg, std::vector<uint8_t>{0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E,
                                  0x53, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x31});

    // device id record count
    pkg.push_back(0x01);

    appendFirmwareDeviceIdRecord1(pkg);

    appendComponentImageInfoArea1(pkg);

    appendCRC(pkg);

    // component image
    pkg.push_back(0x00);

    auto res = PackageParser::parse(pkg, PackagePin::v1);

    if (!res.has_value())
    {
        std::cout << res.error().msg << std::endl;
    }

    ASSERT_TRUE(res.has_value());

    const auto& outfwDeviceIDRecords = res.value()->firmwareDeviceIdRecords;

    std::vector<uint8_t> dd1Data{0x16, 0x20, 0x23, 0xC9, 0x3E, 0xC5,
                                 0x41, 0x15, 0x95, 0xF4, 0x48, 0x70,
                                 0x1D, 0x49, 0xD6, 0x75};

    ASSERT_EQ(outfwDeviceIDRecords.size(), 1);

    // we cannot compare the applicable components here since their memory
    // address will be different
    EXPECT_EQ(outfwDeviceIDRecords[0].deviceUpdateOptionFlags,
              std::bitset<32>(1));
    EXPECT_EQ(outfwDeviceIDRecords[0].componentImageSetVersionString,
              "VersionString2");

#if HAVE_LIBPLDM_API_TESTING
    EXPECT_EQ(outfwDeviceIDRecords[0].getDescriptorTypes(),
              std::vector<uint16_t>({PLDM_FWUP_UUID}));
#endif

    // assert for descriptor type PLDM_FWUP_UUID
    const auto& d1 =
        outfwDeviceIDRecords[0].recordDescriptors.at(PLDM_FWUP_UUID);
    EXPECT_EQ(d1->data, dd1Data);
    EXPECT_EQ(d1->vendorDefinedDescriptorTitle, std::nullopt);

    EXPECT_EQ(outfwDeviceIDRecords[0].firmwareDevicePackageData,
              std::vector<uint8_t>{});

    const auto& outCompImageInfos = res.value()->componentImageInformation;

    ASSERT_EQ(outCompImageInfos.size(), 1);

    EXPECT_EQ(outCompImageInfos[0].componentClassification, 10);
    EXPECT_EQ(outCompImageInfos[0].componentIdentifier, 100);
    EXPECT_EQ(outCompImageInfos[0].compComparisonStamp, 0xFFFFFFFF);
    EXPECT_EQ(outCompImageInfos[0].componentOptions, 0);
    EXPECT_EQ(outCompImageInfos[0].requestedComponentActivationMethod, 0);
    EXPECT_EQ(outCompImageInfos[0].componentLocation.length, 1);
    EXPECT_EQ(outCompImageInfos[0].componentVersion, "VersionString3");
}

TEST(PackageParserTest, ValidPkgMultipleDescriptorsMultipleComponents)
{
    std::vector<uint8_t> pkg;

    appendPackageHeaderIdentifier(pkg, pldm::fw_update::PackagePin::v1);

    // pkg header size
    appendLE16(pkg, 0x0146);

    // pkg release date time, 13 bytes, timestamp104
    appendTimestamp104(pkg);

    // component bitmap bit length
    appendLE16(pkg, 0x08);

    // package version string
    appendTypeLengthString(
        pkg, std::vector<uint8_t>{0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E,
                                  0x53, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x31});

    appendFirmwareDeviceIdArea1(pkg);

    appendComponentImageInfoArea2(pkg);

    appendCRC(pkg);

    // component image
    pkg.push_back(0x00);

    auto res = PackageParser::parse(pkg, PackagePin::v1);

    if (!res.has_value())
    {
        std::cout << res.error().msg << std::endl;
    }

    ASSERT_TRUE(res.has_value());

    const std::vector<FirmwareDeviceIDRecord>& outfwDeviceIDRecords =
        res.value()->firmwareDeviceIdRecords;

    std::vector<uint8_t> dd1Data{0x12, 0x44, 0xD2, 0x64, 0x8D, 0x7D,
                                 0x47, 0x18, 0xA0, 0x30, 0xFC, 0x8A,
                                 0x56, 0x58, 0x7D, 0x5B};

    std::vector<uint8_t> dd2Data{0x47, 0x16, 0x00, 0x00};

    std::vector<uint8_t> dd3Data{0x12, 0x34};

    std::vector<uint8_t> dd4Data{0x12, 0x44, 0xD2, 0x64, 0x8D, 0x7D,
                                 0x47, 0x18, 0xA0, 0x30, 0xFC, 0x8A,
                                 0x56, 0x58, 0x7D, 0x5C};

    std::vector<uint8_t> dd5Data{0x12, 0x44, 0xD2, 0x64, 0x8D, 0x7D,
                                 0x47, 0x18, 0xA0, 0x30, 0xFC, 0x8A,
                                 0x56, 0x58, 0x7D, 0x5D};

    ASSERT_EQ(outfwDeviceIDRecords.size(), 3);

    // start asserting fw device id records
    // fw device id records index 0 (record 1)

    // we cannot compare the applicable components here since their memory
    // address will be different
    EXPECT_EQ(outfwDeviceIDRecords[0].deviceUpdateOptionFlags,
              std::bitset<32>(1));

    EXPECT_EQ(outfwDeviceIDRecords[0].componentImageSetVersionString,
              "VersionString2");

#if HAVE_LIBPLDM_API_TESTING
    // assert record descriptor types
    const auto types = outfwDeviceIDRecords[0].getDescriptorTypes();
    EXPECT_THAT(types, ::testing::UnorderedElementsAre(
                           PLDM_FWUP_UUID, PLDM_FWUP_IANA_ENTERPRISE_ID,
                           PLDM_FWUP_VENDOR_DEFINED));
#endif

    // assert record descriptor contents
    EXPECT_EQ(
        outfwDeviceIDRecords[0].recordDescriptors.at(PLDM_FWUP_UUID)->data,
        dd1Data);
    EXPECT_EQ(outfwDeviceIDRecords[0]
                  .recordDescriptors.at(PLDM_FWUP_UUID)
                  ->vendorDefinedDescriptorTitle,
              std::nullopt);

    EXPECT_EQ(outfwDeviceIDRecords[0]
                  .recordDescriptors.at(PLDM_FWUP_IANA_ENTERPRISE_ID)
                  ->data,
              dd2Data);
    EXPECT_EQ(outfwDeviceIDRecords[0]
                  .recordDescriptors.at(PLDM_FWUP_IANA_ENTERPRISE_ID)
                  ->vendorDefinedDescriptorTitle,
              std::nullopt);

    EXPECT_EQ(outfwDeviceIDRecords[0]
                  .recordDescriptors.at(PLDM_FWUP_VENDOR_DEFINED)
                  ->data,
              dd3Data);
    EXPECT_EQ(outfwDeviceIDRecords[0]
                  .recordDescriptors.at(PLDM_FWUP_VENDOR_DEFINED)
                  ->vendorDefinedDescriptorTitle,
              "OpenBMC");

    EXPECT_EQ(outfwDeviceIDRecords[0].firmwareDevicePackageData,
              std::vector<uint8_t>{});

    // fw device id records index 1 (record 2)

    // we cannot compare the applicable components here since their memory
    // address will be different
    EXPECT_EQ(outfwDeviceIDRecords[1].deviceUpdateOptionFlags,
              std::bitset<32>(0));
    EXPECT_EQ(outfwDeviceIDRecords[1].componentImageSetVersionString,
              "VersionString3");
#if HAVE_LIBPLDM_API_TESTING
    EXPECT_EQ(outfwDeviceIDRecords[1].getDescriptorTypes(),
              std::vector<uint16_t>{PLDM_FWUP_UUID});
#endif

    const auto& d1 =
        outfwDeviceIDRecords[1].recordDescriptors.at(PLDM_FWUP_UUID);

    EXPECT_EQ(d1->data, dd4Data);
    EXPECT_EQ(d1->vendorDefinedDescriptorTitle, std::nullopt);

    EXPECT_EQ(outfwDeviceIDRecords[1].firmwareDevicePackageData,
              std::vector<uint8_t>{});

    // fw device id records index 2 (record 3)

    // we cannot compare the applicable components here since their memory
    // address will be different
    EXPECT_EQ(outfwDeviceIDRecords[2].deviceUpdateOptionFlags,
              std::bitset<32>(0));
    EXPECT_EQ(outfwDeviceIDRecords[2].componentImageSetVersionString,
              "VersionString4");
#if HAVE_LIBPLDM_API_TESTING
    EXPECT_EQ(outfwDeviceIDRecords[2].getDescriptorTypes(),
              std::vector<uint16_t>{PLDM_FWUP_UUID});
#endif

    // assert for descriptor type PLDM_FWUP_UUID
    const auto& d2 =
        outfwDeviceIDRecords[2].recordDescriptors.at(PLDM_FWUP_UUID);

    EXPECT_EQ(d2->data, dd5Data);
    EXPECT_EQ(d2->vendorDefinedDescriptorTitle, std::nullopt);

    EXPECT_EQ(outfwDeviceIDRecords[2].firmwareDevicePackageData,
              std::vector<uint8_t>{});

    // end asserting fw device id records

    const auto& outCompImageInfos = res.value()->componentImageInformation;

    ASSERT_EQ(outCompImageInfos.size(), 3);

    // start asserting component image info
    // component image info index 0

    EXPECT_EQ(outCompImageInfos[0].componentClassification, 10);
    EXPECT_EQ(outCompImageInfos[0].componentIdentifier, 100);
    EXPECT_EQ(outCompImageInfos[0].compComparisonStamp, 0xFFFFFFFF);
    EXPECT_EQ(outCompImageInfos[0].componentOptions, 0);
    EXPECT_EQ(outCompImageInfos[0].requestedComponentActivationMethod, 0);
    EXPECT_EQ(outCompImageInfos[0].componentLocation.length, 1);
    EXPECT_EQ(outCompImageInfos[0].componentVersion, "VersionString5");

    // component image info index 1

    EXPECT_EQ(outCompImageInfos[1].componentClassification, 10);
    EXPECT_EQ(outCompImageInfos[1].componentIdentifier, 200);
    EXPECT_EQ(outCompImageInfos[1].compComparisonStamp, 0xFFFFFFFF);
    EXPECT_EQ(outCompImageInfos[1].componentOptions, 0);
    EXPECT_EQ(outCompImageInfos[1].requestedComponentActivationMethod, 1);
    EXPECT_EQ(outCompImageInfos[1].componentLocation.length, 1);
    EXPECT_EQ(outCompImageInfos[1].componentVersion, "VersionString6");

    // component image info index 2

    EXPECT_EQ(outCompImageInfos[2].componentClassification, 11);
    EXPECT_EQ(outCompImageInfos[2].componentIdentifier, 300);
    EXPECT_EQ(outCompImageInfos[2].compComparisonStamp, 0xFFFFFFFF);
    EXPECT_EQ(outCompImageInfos[2].componentOptions, 1);
    EXPECT_EQ(outCompImageInfos[2].requestedComponentActivationMethod, 12);
    EXPECT_EQ(outCompImageInfos[2].componentLocation.length, 1);
    EXPECT_EQ(outCompImageInfos[2].componentVersion, "VersionString7");

    // end asserting component image info
}

TEST(PackageParserTest,
     InValidPkgSingleDescriptorSingleComponentApplicableComponentOOB)
{
    // failure: the applicable component is out of bounds
    std::vector<uint8_t> pkg;

    appendPackageHeaderIdentifier(pkg, pldm::fw_update::PackagePin::v1);

    // pkg header size
    appendLE16(pkg, 0x8b);

    // pkg release date time (13 bytes, timestamp104)
    appendTimestamp104(pkg);

    // component bitmap bit length
    appendLE16(pkg, 0x08);

    // package version string
    appendTypeLengthString(
        pkg, std::vector<uint8_t>{0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E,
                                  0x53, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x31});

    // device id record count
    pkg.push_back(0x01);

    // describes an applicable component which does not exist in the package
    appendFirmwareDeviceIdRecord1InvalidApplicableComponentOOB(pkg);

    appendComponentImageInfoArea1(pkg);

    appendCRC(pkg);

    // component image
    pkg.push_back(0x00);

    auto res = PackageParser::parse(pkg, PackagePin::v1);

    if (!res.has_value())
    {
        std::cout << res.error().msg << std::endl;

        EXPECT_EQ(res.error().msg,
                  "applicable component index 3 is out of bounds");
    }

    ASSERT_FALSE(res.has_value());
}

TEST(PackageParserTest, InvalidPkgBadChecksum)
{
    auto fwPkgHdr = fwPkgHdrSingleComponent;
    fwPkgHdr[fwPkgHdr.size() - 5] ^= 0x01;

    std::expected<std::unique_ptr<Package>, PackageParserError> result =
        PackageParser::parse(fwPkgHdr, PackagePin::v1);

    EXPECT_FALSE(result.has_value());
}

} // namespace fw_update
} // namespace pldm
