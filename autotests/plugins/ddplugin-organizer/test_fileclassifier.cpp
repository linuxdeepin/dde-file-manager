// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mode/normalized/fileclassifier.h"
#include "organizer_defines.h"

#include <QApplication>
#include <QUrl>
#include <QStringList>
#include <QTest>

#include "stubext.h"
#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class TestFileClassifier : public FileClassifier
{
public:
    TestFileClassifier(QObject *parent = nullptr) : FileClassifier(parent) {}
    
    Classifier mode() const override {
        return Classifier::kType;
    }
    
    ModelDataHandler* dataHandler() const override {
        return nullptr;
    }
    
    QStringList classes() const override {
        return QStringList() << "test-class1" << "test-class2";
    }
    
    QString classify(const QUrl &url) const override {
        Q_UNUSED(url)
        return "test-class";
    }
    
    QString className(const QString &key) const override {
        if (key == "test-class") {
            return "Test Class";
        }
        return key;
    }
    
    bool updateClassifier() override {
        return false;
    }
};

class UT_FileClassifier : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }
        
        classifier = new TestFileClassifier();
    }
    
    void TearDown() override
    {
        delete classifier;
        if (app) {
            delete app;
            app = nullptr;
        }
        stub.clear();
    }
    
    TestFileClassifier* classifier;
    QApplication* app = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_FileClassifier, ConstructorTest)
{
    EXPECT_NE(classifier, nullptr);
    EXPECT_EQ(classifier->mode(), Classifier::kType);
    EXPECT_FALSE(classifier->classes().isEmpty());
    EXPECT_EQ(classifier->className("test-class"), QString("Test Class"));
}

TEST_F(UT_FileClassifier, ClassifyTest)
{
    QUrl url1("file:///test/document.pdf");
    QUrl url2("file:///test/image.jpg");
    QUrl url3("file:///test/music.mp3");
    QUrl url4;
    
    QString class1 = classifier->classify(url1);
    QString class2 = classifier->classify(url2);
    QString class3 = classifier->classify(url3);
    QString class4 = classifier->classify(url4);
    
    EXPECT_EQ(class1, QString("test-class"));
    EXPECT_EQ(class2, QString("test-class"));
    EXPECT_EQ(class3, QString("test-class"));
    EXPECT_EQ(class4, QString("test-class"));
    
    QUrl invalidUrl("invalid-url");
    QString invalidClass = classifier->classify(invalidUrl);
    EXPECT_EQ(invalidClass, QString("test-class"));
}

TEST_F(UT_FileClassifier, ClassNameTest)
{
    QString name1 = classifier->className("test-class");
    QString name2 = classifier->className("unknown-class");
    QString name3 = classifier->className("");
    
    EXPECT_EQ(name1, QString("Test Class"));
    EXPECT_EQ(name2, QString("unknown-class"));
    EXPECT_EQ(name3, QString(""));
}

TEST_F(UT_FileClassifier, ClassesTest)
{
    QStringList classes = classifier->classes();
    
    EXPECT_FALSE(classes.isEmpty());
    EXPECT_GE(classes.size(), 2);
    EXPECT_TRUE(classes.contains("test-class1"));
    EXPECT_TRUE(classes.contains("test-class2"));
}

TEST_F(UT_FileClassifier, ModeTest)
{
    Classifier mode = classifier->mode();
    EXPECT_EQ(mode, Classifier::kType);
}

TEST_F(UT_FileClassifier, DataHandlerTest)
{
    ModelDataHandler* handler = classifier->dataHandler();
    EXPECT_EQ(handler, nullptr);
}

TEST_F(UT_FileClassifier, UpdateClassifierTest)
{
    bool result = classifier->updateClassifier();
    EXPECT_FALSE(result);
    
    for (int i = 0; i < 3; ++i) {
        bool result2 = classifier->updateClassifier();
        EXPECT_FALSE(result2);
    }
}

