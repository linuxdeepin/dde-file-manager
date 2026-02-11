// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/searchhistroymanager.h"
#include "dfmplugin_titlebar_global.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-mount/base/dmount_global.h>

#include <gtest/gtest.h>
#include <QDateTime>
#include <QUrl>
#include <QSignalSpy>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

class SearchHistroyManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        manager = SearchHistroyManager::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

    SearchHistroyManager *manager { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(SearchHistroyManagerTest, Instance_Singleton_ReturnsSameInstance)
{
    auto manager1 = SearchHistroyManager::instance();
    auto manager2 = SearchHistroyManager::instance();
    EXPECT_EQ(manager1, manager2);
    EXPECT_NE(manager1, nullptr);
}

TEST_F(SearchHistroyManagerTest, GetSearchHistroy_EmptyHistory_ReturnsEmptyList)
{
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    QStringList result = manager->getSearchHistroy();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(SearchHistroyManagerTest, GetSearchHistroy_HasHistory_ReturnsHistoryList)
{
    QStringList mockHistory;
    mockHistory << "keyword1"
                << "keyword2"
                << "smb://192.168.1.1";

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value),
                   [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
                       __DBG_STUB_INVOKE__
                       return QVariant(mockHistory);
                   });

    QStringList result = manager->getSearchHistroy();
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result, mockHistory);
}

TEST_F(SearchHistroyManagerTest, GetIPHistory_EmptyHistory_ReturnsEmptyList)
{
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QVariantList());
    });

    QList<IPHistroyData> result = manager->getIPHistory();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(SearchHistroyManagerTest, GetIPHistory_HasValidHistory_ReturnsIPHistoryList)
{
    QVariantList mockHistory;
    QVariantMap item1;
    item1["ip"] = "smb://192.168.1.1";
    item1["lastAccessed"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    mockHistory << item1;

    QVariantMap item2;
    item2["ip"] = "ftp://192.168.1.2";
    item2["lastAccessed"] = QDateTime::currentDateTime().addDays(-2).toString(Qt::ISODate);
    mockHistory << item2;

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value),
                   [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
                       __DBG_STUB_INVOKE__
                       return QVariant(mockHistory);
                   });

    QList<IPHistroyData> result = manager->getIPHistory();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].accessedType, "smb");
    EXPECT_EQ(result[0].ipData, "192.168.1.1");
    EXPECT_EQ(result[1].accessedType, "ftp");
    EXPECT_EQ(result[1].ipData, "192.168.1.2");
}

TEST_F(SearchHistroyManagerTest, GetIPHistory_InvalidData_SkipsInvalidEntries)
{
    QVariantList mockHistory;
    QVariantMap item1;
    item1["ip"] = "";   // Empty IP
    item1["lastAccessed"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    mockHistory << item1;

    QVariantMap item2;
    item2["ip"] = "smb://192.168.1.1";
    item2["lastAccessed"] = "";   // Empty time
    mockHistory << item2;

    QVariantMap item3;
    item3["ip"] = "ftp://192.168.1.2";
    item3["lastAccessed"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    mockHistory << item3;

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value),
                   [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
                       __DBG_STUB_INVOKE__
                       return QVariant(mockHistory);
                   });

    QList<IPHistroyData> result = manager->getIPHistory();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].accessedType, "ftp");
    EXPECT_EQ(result[0].ipData, "192.168.1.2");
}

TEST_F(SearchHistroyManagerTest, WriteIntoSearchHistory_EmptyKeyword_DoesNotWrite)
{
    bool valueSetCalled = false;
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue),
                   [&valueSetCalled](Settings *, const QString &, const QString &, const QVariant &) {
                       __DBG_STUB_INVOKE__
                       valueSetCalled = true;
                   });

    manager->writeIntoSearchHistory("");
    EXPECT_FALSE(valueSetCalled);
}

TEST_F(SearchHistroyManagerTest, WriteIntoSearchHistory_ValidKeyword_WritesToHistory)
{
    QStringList mockHistory;
    mockHistory << "old_keyword";

    bool valueSetCalled = false;
    QVariant savedValue;

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(mockHistory);
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled, &savedValue](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
        savedValue = value;
    });

    manager->writeIntoSearchHistory("new_keyword");
    EXPECT_TRUE(valueSetCalled);
    QStringList resultList = savedValue.toStringList();
    EXPECT_EQ(resultList.size(), 2);
    EXPECT_EQ(resultList[0], "old_keyword");
    EXPECT_EQ(resultList[1], "new_keyword");
}

