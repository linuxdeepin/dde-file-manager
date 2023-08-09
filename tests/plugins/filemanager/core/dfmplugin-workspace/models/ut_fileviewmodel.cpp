// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-workspace/views/fileview.h"
#include "plugins/filemanager/core/dfmplugin-workspace/models/fileviewmodel.h"
#include "plugins/filemanager/core/dfmplugin-workspace/utils/workspacehelper.h"
#include "plugins/filemanager/core/dfmplugin-workspace/utils/filesortworker.h"
#include "plugins/filemanager/core/dfmplugin-workspace/utils/filedatamanager.h"
#include "plugins/filemanager/core/dfmplugin-workspace/events/workspaceeventsequence.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/fileutils.h>

#include <gtest/gtest.h>

#include <QStandardPaths>
#include <QApplication>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

class UT_FileViewModel : public testing::Test
{
protected:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false,
                            QObject::tr("System Disk"));

        InfoFactory::regClass<dfmbase::SyncFileInfo>(Global::Scheme::kFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

        QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
        url.setScheme(Scheme::kFile);
        view = new FileView(url);
        model = new FileViewModel(view);
    }

    void TearDown() override
    {
        stub.clear();

        delete model;
        model = nullptr;

        delete view;
        view = nullptr;
    }

private:
    stub_ext::StubExt stub;

    FileView* view;
    FileViewModel* model;
};

TEST_F(UT_FileViewModel, SetRootUrl) {
    auto index = model->setRootUrl(QUrl());
    EXPECT_FALSE(index.isValid());

    bool beginReset { false };
    bool endReset { false };
    stub.set_lamda(ADDR(FileViewModel, beginResetModel), [&beginReset]{ beginReset = true; });
    stub.set_lamda(ADDR(FileViewModel, endResetModel), [&endReset]{ endReset = true; });

    bool initWork { false };
    stub.set_lamda(ADDR(FileViewModel, initFilterSortWork), [&initWork]{ initWork = true; });

    QModelIndex rootIndex = QModelIndex(1, 1, nullptr, model);
    stub.set_lamda(ADDR(FileViewModel, rootIndex), [&rootIndex]{ return rootIndex;});

    QModelIndex fetchIndex = QModelIndex();
    stub.set_lamda(VADDR(FileViewModel, fetchMore), [&fetchIndex](FileViewModel*, const QModelIndex &index) {
        fetchIndex = index;
    });

    bool havePrehandler = false;
    stub.set_lamda(ADDR(WorkspaceHelper, haveViewRoutePrehandler), [&havePrehandler]{ return havePrehandler; });

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);

    QModelIndex resultIndex = model->setRootUrl(url);
    EXPECT_TRUE(beginReset);
    EXPECT_TRUE(endReset);
    EXPECT_TRUE(initWork);
    EXPECT_EQ(model->dirRootUrl, url);
    EXPECT_TRUE(model->canFetchFiles);
    EXPECT_EQ(fetchIndex, rootIndex);
    EXPECT_EQ(resultIndex, rootIndex);
}

TEST_F(UT_FileViewModel, InitFilterSortWork) {
    bool destroyOld = false;
    stub.set_lamda(ADDR(FileViewModel, discardFilterSortObjects), [&destroyOld]{ destroyOld = true; });

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);
    QString key("key");

    model->dirRootUrl = url;
    model->currentKey = key;

    bool beginInsert = false;
    bool endInsert = false;
    stub.set_lamda(ADDR(FileViewModel, beginInsertRows), [&beginInsert]{ beginInsert = true; });
    stub.set_lamda(ADDR(FileViewModel, endInsertRows), [&endInsert]{ endInsert = true; });

    bool threadStart = false;
    stub.set_lamda(ADDR(QThread, start), [&threadStart]{ threadStart = true;});

    model->initFilterSortWork();

    EXPECT_TRUE(beginInsert);
    EXPECT_TRUE(endInsert);
    EXPECT_TRUE(threadStart);
    EXPECT_FALSE(model->filterSortWorker.isNull());
    if (!model->filterSortWorker.isNull()) {
        EXPECT_EQ(model->filterSortWorker->current, url);
        EXPECT_EQ(model->filterSortWorker->currentKey, key);
    }
}

TEST_F(UT_FileViewModel, RootIndex) {
    QModelIndex index = model->rootIndex();
    EXPECT_FALSE(index.isValid());

    model->initFilterSortWork();
    index = model->rootIndex();
    EXPECT_TRUE(index.isValid());
}

TEST_F(UT_FileViewModel, RootUrl) {
    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);

    model->dirRootUrl = url;
    EXPECT_EQ(url, model->rootUrl());
}

