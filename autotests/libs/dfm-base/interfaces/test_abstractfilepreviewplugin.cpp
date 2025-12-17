// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_abstractfilepreviewplugin.cpp - AbstractFilePreviewPlugin class unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QTest>
#include <QApplication>
#include <memory>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>
#include <dfm-base/interfaces/abstractbasepreview.h>

DFMBASE_USE_NAMESPACE

/**
 * @brief Concrete implementation of AbstractBasePreview for testing plugin
 */
class PluginTestPreview : public AbstractBasePreview
{
    Q_OBJECT
public:
    explicit PluginTestPreview(const QString &type, QObject *parent = nullptr)
        : AbstractBasePreview(parent), previewType(type)
    {
    }
    
    bool setFileUrl(const QUrl &url) override
    {
        currentUrl = url;
        return true;
    }
    
    QUrl fileUrl() const override
    {
        return currentUrl;
    }
    
    QWidget *contentWidget() const override
    {
        return nullptr;  // Minimal implementation for testing
    }
    
    QString getPreviewType() const { return previewType; }
    
private:
    QString previewType;
    QUrl currentUrl;
};

/**
 * @brief Concrete implementation of AbstractFilePreviewPlugin for testing
 */
class TestFilePreviewPlugin : public AbstractFilePreviewPlugin
{
    Q_OBJECT
public:
    explicit TestFilePreviewPlugin(QObject *parent = nullptr)
        : AbstractFilePreviewPlugin(parent)
    {
        supportedTypes << "image" << "text" << "video";
    }
    
    AbstractBasePreview *create(const QString &key) override
    {
        createCount++;
        
        if (supportedTypes.contains(key)) {
            return new PluginTestPreview(key);
        }
        return nullptr;
    }
    
    // Test helper methods
    int getCreateCount() const { return createCount; }
    bool isSupportedType(const QString &type) const { return supportedTypes.contains(type); }
    void addSupportedType(const QString &type) { supportedTypes << type; }
    
private:
    QStringList supportedTypes;
    int createCount { 0 };
};

/**
 * @brief Another concrete implementation for testing multiple plugins
 */
class AlternativeTestPlugin : public AbstractFilePreviewPlugin
{
    Q_OBJECT
public:
    explicit AlternativeTestPlugin(QObject *parent = nullptr)
        : AbstractFilePreviewPlugin(parent)
    {
    }
    
    AbstractBasePreview *create(const QString &key) override
    {
        createCount++;
        return new PluginTestPreview("alternative:" + key);
    }
    
    int getCreateCount() const { return createCount; }
    
private:
    int createCount { 0 };
};

/**
 * @brief AbstractFilePreviewPlugin class unit tests
 *
 * Test scope:
 * 1. Construction and initialization
 * 2. Pure virtual function implementation (create)
 * 3. Plugin factory behavior
 * 4. Preview instance creation and management
 * 5. Multiple plugin instances
 * 6. Type handling and validation
 * 7. Memory management
 */
class AbstractFilePreviewPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();
        
        // Set up test application if not exists
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { const_cast<char*>("test_abstractfilepreviewplugin") };
            app = std::make_unique<QApplication>(argc, argv);
        }
        
        // Create test plugin instance
        plugin = std::make_unique<TestFilePreviewPlugin>();
    }
    
    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
        plugin.reset();
        app.reset();
    }
    
    // Test stubbing utility
    stub_ext::StubExt stub;
    std::unique_ptr<QApplication> app;
    std::unique_ptr<TestFilePreviewPlugin> plugin;
};

TEST_F(AbstractFilePreviewPluginTest, Constructor)
{
    // Test basic construction
    EXPECT_NE(plugin.get(), nullptr);
    EXPECT_EQ(plugin->parent(), nullptr);
    
    // Test construction with parent
    QObject parent;
    auto childPlugin = std::make_unique<TestFilePreviewPlugin>(&parent);
    EXPECT_EQ(childPlugin->parent(), &parent);
}

TEST_F(AbstractFilePreviewPluginTest, CreateValidPreview)
{
    // Test creating valid preview instances
    auto *imagePreview = plugin->create("image");
    EXPECT_NE(imagePreview, nullptr);
    EXPECT_EQ(plugin->getCreateCount(), 1);
    delete imagePreview;
    
    auto *textPreview = plugin->create("text");
    EXPECT_NE(textPreview, nullptr);
    EXPECT_EQ(plugin->getCreateCount(), 2);
    delete textPreview;
    
    auto *videoPreview = plugin->create("video");
    EXPECT_NE(videoPreview, nullptr);
    EXPECT_EQ(plugin->getCreateCount(), 3);
    delete videoPreview;
}

TEST_F(AbstractFilePreviewPluginTest, CreateInvalidPreview)
{
    // Test creating preview with unsupported type
    auto *invalidPreview = plugin->create("unsupported_type");
    EXPECT_EQ(invalidPreview, nullptr);
    
    // Create count should not increase for invalid types
    EXPECT_EQ(plugin->getCreateCount(), 0);
    
    // Test with empty string
    auto *emptyPreview = plugin->create("");
    EXPECT_EQ(emptyPreview, nullptr);
    EXPECT_EQ(plugin->getCreateCount(), 0);
}

