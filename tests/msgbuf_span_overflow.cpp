/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/*
 * Exercise the defensive overflow guards in pldm_msgbuf_span_string_ascii()
 * and pldm_msgbuf_span_string_utf16() that are unreachable with conforming
 * strnlen/memmem implementations.
 *
 * The guards protect against measured > remaining after the NUL-terminator
 * search. Under normal conditions strnlen/memmem guarantee measured <=
 * remaining, making the overflow path dead code. We mock these functions via
 * preprocessor redirection so the guards are exercised.
 *
 * Compiled at -O0 to prevent the compiler from eliminating the mock paths
 * and to ensure gcov attributes coverage to src/msgbuf.h.
 */

#ifndef NDEBUG
#define NDEBUG 1
#endif

/*
 * Pull in all standard headers that msgbuf.h needs BEFORE we redefine
 * strnlen/memmem, so the real declarations are processed without our macros.
 */
#include <endian.h>
#include <libpldm/base.h>
#include <libpldm/pldm_types.h>
// NOLINTBEGIN(modernize-deprecated-headers)
#include <stdbool.h>
#include <sys/types.h>
#include <uchar.h>
// NOLINTEND(modernize-deprecated-headers)

#include <cassert>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstring>

/*
 * Pointer to ctx->remaining so mocks can mutate it as a side-effect, after
 * the function's entry guard (remaining < 0) has already passed.
 * We use intmax_t* rather than struct pldm_msgbuf* because the struct is
 * not yet defined at this point (it lives inside msgbuf.h).
 */
static intmax_t* mock_remaining;

/*
 * Mock strnlen:
 *   armed == 1: returns maxlen + 1 (exercises the -EOVERFLOW guard)
 *   armed == 2: corrupts ctx->remaining to INTMAX_MIN and returns 0
 *               (exercises the final pldm__msgbuf_invalidate fallthrough)
 */
static int mock_strnlen_armed;

static size_t real_strnlen_impl(const char* s, size_t maxlen)
{
    size_t i;

    for (i = 0; i < maxlen && s[i]; i++)
    {
    }

    return i;
}

static size_t mock_strnlen_fn(const char* s, size_t maxlen)
{
    if (mock_strnlen_armed == 1)
    {
        return maxlen + 1;
    }

    if (mock_strnlen_armed == 2)
    {
        *mock_remaining = INTMAX_MIN;
        return 0;
    }

    return real_strnlen_impl(s, maxlen);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define strnlen mock_strnlen_fn

/*
 * Mock memmem:
 *   armed == 1: returns haystack + haystacklen - 1 (exercises -EOVERFLOW guard)
 *   armed == 2: corrupts ctx->remaining to INTMAX_MIN and returns haystack
 *               (exercises the final pldm__msgbuf_invalidate fallthrough)
 *
 * For armed == 1 the returned pointer must have the same alignment parity as
 * the cursor so the alignment loop exits immediately.  Use an odd remaining.
 */
static int mock_memmem_armed;

static void* real_memmem_impl(const void* haystack, size_t haystacklen,
                              const void* needle, size_t needlelen)
{
    const auto* h = static_cast<const char*>(haystack);

    if (needlelen == 0)
    {
        return const_cast<void*>(haystack);
    }
    if (haystacklen < needlelen)
    {
        return nullptr;
    }

    for (size_t i = 0; i <= haystacklen - needlelen; i++)
    {
        if (memcmp(h + i, needle, needlelen) == 0)
        {
            return const_cast<void*>(static_cast<const void*>(h + i));
        }
    }

    return nullptr;
}

static void* mock_memmem_fn(const void* haystack, size_t haystacklen,
                            const void* needle, size_t needlelen)
{
    if (mock_memmem_armed == 1 && haystacklen >= needlelen)
    {
        return static_cast<char*>(const_cast<void*>(haystack)) + haystacklen -
               1;
    }

    if (mock_memmem_armed == 2)
    {
        *mock_remaining = INTMAX_MIN;
        return const_cast<void*>(haystack);
    }

    return real_memmem_impl(haystack, haystacklen, needle, needlelen);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define memmem mock_memmem_fn

/* Now include msgbuf.h - strnlen/memmem calls resolve to our mocks */
#include "compiler.h"
#include "msgbuf.h"

#undef strnlen
#undef memmem

#include <gtest/gtest.h>

TEST(MsgbufSpanOverflow, AsciiHappyPath)
{
    struct pldm_msgbuf _ctx;
    struct pldm_msgbuf* ctx = &_ctx;
    uint8_t buf[] = "hello";
    void* cursor = nullptr;
    size_t len = 0;

    ASSERT_EQ(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)), 0);
    EXPECT_EQ(pldm_msgbuf_span_string_ascii(ctx, &cursor, &len), 0);
    EXPECT_EQ(len, 6U);
    EXPECT_EQ(pldm_msgbuf_complete_consumed(ctx), 0);
}

