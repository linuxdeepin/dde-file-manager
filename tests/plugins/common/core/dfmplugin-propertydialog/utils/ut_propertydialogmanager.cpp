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
#include "plugins/common/core/dfmplugin-propertydialog/utils/propertydialogmanager.h"

#include <QUrl>

#include <gtest/gtest.h>

DPPROPERTYDIALOG_USE_NAMESPACE

class UT_PropertyDialogManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        mng = &PropertyDialogManager::instance();
    }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    PropertyDialogManager *mng { nullptr };
};

TEST_F(UT_PropertyDialogManager, CreateCustomView)
{
    mng->viewCreateFunctionHash.clear();
    EXPECT_FALSE(mng->createCustomView(QUrl()));
    EXPECT_FALSE(mng->createCustomView(QUrl::fromLocalFile("/home")));

    mng->viewCreateFunctionHash.insert("entry", [](const QUrl &url) -> QWidget * {
        __DBG_STUB_INVOKE__
        return url.scheme() == "entry" ? new QWidget() : nullptr;
    });
    EXPECT_FALSE(mng->createCustomView(QUrl::fromLocalFile("/home")));
    QWidget *wid { nullptr };
    EXPECT_NO_FATAL_FAILURE(wid = mng->createCustomView(QUrl("entry:///sdb1.blockdev")));
    EXPECT_TRUE(wid);
    delete wid;
    mng->viewCreateFunctionHash.clear();
}
