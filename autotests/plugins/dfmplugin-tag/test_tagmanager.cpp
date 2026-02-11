// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/utils/tagmanager.h"
#include "plugins/common/dfmplugin-tag/utils/taghelper.h"
#include "plugins/common/dfmplugin-tag/utils/filetagcache.h"
#include "plugins/common/dfmplugin-tag/data/tagproxyhandle.h"
#include "plugins/common/dfmplugin-tag/files/tagfileinfo.h"
#include "plugins/common/dfmplugin-tag/utils/anythingmonitorfilter.h"
#include "plugins/common/dfmplugin-tag/events/tageventcaller.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/file/local/desktopfileinfo.h>

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

#include <QUrl>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QMenu>
#include <QDBusVariant>

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

class UT_TagManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ins = TagManager::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TagManager *ins = nullptr;
};

TEST_F(UT_TagManager, instance)
{
    // Test singleton instance
    TagManager *instance1 = TagManager::instance();
    TagManager *instance2 = TagManager::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_TagManager, scheme)
{
    // Test scheme method
    EXPECT_EQ(TagManager::scheme(), "tag");
}

TEST_F(UT_TagManager, rootUrl)
{
    // Test rootUrl static method
    QUrl root = TagManager::rootUrl();

    EXPECT_EQ(root.scheme(), "tag");
    EXPECT_EQ(root.path(), "/");
}

TEST_F(UT_TagManager, canTagFile)
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

TEST_F(UT_TagManager, registerTagColor_NewTag)
{
    // Test registering new tag color
    QString tagName = "NewTestTag";
    QString color = "#123456";

    bool result = ins->registerTagColor(tagName, color);

    EXPECT_TRUE(result);
}

TEST_F(UT_TagManager, registerTagColor_ExistingTag)
{
    // Test registering existing tag color
    QString tagName = "ExistingTag";
    QString color1 = "#123456";
    QString color2 = "#654321";

    ins->registerTagColor(tagName, color1);
    bool result = ins->registerTagColor(tagName, color2);

    EXPECT_FALSE(result);
}

TEST_F(UT_TagManager, getTagIconName_EmptyTag)
{
    // Test with empty tag name
    QString iconName = ins->getTagIconName("");

    EXPECT_TRUE(iconName.isEmpty());
}

TEST_F(UT_TagManager, getTagIconName_ValidTag)
{
    // Test with valid tag name
    stub.set_lamda(&TagProxyHandle::getTagsColor, [](TagProxyHandle *, const QStringList &) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["TestTag"] = QVariant("#ffa503");
        return map;
    });

    stub.set_lamda(&TagHelper::qureyIconNameByColor, [](const TagHelper *, const QColor &) -> QString {
        __DBG_STUB_INVOKE__
        return "dfm_tag_orange";
    });

    QString iconName = ins->getTagIconName("TestTag");

    EXPECT_EQ(iconName, "dfm_tag_orange");
}

TEST_F(UT_TagManager, getAllTags)
{
    // Test getting all tags
    stub.set_lamda(&TagProxyHandle::getAllTags, [](TagProxyHandle *) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["Tag1"] = QVariant(QColor("#ffa503"));
        map["Tag2"] = QVariant(QColor("#ff1c49"));
        return map;
    });

    TagManager::TagColorMap tags = ins->getAllTags();

    EXPECT_EQ(tags.size(), 2);
    EXPECT_TRUE(tags.contains("Tag1"));
    EXPECT_TRUE(tags.contains("Tag2"));
}

TEST_F(UT_TagManager, getTagsColor_Empty)
{
    // Test with empty tag list
    QStringList tags;
    TagManager::TagColorMap result = ins->getTagsColor(tags);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_TagManager, getTagsColor_ValidTags)
{
    // Test with valid tags
    QStringList tags;
    tags << "Tag1"
         << "Tag2";

    stub.set_lamda(&TagProxyHandle::getTagsColor, [](TagProxyHandle *, const QStringList &) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["Tag1"] = QVariant("#ffa503");
        map["Tag2"] = QVariant("#ff1c49");
        return map;
    });

    TagManager::TagColorMap result = ins->getTagsColor(tags);

    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.contains("Tag1"));
    EXPECT_EQ(result["Tag1"].name().toLower(), "#ffa503");
}

