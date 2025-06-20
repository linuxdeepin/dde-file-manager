// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mode/normalized/fileclassifier.h"
#include "mode/normalized/type/typeclassifier.h"
#include "config/configpresenter.h"

#include <dfm-framework/dpf.h>

#include <stubext.h>

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE
using namespace testing;

namespace {
class TestFileClassifier : public FileClassifier, public Test
{
public:
    Classifier mode() const { return kName; }
    ModelDataHandler *dataHandler() const { return nullptr; }
    QStringList classes() const { return ids.keys(); }
    QString classify(const QUrl &url) const { return ids.key(url.fileName().left(3)); }
    QString className(const QString &key) const { return ids.value(key); }
    void updateClassifier() {};

public:
    void SetUp() override
    {

        stub.set_lamda(&ConfigPresenter::instance, [this]() {
            return cfg;
        });

        cfg = new ConfigPresenter;
        ids.insert("1", "one");
        ids.insert("2", "two");
    }

    void TearDown() override
    {
        delete cfg;
    }

    stub_ext::StubExt stub;
    ConfigPresenter *cfg;
    QMap<QString, QString> ids;
};

}

TEST_F(TestFileClassifier, createClassifier)
{
    stub.set_lamda(&ConfigPresenter::enabledTypeCategories, []() {
        return kCatDefault;
    });

    auto ret = ClassifierCreator::createClassifier(Classifier::kType);
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ret->mode(), Classifier::kType);
    EXPECT_NE(dynamic_cast<TypeClassifier *>(ret), nullptr);
    delete ret;
}

TEST_F(TestFileClassifier, construct)
{
    CollectionBaseDataPtr dp(new CollectionBaseData);
    collections.insert("1", dp);
    QList<CollectionBaseDataPtr> in;
    stub.set_lamda(&ConfigPresenter::saveNormalProfile,
                   [&in](ConfigPresenter *, const QList<CollectionBaseDataPtr> &baseDatas) {
                       in = baseDatas;
                   });

    emit this->itemsChanged("1");

    ASSERT_EQ(in.size(), 1);
    EXPECT_EQ(in.first(), dp);
}

namespace {
class TestFileClassifier2 : public TestFileClassifier
{
public:
    void SetUp() override
    {
        TestFileClassifier::SetUp();
        stub.set_lamda(&ConfigPresenter::saveNormalProfile, []() {});
        QObject::connect(this, &FileClassifier::itemsChanged, this, [this](const QString &t) {
            types.append(t);
        });
    }

    void initDP()
    {
        dp.reset(new CollectionBaseData);
        dp->name = "one";
        dp->key = "1";
        this->collections.insert("1", dp);

        dp2.reset(new CollectionBaseData);
        dp2->name = "two";
        dp2->key = "2";
        this->collections.insert("2", dp2);
    }

    QStringList types;
    QUrl one1 = QUrl::fromLocalFile("/tmp/one1");
    QUrl one2 = QUrl::fromLocalFile("/tmp/one2");
    QUrl two1 = QUrl::fromLocalFile("/tmp/two1");
    QUrl test = QUrl::fromLocalFile("/tmp/test");
    CollectionBaseDataPtr dp;
    CollectionBaseDataPtr dp2;
};
}

TEST_F(TestFileClassifier2, reset)
{
    QList<QUrl> ins;
    ins.append(one1);
    ins.append(one2);
    ins.append(two1);
    ins.append(test);

    this->reset(ins);

    ASSERT_EQ(this->collections.size(), 2);

    {
        auto data = this->collections.value("1");
        ASSERT_NE(data, nullptr);
        EXPECT_EQ(data->key, QString("1"));
        EXPECT_EQ(data->name, QString("one"));
        ASSERT_EQ(data->items.size(), 2);
        EXPECT_EQ(data->items.first(), one1);
        EXPECT_EQ(data->items.last(), one2);

        EXPECT_EQ(this->baseData("1"), data);
    }

    {
        auto data = this->collections.value("2");
        ASSERT_NE(data, nullptr);
        EXPECT_EQ(data->key, QString("2"));
        EXPECT_EQ(data->name, QString("two"));
        ASSERT_EQ(data->items.size(), 1);
        EXPECT_EQ(data->items.first(), two1);

        EXPECT_EQ(this->baseData("2"), data);
    }
}

