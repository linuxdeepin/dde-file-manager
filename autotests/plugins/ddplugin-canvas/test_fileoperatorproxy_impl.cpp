// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"
#include "canvas_test_common.h"

#include "plugins/desktop/ddplugin-canvas/view/operator/fileoperatorproxy.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/fileoperatorproxy_p.h"
#include "plugins/desktop/ddplugin-canvas/canvasmanager.h"
#include "plugins/desktop/ddplugin-canvas/view/canvasview.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasselectionmodel.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-framework/dpf.h>

#include <QMimeData>
#include <QModelIndex>
#include <QThread>
#include <QTimer>
#include <QUrl>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace ddplugin_canvas;

namespace {

const QString kKeyScreenNumber = QStringLiteral("screenNumber");
const QString kKeyPoint = QStringLiteral("point");

} // namespace

class FileOperatorProxyImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        // Swallow all signal events so real handlers are never invoked.
        filterObj = new QObject();
        dpfSignalDispatcher->installGlobalEventFilter(
            filterObj, [](dpf::EventType, const QVariantList &) -> bool { return true; });

        // Keep CanvasManager construction lightweight.
        using QThreadStartFunc = void (QThread::*)(QThread::Priority);
        stub.set_lamda(static_cast<QThreadStartFunc>(&QThread::start), [](QThread *, QThread::Priority) {});
        using QTimerStartVoidFunc = void (QTimer::*)();
        using QTimerStartIntFunc = void (QTimer::*)(int);
        stub.set_lamda(static_cast<QTimerStartVoidFunc>(&QTimer::start), [](QTimer *) {});
        stub.set_lamda(static_cast<QTimerStartIntFunc>(&QTimer::start), [](QTimer *, int) {});

        stub.set_lamda(ADDR(Application, instance), []() -> Application * {
            return canvas_test::sharedApp();   // single shared instance
        });
        stub.set_lamda(ADDR(Application, appAttribute),
                       [](Application::ApplicationAttribute) -> QVariant { return QVariant(); });

        manager = new CanvasManager();
        // CanvasProxyModel::rootUrl() dereferences the source model, so wire
        // a real FileInfoModel underneath to avoid a null-pointer crash.
        srcModel = new FileInfoModel();
        proxyModel = new CanvasProxyModel();
        proxyModel->setSourceModel(srcModel);
        selectionModel = new CanvasSelectionModel(proxyModel, nullptr);
        view = new CanvasView();

        // CanvasManager stubs.
        stub.set_lamda(&CanvasManager::instance, [this]() -> CanvasManager * { return manager; });

        using ViewsFunc = QList<QSharedPointer<CanvasView>> (CanvasManager::*)() const;
        stub.set_lamda(static_cast<ViewsFunc>(&CanvasManager::views),
                       [this](CanvasManager *) -> QList<QSharedPointer<CanvasView>> {
                           QList<QSharedPointer<CanvasView>> list;
                           list.append(QSharedPointer<CanvasView>(view, [](CanvasView *) {}));
                           return list;
                       });

        using SelModelFunc = CanvasSelectionModel * (CanvasManager::*)() const;
        stub.set_lamda(static_cast<SelModelFunc>(&CanvasManager::selectionModel),
                       [this](CanvasManager *) -> CanvasSelectionModel * { return selectionModel; });

        using ModelFunc = CanvasProxyModel * (CanvasManager::*)() const;
        stub.set_lamda(static_cast<ModelFunc>(&CanvasManager::model),
                       [this](CanvasManager *) -> CanvasProxyModel * { return proxyModel; });

        // CanvasView stubs.
        stub.set_lamda(ADDR(CanvasView, screenNum), [](CanvasView *) -> int { return 1; });
        stub.set_lamda(ADDR(CanvasView, winId), [](CanvasView *) -> WId { return 12345; });

        using ViewModelFunc = CanvasProxyModel * (CanvasView::*)() const;
        stub.set_lamda(static_cast<ViewModelFunc>(&CanvasView::model),
                       [this](CanvasView *) -> CanvasProxyModel * { return proxyModel; });

        using ViewSelFunc = CanvasSelectionModel * (CanvasView::*)() const;
        stub.set_lamda(static_cast<ViewSelFunc>(&CanvasView::selectionModel),
                       [this](CanvasView *) -> CanvasSelectionModel * { return selectionModel; });

        // Selection stub.
        using SelectedUrlsFunc = QList<QUrl> (CanvasSelectionModel::*)() const;
        stub.set_lamda(static_cast<SelectedUrlsFunc>(&CanvasSelectionModel::selectedUrls),
                       [this](CanvasSelectionModel *) -> QList<QUrl> { return selectedUrls; });

        // Clipboard stub.
        stub.set_lamda(ADDR(ClipBoard, instance), []() -> ClipBoard * {
            static ClipBoard cb;
            return &cb;
        });
        using ClipUrlsFunc = QList<QUrl> (ClipBoard::*)() const;
        stub.set_lamda(static_cast<ClipUrlsFunc>(&ClipBoard::clipboardFileUrlList),
                       [this](ClipBoard *) -> QList<QUrl> { return clipUrls; });
        using ClipActionFunc = ClipBoard::ClipboardAction (ClipBoard::*)() const;
        stub.set_lamda(static_cast<ClipActionFunc>(&ClipBoard::clipboardAction),
                       [this](ClipBoard *) -> ClipBoard::ClipboardAction { return clipAction; });
        stub.set_lamda(&ClipBoard::clearClipboard, [this]() { clipboardCleared = true; });

        selectedUrls = { QUrl("file:///tmp/a.txt"), QUrl("file:///tmp/b.txt") };
        clipUrls = { QUrl("file:///tmp/c.txt") };
        clipAction = ClipBoard::kCopyAction;
    }

    void TearDown() override
    {
        FileOperatorProxy::instance()->clearTouchFileData();
        FileOperatorProxy::instance()->clearRenameFileData();
        FileOperatorProxy::instance()->clearPasteFileData();

        dpfSignalDispatcher->removeGlobalEventFilter(filterObj);
        delete filterObj;

        delete manager;
        delete selectionModel;
        delete proxyModel;
        delete srcModel;
        delete view;
        stub.clear();
    }

    QObject *filterObj = nullptr;
    CanvasManager *manager = nullptr;
    CanvasProxyModel *proxyModel = nullptr;
    FileInfoModel *srcModel = nullptr;
    CanvasSelectionModel *selectionModel = nullptr;
    CanvasView *view = nullptr;

    QList<QUrl> selectedUrls;
    QList<QUrl> clipUrls;
    ClipBoard::ClipboardAction clipAction = ClipBoard::kCopyAction;
    bool clipboardCleared = false;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorProxyImpl, instanceAndDataAccessors)
{
    FileOperatorProxy *proxy = FileOperatorProxy::instance();
    EXPECT_NE(proxy, nullptr);
    EXPECT_EQ(proxy, FileOperatorProxy::instance());

    EXPECT_TRUE(proxy->touchFileData().first.isEmpty());
    EXPECT_TRUE(proxy->renameFileData().isEmpty());
    EXPECT_TRUE(proxy->pasteFileData().isEmpty());

    proxy->clearTouchFileData();
    proxy->clearRenameFileData();
    proxy->clearPasteFileData();
}

