// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "utils/tagpainter.h"
#include "utils/taghelper.h"

#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagPainterTest : public testing::Test
{

protected:
    virtual void SetUp() override
    {
        ins = new TagPainter();
    }
    virtual void TearDown() override
    {
        stub.clear();
        if (ins) {
            delete ins;
            ins = nullptr;
        }
    }

private:
    stub_ext::StubExt stub;
    TagPainter *ins;
};

TEST_F(TagPainterTest, intrinsicSize)
{
    QTextDocument doc;
    EXPECT_TRUE(ins->intrinsicSize(&doc, 1, QTextFormat()) == QSizeF(kTagDiameter / 2, kTagDiameter));
}

TEST_F(TagPainterTest, drawObject)
{
    bool flag = false;
    QPainter painter;
    QTextDocument doc;

    stub.set_lamda(&TagHelper::paintTags, [&flag]() {
        __DBG_STUB_INVOKE__
        flag = true;
    });
    ins->drawObject(&painter, QRectF(), &doc, 1, QTextFormat());
    EXPECT_TRUE(flag);
}
