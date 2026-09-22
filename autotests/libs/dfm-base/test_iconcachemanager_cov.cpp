// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconcachemanager_cov.cpp
 * @brief Coverage-focused unit tests for IconCacheManager
 *        (src/dfm-base/utils/iconcachemanager.cpp).
 *
 * Covered functions from the gap list (11 entries):
 *   - clearTimer() + its nested timeout lambda (via clear + event loop)
 *   - connectScreenDpiChanged (via lazy initialize inside getPixmap)
 *   - initialize() + its lambda (lazy static init on first getPixmap)
 *   - getPixmap (invalid-argument branch, cache-hit branch, theme-load path)
 *   - getPixmap's lazy-init lambda
 *   - clear()
 *   - makeCacheKey()
 *
 * Case -> function mapping:
 *   MakeCacheKey_*   -> makeCacheKey (exact format string)
 *   GetPixmap_*      -> getPixmap branches + initialize/connectScreenDpiChanged
 *   Clear_*          -> clear + clearTimer + timeout lambda
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QPixmapCache>
#include <QSize>
#include <QString>

#include <dfm-base/utils/iconcachemanager.h>

using namespace dfmbase;

class UT_IconCacheManagerCov : public ::testing::Test
{
protected:
    void SetUp() override { QPixmapCache::clear(); }
    void TearDown() override { QPixmapCache::clear(); }
};

TEST_F(UT_IconCacheManagerCov, MakeCacheKey_FullArguments_ExpectExactKeyFormat)
{
    // NOTE: runtime enum values on this platform are QIcon::Normal=0,
    // Disabled=1, Active=2, Selected=3, On=0, Off=1.
    // Arrange
    QString key1 = IconCacheManager::makeCacheKey(
            "folder", QSize(16, 16), 2.0, QIcon::Normal, QIcon::On);

    // Act
    QString key2 = IconCacheManager::makeCacheKey("x", QSize(32, 48), 1.5,
                                                  QIcon::Disabled, QIcon::Off);

    // Assert
    EXPECT_EQ(key1, QStringLiteral("dfm:icon:folder:16x16:dpr2:0:0"));
    EXPECT_EQ(key2, QStringLiteral("dfm:icon:x:32x48:dpr1.5:1:1"));
}

TEST_F(UT_IconCacheManagerCov, GetPixmap_EmptyIconName_ExpectNullPixmap)
{
    // Arrange: first call also triggers the lazy initialize()
    // Act
    QPixmap px = IconCacheManager::getPixmap("", QSize(16, 16), 1.0);

    // Assert
    EXPECT_TRUE(px.isNull());
    EXPECT_EQ(px.width(), 0);
}

TEST_F(UT_IconCacheManagerCov, GetPixmap_InvalidSize_ExpectNullPixmap)
{
    // Arrange
    // Act
    QPixmap zeroW = IconCacheManager::getPixmap("folder", QSize(0, 16), 1.0);
    QPixmap negH = IconCacheManager::getPixmap("folder", QSize(16, -4), 1.0);

    // Assert
    EXPECT_TRUE(zeroW.isNull());
    EXPECT_EQ(zeroW.width(), 0);
    EXPECT_TRUE(negH.isNull());
}

TEST_F(UT_IconCacheManagerCov, GetPixmap_PreCachedKey_ExpectCacheHitReturnsStoredPixmap)
{
    // Arrange: prime QPixmapCache with the exact key the manager computes
    QPixmap stored(20, 20);
    stored.fill(Qt::red);
    const QString key = IconCacheManager::makeCacheKey("ut-cached-icon", QSize(20, 20), 1.0,
                                                       QIcon::Normal, QIcon::Off);
    ASSERT_TRUE(QPixmapCache::insert(key, stored));

    // Act
    QPixmap px = IconCacheManager::getPixmap("ut-cached-icon", QSize(20, 20), 1.0);

    // Assert
    ASSERT_FALSE(px.isNull());
    EXPECT_EQ(px.width(), 20);
    EXPECT_EQ(px.height(), 20);
}

TEST_F(UT_IconCacheManagerCov, GetPixmap_UnknownThemeIcon_ExpectNullPixmapFromLoadPath)
{
    // Arrange: a name no icon theme provides exercises the theme-load path
    // Act
    QPixmap px1 = IconCacheManager::getPixmap("ut-no-such-icon-xyz", QSize(16, 16), 1.0);
    QPixmap px2 = IconCacheManager::getPixmap("ut-no-such-icon-xyz", QSize(16, 16), 1.0);

    // Assert
    EXPECT_TRUE(px1.isNull());
    EXPECT_EQ(px1.isNull(), px2.isNull());
}

TEST_F(UT_IconCacheManagerCov, Clear_SchedulesDeferredCacheFlush_ExpectCacheEmptied)
{
    // Arrange
    QPixmap stored(10, 10);
    stored.fill(Qt::blue);
    ASSERT_TRUE(QPixmapCache::insert("ut-icon-cache-key", stored));
    ASSERT_TRUE(QPixmapCache::find("ut-icon-cache-key", &stored));

    // Act: clear() only arms the single-shot clearTimer
    IconCacheManager::clear();
    IconCacheManager::clear();   // second start() must merge, not crash
    bool stillThereBeforeLoop = QPixmapCache::find("ut-icon-cache-key", &stored);
    qApp->processEvents();
    QPixmap gone;

    // Assert
    EXPECT_TRUE(stillThereBeforeLoop);
    EXPECT_FALSE(QPixmapCache::find("ut-icon-cache-key", &gone));
    EXPECT_EQ(gone.width(), 0);
}

TEST_F(UT_IconCacheManagerCov, Initialize_Idempotent_ExpectCacheStillUsable)
{
    // Arrange: initialize is a no-op after the first call (magic static)
    // Act
    IconCacheManager::initialize();
    IconCacheManager::initialize();

    // Assert: a fresh pixmap can still be cached and cleared afterwards
    QPixmap px(5, 5);
    px.fill(Qt::green);
    EXPECT_EQ(px.width(), 5);
    EXPECT_TRUE(QPixmapCache::insert("ut-icon-init-key", px));
    IconCacheManager::clear();
    qApp->processEvents();
    EXPECT_FALSE(QPixmapCache::find("ut-icon-init-key", &px));
}