TEST_F(TestFileClassifier2, replace)
{
    initDP();

    // new is unknown
    {
        dp->items.append(one1);
        EXPECT_TRUE(this->replace(one1, test).isEmpty());
        EXPECT_TRUE(dp->items.isEmpty());
        EXPECT_TRUE(dp2->items.isEmpty());
        EXPECT_TRUE(types.isEmpty());
    }

    // new is same type
    {
        dp->items.clear();
        dp2->items.clear();
        types.clear();
        dp->items.append(one1);
        EXPECT_EQ(this->replace(one1, one2), QString("1"));
        ASSERT_EQ(dp->items.size(), 1);
        EXPECT_EQ(dp->items.first(), one2);
        EXPECT_TRUE(dp2->items.isEmpty());
        ASSERT_EQ(types.size(), 1);
        EXPECT_EQ(types.first(), QString("1"));
    }

    // type change
    {
        dp->items.clear();
        dp2->items.clear();
        types.clear();
        dp->items.append(one1);
        EXPECT_EQ(this->replace(one1, two1), QString("2"));
        ASSERT_EQ(dp2->items.size(), 1);
        EXPECT_EQ(dp2->items.first(), two1);
        EXPECT_TRUE(dp->items.isEmpty());
        ASSERT_EQ(types.size(), 2);
        EXPECT_EQ(types.first(), QString("1"));
        EXPECT_EQ(types.last(), QString("2"));
    }
}

TEST_F(TestFileClassifier2, append)
{
    initDP();

    // unknown
    {
        EXPECT_TRUE(this->append(test).isEmpty());
        EXPECT_TRUE(dp->items.isEmpty());
        EXPECT_TRUE(dp2->items.isEmpty());
        EXPECT_TRUE(types.isEmpty());
    }

    // do not exist
    {
        dp->items.clear();
        dp2->items.clear();
        types.clear();
        EXPECT_EQ(this->append(one1), QString("1"));
        ASSERT_EQ(dp->items.size(), 1);
        EXPECT_EQ(dp->items.first(), one1);
        EXPECT_TRUE(dp2->items.isEmpty());
        ASSERT_EQ(types.size(), 1);
        EXPECT_EQ(types.first(), QString("1"));

        types.clear();
        EXPECT_EQ(this->append(two1), QString("2"));
        ASSERT_EQ(dp2->items.size(), 1);
        EXPECT_EQ(dp2->items.first(), two1);
        EXPECT_EQ(dp->items.size(), 1);
        ASSERT_EQ(types.size(), 1);
        EXPECT_EQ(types.first(), QString("2"));

        types.clear();
        EXPECT_EQ(this->append(one2), QString("1"));
        ASSERT_EQ(dp->items.size(), 2);
        EXPECT_EQ(dp->items.last(), one2);
    }

    // exist and same
    {
        dp->items.clear();
        dp2->items.clear();
        types.clear();
        dp->items.append(one1);
        EXPECT_EQ(this->append(one1), QString("1"));
        EXPECT_EQ(dp->items.size(), 1);
        EXPECT_TRUE(dp2->items.isEmpty());
        EXPECT_TRUE(types.isEmpty());
    }

    // replace
    {
        dp->items.clear();
        dp2->items.clear();
        types.clear();
        dp->items.append(one1);

        stub.set_lamda(VADDR(TestFileClassifier, classify), [this](TestFileClassifier *self, const QUrl &url) {
            EXPECT_EQ(self->ids.key(url.fileName().left(3)), QString("1"));
            return QString("2");
        });

        EXPECT_EQ(this->append(one1), QString("2"));
        EXPECT_TRUE(dp->items.isEmpty());
        ASSERT_EQ(dp2->items.size(), 1);
        EXPECT_EQ(dp2->items.first(), one1);
        ASSERT_EQ(types.size(), 2);
        EXPECT_EQ(types.first(), QString("1"));
        EXPECT_EQ(types.last(), QString("2"));

        dp->items.append(one2);
        EXPECT_EQ(this->append(one2), QString("2"));
        ASSERT_EQ(dp2->items.size(), 2);
        EXPECT_EQ(dp2->items.last(), one2);
    }
}