TEST_F(FileOperatorProxyImpl, touchFileAndFolder)
{
    FileOperatorProxy *proxy = FileOperatorProxy::instance();

    QPoint pos(10, 20);
    proxy->touchFile(view, pos, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeText, QStringLiteral("txt"));
    proxy->touchFile(view, pos, QUrl("file:///tmp/template.txt"));
    proxy->touchFolder(view, pos);

    // The methods are fire-and-forget; just verify no crash and data accessors stay clean.
    EXPECT_TRUE(proxy->touchFileData().first.isEmpty());
}

TEST_F(FileOperatorProxyImpl, copyCutPaste)
{
    FileOperatorProxy *proxy = FileOperatorProxy::instance();

    EXPECT_NO_THROW(proxy->copyFiles(view));
    EXPECT_NO_THROW(proxy->copyFilePath(view));
    EXPECT_NO_THROW(proxy->cutFiles(view));

    EXPECT_NO_THROW(proxy->pasteFiles(view, QPoint(5, 5)));

    clipAction = ClipBoard::kCutAction;
    EXPECT_NO_THROW(proxy->pasteFiles(view));
    EXPECT_TRUE(clipboardCleared);
}

TEST_F(FileOperatorProxyImpl, openAndRename)
{
    FileOperatorProxy *proxy = FileOperatorProxy::instance();

    EXPECT_NO_THROW(proxy->openFiles(view));
    EXPECT_NO_THROW(proxy->openFiles(view, selectedUrls));

    proxy->renameFile(12345, QUrl("file:///tmp/old.txt"), QUrl("file:///tmp/new.txt"));

    QPair<QString, QString> pattern { "old", "new" };
    proxy->renameFiles(view, selectedUrls, pattern, false);

    QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> flagPattern {
        "base", DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag::kPrefix
    };
    proxy->renameFiles(view, selectedUrls, flagPattern);
}

