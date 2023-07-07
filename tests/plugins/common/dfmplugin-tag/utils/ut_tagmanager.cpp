// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "utils/tagmanager.h"
#include "utils/taghelper.h"
#include "data/tagproxyhandle.h"
#include "utils/filetagcache.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/base/schemefactory.h>
#include "utils/anythingmonitorfilter.h"
#include "files/tagfileinfo.h"

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>
#include <QMenu>
#include "qdbusabstractinterface.h"
#include <QDBusPendingCall>

DFMBASE_USE_NAMESPACE using namespace dfmplugin_tag;

class TagManagerTest : public testing::Test
{

protected:
    virtual void SetUp() override
    {
        ins = TagManager::instance();
        stub.set_lamda(&QDBusAbstractInterface::asyncCallWithArgumentList, []() {
            return QDBusPendingCall::fromError(QDBusError());
        });
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    TagManager *ins;
};

TEST_F(TagManagerTest, canTagFile)
{
    FileInfoPointer info(new FileInfo(QUrl("file:///test")));
    stub.set_lamda(&InfoFactory::create<FileInfo>, [info](const QUrl &url, const Global::CreateFileInfoType type, QString *errorString) {
        return info;
    });
    stub.set_lamda(&TagManager::localFileCanTagFilter, []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_FALSE(ins->canTagFile(QUrl()));
    EXPECT_TRUE(ins->canTagFile(QUrl("file:/hello/world")));
    EXPECT_FALSE(ins->canTagFile(FileInfoPointer()));
    EXPECT_TRUE(ins->canTagFile(info));
}

TEST_F(TagManagerTest, paintListTagsHandle)
{
    DFMGLOBAL_USE_NAMESPACE
    FileInfoPointer info(new FileInfo(QUrl("file:///test")));
    QPainter painter;
    QRectF rect;
    //overload func
    auto func = static_cast<bool (TagManager::*)(const FileInfoPointer &) const>(&TagManager::canTagFile);
    stub.set_lamda(func, []() { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(ins->paintListTagsHandle(1, info, &painter, &rect));
    stub.set_lamda(func, []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_FALSE(ins->paintListTagsHandle(1, info, &painter, &rect));
    EXPECT_FALSE(ins->paintListTagsHandle(kItemFileDisplayNameRole, info, &painter, &rect));
}

TEST_F(TagManagerTest, paintIconTagsHandle2)
{
    FileInfoPointer info(new FileInfo(QUrl("file:///test")));
    auto func = static_cast<bool (TagManager::*)(const FileInfoPointer &) const>(&TagManager::canTagFile);
    stub.set_lamda(func, []() { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(ins->addIconTagsHandle(info, nullptr));
    stub.set_lamda(func, []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_FALSE(ins->addIconTagsHandle(info, nullptr));
}

TEST_F(TagManagerTest, fileDropHandle)
{
    stub.set_lamda(&InfoFactory::create<TagFileInfo>, []() {
        return nullptr;
    });
    EXPECT_TRUE(ins->fileDropHandle(QList<QUrl>() << QUrl("file:///test"), TagManager::rootUrl()));
    auto func = static_cast<bool (TagManager::*)(const FileInfoPointer &) const>(&TagManager::canTagFile);
    stub.set_lamda(func, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagManager::setTagsForFiles, []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(ins->fileDropHandle(QList<QUrl>() << QUrl("file:///test"), TagManager::rootUrl()));
    EXPECT_FALSE(ins->fileDropHandle(QList<QUrl>() << QUrl("file:///test"), QUrl("file:///test")));
}

TEST_F(TagManagerTest, fileDropHandleWithAction)
{
    stub.set_lamda(&TagManager::fileDropHandle, []() { __DBG_STUB_INVOKE__ return true; });
    Qt::DropAction action;
    EXPECT_TRUE(ins->fileDropHandleWithAction(QList<QUrl>() << QUrl("file:///test"), TagManager::rootUrl(), &action));
}

TEST_F(TagManagerTest, sepateTitlebarCrumb)
{
    QList<QVariantMap> map;
    stub.set_lamda(&TagHelper::getTagNameFromUrl, []() { __DBG_STUB_INVOKE__ return QString(); });
    stub.set_lamda(&TagManager::getTagIconName, []() { __DBG_STUB_INVOKE__ return QString(); });
    EXPECT_TRUE(ins->sepateTitlebarCrumb(TagManager::rootUrl(), &map));
    EXPECT_FALSE(ins->sepateTitlebarCrumb(QUrl("file:///test"), &map));
}

TEST_F(TagManagerTest, registerTagColor)
{
    EXPECT_TRUE(ins->registerTagColor(QString("test"), QString("red")));
    EXPECT_FALSE(ins->registerTagColor(QString("test"), QString("red")));
}

TEST_F(TagManagerTest, getTagIconName)
{
    stub.set_lamda(&TagHelper::qureyIconNameByColor, []() { __DBG_STUB_INVOKE__ return QString("case1"); });
    stub.set_lamda(&TagManager::getTagsColorName, []() {
        __DBG_STUB_INVOKE__
        QMap<QString, QString> map;
        map["test"] = "test2";
        return map;
    });

    EXPECT_TRUE(ins->getTagIconName(QString()).isEmpty());
    EXPECT_TRUE(ins->getTagIconName(QString("test1")).isEmpty());
    EXPECT_TRUE(ins->getTagIconName(QString("test")) == QString("case1"));
}

TEST_F(TagManagerTest, getAllTags)
{
    stub.set_lamda(&TagProxyHandle::getAllTags, []() {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["test"] = QColor("red");
        return map;
    });

    EXPECT_FALSE(ins->getAllTags().isEmpty());
}

TEST_F(TagManagerTest, getTagsColor)
{
    stub.set_lamda(&TagProxyHandle::getTagsColor, []() {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["test"] = QColor("red");
        return map;
    });

    EXPECT_FALSE(ins->getTagsColor(QStringList() << QString("test")).isEmpty());
}

TEST_F(TagManagerTest, getTagsByUrls)
{
    EXPECT_TRUE(ins->getTagsByUrls(QList<QUrl>()).isEmpty());
    stub.set_lamda(&FileTagCacheController::getTagsByFiles, []() {
        __DBG_STUB_INVOKE__
        QStringList list;
        list.append("red");
        return list;
    });
    EXPECT_FALSE(ins->getTagsByUrls(QList<QUrl>() << QUrl("file:///test")).isEmpty());
}

TEST_F(TagManagerTest, getFilesByTag)
{
    stub.set_lamda(&TagProxyHandle::getFilesThroughTag, []() { __DBG_STUB_INVOKE__ return QVariantMap(); });

    EXPECT_TRUE(ins->getFilesByTag(QString()).isEmpty());
    EXPECT_TRUE(ins->getFilesByTag(QString("test")).isEmpty());

    stub.set_lamda(&TagProxyHandle::getFilesThroughTag, []() {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["test"] = QString("red");
        return map;
    });

    EXPECT_FALSE(ins->getFilesByTag(QString("test")).isEmpty());
}

TEST_F(TagManagerTest, setTagsForFiles)
{
    stub.set_lamda(&TagManager::getTagsByUrls, []() { __DBG_STUB_INVOKE__ return QStringList(); });
    stub.set_lamda(&TagManager::removeTagsOfFiles, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagManager::addTagsForFiles, []() { __DBG_STUB_INVOKE__ return true; });

    EXPECT_FALSE(ins->setTagsForFiles(QStringList(), QList<QUrl>()));
    EXPECT_TRUE(ins->setTagsForFiles(QStringList() << QString("test"), QList<QUrl>() << QUrl("file:///test")));
}

TEST_F(TagManagerTest, addTagsForFiles)
{
    stub.set_lamda(&TagHelper::qureyColorByDisplayName, []() { __DBG_STUB_INVOKE__ return QColor("red"); });
    stub.set_lamda(&TagProxyHandle::addTags, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagProxyHandle::addTagsForFiles, []() { __DBG_STUB_INVOKE__ return false; });

    EXPECT_FALSE(ins->addTagsForFiles(QStringList(), QList<QUrl>()));
    EXPECT_FALSE(ins->addTagsForFiles(QStringList() << QString("test"), QList<QUrl>() << QUrl("file:///test")));

    stub.set_lamda(&TagProxyHandle::addTagsForFiles, []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(ins->addTagsForFiles(QStringList() << QString("test"), QList<QUrl>() << QUrl("file:///test")));
}

TEST_F(TagManagerTest, removeTagsOfFiles)
{
    EXPECT_FALSE(ins->removeTagsOfFiles(QStringList(), QList<QUrl>()));

    stub.set_lamda(&TagProxyHandle::deleteFileTags, []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(ins->removeTagsOfFiles(QStringList() << QString("test"), QList<QUrl>() << QUrl("file:///test")));
}

TEST_F(TagManagerTest, pasteHandle)
{
    EXPECT_FALSE(ins->pasteHandle(1, QList<QUrl>(), QUrl("file:///test")));
    stub.set_lamda(&TagManager::addTagsForFiles, []() { __DBG_STUB_INVOKE__ return true; });
    auto func = static_cast<bool (TagManager::*)(const FileInfoPointer &) const>(&TagManager::canTagFile);
    stub.set_lamda(func, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&ClipBoard::clipboardAction, []() { __DBG_STUB_INVOKE__ return ClipBoard::kCutAction; });

    EXPECT_TRUE(ins->pasteHandle(1, QList<QUrl>(), QUrl("tag:///test")));

    stub.set_lamda(&ClipBoard::clipboardAction, []() { __DBG_STUB_INVOKE__ return ClipBoard::kCopyAction; });
    EXPECT_TRUE(ins->pasteHandle(1, QList<QUrl>(), QUrl("tag:///test")));
    EXPECT_TRUE(ins->pasteHandle(1, QList<QUrl>(), QUrl("tag:///test")));
}

TEST_F(TagManagerTest, deleteTags)
{
    stub.set_lamda(&TagManager::deleteTagData, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagHelper::makeTagUrlByTagName, []() { __DBG_STUB_INVOKE__ return QUrl(); });
    QObject::connect(ins, &TagManager::tagDeleted, [](QString tag) {
        EXPECT_TRUE(tag == QString("test"));
    });

    ins->deleteTags(QStringList() << QString("test"));
}

TEST_F(TagManagerTest, deleteFiles)
{
    stub.set_lamda(&TagManager::deleteTagData, [](TagManager *, const QStringList &data, const DeleteOpts &) {
        __DBG_STUB_INVOKE__
        if (!data.isEmpty())
            EXPECT_TRUE(data.at(0) == QUrl("tag:///test").toString());
        return true;
    });
    stub.set_lamda(&TagHelper::makeTagUrlByTagName, []() { __DBG_STUB_INVOKE__ return QUrl(); });

    ins->deleteFiles(QList<QUrl>() << QUrl("tag:///test"));
}

TEST_F(TagManagerTest, changeTagColor)
{
    stub.set_lamda(&TagProxyHandle::changeTagsColor, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagHelper::qureyColorByColorName, []() { __DBG_STUB_INVOKE__ return QColor("red"); });

    QObject::connect(ins, &TagManager::tagDeleted, [](QString tag) {
        EXPECT_TRUE(tag == QString("test"));
    });
    ins->changeTagColor(QString("test"), QString("red"));
}

TEST_F(TagManagerTest, changeTagName)
{
    stub.set_lamda(&TagProxyHandle::changeTagNamesWithFiles, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagManager::getAllTags, []() { __DBG_STUB_INVOKE__ return TagManager::TagColorMap(); });

    EXPECT_FALSE(ins->changeTagName(QString(), QString()));
    EXPECT_TRUE(ins->changeTagName(QString("test"), QString("red")));
}

TEST_F(TagManagerTest, getTagsColorName)
{
    stub.set_lamda(&TagProxyHandle::getTagsColor, []() {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["test"] = QString("red");
        return map;
    });

    EXPECT_TRUE(ins->getTagsColorName(QStringList()).isEmpty());
    EXPECT_FALSE(ins->getTagsColorName(QStringList() << QString("test")).isEmpty());
}

TEST_F(TagManagerTest, deleteTagData)
{
    stub.set_lamda(&TagProxyHandle::deleteTags, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagProxyHandle::deleteFiles, []() { __DBG_STUB_INVOKE__ return true; });

    EXPECT_FALSE(ins->deleteTagData(QStringList(), DeleteOpts::kTagOfFiles));
    EXPECT_FALSE(ins->deleteTagData(QStringList() << QString("test"), DeleteOpts::kTagOfFiles));
    EXPECT_TRUE(ins->deleteTagData(QStringList() << QString("test"), DeleteOpts::kTags));
    EXPECT_TRUE(ins->deleteTagData(QStringList() << QString("test"), DeleteOpts::kTags));
}

TEST_F(TagManagerTest, localFileCanTagFilter)
{
    stub.set_lamda(&AnythingMonitorFilter::readSettings, []() { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&AnythingMonitorFilter::whetherFilterCurrentPath, []() { __DBG_STUB_INVOKE__ return true; });
    FileInfoPointer info(new FileInfo(QUrl("file:///test")));

    EXPECT_TRUE(ins->localFileCanTagFilter(info));
}

TEST_F(TagManagerTest, assignColorToTags)
{
    stub.set_lamda(&TagManager::getAllTags, []() { __DBG_STUB_INVOKE__ return TagManager::TagColorMap(); });
    stub.set_lamda(&TagManager::getTagsColor, []() { __DBG_STUB_INVOKE__ return TagManager::TagColorMap(); });
    stub.set_lamda(&TagManager::registerTagColor, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagHelper::qureyColorByColorName, []() { __DBG_STUB_INVOKE__ return QColor("red"); });
    stub.set_lamda(&TagHelper::getColorNameByTag, []() { __DBG_STUB_INVOKE__ return QString("test"); });

    EXPECT_TRUE(ins->assignColorToTags(QStringList() << QString("test")).contains("test"));
}

TEST_F(TagManagerTest, onTagAdded)
{
    stub.set_lamda(&TagHelper::makeTagUrlByTagName, [](TagHelper *, const QString &tagName) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(tagName == QString("test"));
        return QUrl();
    });
    stub.set_lamda(&TagHelper::createSidebarItemInfo, [](TagHelper *, const QString &tagName) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(tagName == QString("test"));
        return QVariantMap();
    });

    QVariantMap map;
    map["test"] = QString("red");
    ins->onTagAdded(map);
}

TEST_F(TagManagerTest, onTagDeleted)
{
    stub.set_lamda(&TagHelper::makeTagUrlByTagName, [](TagHelper *, const QString &tagName) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(tagName == QString("test"));
        return QUrl();
    });
    ins->onTagDeleted(QStringList() << QString("test"));
}

TEST_F(TagManagerTest, onTagColorChanged)
{
    stub.set_lamda(&TagHelper::makeTagUrlByTagName, [](TagHelper *, const QString &tagName) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(tagName == QString("test"));
        return QUrl();
    });
    QVariantMap map;
    map["test"] = QString("red");
    ins->onTagColorChanged(map);
}

TEST_F(TagManagerTest, onFilesTagged)
{
    QVariantMap map;
    map["test"] = QString("red");
    QObject::connect(ins, &TagManager::filesTagged, [&map](QVariantMap fileAndTags) {
        EXPECT_TRUE(fileAndTags == map);
    });
    ins->onFilesTagged(map);
}

TEST_F(TagManagerTest, onFilesUntagged)
{
    QVariantMap map;
    map["test"] = QString("red");
    QObject::connect(ins, &TagManager::filesUntagged, [&map](QVariantMap fileAndTags) {
        EXPECT_TRUE(fileAndTags == map);
    });
    ins->onFilesUntagged(map);
}

TEST_F(TagManagerTest, onTagNameChanged)
{
    QVariantMap map;
    map["test"] = QString("red");
    bool isRun = false;
    stub.set_lamda(&TagHelper::makeTagUrlByTagName, []() {
        __DBG_STUB_INVOKE__
        return QUrl();
    });
    stub.set_lamda(&TagHelper::createSidebarItemInfo, [&isRun]() {
        __DBG_STUB_INVOKE__
        isRun = true;
        return QVariantMap();
    });
    ins->onTagNameChanged(map);
    EXPECT_TRUE(isRun);
}

TEST_F(TagManagerTest, contenxtMenuHandle)
{
    QPoint q;
    bool isCall = false;
    stub.set_lamda((QAction * (QMenu::*)(const QPoint &, QAction *)) ADDR(QMenu, exec), [&]() {
        isCall = true;
        return nullptr;
    });
    TagManager::contenxtMenuHandle(1, QUrl(), q);
    EXPECT_TRUE(isCall);
}

TEST_F(TagManagerTest, renameHandle)
{
    bool isCall = false;
    stub.set_lamda(&TagManager::changeTagName, [&isCall]() {
        isCall = true;
        return true;
    });
    TagManager::renameHandle(1, QUrl(), "111");
    EXPECT_TRUE(isCall);
}

TEST_F(TagManagerTest, hideFiles)
{
    bool isCall = false;
    QObject::connect(ins, &TagManager::filesHidden, [&isCall]() {
        isCall = true;
    });
    ins->hideFiles(QList<QString>() << "red", QList<QUrl>() << QUrl("/test"));
    EXPECT_TRUE(isCall);
}
