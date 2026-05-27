#include "transport/transport.h"

#include <libpldm/transport.h>

#include "array.h"
#include "transport/test.h"

#include <gtest/gtest.h>

TEST(Transport, create)
{
    struct pldm_transport_test* test = NULL;

    EXPECT_EQ(pldm_transport_test_init(&test, NULL, 0), 0);
    EXPECT_NE(pldm_transport_test_core(test), nullptr);
    pldm_transport_test_destroy(test);
}

TEST(Transport, send_one)
{
    const uint8_t msg[] = {0x81, 0x00, 0x01, 0x01};
    const struct pldm_transport_test_descriptor seq[] = {
        {
            .type = PLDM_TRANSPORT_TEST_ELEMENT_MSG_SEND,
            .send_msg =
                {
                    .dst = 1,
                    .msg = msg,
                    .len = sizeof(msg),
                },
        },
    };
    struct pldm_transport_test* test = NULL;
    struct pldm_transport* ctx;
    int rc;

    EXPECT_EQ(pldm_transport_test_init(&test, seq, ARRAY_SIZE(seq)), 0);
    ctx = pldm_transport_test_core(test);
    rc = pldm_transport_send_msg(ctx, 1, msg, sizeof(msg));
    EXPECT_EQ(rc, PLDM_REQUESTER_SUCCESS);
    pldm_transport_test_destroy(test);
}

TEST(Transport, recv_one)
{
    uint8_t msg[] = {0x01, 0x00, 0x01, 0x00};
    const pldm_tid_t src_tid = 1;
    const struct pldm_transport_test_descriptor seq[] = {
        {
            .type = PLDM_TRANSPORT_TEST_ELEMENT_MSG_RECV,
            .recv_msg =
                {
                    .src = src_tid,
                    .msg = msg,
                    .len = sizeof(msg),
                },
        },
    };
    struct pldm_transport_test* test = NULL;
    struct pldm_transport* ctx;
    void* recvd;
    size_t len;
    int rc;
    pldm_tid_t tid;

    EXPECT_EQ(pldm_transport_test_init(&test, seq, ARRAY_SIZE(seq)), 0);
    ctx = pldm_transport_test_core(test);
    rc = pldm_transport_recv_msg(ctx, &tid, &recvd, &len);
    EXPECT_EQ(rc, PLDM_REQUESTER_SUCCESS);
    EXPECT_EQ(len, sizeof(msg));
    EXPECT_EQ(memcmp(recvd, msg, len), 0);
    EXPECT_EQ(tid, src_tid);
    free(recvd);
    pldm_transport_test_destroy(test);
}

#if HAVE_LIBPLDM_ABI_STABLE
TEST(Transport, stable_error_branches)
{
    uint8_t req[] = {0x81, 0x00, 0x01, 0x01};
    struct pldm_transport noPoll = {};

    EXPECT_EQ(pldm_transport_poll(nullptr, 0), PLDM_REQUESTER_INVALID_SETUP);
    EXPECT_EQ(pldm_transport_poll(&noPoll, 0), 1);

    EXPECT_EQ(pldm_transport_send_msg(nullptr, 1, req, sizeof(req)),
              PLDM_REQUESTER_INVALID_SETUP);
    EXPECT_EQ(pldm_transport_send_msg(&noPoll, 1, nullptr, sizeof(req)),
              PLDM_REQUESTER_INVALID_SETUP);
    EXPECT_EQ(
        pldm_transport_send_msg(&noPoll, 1, req, sizeof(pldm_msg_hdr) - 1),
        PLDM_REQUESTER_NOT_REQ_MSG);

    pldm_tid_t tid = 0;
    void* msg = nullptr;
    size_t len = 0;
    EXPECT_EQ(pldm_transport_recv_msg(nullptr, &tid, &msg, &len),
              PLDM_REQUESTER_INVALID_SETUP);
    EXPECT_EQ(pldm_transport_recv_msg(&noPoll, &tid, &msg, nullptr),
              PLDM_REQUESTER_INVALID_SETUP);

    struct pldm_transport_test* test = nullptr;
    EXPECT_EQ(pldm_transport_test_init(&test, nullptr, 0), 0);
    EXPECT_EQ(pldm_transport_poll(pldm_transport_test_core(test), 0),
              PLDM_REQUESTER_POLL_FAIL);
    EXPECT_EQ(pldm_transport_recv_msg(pldm_transport_test_core(test), &tid,
                                      &msg, &len),
              PLDM_REQUESTER_RECV_FAIL);
    pldm_transport_test_destroy(test);

    const uint8_t shortResp[] = {0x01, 0x00};
    const struct pldm_transport_test_descriptor seq[] = {
        {
            .type = PLDM_TRANSPORT_TEST_ELEMENT_MSG_RECV,
            .recv_msg =
                {
                    .src = 1,
                    .msg = shortResp,
                    .len = sizeof(shortResp),
                },
        },
    };
    test = nullptr;
    EXPECT_EQ(pldm_transport_test_init(&test, seq, ARRAY_SIZE(seq)), 0);
    EXPECT_EQ(pldm_transport_recv_msg(pldm_transport_test_core(test), &tid,
                                      &msg, &len),
              PLDM_REQUESTER_INVALID_RECV_LEN);
    EXPECT_EQ(msg, nullptr);
    pldm_transport_test_destroy(test);
}

