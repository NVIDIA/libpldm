/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/*
 * Keep a C translation-unit instantiation of pldm_xlate_errno() covered. The
 * C++ gtest covers the C++ inline instance, while C library call-sites produce
 * a separate C coverage bucket in gcovr.
 */

#ifndef NDEBUG
#define NDEBUG 1
#endif

#include <stdio.h>
#include <stdlib.h>

#include "api.h"

#define expect_eq(actual, expected)                                            \
    do                                                                         \
    {                                                                          \
        enum pldm_completion_codes actual_ = (actual);                         \
        enum pldm_completion_codes expected_ = (expected);                     \
        if (actual_ != expected_)                                              \
        {                                                                      \
            fprintf(stderr, "%s:%d: got %d expected %d\n", __func__, __LINE__, \
                    actual_, expected_);                                       \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)

int main(void)
{
#if HAVE_LIBPLDM_ABI_STABLE
    expect_eq(pldm_xlate_errno(-EBADMSG), PLDM_ERROR_INVALID_DATA);
    expect_eq(pldm_xlate_errno(-EINVAL), PLDM_ERROR_INVALID_DATA);
    expect_eq(pldm_xlate_errno(-EPROTO), PLDM_ERROR_INVALID_DATA);
    expect_eq(pldm_xlate_errno(-EUCLEAN), PLDM_ERROR_INVALID_DATA);
    expect_eq(pldm_xlate_errno(-ENOMSG), PLDM_ERROR_INVALID_PLDM_TYPE);
    expect_eq(pldm_xlate_errno(-EOVERFLOW), PLDM_ERROR_INVALID_LENGTH);
    expect_eq(pldm_xlate_errno(-ENOTSUP), PLDM_ERROR);
    expect_eq(pldm_xlate_errno(-ERANGE), PLDM_ERROR);
#endif
    return EXIT_SUCCESS;
}
