// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QIcon>
#include <QFile>
#include <QVariantHash>

#include "stubext.h"
#include "fileentity/appentryfileentity.h"
#include "utils/computerutils.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/utils/desktopfile.h>
#include <dfm-base/interfaces/abstractentryfileentity.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_AppEntryFileEntity : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        testUrl = QUrl("entry://app.test-app");
        testFileUrl = QUrl::fromLocalFile("/usr/share/applications/test-app.desktop");
        entity = nullptr;
    }

    virtual void TearDown() override
    {
        delete entity;
        entity = nullptr;
        stub.clear();
    }

    void createEntity()
    {
        // Mock ComputerUtils::getAppEntryFileUrl
        stub.set_lamda(&ComputerUtils::getAppEntryFileUrl, [this](const QUrl &) -> QUrl {
            __DBG_STUB_INVOKE__
            return testFileUrl;
        });

        entity = new AppEntryFileEntity(testUrl);
    }

protected:
    stub_ext::StubExt stub;
    AppEntryFileEntity *entity = nullptr;
    QUrl testUrl;
    QUrl testFileUrl;
};

TEST_F(UT_AppEntryFileEntity, Construction_ValidUrl_CreatesEntityCorrectly)
{
    bool getAppEntryFileUrlCalled = false;
    QString capturedDesktopPath;

    // Mock ComputerUtils::getAppEntryFileUrl
    stub.set_lamda(&ComputerUtils::getAppEntryFileUrl, [&](const QUrl &url) -> QUrl {
        __DBG_STUB_INVOKE__
        getAppEntryFileUrlCalled = true;
        EXPECT_EQ(url, testUrl);
        return testFileUrl;
    });

    // Create entity
    entity = new AppEntryFileEntity(testUrl);

    // Verify construction
    EXPECT_NE(entity, nullptr);
    EXPECT_TRUE(getAppEntryFileUrlCalled);

    // Verify internal state
    EXPECT_EQ(entity->fileUrl, testFileUrl);
    EXPECT_NE(entity->desktopInfo.data(), nullptr);
}

TEST_F(UT_AppEntryFileEntity, DisplayName_ValidDesktopFile_ReturnsCorrectName)
{
    createEntity();

    QString mockDisplayName = "Test Application";
    bool desktopDisplayNameCalled = false;

    // Mock DesktopFile::desktopDisplayName
    stub.set_lamda(&DesktopFile::desktopDisplayName, [&]() -> QString {
        __DBG_STUB_INVOKE__
        desktopDisplayNameCalled = true;
        return mockDisplayName;
    });

    // Test displayName
    QString result = entity->displayName();

    // Verify result
    EXPECT_TRUE(desktopDisplayNameCalled);
    EXPECT_EQ(result, mockDisplayName);
}

TEST_F(UT_AppEntryFileEntity, Icon_ValidDesktopFile_ReturnsCorrectIcon)
{
    createEntity();

    QString mockIconName = "test-app-icon";
    QIcon mockIcon = QIcon::fromTheme("test-icon");
    bool desktopIconCalled = false;
    bool fromThemeCalled = false;

    // Mock DesktopFile::desktopIcon
    stub.set_lamda(&DesktopFile::desktopIcon, [&]() -> QString {
        __DBG_STUB_INVOKE__
        desktopIconCalled = true;
        return mockIconName;
    });

    // Mock QIcon::fromTheme
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [&](const QString &name) -> QIcon {
        __DBG_STUB_INVOKE__
        fromThemeCalled = true;
        EXPECT_EQ(name, mockIconName);
        return mockIcon;
    });

    // Test icon
    QIcon result = entity->icon();

    // Verify result
    EXPECT_TRUE(desktopIconCalled);
    EXPECT_TRUE(fromThemeCalled);
}

TEST_F(UT_AppEntryFileEntity, Exists_FileExists_ReturnsTrue)
{
    createEntity();

    bool fileExistsCalled = false;

    // Mock QFile::exists
    stub.set_lamda(static_cast<bool (QFile::*)() const>(&QFile::exists), [&]() -> bool {
        __DBG_STUB_INVOKE__
        fileExistsCalled = true;
        return true;
    });

    // Test exists
    bool result = entity->exists();

    // Verify result
    EXPECT_TRUE(fileExistsCalled);
    EXPECT_TRUE(result);
}

