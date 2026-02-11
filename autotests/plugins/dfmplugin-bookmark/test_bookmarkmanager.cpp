// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "controller/bookmarkmanager.h"
#include "utils/bookmarkhelper.h"
#include "controller/defaultitemmanager.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/event/event.h>

#include <DDialog>

#include <QUrl>
#include <QDateTime>
#include <QVariantMap>
#include <QVariantList>

DFMBASE_USE_NAMESPACE
DPBOOKMARK_USE_NAMESPACE
DPF_USE_NAMESPACE

Q_DECLARE_METATYPE(QList<QUrl> *)

class UT_BookmarkManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ins = BookMarkManager::instance();
        ins->initData();
        const QString &nameKey = "Recent";
        const QString &displayName = QObject::tr("Recent");

        map = {
            { "Property_Key_NameKey", nameKey },
            { "Property_Key_DisplayName", displayName },
            { "Property_Key_Url", QUrl("recent:/") },
            { "Property_Key_Index", 0 },
            { "Property_Key_IsDefaultItem", true },
            { "Property_Key_PluginItemData", true }
        };
        bookmarkData.resetData(map);
        typedef QVariant (Settings::*ValueFunc)(const QString &, const QString &, const QVariant &) const;
        auto valueFunc = static_cast<ValueFunc>(&Settings::value);
        stub.set_lamda(valueFunc, [&] {
            __DBG_STUB_INVOKE__
            QVariantList temList;
            QVariantMap temData;
            temData.insert("url", testUrl);
            temList.append(temData);
            return QVariant::fromValue(temList);
        });
        typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
        auto setValueFunc = static_cast<SetValue>(&Settings::setValue);
        stub.set_lamda(setValueFunc, [] {});
    }
    virtual void TearDown() override
    {
        stub.clear();
    }
    static void SetUpTestCase()
    {
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
    }

protected:
    stub_ext::StubExt stub;
    BookMarkManager *ins;
    QVariantMap map;
    BookmarkData bookmarkData;
    QUrl testUrl = QUrl("file:///home/bookmark_test_dir");   // 测试URL，只做字符串使用，没有实际操作文件
};

TEST_F(UT_BookmarkManager, initDefaultItems)
{
    DefaultItemManager::instance()->initDefaultItems();
    QList<BookmarkData> list = DefaultItemManager::instance()->defaultItemInitOrder();

    EXPECT_TRUE(list.count() == 7);   // 默认项数据初始化完成后，应该有7个默认项。
}

TEST_F(UT_BookmarkManager, addQuickAccess)
{
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QList<QUrl>, QList<QUrl> *&&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [] { return true; });

    typedef QVariant (Settings::*ValueFunc)(const QString &, const QString &, const QVariant &) const;
    auto valueFunc = static_cast<ValueFunc>(&Settings::value);
    stub.set_lamda(valueFunc, [] { return QVariant(); });

    typedef void (Settings::*SetValueFunc)(const QString &, const QString &, const QVariant &);
    auto setValueFunc = static_cast<SetValueFunc>(&Settings::setValue);
    stub.set_lamda(setValueFunc, [] { __DBG_STUB_INVOKE__ return; });

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, QUrl, QVariantMap &);
    auto push = static_cast<PushFunc>(&EventChannelManager::push);
    stub.set_lamda(push, [] { return QVariant(); });

    stub.set_lamda(&QFileInfo::isDir, [] { return true; });
    stub.set_lamda(&BookMarkHelper::icon, [] { return QIcon(); });

    QList<QUrl> testUrls;
    testUrls << testUrl;

    EXPECT_TRUE(ins->addBookMark(testUrls));
};

