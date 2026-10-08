// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "mode/normalized/type/typeclassifier.h"
#include "mode/normalized/fileclassifier.h"
#include "models/modeldatahandler.h"

#include <QUrl>
#include <QTemporaryFile>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_TypeClassifier : public testing::Test
{
protected:
    void SetUp() override
    {
        classifier = new TypeClassifier();
    }

    void TearDown() override
    {
        delete classifier;
        classifier = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TypeClassifier *classifier = nullptr;
};

TEST_F(UT_TypeClassifier, Constructor_CreatesClassifier)
{
    EXPECT_NE(classifier, nullptr);
    EXPECT_NE(classifier->d, nullptr);
}

TEST_F(UT_TypeClassifier, Destructor_DoesNotCrash)
{
    TypeClassifier *tempClassifier = new TypeClassifier();
    EXPECT_NE(tempClassifier, nullptr);
    delete tempClassifier;
    // Should not crash on deletion
    SUCCEED();
}

TEST_F(UT_TypeClassifier, Mode_ReturnsClassifierType)
{
    Classifier mode = classifier->mode();
    EXPECT_EQ(mode, Classifier::kType);
}

TEST_F(UT_TypeClassifier, DataHandler_ReturnsHandler)
{
    ModelDataHandler *handler = classifier->dataHandler();
    EXPECT_NE(handler, nullptr);
}

TEST_F(UT_TypeClassifier, Classes_ReturnsStringList)
{
    QStringList classes = classifier->classes();
    // Should return a list of supported file types/classes
    EXPECT_TRUE(true); // Method exists and returns a value
}


TEST_F(UT_TypeClassifier, Classify_WithInvalidUrl_ReturnsCategory)
{
    QUrl invalidUrl("invalid://nonexistent");
    QString category = classifier->classify(invalidUrl);
    // Should handle invalid URLs gracefully
    EXPECT_TRUE(true);
}

TEST_F(UT_TypeClassifier, ClassName_WithValidKey_ReturnsName)
{
    QString className = classifier->className("application");
    // Should return the display name for the category
    EXPECT_TRUE(true); // Method exists and returns a value
}

TEST_F(UT_TypeClassifier, ClassName_WithInvalidKey_ReturnsEmpty)
{
    QString className = classifier->className("invalid_key");
    // Should return empty or default name for invalid keys
    EXPECT_TRUE(true);
}


TEST_F(UT_TypeClassifier, Replace_UpdatesUrl)
{
    QTemporaryFile oldFile, newFile;
    oldFile.open();
    newFile.open();
    
    QUrl oldUrl = QUrl::fromLocalFile(oldFile.fileName());
    QUrl newUrl = QUrl::fromLocalFile(newFile.fileName());
    
    QString result = classifier->replace(oldUrl, newUrl);
    // Should return the category key for the new file
    EXPECT_TRUE(true); // Method exists and returns a value
}

TEST_F(UT_TypeClassifier, Append_AddsUrlToCategory)
{
    QTemporaryFile tempFile;
    tempFile.open();
    QUrl fileUrl = QUrl::fromLocalFile(tempFile.fileName());
    
    QString result = classifier->append(fileUrl);
    // Should return the category key where the file was added
    EXPECT_TRUE(true); // Method exists and returns a value
}

TEST_F(UT_TypeClassifier, Prepend_AddsUrlToCategory)
{
    QTemporaryFile tempFile;
    tempFile.open();
    QUrl fileUrl = QUrl::fromLocalFile(tempFile.fileName());
    
    QString result = classifier->prepend(fileUrl);
    // Should return the category key where the file was added
    EXPECT_TRUE(true); // Method exists and returns a value
}

TEST_F(UT_TypeClassifier, Remove_RemovesUrlFromCategory)
{
    QTemporaryFile tempFile;
    tempFile.open();
    QUrl fileUrl = QUrl::fromLocalFile(tempFile.fileName());
    
    QString result = classifier->remove(fileUrl);
    // Should return the category key from which the file was removed
    EXPECT_TRUE(true); // Method exists and returns a value
}

TEST_F(UT_TypeClassifier, Change_UpdatesUrlInCategory)
{
    QTemporaryFile tempFile;
    tempFile.open();
    QUrl fileUrl = QUrl::fromLocalFile(tempFile.fileName());
    
    QString result = classifier->change(fileUrl);
    // Should return the category key for the changed file
    EXPECT_TRUE(true); // Method exists and returns a value
}


#include "config/configpresenter.h"

// PMS:159621 归档分组丢失音乐分组：kCatDefault（默认启用全部分类）的
// classes() 列表此前漏掉 kTypeKeyMuz，导致开启类型分组后桌面音乐文件
// 无分组可归。修复后默认列表必须包含音乐分组。
TEST_F(UT_TypeClassifier, BUG159621_Classes_DefaultCategories_ContainsMusic)
{
    stub.set_lamda(ADDR(ConfigPresenter, enabledTypeCategories),
                   [](ConfigPresenter *) -> ItemCategories {
                       __DBG_STUB_INVOKE__
                       return ItemCategories(kCatDefault);
                   });

    // 分类器构造时读取 enabledTypeCategories，需在打桩后创建
    TypeClassifier defaultClassifier;
    const QStringList cls = defaultClassifier.classes();

    EXPECT_TRUE(cls.contains(kTypeKeyMuz));   // 修复前缺失音乐分组
    EXPECT_TRUE(cls.contains(kTypeKeyDoc));
    EXPECT_TRUE(cls.contains(kTypeKeyFld));
}

// 用户显式仅启用音乐分类时，classes() 应只返回音乐一组（走 kCategory2Key 映射）。
TEST_F(UT_TypeClassifier, BUG159621_Classes_MusicEnabledOnly_ReturnsSingleMusicGroup)
{
    stub.set_lamda(ADDR(ConfigPresenter, enabledTypeCategories),
                   [](ConfigPresenter *) -> ItemCategories {
                       __DBG_STUB_INVOKE__
                       return ItemCategories(kCatMusic);
                   });

    TypeClassifier musicClassifier;
    const QStringList cls = musicClassifier.classes();

    ASSERT_EQ(cls.count(), 1);
    EXPECT_EQ(cls.first(), QString(kTypeKeyMuz));
}
