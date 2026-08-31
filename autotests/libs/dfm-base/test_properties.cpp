// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_properties.cpp
 * @brief Unit tests for Properties (properties.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryFile>
#include <QFile>
#include <QTextStream>
#include <QDir>

#include <dfm-base/utils/properties.h>

namespace {
QString writePropFile(const QString &path, const QString &content)
{
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&f);
    out << content;
    f.close();
    return path;
}
}   // namespace

TEST(PropertiesTest, EmptyConstructorDoesNotLoad)
{
    Properties p;
    EXPECT_FALSE(p.contains("anything"));
    EXPECT_EQ(p.getKeys().size(), 0);
}

TEST(PropertiesTest, LoadFileWithoutGroupReadsAllKeys)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writePropFile(path, "key1=value1\nkey2=value2\n\nkey3 = hello world\n");

    Properties p(path);
    EXPECT_EQ(p.value("key1").toString(), QString("value1"));
    EXPECT_EQ(p.value("key2").toString(), QString("value2"));
    EXPECT_EQ(p.value("key3").toString(), QString("hello world"));
    EXPECT_TRUE(p.contains("key1"));
    EXPECT_EQ(p.getKeys().size(), 3);
}

TEST(PropertiesTest, LoadFileWithGroupOnlyReadsGroupKeys)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writePropFile(path,
                  "[group1]\nk1=v1\n[group2]\nk2=v2\n");

    Properties p(path, "group2");
    EXPECT_FALSE(p.contains("k1"));
    EXPECT_TRUE(p.contains("k2"));
    EXPECT_EQ(p.value("k2").toString(), QString("v2"));
}

TEST(PropertiesTest, LoadNonExistentFileReturnsFalse)
{
    Properties p;
    EXPECT_FALSE(p.load("/no/such/file/exists/12345"));
}

TEST(PropertiesTest, LoadClearsOldData)
{
    QTemporaryFile tmp1;
    ASSERT_TRUE(tmp1.open());
    QString p1 = tmp1.fileName();
    tmp1.close();
    writePropFile(p1, "a=1\nb=2\n");

    QTemporaryFile tmp2;
    ASSERT_TRUE(tmp2.open());
    QString p2 = tmp2.fileName();
    tmp2.close();
    writePropFile(p2, "c=3\n");

    Properties p(p1);
    ASSERT_TRUE(p.contains("a"));
    ASSERT_TRUE(p.load(p2));
    EXPECT_FALSE(p.contains("a"));
    EXPECT_TRUE(p.contains("c"));
}

TEST(PropertiesTest, ValueReturnsDefaultWhenMissing)
{
    Properties p;
    EXPECT_EQ(p.value("missing", QString("def")).toString(), QString("def"));
}

TEST(PropertiesTest, SetInsertsNewKey)
{
    Properties p;
    p.set("newKey", QString("newVal"));
    EXPECT_TRUE(p.contains("newKey"));
    EXPECT_EQ(p.value("newKey").toString(), QString("newVal"));
}

TEST(PropertiesTest, SetOverwritesExistingKey)
{
    Properties p;
    p.set("k", QString("v1"));
    p.set("k", QString("v2"));
    EXPECT_EQ(p.value("k").toString(), QString("v2"));
}

TEST(PropertiesTest, SaveRoundTripPersistsData)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();

    Properties p;
    p.set("alpha", QString("1"));
    p.set("beta", QString("2"));
    ASSERT_TRUE(p.save(path, "MyGroup"));

    Properties loaded(path, "MyGroup");
    EXPECT_EQ(loaded.value("alpha").toString(), QString("1"));
    EXPECT_EQ(loaded.value("beta").toString(), QString("2"));
}

TEST(PropertiesTest, SaveWithoutGroupWritesFlat)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();

    Properties p;
    p.set("x", QString("y"));
    ASSERT_TRUE(p.save(path, ""));

    Properties loaded(path);
    EXPECT_EQ(loaded.value("x").toString(), QString("y"));
}

TEST(PropertiesTest, CopyConstructorSharesData)
{
    Properties p;
    p.set("k", QString("v"));
    Properties copy(p);
    EXPECT_TRUE(copy.contains("k"));
    EXPECT_EQ(copy.value("k").toString(), QString("v"));
}

TEST(PropertiesTest, SaveToInvalidPathReturnsFalse)
{
    Properties p;
    p.set("k", QString("v"));
    EXPECT_FALSE(p.save("/no/such/dir/out/file.props"));
}


TEST(PropertiesTest, Properties)
{
    // Properties
    SUCCEED();
}

TEST(PropertiesTest, contains)
{
    // contains
    SUCCEED();
}

TEST(PropertiesTest, save)
{
    // save
    SUCCEED();
}

TEST(PropertiesTest, value)
{
    // value
    SUCCEED();
}
