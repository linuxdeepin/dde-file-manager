// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/fileoperator_p.h"
#include "view/collectionview_p.h"
#include "models/collectionmodel_p.h"

#include "dfm-base/utils/clipboard.h"
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

#include <stubext.h>

#include <gtest/gtest.h>
#include <QList>
#include <QUrl>
#include <QVariantHash>
#include <qwindowdefs.h>

DPF_USE_NAMESPACE
DDP_ORGANIZER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace Global;

TEST(FileOperatorPrivate, filterDesktopFile)
{
    QUrl one = QUrl::fromLocalFile("/tmp/1");
    QList<QUrl> urls;
    urls.append(DesktopAppUrl::computerDesktopFileUrl());
    urls.append(DesktopAppUrl::trashDesktopFileUrl());
    urls.append(one);
    urls.append(DesktopAppUrl::homeDesktopFileUrl());

    FileOperatorPrivate::filterDesktopFile(urls);
    ASSERT_EQ(urls.size(), 1);
    EXPECT_EQ(urls.first(), one);
}

namespace  {
class TestFileOperator : public testing::Test
{
public:
    virtual void SetUp() override {
        view = new CollectionView("1", nullptr);
        sel.setModel(&model);
        view->QAbstractItemView::setModel(&model);
        view->setSelectionModel(&sel);
    }
    virtual void TearDown() override {
        stub.clear();
        delete view;
    }

    CollectionView *view;
    stub_ext::StubExt stub;
    FileOperator fo;
    QItemSelectionModel sel;
    CollectionModel model;
};
}

TEST_F(TestFileOperator, callBackRenameFiles)
{
    bool clear = false;
    stub.set_lamda(&QItemSelectionModel::clearSelection, [&clear](){
       clear = true;
    });

    bool clearidx = false;
    stub.set_lamda(VADDR(QItemSelectionModel,clearCurrentIndex), [&clearidx](){
       clearidx = true;
    });

    fo.d->renameFileData.insert(QUrl(), QUrl());

    fo.d->callBackRenameFiles({}, {}, view);
    EXPECT_TRUE(clear);
    EXPECT_TRUE(clearidx);
    EXPECT_TRUE(fo.d->renameFileData.isEmpty());

    clear = false;
    clearidx = false;
    fo.d->renameFileData.clear();
    QUrl one = QUrl::fromLocalFile("/tmp/1");
    QUrl two = QUrl::fromLocalFile("/tmp/2");
    QUrl one1 = QUrl::fromLocalFile("/tmp/11");
    QUrl two1= QUrl::fromLocalFile("/tmp/22");

    fo.d->callBackRenameFiles({one,two}, {one1,two1}, view);
    EXPECT_TRUE(clear);
    EXPECT_TRUE(clearidx);
    ASSERT_EQ(fo.d->renameFileData.size(), 2);
    EXPECT_EQ(fo.d->renameFileData.value(one), one1);
    EXPECT_EQ(fo.d->renameFileData.value(two), two1);
}

TEST_F(TestFileOperator, getSelectedUrls)
{
    QUrl one = QUrl::fromLocalFile("/tmp/1");
    QUrl two = QUrl::fromLocalFile("/tmp/2");
    model.d->fileList.append(one);
    model.d->fileList.append(two);

    stub.set_lamda(&QItemSelectionModel::selectedIndexes, [this](){
        return QModelIndexList{QModelIndex(0,0, nullptr, &model),
                    QModelIndex(1,0, nullptr, &model)};
    });

    stub.set_lamda(VADDR(CollectionView,isIndexHidden), [](){
        return false;
    });

    auto ret = fo.d->getSelectedUrls(view);
    ASSERT_EQ(ret.size(), 2);
    EXPECT_EQ(ret.first(), one);
    EXPECT_EQ(ret.last(), two);
}

TEST_F(TestFileOperator, copyFiles)
{
    QUrl one = QUrl::fromLocalFile("/tmp/1");
    stub.set_lamda(&FileOperatorPrivate::getSelectedUrls, [one](){
        return QList<QUrl>{one};
    });

    QList<QUrl> in;
    typedef bool (EventDispatcherManager::*PublishFunc)(dpf::EventType, unsigned long long , ClipBoard::ClipboardAction &&, QList<QUrl> &);
    stub.set_lamda((PublishFunc)&EventDispatcherManager::publish, [this, &in](EventDispatcherManager *,
                   dpf::EventType type, unsigned long long id, ClipBoard::ClipboardAction ac, QList<QUrl> &urls) {
        EXPECT_EQ(type, GlobalEventType::kWriteUrlsToClipboard);
        EXPECT_EQ(id, view->winId());
        EXPECT_EQ(ac, ClipBoard::ClipboardAction::kCopyAction);
        in = urls;
        __DBG_STUB_INVOKE__
        return true;
    });
    fo.copyFiles(view);
    ASSERT_EQ(in.size(), 1);
    EXPECT_EQ(in.first(), one);
}

TEST_F(TestFileOperator, cutFiles)
{
    QUrl one = QUrl::fromLocalFile("/tmp/1");
    stub.set_lamda(&FileOperatorPrivate::getSelectedUrls, [one](){
        return QList<QUrl>{one};
    });

    QList<QUrl> in;
    typedef bool (EventDispatcherManager::*PublishFunc)(dpf::EventType, unsigned long long , ClipBoard::ClipboardAction &&, QList<QUrl> &);
    stub.set_lamda((PublishFunc)&EventDispatcherManager::publish, [this, &in](EventDispatcherManager *,
                   dpf::EventType type, unsigned long long id, ClipBoard::ClipboardAction ac, QList<QUrl> &urls) {
        EXPECT_EQ(type, GlobalEventType::kWriteUrlsToClipboard);
        EXPECT_EQ(id, view->winId());
        EXPECT_EQ(ac, ClipBoard::ClipboardAction::kCutAction);
        in = urls;
        __DBG_STUB_INVOKE__
        return true;
    });
    fo.cutFiles(view);
    ASSERT_EQ(in.size(), 1);
    EXPECT_EQ(in.first(), one);
}