TEST_F(UT_TagManager, getTagsByUrls_Empty)
{
    // Test with empty URL list
    QList<QUrl> urls;
    QStringList tags = ins->getTagsByUrls(urls);

    EXPECT_TRUE(tags.isEmpty());
}

TEST_F(UT_TagManager, getTagsByUrls_ValidUrls)
{
    // Test with valid URLs
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/home/user/test.txt");

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [&urls](const QList<QUrl> &, QList<QUrl> *realUrls) -> bool {
        __DBG_STUB_INVOKE__
        *realUrls = urls;
        return true;
    });

    stub.set_lamda(&FileUtils::bindUrlTransform, [](const QUrl &url) -> QUrl {
        __DBG_STUB_INVOKE__
        return url;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) -> bool {
        __DBG_STUB_INVOKE__
        return url1 == url2;
    });

    stub.set_lamda(&FileTagCacheController::getTagsByFiles, [](FileTagCacheController *, const QStringList &) -> QStringList {
        __DBG_STUB_INVOKE__
        QStringList tags;
        tags << "Tag1"
             << "Tag2";
        return tags;
    });

    QStringList tags = ins->getTagsByUrls(urls);

    EXPECT_EQ(tags.size(), 2);
}

TEST_F(UT_TagManager, getFilesByTag_EmptyTag)
{
    // Test with empty tag
    QStringList files = ins->getFilesByTag("");

    EXPECT_TRUE(files.isEmpty());
}

TEST_F(UT_TagManager, getFilesByTag_ValidTag)
{
    // Test with valid tag
    stub.set_lamda(&TagProxyHandle::getFilesThroughTag, [](TagProxyHandle *, const QStringList &) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        QStringList files;
        files << "/home/user/file1.txt"
              << "/home/user/file2.txt";
        map["TestTag"] = QVariant(files);
        return map;
    });

    QStringList files = ins->getFilesByTag("TestTag");

    EXPECT_EQ(files.size(), 2);
}

TEST_F(UT_TagManager, setTagsForFiles_EmptyFiles)
{
    // Test with empty file list
    QStringList tags;
    tags << "Tag1";
    QList<QUrl> files;

    bool result = ins->setTagsForFiles(tags, files);

    EXPECT_FALSE(result);
}