TEST_F(AbstractFilePreviewPluginTest, PreviewInstanceFunctionality)
{
    // Create a preview instance
    auto *preview = plugin->create("image");
    ASSERT_NE(preview, nullptr);
    
    // Test if it's the correct type
    auto *testPreview = qobject_cast<PluginTestPreview*>(preview);
    EXPECT_NE(testPreview, nullptr);
    EXPECT_EQ(testPreview->getPreviewType(), "image");
    
    // Test preview functionality
    QUrl testUrl("file:///test/image.jpg");
    EXPECT_TRUE(preview->setFileUrl(testUrl));
    EXPECT_EQ(preview->fileUrl(), testUrl);
    
    delete preview;
}

TEST_F(AbstractFilePreviewPluginTest, MultiplePreviewCreation)
{
    // Test creating multiple previews of the same type
    QList<AbstractBasePreview*> previews;
    
    for (int i = 0; i < 5; ++i) {
        auto *preview = plugin->create("text");
        EXPECT_NE(preview, nullptr);
        previews.append(preview);
    }
    
    // Verify all previews were created
    EXPECT_EQ(previews.size(), 5);
    EXPECT_EQ(plugin->getCreateCount(), 5);
    
    // Clean up
    for (auto *preview : previews) {
        delete preview;
    }
}

TEST_F(AbstractFilePreviewPluginTest, DynamicTypeSupport)
{
    // Test adding new supported types
    EXPECT_FALSE(plugin->isSupportedType("audio"));
    
    plugin->addSupportedType("audio");
    EXPECT_TRUE(plugin->isSupportedType("audio"));
    
    // Create preview with newly added type
    auto *audioPreview = plugin->create("audio");
    EXPECT_NE(audioPreview, nullptr);
    
    auto *testPreview = qobject_cast<PluginTestPreview*>(audioPreview);
    EXPECT_EQ(testPreview->getPreviewType(), "audio");
    
    delete audioPreview;
}

TEST_F(AbstractFilePreviewPluginTest, MultiplePlugins)
{
    // Test creating multiple plugin instances
    auto plugin1 = std::make_unique<TestFilePreviewPlugin>();
    auto plugin2 = std::make_unique<AlternativeTestPlugin>();
    
    // Test that plugins work independently
    auto *preview1 = plugin1->create("image");
    auto *preview2 = plugin2->create("image");
    
    EXPECT_NE(preview1, nullptr);
    EXPECT_NE(preview2, nullptr);
    EXPECT_EQ(plugin1->getCreateCount(), 1);
    EXPECT_EQ(plugin2->getCreateCount(), 1);
    
    // Test different behaviors
    auto *test1 = qobject_cast<PluginTestPreview*>(preview1);
    auto *test2 = qobject_cast<PluginTestPreview*>(preview2);
    
    EXPECT_EQ(test1->getPreviewType(), "image");
    EXPECT_EQ(test2->getPreviewType(), "alternative:image");
    
    delete preview1;
    delete preview2;
}

TEST_F(AbstractFilePreviewPluginTest, EdgeCases)
{
    // Test with case sensitivity
    auto *lowerCase = plugin->create("image");
    auto *upperCase = plugin->create("IMAGE");
    
    EXPECT_NE(lowerCase, nullptr);
    EXPECT_EQ(upperCase, nullptr);  // Case sensitive
    
    delete lowerCase;
    
    // Test with special characters in key
    plugin->addSupportedType("test-with-dash");
    plugin->addSupportedType("test_with_underscore");
    plugin->addSupportedType("test.with.dots");
    
    auto *dashPreview = plugin->create("test-with-dash");
    auto *underscorePreview = plugin->create("test_with_underscore");
    auto *dotPreview = plugin->create("test.with.dots");
    
    EXPECT_NE(dashPreview, nullptr);
    EXPECT_NE(underscorePreview, nullptr);
    EXPECT_NE(dotPreview, nullptr);
    
    delete dashPreview;
    delete underscorePreview;
    delete dotPreview;
}

TEST_F(AbstractFilePreviewPluginTest, MemoryManagement)
{
    // Test memory leak prevention
    QPointer<AbstractBasePreview> previewPtr;
    
    {
        auto *preview = plugin->create("text");
        previewPtr = preview;
        EXPECT_NE(preview, nullptr);
        EXPECT_NE(previewPtr.data(), nullptr);
        
        // Don't delete here - should be cleaned up automatically in real usage
        // In test, we need to delete to prevent memory leak
        delete preview;
    }
    
    // After deletion, pointer should be null
    EXPECT_EQ(previewPtr.data(), nullptr);
}

TEST_F(AbstractFilePreviewPluginTest, FactoryInterfaceId)
{
    // Test that the interface ID is defined correctly
    // This is a compile-time check for the interface
    EXPECT_STREQ(FilePreviewFactoryInterface_iid, 
                 "com.deepin.filemanager.FilePreviewFactoryInterface_iid");
}

#include "test_abstractfilepreviewplugin.moc"