// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <gtest/gtest.h>

class UT_InfoFactory : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

DFMBASE_USE_NAMESPACE

TEST_F(UT_InfoFactory, Bug_199927_199517_scheme)
{
    QUrl url("dfmvault:///10000/oooo");
    InfoFactory infoFactory;
    EXPECT_EQ("dfmvault", infoFactory.scheme(url));

    stub_ext::StubExt stub;
    stub.set_lamda(&FileUtils::isLocalDevice, []{ __DBG_STUB_INVOKE__ return false; });
    url = QUrl("file:///10000/oooo");
    EXPECT_EQ("asyncfile", infoFactory.scheme(url));

    stub.set_lamda(&FileUtils::isLocalDevice, []{ __DBG_STUB_INVOKE__ return true; });
    EXPECT_EQ("file", infoFactory.scheme(url));

    int testi{ 0 };
    stub.set_lamda(&FileUtils::isLocalDevice, [&testi]{
        __DBG_STUB_INVOKE__
        if (testi > 0)
            return false;
        testi++;
        return true; });
    int testa{ 0 };
    stub.set_lamda(&dfmio::DFileInfo::attribute, []{ __DBG_STUB_INVOKE__ return true; });
    EXPECT_EQ("asyncfile", infoFactory.scheme(url));
}