TEST_F(SearchHistroyManagerTest, WriteIntoSearchHistory_DuplicateKeyword_MovesToEnd)
{
    QStringList mockHistory;
    mockHistory << "keyword1"
                << "keyword2"
                << "keyword3";

    QVariant savedValue;

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(mockHistory);
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&savedValue](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        savedValue = value;
    });

    manager->writeIntoSearchHistory("keyword2");
    QStringList resultList = savedValue.toStringList();
    EXPECT_EQ(resultList.size(), 3);
    EXPECT_EQ(resultList[0], "keyword1");
    EXPECT_EQ(resultList[1], "keyword3");
    EXPECT_EQ(resultList[2], "keyword2");
}

TEST_F(SearchHistroyManagerTest, WriteIntoSearchHistory_InvalidSmbUrl_DoesNotWrite)
{
    bool valueSetCalled = false;
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
    });

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    manager->writeIntoSearchHistory("smb://invalid url with spaces");
    EXPECT_FALSE(valueSetCalled);
}

TEST_F(SearchHistroyManagerTest, WriteIntoSearchHistory_ValidSmbUrl_WritesToHistory)
{
    bool valueSetCalled = false;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
    });

    manager->writeIntoSearchHistory("smb://192.168.1.1");
    EXPECT_TRUE(valueSetCalled);
}

TEST_F(SearchHistroyManagerTest, WriteIntoSearchHistory_ValidFtpUrl_WritesToHistory)
{
    bool valueSetCalled = false;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
    });

    manager->writeIntoSearchHistory("ftp://192.168.1.1");
    EXPECT_TRUE(valueSetCalled);
}

TEST_F(SearchHistroyManagerTest, AddIPHistoryCache_ValidAddress_AddedToCache)
{
    manager->addIPHistoryCache("smb://192.168.1.1");
    // Access private member to verify (can access due to compiler flags)
    EXPECT_TRUE(manager->ipAddressCache.contains("smb://192.168.1.1"));
}

TEST_F(SearchHistroyManagerTest, AddIPHistoryCache_AddressWithTrailingSlash_TrimsSlash)
{
    manager->addIPHistoryCache("smb://192.168.1.1/");
    EXPECT_TRUE(manager->ipAddressCache.contains("smb://192.168.1.1"));
    EXPECT_FALSE(manager->ipAddressCache.contains("smb://192.168.1.1/"));
}

TEST_F(SearchHistroyManagerTest, AddIPHistoryCache_DuplicateAddress_DoesNotAddAgain)
{
    manager->addIPHistoryCache("smb://192.168.1.1");
    int initialSize = manager->ipAddressCache.size();
    manager->addIPHistoryCache("smb://192.168.1.1");
    EXPECT_EQ(manager->ipAddressCache.size(), initialSize);
}

TEST_F(SearchHistroyManagerTest, RemoveSearchHistory_EmptyKeyword_ReturnsFalse)
{
    bool result = manager->removeSearchHistory("");
    EXPECT_FALSE(result);
}

TEST_F(SearchHistroyManagerTest, RemoveSearchHistory_ExistingKeyword_RemovesAndReturnsTrue)
{
    QStringList mockHistory;
    mockHistory << "keyword1"
                << "keyword2"
                << "keyword3";

    bool valueSetCalled = false;
    QVariant savedValue;

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(mockHistory);
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled, &savedValue](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
        savedValue = value;
    });

    bool result = manager->removeSearchHistory("keyword2");
    EXPECT_TRUE(result);
    EXPECT_TRUE(valueSetCalled);
    QStringList resultList = savedValue.toStringList();
    EXPECT_EQ(resultList.size(), 2);
    EXPECT_FALSE(resultList.contains("keyword2"));
}

TEST_F(SearchHistroyManagerTest, RemoveSearchHistory_NonExistingKeyword_ReturnsFalse)
{
    QStringList mockHistory;
    mockHistory << "keyword1"
                << "keyword2";

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(mockHistory);
    });

    bool result = manager->removeSearchHistory("nonexistent");
    EXPECT_FALSE(result);
}

