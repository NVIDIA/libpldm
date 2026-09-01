/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Force elision of assert() */
#ifndef NDEBUG
#define NDEBUG
#endif

#include <assert.h>
#include <libpldm/edac.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"

static int malloc_failures;

static void* test_malloc(size_t size)
{
    if (malloc_failures)
    {
        malloc_failures--;
        return NULL;
    }

    return malloc(size);
}

#define malloc test_malloc
/* NOLINTNEXTLINE(bugprone-suspicious-include) */
#include "dsp/bios_table.c"
#undef malloc

/* Satisfy the symbol needs of bios_table.c */
uint32_t pldm_edac_crc32(const void* data LIBPLDM_CC_UNUSED,
                         size_t size LIBPLDM_CC_UNUSED)
{
    return 0;
}

#define expect(cond) expect_impl(__func__, __LINE__, (cond), #cond)
static int expect_impl(const char* fn, int line, int cond, const char* expr)
{
    if (!cond)
    {
        fprintf(stderr, "%s:%d: failed expectation: %s\n", fn, line, expr);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int test_iterator_unknown_attr_type_is_end(void)
{
    struct pldm_bios_attr_table_entry entries[2] = {0};
    struct pldm_bios_table_iter* iter;
    int result;

    static_assert(2 * sizeof(entries[0]) == sizeof(entries), "");

    entries[0].attr_type = PLDM_BIOS_PASSWORD;
    entries[1].attr_type = PLDM_BIOS_STRING_READ_ONLY;

    iter = pldm_bios_table_iter_create(entries, sizeof(entries),
                                       PLDM_BIOS_ATTR_TABLE);

    /*
     * We expect the test configuration to claim the iterator has reached the
     * end because the there's no entry length descriptor for the
     * PLDM_BIOS_PASSWORD entry type. By the attr_able_entry_length()
     * implementation this would normally trigger an assert() to uphold that the
     * necessary pointers are not NULL. However, we've defined NDEBUG above and
     * so the assert() is elided. That should force us down the path of the
     * early-exit, which should in-turn yield a `true` result from
     * pldm_bios_table_iter_is_end() to prevent further attempts to access
     * invalid objects.
     */
    result = pldm_bios_table_iter_is_end(iter) ? EXIT_SUCCESS : EXIT_FAILURE;

    pldm_bios_table_iter_free(iter);

    return result;
}

#if HAVE_LIBPLDM_ABI_DEPRECATED
static int test_iterator_create_malloc_failure(void)
{
    struct pldm_bios_table_iter* iter;

    malloc_failures = 1;
    iter = pldm_bios_table_iter_create(NULL, 0, PLDM_BIOS_STRING_TABLE);

    return expect(iter == NULL) || expect(malloc_failures == 0);
}
#endif

#if HAVE_LIBPLDM_ABI_STABLE
static int test_string_entry_encode_edges(void)
{
    uint8_t entry[sizeof(struct pldm_bios_string_table_entry)] = {0};

    return expect(pldm_bios_table_string_entry_encode(entry, sizeof(entry), "a",
                                                      0) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_string_entry_encode(NULL, sizeof(entry), "a",
                                                      1) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_string_entry_encode(entry, sizeof(entry),
                                                      NULL, 1) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_string_entry_encode(entry, sizeof(entry) - 1,
                                                      "a", 1) ==
                  PLDM_ERROR_INVALID_LENGTH);
}

static int test_string_handle_exhaustion(void)
{
    uint8_t entry[sizeof(struct pldm_bios_string_table_entry)] = {0};

    for (uint32_t i = 0; i < UINT16_MAX; i++)
    {
        if (expect(pldm_bios_table_string_entry_encode(entry, sizeof(entry),
                                                       "a", 1) == PLDM_SUCCESS))
        {
            return EXIT_FAILURE;
        }
    }

    return expect(pldm_bios_table_string_entry_encode(
                      entry, sizeof(entry), "a", 1) == PLDM_ERROR_INVALID_DATA);
}

static int test_string_entry_decode_string_edges(void)
{
    struct pldm_bios_string_table_entry entry = {0};
    char buffer[2] = {0};

    entry.string_length = htole16(1);
    entry.name[0] = 'a';

    return expect(pldm_bios_table_string_entry_decode_string(NULL, buffer,
                                                             sizeof(buffer)) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_string_entry_decode_string(&entry, NULL,
                                                             sizeof(buffer)) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_string_entry_decode_string(
                      &entry, buffer, sizeof(buffer)) == PLDM_SUCCESS) ||
           expect(buffer[0] == 'a') ||
           expect(pldm_bios_table_string_entry_decode_string(
                      &entry, buffer, 0) == PLDM_ERROR_INVALID_LENGTH);
}

static int test_attr_entry_encode_edges(void)
{
    struct pldm_bios_table_attr_entry_string_info string_info = {
        .name_handle = 1,
        .read_only = false,
        .string_type = 1,
        .min_length = 1,
        .max_length = 1,
        .def_length = 1,
        .def_string = "a",
    };
    struct pldm_bios_table_attr_entry_integer_info integer_info = {
        .name_handle = 1,
        .read_only = false,
        .lower_bound = 1,
        .upper_bound = 1,
        .scalar_increment = 0,
        .default_value = 1,
    };
    uint8_t entry[64] = {0};

    return expect(pldm_bios_table_attr_entry_string_encode(NULL, sizeof(entry),
                                                           &string_info) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_attr_entry_string_encode(
                      entry, sizeof(entry), NULL) == PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_attr_entry_string_encode(
                      entry, sizeof(struct pldm_bios_attr_table_entry) - 1,
                      &string_info) == PLDM_ERROR_INVALID_LENGTH) ||
           expect(pldm_bios_table_attr_entry_integer_encode(NULL, sizeof(entry),
                                                            &integer_info) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_attr_entry_integer_encode(
                      entry, sizeof(entry), NULL) == PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_attr_entry_integer_encode(
                      entry, sizeof(struct pldm_bios_attr_table_entry) - 1,
                      &integer_info) == PLDM_ERROR_INVALID_LENGTH);
}

static int test_attr_value_integer_encode_edges(void)
{
    uint8_t entry[sizeof(struct pldm_bios_attr_val_table_entry) - 1 +
                  sizeof(uint64_t)] = {0};

    return expect(pldm_bios_table_attr_value_entry_encode_integer(
                      NULL, sizeof(entry), 0, PLDM_BIOS_INTEGER, 5) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_attr_value_entry_encode_integer(
                      entry, sizeof(entry), 0, PLDM_BIOS_INTEGER, 5) ==
                  PLDM_SUCCESS) ||
           expect(pldm_bios_table_attr_value_entry_encode_integer(
                      entry, sizeof(entry), 0, PLDM_BIOS_STRING, 5) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_attr_value_entry_encode_integer(
                      entry, sizeof(entry) - 1, 0, PLDM_BIOS_INTEGER, 5) ==
                  PLDM_ERROR_INVALID_LENGTH);
}

static int test_pad_checksum_and_checksum_edges(void)
{
    uint8_t table[12] = {0};

    if (expect(pldm_bios_table_append_pad_checksum(NULL, 0, NULL) ==
               PLDM_ERROR_INVALID_DATA))
    {
        return EXIT_FAILURE;
    }

    if (expect(pldm_bios_table_checksum(table, sizeof(table))))
    {
        return EXIT_FAILURE;
    }
    if (expect(!pldm_bios_table_checksum(NULL, sizeof(table))))
    {
        return EXIT_FAILURE;
    }

    table[sizeof(table) - 1] = 1;

    return expect(!pldm_bios_table_checksum(table, sizeof(table)));
}

#if HAVE_LIBPLDM_ABI_DEPRECATED
static int test_attr_entry_enum_encode_edges(void)
{
    uint16_t pv_handle = 1;
    uint8_t def_index = 0;
    struct pldm_bios_table_attr_entry_enum_info enum_info = {
        .name_handle = 1,
        .read_only = false,
        .pv_num = 1,
        .pv_handle = &pv_handle,
        .def_num = 1,
        .def_index = &def_index,
    };
    uint8_t entry[64] = {0};

    return expect(pldm_bios_table_attr_entry_enum_encode(NULL, sizeof(entry),
                                                         &enum_info) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_attr_entry_enum_encode(
                      entry, sizeof(entry), NULL) == PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_attr_entry_enum_encode(
                      entry, sizeof(struct pldm_bios_attr_table_entry) - 1,
                      &enum_info) == PLDM_ERROR_INVALID_LENGTH);
}
#endif

static int test_attr_handle_exhaustion(void)
{
    uint16_t pv_handle = 1;
    uint8_t def_index = 0;
    struct pldm_bios_table_attr_entry_string_info string_info = {
        .name_handle = 1,
        .read_only = false,
        .string_type = 1,
        .min_length = 1,
        .max_length = 1,
        .def_length = 1,
        .def_string = "a",
    };
    struct pldm_bios_table_attr_entry_integer_info integer_info = {
        .name_handle = 1,
        .read_only = false,
        .lower_bound = 1,
        .upper_bound = 1,
        .scalar_increment = 0,
        .default_value = 1,
    };
    uint8_t entry[64] = {0};

#if HAVE_LIBPLDM_ABI_DEPRECATED
    struct pldm_bios_table_attr_entry_enum_info enum_info = {
        .name_handle = 1,
        .read_only = false,
        .pv_num = 1,
        .pv_handle = &pv_handle,
        .def_num = 1,
        .def_index = &def_index,
    };
#endif

    for (uint32_t i = 0; i < UINT16_MAX; i++)
    {
        int rc;

#if HAVE_LIBPLDM_ABI_DEPRECATED
        if ((i % 3) == 0)
        {
            rc = pldm_bios_table_attr_entry_enum_encode(entry, sizeof(entry),
                                                        &enum_info);
        }
        else if ((i % 3) == 1)
        {
            rc = pldm_bios_table_attr_entry_string_encode(entry, sizeof(entry),
                                                          &string_info);
        }
        else
        {
            rc = pldm_bios_table_attr_entry_integer_encode(entry, sizeof(entry),
                                                           &integer_info);
        }
#else
        if ((i % 2) == 0)
        {
            rc = pldm_bios_table_attr_entry_string_encode(entry, sizeof(entry),
                                                          &string_info);
        }
        else
        {
            rc = pldm_bios_table_attr_entry_integer_encode(entry, sizeof(entry),
                                                           &integer_info);
        }
#endif

        if (expect(rc == PLDM_SUCCESS))
        {
            return EXIT_FAILURE;
        }
    }

#if HAVE_LIBPLDM_ABI_DEPRECATED
    if (expect(pldm_bios_table_attr_entry_enum_encode(entry, sizeof(entry),
                                                      &enum_info) ==
               PLDM_ERROR_INVALID_DATA))
    {
        return EXIT_FAILURE;
    }
#else
    (void)pv_handle;
    (void)def_index;
#endif

    return expect(pldm_bios_table_attr_entry_string_encode(entry, sizeof(entry),
                                                           &string_info) ==
                  PLDM_ERROR_INVALID_DATA) ||
           expect(pldm_bios_table_attr_entry_integer_encode(
                      entry, sizeof(entry), &integer_info) ==
                  PLDM_ERROR_INVALID_DATA);
}
#endif

/* This is the non-death version of TEST(Iterator, DeathTest) */
int main(void)
{
    if (test_iterator_unknown_attr_type_is_end())
    {
        return EXIT_FAILURE;
    }

#if HAVE_LIBPLDM_ABI_DEPRECATED
    if (test_iterator_create_malloc_failure())
    {
        return EXIT_FAILURE;
    }
#endif

#if HAVE_LIBPLDM_ABI_STABLE
#if HAVE_LIBPLDM_ABI_DEPRECATED
    if (test_attr_entry_enum_encode_edges())
    {
        return EXIT_FAILURE;
    }
#endif

    if (test_string_entry_encode_edges() || test_string_handle_exhaustion() ||
        test_string_entry_decode_string_edges() ||
        test_attr_entry_encode_edges() ||
        test_attr_value_integer_encode_edges() ||
        test_pad_checksum_and_checksum_edges() || test_attr_handle_exhaustion())
    {
        return EXIT_FAILURE;
    }
#endif

    return EXIT_SUCCESS;
}
