// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/filemanager/core/dfmplugin-detailspace/utils/detailspacehelper.h"
#include "plugins/filemanager/core/dfmplugin-detailspace/views/detailspacewidget.h"

#include <gtest/gtest.h>

DPDETAILSPACE_USE_NAMESPACE

class UT_DetailSpaceHelper : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_DetailSpaceHelper, bug_144607_DetailViewCrashed)
{
    QUrl testUrl { QUrl::fromLocalFile("/tmp") };
    DetailSpaceWidget w;
    stub.set_lamda(&DetailSpaceWidget::isVisible, [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    // if detailspace isn't visilble, cannot set url
    DetailSpaceHelper::setDetailViewByUrl(&w, testUrl);
    EXPECT_FALSE(w.detailSpaceUrl.isValid());
}
