// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/displayconfig.h"

#include <QSettings>
#include <QTimer>
#include <QThread>
#include <QThreadPool>
#include <QHash>
#include <QPoint>
#include <QAnyStringView>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

class UT_DisplayConfig : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Use real DisplayConfig instance for testing
        config = DisplayConfig::instance();
        
        // Stub sync method to prevent deadlock issues
        stub.set_lamda(ADDR(DisplayConfig, sync), [](DisplayConfig*) {
            __DBG_STUB_INVOKE__
            // Do nothing to avoid cross-thread invokeMethod deadlock
        });
    }

    virtual void TearDown() override
    {
        // Clear stubs first
        stub.clear();
        
        // Wait for thread pool tasks to complete
        QThreadPool::globalInstance()->waitForDone(1000);
        
        // Brief wait to ensure cleanup completion
        QThread::msleep(50);
    }

public:
    stub_ext::StubExt stub;
    DisplayConfig *config = nullptr;
};

TEST_F(UT_DisplayConfig, instance)
{
    EXPECT_NE(config, nullptr);
}

TEST_F(UT_DisplayConfig, profile)
{
    bool childKeysCalled = false;
    bool valueCalled = false;
    
    // Stub QSettings::childKeys to return test keys
    stub.set_lamda(ADDR(QSettings, childKeys), [&childKeysCalled](QSettings*) -> QStringList {
        __DBG_STUB_INVOKE__
        childKeysCalled = true;
        return QStringList{"1", "2"};
    });
    
    // Stub QSettings::value to return test profile values
    stub.set_lamda(static_cast<QVariant(QSettings::*)(QAnyStringView) const>(&QSettings::value), 
                   [&valueCalled](QSettings*, QAnyStringView key) -> QVariant {
        __DBG_STUB_INVOKE__
        valueCalled = true;
        QString keyStr = key.toString();
        if (keyStr == "1") return QString("screen1");
        if (keyStr == "2") return QString("screen2");
        return QVariant();
    });

    QList<QString> result = config->profile();
    EXPECT_TRUE(childKeysCalled);
    EXPECT_TRUE(valueCalled);
    EXPECT_EQ(result.size(), 2);
}

TEST_F(UT_DisplayConfig, setProfile)
{
    bool setValuesCalled = false;
    QString capturedGroup;
    QHash<QString, QVariant> capturedValues;
    
    // Stub setValues method (protected method)
    stub.set_lamda(ADDR(DisplayConfig, setValues), [&setValuesCalled, &capturedGroup, &capturedValues](DisplayConfig *, const QString &group, const QHash<QString, QVariant> &values) {
        __DBG_STUB_INVOKE__
        setValuesCalled = true;
        capturedGroup = group;
        capturedValues = values;
    });

    QList<QString> testProfile = {"display1", "display2", "display3"};
    bool result = config->setProfile(testProfile);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(setValuesCalled);
    EXPECT_EQ(capturedGroup, QString("Profile")); // Use correct constant
    EXPECT_TRUE(capturedValues.contains("1"));
    EXPECT_TRUE(capturedValues.contains("2"));
    EXPECT_TRUE(capturedValues.contains("3"));
}

TEST_F(UT_DisplayConfig, coordinates)
{
    bool childKeysCalled = false;
    bool valueCalled = false;
    
    // Stub QSettings::childKeys to return test position keys
    stub.set_lamda(ADDR(QSettings, childKeys), [&childKeysCalled](QSettings*) -> QStringList {
        __DBG_STUB_INVOKE__
        childKeysCalled = true;
        return QStringList{"100_200", "300_400"}; // Position format: x_y
    });
    
    // Stub QSettings::value to return filenames
    stub.set_lamda(static_cast<QVariant(QSettings::*)(QAnyStringView) const>(&QSettings::value), 
                   [&valueCalled](QSettings*, QAnyStringView key) -> QVariant {
        __DBG_STUB_INVOKE__
        valueCalled = true;
        QString keyStr = key.toString();
        if (keyStr == "100_200") return QString("file1.txt");
        if (keyStr == "300_400") return QString("file2.txt");
        return QVariant();
    });

    QHash<QString, QPoint> result = config->coordinates("testkey");
    EXPECT_TRUE(childKeysCalled);
    EXPECT_TRUE(valueCalled);
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.contains("file1.txt"));
    EXPECT_TRUE(result.contains("file2.txt"));
}

