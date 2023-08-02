// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "view/operator/fileoperatorproxy.h"
#include "view/operator/fileoperatorproxy_p.h"
#include "grid/canvasgrid.h"
#include "view/canvasview.h"
#include "view/canvasview_p.h"
#include "private/canvasmanager_p.h"
#include "model/canvasproxymodel.h"
#include "model/canvasselectionmodel.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/standardpaths.h>

#include <dfm-framework/dpf.h>

#include "stubext.h"
#include <gtest/gtest.h>


DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#define KEY_SCREENNUMBER "screenNumber"
#define KEY_POINT "point"
using namespace ddplugin_canvas ;
class UT_FileOperatorProxy : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        file = new FileOperatorProxy();
        fileP = new FileOperatorProxyPrivate(file);
        md = new CanvasProxyModel(nullptr);
        sm = new CanvasSelectionModel(md,nullptr);
        stub.set_lamda(&CanvasView::model, [this]() -> CanvasProxyModel* {
            __DBG_STUB_INVOKE__
            return md;
        });
        stub.set_lamda(&CanvasView::selectionModel, [this]() -> CanvasSelectionModel* {
            __DBG_STUB_INVOKE__
            return sm;
        });
        stub.set_lamda(&FileInfoModel::rootUrl,[](){
            __DBG_STUB_INVOKE__
                    return QUrl("temp");
        });
        stub.set_lamda(&CanvasManager::views,[](){
            __DBG_STUB_INVOKE__
                   CanvasViewPointer ptr(new  CanvasView());
                   QList<CanvasViewPointer> res{ptr};
                   return res;
        });

    }
    virtual void TearDown() override
    {
        delete fileP;
        delete file;
        delete md;
        delete sm;
        stub.clear();
    }

    FileOperatorProxy *file = nullptr;
    FileOperatorProxyPrivate *fileP = nullptr;
    CanvasProxyModel *md = nullptr;
    CanvasSelectionModel *sm = nullptr;
    stub_ext::StubExt stub ;
};

TEST_F(UT_FileOperatorProxy, callBackTouchFile)
{

    QUrl target = QUrl("file:/temp_url");
    QVariantMap customData;
    customData.insert(KEY_SCREENNUMBER, QVariant::fromValue(2));
    customData.insert(KEY_POINT,QVariant::fromValue(QPoint(1,1)));
    fileP->callBackTouchFile(target,customData);
    stub.set_lamda(&CanvasGrid::point,[](CanvasGrid *slfe,const QString &item, QPair<int, QPoint> &pos){
        __DBG_STUB_INVOKE__
                return true;
    });
    fileP->callBackTouchFile(target,customData);
    EXPECT_EQ(file->d->touchFileData, qMakePair(QString(), qMakePair(-1, QPoint(-1, -1))));

}

TEST_F(UT_FileOperatorProxy, callBackPasteFiles)
{
    QObject parent;
    CanvasManager manager(&parent);

    JobInfoPointer info { new QMap<quint8,QVariant>()};
    QUrl url1 = QUrl::fromLocalFile("file:/temp_url1");
    QUrl url2 = QUrl::fromLocalFile("file:/temp_url2");
    QList<QUrl> urls {url1,url2};
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey,QVariant::fromValue(urls));
    fileP->callBackPasteFiles(info);
    CanvasProxyModel proxyModel;

    CanvasSelectionModel select(&proxyModel,nullptr);
    CanvasIns->d->selectionModel = &select;
    CanvasIns->d->canvasModel = &proxyModel;

    fileP->callBackPasteFiles(info);
    EXPECT_TRUE(fileP->pasteFileData.contains(url1));
    EXPECT_TRUE(fileP->pasteFileData.contains(url2));
}

