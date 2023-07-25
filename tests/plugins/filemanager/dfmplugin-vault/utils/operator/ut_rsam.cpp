// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/operator/rsam.h"

#include <gtest/gtest.h>

TEST(UT_RSAM, privateKey_encrypt_publicKey_decrypt)
{
    QString publicKey, privateKey;
    rsam::createPublicAndPrivateKey(publicKey, privateKey);
    QString cipher = rsam::privateKeyEncrypt("UT_TEST", privateKey);
    QString password = rsam::publicKeyDecrypt(cipher, publicKey);

    EXPECT_TRUE(password == "UT_TEST");
}