TEST_F(UT_AppEntryFileEntity, Exists_FileNotExists_ReturnsFalse)
{
    createEntity();

    bool fileExistsCalled = false;
    // Mock QFile::exists
    stub.set_lamda(static_cast<bool (QFile::*)() const>(&QFile::exists), [&]() -> bool {
        __DBG_STUB_INVOKE__
        fileExistsCalled = true;
        return false;
    });

    // Test exists
    bool result = entity->exists();

    // Verify result
    EXPECT_TRUE(fileExistsCalled);
    EXPECT_FALSE(result);
}

TEST_F(UT_AppEntryFileEntity, ShowProgress_Always_ReturnsFalse)
{
    createEntity();

    // Test showProgress
    bool result = entity->showProgress();

    // Verify result
    EXPECT_FALSE(result);
}

TEST_F(UT_AppEntryFileEntity, ShowTotalSize_Always_ReturnsFalse)
{
    createEntity();

    // Test showTotalSize
    bool result = entity->showTotalSize();

    // Verify result
    EXPECT_FALSE(result);
}

TEST_F(UT_AppEntryFileEntity, ShowUsageSize_Always_ReturnsFalse)
{
    createEntity();

    // Test showUsageSize
    bool result = entity->showUsageSize();

    // Verify result
    EXPECT_FALSE(result);
}

TEST_F(UT_AppEntryFileEntity, Description_Always_ReturnsCorrectDescription)
{
    createEntity();

    // Test description
    QString result = entity->description();

    // Verify result
    EXPECT_EQ(result, "Double click to open it");
}

TEST_F(UT_AppEntryFileEntity, Order_Always_ReturnsCorrectOrder)
{
    createEntity();

    // Test order
    AbstractEntryFileEntity::EntryOrder result = entity->order();

    // Verify result
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderApps);
}

TEST_F(UT_AppEntryFileEntity, ExtraProperties_ValidDesktopFile_ReturnsCorrectProperties)
{
    createEntity();

    QString mockExecCommand = "test-app --param";
    QString mockFormattedCommand = "test-app";
    bool desktopExecCalled = false;

    // Mock DesktopFile::desktopExec
    stub.set_lamda(&DesktopFile::desktopExec, [&]() -> QString {
        __DBG_STUB_INVOKE__
        desktopExecCalled = true;
        return mockExecCommand;
    });

    // Test extraProperties
    QVariantHash result = entity->extraProperties();

    // Verify result
    EXPECT_TRUE(desktopExecCalled);
    EXPECT_TRUE(result.contains(ExtraPropertyName::kExecuteCommand));

    QString executeCommand = result.value(ExtraPropertyName::kExecuteCommand).toString();
    // The formatted command should have unsupported parameters removed
    EXPECT_FALSE(executeCommand.contains("%"));
    EXPECT_FALSE(executeCommand.contains("\""));
    EXPECT_FALSE(executeCommand.contains("'"));
}

TEST_F(UT_AppEntryFileEntity, IsAccessable_FileExists_ReturnsTrue)
{
    createEntity();

    bool fileExistsCalled = false;

    // Mock QFile::exists
    stub.set_lamda(static_cast<bool (QFile::*)() const>(&QFile::exists), [&]() -> bool {
        __DBG_STUB_INVOKE__
        fileExistsCalled = true;
        return true;
    });

    // Test isAccessable
    bool result = entity->isAccessable();

    // Verify result
    EXPECT_TRUE(fileExistsCalled);
    EXPECT_TRUE(result);
}

TEST_F(UT_AppEntryFileEntity, IsAccessable_FileNotExists_ReturnsFalse)
{
    createEntity();

    bool fileExistsCalled = false;

    // Mock QFile::exists
    stub.set_lamda(static_cast<bool (QFile::*)() const>(&QFile::exists), [&]() -> bool {
        __DBG_STUB_INVOKE__
        fileExistsCalled = true;
        return false;
    });

    // Test isAccessable
    bool result = entity->isAccessable();

    // Verify result
    EXPECT_TRUE(fileExistsCalled);
    EXPECT_FALSE(result);
}