TEST_F(FileOperatorProxyImpl, trashDeletePropertyBluetoothUndoRedo)
{
    FileOperatorProxy *proxy = FileOperatorProxy::instance();

    EXPECT_NO_THROW(proxy->moveToTrash(view));
    EXPECT_NO_THROW(proxy->deleteFiles(view));
    EXPECT_NO_THROW(proxy->showFilesProperty(view));
    EXPECT_NO_THROW(proxy->sendFilesToBluetooth(view));
    EXPECT_NO_THROW(proxy->undoFiles(view));
    EXPECT_NO_THROW(proxy->redoFiles(view));
}

TEST_F(FileOperatorProxyImpl, dropOperations)
{
    FileOperatorProxy *proxy = FileOperatorProxy::instance();

    QList<QUrl> urls = selectedUrls;
    QUrl target("file:///tmp/target");

    EXPECT_NO_THROW(proxy->dropFiles(Qt::CopyAction, target, urls));
    EXPECT_NO_THROW(proxy->dropFiles(Qt::MoveAction, target, urls));
    EXPECT_NO_THROW(proxy->dropToTrash(urls));
    EXPECT_NO_THROW(proxy->dropToApp(urls, QStringLiteral("/usr/share/applications/app.desktop")));
}

TEST_F(FileOperatorProxyImpl, dataMapsManipulation)
{
    FileOperatorProxy *proxy = FileOperatorProxy::instance();

    QUrl oldUrl("file:///tmp/old.txt");
    QUrl newUrl("file:///tmp/new.txt");

    QHash<QUrl, QUrl> renameData;
    renameData.insert(oldUrl, newUrl);

    // Use the callback path to populate rename data.
    DFMBASE_NAMESPACE::AbstractJobHandler::CallbackArgus arg(
        new QMap<DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey, QVariant>());
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> custom(
        FileOperatorProxyPrivate::kCallBackRenameFiles, QVariant());
    (*arg)[DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kCustom] = QVariant::fromValue(custom);
    (*arg)[DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kSourceUrls] = QVariant::fromValue(QList<QUrl> { oldUrl });
    (*arg)[DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kTargets] = QVariant::fromValue(QList<QUrl> { newUrl });
    proxy->callBackFunction(arg);

    EXPECT_EQ(proxy->renameFileData().value(oldUrl), newUrl);
    proxy->removeRenameFileData(oldUrl);
    EXPECT_TRUE(proxy->renameFileData().isEmpty());

    // pasteFileData() returns a const copy; it cannot be mutated through the
    // public API. Removing an unknown URL must be a harmless no-op.
    EXPECT_FALSE(proxy->pasteFileData().contains(oldUrl));
    proxy->removePasteFileData(oldUrl);
    EXPECT_FALSE(proxy->pasteFileData().contains(oldUrl));
}

TEST_F(FileOperatorProxyImpl, touchFileCallback)
{
    FileOperatorProxy *proxy = FileOperatorProxy::instance();

    QUrl target("file:///tmp/created.txt");
    QVariantMap data;
    data.insert(kKeyScreenNumber, 1);
    data.insert(kKeyPoint, QPoint(7, 8));

    DFMBASE_NAMESPACE::AbstractJobHandler::CallbackArgus arg(
        new QMap<DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey, QVariant>());
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> custom(
        FileOperatorProxyPrivate::kCallBackTouchFile, data);
    (*arg)[DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kCustom] = QVariant::fromValue(custom);
    (*arg)[DFMBASE_NAMESPACE::AbstractJobHandler::CallbackKey::kTargets] = QVariant::fromValue(QList<QUrl> { target });

    proxy->callBackFunction(arg);

    auto touchData = proxy->touchFileData();
    EXPECT_EQ(touchData.first, target.toString());
    EXPECT_EQ(touchData.second.first, 1);
    EXPECT_EQ(touchData.second.second, QPoint(7, 8));
}
