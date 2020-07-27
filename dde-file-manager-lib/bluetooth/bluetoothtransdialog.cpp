#include "bluetoothtransdialog.h"
#include "dguiapplicationhelper.h"
#include "bluetooth/bluetoothmanager.h"
#include "bluetooth/bluetoothadapter.h"
#include "bluetooth/bluetoothmodel.h"
#include "app/define.h"

#include <QStackedWidget>
#include <QVBoxLayout>
#include <DLabel>
#include <DListView>
#include <QStandardItem>
#include <DCommandLinkButton>
#include <DSpinner>
#include <DProgressBar>
#include <QFont>
#include <QSpacerItem>

#include <QDebug>
#include <QTimer>

#define TITLE_BT_TRANS_FILE BluetoothTransDialog::tr("Bluetooth File Transfer")
#define TITLE_BT_TRANS_SUCC BluetoothTransDialog::tr("File Transfer Successful")
#define TITLE_BT_TRANS_FAIL BluetoothTransDialog::tr("File Transfer Failed")

#define TXT_SENDING_FILE BluetoothTransDialog::tr("Sending files to \"%1\"")
#define TXT_SENDING_FAIL BluetoothTransDialog::tr("Failed to send files to \"%1\"")
#define TXT_SENDING_SUCC BluetoothTransDialog::tr("Sent to \"%1\" successfully")
#define TXT_SELECT_DEVIC BluetoothTransDialog::tr("Select a Bluetooth device to receive files")
#define TXT_NO_DEV_FOUND BluetoothTransDialog::tr("Cannot find the connected Bluetooth device")
#define TXT_WAIT_FOR_RCV BluetoothTransDialog::tr("Waiting to be received...")
#define TXT_GOTO_BT_SETS BluetoothTransDialog::tr("Go to Bluetooth Settings")
#define TXT_SEND_PROGRES BluetoothTransDialog::tr("%1/%2 Sent")
#define TXT_ERROR_REASON BluetoothTransDialog::tr("Error: the Bluetooth device is disconnected")

#define TXT_NEXT BluetoothTransDialog::tr("Next")
#define TXT_CANC BluetoothTransDialog::tr("Cancel")
#define TXT_DONE BluetoothTransDialog::tr("Done")
#define TXT_RTRY BluetoothTransDialog::tr("Retry")

#define ICON_CONNECT "notification-bluetooth-connected"
#define ICON_DISCONN "notification-bluetooth-disconnected"

#define PXMP_NO_DEV_LIGHT "://icons/deepin/builtin/light/icons/dfm_bluetooth_empty_light.svg"
#define PXMP_NO_DEV_DARKY "://icons/deepin/builtin/dark/icons/dfm_bluetooth_empty_dark.svg"

BluetoothTransDialog::BluetoothTransDialog(const QStringList &urls, BluetoothTransDialog::TransferMode mode, QString targetDevId, QWidget *parent)
    : DDialog(parent)
    , m_urls(urls)
{
    // 测试进度条
    timer = new QTimer(this);

    initUI();
    initConn();
    updateDeviceList(); // 打开多个窗口的时候蓝牙设备不一定任何更新操作，因此这时依靠蓝牙状态的变更去更新列表不可取，手动获取一次列表
    bluetoothManager->refresh();

    if (mode == DirectlySend)
        sendFilesToDevice(targetDevId);

    // 调试布局
    //    setStyleSheet("border: 1px solid blue;");
}

void BluetoothTransDialog::sendFilesToDevice(const QString &devId)
{
    const BluetoothDevice *dev = nullptr;
    QMapIterator<QString, const BluetoothAdapter *> iter(bluetoothManager->model()->adapters());
    while (iter.hasNext()) {
        iter.next();
        dev = (iter.value()->deviceById(devId));
        if (dev)
            break;
    }

    if (!dev) {
        qDebug() << "can not find device: " << devId;
    } else {
        m_selectedDevice = dev->alias();
        m_selectedDeviceId = devId;
        sendFiles();
    }
}