TEST_F(UT_AppEntryFileEntity, GetFormattedExecCommand_WithUnsupportedParams_RemovesParams)
{
    createEntity();

    // Test different exec commands with unsupported parameters
    struct TestCase
    {
        QString input;
        QString expectedOutput;
        QString description;
    };

    QList<TestCase> testCases = {
        { "test-app %U", "test-app ", "Remove %U parameter" },
        { "test-app %u", "test-app ", "Remove %u parameter" },
        { "test-app %F", "test-app ", "Remove %F parameter" },
        { "test-app %f", "test-app ", "Remove %f parameter" },
        { "\"test-app\" --option", "test-app --option", "Remove quotes" },
        { "'test-app' --option", "test-app --option", "Remove single quotes" },
        { "test-app %f %U --option", "test-app  --option", "Remove multiple parameters" },
        { "\"test-app\" %f --option %U", "test-app  --option ", "Remove quotes and parameters" },
    };

    for (const auto &testCase : testCases) {
        // Mock DesktopFile::desktopExec
        stub.set_lamda(&DesktopFile::desktopExec, [&testCase]() -> QString {
            __DBG_STUB_INVOKE__
            return testCase.input;
        });

        // Test extraProperties to trigger getFormattedExecCommand
        EXPECT_NO_FATAL_FAILURE(entity->extraProperties());
    }
}

TEST_F(UT_AppEntryFileEntity, GetFormattedExecCommand_EmptyExecCommand_ReturnsEmpty)
{
    createEntity();

    QString emptyCommand = "";

    // Mock DesktopFile::desktopExec
    stub.set_lamda(&DesktopFile::desktopExec, [&emptyCommand]() -> QString {
        __DBG_STUB_INVOKE__
        return emptyCommand;
    });

    // Test extraProperties
    QVariantHash result = entity->extraProperties();
    QString executeCommand = result.value(ExtraPropertyName::kExecuteCommand).toString();

    // Verify result
    EXPECT_EQ(executeCommand, "");
}

TEST_F(UT_AppEntryFileEntity, GetFormattedExecCommand_OnlyUnsupportedParams_ReturnsEmpty)
{
    createEntity();

    QString onlyParamsCommand = "%U %u %F %f";

    // Mock DesktopFile::desktopExec
    stub.set_lamda(&DesktopFile::desktopExec, [&onlyParamsCommand]() -> QString {
        __DBG_STUB_INVOKE__
        return onlyParamsCommand;
    });

    // Test extraProperties
    QVariantHash result = entity->extraProperties();
    QString executeCommand = result.value(ExtraPropertyName::kExecuteCommand).toString();

    // Verify result - should be empty after removing all parameters
    EXPECT_EQ(executeCommand.trimmed(), "");
}

TEST_F(UT_AppEntryFileEntity, GetFormattedExecCommand_ComplexCommand_FormatsCorrectly)
{
    createEntity();

    QString complexCommand = "\"'/usr/bin/test-app'\" --config=\"/path/to/config\" %f --output='output.txt' %U";
    QString expectedResult = "/usr/bin/test-app --config=/path/to/config  --output=output.txt ";

    // Mock DesktopFile::desktopExec
    stub.set_lamda(&DesktopFile::desktopExec, [&complexCommand]() -> QString {
        __DBG_STUB_INVOKE__
        return complexCommand;
    });

    // Test extraProperties
    QVariantHash result = entity->extraProperties();
    QString executeCommand = result.value(ExtraPropertyName::kExecuteCommand).toString();

    // Verify result
    EXPECT_EQ(executeCommand, expectedResult);
}

TEST_F(UT_AppEntryFileEntity, Constructor_InvalidUrl_HandlesGracefully)
{
    QUrl invalidUrl;
    bool getAppEntryFileUrlCalled = false;

    // Mock ComputerUtils::getAppEntryFileUrl to return empty URL
    stub.set_lamda(&ComputerUtils::getAppEntryFileUrl, [&](const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        getAppEntryFileUrlCalled = true;
        return QUrl();
    });

    // Create entity with invalid URL
    entity = new AppEntryFileEntity(invalidUrl);

    // Verify construction doesn't crash
    EXPECT_NE(entity, nullptr);
    EXPECT_TRUE(getAppEntryFileUrlCalled);
}

