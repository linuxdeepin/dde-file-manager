/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "utils/dragdrophelper.h"
#include "dfm-base/utils/fileutils.h"

#include <gtest/gtest.h>

#include <QMimeData>

using namespace testing;

class DragDropHelperTest : public Test
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(DragDropHelperTest, dragEnter)
{
    DFMBASE_USE_NAMESPACE
    DPWORKSPACE_USE_NAMESPACE

    bool isCall { false };
    DragDropHelper dragDropHelper(nullptr);
    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);
    QMimeData data;

    stub.set_lamda(&DragDropHelper::checkProhibitPaths, []() -> bool {
        return true;
    });

    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return {};
    });

    stub.set_lamda(&QDropEvent::mimeData, [&]() -> const QMimeData * {
        return &data;
    });

    EXPECT_TRUE(dragDropHelper.dragEnter(&event));
}

TEST_F(DragDropHelperTest, checkProhibitPaths)
{
    DFMBASE_USE_NAMESPACE
    DPWORKSPACE_USE_NAMESPACE

    DragDropHelper opt(nullptr);
    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);

    // action1

    stub.set_lamda(&FileUtils::isContainProhibitPath, []() -> bool {
        return false;
    });
    EXPECT_EQ(true, opt.checkProhibitPaths(&event, {}));

    //action2
    EXPECT_EQ(false, opt.checkProhibitPaths(&event, { QUrl("/home/uos") }));
    stub.reset(&QMimeData::urls);
    stub.reset(&FileUtils::isContainProhibitPath);

    //action3
    stub.set_lamda(&FileUtils::isContainProhibitPath, []() -> bool {
        return true;
    });
    EXPECT_EQ(true, opt.checkProhibitPaths(&event, { QUrl("/home/uos") }));
}