TEST_F(UT_FileOperatorProxy, callBackRenameFiles)
{
    QUrl url1 = QUrl::fromLocalFile("source_url1");
    QUrl url2 = QUrl::fromLocalFile("source_url2");
    QUrl turl1 = QUrl::fromLocalFile("target_url1");
    QUrl turl2 = QUrl::fromLocalFile("target_url2");
    QList<QUrl> urls {url1,url2};
    QList<QUrl> targets{turl1,turl2};

    fileP->callBackRenameFiles(urls,targets);
    EXPECT_EQ(fileP->renameFileData.size(),2);
    EXPECT_TRUE(fileP->renameFileData.contains(url1));
    EXPECT_TRUE(fileP->renameFileData.contains(url2));
}

TEST_F(UT_FileOperatorProxy, filterDesktopFile)
{
    QUrl url1 = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-trash.desktop");
    QUrl url2= QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-computer.desktop");
    QUrl url3 = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-home.desktop");
    QList<QUrl> urls{url1,url2,url3};
    fileP->filterDesktopFile(urls);
    EXPECT_TRUE(urls.isEmpty());
}

TEST_F(UT_FileOperatorProxy, touchFile)
{
    typedef bool (dpf::EventDispatcherManager::*Publish)(dpf::EventType,unsigned long long,
                                                         QUrl &rootUrl,
                                                         const DFMBASE_NAMESPACE::Global::CreateFileType &type,
                                                         const QString &suffix, QVariant &custom,
                                                         AbstractJobHandler::OperatorCallback &callback);
    stub.set_lamda((Publish)(&dpf::EventDispatcherManager::publish), [] {
        return true;
    });

    CanvasView view ;
    view.setScreenNum(2);
    DFMBASE_NAMESPACE::Global::CreateFileType type =CreateFileType::kCreateFileTypeFolder;
    EXPECT_NO_FATAL_FAILURE(file->touchFile(&view,QPoint(1,1),type,QString("temp_str")));


    EXPECT_NO_FATAL_FAILURE(file->touchFile(&view,QPoint(1,1),QString("temp_str")));

}

TEST_F(UT_FileOperatorProxy, copyFiles)
{

    typedef bool (dpf::EventDispatcherManager::*Publish)(dpf::EventType,unsigned long long,
                                                         QUrl &rootUrl,
                                                         const DFMBASE_NAMESPACE::Global::CreateFileType &type,
                                                         QString &suffix, QVariant &custom,
                                                         AbstractJobHandler::OperatorCallback &callback);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] (dpf::EventDispatcherManager*self, dpf::EventType,unsigned long long,
                   QUrl &rootUrl,
                   const DFMBASE_NAMESPACE::Global::CreateFileType &type,
                   QString &suffix, QVariant &custom,
                   AbstractJobHandler::OperatorCallback &callback){ return true; });
    stub.set_lamda(&CanvasSelectionModel::selectedUrls,[](){
    __DBG_STUB_INVOKE__
             QList<QUrl> res;
            res.push_back(QUrl("temp_url"));
            return res;
    });
    CanvasView view ;
    EXPECT_NO_FATAL_FAILURE(file->copyFiles(&view));
}

TEST_F(UT_FileOperatorProxy, touchFolder)
{

    typedef bool (dpf::EventDispatcherManager::*Publish)(dpf::EventType,unsigned long long,
                                                         QUrl &rootUrl, QVariant &custom,
                                                         AbstractJobHandler::OperatorCallback &callback);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { return true; });
    CanvasView view;
    EXPECT_NO_FATAL_FAILURE(file->touchFolder(&view,QPoint(1,1)));
}
TEST_F(UT_FileOperatorProxy, cutFiles)
{
    typedef bool (dpf::EventDispatcherManager::*Publish)(dpf::EventType,unsigned long long,ClipBoard::ClipboardAction&,
                                                         QUrl &rootUrl);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { return true; });

    CanvasView view;
    EXPECT_NO_FATAL_FAILURE(file->cutFiles(&view));
}
TEST_F(UT_FileOperatorProxy, pasteFiles)
{
    CanvasView view ;
    EXPECT_NO_FATAL_FAILURE(file->pasteFiles(&view,QPoint(1,1)));
}

