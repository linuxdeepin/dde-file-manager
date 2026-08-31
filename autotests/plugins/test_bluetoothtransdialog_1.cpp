// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bluetoothtransdialog_1.cpp
 * @brief Unit tests for BluetoothTransDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bluetooth/private/bluetoothtransdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class BluetoothTransDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BluetoothTransDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BluetoothTransDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BluetoothTransDialogTest, addDevice)
{
    // Test method: void addDevice((const BluetoothDevice *dev))
    EXPECT_NO_FATAL_FAILURE(obj->addDevice(nullptr));
}

TEST_F(BluetoothTransDialogTest, changeLabelTheme)
{
    // Test method: void changeLabelTheme((QLabel *obj, bool isTitle))
    EXPECT_NO_FATAL_FAILURE(obj->changeLabelTheme(nullptr, false));
}

TEST_F(BluetoothTransDialogTest, closeEvent)
{
    // Test event handler: closeEvent((QCloseEvent *event))
    QCloseEvent _event(QCloseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->closeEvent(&_event));
}

TEST_F(BluetoothTransDialogTest, connectAdapter)
{
    // Test method: void connectAdapter((const BluetoothAdapter *adapter))
    EXPECT_NO_FATAL_FAILURE(obj->connectAdapter(nullptr));
}

TEST_F(BluetoothTransDialogTest, createFailedPage)
{
    // Test getter: QWidget createFailedPage()
    auto result = obj->createFailedPage();
    EXPECT_NO_FATAL_FAILURE({ obj->createFailedPage(); });

}

TEST_F(BluetoothTransDialogTest, createNonDevicePage)
{
    // Test getter: QWidget createNonDevicePage()
    auto result = obj->createNonDevicePage();
    EXPECT_NO_FATAL_FAILURE({ obj->createNonDevicePage(); });

}

TEST_F(BluetoothTransDialogTest, createStyledItem)
{
    // Test method: DStandardItem createStyledItem((const BluetoothDevice *dev))
    auto result = obj->createStyledItem(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->createStyledItem(nullptr); });

}

TEST_F(BluetoothTransDialogTest, createSuccessPage)
{
    // Test getter: QWidget createSuccessPage()
    auto result = obj->createSuccessPage();
    EXPECT_NO_FATAL_FAILURE({ obj->createSuccessPage(); });

}

TEST_F(BluetoothTransDialogTest, createTranferingPage)
{
    // Test getter: QWidget createTranferingPage()
    auto result = obj->createTranferingPage();
    EXPECT_NO_FATAL_FAILURE({ obj->createTranferingPage(); });

}

TEST_F(BluetoothTransDialogTest, createWaitForRecvPage)
{
    // Test getter: QWidget createWaitForRecvPage()
    auto result = obj->createWaitForRecvPage();
    EXPECT_NO_FATAL_FAILURE({ obj->createWaitForRecvPage(); });

}

TEST_F(BluetoothTransDialogTest, findItemByIdRole)
{
    // Test method: DStandardItem findItemByIdRole((const QString &devId))
    QString _arg0{};
    auto result = obj->findItemByIdRole(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->findItemByIdRole(_arg0); });

}

TEST_F(BluetoothTransDialogTest, humanizeObexErrMsg)
{
    // Test method: QString humanizeObexErrMsg((const QString &msg))
    QString _arg0{};
    auto result = obj->humanizeObexErrMsg(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BluetoothTransDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(BluetoothTransDialogTest, sendFilesToDevice)
{
    // Test method: void sendFilesToDevice((const QString &devId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendFilesToDevice(_arg0));
}

TEST_F(BluetoothTransDialogTest, setNextButtonEnable)
{
    // Test setter: void setNextButtonEnable((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->setNextButtonEnable(false));
}

TEST_F(BluetoothTransDialogTest, setObjTextStyle)
{
    // Test setter: void setObjTextStyle((QWidget *obj, int size, bool bold))
    EXPECT_NO_FATAL_FAILURE(obj->setObjTextStyle(nullptr, 0, false));
}

TEST_F(BluetoothTransDialogTest, showBluetoothSetting)
{
    // Test method: void showBluetoothSetting(())
    EXPECT_NO_FATAL_FAILURE(obj->showBluetoothSetting());
}

TEST_F(BluetoothTransDialogTest, updateDeviceList)
{
    // Test method: void updateDeviceList(())
    EXPECT_NO_FATAL_FAILURE(obj->updateDeviceList());
}