TEST_F(UT_FileViewModel, Refresh) {
    bool cleanRootCalled = false;
    stub.set_lamda((void(FileDataManager::*)(const QUrl &, const QString &, const bool))ADDR(FileDataManager, cleanRoot), [&cleanRootCalled]{ cleanRootCalled = true;});

    model->refresh();
    EXPECT_TRUE(cleanRootCalled);
}

TEST_F(UT_FileViewModel, CurrentState) {
    ModelState testState = ModelState::kIdle;
    model->state = testState;

    EXPECT_EQ(testState, model->currentState());
}

TEST_F(UT_FileViewModel, FileInfo) {
    QModelIndex index;
    auto infoPtr = model->fileInfo(index);
    EXPECT_TRUE(infoPtr.isNull());

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);

    index = model->setRootUrl(url);
    model->initFilterSortWork();

    infoPtr = model->fileInfo(index);
    EXPECT_FALSE(infoPtr.isNull());
}

TEST_F(UT_FileViewModel, GetChildrenUrls) {
    bool getChildrenUrlsCalled { false };
    stub.set_lamda(ADDR(FileSortWorker, getChildrenUrls), [&getChildrenUrlsCalled]{
        getChildrenUrlsCalled = true;
        return QList<QUrl>();
    });

    auto urls = model->getChildrenUrls();
    EXPECT_TRUE(urls.isEmpty());
    EXPECT_FALSE(getChildrenUrlsCalled);

    model->initFilterSortWork();
    model->getChildrenUrls();
    EXPECT_TRUE(getChildrenUrlsCalled);
}

TEST_F(UT_FileViewModel, GetIndexByUrl) {
    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);

    QModelIndex index = model->getIndexByUrl(url);
    EXPECT_FALSE(index.isValid());

    model->initFilterSortWork();

    stub.set_lamda(ADDR(FileSortWorker, getChildShowIndex), [&url](FileSortWorker *, const QUrl &itemUrl){
        if (url == itemUrl)
            return 0;
        return -1;
    });

    index = model->getIndexByUrl(url);
    EXPECT_EQ(index, model->index(0, 0, model->rootIndex()));

    index = model->getIndexByUrl(QUrl());
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_FileViewModel, GetColumnWidth) {
    stub.set_lamda(ADDR(FileViewModel, getRoleByColumn), []{ return ItemRoles::kItemUnknowRole; });
    int width = model->getColumnWidth(0);
    EXPECT_EQ(width, 120);

    stub.clear();
    stub.set_lamda(ADDR(FileViewModel, getRoleByColumn), []{ return ItemRoles::kItemFileDisplayNameRole; });
    width = model->getColumnWidth(0);

    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "ViewColumnState").toMap();
    int colWidth = state.value(QString::number(ItemRoles::kItemFileDisplayNameRole), -1).toInt();
    if (colWidth > 0) {
        EXPECT_EQ(width, colWidth);
    } else {
        EXPECT_EQ(width, 120);
    }
}

TEST_F(UT_FileViewModel, GetRoleByColumn) {
    QList<ItemRoles> roleList = QList<ItemRoles>() << kItemFileDisplayNameRole
                                                   << kItemFileLastModifiedRole
                                                   << kItemFileSizeRole
                                                   << kItemFileMimeTypeRole;
    stub.set_lamda(ADDR(FileViewModel, getColumnRoles), [&roleList]{ return roleList; });

    ItemRoles role = model->getRoleByColumn(1);
    EXPECT_EQ(role, kItemFileLastModifiedRole);

    role = model->getRoleByColumn(roleList.length());
    EXPECT_EQ(role, kItemFileDisplayNameRole);
}

