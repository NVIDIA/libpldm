/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Test pldm_msgbuf_extract() separately because we can't do _Generic() in C++
 * code, i.e. gtest */

#include <endian.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

/* We're exercising the implementation so disable the asserts for now */
#ifndef NDEBUG
#define NDEBUG 1
#endif

#include <libpldm/compiler.h>

/*
 * These tests exercise the msgbuf implementation itself. Emitting the helpers
 * as noinline test-local functions avoids per-call-site inline coverage
 * artifacts while leaving the library build unchanged.
 */
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
#pragma push_macro("LIBPLDM_CC_ALWAYS_INLINE")
#define PLDM_MSGBUF_RESTORE_ALWAYS_INLINE
#endif
#undef LIBPLDM_CC_ALWAYS_INLINE
#define LIBPLDM_CC_ALWAYS_INLINE static __attribute__((noinline, unused))

#include "msgbuf.h"
#include "msgbuf/platform.h"

#ifdef PLDM_MSGBUF_RESTORE_ALWAYS_INLINE
#pragma pop_macro("LIBPLDM_CC_ALWAYS_INLINE")
#endif

/* Given we disabled asserts above, set up our own expectation framework */
#define expect(cond) __expect(__func__, __LINE__, (cond))
#define __expect(fn, line, cond)                                               \
    do                                                                         \
    {                                                                          \
        if (!(cond))                                                           \
        {                                                                      \
            fprintf(stderr, "%s:%d: failed expectation: %s\n", fn, line,       \
                    #cond);                                                    \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)

static void test_msgbuf_extract_generic_uint8(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    uint8_t buf[1] = {0xa5};
    uint8_t val;

    expect(pldm_msgbuf_init_errno(ctx, sizeof(buf), buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == 0);
    expect(val == 0xa5);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_extract_generic_int8(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    int8_t buf[1] = {-1};
    int8_t val;

    expect(pldm_msgbuf_init_errno(ctx, sizeof(buf), buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == 0);
    expect(val == -1);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_extract_generic_uint16(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    uint16_t buf[1] = {0x5aa5};
    uint16_t val;

    expect(pldm_msgbuf_init_errno(ctx, sizeof(buf), buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == 0);
    expect(val == 0x5aa5);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_extract_generic_int16(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    int16_t buf[1] = {(int16_t)(htole16((uint16_t)INT16_MIN))};
    int16_t val;

    expect(pldm_msgbuf_init_errno(ctx, sizeof(buf), buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == 0);
    expect(val == INT16_MIN);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_extract_generic_uint32(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    uint32_t buf[1] = {0x5a00ffa5};
    uint32_t val;

    expect(pldm_msgbuf_init_errno(ctx, sizeof(buf), buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == 0);
    expect(val == 0x5a00ffa5);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_extract_generic_int32(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    int32_t buf[1] = {(int32_t)(htole32((uint32_t)INT32_MIN))};
    int32_t val;

    expect(pldm_msgbuf_init_errno(ctx, sizeof(buf), buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == 0);
    expect(val == INT32_MIN);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_extract_generic_real32(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    uint32_t buf[1];
    uint32_t xform;
    real32_t val;

    val = FLT_MAX;
    memcpy(&xform, &val, sizeof(val));
    buf[0] = htole32(xform);
    val = 0;

    expect(pldm_msgbuf_init_errno(ctx, sizeof(buf), buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == 0);
    expect(val == FLT_MAX);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_extract_array_generic_uint8(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    uint32_t buf[1] = {0};
    uint8_t arr[1];

    expect(pldm_msgbuf_init_errno(ctx, sizeof(buf), buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract_array(ctx, 1, arr, 1) == 0);
    expect(arr[0] == 0);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_insert_generic_real32(void)
{
    struct pldm_msgbuf_rw _ctx;
    struct pldm_msgbuf_rw* ctx = &_ctx;
    real32_t src = 2.5f;
    real32_t checkVal = 0;
    uint8_t buf[sizeof(real32_t)] = {0};

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_insert(ctx, src) == 0);

    struct pldm_msgbuf_ro _ctxExtract;
    struct pldm_msgbuf_ro* ctxExtract = &_ctxExtract;

    expect(pldm_msgbuf_init_errno(ctxExtract, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctxExtract, checkVal) == 0);

    expect(src == checkVal);
    expect(pldm_msgbuf_complete(ctxExtract) == 0);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_insert_generic_int32(void)
{
    struct pldm_msgbuf_rw _ctx;
    struct pldm_msgbuf_rw* ctx = &_ctx;
    int32_t src = -12345;
    int32_t checkVal = 0;
    uint8_t buf[sizeof(int32_t)] = {0};

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_insert(ctx, src) == 0);

    struct pldm_msgbuf_ro _ctxExtract;
    struct pldm_msgbuf_ro* ctxExtract = &_ctxExtract;

    expect(pldm_msgbuf_init_errno(ctxExtract, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctxExtract, checkVal) == 0);

    expect(src == checkVal);
    expect(pldm_msgbuf_complete(ctxExtract) == 0);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_insert_generic_uint32(void)
{
    struct pldm_msgbuf_rw _ctx;
    struct pldm_msgbuf_rw* ctx = &_ctx;
    uint32_t src = 0xf1223344;
    uint32_t checkVal = 0;
    uint8_t buf[sizeof(uint32_t)] = {0};

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_insert(ctx, src) == 0);

    struct pldm_msgbuf_ro _ctxExtract;
    struct pldm_msgbuf_ro* ctxExtract = &_ctxExtract;

    expect(pldm_msgbuf_init_errno(ctxExtract, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctxExtract, checkVal) == 0);

    expect(src == checkVal);
    expect(pldm_msgbuf_complete(ctxExtract) == 0);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_insert_generic_uint16(void)
{
    struct pldm_msgbuf_rw _ctx;
    struct pldm_msgbuf_rw* ctx = &_ctx;
    uint16_t src = 0xf344;
    uint16_t checkVal = 0;
    uint8_t buf[sizeof(uint16_t)] = {0};

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(uint16_t)) == 0);
    expect(pldm_msgbuf_insert(ctx, src) == 0);

    struct pldm_msgbuf_ro _ctxExtract;
    struct pldm_msgbuf_ro* ctxExtract = &_ctxExtract;

    expect(pldm_msgbuf_init_errno(ctxExtract, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctxExtract, checkVal) == 0);

    expect(src == checkVal);
    expect(pldm_msgbuf_complete(ctxExtract) == 0);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_insert_generic_int16(void)
{
    struct pldm_msgbuf_rw _ctx;
    struct pldm_msgbuf_rw* ctx = &_ctx;
    int16_t src = -12;
    int16_t checkVal = 0;
    uint8_t buf[sizeof(int16_t)] = {0};

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(uint16_t)) == 0);
    expect(pldm_msgbuf_insert(ctx, src) == 0);

    struct pldm_msgbuf_ro _ctxExtract;
    struct pldm_msgbuf_ro* ctxExtract = &_ctxExtract;

    expect(pldm_msgbuf_init_errno(ctxExtract, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctxExtract, checkVal) == 0);

    expect(src == checkVal);
    expect(pldm_msgbuf_complete(ctxExtract) == 0);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_insert_generic_uint8(void)
{
    struct pldm_msgbuf_rw _ctx;
    struct pldm_msgbuf_rw* ctx = &_ctx;
    uint8_t src = 0xf4;
    uint8_t checkVal = 0;
    uint8_t buf[sizeof(uint8_t)] = {0};

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_insert(ctx, src) == 0);

    struct pldm_msgbuf_ro _ctxExtract;
    struct pldm_msgbuf_ro* ctxExtract = &_ctxExtract;

    expect(pldm_msgbuf_init_errno(ctxExtract, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctxExtract, checkVal) == 0);

    expect(src == checkVal);
    expect(pldm_msgbuf_complete(ctxExtract) == 0);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_insert_generic_int8(void)
{
    struct pldm_msgbuf_rw _ctx;
    struct pldm_msgbuf_rw* ctx = &_ctx;
    int8_t src = -4;
    int8_t checkVal = 0;
    uint8_t buf[sizeof(int8_t)] = {0};

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_insert(ctx, src) == 0);

    struct pldm_msgbuf_ro _ctxExtract;
    struct pldm_msgbuf_ro* ctxExtract = &_ctxExtract;

    expect(pldm_msgbuf_init_errno(ctxExtract, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctxExtract, checkVal) == 0);

    expect(src == checkVal);
    expect(pldm_msgbuf_complete(ctxExtract) == 0);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_insert_array_generic_uint8(void)
{
    struct pldm_msgbuf_rw _ctx;
    struct pldm_msgbuf_rw* ctx = &_ctx;
    uint8_t src[6] = {0x11, 0x22, 0x44, 0x55, 0x66, 0x77};
    uint8_t buf[6] = {0};
    uint8_t retBuff[6] = {0};

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_insert_array(ctx, sizeof(src), src, sizeof(src)) == 0);

    struct pldm_msgbuf_ro _ctxExtract;
    struct pldm_msgbuf_ro* ctxExtract = &_ctxExtract;

    expect(pldm_msgbuf_init_errno(ctxExtract, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract_array(ctxExtract, sizeof(retBuff), retBuff,
                                     sizeof(retBuff)) == 0);

    expect(memcmp(src, retBuff, sizeof(retBuff)) == 0);
    expect(pldm_msgbuf_complete(ctxExtract) == 0);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_extract_generic_uint64(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    uint64_t buf[1] = {htole64(UINT64_C(0x1122334455667788))};
    uint64_t val = 0;

    expect(pldm_msgbuf_init_errno(ctx, sizeof(buf), buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == 0);
    expect(val == UINT64_C(0x1122334455667788));
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_extract_generic_int64(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    int64_t src = -INT64_C(0x0102030405060708);
    uint64_t buf[1] = {htole64((uint64_t)src)};
    int64_t val = 0;

    expect(pldm_msgbuf_init_errno(ctx, sizeof(buf), buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == 0);
    expect(val == src);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

#if HAVE_LIBPLDM_ABI_STABLE
static void test_msgbuf_extract_generic_uint64_errors(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    uint64_t buf[1] = {0};
    uint64_t val = 0;

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == -EOVERFLOW);
    expect(pldm_msgbuf_complete(ctx) == -EOVERFLOW);

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
    ctx->remaining = INTMAX_MIN + (intmax_t)sizeof(val) - 1;
    expect(pldm_msgbuf_extract(ctx, val) == -EOVERFLOW);
    expect(pldm_msgbuf_complete(ctx) == -EOVERFLOW);
}

static void test_msgbuf_extract_generic_int64_errors(void)
{
    struct pldm_msgbuf_ro _ctx;
    struct pldm_msgbuf_ro* ctx = &_ctx;
    int64_t buf[1] = {0};
    int64_t val = 0;

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
    expect(pldm_msgbuf_extract(ctx, val) == -EOVERFLOW);
    expect(pldm_msgbuf_complete(ctx) == -EOVERFLOW);

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
    ctx->remaining = INTMAX_MIN + (intmax_t)sizeof(val) - 1;
    expect(pldm_msgbuf_extract(ctx, val) == -EOVERFLOW);
    expect(pldm_msgbuf_complete(ctx) == -EOVERFLOW);
}
#endif

static void test_msgbuf_insert_generic_uint64(void)
{
    struct pldm_msgbuf_rw _ctx;
    struct pldm_msgbuf_rw* ctx = &_ctx;
    uint64_t src = UINT64_C(0x1122334455667788);
    uint64_t checkVal = 0;
    uint8_t buf[sizeof(uint64_t)] = {0};

    expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_insert(ctx, src) == 0);

    struct pldm_msgbuf_ro _ctxExtract;
    struct pldm_msgbuf_ro* ctxExtract = &_ctxExtract;
    expect(pldm_msgbuf_init_errno(ctxExtract, 0, buf, sizeof(buf)) == 0);
    expect(pldm_msgbuf_extract(ctxExtract, checkVal) == 0);
    expect(checkVal == src);
    expect(pldm_msgbuf_complete(ctxExtract) == 0);
    expect(pldm_msgbuf_complete(ctx) == 0);
}

static void test_msgbuf_platform_extract_value_pdr_hdr(void)
{
    {
        uint8_t wire[sizeof(struct pldm_value_pdr_hdr)] = {0};
        struct pldm_msgbuf_rw _enc;
        struct pldm_msgbuf_rw* enc = &_enc;
        struct pldm_msgbuf_ro _dec;
        struct pldm_msgbuf_ro* dec = &_dec;
        struct pldm_value_pdr_hdr hdr = {0};

        expect(pldm_msgbuf_init_errno(enc, 0, wire, sizeof(wire)) == 0);
        expect(pldm_msgbuf_insert_uint32(enc, UINT32_C(0x10203040)) == 0);
        expect(pldm_msgbuf_insert_uint8(enc, 0x22) == 0);
        expect(pldm_msgbuf_insert_uint8(enc, 0x23) == 0);
        expect(pldm_msgbuf_insert_uint16(enc, 0x3344) == 0);
        expect(pldm_msgbuf_insert_uint16(enc, 0x0010) == 0);
        expect(pldm_msgbuf_complete(enc) == 0);

        expect(pldm_msgbuf_init_errno(dec, 0, wire, sizeof(wire)) == 0);
        expect(pldm_msgbuf_extract_value_pdr_hdr(
                   dec, &hdr, sizeof(struct pldm_msgbuf_rw) + 0x0010, 0x0010) ==
               0);
        expect(hdr.record_handle == UINT32_C(0x10203040));
        expect(hdr.version == 0x22);
        expect(hdr.type == 0x23);
        expect(hdr.record_change_num == 0x3344);
        expect(hdr.length == 0x0010);
        expect(pldm_msgbuf_complete(dec) == 0);
    }

    {
        uint8_t wire[5] = {0};
        struct pldm_msgbuf_ro _dec;
        struct pldm_msgbuf_ro* dec = &_dec;
        struct pldm_value_pdr_hdr hdr = {0};

        expect(pldm_msgbuf_init_errno(dec, 0, wire, sizeof(wire)) == 0);
        expect(pldm_msgbuf_extract_value_pdr_hdr(dec, &hdr, 0, UINT16_MAX) ==
               -EOVERFLOW);
        expect(pldm_msgbuf_complete(dec) == -EOVERFLOW);
    }

    {
        uint8_t wire[sizeof(struct pldm_value_pdr_hdr)] = {0};
        struct pldm_msgbuf_rw _enc;
        struct pldm_msgbuf_rw* enc = &_enc;
        struct pldm_msgbuf_ro _dec;
        struct pldm_msgbuf_ro* dec = &_dec;
        struct pldm_value_pdr_hdr hdr = {0};

        expect(pldm_msgbuf_init_errno(enc, 0, wire, sizeof(wire)) == 0);
        expect(pldm_msgbuf_insert_uint32(enc, UINT32_C(0x10203040)) == 0);
        expect(pldm_msgbuf_insert_uint8(enc, 0x22) == 0);
        expect(pldm_msgbuf_insert_uint8(enc, 0x23) == 0);
        expect(pldm_msgbuf_insert_uint16(enc, 0x3344) == 0);
        expect(pldm_msgbuf_insert_uint16(enc, 0x0001) == 0);
        expect(pldm_msgbuf_complete(enc) == 0);

        expect(pldm_msgbuf_init_errno(dec, 0, wire, sizeof(wire)) == 0);
        expect(pldm_msgbuf_extract_value_pdr_hdr(
                   dec, &hdr, sizeof(struct pldm_msgbuf_rw) + 0x0002, 0x0010) ==
               -EOVERFLOW);
        expect(pldm_msgbuf_complete(dec) == 0);
    }

    {
        uint8_t wire[sizeof(struct pldm_value_pdr_hdr)] = {0};
        struct pldm_msgbuf_rw _enc;
        struct pldm_msgbuf_rw* enc = &_enc;
        struct pldm_msgbuf_ro _dec;
        struct pldm_msgbuf_ro* dec = &_dec;
        struct pldm_value_pdr_hdr hdr = {0};

        expect(pldm_msgbuf_init_errno(enc, 0, wire, sizeof(wire)) == 0);
        expect(pldm_msgbuf_insert_uint32(enc, UINT32_C(0x10203040)) == 0);
        expect(pldm_msgbuf_insert_uint8(enc, 0x22) == 0);
        expect(pldm_msgbuf_insert_uint8(enc, 0x23) == 0);
        expect(pldm_msgbuf_insert_uint16(enc, 0x3344) == 0);
        expect(pldm_msgbuf_insert_uint16(enc, 0x0020) == 0);
        expect(pldm_msgbuf_complete(enc) == 0);

        expect(pldm_msgbuf_init_errno(dec, 0, wire, sizeof(wire)) == 0);
        expect(pldm_msgbuf_extract_value_pdr_hdr(dec, &hdr, 0, 0x0010) ==
               -EOVERFLOW);
        expect(pldm_msgbuf_complete(dec) == 0);
    }
}

static void test_msgbuf_platform_extract_sensor_data(void)
{
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_sensor_data_size value = {0};
        uint8_t buf[1] = {0x7f};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_data(ctx, PLDM_SENSOR_DATA_SIZE_UINT8,
                                               &value) == 0);
        expect(value.value_u8 == 0x7f);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_sensor_data_size value = {0};
        int8_t buf[1] = {-1};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_data(ctx, PLDM_SENSOR_DATA_SIZE_SINT8,
                                               &value) == 0);
        expect(value.value_s8 == -1);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_sensor_data_size value = {0};
        uint16_t buf[1] = {htole16(0x1234)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_data(
                   ctx, PLDM_SENSOR_DATA_SIZE_UINT16, &value) == 0);
        expect(value.value_u16 == 0x1234);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_sensor_data_size value = {0};
        int16_t buf[1] = {(int16_t)htole16((uint16_t)-1234)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_data(
                   ctx, PLDM_SENSOR_DATA_SIZE_SINT16, &value) == 0);
        expect(value.value_s16 == -1234);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_sensor_data_size value = {0};
        uint32_t buf[1] = {htole32(UINT32_C(0x55667788))};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_data(
                   ctx, PLDM_SENSOR_DATA_SIZE_UINT32, &value) == 0);
        expect(value.value_u32 == UINT32_C(0x55667788));
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_sensor_data_size value = {0};
        int32_t buf[1] = {(int32_t)htole32((uint32_t)-12345678)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_data(
                   ctx, PLDM_SENSOR_DATA_SIZE_SINT32, &value) == 0);
        expect(value.value_s32 == -12345678);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_sensor_data_size value = {0};
        uint64_t buf[1] = {htole64(UINT64_C(0x1122334455667788))};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_data(
                   ctx, PLDM_SENSOR_DATA_SIZE_UINT64, &value) == 0);
        expect(value.value_u64 == UINT64_C(0x1122334455667788));
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_sensor_data_size value = {0};
        int64_t src = -INT64_C(0x0102030405060708);
        uint64_t buf[1] = {htole64((uint64_t)src)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_data(
                   ctx, PLDM_SENSOR_DATA_SIZE_SINT64, &value) == 0);
        expect(value.value_s64 == src);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_sensor_data_size value = {0};
        uint8_t buf[1] = {0};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
        expect(pldm_msgbuf_extract_sensor_data(
                   ctx, (enum pldm_sensor_readings_data_type)0xff, &value) ==
               -PLDM_ERROR_INVALID_DATA);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
}

static void test_msgbuf_platform_extract_sensor_value(void)
{
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        uint8_t val = 0;
        uint8_t buf[1] = {0x7f};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_value(
                   ctx, PLDM_SENSOR_DATA_SIZE_UINT8, &val) == 0);
        expect(val == 0x7f);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        int8_t val = 0;
        int8_t buf[1] = {-1};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_value(
                   ctx, PLDM_SENSOR_DATA_SIZE_SINT8, &val) == 0);
        expect(val == -1);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        uint16_t val = 0;
        uint16_t buf[1] = {htole16(0x1234)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_value(
                   ctx, PLDM_SENSOR_DATA_SIZE_UINT16, &val) == 0);
        expect(val == 0x1234);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        int16_t val = 0;
        int16_t buf[1] = {(int16_t)htole16((uint16_t)-1234)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_value(
                   ctx, PLDM_SENSOR_DATA_SIZE_SINT16, &val) == 0);
        expect(val == -1234);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        uint32_t val = 0;
        uint32_t buf[1] = {htole32(UINT32_C(0x55667788))};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_value(
                   ctx, PLDM_SENSOR_DATA_SIZE_UINT32, &val) == 0);
        expect(val == UINT32_C(0x55667788));
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        int32_t val = 0;
        int32_t buf[1] = {(int32_t)htole32((uint32_t)-12345678)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_value(
                   ctx, PLDM_SENSOR_DATA_SIZE_SINT32, &val) == 0);
        expect(val == -12345678);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        uint64_t val = 0;
        uint64_t buf[1] = {htole64(UINT64_C(0x1122334455667788))};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_value(
                   ctx, PLDM_SENSOR_DATA_SIZE_UINT64, &val) == 0);
        expect(val == UINT64_C(0x1122334455667788));
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        int64_t val = 0;
        int64_t src = -INT64_C(0x0102030405060708);
        uint64_t buf[1] = {htole64((uint64_t)src)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_sensor_value(
                   ctx, PLDM_SENSOR_DATA_SIZE_SINT64, &val) == 0);
        expect(val == src);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        uint8_t val = 0;
        uint8_t buf[1] = {0};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
        expect(pldm_msgbuf_extract_sensor_value(
                   ctx, (enum pldm_sensor_readings_data_type)0xff, &val) ==
               -PLDM_ERROR_INVALID_DATA);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
}

static void test_msgbuf_platform_extract_range_field_format(void)
{
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format value = {0};
        uint8_t buf[1] = {0x7f};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_range_field_format(
                   ctx, PLDM_RANGE_FIELD_FORMAT_UINT8, &value) == 0);
        expect(value.value_u8 == 0x7f);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format value = {0};
        int8_t buf[1] = {-1};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_range_field_format(
                   ctx, PLDM_RANGE_FIELD_FORMAT_SINT8, &value) == 0);
        expect(value.value_s8 == -1);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format value = {0};
        uint16_t buf[1] = {htole16(0x1234)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_range_field_format(
                   ctx, PLDM_RANGE_FIELD_FORMAT_UINT16, &value) == 0);
        expect(value.value_u16 == 0x1234);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format value = {0};
        int16_t buf[1] = {(int16_t)htole16((uint16_t)-1234)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_range_field_format(
                   ctx, PLDM_RANGE_FIELD_FORMAT_SINT16, &value) == 0);
        expect(value.value_s16 == -1234);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format value = {0};
        uint32_t buf[1] = {htole32(UINT32_C(0x55667788))};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_range_field_format(
                   ctx, PLDM_RANGE_FIELD_FORMAT_UINT32, &value) == 0);
        expect(value.value_u32 == UINT32_C(0x55667788));
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format value = {0};
        int32_t buf[1] = {(int32_t)htole32((uint32_t)-12345678)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_range_field_format(
                   ctx, PLDM_RANGE_FIELD_FORMAT_SINT32, &value) == 0);
        expect(value.value_s32 == -12345678);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        real32_t src = FLT_MAX;
        uint32_t raw = 0;
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format value = {0};
        uint32_t buf[1] = {0};
        memcpy(&raw, &src, sizeof(raw));
        buf[0] = htole32(raw);

        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_range_field_format(
                   ctx, PLDM_RANGE_FIELD_FORMAT_REAL32, &value) == 0);
        expect(value.value_f32 == FLT_MAX);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format value = {0};
        uint64_t buf[1] = {htole64(UINT64_C(0x1122334455667788))};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_range_field_format(
                   ctx, PLDM_RANGE_FIELD_FORMAT_UINT64, &value) == 0);
        expect(value.value_u64 == UINT64_C(0x1122334455667788));
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format value = {0};
        int64_t src = -INT64_C(0x0102030405060708);
        uint64_t buf[1] = {htole64((uint64_t)src)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_range_field_format(
                   ctx, PLDM_RANGE_FIELD_FORMAT_SINT64, &value) == 0);
        expect(value.value_s64 == src);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format value = {0};
        uint8_t buf[1] = {0};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
        expect(pldm__msgbuf_extract_range_field_format(
                   ctx, (enum pldm_range_field_format)0xff, &value) ==
               -PLDM_ERROR_INVALID_DATA);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
}

static void test_msgbuf_platform_extract_effecter_value(void)
{
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        uint8_t val = 0;
        uint8_t buf[1] = {0x7f};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_effecter_value(
                   ctx, PLDM_EFFECTER_DATA_SIZE_UINT8, &val) == 0);
        expect(val == 0x7f);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        int8_t val = 0;
        int8_t buf[1] = {-1};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_effecter_value(
                   ctx, PLDM_EFFECTER_DATA_SIZE_SINT8, &val) == 0);
        expect(val == -1);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        uint16_t val = 0;
        uint16_t buf[1] = {htole16(0x1234)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_effecter_value(
                   ctx, PLDM_EFFECTER_DATA_SIZE_UINT16, &val) == 0);
        expect(val == 0x1234);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        int16_t val = 0;
        int16_t buf[1] = {(int16_t)htole16((uint16_t)-1234)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_effecter_value(
                   ctx, PLDM_EFFECTER_DATA_SIZE_SINT16, &val) == 0);
        expect(val == -1234);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        uint32_t val = 0;
        uint32_t buf[1] = {htole32(UINT32_C(0x55667788))};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_effecter_value(
                   ctx, PLDM_EFFECTER_DATA_SIZE_UINT32, &val) == 0);
        expect(val == UINT32_C(0x55667788));
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        int32_t val = 0;
        int32_t buf[1] = {(int32_t)htole32((uint32_t)-12345678)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_effecter_value(
                   ctx, PLDM_EFFECTER_DATA_SIZE_SINT32, &val) == 0);
        expect(val == -12345678);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        uint64_t val = 0;
        uint64_t buf[1] = {htole64(UINT64_C(0x1122334455667788))};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_effecter_value(
                   ctx, PLDM_EFFECTER_DATA_SIZE_UINT64, &val) == 0);
        expect(val == UINT64_C(0x1122334455667788));
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        int64_t val = 0;
        int64_t src = -INT64_C(0x0102030405060708);
        uint64_t buf[1] = {htole64((uint64_t)src)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm_msgbuf_extract_effecter_value(
                   ctx, PLDM_EFFECTER_DATA_SIZE_SINT64, &val) == 0);
        expect(val == src);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        uint8_t val = 0;
        uint8_t buf[1] = {0};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
        expect(pldm_msgbuf_extract_effecter_value(
                   ctx, (enum pldm_effecter_data_size)0xff, &val) ==
               -PLDM_ERROR_INVALID_DATA);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
}

static void test_msgbuf_platform_extract_effecter_data(void)
{
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_effecter_data_size value = {0};
        uint8_t buf[1] = {0x7f};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_effecter_data(
                   ctx, PLDM_EFFECTER_DATA_SIZE_UINT8, &value) == 0);
        expect(value.value_u8 == 0x7f);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_effecter_data_size value = {0};
        int8_t buf[1] = {-1};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_effecter_data(
                   ctx, PLDM_EFFECTER_DATA_SIZE_SINT8, &value) == 0);
        expect(value.value_s8 == -1);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_effecter_data_size value = {0};
        uint16_t buf[1] = {htole16(0x1234)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_effecter_data(
                   ctx, PLDM_EFFECTER_DATA_SIZE_UINT16, &value) == 0);
        expect(value.value_u16 == 0x1234);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_effecter_data_size value = {0};
        int16_t buf[1] = {(int16_t)htole16((uint16_t)-1234)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_effecter_data(
                   ctx, PLDM_EFFECTER_DATA_SIZE_SINT16, &value) == 0);
        expect(value.value_s16 == -1234);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_effecter_data_size value = {0};
        uint32_t buf[1] = {htole32(UINT32_C(0x55667788))};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_effecter_data(
                   ctx, PLDM_EFFECTER_DATA_SIZE_UINT32, &value) == 0);
        expect(value.value_u32 == UINT32_C(0x55667788));
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_effecter_data_size value = {0};
        int32_t buf[1] = {(int32_t)htole32((uint32_t)-12345678)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_effecter_data(
                   ctx, PLDM_EFFECTER_DATA_SIZE_SINT32, &value) == 0);
        expect(value.value_s32 == -12345678);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_effecter_data_size value = {0};
        uint64_t buf[1] = {htole64(UINT64_C(0x1122334455667788))};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_effecter_data(
                   ctx, PLDM_EFFECTER_DATA_SIZE_UINT64, &value) == 0);
        expect(value.value_u64 == UINT64_C(0x1122334455667788));
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_effecter_data_size value = {0};
        int64_t src = -INT64_C(0x0102030405060708);
        uint64_t buf[1] = {htole64((uint64_t)src)};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)) == 0);
        expect(pldm__msgbuf_extract_effecter_data(
                   ctx, PLDM_EFFECTER_DATA_SIZE_SINT64, &value) == 0);
        expect(value.value_s64 == src);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_effecter_data_size value = {0};
        uint8_t buf[1] = {0};
        expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
        expect(pldm__msgbuf_extract_effecter_data(
                   ctx, (enum pldm_effecter_data_size)0xff, &value) ==
               -PLDM_ERROR_INVALID_DATA);
        expect(pldm_msgbuf_complete(ctx) == 0);
    }
}

