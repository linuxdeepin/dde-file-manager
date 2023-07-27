// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/bookmarkhelper.h"
#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_global_defines.h>

#include <gtest/gtest.h>
#include <QIcon>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_bookmark;

class BookMarkHelperSceneTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ins = BookMarkHelper::instance();
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    BookMarkHelper *ins { nullptr };
};

TEST_F(BookMarkHelperSceneTest, info)
{
    QUrl url;
    url.setScheme("bookmark");
    url.setPath("/");
    EXPECT_TRUE(ins->rootUrl() == url);
    EXPECT_TRUE(ins->icon().name() == "folder-bookmark-symbolic");
}