TEST_F(UT_FileViewModel, GetColumnRoles) {
    bool fetchResult = false;
    stub.set_lamda(ADDR(WorkspaceEventSequence, doFetchCustomColumnRoles), [&fetchResult]{ return fetchResult; });
    stub.set_lamda((QVariant(Settings::*)(const QString &, const QUrl &, const QVariant &) const)ADDR(Settings, value), []{ return QVariant(); });

    QList<ItemRoles> list = model->getColumnRoles();
    EXPECT_FALSE(list.isEmpty());

    fetchResult = true;
    list = model->getColumnRoles();
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(UT_FileViewModel, RoleDisplayString) {
    stub.set_lamda(ADDR(WorkspaceEventSequence, doFetchCustomRoleDiaplayName), []{ return false; });

    QString displayStr;
    displayStr = model->roleDisplayString(kItemFileDisplayNameRole);
    EXPECT_EQ(displayStr, FileViewModel::tr("Name"));
    displayStr = model->roleDisplayString(kItemFileLastModifiedRole);
    EXPECT_EQ(displayStr, FileViewModel::tr("Time modified"));
    displayStr = model->roleDisplayString(kItemFileSizeRole);
    EXPECT_EQ(displayStr, FileViewModel::tr("Size"));
    displayStr = model->roleDisplayString(kItemFileMimeTypeRole);
    EXPECT_EQ(displayStr, FileViewModel::tr("Type"));
    displayStr = model->roleDisplayString(kItemUnknowRole);
    EXPECT_TRUE(displayStr.isEmpty());
}

TEST_F(UT_FileViewModel, SortOrder) {
    Qt::SortOrder order = model->sortOrder();
    EXPECT_EQ(order, Qt::AscendingOrder);

    model->initFilterSortWork();
    if (!model->filterSortWorker.isNull())
        model->filterSortWorker->sortOrder = Qt::DescendingOrder;

    order = model->sortOrder();
    EXPECT_EQ(order, Qt::DescendingOrder);
}

TEST_F(UT_FileViewModel, SortRole) {
    ItemRoles role = model->sortRole();
    EXPECT_EQ(role, kItemFileDisplayNameRole);

    model->initFilterSortWork();
    if (!model->filterSortWorker.isNull())
        model->filterSortWorker->orgSortRole = kItemFileSizeRole;

    role = model->sortRole();
    EXPECT_EQ(role, kItemFileSizeRole);
}

TEST_F(UT_FileViewModel, SetFilters) {
    QDir::Filters targetFilters;
    QDir::Filters setFilters = QDir::Filter::Dirs | QDir::Filter::Files;
    QObject::connect(model, &FileViewModel::requestChangeFilters, model, [&targetFilters](QDir::Filters filters){
        targetFilters = filters;
    });

    model->setFilters(setFilters);
    EXPECT_EQ(targetFilters, setFilters);
}

TEST_F(UT_FileViewModel, GetFilters) {
    QDir::Filters filters;
    filters = model->getFilters();
    EXPECT_EQ(filters, QDir::NoFilter);

    model->initFilterSortWork();
    model->filterSortWorker->filters = QDir::Filter::Dirs | QDir::Filter::Files;
    filters = model->getFilters();
    EXPECT_EQ(filters, QDir::Filter::Dirs | QDir::Filter::Files);
}

TEST_F(UT_FileViewModel, SetNameFilters) {
    QStringList filters{ "filter" };
    QStringList targetFilters{};

    QObject::connect(model, &FileViewModel::requestChangeNameFilters, model, [&targetFilters] (const QStringList &filters){
        targetFilters = filters;
    });

    model->setNameFilters(filters);
    EXPECT_EQ(model->getNameFilters(), filters);
    EXPECT_EQ(targetFilters, filters);

    targetFilters.clear();
    model->setNameFilters(filters);
    EXPECT_TRUE(targetFilters.isEmpty());
}

TEST_F(UT_FileViewModel, SetFilterData) {
    QVariant filterData = QVariant("filterData");
    QVariant recData;

    QObject::connect(model, &FileViewModel::requestSetFilterData, model, [&recData] (const QVariant &data){
        recData = data;
    });

    model->setFilterData(filterData);
    EXPECT_EQ(model->filterData, filterData);
    EXPECT_EQ(recData, filterData);
}

TEST_F(UT_FileViewModel, ToggleHiddenFiles) {
    bool emitSignal = false;
    QObject::connect(model, &FileViewModel::requestChangeHiddenFilter, model, [&emitSignal]{
        emitSignal = true;
    });

    model->toggleHiddenFiles();
    EXPECT_TRUE(emitSignal);
}

TEST_F(UT_FileViewModel, SetReadOnly) {
    model->setReadOnly(true);
    EXPECT_TRUE(model->readOnly);

    model->setReadOnly(false);
    EXPECT_FALSE(model->readOnly);
}

TEST_F(UT_FileViewModel, UpdateThumbnailIcon) {
    QVariant thumbnailValue;
    stub.set_lamda(VADDR(FileInfo, setExtendedAttributes), [&thumbnailValue](FileInfo*, const ExtInfoType &key, const QVariant &value){
        if (key == ExtInfoType::kFileThumbnail)
            thumbnailValue = value;
    });

    model->updateThumbnailIcon(QModelIndex(), "");
    EXPECT_FALSE(thumbnailValue.isValid());

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);

    QModelIndex index = model->setRootUrl(url);
    model->initFilterSortWork();

    model->updateThumbnailIcon(index, QIcon::fromTheme("empty").name());
    EXPECT_TRUE(thumbnailValue.isValid());
}