TEST_F(TestFileClassifier2, prepend)
{
    initDP();

    // unknown
    {
        EXPECT_TRUE(this->prepend(test).isEmpty());
        EXPECT_TRUE(dp->items.isEmpty());
        EXPECT_TRUE(dp2->items.isEmpty());
        EXPECT_TRUE(types.isEmpty());
    }

    // do not exist
    {
        dp->items.clear();
        dp2->items.clear();
        types.clear();
        EXPECT_EQ(this->prepend(one1), QString("1"));
        ASSERT_EQ(dp->items.size(), 1);
        EXPECT_EQ(dp->items.first(), one1);
        EXPECT_TRUE(dp2->items.isEmpty());
        ASSERT_EQ(types.size(), 1);
        EXPECT_EQ(types.first(), QString("1"));

        types.clear();
        EXPECT_EQ(this->prepend(two1), QString("2"));
        ASSERT_EQ(dp2->items.size(), 1);
        EXPECT_EQ(dp2->items.first(), two1);
        EXPECT_EQ(dp->items.size(), 1);
        ASSERT_EQ(types.size(), 1);
        EXPECT_EQ(types.first(), QString("2"));

        types.clear();
        EXPECT_EQ(this->prepend(one2), QString("1"));
        ASSERT_EQ(dp->items.size(), 2);
        EXPECT_EQ(dp->items.first(), one2);
        EXPECT_EQ(dp->items.last(), one1);
    }

    // exist and same
    {
        dp->items.clear();
        dp2->items.clear();
        types.clear();
        dp->items.append(one1);
        EXPECT_EQ(this->prepend(one1), QString("1"));
        EXPECT_EQ(dp->items.size(), 1);
        EXPECT_TRUE(dp2->items.isEmpty());
        EXPECT_TRUE(types.isEmpty());
    }

    // replace
    {
        dp->items.clear();
        dp2->items.clear();
        types.clear();
        dp->items.append(one1);

        stub.set_lamda(VADDR(TestFileClassifier, classify), [this](TestFileClassifier *self, const QUrl &url) {
            EXPECT_EQ(self->ids.key(url.fileName().left(3)), QString("1"));
            return QString("2");
        });

        EXPECT_EQ(this->prepend(one1), QString("2"));
        EXPECT_TRUE(dp->items.isEmpty());
        ASSERT_EQ(dp2->items.size(), 1);
        EXPECT_EQ(dp2->items.first(), one1);
        ASSERT_EQ(types.size(), 2);
        EXPECT_EQ(types.first(), QString("1"));
        EXPECT_EQ(types.last(), QString("2"));

        dp->items.append(one2);
        EXPECT_EQ(this->prepend(one2), QString("2"));
        ASSERT_EQ(dp2->items.size(), 2);
        EXPECT_EQ(dp2->items.first(), one2);
        EXPECT_EQ(dp2->items.last(), one1);
    }
}

TEST_F(TestFileClassifier2, remove)
{
    initDP();
    dp->items.append(one1);
    dp2->items.append(two1);

    EXPECT_TRUE(this->remove(test).isEmpty());
    EXPECT_TRUE(types.isEmpty());
    EXPECT_EQ(dp->items.size(), 1);
    EXPECT_EQ(dp2->items.size(), 1);

    types.clear();
    EXPECT_TRUE(this->remove(one2).isEmpty());
    EXPECT_TRUE(types.isEmpty());
    EXPECT_EQ(dp->items.size(), 1);
    EXPECT_EQ(dp2->items.size(), 1);

    types.clear();
    EXPECT_EQ(this->remove(one1), QString("1"));
    EXPECT_TRUE(dp->items.isEmpty());
    EXPECT_EQ(dp2->items.size(), 1);
    ASSERT_EQ(types.size(), 1);
    EXPECT_EQ(types.first(), QString("1"));

    types.clear();
    EXPECT_EQ(this->remove(two1), QString("2"));
    EXPECT_TRUE(dp->items.isEmpty());
    EXPECT_TRUE(dp2->items.isEmpty());
    ASSERT_EQ(types.size(), 1);
    EXPECT_EQ(types.first(), QString("2"));
}

TEST_F(TestFileClassifier2, change)
{
    initDP();
    EXPECT_TRUE(this->change(one1).isEmpty());
    EXPECT_TRUE(types.isEmpty());

    dp->items.append(one1);
    types.clear();
    EXPECT_TRUE(this->change(one1).isEmpty());
    EXPECT_TRUE(types.isEmpty());

    stub.set_lamda(VADDR(TestFileClassifier, classify), [this](TestFileClassifier *self, const QUrl &url) {
        EXPECT_EQ(self->ids.key(url.fileName().left(3)), QString("1"));
        return QString("2");
    });

    types.clear();
    EXPECT_EQ(this->change(one1), QString("2"));
    EXPECT_TRUE(dp->items.isEmpty());
    ASSERT_EQ(dp2->items.size(), 1);
    EXPECT_EQ(dp2->items.first(), one1);
    ASSERT_EQ(types.size(), 2);
    EXPECT_EQ(types.first(), QString("1"));
    EXPECT_EQ(types.last(), QString("2"));
}
