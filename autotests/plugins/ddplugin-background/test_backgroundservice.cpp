// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "backgroundservice.h"

#include <QStandardPaths>
#include <QSettings>
#include <QDBusInterface>
#include <QAnyStringView>  // Add QAnyStringView header file

DDP_BACKGROUND_USE_NAMESPACE

class UT_BackgroundService : public testing::Test
{
protected:
    void SetUp() override
    {
        // Use correct function signature
        stub.set_lamda(
            // Use correct parameter type QAnyStringView
            static_cast<QVariant (QSettings::*)(QAnyStringView, const QVariant &) const>(&QSettings::value),
            [](QSettings *, QAnyStringView key, const QVariant &defaultValue) -> QVariant {
                __DBG_STUB_INVOKE__
                if (key == "Workspace/CurrentDesktop")
                    return QVariant(2);  // Mock current workspace as 2
                return defaultValue;
            }
        );
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// Test non-pure virtual functions of abstract class
class MockBackgroundService : public BackgroundService
{
public:
    explicit MockBackgroundService(QObject *parent = nullptr) : BackgroundService(parent) {}
    QString background(const QString &screen) override { return QString(); }
    QString getDefaultBackground() override { return BackgroundService::getDefaultBackground(); }
};

TEST_F(UT_BackgroundService, testGetDefaultBackground)
{
    MockBackgroundService service;
    EXPECT_EQ(service.getDefaultBackground(), QString("/usr/share/backgrounds/default_background.jpg"));
}

TEST_F(UT_BackgroundService, testGetCurrentWorkspaceIndex)
{
    MockBackgroundService service;
    EXPECT_EQ(service.getCurrentWorkspaceIndex(), 2);  // Since we stubbed QSettings::value to return 2
}

TEST_F(UT_BackgroundService, testOnWorkspaceSwitched)
{
    MockBackgroundService service;
    
    bool signalEmitted = false;
    QObject::connect(&service, &BackgroundService::backgroundChanged, [&signalEmitted]() {
        signalEmitted = true;
    });
    
    service.onWorkspaceSwitched(1, 3);
    
    EXPECT_TRUE(signalEmitted);
    EXPECT_EQ(service.currentWorkspaceIndex, 3);
}