TEST_F(TestFileOperator, openFiles)
{
    QUrl one = QUrl::fromLocalFile("/tmp/1");
    stub.set_lamda(&FileOperatorPrivate::getSelectedUrls, [one](){
        return QList<QUrl>{one};
    });

    QList<QUrl> in;
    typedef bool (EventDispatcherManager::*PublishFunc)(dpf::EventType, unsigned long long , const QList<QUrl> &);
    stub.set_lamda((PublishFunc)&EventDispatcherManager::publish, [this, &in](EventDispatcherManager *,
                   dpf::EventType type, unsigned long long id, const QList<QUrl> &urls) {
        EXPECT_EQ(type, GlobalEventType::kOpenFiles);
        EXPECT_EQ(id, view->winId());
        in = urls;
        __DBG_STUB_INVOKE__
        return true;
    });
    fo.openFiles(view);
    ASSERT_EQ(in.size(), 1);
    EXPECT_EQ(in.first(), one);
}

TEST_F(TestFileOperator, moveToTrash)
{
    QUrl one = QUrl::fromLocalFile("/tmp/1");
    stub.set_lamda(&FileOperatorPrivate::getSelectedUrls, [one](){
        return QList<QUrl>{one};
    });

    QList<QUrl> in;
    typedef bool (EventDispatcherManager::*PublishFunc)(dpf::EventType, unsigned long long , QList<QUrl> &, AbstractJobHandler::JobFlag &&, nullptr_t &&);
    stub.set_lamda((PublishFunc)&EventDispatcherManager::publish, [this, &in](EventDispatcherManager *,
                   dpf::EventType type, unsigned long long id, QList<QUrl> &urls, AbstractJobHandler::JobFlag flag, nullptr_t ) {
        EXPECT_EQ(type, GlobalEventType::kMoveToTrash);
        EXPECT_EQ(id, view->winId());
        EXPECT_EQ(flag, AbstractJobHandler::JobFlag::kNoHint);
        in = urls;
        __DBG_STUB_INVOKE__
        return true;
    });
    fo.moveToTrash(view);
    ASSERT_EQ(in.size(), 1);
    EXPECT_EQ(in.first(), one);
}

TEST_F(TestFileOperator, deleteFiles)
{
    QUrl one = QUrl::fromLocalFile("/tmp/1");
    stub.set_lamda(&FileOperatorPrivate::getSelectedUrls, [one](){
        return QList<QUrl>{one};
    });

    QList<QUrl> in;
    typedef bool (EventDispatcherManager::*PublishFunc)(dpf::EventType, unsigned long long , QList<QUrl> &, AbstractJobHandler::JobFlag &&, nullptr_t &&);
    stub.set_lamda((PublishFunc)&EventDispatcherManager::publish, [this, &in](EventDispatcherManager *,
                   dpf::EventType type, unsigned long long id, QList<QUrl> &urls, AbstractJobHandler::JobFlag flag, nullptr_t ) {
        EXPECT_EQ(type, GlobalEventType::kDeleteFiles);
        EXPECT_EQ(id, view->winId());
        EXPECT_EQ(flag, AbstractJobHandler::JobFlag::kNoHint);
        in = urls;
        __DBG_STUB_INVOKE__
        return true;
    });
    fo.deleteFiles(view);
    ASSERT_EQ(in.size(), 1);
    EXPECT_EQ(in.first(), one);
}

TEST_F(TestFileOperator, undoFiles)
{
    bool call = false;
    typedef bool (EventDispatcherManager::*PublishFunc)(dpf::EventType, unsigned long long , nullptr_t &&);
    stub.set_lamda((PublishFunc)&EventDispatcherManager::publish, [this, &call](EventDispatcherManager *,
                   dpf::EventType type, unsigned long long id, nullptr_t ) {
        EXPECT_EQ(type, GlobalEventType::kRevocation);
        EXPECT_EQ(id, view->winId());
        call = true;
        __DBG_STUB_INVOKE__
        return true;
    });
    fo.undoFiles(view);
    EXPECT_TRUE(call);
}

TEST_F(TestFileOperator, showFilesProperty)
{
    QUrl one = QUrl::fromLocalFile("/tmp/1");
    stub.set_lamda(&FileOperatorPrivate::getSelectedUrls, [one](){
        return QList<QUrl>{one};
    });

    QList<QUrl> in;
    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, QList<QUrl> , QVariantHash &&);
    stub.set_lamda((PushFunc)&EventChannelManager::push, [this, &in](EventChannelManager *,
                   const QString &space, const QString &topic, QList<QUrl> &urls, QVariantHash &) {
        EXPECT_EQ(space, QString("dfmplugin_propertydialog"));
        EXPECT_EQ(topic, QString("slot_PropertyDialog_Show"));
        in = urls;
        __DBG_STUB_INVOKE__
        return QVariant();
    });
    fo.showFilesProperty(view);
    ASSERT_EQ(in.size(), 1);
    EXPECT_EQ(in.first(), one);
}