TEST_F(UT_FileOperatorProxy, file)
{
    QUrl url1 = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-trash.desktop");
    QUrl url2= QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-computer.desktop");
    QUrl url3 = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-home.desktop");
    QList<QUrl> urls{url1,url2,url3};
    CanvasView view;
    file->openFiles(&view);
    typedef bool (dpf::EventDispatcherManager::*Publish)(dpf::EventType,unsigned long long,
                                                         const QList<QUrl> &rootUrl);
    auto publish = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish, [] { return true; });
    EXPECT_NO_FATAL_FAILURE(file->openFiles(&view,urls));
    EXPECT_NO_FATAL_FAILURE(file->renameFile(1,url1,url2));
    QPair<QString, QString> pair;
    EXPECT_NO_FATAL_FAILURE(file->renameFiles(&view,urls,pair,true));
    QPair<QString, AbstractJobHandler::FileNameAddFlag> pair2;
    EXPECT_NO_FATAL_FAILURE(file->renameFiles(&view,urls,pair2));
    EXPECT_NO_FATAL_FAILURE(file->moveToTrash(&view));
    EXPECT_NO_FATAL_FAILURE(file->deleteFiles(&view));
    EXPECT_NO_FATAL_FAILURE(file->showFilesProperty(&view));
    EXPECT_NO_FATAL_FAILURE(file->sendFilesToBluetooth(&view));
    EXPECT_NO_FATAL_FAILURE(file->undoFiles(&view));
    EXPECT_NO_FATAL_FAILURE(file->dropToTrash(urls));
    EXPECT_NO_FATAL_FAILURE(file->dropFiles(Qt::CopyAction,url1,urls));
    EXPECT_NO_FATAL_FAILURE(file->dropToApp(urls,QString("window")));
}


TEST_F(UT_FileOperatorProxy, callBackFunction)
{
    stub.set_lamda(&FileOperatorProxyPrivate::callBackRenameFiles,
                   [](FileOperatorProxyPrivate*,const QList<QUrl> &sources, const QList<QUrl> &){
        __DBG_STUB_INVOKE__
    });
    QUrl url1 = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-trash.desktop");
    QUrl url2= QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-computer.desktop");
    QUrl url3 = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-home.desktop");
    QList<QUrl> urls{url1,url2,url3};
    JobHandlePointer job(new DFMBASE_NAMESPACE::AbstractJobHandler);
    AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>());
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> cunstom( FileOperatorProxyPrivate::CallBackFunc::kCallBackTouchFile,QVariant::fromValue(1));
    args->insert(AbstractJobHandler::CallbackKey::kCustom,QVariant::fromValue(cunstom));
    args->insert(AbstractJobHandler::CallbackKey::kTargets,QVariant::fromValue(urls));
    args->insert(AbstractJobHandler::CallbackKey::kJobHandle,QVariant::fromValue(job));
    args->insert(AbstractJobHandler::CallbackKey::kSourceUrls,QVariant::fromValue(urls));
    args->insert(AbstractJobHandler::CallbackKey::kTargets,QVariant::fromValue(urls));

    EXPECT_NO_FATAL_FAILURE(file->callBackFunction(args));
    cunstom.first = FileOperatorProxyPrivate::CallBackFunc::kCallBackTouchFolder;
    args->insert(AbstractJobHandler::CallbackKey::kCustom,QVariant::fromValue(cunstom));
    EXPECT_NO_FATAL_FAILURE(file->callBackFunction(args));
    cunstom.first = FileOperatorProxyPrivate::CallBackFunc::kCallBackPasteFiles;
    args->insert(AbstractJobHandler::CallbackKey::kCustom,QVariant::fromValue(cunstom));
    EXPECT_NO_FATAL_FAILURE(file->callBackFunction(args));
    cunstom.first = FileOperatorProxyPrivate::CallBackFunc::kCallBackRenameFiles;
    args->insert(AbstractJobHandler::CallbackKey::kCustom,QVariant::fromValue(cunstom));
    EXPECT_NO_FATAL_FAILURE(file->callBackFunction(args));
}
