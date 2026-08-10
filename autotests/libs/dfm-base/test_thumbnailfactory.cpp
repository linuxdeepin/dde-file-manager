// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_thumbnailfactory.cpp
 * @brief Unit tests for ThumbnailFactory (thumbnailfactory.cpp)
 *
 * ThumbnailFactory is a singleton whose constructor registers a set of
 * built-in thumbnail creators and starts a worker thread. Here we exercise
 * the singleton accessor and the registerThumbnailCreator() boolean contract
 * (new type succeeds, already-registered type is rejected). joinThumbnailJob
 * is intentionally NOT called to avoid spawning real thumbnail work.
 */

#include <gtest/gtest.h>
#include <dfm-base/utils/thumbnail/thumbnailfactory.h>

#include <QString>
#include <QImage>

using namespace dfmbase;

namespace {
ThumbnailFactory::ThumbnailCreator utNoopCreator =
    [](const QString &, DFMGLOBAL_NAMESPACE::ThumbnailSize) {
        return QImage();
    };
}   // namespace

TEST(ThumbnailFactoryTest, InstanceReturnsNonNullSingleton)
{
    auto *a = ThumbnailFactory::instance();
    auto *b = ThumbnailFactory::instance();
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a, b);
}

TEST(ThumbnailFactoryTest, RegisterNewMimeTypeSucceeds)
{
    auto *f = ThumbnailFactory::instance();
    bool ok = f->registerThumbnailCreator(QStringLiteral("application/x-ut-thumbnail-test"),
                                          utNoopCreator);
    EXPECT_TRUE(ok);
}

TEST(ThumbnailFactoryTest, RegisterDuplicateMimeTypeReturnsFalse)
{
    auto *f = ThumbnailFactory::instance();
    // "image/*" is registered by the constructor.
    bool ok = f->registerThumbnailCreator(QStringLiteral("image/*"), utNoopCreator);
    EXPECT_FALSE(ok);
}

TEST(ThumbnailFactoryTest, RegisterDuplicateOfJustRegisteredReturnsFalse)
{
    auto *f = ThumbnailFactory::instance();
    const QString mime = QStringLiteral("application/x-ut-thumbnail-dup");
    ASSERT_TRUE(f->registerThumbnailCreator(mime, utNoopCreator));
    EXPECT_FALSE(f->registerThumbnailCreator(mime, utNoopCreator));
}