TEST_F(UT_BookmarkManager, removeQuickAccess)
{
    stub.set_lamda(&BookMarkManager::saveSortedItemsToConfigFile, [] { __DBG_STUB_INVOKE__ });
    typedef QVariant (EventChannelManager::*PushFunc2)(const QString &, const QString &, QVariantMap &);
    auto push2 = static_cast<PushFunc2>(&EventChannelManager::push);
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    typedef QVariant (Settings::*ValueFunc)(const QString &, const QString &, const QVariant &) const;
    auto valueFunc = static_cast<ValueFunc>(&Settings::value);
    stub.set_lamda(valueFunc, [&] {
        __DBG_STUB_INVOKE__
        QVariantList temList;
        QVariantMap temData;
        temData.insert("url", testUrl);
        temList.append(temData);
        return QVariant::fromValue(temList);
    });
    EXPECT_TRUE(ins->removeBookMark(testUrl));
};

TEST_F(UT_BookmarkManager, fileRenamed)
{
    typedef QVariant (Settings::*ValueFunc)(const QString &, const QString &, const QVariant &) const;
    auto valueFunc = static_cast<ValueFunc>(&Settings::value);
    stub.set_lamda(valueFunc, [&] {
        __DBG_STUB_INVOKE__
        QVariantList temList;
        QVariantMap temData;
        temData.insert("url", testUrl);
        temList.append(temData);
        return QVariant::fromValue(temList);
    });
    ins->quickAccessDataMap.insert(testUrl, BookmarkData());
    ins->fileRenamed(testUrl, testUrl);
};
TEST_F(UT_BookmarkManager, addQuickAccessItemsFromConfig)
{
    bool isRun = false;
    QUrl url = testUrl;
    typedef QVariant (Settings::*ValueFunc)(const QString &, const QString &, const QVariant &) const;
    auto valueFunc = static_cast<ValueFunc>(&Settings::value);
    stub.set_lamda(valueFunc, [&isRun, url] {
        __DBG_STUB_INVOKE__
        QVariantList temList;
        QVariantMap temData;
        temData.insert("name", "testUrl");
        temData.insert("url", url);
        isRun = true;
        temList.append(temData);
        return QVariant::fromValue(temList);
    });
    ins->addQuickAccessItemsFromConfig();
    EXPECT_TRUE(isRun);
};
TEST_F(UT_BookmarkManager, bookMarkRename)
{
    typedef QVariant (Settings::*ValueFunc)(const QString &, const QString &, const QVariant &) const;
    auto valueFunc = static_cast<ValueFunc>(&Settings::value);
    stub.set_lamda(valueFunc, [&] {
        __DBG_STUB_INVOKE__
        QVariantList temList;
        QVariantMap temData;
        temData.insert("name", "testUrl");
        temData.insert("url", testUrl);
        temList.append(temData);
        return QVariant::fromValue(temList);
    });
    EXPECT_TRUE(ins->bookMarkRename(testUrl, "666"));
    EXPECT_FALSE(ins->bookMarkRename(testUrl, ""));
};
TEST_F(UT_BookmarkManager, showRemoveBookMarkDialog)
{
    bool isRun = false;
    FileManagerWindowsManager::FMWindow *window = new FileManagerWindowsManager::FMWindow(QUrl());
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&window] {
        return window;
    });
    stub.set_lamda(VADDR(DDialog, exec), [&isRun] {
        isRun = true;
        return 0;
    });

    ins->showRemoveBookMarkDialog(1);
    EXPECT_TRUE(isRun);
};
TEST_F(UT_BookmarkManager, getBookMarkDataMap)
{
    EXPECT_TRUE(!ins->getBookMarkDataMap().isEmpty());
};
TEST_F(UT_BookmarkManager, sortItemsByOrder)
{
    bool isRun = false;
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    auto setValueFunc = static_cast<SetValue>(&Settings::setValue);
    stub.set_lamda(setValueFunc, [&isRun] {
        isRun = true;
    });
    ins->saveSortedItemsToConfigFile(QList<QUrl>() << testUrl);
    EXPECT_TRUE(isRun);
};
