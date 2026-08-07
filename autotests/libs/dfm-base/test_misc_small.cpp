// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_misc_small.cpp
 * @brief Small coverage tests for:
 *        - CustomSettingItemRegister (base/configs/customsettingitemregister.cpp)
 *        - AbstractFrame (interfaces/abstractframe.cpp) — ctor via minimal subclass
 *        - MimesAppsManager D0 destructor path (heap-allocate the singleton)
 *        - LocalFileIconProvider, LocalFileWatcher, MimeTypeDisplayManager,
 *          ConfigSynchronizer, SqliteConnectionPool, Shortcut, Settings
 */

#include <gtest/gtest.h>
#include <QString>
#include <QMap>
#include <QUrl>
#include <QDir>
#include <QTemporaryDir>
#include <QFile>

#include <dfm-base/settingdialog/customsettingitemregister.h>
#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/file/local/localfileiconprovider.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/db/sqliteconnectionpool.h>
#include <dfm-base/shortcut/shortcut.h>
#include <dfm-base/base/application/settings.h>

using namespace dfmbase;

// ---- CustomSettingItemRegister ----

TEST(CustomSettingItemRegisterTest, InstanceReturnsSingleton)
{
    auto *a = CustomSettingItemRegister::instance();
    auto *b = CustomSettingItemRegister::instance();
    EXPECT_EQ(a, b);
}

TEST(CustomSettingItemRegisterTest, GetCreatorsReturnsMap)
{
    EXPECT_NO_FATAL_FAILURE({ (void)CustomSettingItemRegister::instance()->getCreators(); });
}

TEST(CustomSettingItemRegisterTest, RegisterDuplicateTypeReturnsFalse)
{
    auto *reg = CustomSettingItemRegister::instance();
    QString type = "dfm_test_custom_type";
    // Register with a valid function pointer (nullptr is valid for the typedef)
    CustomSettingItemCreator creator = nullptr;
    bool first = reg->registCustomSettingItemType(type, creator);
    bool second = reg->registCustomSettingItemType(type, creator);
    // At least one must be false (duplicate)
    EXPECT_FALSE(first && second);
}

// ---- AbstractFrame ----

namespace {
class FakeFrame : public AbstractFrame
{
public:
    explicit FakeFrame(QWidget *parent = nullptr) : AbstractFrame(parent) {}
    void setCurrentUrl(const QUrl &) override {}
    QUrl currentUrl() const override { return QUrl(); }
};
}   // namespace

TEST(AbstractFrameTest, CtorAndDtorSafe)
{
    {
        FakeFrame f;
        SUCCEED();
    }
}

// ---- MimesAppsManager D0 destructor ----

TEST(MimesAppsManagerTest, HeapAllocatedDtorPath)
{
    // The singleton's D0 (deleting) destructor is not normally exercised;
    // heap-allocate a separate instance to hit that path.
    auto *ptr = new MimesAppsManager();
    EXPECT_NO_FATAL_FAILURE({ delete ptr; });
}

// ---- Misc small R20 coverage tests ----

TEST(MiscSmallTest, LocalFileIconProviderCtor)
{
    LocalFileIconProvider p;
    SUCCEED();
}

TEST(MiscSmallTest, LocalFileWatcherCtor)
{
    LocalFileWatcher w(QUrl::fromLocalFile("/tmp/dfm_test_watcher"));
    SUCCEED();
}

TEST(MiscSmallTest, MimeTypeDisplayManagerInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)MimeTypeDisplayManager::instance(); });
}

TEST(MiscSmallTest, ConfigSynchronizerInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ConfigSynchronizer::instance(); });
}

TEST(MiscSmallTest, SqliteConnectionPoolInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)SqliteConnectionPool::instance(); });
}

TEST(MiscSmallTest, ShortcutToStrReturnsJson)
{
    Shortcut s;
    QString result = s.toStr();
    EXPECT_FALSE(result.isEmpty());
}

TEST(MiscSmallTest, SettingsHeapD0)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString f = tmp.path() + "/s.json";
    QFile out(f);
    out.open(QIODevice::WriteOnly);
    out.write("{}");
    out.close();
    auto *s = new Settings(f, f, f);
    EXPECT_NO_FATAL_FAILURE({ delete s; });
}
