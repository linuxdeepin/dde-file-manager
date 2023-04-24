// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/dragdrophelper.h"
#include "views/fileview.h"
#include <dfm-base/utils/fileutils.h>

#include <gtest/gtest.h>

#include <QStandardPaths>
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
    FileView view(QUrl(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()));
    DragDropHelper dragDropHelper(&view);
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

    FileView view(QUrl(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()));
    DragDropHelper opt(&view);
    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);

    // action1

    stub.set_lamda(&FileUtils::isContainProhibitPath, []() -> bool {
        return false;
    });
    EXPECT_FALSE(opt.checkProhibitPaths(&event, {}));

    //action2
    EXPECT_FALSE(opt.checkProhibitPaths(&event, { QUrl("/home/uos") }));
    stub.reset(&QMimeData::urls);
    stub.reset(&FileUtils::isContainProhibitPath);

    //action3
    stub.set_lamda(&FileUtils::isContainProhibitPath, []() -> bool {
        return true;
    });
    EXPECT_TRUE(opt.checkProhibitPaths(&event, { QUrl("/home/uos") }));
}
