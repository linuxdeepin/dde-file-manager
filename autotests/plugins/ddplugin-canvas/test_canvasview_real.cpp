// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#define private public
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel_p.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasselectionmodel.h"
#include "plugins/desktop/ddplugin-canvas/view/canvasview.h"
#include "plugins/desktop/ddplugin-canvas/view/canvasview_p.h"
#include "plugins/desktop/ddplugin-canvas/delegate/canvasitemdelegate.h"
#include "plugins/desktop/ddplugin-canvas/grid/canvasgrid.h"
#undef private

#include <dfm-base/base/application/application.h>
#include "canvas_test_common.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <QTemporaryDir>
#include <QFile>
#include <QStyleOptionViewItem>
#include <QItemSelectionModel>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace ddplugin_canvas;

namespace {

QUrl makeUrl(QTemporaryDir *tempDir, const QString &name)
{
    const QString path = tempDir->filePath(name);
    QFile f(path);
    if (!QFile::exists(path)) {
        f.open(QIODevice::WriteOnly);
        f.close();
    }
    return QUrl::fromLocalFile(path);
}

} // namespace

class CanvasViewReal : public testing::Test
{
protected:
    void SetUp() override
    {
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());

        stub.set_lamda(ADDR(Application, instance), []() -> Application * {
            return canvas_test::sharedApp();
        });
        stub.set_lamda(ADDR(Application, appAttribute),
                       [](Application::ApplicationAttribute) -> QVariant { return QVariant(true); });

        stub.set_lamda(
            static_cast<FileInfoPointer (*)(const QUrl &, dfmbase::Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
            [](const QUrl &url, dfmbase::Global::CreateFileInfoType, QString *err) -> FileInfoPointer {
                if (err)
                    *err = QString();
                FileInfoPointer info(new SyncFileInfo(url));
                if (info)
                    info->refresh();
                return info;
            });

        srcModel = new FileInfoModel();
        srcModel->setRootUrl(QUrl::fromLocalFile(tempDir->path()));

        urls.clear();
        urls << makeUrl(tempDir.data(), "one.txt")
             << makeUrl(tempDir.data(), "two.txt");

        proxy = new CanvasProxyModel();
        proxy->setSourceModel(srcModel);
        srcModel->d->resetData(urls);

        // Configure the shared grid so that the view can map items to visual positions.
        grid = CanvasGrid::instance();
        grid->requestSync(0);
        grid->initSurface(1);
        grid->setMode(CanvasGrid::Mode::Custom);
        grid->updateSize(1, QSize(4, 4));
        grid->setItems({ urls.at(0).toString(), urls.at(1).toString() });

        view = new CanvasView(nullptr);
        view->setScreenNum(1);
        view->setModel(proxy);
        view->setSelectionModel(new CanvasSelectionModel(proxy, view));
        auto delegate = new CanvasItemDelegate(view);
        view->setItemDelegate(delegate);
        view->setIconSize(delegate->iconSize(delegate->iconLevel()));
        delegate->updateItemSizeHint();
        view->setGeometry(QRect(0, 0, 800, 600));
        view->updateGrid();
    }

    void TearDown() override
    {
        delete view;
        view = nullptr;
        delete proxy;
        proxy = nullptr;
        delete srcModel;
        srcModel = nullptr;
        grid->initSurface(1);
        grid->setMode(CanvasGrid::Mode::Custom);
        grid->setItems(QStringList());
        stub.clear();
    }

    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tempDir;
    FileInfoModel *srcModel = nullptr;
    CanvasProxyModel *proxy = nullptr;
    CanvasView *view = nullptr;
    CanvasGrid *grid = nullptr;
    QList<QUrl> urls;
};

TEST_F(CanvasViewReal, constructAndAccessors)
{
    EXPECT_NE(view, nullptr);
    EXPECT_EQ(view->model(), proxy);
    EXPECT_EQ(view->screenNum(), 1);

    EXPECT_NO_THROW(view->setGeometry(QRect(0, 0, 640, 480)));
    EXPECT_NO_THROW(view->updateGrid());
    view->showGrid(true);
    EXPECT_TRUE(view->d->showGrid);
}

TEST_F(CanvasViewReal, visualRectAndIndexAt)
{
    QModelIndex first = proxy->index(urls.at(0));
    EXPECT_TRUE(first.isValid());

    QRect rect = view->visualRect(first);
    EXPECT_FALSE(rect.isEmpty());

    QList<QRect> geos = view->itemPaintGeomertys(first);
    EXPECT_FALSE(geos.isEmpty());

    QRect expended = view->expendedVisualRect(first);
    EXPECT_FALSE(expended.isEmpty());

    QPoint center = rect.center();
    QModelIndex at = view->indexAt(center);
    EXPECT_EQ(at, first);

    QModelIndex baseAt = view->baseIndexAt(center);
    EXPECT_EQ(baseAt, first);
}

TEST_F(CanvasViewReal, offsetsAndHidden)
{
    EXPECT_GE(view->horizontalOffset(), 0);
    EXPECT_GE(view->verticalOffset(), 0);

    QModelIndex first = proxy->index(urls.at(0));
    EXPECT_FALSE(view->isIndexHidden(first));
}

TEST_F(CanvasViewReal, selectionAndCursor)
{
    QModelIndex first = proxy->index(urls.at(0));
    view->selectionModel()->select(first, QItemSelectionModel::ClearAndSelect);

    EXPECT_NO_THROW(view->selectAll());
    EXPECT_NO_THROW(view->toggleSelect());

    EXPECT_NO_THROW(view->scrollTo(first, QAbstractItemView::EnsureVisible));
    EXPECT_EQ(view->moveCursor(QAbstractItemView::MoveHome, Qt::NoModifier), view->d->firstIndex());
}

TEST_F(CanvasViewReal, resetAndRefresh)
{
    EXPECT_NO_THROW(view->reset());
    EXPECT_NO_THROW(view->refresh(true));
}

TEST_F(CanvasViewReal, inputMethodAndWinId)
{
    QVariant area = view->inputMethodQuery(Qt::ImCursorRectangle);
    EXPECT_TRUE(area.isValid());

    // winId requires a native window; just ensure the code path runs without crash.
    WId id = view->winId();
    (void)id;
    SUCCEED();
}