TEST_F(UT_FileViewModel, OnFileThumbUpdated) {
    QModelIndex updateIndex;
    stub.set_lamda((void(FileView::*)(const QModelIndex &))ADDR(FileView, update), [&updateIndex](FileView *, const QModelIndex &index){
        updateIndex = index;
    });

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);

    model->onFileThumbUpdated(url, "");
    EXPECT_FALSE(updateIndex.isValid());

    model->setRootUrl(url);
    model->initFilterSortWork();

    QModelIndex validIndex = model->index(0, 0, model->rootIndex());
    stub.set_lamda(ADDR(FileViewModel, getIndexByUrl), [&validIndex]{
        return validIndex;
    });

    model->onFileThumbUpdated(url, "");
    EXPECT_EQ(updateIndex, validIndex);
}

TEST_F(UT_FileViewModel, OnFileUpdated) {
    bool calledUpdate = false;
    stub.set_lamda((void(FileView::*)(const QModelIndex &))ADDR(FileView, update), [&calledUpdate]{
        calledUpdate = true;
    });

    model->onFileUpdated(0);
    EXPECT_TRUE(calledUpdate);
}

TEST_F(UT_FileViewModel, OnInsert) {
    int firstIndex = 1;
    int count = 1;

    int recFirst = 0;
    int recLast = 0;
    stub.set_lamda(ADDR(QAbstractItemModel, beginInsertRows), [&recFirst, &recLast](QAbstractItemModel *, const QModelIndex &, int first, int last){
        recFirst = first;
        recLast = last;
    });

    model->onInsert(firstIndex, count);
    EXPECT_EQ(recFirst, firstIndex);
    EXPECT_EQ(recLast, firstIndex + count - 1);
}

TEST_F(UT_FileViewModel, OnInsertFinish) {
    bool calledInsertEnd = false;
    stub.set_lamda(ADDR(QAbstractItemModel, endInsertRows), [&calledInsertEnd]{
        calledInsertEnd = true;
    });

    model->onInsertFinish();
    EXPECT_TRUE(calledInsertEnd);
}

TEST_F(UT_FileViewModel, OnRemove) {
    int firstIndex = 1;
    int count = 1;

    int recFirst = 0;
    int recLast = 0;
    stub.set_lamda(ADDR(QAbstractItemModel, beginRemoveRows), [&recFirst, &recLast](QAbstractItemModel *, const QModelIndex &, int first, int last){
        recFirst = first;
        recLast = last;
    });

    model->onRemove(firstIndex, count);
    EXPECT_EQ(recFirst, firstIndex);
    EXPECT_EQ(recLast, firstIndex + count - 1);
}

TEST_F(UT_FileViewModel, OnRemoveFinish) {
    bool calledInsertEnd = false;
    stub.set_lamda(ADDR(QAbstractItemModel, endInsertRows), [&calledInsertEnd]{
        calledInsertEnd = true;
    });

    model->onInsertFinish();
    EXPECT_TRUE(calledInsertEnd);
}

TEST_F(UT_FileViewModel, OnUpdateView) {
    bool calledUpdate = false;
    stub.set_lamda(ADDR(FileViewModel, onUpdateView), [&calledUpdate]{
        calledUpdate = true;
    });

    model->onUpdateView();
    EXPECT_TRUE(calledUpdate);
}

TEST_F(UT_FileViewModel, OnGenericAttributeChanged) {
    bool calledClearThumbnail = false;
    QObject::connect(model, &FileViewModel::requestClearThumbnail, model, [&calledClearThumbnail]{
        calledClearThumbnail = true;
    });

    model->onGenericAttributeChanged(Application::kPreviewAudio, QVariant());
    EXPECT_TRUE(calledClearThumbnail);

    calledClearThumbnail = false;
    model->onGenericAttributeChanged(Application::kPreviewImage, QVariant());
    EXPECT_TRUE(calledClearThumbnail);

    calledClearThumbnail = false;
    model->onGenericAttributeChanged(Application::kPreviewVideo, QVariant());
    EXPECT_TRUE(calledClearThumbnail);

    calledClearThumbnail = false;
    model->onGenericAttributeChanged(Application::kPreviewTextFile, QVariant());
    EXPECT_TRUE(calledClearThumbnail);

    calledClearThumbnail = false;
    model->onGenericAttributeChanged(Application::kPreviewDocumentFile, QVariant());
    EXPECT_TRUE(calledClearThumbnail);

    bool isGvfsFile = false;
    stub.set_lamda(ADDR(FileUtils, isGvfsFile), [&isGvfsFile]{
        return isGvfsFile;
    });

    calledClearThumbnail = false;
    model->onGenericAttributeChanged(Application::kShowThunmbnailInRemote, QVariant());
    EXPECT_FALSE(calledClearThumbnail);

    isGvfsFile = true;
    model->onGenericAttributeChanged(Application::kShowThunmbnailInRemote, QVariant());
    EXPECT_TRUE(calledClearThumbnail);
}

