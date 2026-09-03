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
#include "plugins/desktop/ddplugin-canvas/delegate/canvasitemdelegate.h"
#undef private

#include <dfm-base/base/application/application.h>
#include "canvas_test_common.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <QTemporaryDir>
#include <QFile>
#include <QStyleOptionViewItem>

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

class CanvasItemDelegateReal : public testing::Test
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

        QList<QUrl> urls;
        urls << makeUrl(tempDir.data(), "alpha.txt")
             << makeUrl(tempDir.data(), "beta.txt");

        proxy = new CanvasProxyModel();
        proxy->setSourceModel(srcModel);
        srcModel->d->resetData(urls);

        view = new CanvasView(nullptr);
        view->setModel(proxy);
        view->setSelectionModel(new CanvasSelectionModel(proxy, view));
        delegate = new CanvasItemDelegate(view);
        view->setItemDelegate(delegate);

        // Initialize the delegate's cached item size so that sizeHint returns a real value.
        view->setIconSize(delegate->iconSize(delegate->iconLevel()));
        delegate->updateItemSizeHint();
    }

    void TearDown() override
    {
        delete view;
        view = nullptr;
        delete proxy;
        proxy = nullptr;
        delete srcModel;
        srcModel = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tempDir;
    FileInfoModel *srcModel = nullptr;
    CanvasProxyModel *proxy = nullptr;
    CanvasView *view = nullptr;
    CanvasItemDelegate *delegate = nullptr;
};

TEST_F(CanvasItemDelegateReal, constructAndAccessors)
{
    EXPECT_NE(delegate, nullptr);
    EXPECT_EQ(delegate->parent(), view);

    EXPECT_GT(delegate->iconSize(0).width(), 0);
    EXPECT_EQ(delegate->minimumIconLevel(), 0);
    EXPECT_GT(delegate->maximumIconLevel(), delegate->minimumIconLevel());
    EXPECT_GE(delegate->iconLevel(), delegate->minimumIconLevel());
    EXPECT_LE(delegate->iconLevel(), delegate->maximumIconLevel());

    int prev = delegate->iconLevel();
    int next = delegate->setIconLevel(delegate->maximumIconLevel());
    EXPECT_EQ(delegate->iconLevel(), next);
    EXPECT_GT(delegate->textLineHeight(), 0);
    delegate->setIconLevel(prev);
}

TEST_F(CanvasItemDelegateReal, sizeHintAndRects)
{
    QModelIndex idx = proxy->index(0, 0);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);

    QSize hint = delegate->sizeHint(option, idx);
    EXPECT_GT(hint.width(), 0);
    EXPECT_GT(hint.height(), 0);

    QList<QRect> geos = delegate->paintGeomertys(option, idx);
    EXPECT_FALSE(geos.isEmpty());

    QRect icon = delegate->iconRect(option.rect);
    EXPECT_FALSE(icon.isEmpty());

    QRect label = CanvasItemDelegate::labelRect(option.rect, icon);
    EXPECT_FALSE(label.isEmpty());

    QRectF expended = delegate->expendedGeomerty(option, idx);
    EXPECT_FALSE(expended.isEmpty());

    QList<QRectF> list { QRectF(0, 0, 10, 10), QRectF(5, 5, 10, 10) };
    QRectF bound = CanvasItemDelegate::boundingRect(list);
    EXPECT_FALSE(bound.isEmpty());
}

TEST_F(CanvasItemDelegateReal, editorLifecycle)
{
    QModelIndex idx = proxy->index(0, 0);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);

    QWidget *editor = delegate->createEditor(view, option, idx);
    EXPECT_NE(editor, nullptr);

    delegate->setEditorData(editor, idx);
    delegate->updateEditorGeometry(editor, option, idx);

    delete editor;
}