TEST_F(UT_DisplayConfig, setCoordinates)
{
    bool setValuesCalled = false;
    QString capturedGroup;
    QHash<QString, QVariant> capturedValues;
    
    // Stub setValues method (protected method)
    stub.set_lamda(ADDR(DisplayConfig, setValues), [&setValuesCalled, &capturedGroup, &capturedValues](DisplayConfig *, const QString &group, const QHash<QString, QVariant> &values) {
        __DBG_STUB_INVOKE__
        setValuesCalled = true;
        capturedGroup = group;
        capturedValues = values;
    });

    QHash<QString, QPoint> testCoords;
    testCoords["file1.txt"] = QPoint(100, 200);
    testCoords["file2.txt"] = QPoint(300, 400);
    
    bool result = config->setCoordinates("testkey", testCoords);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(setValuesCalled);
    EXPECT_EQ(capturedGroup, QString("testkey")); // group is the key parameter
    EXPECT_TRUE(capturedValues.contains("100_200")); // position format: x_y
    EXPECT_TRUE(capturedValues.contains("300_400"));
}

TEST_F(UT_DisplayConfig, sortMethod)
{
    bool valueCalled = false;
    int callCount = 0;
    
    // Stub QSettings::value to return sort data
    // First overload: value(QAnyStringView key) const
    stub.set_lamda(static_cast<QVariant(QSettings::*)(QAnyStringView) const>(&QSettings::value), 
                   [&valueCalled, &callCount](QSettings*, QAnyStringView key) -> QVariant {
        __DBG_STUB_INVOKE__
        valueCalled = true;
        callCount++;
        
        QString keyStr = key.toString();
        if (keyStr == "SortBy") { // kKeySortBy
            return 1; // Mock sort role
        } else if (keyStr == "SortOrder") { // kKeySortOrder
            return static_cast<int>(Qt::DescendingOrder);
        }
        return QVariant();
    });
    
    // Also stub the overload with default value
    stub.set_lamda(static_cast<QVariant(QSettings::*)(QAnyStringView, const QVariant &) const>(&QSettings::value), 
                   [&valueCalled, &callCount](QSettings*, QAnyStringView key, const QVariant &defaultVar) -> QVariant {
        __DBG_STUB_INVOKE__
        valueCalled = true;
        callCount++;
        
        QString keyStr = key.toString();
        if (keyStr == "SortBy") { // kKeySortBy
            return 1; // Mock sort role
        } else if (keyStr == "SortOrder") { // kKeySortOrder
            return static_cast<int>(Qt::DescendingOrder);
        }
        return defaultVar;
    });

    int role = 0;
    Qt::SortOrder order = Qt::AscendingOrder;
    config->sortMethod(role, order);
    
    EXPECT_TRUE(valueCalled);
    EXPECT_GE(callCount, 1); // Should be called at least once
    EXPECT_EQ(role, 1);
    EXPECT_EQ(order, Qt::DescendingOrder);
}

TEST_F(UT_DisplayConfig, setSortMethod)
{
    bool setValuesCalled = false;
    QString capturedGroup;
    QHash<QString, QVariant> capturedValues;
    
    // Stub setValues method (protected method)
    stub.set_lamda(ADDR(DisplayConfig, setValues), [&setValuesCalled, &capturedGroup, &capturedValues](DisplayConfig *, const QString &group, const QHash<QString, QVariant> &values) {
        __DBG_STUB_INVOKE__
        setValuesCalled = true;
        capturedGroup = group;
        capturedValues = values;
    });

    bool result = config->setSortMethod(2, Qt::DescendingOrder);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(setValuesCalled);
    EXPECT_EQ(capturedGroup, QString("GeneralConfig")); // kGroupGeneral
    EXPECT_TRUE(capturedValues.contains("SortBy")); // kKeySortBy
    EXPECT_TRUE(capturedValues.contains("SortOrder")); // kKeySortOrder
    EXPECT_EQ(capturedValues["SortBy"].toInt(), 2);
    EXPECT_EQ(capturedValues["SortOrder"].toInt(), static_cast<int>(Qt::DescendingOrder));
}