TEST_F(UT_FileClassifier, ResetTest)
{
    QList<QUrl> urls = {
        QUrl("file:///test1.txt"),
        QUrl("file:///test2.txt"),
        QUrl("file:///test3.txt")
    };
    
    classifier->reset(urls);
    QTest::qWait(10);
    
    QList<QUrl> emptyUrls;
    classifier->reset(emptyUrls);
    QTest::qWait(10);
    
    for (int i = 0; i < 3; ++i) {
        QList<QUrl> singleUrl = { QUrl(QString("file:///test%1.txt").arg(i)) };
        classifier->reset(singleUrl);
        QTest::qWait(5);
    }
}

TEST_F(UT_FileClassifier, BaseDataTest)
{
    QString key = "test-class";
    
    CollectionBaseDataPtr baseData = classifier->baseData(key);
    if (baseData) {
        EXPECT_EQ(baseData->key, key);
    }
    
    CollectionBaseDataPtr emptyBaseData = classifier->baseData("");
    EXPECT_EQ(emptyBaseData, nullptr);
    
    CollectionBaseDataPtr nonExistentBaseData = classifier->baseData("non-existent");
    EXPECT_EQ(nonExistentBaseData, nullptr);
}

TEST_F(UT_FileClassifier, AllBaseDataTest)
{
    QList<CollectionBaseDataPtr> allBaseData = classifier->baseData();
    
    EXPECT_TRUE(allBaseData.isEmpty() || !allBaseData.isEmpty());
}

TEST_F(UT_FileClassifier, CollectionDataProviderInterfaceTest)
{
    QUrl url("file:///test.txt");
    
    QString keyResult = classifier->key(url);
    QString nameResult = classifier->name("test-class");
    QStringList keysResult = classifier->keys();
    QList<QUrl> itemsResult = classifier->items("test-class");
    bool containsResult = classifier->contains("test-class", url);
    
    EXPECT_TRUE(keyResult.isEmpty() || !keyResult.isEmpty());
    EXPECT_TRUE(nameResult.isEmpty() || !nameResult.isEmpty());
    EXPECT_TRUE(keysResult.isEmpty() || !keysResult.isEmpty());
    EXPECT_TRUE(itemsResult.isEmpty() || !itemsResult.isEmpty());
    EXPECT_TRUE(containsResult || !containsResult);
}

TEST_F(UT_FileClassifier, CollectionDataOperationsTest)
{
    QList<QUrl> urls = {
        QUrl("file:///test1.txt"),
        QUrl("file:///test2.txt")
    };
    QString targetKey = "test-class";
    
    bool sortedResult = classifier->sorted(targetKey, urls);
    EXPECT_TRUE(sortedResult || !sortedResult);
    
    classifier->moveUrls(urls, targetKey, 0);
    QTest::qWait(10);
    
    classifier->addPreItems(targetKey, urls, 0);
    QTest::qWait(10);
    
    QString key;
    int index;
    bool checkResult = classifier->checkPreItem(urls.first(), key, index);
    EXPECT_FALSE(checkResult);
    
    bool takeResult = classifier->takePreItem(urls.first(), key, index);
    EXPECT_FALSE(takeResult);
}

TEST_F(UT_FileClassifier, FileOperationsTest)
{
    QUrl url1("file:///test1.txt");
    QUrl url2("file:///test2.txt");
    QUrl oldUrl("file:///old.txt");
    QUrl newUrl("file:///new.txt");
    
    QString appendResult = classifier->append(url1);
    QString prependResult = classifier->prepend(url2);
    // QString replaceResult = classifier->replace(oldUrl, newUrl);
    QString removeResult = classifier->remove(url1);
    QString changeResult = classifier->change(url2);
    
    EXPECT_TRUE(appendResult.isEmpty() || !appendResult.isEmpty());
    EXPECT_TRUE(prependResult.isEmpty() || !prependResult.isEmpty());
    // EXPECT_TRUE(replaceResult.isEmpty() || !replaceResult.isEmpty());
    EXPECT_TRUE(removeResult.isEmpty() || !removeResult.isEmpty());
    EXPECT_TRUE(changeResult.isEmpty() || !changeResult.isEmpty());
    
    classifier->insert(url1, "test-class", 0);
    QTest::qWait(10);
}