TEST(MsgbufSpanOverflow, AsciiOverflowGuard)
{
    struct pldm_msgbuf _ctx;
    struct pldm_msgbuf* ctx = &_ctx;
    uint8_t buf[] = "test";

    ASSERT_EQ(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)), 0);

    mock_strnlen_armed = 1;
    EXPECT_EQ(pldm_msgbuf_span_string_ascii(ctx, nullptr, nullptr), -EOVERFLOW);
    mock_strnlen_armed = 0;
}

TEST(MsgbufSpanOverflow, Utf16HappyPath)
{
    struct pldm_msgbuf _ctx;
    struct pldm_msgbuf* ctx = &_ctx;
    /* "Hi" in UTF-16LE followed by NUL terminator */
    uint8_t buf[] = {'H', 0, 'i', 0, 0, 0};
    void* cursor = nullptr;
    size_t len = 0;

    ASSERT_EQ(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)), 0);
    EXPECT_EQ(pldm_msgbuf_span_string_utf16(ctx, &cursor, &len), 0);
    EXPECT_EQ(len, 6U);
    EXPECT_EQ(pldm_msgbuf_complete_consumed(ctx), 0);
}

TEST(MsgbufSpanOverflow, Utf16OverflowGuard)
{
    struct pldm_msgbuf _ctx;
    struct pldm_msgbuf* ctx = &_ctx;
    /*
     * Buffer must have odd length so the mocked memmem return value has the
     * same alignment parity as the cursor, causing the alignment loop to
     * exit on the first iteration.
     */
    uint8_t buf[] = {'H', 0, 'i', 0, 0, 0, 0};

    ASSERT_EQ(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)), 0);

    mock_memmem_armed = 1;
    EXPECT_EQ(pldm_msgbuf_span_string_utf16(ctx, nullptr, nullptr), -EOVERFLOW);
    mock_memmem_armed = 0;
}

TEST(MsgbufSpanOverflow, ConsumedNotExhausted)
{
    struct pldm_msgbuf _ctx;
    struct pldm_msgbuf* ctx = &_ctx;
    uint8_t buf[] = {0, 1, 2, 3};

    ASSERT_EQ(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)), 0);
    EXPECT_EQ(pldm_msgbuf_complete_consumed(ctx), -EBADMSG);
}

/*
 * The final pldm__msgbuf_invalidate() fallthrough (lines 1118 / 1193) requires
 * remaining <= INTMAX_MIN + measured, which is impossible when remaining >= 0
 * and measured fits in intmax_t.  We reach it by having the mock corrupt
 * ctx->remaining to INTMAX_MIN *after* the function's entry guard has passed.
 *
 * Flow for ASCII (armed == 2):
 *   1. remaining = N >= 0  →  passes the (remaining < 0) entry check
 *   2. mock_strnlen sets remaining = INTMAX_MIN, returns 0
 *   3. measured = 0, measured++ = 1
 *   4. remaining (INTMAX_MIN) >= measured (1)  →  false  (skip happy path)
 *   5. remaining (INTMAX_MIN) > INTMAX_MIN + 1  →  false  (skip overflow)
 *   6. falls through to pldm__msgbuf_invalidate()  →  line 1118
 */
TEST(MsgbufSpanOverflow, AsciiInvalidateFallthrough)
{
    struct pldm_msgbuf _ctx;
    struct pldm_msgbuf* ctx = &_ctx;
    uint8_t buf[] = "test";

    ASSERT_EQ(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)), 0);

    mock_remaining = &ctx->remaining;
    mock_strnlen_armed = 2;
    EXPECT_EQ(pldm_msgbuf_span_string_ascii(ctx, nullptr, nullptr), -EOVERFLOW);
    mock_strnlen_armed = 0;
    mock_remaining = nullptr;
}

/*
 * Same idea for UTF-16 (armed == 2):
 *   1. remaining = N >= 0  →  passes entry guard
 *   2. mock_memmem sets remaining = INTMAX_MIN, returns cursor
 *   3. alignment matches (same pointer), loop exits
 *   4. end = cursor + sizeof(char16_t), measured = 2
 *   5. remaining (INTMAX_MIN) >= 2  →  false
 *   6. remaining (INTMAX_MIN) > INTMAX_MIN + 2  →  false
 *   7. falls through to pldm__msgbuf_invalidate()  →  line 1193
 */
TEST(MsgbufSpanOverflow, Utf16InvalidateFallthrough)
{
    struct pldm_msgbuf _ctx;
    struct pldm_msgbuf* ctx = &_ctx;
    uint8_t buf[] = {'H', 0, 'i', 0, 0, 0};

    ASSERT_EQ(pldm_msgbuf_init_errno(ctx, 0, buf, sizeof(buf)), 0);

    mock_remaining = &ctx->remaining;
    mock_memmem_armed = 2;
    EXPECT_EQ(pldm_msgbuf_span_string_utf16(ctx, nullptr, nullptr), -EOVERFLOW);
    mock_memmem_armed = 0;
    mock_remaining = nullptr;
}
