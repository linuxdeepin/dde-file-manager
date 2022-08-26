/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stubext.h"
#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"

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