#if HAVE_LIBPLDM_ABI_STABLE
static void test_msgbuf_platform_extract_overflow_paths(void)
{
    const enum pldm_sensor_readings_data_type sensor_tags[] = {
        PLDM_SENSOR_DATA_SIZE_UINT8,  PLDM_SENSOR_DATA_SIZE_SINT8,
        PLDM_SENSOR_DATA_SIZE_UINT16, PLDM_SENSOR_DATA_SIZE_SINT16,
        PLDM_SENSOR_DATA_SIZE_UINT32, PLDM_SENSOR_DATA_SIZE_SINT32,
        PLDM_SENSOR_DATA_SIZE_UINT64, PLDM_SENSOR_DATA_SIZE_SINT64,
    };
    const enum pldm_range_field_format range_tags[] = {
        PLDM_RANGE_FIELD_FORMAT_UINT8,  PLDM_RANGE_FIELD_FORMAT_SINT8,
        PLDM_RANGE_FIELD_FORMAT_UINT16, PLDM_RANGE_FIELD_FORMAT_SINT16,
        PLDM_RANGE_FIELD_FORMAT_UINT32, PLDM_RANGE_FIELD_FORMAT_SINT32,
        PLDM_RANGE_FIELD_FORMAT_REAL32, PLDM_RANGE_FIELD_FORMAT_UINT64,
        PLDM_RANGE_FIELD_FORMAT_SINT64,
    };
    const enum pldm_effecter_data_size effecter_tags[] = {
        PLDM_EFFECTER_DATA_SIZE_UINT8,  PLDM_EFFECTER_DATA_SIZE_SINT8,
        PLDM_EFFECTER_DATA_SIZE_UINT16, PLDM_EFFECTER_DATA_SIZE_SINT16,
        PLDM_EFFECTER_DATA_SIZE_UINT32, PLDM_EFFECTER_DATA_SIZE_SINT32,
        PLDM_EFFECTER_DATA_SIZE_UINT64, PLDM_EFFECTER_DATA_SIZE_SINT64,
    };
    uint8_t buf[1] = {0};

    for (size_t i = 0; i < sizeof(sensor_tags) / sizeof(sensor_tags[0]); i++)
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_sensor_data_size sensor = {0};
        uint64_t value = 0;

        expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
        expect(pldm_msgbuf_extract_sensor_data(ctx, sensor_tags[i], &sensor) ==
               -EOVERFLOW);
        expect(pldm_msgbuf_complete(ctx) == -EOVERFLOW);

        expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
        expect(pldm_msgbuf_extract_sensor_value(ctx, sensor_tags[i], &value) ==
               -EOVERFLOW);
        expect(pldm_msgbuf_complete(ctx) == -EOVERFLOW);
    }

    for (size_t i = 0; i < sizeof(range_tags) / sizeof(range_tags[0]); i++)
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_range_field_format range = {0};

        expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
        expect(pldm__msgbuf_extract_range_field_format(ctx, range_tags[i],
                                                       &range) == -EOVERFLOW);
        expect(pldm_msgbuf_complete(ctx) == -EOVERFLOW);
    }

    for (size_t i = 0; i < sizeof(effecter_tags) / sizeof(effecter_tags[0]);
         i++)
    {
        struct pldm_msgbuf_ro _ctx;
        struct pldm_msgbuf_ro* ctx = &_ctx;
        union_effecter_data_size effecter = {0};
        uint64_t value = 0;

        expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
        expect(pldm_msgbuf_extract_effecter_value(ctx, effecter_tags[i],
                                                  &value) == -EOVERFLOW);
        expect(pldm_msgbuf_complete(ctx) == -EOVERFLOW);

        expect(pldm_msgbuf_init_errno(ctx, 0, buf, 0) == 0);
        expect(pldm__msgbuf_extract_effecter_data(ctx, effecter_tags[i],
                                                  &effecter) == -EOVERFLOW);
        expect(pldm_msgbuf_complete(ctx) == -EOVERFLOW);
    }
}
#endif

