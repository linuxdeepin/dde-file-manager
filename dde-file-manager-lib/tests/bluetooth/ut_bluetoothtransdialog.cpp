#include <gtest/gtest.h>
#include <QApplication>
#include <QStackedWidget>
#include <QTimer>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

#define private public
#define protected public

#include "bluetooth/bluetoothtransdialog.h"
#include "bluetooth/bluetoothmanager.h"

namespace {
class TestBluetoothTransDialog : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestBluetoothTransDialog";
        dlg = new BluetoothTransDialog(QStringList() << "/usr/bin/dde-desktop", BluetoothTransDialog::DirectlySend,
                                       "12-23-34-45-56");
    }

    void TearDown() override
    {
        std::cout << "end TestBluetoothTransDialog";
        delete dlg;
        dlg = nullptr;
    }

public:
    BluetoothTransDialog *dlg = nullptr;
};
} // namespace

TEST_F(TestBluetoothTransDialog, ShowDialog)
{
    dlg->show();
    dlg->hide();
}

TEST_F(TestBluetoothTransDialog, tstHumanizedStrOfObexErrMsg)
{
    QString str("Timed out");
    EXPECT_TRUE(qApp->translate("BluetoothTransDialog", "File sending request timed out") == dlg->humanizedStrOfObexErrMsg(str));
    str = "0x53";
    EXPECT_TRUE(qApp->translate("BluetoothTransDialog", "The service is busy and unable to process the request") == dlg->humanizedStrOfObexErrMsg(str));
    str = "other";
    EXPECT_TRUE(str == dlg->humanizedStrOfObexErrMsg(str));
}

TEST_F(TestBluetoothTransDialog, tstSetObjTextStyle)
{
    dlg->setObjTextStyle(nullptr, 10, true);
    dlg->changeLabelTheme(nullptr, true);
}

TEST_F(TestBluetoothTransDialog, tstCanSendFiles)
{
    EXPECT_TRUE(dlg->canSendFiles());
}

TEST_F(TestBluetoothTransDialog, tstLambdaSlots)
{
    dlg->m_currSessionPath = "123";
    BluetoothManager::instance()->transferProgressUpdated("1234", 123, 1234, 0);

    BluetoothManager::instance()->transferProgressUpdated("123", 12, 1234, 0);

    dlg->m_progressUpdateShouldBeIgnore = true;
    BluetoothManager::instance()->transferProgressUpdated("123", 123, 12, 0);
    dlg->m_progressUpdateShouldBeIgnore = false;

    dlg->m_firstTransSize = 123;
    BluetoothManager::instance()->transferProgressUpdated("123", 1234, 123, 0);

    dlg->m_stack->setCurrentIndex(BluetoothTransDialog::SuccessPage);
    BluetoothManager::instance()->transferProgressUpdated("123", 1234, 1234, 0);

    QEventLoop loop;
    QTimer::singleShot(1500, nullptr, [&loop]{
        loop.exit();
    });
    loop.exec();

    BluetoothManager::instance()->transferCancledByRemote("123");
    BluetoothManager::instance()->transferCancledByRemote("1234");

    BluetoothManager::instance()->transferFailed("123", "123", "123");
    BluetoothManager::instance()->transferFailed("1234", "123", "123");

    dlg->m_urls << "test";
    dlg->m_finishedUrls.clear();
    BluetoothManager::instance()->fileTransferFinished("123", "123");
    BluetoothManager::instance()->fileTransferFinished("1234", "123");

    BluetoothManager::instance()->transferEstablishFinish("123", "11234");
    BluetoothManager::instance()->transferEstablishFinish("", "11234");

    DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::UnknownType);
}
