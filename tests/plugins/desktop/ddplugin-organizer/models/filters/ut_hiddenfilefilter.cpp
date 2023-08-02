// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/base/schemefactory.h>
#include "models/filters/hiddenfilefilter.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QList>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

class UT_HiddenFileFilter : public testing::Test
{
public:
    virtual void SetUp() override {
        hid = new HiddenFileFilter;
    }
    virtual void TearDown() override {
        stub.clear();
        delete hid;
        hid = nullptr;
    }
    stub_ext::StubExt stub;
    HiddenFileFilter *hid;

};

TEST_F(UT_HiddenFileFilter, acceptInsert)
{
    QUrl url("temp_url");
    EXPECT_TRUE(hid->acceptInsert(url));
}

TEST_F(UT_HiddenFileFilter, acceptReset)
{
    QList<QUrl> list{QUrl("temp_url")};
    QList<QUrl> res = hid->acceptReset(list);
    EXPECT_EQ(res,list);
}

TEST_F(UT_HiddenFileFilter, acceptUpdate)
{
    QUrl url(".hidden");

    QVector<int> roles{Global::kItemCreateFileInfoRole};
    EXPECT_FALSE(hid->acceptUpdate(url,roles));
}

TEST_F(UT_HiddenFileFilter, flag)
{
    bool call = true;
    hid->updateFlag();
    hid->hiddenFlagChanged(true);
    EXPECT_TRUE(call);
}