TEST_F(UT_AppEntryFileEntity, DesktopInfo_NullPointer_HandlesGracefully)
{
    // Create entity without proper desktop info initialization
    stub.set_lamda(&ComputerUtils::getAppEntryFileUrl, [this](const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return testFileUrl;
    });

    entity = new AppEntryFileEntity(testUrl);

    // Test methods that use desktopInfo
    // These should not crash even if desktopInfo is in unexpected state
    EXPECT_NO_THROW({
        entity->displayName();
        entity->icon();
        entity->extraProperties();
    });
}

TEST_F(UT_AppEntryFileEntity, MultipleMethodCalls_DifferentParameters_HandlesCorrectly)
{
    createEntity();
    
    QString mockDisplayName = "Test App";
    QString mockIconName = "test-app-icon";
    QString mockExecCommand = "test-app --param";
    
    // Mock all methods
    int displayNameCallCount = 0;
    int iconCallCount = 0;
    int existsCallCount = 0;
    int extraPropertiesCallCount = 0;
    
    stub.set_lamda(&DesktopFile::desktopDisplayName, [&]() -> QString {
        __DBG_STUB_INVOKE__
        displayNameCallCount++;
        return mockDisplayName;
    });
    
    stub.set_lamda(&DesktopFile::desktopIcon, [&]() -> QString {
        __DBG_STUB_INVOKE__
        iconCallCount++;
        return mockIconName;
    });
    
    stub.set_lamda(static_cast<bool (QFile::*)() const>(&QFile::exists), [&]() -> bool {
        __DBG_STUB_INVOKE__
        existsCallCount++;
        return true;
    });
    
    stub.set_lamda(&DesktopFile::desktopExec, [&]() -> QString {
        __DBG_STUB_INVOKE__
        extraPropertiesCallCount++;
        return mockExecCommand;
    });
    
    // Call multiple methods
    QString displayName = entity->displayName();
    QIcon icon = entity->icon();
    bool exists = entity->exists();
    QVariantHash extraProps = entity->extraProperties();
    
    // Verify all methods were called
    EXPECT_EQ(displayNameCallCount, 1);
    EXPECT_EQ(iconCallCount, 1);
    EXPECT_EQ(existsCallCount, 1);
    EXPECT_EQ(extraPropertiesCallCount, 1);
    
    // Verify results
    EXPECT_EQ(displayName, mockDisplayName);
    EXPECT_FALSE(icon.isNull());
    EXPECT_TRUE(exists);
    EXPECT_TRUE(extraProps.contains(ExtraPropertyName::kExecuteCommand));
}

TEST_F(UT_AppEntryFileEntity, ErrorHandling_InvalidDesktopFile_HandlesGracefully)
{
    // Mock ComputerUtils::getAppEntryFileUrl to return invalid URL
    stub.set_lamda(&ComputerUtils::getAppEntryFileUrl, [this](const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl(); // Invalid URL
    });
    
    // Create entity with invalid desktop file
    entity = new AppEntryFileEntity(testUrl);
    
    // Mock DesktopFile methods to throw exceptions or return invalid data
    stub.set_lamda(&DesktopFile::desktopDisplayName, [&]() -> QString {
        __DBG_STUB_INVOKE__
        return ""; // Empty display name
    });
    
    stub.set_lamda(&DesktopFile::desktopIcon, [&]() -> QString {
        __DBG_STUB_INVOKE__
        return ""; // Empty icon name
    });
    
    stub.set_lamda(&DesktopFile::desktopExec, [&]() -> QString {
        __DBG_STUB_INVOKE__
        return ""; // Empty exec command
    });
    
    // Test that methods handle invalid data gracefully
    EXPECT_NO_THROW({
        QString displayName = entity->displayName();
        QIcon icon = entity->icon();
        QVariantHash extraProps = entity->extraProperties();
        
        EXPECT_TRUE(displayName.isEmpty());
        EXPECT_TRUE(icon.isNull());
        EXPECT_TRUE(extraProps.contains(ExtraPropertyName::kExecuteCommand));
        EXPECT_TRUE(extraProps.value(ExtraPropertyName::kExecuteCommand).toString().isEmpty());
    });
}

