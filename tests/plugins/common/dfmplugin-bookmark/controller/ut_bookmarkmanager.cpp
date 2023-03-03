// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-bookmark/controller/bookmarkmanager.h"
#include "plugins/common/core/dfmplugin-bookmark/controller/defaultitemmanager.h"
#include "plugins/common/core/dfmplugin-bookmark/utils/bookmarkhelper.h"

#include "dfm-base/interfaces/abstractfilewatcher.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>
#include <gtest/gtest_prod.h>

#include <QDebug>
#include <QIcon>

DFMBASE_USE_NAMESPACE
DPBOOKMARK_USE_NAMESPACE
DPF_USE_NAMESPACE

Q_DECLARE_METATYPE(QList<QUrl> *)

class UT_BookmarkManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
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

private:
    stub_ext::StubExt stub;
    QUrl testUrl = QUrl("file:///home/bookmark_test_dir");//测试URL，只做字符串使用，没有实际操作文件
};

TEST_F(UT_BookmarkManager, initDefaultItems)
{
    DefaultItemManager::instance()->initDefaultItems();
    QList<BookmarkData> list = DefaultItemManager::instance()->defaultItemInitOrder();

    EXPECT_TRUE(list.count() == 7); //默认项数据初始化完成后，应该有7个默认项。
}

TEST_F(UT_BookmarkManager, addPluginItem)
{
    const QString &nameKey = "Recent";
    const QString &displayName = QObject::tr("Recent");

    QVariantMap map {
        { "Property_Key_NameKey", nameKey },
        { "Property_Key_DisplayName", displayName },
        { "Property_Key_Url", QUrl("recent:/") },
        { "Property_Key_Index", 0 },
        { "Property_Key_IsDefaultItem", true }
    };
    DefaultItemManager::instance()->addPluginItem(map);
    EXPECT_TRUE(DefaultItemManager::instance()->pluginItemData().count() > 0);
    EXPECT_TRUE(DefaultItemManager::instance()->pluginItemData().keys().first() == nameKey);
};

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

    EXPECT_TRUE(BookMarkManager::instance()->addBookMark(testUrls));
};

TEST_F(UT_BookmarkManager, removeQuickAccess)
{
    stub.set_lamda(&BookMarkManager::sortItemsByOrder, [] { __DBG_STUB_INVOKE__ });
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
    EXPECT_TRUE(BookMarkManager::instance()->removeBookMark(testUrl));
};
