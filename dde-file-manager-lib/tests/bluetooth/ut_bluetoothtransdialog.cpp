#include <gtest/gtest.h>
#include <QApplication>
#include <QStackedWidget>
#include <QTimer>
#include <DGuiApplicationHelper>
#include <DListView>

#include "stub.h"

DGUI_USE_NAMESPACE

#define private public
#define protected public

#include "bluetooth/bluetoothtransdialog.h"
#include "bluetooth/bluetoothmanager.h"
#include "bluetooth/bluetoothadapter.h"
#include "bluetooth/bluetoothmodel.h"

namespace BluetoothUnitTest {
    QMap<QString, const BluetoothAdapter *> (*adapters_stub)(void *) = [](void *) {
        BluetoothAdapter *adapter = new BluetoothAdapter;
        adapter->setId("1");
        adapter->setName("local");
        adapter->setPowered(true);

        BluetoothDevice *dev = new BluetoothDevice;
        dev->setId("1234");
        dev->setIcon("phone");
        adapter->addDevice(dev);

        QMap<QString, const BluetoothAdapter *> map;
        map.insert(adapter->id(), adapter);
        return map;
    };
}

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

    dlg->m_currSessionPath = "123";
    BluetoothManager::instance()->transferCancledByRemote("123");
    dlg->m_currSessionPath = "123";
    BluetoothManager::instance()->transferCancledByRemote("1234");

    dlg->m_currSessionPath = "123";
    BluetoothManager::instance()->transferFailed("123", "123", "123");
    dlg->m_currSessionPath = "123";
    BluetoothManager::instance()->transferFailed("1234", "123", "123");

    dlg->m_urls << "test";
    dlg->m_finishedUrls.clear();
    dlg->m_currSessionPath = "123";
    BluetoothManager::instance()->fileTransferFinished("123", "123");
    dlg->m_currSessionPath = "123";
    BluetoothManager::instance()->fileTransferFinished("1234", "123");

    BluetoothManager::instance()->transferEstablishFinish("123", "11234");
    BluetoothManager::instance()->transferEstablishFinish("", "11234");

    DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::UnknownType);
    DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::DarkType);
}

TEST_F(TestBluetoothTransDialog, tstFindItemByIdRole)
{
    BluetoothDevice *dev = nullptr;
    EXPECT_TRUE(nullptr == dlg->findItemByIdRole(dev));
    EXPECT_TRUE(nullptr == dlg->findItemByIdRole("testNullptr"));
}

TEST_F(TestBluetoothTransDialog, tstSendFilesToDevice)
{
    void (*sendFiles_stub)(void *, QString, QStringList) = [](void *, QString, QStringList) {;};
    Stub st;
    st.set(ADDR(BluetoothModel, adapters), BluetoothUnitTest::adapters_stub);
    st.set(static_cast<void(BluetoothManager::*)(const QString &, const QStringList &)>(&BluetoothManager::sendFiles), sendFiles_stub);

    dlg->m_finishedUrls << "/usr/bin/dde-file-manager";
    dlg->m_urls << "/usr/bin/dde-file-manager" << "/usr/bin/dde-desktop";
    dlg->sendFilesToDevice("1234");
    dlg->sendFilesToDevice("12345");

    dlg->initConn();
    emit BluetoothManager::instance()->model()->adapterAdded(nullptr);
    dlg->updateDeviceList();

    BluetoothDevice *dev = new BluetoothDevice;
    dev->setId("testDev");
    dlg->addDevice(dev);
    dlg->removeDevice("testDev");

    BluetoothAdapter *adapter = new BluetoothAdapter;
    adapter->setId("1");
    adapter->addDevice(dev);
    BluetoothManager::instance()->model()->adapterRemoved(adapter);
    BluetoothManager::instance()->model()->adapterRemoved(nullptr);

    dlg->getStyledItem(nullptr);
    dev->setPaired(true);
    dev->setIcon("phone");
    dev->setState(BluetoothDevice::StateConnected);
    dlg->getStyledItem(dev);

    delete dev;
    delete adapter;

    dlg->close();
}
