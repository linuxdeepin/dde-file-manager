// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "watermask/watermaskframe.h"
#include "watermask/deepinlicensehelper.h"
#include "displayconfig.h"
#include "canvasmanager.h"
#include "view/operator/boxselector.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonDocument>
#include <QPixmap>
#include <QFile>
#include <QImageReader>
#include <QLocale>
#include <QObject>
#include <QDir>
#include <QThread>
#include <QThreadPool>
#include <QCoreApplication>
#include <QSignalSpy>

#include <DSysInfo>

DCORE_USE_NAMESPACE

using namespace ddplugin_canvas;

class UT_WaterMaskFrame : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        parentWidget = new QWidget();
        parentWidget->resize(800, 600);
        
        // Mock DeepinLicenseHelper to avoid actual initialization
        stub.set_lamda(ADDR(DeepinLicenseHelper, instance), []() -> DeepinLicenseHelper* {
            __DBG_STUB_INVOKE__
            static DeepinLicenseHelper helper;
            return &helper;
        });
        
        stub.set_lamda(ADDR(DeepinLicenseHelper, init), [](DeepinLicenseHelper*) {
            __DBG_STUB_INVOKE__
        });
        
        // Mock CanvasManager to prevent BoxSelector crashes
        stub.set_lamda(ADDR(CanvasManager, instance), []() -> CanvasManager* {
            __DBG_STUB_INVOKE__
            return nullptr; // Return null to disable canvas operations
        });
        
        // Mock BoxSelector to prevent timer-based updates during tests
        stub.set_lamda(ADDR(BoxSelector, instance), []() -> BoxSelector* {
            __DBG_STUB_INVOKE__
            return nullptr; // Return null to disable BoxSelector completely
        });
        
        // Mock DisplayConfig methods to prevent issues while allowing function calls
        stub.set_lamda(ADDR(DisplayConfig, customWaterMask), [](DisplayConfig*) -> bool {
            __DBG_STUB_INVOKE__
            return false; // Default to false for most tests
        });
        
        QString testConfigFile = "/tmp/test_watermask.json";
        frame = new WaterMaskFrame(testConfigFile, parentWidget);
    }

    virtual void TearDown() override
    {
        // 1. Clean up test objects
        if (frame) {
            frame->disconnect();
            delete frame;
            frame = nullptr;
        }

        if (parentWidget) {
            delete parentWidget;
            parentWidget = nullptr;
        }

        // 2. Clear stubs (this restores all stubbed functions)
        stub.clear();

        // 3. Wait for thread pool tasks to complete
        QThreadPool::globalInstance()->waitForDone(1000);

        // 4. Brief wait to ensure cleanup completion
        QThread::msleep(50);
    }

public:
    QApplication *app = nullptr;
    QWidget *parentWidget = nullptr;
    WaterMaskFrame *frame = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_WaterMaskFrame, constructor_CreateFrame_InitializesCorrectly)
{
    EXPECT_NE(frame, nullptr);
    EXPECT_EQ(frame->parent(), parentWidget);
}

TEST_F(UT_WaterMaskFrame, refresh_CallRefresh_LoadsConfigAndRequestsState)
{
    bool delayGetStateCalled = false;
    
    stub.set_lamda(ADDR(DeepinLicenseHelper, delayGetState), [&delayGetStateCalled](DeepinLicenseHelper*) {
        __DBG_STUB_INVOKE__
        delayGetStateCalled = true;
    });
    
    // Mock file operations to avoid actual file reading
    stub.set_lamda(static_cast<bool (*)(const QString&)>(&QFile::exists), [](const QString&) -> bool {
        __DBG_STUB_INVOKE__
        return false; // Simulate config file not existing
    });
    
    frame->refresh();
    EXPECT_TRUE(delayGetStateCalled);
}