TEST(Transport, stable_send_recv_rejects_invalid_request)
{
    uint8_t response[] = {0x01, 0x00, 0x01, 0x00};
    size_t len = 0;
    void* msg = nullptr;
    struct pldm_transport_test* test = nullptr;

    EXPECT_EQ(pldm_transport_send_recv_msg(
                  nullptr, 1, response, sizeof(pldm_msg_hdr) - 1, &msg, &len),
              PLDM_REQUESTER_INVALID_SETUP);
    EXPECT_EQ(pldm_transport_send_recv_msg(nullptr, 1, response,
                                           sizeof(response), &msg, nullptr),
              PLDM_REQUESTER_INVALID_SETUP);

    ASSERT_EQ(pldm_transport_test_init(&test, nullptr, 0), 0);
    response[0] = 0x01;
    EXPECT_EQ(pldm_transport_send_recv_msg(pldm_transport_test_core(test), 1,
                                           response, sizeof(response), &msg,
                                           &len),
              PLDM_REQUESTER_NOT_REQ_MSG);
    pldm_transport_test_destroy(test);
}
#endif

TEST(Transport, send_recv_drain_one_unwanted)
{
    uint8_t unwanted[] = {0x01, 0x00, 0x01, 0x01};
    uint8_t req[] = {0x81, 0x00, 0x01, 0x01};
    uint8_t resp[] = {0x01, 0x00, 0x01, 0x00};
    const struct pldm_transport_test_descriptor seq[] = {
        {
            .type = PLDM_TRANSPORT_TEST_ELEMENT_MSG_RECV,
            .recv_msg =
                {
                    .src = 2,
                    .msg = unwanted,
                    .len = sizeof(unwanted),
                },
        },
        {
            .type = PLDM_TRANSPORT_TEST_ELEMENT_MSG_SEND,
            .send_msg =
                {
                    .dst = 1,
                    .msg = req,
                    .len = sizeof(req),
                },
        },
        {
            .type = PLDM_TRANSPORT_TEST_ELEMENT_MSG_RECV,
            .recv_msg =
                {
                    .src = 1,
                    .msg = resp,
                    .len = sizeof(resp),
                },
        },
    };
    struct pldm_transport_test* test = NULL;
    struct pldm_transport* ctx;
    size_t len;
    void* msg = NULL;
    int rc;

    EXPECT_EQ(pldm_transport_test_init(&test, seq, ARRAY_SIZE(seq)), 0);
    ctx = pldm_transport_test_core(test);
    rc = pldm_transport_send_recv_msg(ctx, 1, req, sizeof(req), &msg, &len);
    ASSERT_EQ(rc, PLDM_REQUESTER_SUCCESS);
    EXPECT_NE(memcmp(msg, unwanted, len), 0);
    EXPECT_EQ(memcmp(msg, resp, len), 0);
    free(msg);
    pldm_transport_test_destroy(test);
}

TEST(Transport, send_recv_req_echo)
{
    uint8_t req[] = {0x81, 0x00, 0x01, 0x01};
    uint8_t echo[] = {0x81, 0x00, 0x01, 0x01};
    uint8_t resp[] = {0x01, 0x00, 0x01, 0x00};
    const struct pldm_transport_test_descriptor seq[] = {
        {
            .type = PLDM_TRANSPORT_TEST_ELEMENT_MSG_SEND,
            .send_msg =
                {
                    .dst = 1,
                    .msg = req,
                    .len = sizeof(req),
                },
        },
        {
            .type = PLDM_TRANSPORT_TEST_ELEMENT_MSG_RECV,
            .recv_msg =
                {
                    .src = 1,
                    .msg = echo,
                    .len = sizeof(echo),
                },
        },
        {
            .type = PLDM_TRANSPORT_TEST_ELEMENT_MSG_RECV,
            .recv_msg =
                {
                    .src = 1,
                    .msg = resp,
                    .len = sizeof(resp),
                },
        },
    };
    struct pldm_transport_test* test = NULL;
    struct pldm_transport* ctx;
    size_t len;
    void* msg;
    int rc;

    EXPECT_EQ(pldm_transport_test_init(&test, seq, ARRAY_SIZE(seq)), 0);
    ctx = pldm_transport_test_core(test);
    rc = pldm_transport_send_recv_msg(ctx, 1, req, sizeof(req), &msg, &len);
    ASSERT_EQ(rc, PLDM_REQUESTER_SUCCESS);
    EXPECT_NE(memcmp(msg, echo, len), 0);
    EXPECT_EQ(memcmp(msg, resp, len), 0);
    free(msg);
    pldm_transport_test_destroy(test);
}
