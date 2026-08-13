// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractentryfileentity.cpp
 * @brief Unit tests for AbstractEntryFileEntity (abstractentryfileentity.cpp)
 *        and the EntryEntityFactor factory.
 *
 * AbstractEntryFileEntity is an abstract QObject base for device entry
 * entities; only the constructor and destructor have .cpp bodies. A small
 * TestableEntryEntity stub implements the pure virtuals so the base can be
 * instantiated. EntryEntityFactor::registCreator/create are exercised with a
 * unique suffix to avoid colliding with other test translation units.
 */

#include <gtest/gtest.h>
#include <dfm-base/interfaces/abstractentryfileentity.h>

#include <QUrl>
#include <QString>
#include <QVariant>

using namespace dfmbase;

namespace {
class TestableEntryEntity : public AbstractEntryFileEntity
{
public:
    explicit TestableEntryEntity(const QUrl &url)
        : AbstractEntryFileEntity(url) {}

    QString displayName() const override { return QStringLiteral("ut-entry"); }
    QIcon icon() const override { return {}; }
    bool exists() const override { return true; }
    bool showProgress() const override { return false; }
    bool showTotalSize() const override { return false; }
    bool showUsageSize() const override { return false; }
    EntryOrder order() const override { return kOrderCustom; }
};
}   // namespace

TEST(AbstractEntryFileEntityTest, ConstructAndDestructWithoutCrash)
{
    {
        TestableEntryEntity entity(QUrl(QStringLiteral("entry:///ut")));
        (void)entity;
    }
    SUCCEED();
}

TEST(AbstractEntryFileEntityTest, EntryEntityFactorRegisterNewSuffixSucceeds)
{
    bool ok = EntryEntityFactor::registCreator<TestableEntryEntity>(
        QStringLiteral("ut-entry-suffix-1"));
    EXPECT_TRUE(ok);
}

TEST(AbstractEntryFileEntityTest, EntryEntityFactorRegisterDuplicateReturnsFalse)
{
    ASSERT_TRUE(EntryEntityFactor::registCreator<TestableEntryEntity>(
        QStringLiteral("ut-entry-suffix-2")));
    EXPECT_FALSE(EntryEntityFactor::registCreator<TestableEntryEntity>(
        QStringLiteral("ut-entry-suffix-2")));
}

TEST(AbstractEntryFileEntityTest, EntryEntityFactorCreateReturnsInstanceForRegisteredSuffix)
{
    ASSERT_TRUE(EntryEntityFactor::registCreator<TestableEntryEntity>(
        QStringLiteral("ut-entry-suffix-3")));
    AbstractEntryFileEntity *entity =
        EntryEntityFactor::create(QStringLiteral("ut-entry-suffix-3"),
                                  QUrl(QStringLiteral("entry:///ut3")));
    ASSERT_NE(entity, nullptr);
    delete entity;
}

TEST(AbstractEntryFileEntityTest, EntryEntityFactorCreateReturnsNullForUnknownSuffix)
{
    AbstractEntryFileEntity *entity =
        EntryEntityFactor::create(QStringLiteral("ut-entry-unknown-9999"),
                                  QUrl(QStringLiteral("entry:///none")));
    EXPECT_EQ(entity, nullptr);
}

// --- Additional coverage for inline virtual methods ---

TEST(AbstractEntryFileEntityTest, InlineMethods_AllDefaults)
{
    QUrl url(QStringLiteral("entry:///ut-inline"));
    TestableEntryEntity entity(url);

    // refresh() is inline no-op
    EXPECT_NO_FATAL_FAILURE({ entity.refresh(); });

    // sizeTotal() returns 0
    EXPECT_EQ(entity.sizeTotal(), 0u);

    // sizeUsage() returns 0
    EXPECT_EQ(entity.sizeUsage(), 0u);

    // description() returns empty string
    EXPECT_TRUE(entity.description().isEmpty());

    // targetUrl() returns empty
    EXPECT_FALSE(entity.targetUrl().isValid());

    // isAccessable() returns true
    EXPECT_TRUE(entity.isAccessable());

    // renamable() returns false
    EXPECT_FALSE(entity.renamable());

    // extraProperties() returns empty hash by default
    QVariantHash props = entity.extraProperties();
    EXPECT_TRUE(props.isEmpty());

    // editDisplayText() returns displayName()
    EXPECT_EQ(entity.editDisplayText(), entity.displayName());

    // setExtraProperty / extraProperties round-trip
    entity.setExtraProperty("key1", 42);
    entity.setExtraProperty("key2", QStringLiteral("hello"));
    QVariantHash props2 = entity.extraProperties();
    EXPECT_EQ(props2.value("key1").toInt(), 42);
    EXPECT_EQ(props2.value("key2").toString(), QStringLiteral("hello"));

    // Overwrite a property
    entity.setExtraProperty("key1", 99);
    EXPECT_EQ(entity.extraProperties().value("key1").toInt(), 99);
}
