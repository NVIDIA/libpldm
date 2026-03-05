/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/*
 * Compiled at -O0 so pldm_xlate_errno() is not fully inlined and gcov
 * attributes coverage to src/api.h rather than this test file.
 */

#ifndef NDEBUG
#define NDEBUG 1
#endif

#include "api.h"

#include <gtest/gtest.h>

TEST(XlateErrno, BadMsg)
{
    EXPECT_EQ(pldm_xlate_errno(-EBADMSG), PLDM_ERROR_INVALID_DATA);
}

TEST(XlateErrno, Inval)
{
    EXPECT_EQ(pldm_xlate_errno(-EINVAL), PLDM_ERROR_INVALID_DATA);
}

TEST(XlateErrno, Proto)
{
    EXPECT_EQ(pldm_xlate_errno(-EPROTO), PLDM_ERROR_INVALID_DATA);
}

TEST(XlateErrno, Uclean)
{
    EXPECT_EQ(pldm_xlate_errno(-EUCLEAN), PLDM_ERROR_INVALID_DATA);
}

TEST(XlateErrno, NoMsg)
{
    EXPECT_EQ(pldm_xlate_errno(-ENOMSG), PLDM_ERROR_INVALID_PLDM_TYPE);
}

TEST(XlateErrno, Overflow)
{
    EXPECT_EQ(pldm_xlate_errno(-EOVERFLOW), PLDM_ERROR_INVALID_LENGTH);
}

TEST(XlateErrno, NotSup)
{
    EXPECT_EQ(pldm_xlate_errno(-ENOTSUP), PLDM_ERROR);
}

TEST(XlateErrno, DefaultPath)
{
    EXPECT_EQ(pldm_xlate_errno(-ERANGE), PLDM_ERROR);
}