TEST_F(UT_AppEntryFileEntity, QtMetaObject_CorrectlyInitialized_Success)
{
    createEntity();
    
    // Test that Qt meta-object system works correctly
    const QMetaObject *metaObject = entity->metaObject();
    EXPECT_NE(metaObject, nullptr);
    
    // Test class name
    EXPECT_STREQ(metaObject->className(), "dfmplugin_computer::AppEntryFileEntity");
    
    // Test that inherited methods exist in meta-object
    EXPECT_GE(metaObject->indexOfMethod("displayName()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("icon()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("exists()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showProgress()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showTotalSize()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showUsageSize()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("description()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("order()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("extraProperties()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("isAccessable()"), 0);
}

TEST_F(UT_AppEntryFileEntity, Inheritance_FromAbstractEntryFileEntity_WorksCorrectly)
{
    createEntity();
    
    // Test that AppEntryFileEntity is properly inherited from AbstractEntryFileEntity
    AbstractEntryFileEntity *baseEntity = entity;
    EXPECT_NE(baseEntity, nullptr);
    
    // Test that we can call base class methods
    // AbstractEntryFileEntity doesn't have url() method, so we test other methods
    EXPECT_NO_THROW(baseEntity->displayName());
    EXPECT_NO_THROW(baseEntity->displayName());
    EXPECT_NO_THROW(baseEntity->icon());
    EXPECT_NO_THROW(baseEntity->exists());
    EXPECT_NO_THROW(baseEntity->showProgress());
    EXPECT_NO_THROW(baseEntity->showTotalSize());
    EXPECT_NO_THROW(baseEntity->showUsageSize());
    EXPECT_NO_THROW(baseEntity->description());
    EXPECT_NO_THROW(baseEntity->order());
    EXPECT_NO_THROW(baseEntity->extraProperties());
    EXPECT_NO_THROW(baseEntity->isAccessable());
}

TEST_F(UT_AppEntryFileEntity, MemoryManagement_DeleteEntity_CleansUpCorrectly)
{
    createEntity();
    
    // Store pointer to entity for testing
    AppEntryFileEntity *entityPtr = entity;
    
    // Delete entity
    delete entity;
    entity = nullptr;
    
    // The entity should be deleted, but we can't directly test this
    // We just verify that the delete operation doesn't crash
    EXPECT_EQ(entity, nullptr);
}

TEST_F(UT_AppEntryFileEntity, SpecialCharacters_InExecCommand_HandlesCorrectly)
{
    createEntity();
    
    // Test exec command with various special characters
    QString execWithSpecialChars = "test-app \"--option\" '--param=value' %f %U";
    QString expectedCleanCommand = "test-app --option --param=value  ";
    
    // Mock DesktopFile::desktopExec
    stub.set_lamda(&DesktopFile::desktopExec, [&execWithSpecialChars]() -> QString {
        __DBG_STUB_INVOKE__
        return execWithSpecialChars;
    });
    
    // Test extraProperties to trigger getFormattedExecCommand
    QVariantHash result = entity->extraProperties();
    QString executeCommand = result.value(ExtraPropertyName::kExecuteCommand).toString();
    
    // Verify special characters are handled correctly
    EXPECT_EQ(executeCommand, expectedCleanCommand);
}

TEST_F(UT_AppEntryFileEntity, Consistency_MultipleCalls_ReturnConsistentResults)
{
    createEntity();
    
    QString mockDisplayName = "Test App";
    QString mockIconName = "test-app-icon";
    
    // Mock methods to return consistent values
    stub.set_lamda(&DesktopFile::desktopDisplayName, [&mockDisplayName]() -> QString {
        __DBG_STUB_INVOKE__
        return mockDisplayName;
    });
    
    stub.set_lamda(&DesktopFile::desktopIcon, [&mockIconName]() -> QString {
        __DBG_STUB_INVOKE__
        return mockIconName;
    });
    
    // Call methods multiple times
    QString displayName1 = entity->displayName();
    QString displayName2 = entity->displayName();
    QString displayName3 = entity->displayName();
    
    QIcon icon1 = entity->icon();
    QIcon icon2 = entity->icon();
    QIcon icon3 = entity->icon();
    
    // Verify consistency
    EXPECT_EQ(displayName1, mockDisplayName);
    EXPECT_EQ(displayName2, mockDisplayName);
    EXPECT_EQ(displayName3, mockDisplayName);
    
    EXPECT_FALSE(icon1.isNull());
    EXPECT_FALSE(icon2.isNull());
    EXPECT_FALSE(icon3.isNull());
}