void BluetoothTransDialog::initUI()
{
    setIcon(QIcon::fromTheme(ICON_CONNECT));
    setFixedSize(381, 271);

    // main structure
    QFrame *mainFrame = new QFrame(this);
    QVBoxLayout *pLayout = new QVBoxLayout(mainFrame);
    pLayout->setSpacing(0);
    pLayout->setMargin(0);

    mainFrame->setLayout(pLayout);
    addContent(mainFrame);

    // public title
    m_titleOfDialog = new DLabel(TITLE_BT_TRANS_FILE, this);
    QFont fnt = m_titleOfDialog->font();
    fnt.setBold(true);
    fnt.setPixelSize(14);
    m_titleOfDialog->setFont(fnt);
    m_titleOfDialog->setAlignment(Qt::AlignCenter);
    pLayout->addWidget(m_titleOfDialog);

    // stacked area
    m_stack = new QStackedWidget(this);
    connect(m_stack, &QStackedWidget::currentChanged, this, &BluetoothTransDialog::onPageChagned);

    pLayout->addWidget(m_stack);

    // 以下顺序固定以便进行枚举遍历
    m_stack->addWidget(initDeviceSelectorPage());
    m_stack->addWidget(initNonDevicePage());
    m_stack->addWidget(initWaitForRecvPage());
    m_stack->addWidget(initTranferingPage());
    m_stack->addWidget(initFailedPage());
    m_stack->addWidget(initSuccessPage());

    m_stack->setCurrentIndex(NoneDevicePage); // 初始界面为空界面

    setOnButtonClickedClose(false);

    connect(this, &BluetoothTransDialog::buttonClicked, this, &BluetoothTransDialog::onBtnClicked);
}

void BluetoothTransDialog::initConn()
{
    QMap<QString, const BluetoothAdapter *> adapters = bluetoothManager->model()->adapters();
    QMapIterator<QString, const BluetoothAdapter *> iter(adapters);
    while (iter.hasNext()) {
        iter.next();
        const BluetoothAdapter *adapter = iter.value();
        connectAdapter(adapter);
    }

    connect(bluetoothManager->model(), &BluetoothModel::adapterAdded, this, [=](const BluetoothAdapter *adapter) {
        connectAdapter(adapter);
    });

    connect(bluetoothManager->model(), &BluetoothModel::adapterRemoved, this, [this](const BluetoothAdapter *adapter) {
        if (m_connectedAdapter.contains(adapter->id()))
            m_connectedAdapter.removeAll(adapter->id());

        adapter->disconnect();
        QMap<QString, const BluetoothDevice *> devices = adapter->devices();
        QMapIterator<QString, const BluetoothDevice *> iter(devices);
        while (iter.hasNext()) {
            iter.next();
            iter.value()->disconnect();
        }
    });
}

QWidget *BluetoothTransDialog::initDeviceSelectorPage()
{
    // device selector page
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLayout = new QVBoxLayout(w);
    w->setLayout(pLayout);

    DLabel *statusTxt = new DLabel(TXT_SELECT_DEVIC, this);
    statusTxt->setAlignment(Qt::AlignCenter);
    QFont f = statusTxt->font();
    f.setPixelSize(14);
    statusTxt->setFont(f);
    pLayout->addWidget(statusTxt);
    m_devicesList = new DListView(this);
    m_devModel = new QStandardItemModel(this);
    m_devicesList->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_devicesList->setEditTriggers(QListView::NoEditTriggers);
    m_devicesList->setIconSize(QSize(32, 32));
    m_devicesList->setResizeMode(QListView::Adjust);
    m_devicesList->setMovement(QListView::Static);
    m_devicesList->setSelectionMode(QListView::NoSelection);
    m_devicesList->setFrameShape(QFrame::NoFrame);
    m_devicesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_devicesList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_devicesList->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    m_devicesList->setViewportMargins(0, 0, 0, 0);
    m_devicesList->setItemSpacing(1);
    m_devicesList->setModel(m_devModel);
    connect(m_devicesList, &DListView::clicked, this, [this](const QModelIndex &curr) {
        for (int i = 0; i < m_devModel->rowCount(); i++) {
            DStandardItem *item = dynamic_cast<DStandardItem *>(m_devModel->item(i));
            if (!item)
                continue;
            if (i == curr.row()) {
                item->setCheckState(Qt::Checked);
                m_selectedDevice = item->text();
                m_selectedDeviceId = item->data(DevIdRole).toString();
            } else
                item->setCheckState(Qt::Unchecked);
        }
    });
    pLayout->addWidget(m_devicesList);

    DCommandLinkButton *linkBtn = new DCommandLinkButton(TXT_GOTO_BT_SETS, this);
    connect(linkBtn, &DCommandLinkButton::clicked, this, &BluetoothTransDialog::showBluetoothSetting);
    QHBoxLayout *pLay = new QHBoxLayout(this);
    pLay->setMargin(0);
    pLay->setSpacing(0);
    pLay->addStretch(1);
    pLay->addWidget(linkBtn);
    pLayout->addLayout(pLay);
    pLayout->setStretch(1, 1);

    return w;
}