TEST_F(SearchHistroyManagerTest, RemoveSearchHistory_KeywordWithTrailingSlash_RemovesKeywordWithoutSlash)
{
    QStringList mockHistory;
    mockHistory << "smb://192.168.1.1";

    bool valueSetCalled = false;
    QVariant savedValue;

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(mockHistory);
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled, &savedValue](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
        savedValue = value;
    });

    bool result = manager->removeSearchHistory("smb://192.168.1.1/");
    EXPECT_TRUE(result);
    EXPECT_TRUE(valueSetCalled);
    QStringList resultList = savedValue.toStringList();
    EXPECT_FALSE(resultList.contains("smb://192.168.1.1"));
}

TEST_F(SearchHistroyManagerTest, ClearHistory_EmptySchemeFilters_ClearsAllHistory)
{
    bool valueSetCalled = false;
    QVariant savedValue;

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled, &savedValue](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
        savedValue = value;
    });

    manager->clearHistory();
    EXPECT_TRUE(valueSetCalled);
    QStringList resultList = savedValue.toStringList();
    EXPECT_TRUE(resultList.isEmpty());
}

TEST_F(SearchHistroyManagerTest, ClearHistory_WithSchemeFilters_ClearsOnlyMatchingSchemes)
{
    QStringList mockHistory;
    mockHistory << "smb://192.168.1.1"
                << "ftp://192.168.1.2"
                << "local_search"
                << "sftp://192.168.1.3";

    bool valueSetCalled = false;
    QVariant savedValue;

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(mockHistory);
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled, &savedValue](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
        savedValue = value;
    });

    QStringList schemeFilters;
    schemeFilters << "smb://"
                  << "ftp://";
    manager->clearHistory(schemeFilters);

    EXPECT_TRUE(valueSetCalled);
    QStringList resultList = savedValue.toStringList();
    EXPECT_EQ(resultList.size(), 2);
    EXPECT_TRUE(resultList.contains("local_search"));
    EXPECT_TRUE(resultList.contains("sftp://192.168.1.3"));
    EXPECT_FALSE(resultList.contains("smb://192.168.1.1"));
    EXPECT_FALSE(resultList.contains("ftp://192.168.1.2"));
}

TEST_F(SearchHistroyManagerTest, ClearIPHistory_Called_ClearsIPHistory)
{
    bool valueSetCalled = false;
    QVariant savedValue;

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled, &savedValue](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
        savedValue = value;
    });

    manager->clearIPHistory();
    EXPECT_TRUE(valueSetCalled);
    QVariantList resultList = savedValue.toList();
    EXPECT_TRUE(resultList.isEmpty());
}

TEST_F(SearchHistroyManagerTest, HandleMountNetworkResult_InvalidMount_RemovesFromCache)
{
    manager->addIPHistoryCache("smb://192.168.1.1");
    EXPECT_TRUE(manager->ipAddressCache.contains("smb://192.168.1.1"));

    manager->handleMountNetworkResult("smb://192.168.1.1", false, DFMMOUNT::DeviceError::kGIOErrorFailed, "");
    EXPECT_FALSE(manager->ipAddressCache.contains("smb://192.168.1.1"));
}

TEST_F(SearchHistroyManagerTest, HandleMountNetworkResult_SuccessfulMount_WritesToIPHistory)
{
    manager->addIPHistoryCache("smb://192.168.1.1");

    bool valueSetCalled = false;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariant(QVariantList());
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
    });

    manager->handleMountNetworkResult("smb://192.168.1.1", true, DFMMOUNT::DeviceError::kNoError, "");
    EXPECT_TRUE(valueSetCalled);
    EXPECT_FALSE(manager->ipAddressCache.contains("smb://192.168.1.1"));
}

TEST_F(SearchHistroyManagerTest, HandleMountNetworkResult_AlreadyMountedError_WritesToIPHistory)
{
    manager->addIPHistoryCache("ftp://192.168.1.2");

    bool valueSetCalled = false;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariantList();
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
    });

    manager->handleMountNetworkResult("ftp://192.168.1.2", false, DFMMOUNT::DeviceError::kGIOErrorAlreadyMounted, "");
    EXPECT_TRUE(valueSetCalled);
    EXPECT_FALSE(manager->ipAddressCache.contains("ftp://192.168.1.2"));
}