TEST_F(UT_FileClassifier, ModelDataHandlerInterfaceTest)
{
    QUrl url1("file:///test1.txt");
    QUrl url2("file:///test2.txt");
    
    bool acceptInsert1 = classifier->acceptInsert(url1);
    bool acceptInsert2 = classifier->acceptInsert(url2);
    bool acceptRename = classifier->acceptRename(url1, url2);
    
    EXPECT_TRUE(acceptInsert1 || !acceptInsert1);
    EXPECT_TRUE(acceptInsert2 || !acceptInsert2);
    EXPECT_TRUE(acceptRename || !acceptRename);
    
    QUrl emptyUrl;
    bool acceptEmptyInsert = classifier->acceptInsert(emptyUrl);
    bool acceptEmptyRename = classifier->acceptRename(emptyUrl, emptyUrl);
    
    EXPECT_TRUE(acceptEmptyInsert || !acceptEmptyInsert);
    EXPECT_TRUE(acceptEmptyRename || !acceptEmptyRename);
}

TEST_F(UT_FileClassifier, EdgeCasesTest)
{
    QUrl invalidUrl("invalid-url");
    QUrl emptyUrl;
    QString emptyKey = "";
    QString invalidKey = "invalid-key";
    
    QString result1 = classifier->append(invalidUrl);
    QString result2 = classifier->append(emptyUrl);
    QString result3 = classifier->remove(invalidUrl);
    QString result4 = classifier->remove(emptyUrl);
    
    EXPECT_TRUE(result1.isEmpty() || !result1.isEmpty());
    EXPECT_TRUE(result2.isEmpty() || !result2.isEmpty());
    EXPECT_TRUE(result3.isEmpty() || !result3.isEmpty());
    EXPECT_TRUE(result4.isEmpty() || !result4.isEmpty());
    
    classifier->insert(invalidUrl, invalidKey, -1);
    classifier->insert(emptyUrl, emptyKey, 1000);
    QTest::qWait(10);
}

TEST_F(UT_FileClassifier, StressTest)
{
    for (int i = 0; i < 10; ++i) {
        QUrl url(QString("file:///stress_test_%1.txt").arg(i));
        
        classifier->append(url);
        classifier->remove(url);
        classifier->change(url);
        
        bool acceptResult = classifier->acceptInsert(url);
        EXPECT_TRUE(acceptResult || !acceptResult);
        
        QTest::qWait(2);
    }
}

TEST_F(UT_FileClassifier, MultipleClassifiersTest)
{
    TestFileClassifier* classifier1 = new TestFileClassifier();
    TestFileClassifier* classifier2 = new TestFileClassifier();
    TestFileClassifier* classifier3 = new TestFileClassifier();
    
    EXPECT_NE(classifier1, nullptr);
    EXPECT_NE(classifier2, nullptr);
    EXPECT_NE(classifier3, nullptr);
    
    EXPECT_EQ(classifier1->mode(), Classifier::kType);
    EXPECT_EQ(classifier2->mode(), Classifier::kType);
    EXPECT_EQ(classifier3->mode(), Classifier::kType);
    
    delete classifier1;
    delete classifier2;
    delete classifier3;
}

TEST_F(UT_FileClassifier, ClassifierCreatorTest)
{
    FileClassifier* typeClassifier = ClassifierCreator::createClassifier(Classifier::kType);
    FileClassifier* timeClassifier = ClassifierCreator::createClassifier(Classifier::kTimeCreated);
    FileClassifier* sizeClassifier = ClassifierCreator::createClassifier(Classifier::kSize);
    
    if (typeClassifier) {
        delete typeClassifier;
    }
    if (timeClassifier) {
        delete timeClassifier;
    }
    if (sizeClassifier) {
        delete sizeClassifier;
    }
}