TEST_F(UT_TagManager, addTagsForFiles)
{
    stub.set_lamda(&TagHelper::qureyColorByDisplayName, []() { __DBG_STUB_INVOKE__ return QColor("red"); });
    stub.set_lamda(&TagProxyHandle::addTags, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagProxyHandle::addTagsForFiles, []() { __DBG_STUB_INVOKE__ return false; });

    EXPECT_FALSE(ins->addTagsForFiles(QStringList(), QList<QUrl>()));
    EXPECT_FALSE(ins->addTagsForFiles(QStringList() << QString("test"), QList<QUrl>() << QUrl("file:///test")));

    stub.set_lamda(&TagProxyHandle::addTagsForFiles, []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(ins->addTagsForFiles(QStringList() << QString("test"), QList<QUrl>() << QUrl("file:///test")));
}

TEST_F(UT_TagManager, removeTagsOfFiles_EmptyTags)
{
    // Test with empty tag list
    QList<QString> tags;
    QList<QUrl> files;
    files << QUrl::fromLocalFile("/home/user/test.txt");

    bool result = ins->removeTagsOfFiles(tags, files);

    EXPECT_FALSE(result);
}

TEST_F(UT_TagManager, removeTagsOfFiles_ValidData)
{
    // Test with valid tags and files
    QList<QString> tags;
    tags << "Tag1";
    QList<QUrl> files;
    files << QUrl::fromLocalFile("/home/user/test.txt");

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [&files](const QList<QUrl> &, QList<QUrl> *realUrls) -> bool {
        __DBG_STUB_INVOKE__
        *realUrls = files;
        return true;
    });

    stub.set_lamda(&FileUtils::bindUrlTransform, [](const QUrl &url) -> QUrl {
        __DBG_STUB_INVOKE__
        return url;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) -> bool {
        __DBG_STUB_INVOKE__
        return url1 == url2;
    });

    stub.set_lamda(&TagProxyHandle::deleteFileTags, [](TagProxyHandle *, const QMap<QString, QVariant> &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = ins->removeTagsOfFiles(tags, files);

    EXPECT_TRUE(result);
}

TEST_F(UT_TagManager, pasteHandle_TagScheme_CutAction)
{
    // Test paste with cut action - should return true without processing
    QUrl tagUrl;
    tagUrl.setScheme("tag");
    tagUrl.setPath("/TestTag");

    stub.set_lamda(&ClipBoard::instance, []() -> ClipBoard * {
        __DBG_STUB_INVOKE__
        static ClipBoard clipboard;
        return &clipboard;
    });

    stub.set_lamda(&ClipBoard::clipboardAction, [](ClipBoard *) -> ClipBoard::ClipboardAction {
        __DBG_STUB_INVOKE__
        return ClipBoard::kCutAction;
    });

    bool result = ins->pasteHandle(0, QList<QUrl>(), tagUrl);

    EXPECT_TRUE(result);
}

TEST_F(UT_TagManager, pasteHandle)
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

TEST_F(UT_TagManager, fileDropHandle)
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

TEST_F(UT_TagManager, fileDropHandleWithAction)
{
    // Test file drop with action
    QUrl tagUrl;
    tagUrl.setScheme("tag");
    tagUrl.setPath("/TestTag");

    QList<QUrl> fromUrls;
    Qt::DropAction action = Qt::CopyAction;

    stub.set_lamda(&TagManager::fileDropHandle, [](TagManager *, const QList<QUrl> &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = ins->fileDropHandleWithAction(fromUrls, tagUrl, &action);

    EXPECT_TRUE(result);
    EXPECT_EQ(action, Qt::IgnoreAction);
}

TEST_F(UT_TagManager, sepateTitlebarCrumb_TagUrl)
{
    // Test separate titlebar crumb with tag URL
    QUrl tagUrl;
    tagUrl.setScheme("tag");
    tagUrl.setPath("/TestTag");

    QList<QVariantMap> mapGroup;

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper *, const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    stub.set_lamda(&TagManager::getTagIconName, [](const TagManager *, const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return "dfm_tag_orange";
    });

    bool result = ins->sepateTitlebarCrumb(tagUrl, &mapGroup);

    EXPECT_TRUE(result);
    EXPECT_EQ(mapGroup.size(), 1);
}

TEST_F(UT_TagManager, sepateTitlebarCrumb_NonTagUrl)
{
    // Test separate titlebar crumb with non-tag URL
    QUrl fileUrl = QUrl::fromLocalFile("/home/user/test.txt");
    QList<QVariantMap> mapGroup;

    bool result = ins->sepateTitlebarCrumb(fileUrl, &mapGroup);

    EXPECT_FALSE(result);
    EXPECT_TRUE(mapGroup.isEmpty());
}

TEST_F(UT_TagManager, deleteTags)
{
    stub.set_lamda(&TagManager::deleteTagData, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagHelper::makeTagUrlByTagName, []() { __DBG_STUB_INVOKE__ return QUrl(); });

    EXPECT_NO_THROW(ins->deleteTags(QStringList() << QString("test")));
}

TEST_F(UT_TagManager, changeTagColor_EmptyTag)
{
    // Test changing color with empty tag name
    bool result = ins->changeTagColor("", "#123456");

    EXPECT_FALSE(result);
}

TEST_F(UT_TagManager, changeTagColor_ValidTag)
{
    // Test changing color with valid tag
    stub.set_lamda(&TagHelper::qureyColorByColorName, [](const TagHelper *, const QString &) -> QColor {
        __DBG_STUB_INVOKE__
        return QColor("#ffa503");
    });

    stub.set_lamda(&TagProxyHandle::changeTagsColor, [](TagProxyHandle *, const QVariantMap &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = ins->changeTagColor("TestTag", "Orange");
    EXPECT_TRUE(result);
}

TEST_F(UT_TagManager, changeTagName)
{
    stub.set_lamda(&TagProxyHandle::changeTagNamesWithFiles, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagManager::getAllTags, []() { __DBG_STUB_INVOKE__ return TagManager::TagColorMap(); });
    stub.set_lamda(&TagProxyHandle::changeTagNamesWithFiles, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_FALSE(ins->changeTagName(QString(), QString()));
    EXPECT_TRUE(ins->changeTagName(QString("test"), QString("red")));
}

TEST_F(UT_TagManager, changeTagName_ValidTag)
{
    // Test changing tag name
    stub.set_lamda(&TagManager::getAllTags, [](TagManager *) -> TagManager::TagColorMap {
        __DBG_STUB_INVOKE__
        return TagManager::TagColorMap();
    });

    stub.set_lamda(&TagProxyHandle::changeTagNamesWithFiles, [](TagProxyHandle *, const QVariantMap &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = ins->changeTagName("OldTag", "NewTag");

    EXPECT_TRUE(result);
}

TEST_F(UT_TagManager, hideFiles)
{
    bool isCall = false;
    QObject::connect(ins, &TagManager::filesHidden, [&isCall]() {
        isCall = true;
    });
    ins->hideFiles(QList<QString>() << "red", QList<QUrl>() << QUrl("/test"));
    EXPECT_TRUE(isCall);
}

TEST_F(UT_TagManager, paintListTagsHandle)
{
    DFMGLOBAL_USE_NAMESPACE
    FileInfoPointer info(new FileInfo(QUrl("file:///test")));
    QPainter painter;
    QRectF rect;
    // overload func
    auto func = static_cast<bool (TagManager::*)(const FileInfoPointer &) const>(&TagManager::canTagFile);
    stub.set_lamda(func, []() { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(ins->paintListTagsHandle(1, info, &painter, &rect));
    stub.set_lamda(func, []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_FALSE(ins->paintListTagsHandle(1, info, &painter, &rect));
    EXPECT_FALSE(ins->paintListTagsHandle(kItemFileDisplayNameRole, info, &painter, &rect));
}

TEST_F(UT_TagManager, paintListTagsHandle2)
{
    FileInfoPointer info(new FileInfo(QUrl("file:///test")));
    auto func = static_cast<bool (TagManager::*)(const FileInfoPointer &) const>(&TagManager::canTagFile);
    stub.set_lamda(func, []() { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(ins->addIconTagsHandle(info, nullptr));
    stub.set_lamda(func, []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_FALSE(ins->addIconTagsHandle(info, nullptr));
}

TEST_F(UT_TagManager, assignColorToTags)
{
    stub.set_lamda(&TagManager::getAllTags, []() { __DBG_STUB_INVOKE__ return TagManager::TagColorMap(); });
    stub.set_lamda(&TagManager::getTagsColor, []() { __DBG_STUB_INVOKE__ return TagManager::TagColorMap(); });
    stub.set_lamda(&TagManager::registerTagColor, []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagHelper::qureyColorByColorName, []() { __DBG_STUB_INVOKE__ return QColor("red"); });
    stub.set_lamda(&TagHelper::getColorNameByTag, []() { __DBG_STUB_INVOKE__ return QString("test"); });

    EXPECT_TRUE(ins->assignColorToTags(QStringList() << QString("test")).contains("test"));
}

TEST_F(UT_TagManager, contenxtMenuHandle)
{
    // Test context menu handle - stub to prevent actual menu display
    QUrl tagUrl;
    tagUrl.setScheme("tag");
    tagUrl.setPath("/TestTag");
    QPoint globalPos(100, 100);

    stub.set_lamda(&TagEventCaller::sendCheckTabAddable, [](quint64) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper *, const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    bool isCall = false;
    stub.set_lamda((QAction * (QMenu::*)(const QPoint &, QAction *)) ADDR(QMenu, exec), [&]() {
        isCall = true;
        return nullptr;
    });

    // Note: This method creates and shows a menu, which involves GUI
    // In actual unit tests, we would stub DMenu::exec to prevent showing
    // For now, we just verify the method can be called
    EXPECT_NO_THROW(TagManager::contenxtMenuHandle(0, tagUrl, globalPos));
    EXPECT_TRUE(isCall);
}

TEST_F(UT_TagManager, renameHandle)
{
    // Test rename handle
    QUrl tagUrl;
    tagUrl.setScheme("tag");
    tagUrl.setPath("/OldTag");

    stub.set_lamda(&TagHelper::getTagNameFromUrl, [](const TagHelper *, const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "OldTag";
    });

    stub.set_lamda(&TagManager::changeTagName, [](TagManager *, const QString &, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_THROW(TagManager::renameHandle(0, tagUrl, "NewTag"));
}