TEST_F(SearchHistroyManagerTest, HandleMountNetworkResult_AddressWithTrailingSlash_TrimsSlash)
{
    manager->addIPHistoryCache("smb://192.168.1.1/");

    bool valueSetCalled = false;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariantList();
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
    });

    manager->handleMountNetworkResult("smb://192.168.1.1/", true, DFMMOUNT::DeviceError::kNoError, "");
    EXPECT_TRUE(valueSetCalled);
}

TEST_F(SearchHistroyManagerTest, HandleMountNetworkResult_NotInCache_DoesNotWrite)
{
    bool valueSetCalled = false;
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
    });

    manager->handleMountNetworkResult("smb://192.168.1.99", true, DFMMOUNT::DeviceError::kNoError, "");
    EXPECT_FALSE(valueSetCalled);
}

TEST_F(SearchHistroyManagerTest, HandleMountNetworkResult_InvalidIPFormat_DoesNotWrite)
{
    manager->addIPHistoryCache("smb://hostname");

    bool valueSetCalled = false;
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
    });

    manager->handleMountNetworkResult("smb://hostname", true, DFMMOUNT::DeviceError::kNoError, "");
    EXPECT_FALSE(valueSetCalled);
}

TEST_F(SearchHistroyManagerTest, IsValidMount_NotInCache_ReturnsFalse)
{
    bool result = manager->isValidMount("smb://192.168.1.1", true, DFMMOUNT::DeviceError::kNoError);
    EXPECT_FALSE(result);
}

TEST_F(SearchHistroyManagerTest, IsValidMount_FailedWithError_ReturnsFalse)
{
    manager->addIPHistoryCache("smb://192.168.1.1");
    bool result = manager->isValidMount("smb://192.168.1.1", false, DFMMOUNT::DeviceError::kGIOErrorFailed);
    EXPECT_FALSE(result);
}

TEST_F(SearchHistroyManagerTest, IsValidMount_SuccessfulMount_ReturnsTrue)
{
    manager->addIPHistoryCache("smb://192.168.1.1");
    bool result = manager->isValidMount("smb://192.168.1.1", true, DFMMOUNT::DeviceError::kNoError);
    EXPECT_TRUE(result);
}

TEST_F(SearchHistroyManagerTest, IsValidMount_AlreadyMountedError_ReturnsTrue)
{
    manager->addIPHistoryCache("ftp://192.168.1.2");
    bool result = manager->isValidMount("ftp://192.168.1.2", false, DFMMOUNT::DeviceError::kGIOErrorAlreadyMounted);
    EXPECT_TRUE(result);
}

TEST_F(SearchHistroyManagerTest, IsValidMount_InvalidIPFormat_ReturnsFalse)
{
    manager->addIPHistoryCache("smb://hostname");
    bool result = manager->isValidMount("smb://hostname", true, DFMMOUNT::DeviceError::kNoError);
    EXPECT_FALSE(result);
}

TEST_F(SearchHistroyManagerTest, WriteIntoIPHistory_EmptyAddress_DoesNotWrite)
{
    bool valueSetCalled = false;
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&valueSetCalled](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueSetCalled = true;
    });

    manager->writeIntoIPHistory("");
    EXPECT_FALSE(valueSetCalled);
}

TEST_F(SearchHistroyManagerTest, WriteIntoIPHistory_NewAddress_AddsToHistory)
{
    QVariant savedValue;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return QVariantList();
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&savedValue](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        savedValue = value;
    });

    manager->writeIntoIPHistory("smb://192.168.1.1");
    QVariantList resultList = savedValue.toList();
    EXPECT_EQ(resultList.size(), 1);
    QVariantMap firstItem = resultList[0].toMap();
    EXPECT_EQ(firstItem["ip"].toString(), "smb://192.168.1.1");
}