QWidget *BluetoothTransDialog::initNonDevicePage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    w->setLayout(pLay);

    DLabel *statusTxt = new DLabel(TXT_NO_DEV_FOUND, this);
    statusTxt->setAlignment(Qt::AlignCenter);
    QFont f = statusTxt->font();
    f.setPixelSize(14);
    statusTxt->setFont(f);
    pLay->addWidget(statusTxt);

    DCommandLinkButton *linkBtn = new DCommandLinkButton(TXT_GOTO_BT_SETS, this);
    connect(linkBtn, &DCommandLinkButton::clicked, this, &BluetoothTransDialog::showBluetoothSetting);
    QHBoxLayout *pHLay = new QHBoxLayout(w);
    pHLay->addStretch(1);
    pHLay->addWidget(linkBtn);
    pHLay->addStretch(1);
    pLay->addLayout(pHLay);

    DLabel *pIconLab = new DLabel(this);
    pIconLab->setAlignment(Qt::AlignCenter);
    pIconLab->setPixmap(QPixmap(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType
                                    ? PXMP_NO_DEV_DARKY
                                    : PXMP_NO_DEV_LIGHT));
    pLay->addWidget(pIconLab);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [pIconLab](DGuiApplicationHelper::ColorType t) {
        switch (t) {
        case DGuiApplicationHelper::UnknownType:
        case DGuiApplicationHelper::LightType:
            pIconLab->setPixmap(QPixmap(PXMP_NO_DEV_LIGHT));
            break;
        case DGuiApplicationHelper::DarkType:
            pIconLab->setPixmap(QPixmap(PXMP_NO_DEV_DARKY));
            break;
        }
    });

    return w;
}

QWidget *BluetoothTransDialog::initWaitForRecvPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    w->setLayout(pLay);

    m_subTitleForWaitPage = new DLabel("Sending files to ...");
    m_subTitleForWaitPage->setAlignment(Qt::AlignCenter);
    QFont f = m_subTitleForWaitPage->font();
    f.setPixelSize(14);
    m_subTitleForWaitPage->setFont(f);
    pLay->addWidget(m_subTitleForWaitPage);

    DSpinner *spinner = new DSpinner(this);
    pLay->addWidget(spinner);
    connect(this, &BluetoothTransDialog::startSpinner, spinner, &DSpinner::start);
    connect(this, &BluetoothTransDialog::stopSpinner, spinner, &DSpinner::stop);

    DLabel *txt2 = new DLabel(TXT_WAIT_FOR_RCV, this);
    txt2->setAlignment(Qt::AlignCenter);
    f.setPixelSize(12);
    txt2->setFont(f);
    pLay->addWidget(txt2);

    return w;
}

QWidget *BluetoothTransDialog::initTranferingPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    w->setLayout(pLay);

    m_subTitleOfTransPage = new DLabel("Sending files to ...");
    m_subTitleOfTransPage->setAlignment(Qt::AlignCenter);
    QFont f = m_subTitleOfTransPage->font();
    f.setPixelSize(14);
    m_subTitleOfTransPage->setFont(f);
    pLay->addWidget(m_subTitleOfTransPage);

    DProgressBar *progress = new DProgressBar(this);
    progress->setValue(0);
    progress->setMaximum(100);
    progress->setMaximumHeight(8);
    pLay->addWidget(progress);
    connect(timer, &QTimer::timeout, this, [progress, this] {
        progress->setValue(progress->value() + 4);
        qDebug() << progress->value();
        if (progress->value() == progress->maximum()) {
            m_stack->setCurrentIndex(SuccessPage);
        }
    });
    connect(this, &BluetoothTransDialog::resetProgress, this, [progress] {
        progress->setValue(0);
    });

    m_sendingStatus = new DLabel(TXT_SEND_PROGRES, this);
    m_sendingStatus->setAlignment(Qt::AlignCenter);
    f.setPixelSize(12);
    m_sendingStatus->setFont(f);
    pLay->addWidget(m_sendingStatus);

    return w;
}