typedef void (*testfn)(void);

static const testfn tests[] = {test_msgbuf_extract_generic_uint8,
                               test_msgbuf_extract_generic_int8,
                               test_msgbuf_extract_generic_uint16,
                               test_msgbuf_extract_generic_int16,
                               test_msgbuf_extract_generic_uint32,
                               test_msgbuf_extract_generic_int32,
                               test_msgbuf_extract_generic_uint64,
                               test_msgbuf_extract_generic_int64,
#if HAVE_LIBPLDM_ABI_STABLE
                               test_msgbuf_extract_generic_uint64_errors,
                               test_msgbuf_extract_generic_int64_errors,
#endif
                               test_msgbuf_extract_generic_real32,
                               test_msgbuf_extract_array_generic_uint8,
                               test_msgbuf_insert_generic_uint8,
                               test_msgbuf_insert_generic_int8,
                               test_msgbuf_insert_generic_uint16,
                               test_msgbuf_insert_generic_int16,
                               test_msgbuf_insert_generic_uint32,
                               test_msgbuf_insert_generic_int32,
                               test_msgbuf_insert_generic_uint64,
                               test_msgbuf_insert_generic_real32,
                               test_msgbuf_insert_array_generic_uint8,
                               test_msgbuf_platform_extract_value_pdr_hdr,
                               test_msgbuf_platform_extract_sensor_data,
                               test_msgbuf_platform_extract_sensor_value,
                               test_msgbuf_platform_extract_range_field_format,
                               test_msgbuf_platform_extract_effecter_value,
                               test_msgbuf_platform_extract_effecter_data,
#if HAVE_LIBPLDM_ABI_STABLE
                               test_msgbuf_platform_extract_overflow_paths,
#endif
                               NULL};

int main(void)
{
    testfn const* testp = &tests[0];

    while (*testp)
    {
        (*testp)();
        testp++;
    }

    return 0;
}