TEST_F(UT_DisplayConfig, autoAlign)
{
    bool dConfigValueCalled = false;
    
    // Stub DConfigManager::value to return -1 (fallback to local config)
    stub.set_lamda(&dfmbase::DConfigManager::value, [&dConfigValueCalled](dfmbase::DConfigManager*, const QString &name, const QString &key, const QVariant &defaultVar) -> QVariant {
        __DBG_STUB_INVOKE__
        dConfigValueCalled = true;
        EXPECT_EQ(name, QString("org.deepin.dde.file-manager.desktop"));
        EXPECT_EQ(key, QString("autoAlign"));
        return -1; // Force fallback to local value method
    });
    
    // Stub the protected value method
    stub.set_lamda(ADDR(DisplayConfig, value), [](DisplayConfig *, const QString &group, const QString &key, const QVariant &defaultVar) -> QVariant {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(group, QString("GeneralConfig")); // kGroupGeneral
        EXPECT_EQ(key, QString("AutoSort")); // kKeyAutoAlign
        return true;
    });

    bool result = config->autoAlign();
    EXPECT_TRUE(dConfigValueCalled);
    EXPECT_TRUE(result);
}

TEST_F(UT_DisplayConfig, setAutoAlign)
{
    bool setValuesCalled = false;
    bool dConfigSetValueCalled = false;
    QString capturedGroup;
    QHash<QString, QVariant> capturedValues;
    
    // Stub setValues method (protected method)
    stub.set_lamda(ADDR(DisplayConfig, setValues), [&setValuesCalled, &capturedGroup, &capturedValues](DisplayConfig *, const QString &group, const QHash<QString, QVariant> &values) {
        __DBG_STUB_INVOKE__
        setValuesCalled = true;
        capturedGroup = group;
        capturedValues = values;
    });
    
    // Stub DConfigManager::setValue 
    stub.set_lamda(&dfmbase::DConfigManager::setValue, [&dConfigSetValueCalled](dfmbase::DConfigManager*, const QString &name, const QString &key, const QVariant &value) {
        __DBG_STUB_INVOKE__
        dConfigSetValueCalled = true;
        EXPECT_EQ(name, QString("org.deepin.dde.file-manager.desktop"));
        EXPECT_EQ(key, QString("autoAlign"));
        EXPECT_EQ(value.toInt(), 0); // false -> 0
    });

    config->setAutoAlign(false);
    
    EXPECT_TRUE(setValuesCalled);
    EXPECT_TRUE(dConfigSetValueCalled);
    EXPECT_EQ(capturedGroup, QString("GeneralConfig")); // kGroupGeneral
    EXPECT_TRUE(capturedValues.contains("AutoSort")); // kKeyAutoAlign
    EXPECT_FALSE(capturedValues["AutoSort"].toBool());
}

TEST_F(UT_DisplayConfig, iconLevel)
{
    bool valueCalled = false;
    
    // Stub the protected value method
    stub.set_lamda(ADDR(DisplayConfig, value), [&valueCalled](DisplayConfig *, const QString &group, const QString &key, const QVariant &defaultVar) -> QVariant {
        __DBG_STUB_INVOKE__
        valueCalled = true;
        EXPECT_EQ(group, QString("GeneralConfig")); // kGroupGeneral
        EXPECT_EQ(key, QString("IconLevel")); // kKeyIconLevel
        return 3;
    });

    int result = config->iconLevel();
    EXPECT_TRUE(valueCalled);
    EXPECT_EQ(result, 3);
}

TEST_F(UT_DisplayConfig, setIconLevel)
{
    bool setValuesCalled = false;
    QString capturedGroup;
    QHash<QString, QVariant> capturedValues;
    
    // Stub setValues method (protected method)
    stub.set_lamda(ADDR(DisplayConfig, setValues), [&setValuesCalled, &capturedGroup, &capturedValues](DisplayConfig *, const QString &group, const QHash<QString, QVariant> &values) {
        __DBG_STUB_INVOKE__
        setValuesCalled = true;
        capturedGroup = group;
        capturedValues = values;
    });

    bool result = config->setIconLevel(4);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(setValuesCalled);
    EXPECT_EQ(capturedGroup, QString("GeneralConfig")); // kGroupGeneral
    EXPECT_TRUE(capturedValues.contains("IconLevel")); // kKeyIconLevel
    EXPECT_EQ(capturedValues["IconLevel"].toInt(), 4);
}

TEST_F(UT_DisplayConfig, sync)
{
    // Test sync method - should not crash
    EXPECT_NO_THROW(config->sync());
    SUCCEED();
}
