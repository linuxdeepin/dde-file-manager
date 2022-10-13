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
#include "view/canvasview_p.h"
#include "view/operator/dragdropoper.h"
#include "model/canvasproxymodel.h"
#include "dfm-base/utils/fileutils.h"

#include <DFileDragClient>

#include <gtest/gtest.h>

#include <QMimeData>
#include <QDropEvent>

using namespace testing;
using namespace ddplugin_canvas;

class DragDropOperTest : public Test
{
public:
    void SetUp() override {
    }
    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(DragDropOperTest, testEnter) {
    DFMBASE_USE_NAMESPACE
    DDP_CANVAS_USE_NAMESPACE
    DGUI_USE_NAMESPACE

    //  Prohibit drag  special directory
    CanvasView view;
    DragDropOper opt(&view);
    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);
    CanvasProxyModel *md = new CanvasProxyModel(nullptr);

    // action1
    stub.set_lamda(&DragDropOper::checkProhibitPaths, []() -> bool{
        return true;
    });

    EXPECT_EQ(true, opt.enter(&event));
    stub.reset(&DragDropOper::checkProhibitPaths);

    // action2
    stub.set_lamda(&DragDropOper::checkProhibitPaths, []() -> bool{
        return false;
    });

    stub.set_lamda(&CanvasView::model, [md]() -> CanvasProxyModel* {
        return md;
    });

    stub.set_lamda(&CanvasProxyModel::rootUrl, []() -> QUrl {
        return QUrl();
    });

    stub.set_lamda(&DFileDragClient::checkMimeData, []() -> bool {
        return true;
    });

    stub.set_lamda(&DFileDragClient::setTargetUrl, []() {
            return;
        });

    Qt::DropAction act = Qt::IgnoreAction;
    bool isCallUpdate = false;

    stub.set_lamda(&QDropEvent::setDropAction, [&]() {
        return;
    });

    stub.set_lamda(&DragDropOper::updatePrepareDodgeValue, [&]() {
        isCallUpdate = true;
        return;
    });

    EXPECT_EQ(true, opt.enter(&event));
    EXPECT_TRUE(!isCallUpdate);
    stub.reset(&DFileDragClient::checkMimeData);


    // action3
    stub.set_lamda(&DFileDragClient::checkMimeData, []() -> bool {
        return false;
    });

    stub.set_lamda(&DragDropOper::checkXdndDirectSave, []() -> bool {
        return true;
    });

    opt.enter(&event);
    EXPECT_TRUE(isCallUpdate);
    EXPECT_EQ(true, opt.enter(&event));
    stub.reset(&DragDropOper::checkXdndDirectSave);

    // action4
    stub.set_lamda(&DragDropOper::checkXdndDirectSave, []() -> bool {
        return false;
    });

    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
                       return {};
                   });

    typedef void (*func)(DragDropOper*,QDropEvent *event, const QList<QUrl> &urls, const QUrl &targetFileUrl);
    auto testFunc = (func)(&DragDropOper::preproccessDropEvent);
    stub.set_lamda(testFunc, []() {
        return;
    });

    EXPECT_TRUE(isCallUpdate);
    EXPECT_EQ(false, opt.enter(&event));
    delete md;
    md = nullptr;
}

TEST_F(DragDropOperTest, testCheckProhibitPaths) {
    DFMBASE_USE_NAMESPACE

    DragDropOper opt(nullptr);
    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);

    // action1
    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return {};
    });

    stub.set_lamda(&FileUtils::isContainProhibitPath, []() -> bool {
        return false;
    });
    EXPECT_EQ(true, opt.checkProhibitPaths(&event));
    stub.reset(&QMimeData::urls);

    //action2

    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return { QUrl("/home/uos") };
    });
    EXPECT_EQ(false, opt.checkProhibitPaths(&event));
    stub.reset(&QMimeData::urls);
    stub.reset(&FileUtils::isContainProhibitPath);

    //action3

    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return { QUrl("/home/uos") };
    });

    stub.set_lamda(&FileUtils::isContainProhibitPath, []() -> bool {
        return true;
    });
    EXPECT_EQ(true, opt.checkProhibitPaths(&event));
}