TEST_F(SearchHistroyManagerTest, WriteIntoIPHistory_ExistingAddress_UpdatesTimestamp)
{
    QVariantList mockHistory;
    QVariantMap oldItem;
    oldItem["ip"] = "smb://192.168.1.1";
    oldItem["lastAccessed"] = QDateTime::currentDateTime().addDays(-5).toString(Qt::ISODate);
    mockHistory << oldItem;

    QVariant savedValue;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return mockHistory;
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&savedValue](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        savedValue = value;
    });

    manager->writeIntoIPHistory("smb://192.168.1.1");
    QVariantList resultList = savedValue.toList();
    EXPECT_EQ(resultList.size(), 1);
    QVariantMap updatedItem = resultList[0].toMap();
    EXPECT_EQ(updatedItem["ip"].toString(), "smb://192.168.1.1");
    EXPECT_NE(updatedItem["lastAccessed"].toString(), oldItem["lastAccessed"].toString());
}

TEST_F(SearchHistroyManagerTest, WriteIntoIPHistory_OldEntries_FiltersOutOldEntries)
{
    QVariantList mockHistory;

    QVariantMap recentItem;
    recentItem["ip"] = "smb://192.168.1.1";
    recentItem["lastAccessed"] = QDateTime::currentDateTime().addDays(-3).toString(Qt::ISODate);
    mockHistory << recentItem;

    QVariantMap oldItem;
    oldItem["ip"] = "ftp://192.168.1.2";
    oldItem["lastAccessed"] = QDateTime::currentDateTime().addDays(-10).toString(Qt::ISODate);
    mockHistory << oldItem;

    QVariant savedValue;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&mockHistory](Settings *, const QString &, const QString &, const QVariant &) {
        __DBG_STUB_INVOKE__
        return mockHistory;
    });

    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&savedValue](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        savedValue = value;
    });

    manager->writeIntoIPHistory("sftp://192.168.1.3");
    QVariantList resultList = savedValue.toList();
    EXPECT_EQ(resultList.size(), 2);

    // Verify old entry is filtered out
    bool hasOldEntry = false;
    for (const auto &item : resultList) {
        if (item.toMap()["ip"].toString() == "ftp://192.168.1.2") {
            hasOldEntry = true;
        }
    }
    EXPECT_FALSE(hasOldEntry);
}

TEST_F(SearchHistroyManagerTest, Constructor_InitializesRegExp_RegExpConfigured)
{
    EXPECT_TRUE(manager->protocolIPRegExp.pattern().contains("smb"));
    EXPECT_TRUE(manager->protocolIPRegExp.pattern().contains("ftp"));
    EXPECT_TRUE(manager->protocolIPRegExp.pattern().contains("sftp"));
}

TEST_F(SearchHistroyManagerTest, ProtocolIPRegExp_ValidSmbIP_Matches)
{
    QRegularExpressionMatch match = manager->protocolIPRegExp.match("smb://192.168.1.1");
    EXPECT_TRUE(match.hasMatch());
}

TEST_F(SearchHistroyManagerTest, ProtocolIPRegExp_ValidFtpIP_Matches)
{
    QRegularExpressionMatch match = manager->protocolIPRegExp.match("ftp://10.0.0.1");
    EXPECT_TRUE(match.hasMatch());
}

TEST_F(SearchHistroyManagerTest, ProtocolIPRegExp_ValidSftpIP_Matches)
{
    QRegularExpressionMatch match = manager->protocolIPRegExp.match("sftp://172.16.0.1");
    EXPECT_TRUE(match.hasMatch());
}

TEST_F(SearchHistroyManagerTest, ProtocolIPRegExp_Hostname_DoesNotMatch)
{
    QRegularExpressionMatch match = manager->protocolIPRegExp.match("smb://hostname");
    EXPECT_FALSE(match.hasMatch());
}

TEST_F(SearchHistroyManagerTest, ProtocolIPRegExp_InvalidIP_DoesNotMatch)
{
    QRegularExpressionMatch match = manager->protocolIPRegExp.match("smb://999.999.999.999");
    EXPECT_FALSE(match.hasMatch());
}

TEST_F(SearchHistroyManagerTest, ProtocolIPRegExp_CaseInsensitive_Matches)
{
    QRegularExpressionMatch match = manager->protocolIPRegExp.match("SMB://192.168.1.1");
    EXPECT_TRUE(match.hasMatch());
}

TEST_F(SearchHistroyManagerTest, ProtocolIPRegExp_WithTrailingSlash_Matches)
{
    QRegularExpressionMatch match = manager->protocolIPRegExp.match("smb://192.168.1.1/");
    EXPECT_TRUE(match.hasMatch());
}