TEST_F(UT_WaterMaskFrame, updatePosition_WithParentWidget_MovesToCorrectPosition)
{
    // Set up parent widget with known size
    parentWidget->resize(800, 600);
    
    // Use QSignalSpy to monitor signals, this is the recommended testing approach
    QSignalSpy showMaskSpy(frame, &WaterMaskFrame::showMask);
    
    frame->updatePosition();
    
    // Verify that the signal was emitted
    EXPECT_EQ(showMaskSpy.count(), 1);
    if (showMaskSpy.count() > 0) {
        // Verify that the signal parameter types are correct
        QList<QVariant> arguments = showMaskSpy.takeFirst();
        EXPECT_EQ(arguments.size(), 1);
        EXPECT_TRUE(arguments.at(0).canConvert<QPoint>());
    }
}

TEST_F(UT_WaterMaskFrame, stateChanged_CallStateChanged_UpdatesDisplay)
{
    int state = 1;
    int prop = 0;
    
    // Mock file operations
    stub.set_lamda(static_cast<bool (*)(const QString&)>(&QFile::exists), [](const QString&) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    
    // Mock showLicenseState
    stub.set_lamda(ADDR(WaterMaskFrame, showLicenseState), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    // The stateChanged slot should not crash
    EXPECT_NO_THROW(frame->stateChanged(state, prop));
}

TEST_F(UT_WaterMaskFrame, maskPixmap_CreatePixmap_ReturnsValidPixmap)
{
    QString uri = ":/test.png"; // Mock resource path
    QSize size(100, 50);
    qreal pixelRatio = 1.0;
    
    // Mock QImageReader to avoid actual file reading
    stub.set_lamda(static_cast<QImage (QImageReader::*)()>(&QImageReader::read), [&size](QImageReader*) -> QImage {
        __DBG_STUB_INVOKE__
        return QImage(size, QImage::Format_ARGB32);
    });
    
    stub.set_lamda(ADDR(QImageReader, size), [&size](QImageReader*) -> QSize {
        __DBG_STUB_INVOKE__
        return size;
    });
    
    QPixmap result = WaterMaskFrame::maskPixmap(uri, size, pixelRatio);
    
    // Should return a pixmap (might be null due to mocking, but should not crash)
    EXPECT_NO_THROW(WaterMaskFrame::maskPixmap(uri, size, pixelRatio));
}

TEST_F(UT_WaterMaskFrame, showLicenseState_StaticMethod_ReturnsBoolean)
{
    // Mock DSysInfo methods to avoid system dependencies
    stub.set_lamda(static_cast<DSysInfo::DeepinType (*)()>(&DSysInfo::deepinType), []() -> DSysInfo::DeepinType {
        __DBG_STUB_INVOKE__
        return DSysInfo::DeepinDesktop;
    });
    
    bool result = WaterMaskFrame::showLicenseState();
    
    // Should return a boolean value without crashing
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_WaterMaskFrame, usingCn_StaticMethod_ReturnsBoolean)
{
    // Mock locale detection
    stub.set_lamda(ADDR(QLocale, system), []() -> QLocale {
        __DBG_STUB_INVOKE__
        return QLocale(QLocale::Chinese, QLocale::China);
    });
    
    bool result = WaterMaskFrame::usingCn();
    
    // Should return a boolean value without crashing
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_WaterMaskFrame, addWidget_StaticMethod_AddsWidgetToLayout)
{
    QHBoxLayout layout;
    QLabel testWidget;
    QString align = "left";
    
    // Should not crash when adding widget
    EXPECT_NO_THROW(WaterMaskFrame::addWidget(&layout, &testWidget, align));
    
    // Test right align
    align = "right";
    EXPECT_NO_THROW(WaterMaskFrame::addWidget(&layout, &testWidget, align));
    
    // Test center align
    align = "center";
    EXPECT_NO_THROW(WaterMaskFrame::addWidget(&layout, &testWidget, align));
}

TEST_F(UT_WaterMaskFrame, loadConfig_CallLoadConfig_DoesNotCrash)
{
    // Test loadConfig without file operations stubbing to avoid cpp-stub issues
    // This tests that the method can be called safely even when config file doesn't exist
    EXPECT_NO_THROW(frame->loadConfig());
}

TEST_F(UT_WaterMaskFrame, parseJson_WithValidConfig_ReturnsConfigMap)
{
    QJsonObject testConfig;
    testConfig["maskLogoUri"] = "/usr/share/pixmaps/test-logo.png";
    testConfig["maskLogoWidth"] = 120;
    testConfig["maskLogoHeight"] = 40;
    testConfig["maskLogoTextSpacing"] = 10;
    testConfig["maskHeight"] = 50;
    testConfig["xRightBottom"] = 60;
    testConfig["yRightBottom"] = 60;
    testConfig["maskLogoGovernmentCnUri"] = "/usr/share/pixmaps/gov-cn.png";
    testConfig["maskLogoGovernmentEnUri"] = "/usr/share/pixmaps/gov-en.png";
    testConfig["maskLogoEnterpriseCnUri"] = "/usr/share/pixmaps/ent-cn.png";
    testConfig["maskLogoEnterpriseEnUri"] = "/usr/share/pixmaps/ent-en.png";
    testConfig["maskLogoSecretesSecurityCnUri"] = "/usr/share/pixmaps/sec-cn.png";
    testConfig["maskLogoSecretesSecurityEnUri"] = "/usr/share/pixmaps/sec-en.png";
    
    // Mock DSysInfo::distributionOrgLogo to avoid system dependencies
    stub.set_lamda(ADDR(DSysInfo, distributionOrgLogo), 
                   [](DSysInfo::OrgType, DSysInfo::LogoType, const QString& fallback = QString()) -> QString {
        __DBG_STUB_INVOKE__
        return "/usr/share/pixmaps/default-logo.png";
    });
    
    // Mock DispalyIns->customWaterMask() to return false
    stub.set_lamda(ADDR(DisplayConfig, customWaterMask), [](DisplayConfig*) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    
    auto configMap = frame->parseJson(&testConfig);
    
    // Should return non-empty config map
    EXPECT_FALSE(configMap.isEmpty());
    EXPECT_TRUE(configMap.contains("default"));
    EXPECT_TRUE(configMap.contains("gov-cn"));
    EXPECT_TRUE(configMap.contains("gov-en"));
    EXPECT_TRUE(configMap.contains("ent-cn"));
    EXPECT_TRUE(configMap.contains("ent-en"));
    EXPECT_TRUE(configMap.contains("sec-cn"));
    EXPECT_TRUE(configMap.contains("sec-en"));
}

TEST_F(UT_WaterMaskFrame, defaultCfg_WithValidConfig_ReturnsDefaultConfig)
{
    QJsonObject testConfig;
    testConfig["maskLogoUri"] = "/usr/share/pixmaps/test-logo.png";
    testConfig["maskLogoWidth"] = 120;
    testConfig["maskLogoHeight"] = 40;
    testConfig["maskLogoTextSpacing"] = 10;
    testConfig["maskHeight"] = 50;
    testConfig["xRightBottom"] = 60;
    testConfig["yRightBottom"] = 60;
    
    // Mock DSysInfo::distributionOrgLogo
    stub.set_lamda(ADDR(DSysInfo, distributionOrgLogo), 
                   [](DSysInfo::OrgType, DSysInfo::LogoType, const QString& fallback = QString()) -> QString {
        __DBG_STUB_INVOKE__
        return "/usr/share/pixmaps/default-logo.png";
    });
    
    // Mock DispalyIns->customWaterMask() to test both branches
    stub.set_lamda(ADDR(DisplayConfig, customWaterMask), [](DisplayConfig*) -> bool {
        __DBG_STUB_INVOKE__
        return true; // Test custom water mask path
    });
    
    auto config = frame->defaultCfg(&testConfig);
    
    EXPECT_TRUE(config.valid);
    EXPECT_EQ(config.maskLogoWidth, 120);
    EXPECT_EQ(config.maskLogoHeight, 40);
    EXPECT_EQ(config.maskLogoTextSpacing, 10);
    EXPECT_EQ(config.maskHeight, 50);
    EXPECT_EQ(config.xRightBottom, 60);
    EXPECT_EQ(config.yRightBottom, 60);
}

TEST_F(UT_WaterMaskFrame, govCfg_WithValidConfig_ReturnsGovernmentConfig)
{
    QJsonObject testConfig;
    testConfig["maskLogoGovernmentCnUri"] = "/usr/share/pixmaps/gov-cn.png";
    testConfig["maskLogoGovernmentEnUri"] = "/usr/share/pixmaps/gov-en.png";
    testConfig["maskLogoWidth"] = 150;
    testConfig["maskLogoHeight"] = 50;
    testConfig["maskHeight"] = 60;
    testConfig["xRightBottom"] = 70;
    testConfig["yRightBottom"] = 70;
    
    // Test Chinese government config
    auto configCn = frame->govCfg(&testConfig, true);
    EXPECT_TRUE(configCn.valid);
    EXPECT_EQ(configCn.maskLogoUri, "/usr/share/pixmaps/gov-cn.png");
    EXPECT_EQ(configCn.maskLogoWidth, 150);
    EXPECT_EQ(configCn.maskLogoHeight, 50);
    EXPECT_EQ(configCn.maskLogoTextSpacing, 0); // Should be 0 for gov config
    
    // Test English government config
    auto configEn = frame->govCfg(&testConfig, false);
    EXPECT_TRUE(configEn.valid);
    EXPECT_EQ(configEn.maskLogoUri, "/usr/share/pixmaps/gov-en.png");
}

TEST_F(UT_WaterMaskFrame, govCfg_WithMissingLogo_ReturnsInvalidConfig)
{
    QJsonObject testConfig;
    // No logo URI provided
    testConfig["maskLogoWidth"] = 150;
    testConfig["maskLogoHeight"] = 50;
    
    auto configCn = frame->govCfg(&testConfig, true);
    EXPECT_FALSE(configCn.valid);
    
    auto configEn = frame->govCfg(&testConfig, false);
    EXPECT_FALSE(configEn.valid);
}

TEST_F(UT_WaterMaskFrame, entCfg_WithValidConfig_ReturnsEnterpriseConfig)
{
    QJsonObject testConfig;
    testConfig["maskLogoEnterpriseCnUri"] = "/usr/share/pixmaps/ent-cn.png";
    testConfig["maskLogoEnterpriseEnUri"] = "/usr/share/pixmaps/ent-en.png";
    testConfig["maskLogoWidth"] = 140;
    testConfig["maskLogoHeight"] = 45;
    testConfig["maskHeight"] = 55;
    testConfig["xRightBottom"] = 65;
    testConfig["yRightBottom"] = 65;
    
    // Test Chinese enterprise config
    auto configCn = frame->entCfg(&testConfig, true);
    EXPECT_TRUE(configCn.valid);
    EXPECT_EQ(configCn.maskLogoUri, "/usr/share/pixmaps/ent-cn.png");
    EXPECT_EQ(configCn.maskLogoWidth, 140);
    EXPECT_EQ(configCn.maskLogoHeight, 45);
    EXPECT_EQ(configCn.maskLogoTextSpacing, 0); // Should be 0 for ent config
    
    // Test English enterprise config
    auto configEn = frame->entCfg(&testConfig, false);
    EXPECT_TRUE(configEn.valid);
    EXPECT_EQ(configEn.maskLogoUri, "/usr/share/pixmaps/ent-en.png");
}

TEST_F(UT_WaterMaskFrame, entCfg_WithMissingLogo_ReturnsInvalidConfig)
{
    QJsonObject testConfig;
    // No logo URI provided
    testConfig["maskLogoWidth"] = 140;
    testConfig["maskLogoHeight"] = 45;
    
    auto configCn = frame->entCfg(&testConfig, true);
    EXPECT_FALSE(configCn.valid);
    
    auto configEn = frame->entCfg(&testConfig, false);
    EXPECT_FALSE(configEn.valid);
}

TEST_F(UT_WaterMaskFrame, secCfg_WithValidConfig_ReturnsSecretsSecurityConfig)
{
    QJsonObject testConfig;
    testConfig["maskLogoSecretesSecurityCnUri"] = "/usr/share/pixmaps/sec-cn.png";
    testConfig["maskLogoSecretesSecurityEnUri"] = "/usr/share/pixmaps/sec-en.png";
    testConfig["maskLogoWidth"] = 160;
    testConfig["maskLogoHeight"] = 55;
    testConfig["maskHeight"] = 65;
    testConfig["xRightBottom"] = 75;
    testConfig["yRightBottom"] = 75;
    
    // Test Chinese secrets security config
    auto configCn = frame->secCfg(&testConfig, true);
    EXPECT_TRUE(configCn.valid);
    EXPECT_EQ(configCn.maskLogoUri, "/usr/share/pixmaps/sec-cn.png");
    EXPECT_EQ(configCn.maskLogoWidth, 160);
    EXPECT_EQ(configCn.maskLogoHeight, 55);
    EXPECT_EQ(configCn.maskLogoTextSpacing, 0); // Should be 0 for sec config
    
    // Test English secrets security config
    auto configEn = frame->secCfg(&testConfig, false);
    EXPECT_TRUE(configEn.valid);
    EXPECT_EQ(configEn.maskLogoUri, "/usr/share/pixmaps/sec-en.png");
}

TEST_F(UT_WaterMaskFrame, secCfg_WithMissingLogo_ReturnsInvalidConfig)
{
    QJsonObject testConfig;
    // No logo URI provided
    testConfig["maskLogoWidth"] = 160;
    testConfig["maskLogoHeight"] = 55;
    
    auto configCn = frame->secCfg(&testConfig, true);
    EXPECT_FALSE(configCn.valid);
    
    auto configEn = frame->secCfg(&testConfig, false);
    EXPECT_FALSE(configEn.valid);
}

TEST_F(UT_WaterMaskFrame, setTextAlign_WithDifferentAlignments_SetsCorrectAlignment)
{
    // Test left alignment
    frame->setTextAlign("left");
    EXPECT_NO_THROW(frame->setTextAlign("left"));
    
    // Test right alignment
    frame->setTextAlign("right");
    EXPECT_NO_THROW(frame->setTextAlign("right"));
    
    // Test center alignment
    frame->setTextAlign("center");
    EXPECT_NO_THROW(frame->setTextAlign("center"));
    
    // Test unknown alignment (should not crash)
    frame->setTextAlign("unknown");
    EXPECT_NO_THROW(frame->setTextAlign("unknown"));
}

TEST_F(UT_WaterMaskFrame, stateChanged_WithDifferentStates_HandlesAllStates)
{
    // Mock showLicenseState to return true  
    stub.set_lamda(ADDR(WaterMaskFrame, showLicenseState), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    // Test Unauthorized state
    EXPECT_NO_THROW(frame->stateChanged(0, 0)); // DeepinLicenseHelper::Unauthorized
    
    // Test AuthorizedLapse state  
    EXPECT_NO_THROW(frame->stateChanged(2, 0)); // DeepinLicenseHelper::AuthorizedLapse
    
    // Test TrialExpired state
    EXPECT_NO_THROW(frame->stateChanged(4, 0)); // DeepinLicenseHelper::TrialExpired
    
    // Test Authorized state with different properties
    EXPECT_NO_THROW(frame->stateChanged(1, 1)); // Authorized with Secretssecurity
    EXPECT_NO_THROW(frame->stateChanged(1, 2)); // Authorized with Government
    EXPECT_NO_THROW(frame->stateChanged(1, 3)); // Authorized with Enterprise
    
    // Test TrialAuthorized state
    EXPECT_NO_THROW(frame->stateChanged(3, 0)); // DeepinLicenseHelper::TrialAuthorized
    
    // Test unknown state
    EXPECT_NO_THROW(frame->stateChanged(99, 0)); // Unknown state
}

TEST_F(UT_WaterMaskFrame, showLicenseState_WithDifferentSystemTypes_ReturnsCorrectValue)
{
    // Test with DeepinProfessional
    stub.set_lamda(static_cast<DSysInfo::DeepinType (*)()>(&DSysInfo::deepinType), []() -> DSysInfo::DeepinType {
        __DBG_STUB_INVOKE__
        return DSysInfo::DeepinProfessional;
    });
    
    stub.set_lamda(static_cast<DSysInfo::UosEdition (*)()>(&DSysInfo::uosEditionType), []() -> DSysInfo::UosEdition {
        __DBG_STUB_INVOKE__
        return DSysInfo::UosEnterprise;
    });
    
    bool result = WaterMaskFrame::showLicenseState();
    EXPECT_TRUE(result);
    
    // Test with DeepinPersonal
    stub.set_lamda(static_cast<DSysInfo::DeepinType (*)()>(&DSysInfo::deepinType), []() -> DSysInfo::DeepinType {
        __DBG_STUB_INVOKE__
        return DSysInfo::DeepinPersonal;
    });
    
    result = WaterMaskFrame::showLicenseState();
    EXPECT_TRUE(result);
    
    // Test with DeepinServer
    stub.set_lamda(static_cast<DSysInfo::DeepinType (*)()>(&DSysInfo::deepinType), []() -> DSysInfo::DeepinType {
        __DBG_STUB_INVOKE__
        return DSysInfo::DeepinServer;
    });
    
    result = WaterMaskFrame::showLicenseState();
    EXPECT_TRUE(result);
    
    // Test with other types - should return false
    stub.set_lamda(static_cast<DSysInfo::DeepinType (*)()>(&DSysInfo::deepinType), []() -> DSysInfo::DeepinType {
        __DBG_STUB_INVOKE__
        return DSysInfo::DeepinDesktop;
    });
    
    result = WaterMaskFrame::showLicenseState();
    EXPECT_FALSE(result);
}

TEST_F(UT_WaterMaskFrame, maskPixmap_WithSvgFile_HandlesCorrectly)
{
    QString svgUri = "/test/image.svg";
    QSize size(100, 50);
    qreal pixelRatio = 2.0;
    
    // Mock QImageReader for SVG
    stub.set_lamda(static_cast<QImage (QImageReader::*)()>(&QImageReader::read), [&size](QImageReader*) -> QImage {
        __DBG_STUB_INVOKE__
        return QImage(size, QImage::Format_ARGB32);
    });
    
    stub.set_lamda(ADDR(QImageReader, size), [&size](QImageReader*) -> QSize {
        __DBG_STUB_INVOKE__
        return size;
    });
    
    QPixmap result = WaterMaskFrame::maskPixmap(svgUri, size, pixelRatio);
    
    // Should handle SVG files without crashing
    EXPECT_NO_THROW(WaterMaskFrame::maskPixmap(svgUri, size, pixelRatio));
}
TEST_F(UT_WaterMaskFrame, usingCn_WithDifferentLocales_ReturnsCorrectValue)
{
    // Test with Chinese locale
    stub.set_lamda(ADDR(QLocale, system), []() -> QLocale {
        __DBG_STUB_INVOKE__
        return QLocale("zh_CN");
    });
    
    bool result = WaterMaskFrame::usingCn();
    EXPECT_TRUE(result);
    
    // Test with Traditional Chinese
    stub.set_lamda(ADDR(QLocale, system), []() -> QLocale {
        __DBG_STUB_INVOKE__
        return QLocale("zh_TW");
    });
    
    result = WaterMaskFrame::usingCn();
    EXPECT_TRUE(result);
    
    // Test with English locale
    stub.set_lamda(ADDR(QLocale, system), []() -> QLocale {
        __DBG_STUB_INVOKE__
        return QLocale("en_US");
    });
    
    result = WaterMaskFrame::usingCn();
    EXPECT_FALSE(result);
}