QWidget *BluetoothTransDialog::initFailedPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    w->setLayout(pLay);

    m_subTitleOfFailedPage = new DLabel("Failed to send files to ...");
    m_subTitleOfFailedPage->setAlignment(Qt::AlignCenter);
    QFont f = m_subTitleOfFailedPage->font();
    f.setPixelSize(14);
    m_subTitleOfFailedPage->setFont(f);
    pLay->addWidget(m_subTitleOfFailedPage);

    DLabel *txt2 = new DLabel(TXT_ERROR_REASON, this);
    txt2->setAlignment(Qt::AlignCenter);
    f.setPixelSize(12);
    txt2->setFont(f);
    pLay->addWidget(txt2);

    return w;
}

QWidget *BluetoothTransDialog::initSuccessPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    w->setLayout(pLay);

    m_subTitleOfSuccessPage = new DLabel("Sent to ... successfully");
    m_subTitleOfSuccessPage->setAlignment(Qt::AlignCenter);
    QFont f = m_subTitleOfSuccessPage->font();
    f.setPixelSize(14);
    m_subTitleOfSuccessPage->setFont(f);
    pLay->addWidget(m_subTitleOfSuccessPage);

    return w;
}

DStandardItem *BluetoothTransDialog::getStyledItem(const BluetoothDevice *dev)
{
    // 只有已配对、已信任且状态为已连接的设备才显示在设备列表中
    if (!(dev->paired() && dev->trusted() && dev->state() == BluetoothDevice::StateConnected))
        return nullptr;

    if (findItemByIdRole(dev)) // 列表中已有此设备
        return nullptr;

    DViewItemActionList actLst;
    DViewItemAction *act = new DViewItemAction(Qt::AlignVCenter | Qt::AlignLeft, QSize(22, 22), QSize(), false);
    actLst.append(act);
    act->setIcon(QIcon::fromTheme(dev->icon()));

    DStandardItem *item = new DStandardItem();
    item->setData(dev->id(), DevIdRole);
    item->setText(dev->alias());
    item->setActionList(Qt::LeftEdge, actLst);
    QFont f = item->font();
    f.setPixelSize(12);
    item->setFont(f);
    return item;
}

DStandardItem *BluetoothTransDialog::findItemByIdRole(const BluetoothDevice *dev)
{
    return findItemByIdRole(dev->id());
}

DStandardItem *BluetoothTransDialog::findItemByIdRole(const QString &devId)
{
    const QString &id = devId;
    for (int i = 0; i < m_devModel->rowCount(); i++) {
        if (id == m_devModel->data(m_devModel->index(i, 0), DevIdRole).toString())
            return dynamic_cast<DStandardItem *>(m_devModel->item(i));
    }
    return nullptr;
}

void BluetoothTransDialog::updateDeviceList()
{
    if (!m_devicesList)
        return;

    QMap<QString, const BluetoothAdapter *> adapters = bluetoothManager->model()->adapters();
    QMapIterator<QString, const BluetoothAdapter *> iter(adapters);
    while (iter.hasNext()) {
        iter.next();
        const BluetoothAdapter *adapter = iter.value();
        QMap<QString, const BluetoothDevice *> devices = adapter->devices();
        QMapIterator<QString, const BluetoothDevice *> iterOfDev(devices);
        while (iterOfDev.hasNext()) {
            iterOfDev.next();
            const BluetoothDevice *dev = iterOfDev.value();
            connectDevice(dev);
            addDevice(dev);
        }
    }
}

void BluetoothTransDialog::addDevice(const BluetoothDevice *dev)
{
    DStandardItem *item = getStyledItem(dev);
    if (!item)
        return;

    m_devModel->appendRow(item);
    if (m_stack->currentIndex() == NoneDevicePage) // 仅当页面位于无设备页面时执行跳转
        m_stack->setCurrentIndex(SelectDevicePage);
}

void BluetoothTransDialog::removeDevice(const BluetoothDevice *dev)
{
    removeDevice(dev->id());
}

