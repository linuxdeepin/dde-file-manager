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
#include "plugins/filemanager/core/dfmplugin-sidebar/utils/fileoperatorhelper.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

class UT_FileOperatorHelper : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_FileOperatorHelper, PasteFiles)
{
    DFMBASE_USE_NAMESPACE
    DPF_USE_NAMESPACE
    DPSIDEBAR_USE_NAMESPACE
    // this template function's instance cannot be captured correctlly, wierd...
    //    typedef bool (EventDispatcherManager::*Publish)(EventType, quint64,
    //                                                    QList<QUrl> &, QUrl &,
    //                                                    AbstractJobHandler::JobFlag &&,
    //                                                    decltype(nullptr) &&);
    //    auto publish = static_cast<Publish>(&EventDispatcherManager::publish);
    //    stub.set_lamda(publish, [] { __DBG_STUB_INVOKE__ return true; });

    typedef bool (EventDispatcher::*Dispatch)(const QVariantMap &);
    auto dispatch = static_cast<Dispatch>(&EventDispatcher::dispatch);
    stub.set_lamda(dispatch, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(FileOperatorHelper::instance()->pasteFiles(0, {}, QUrl("file:///hello/world"), Qt::MoveAction));
    EXPECT_NO_FATAL_FAILURE(FileOperatorHelper::instance()->pasteFiles(0, {}, QUrl("file:///hello/world"), Qt::CopyAction));
}
