// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>

#include "widgets/tagcrumbedit.h"
#include <dfm-base/base/application/application.h>

#include <QPaintEvent>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagCrumbEditTest : public testing::Test
{

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

TEST_F(TagCrumbEditTest, mouseDoubleClickEvent)
{
    TagCrumbEdit edit;
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPoint(100, 100), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    edit.mouseDoubleClickEvent(event);
    EXPECT_FALSE(edit.isEditing());
    if (event) {
        delete event;
        event = nullptr;
    }
}