void BluetoothTransDialog::removeDevice(const QString &id)
{
    for (int i = 0; i < m_devModel->rowCount(); i++) {
        if (m_devModel->data(m_devModel->index(i, 0), DevIdRole).toString() == id) {
            m_devModel->removeRow(i);
            if (m_devModel->rowCount() == 0 && m_stack->currentIndex() == SelectDevicePage)
                m_stack->setCurrentIndex(NoneDevicePage);
            return;
        }
    }
}

void BluetoothTransDialog::sendFiles()
{
    m_selectedDevice;
    m_urls;

    m_subTitleForWaitPage->setText(TXT_SENDING_FILE.arg(m_selectedDevice));
    m_subTitleOfTransPage->setText(TXT_SENDING_FILE.arg(m_selectedDevice));
    m_subTitleOfFailedPage->setText(TXT_SENDING_FAIL.arg(m_selectedDevice));
    m_subTitleOfSuccessPage->setText(TXT_SENDING_SUCC.arg(m_selectedDevice));

    m_stack->setCurrentIndex(WaitForRecvPage);
    Q_EMIT startSpinner();

    bluetoothManager->sendFile(m_selectedDeviceId, m_urls.first());
}

void BluetoothTransDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
}

void BluetoothTransDialog::showBluetoothSetting()
{
    bluetoothManager->showBluetoothSettings();
}

void BluetoothTransDialog::onBtnClicked(const int &nIdx)
{
    Page currpage = static_cast<Page>(m_stack->currentIndex());
    switch (currpage) {
    case SelectDevicePage:
        if (m_selectedDevice.isEmpty() && nIdx == 1)
            return;
        if (nIdx == 1)
            sendFiles();
        else
            close();
        break;
    case FailedPage:
        if (nIdx == 1)
            sendFiles();
        else
            close();
        break;
    case WaitForRecvPage:
    case NoneDevicePage:
    case TransferPage:
    case SuccessPage:
        close();
        break;
    }
}

void BluetoothTransDialog::onPageChagned(const int &nIdx)
{
    Q_EMIT stopSpinner();
    setIcon(QIcon::fromTheme(ICON_CONNECT));
    m_titleOfDialog->setText(TITLE_BT_TRANS_FILE);
    clearButtons();

    Page currpage = static_cast<Page>(nIdx);
    switch (currpage) {
    case SelectDevicePage:
        addButton(TXT_CANC);
        addButton(TXT_NEXT, true, ButtonType::ButtonRecommend);
        break;
    case NoneDevicePage:
    case WaitForRecvPage:
    case TransferPage:
        addButton(TXT_CANC);
        break;
    case FailedPage:
        m_titleOfDialog->setText(TITLE_BT_TRANS_FAIL);
        setIcon(QIcon::fromTheme(ICON_DISCONN));
        addButton(TXT_CANC);
        addButton(TXT_RTRY, true, ButtonType::ButtonRecommend);
        break;
    case SuccessPage:
        m_titleOfDialog->setText(TITLE_BT_TRANS_SUCC);
        addButton(TXT_DONE);
        break;
    }

    // 测试，模拟接收方已同意接收文件
    if (currpage == WaitForRecvPage) {
        QTimer::singleShot(1000, nullptr, [this] {
            if (!m_stack)
                return;
            m_stack->setCurrentIndex(TransferPage);
        });
    }

    // 进度条测试
    if (currpage == TransferPage) {
        Q_EMIT resetProgress();
        timer->start(100);
    } else {
        timer->stop();
    }
}

void BluetoothTransDialog::connectAdapter(const BluetoothAdapter *adapter)
{
    if (m_connectedAdapter.contains(adapter->id()))
        return;
    m_connectedAdapter.append(adapter->id());

    connect(adapter, &BluetoothAdapter::deviceAdded, this, [this](const BluetoothDevice *dev) {
        addDevice(dev);
        connectDevice(dev);
    });

    connect(adapter, &BluetoothAdapter::deviceRemoved, this, [this](const QString &deviceId) {
        removeDevice(deviceId);
    });
}

void BluetoothTransDialog::connectDevice(const BluetoothDevice *dev)
{
    connect(dev, &BluetoothDevice::stateChanged, this, [this](const BluetoothDevice::State &state) {
        BluetoothDevice *device = dynamic_cast<BluetoothDevice *>(sender());
        if (!device)
            return;

        switch (state) {
        case BluetoothDevice::StateConnected:
            addDevice(device);
            break;
        default:
            removeDevice(device);
            break;
        }
    });
}
