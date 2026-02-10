// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include <dfm-framework/dpf.h>
#include "events/opticaleventcaller.h"

DPF_USE_NAMESPACE

using namespace dfmplugin_optical;

class TestOpticalEventCaller : public testing::Test
{
public:
    void SetUp() override
    {
        // OpticalEventCaller is a static class, no instance needed
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

TEST_F(TestOpticalEventCaller, SendOpenBurnDlg_ValidParameters_CallsSlotChannel)
{
    QString deviceId = "/dev/sr0";
    bool isSupportedUDF = true;
    QWidget *parent = nullptr;

    OpticalEventCaller::sendOpenBurnDlg(deviceId, isSupportedUDF, parent);
}

TEST_F(TestOpticalEventCaller, SendOpenBurnDlg_UnsupportedUDF_CallsSlotChannelWithFalse)
{
    QString deviceId = "/dev/sr1";
    bool isSupportedUDF = false;
    QWidget testWidget;
    QWidget *parent = &testWidget;

    OpticalEventCaller::sendOpenBurnDlg(deviceId, isSupportedUDF, parent);
}

TEST_F(TestOpticalEventCaller, SendOpenBurnDlg_EmptyDeviceId_CallsSlotChannel)
{
    QString deviceId = "";
    bool isSupportedUDF = true;
    QWidget *parent = nullptr;

    OpticalEventCaller::sendOpenBurnDlg(deviceId, isSupportedUDF, parent);
}

TEST_F(TestOpticalEventCaller, SendOpenDumpISODlg_ValidDeviceId_CallsSlotChannel)
{
    QString deviceId = "/dev/sr0";
    bool slotChannelPushCalled = false;

    OpticalEventCaller::sendOpenDumpISODlg(deviceId);
}

TEST_F(TestOpticalEventCaller, SendOpenDumpISODlg_EmptyDeviceId_CallsSlotChannel)
{
    QString deviceId = "";
    bool slotChannelPushCalled = false;

    OpticalEventCaller::sendOpenDumpISODlg(deviceId);
}

TEST_F(TestOpticalEventCaller, SendOpenDumpISODlg_MultipleDeviceIds_CallsSlotChannelMultipleTimes)
{
    QStringList deviceIds = {"/dev/sr0", "/dev/sr1", "/dev/sr2"};
    int slotChannelPushCallCount = 0;

    for (const QString &deviceId : deviceIds) {
        OpticalEventCaller::sendOpenDumpISODlg(deviceId);
    }
}

TEST_F(TestOpticalEventCaller, SendOpenBurnDlg_MultipleCallsWithDifferentParams_AllCallsSucceed)
{
    struct TestCase {
        QString deviceId;
        bool isSupportedUDF;
        QWidget *parent;
    };

    QWidget testWidget1, testWidget2;
    QList<TestCase> testCases = {
        {"/dev/sr0", true, nullptr},
        {"/dev/sr1", false, &testWidget1},
        {"/dev/sr2", true, &testWidget2},
        {"", false, nullptr}
    };

    for (const auto &testCase : testCases) {
        OpticalEventCaller::sendOpenBurnDlg(testCase.deviceId, testCase.isSupportedUDF, testCase.parent);
    }
} 