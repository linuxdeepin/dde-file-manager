// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>

#include <DLabel>
#include <QMouseEvent>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_ClickableLabel : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        lab = new ClickableLabel;
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete lab;
    }

private:
    stub_ext::StubExt stub;
    ClickableLabel *lab { nullptr };
};

TEST_F(UT_ClickableLabel, MouseReleaseEvent)
{
    stub.set_lamda(VADDR(DLabel, mouseReleaseEvent), [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(lab->mouseReleaseEvent(nullptr));

    auto e = new QMouseEvent(QEvent::Type::MouseButtonRelease, { 0, 0 }, Qt::MouseButton::LeftButton,
                             Qt::MouseButton::LeftButton, Qt::KeyboardModifier::AltModifier);
    EXPECT_NO_FATAL_FAILURE(lab->mouseReleaseEvent(e));
    delete e;
}