TEST_F(UT_FileViewModel, OnDConfigChanged) {
    bool calledClearThumbnail = false;
    QObject::connect(model, &FileViewModel::requestClearThumbnail, model, [&calledClearThumbnail]{
        calledClearThumbnail = true;
    });

    bool isMtpFile = false;
    stub.set_lamda(ADDR(FileUtils, isMtpFile), [&isMtpFile]{
        return isMtpFile;
    });

    model->onDConfigChanged("", "");
    EXPECT_FALSE(calledClearThumbnail);

    model->onDConfigChanged(DConfigInfo::kConfName, "");
    EXPECT_FALSE(calledClearThumbnail);

    model->onDConfigChanged(DConfigInfo::kConfName, DConfigInfo::kMtpThumbnailKey);
    EXPECT_FALSE(calledClearThumbnail);

    isMtpFile = true;
    model->onDConfigChanged(DConfigInfo::kConfName, DConfigInfo::kMtpThumbnailKey);
    EXPECT_TRUE(calledClearThumbnail);
}

TEST_F(UT_FileViewModel, OnSetCursorWait) {
    QCursor cursor;
    stub.set_lamda(ADDR(QApplication, setOverrideCursor), [&cursor](const QCursor &newCursor){
        cursor = newCursor;
    });

    model->changeState(ModelState::kIdle);
    model->onSetCursorWait();
    EXPECT_NE(cursor.shape(), Qt::WaitCursor);

    model->changeState(ModelState::kBusy);
    model->onSetCursorWait();
    EXPECT_EQ(cursor.shape(), Qt::WaitCursor);
}

TEST_F(UT_FileViewModel, QuitFilterSortWork) {
    bool calledCancel = false;
    bool calledQuit = false;
    bool calledWait = false;
    stub.set_lamda(ADDR(FileSortWorker, cancel), [&calledCancel]{
        calledCancel = true;
    });
    stub.set_lamda(ADDR(QThread, quit), [&calledQuit]{
        calledQuit = true;
    });
    stub.set_lamda(ADDR(QThread, wait), [&calledWait]{
        calledWait = true;
        return true;
    });
    stub.set_lamda(ADDR(QThread, start), []{});

    model->quitFilterSortWork();
    EXPECT_FALSE(calledCancel);
    EXPECT_FALSE(calledQuit);
    EXPECT_FALSE(calledWait);

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);

    model->setRootUrl(url);
    model->initFilterSortWork();

    model->quitFilterSortWork();
    EXPECT_TRUE(calledCancel);
    EXPECT_TRUE(calledQuit);
    EXPECT_TRUE(calledWait);
}

TEST_F(UT_FileViewModel, DiscardFilterSortObjects) {
    bool calledCancel = false;
    bool calledQuit = false;
    stub.set_lamda(ADDR(FileSortWorker, cancel), [&calledCancel]{
        calledCancel = true;
    });
    stub.set_lamda(ADDR(QThread, quit), [&calledQuit]{
        calledQuit = true;
    });
    stub.set_lamda(ADDR(QThread, start), []{});

    model->discardFilterSortObjects();
    EXPECT_FALSE(calledCancel);
    EXPECT_FALSE(calledQuit);

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);

    model->setRootUrl(url);
    model->initFilterSortWork();

    model->discardFilterSortObjects();
    EXPECT_TRUE(calledCancel);
    EXPECT_TRUE(calledQuit);
    EXPECT_FALSE(model->discardedObjects.isEmpty());
}

TEST_F(UT_FileViewModel, ChangeState) {
    bool sendStateChanged = false;
    QObject::connect(model, &FileViewModel::stateChanged, model, [&sendStateChanged]{
        sendStateChanged = true;
    });

    model->state = ModelState::kIdle;
    model->changeState(ModelState::kIdle);
    EXPECT_EQ(model->state, ModelState::kIdle);
    EXPECT_FALSE(sendStateChanged);

    model->changeState(ModelState::kBusy);
    EXPECT_EQ(model->state, ModelState::kBusy);
    EXPECT_TRUE(sendStateChanged);
}
