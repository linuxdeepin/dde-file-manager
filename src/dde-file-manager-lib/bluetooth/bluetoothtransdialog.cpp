/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bluetoothtransdialog.h"
#include "dguiapplicationhelper.h"
#include "bluetooth/bluetoothmanager.h"
#include "bluetooth/bluetoothadapter.h"
#include "bluetooth/bluetoothmodel.h"
#include "app/define.h"
#include "dfileservices.h"
#include "dialogs/dialogmanager.h"

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
#include <QPointer>
#include <QDebug>
#include <QTimer>
#include <QPalette>
#include <QSvgWidget>
#include <QUuid>

#define TITLE_BT_TRANS_FILE BluetoothTransDialog::tr("Bluetooth File Transfer")
#define TITLE_BT_TRANS_SUCC BluetoothTransDialog::tr("File Transfer Successful")
#define TITLE_BT_TRANS_FAIL BluetoothTransDialog::tr("File Transfer Failed")

#define TXT_SENDING_FILE BluetoothTransDialog::tr("Sending files to \"<b style=\"font-weight: 550;\">%1</b>\"")
#define TXT_SENDING_FAIL BluetoothTransDialog::tr("Failed to send files to \"<b style=\"font-weight: 550;\">%1</b>\"")
#define TXT_SENDING_SUCC BluetoothTransDialog::tr("Sent to \"<b style=\"font-weight: 550;\">%1</b>\" successfully")
#define TXT_SELECT_DEVIC BluetoothTransDialog::tr("Select a Bluetooth device to receive files")
#define TXT_NO_DEV_FOUND BluetoothTransDialog::tr("Cannot find the connected Bluetooth device")
#define TXT_WAIT_FOR_RCV BluetoothTransDialog::tr("Waiting to be received...")
#define TXT_GOTO_BT_SETS BluetoothTransDialog::tr("Go to Bluetooth Settings")
#define TXT_SEND_PROGRES BluetoothTransDialog::tr("%1/%2 Sent")
#define TXT_ERROR_REASON BluetoothTransDialog::tr("Error: the Bluetooth device is disconnected")
#define TXT_FILE_OVERSIZ BluetoothTransDialog::tr("Unable to send the file more than 2 GB")
#define TXT_FILE_ZEROSIZ BluetoothTransDialog::tr("Unable to send 0 KB files")
#define TXT_FILE_NOEXIST BluetoothTransDialog::tr("File doesn't exist")

#define TXT_NEXT BluetoothTransDialog::tr("Next","button")
#define TXT_CANC BluetoothTransDialog::tr("Cancel","button")
#define TXT_DONE BluetoothTransDialog::tr("Done","button")
#define TXT_RTRY BluetoothTransDialog::tr("Retry","button")
#define TXT_OKAY BluetoothTransDialog::tr("OK","button")

static const QString ICON_CONNECT = "notification-bluetooth-connected";
static const QString ICON_DISCONN = "notification-bluetooth-disconnected";

static const QString lightIcon = ":/icons/deepin/builtin/light/icons/bluetooth_";
static const QString darkIcon = ":/icons/deepin/builtin/dark/icons/bluetooth_";

static const QString PXMP_NO_DEV_LIGHT = "://icons/deepin/builtin/light/icons/dfm_bluetooth_empty_light.svg";
static const QString PXMP_NO_DEV_DARKY = "://icons/deepin/builtin/dark/icons/dfm_bluetooth_empty_dark.svg";

static const qint64 FILE_TRANSFER_LIMITS = 2147483648; // 2GB = 2 * 1024 * 1024 * 1024 Bytes

BluetoothTransDialog::BluetoothTransDialog(const QStringList &urls, BluetoothTransDialog::TransferMode mode, QString targetDevId, QWidget *parent)
    : DDialog(parent)
    , m_urls(urls)
    , m_token(QUuid::createUuid().toString())
{
    initUI();
    initConn();
    m_stack->setCurrentIndex(NoneDevicePage); // 初始界面为空界面

    updateDeviceList(); // 打开多个窗口的时候蓝牙设备不一定任何更新操作，因此这时依靠蓝牙状态的变更去更新列表不可取，手动获取一次列表
    bluetoothManager->refresh();

    if (mode == DirectlySend)
        sendFilesToDevice(targetDevId);

    // 调试布局
    // setStyleSheet("border: 1px solid blue;");
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

void BluetoothTransDialog::changeLabelTheme(QLabel *obj, bool isTitle)
{
    if (!obj)
        return;

    double alpha = isTitle ? 0.9 : 0.7;
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    obj, [obj, alpha](DGuiApplicationHelper::ColorType themeType) {
        QPalette pal = obj->palette();
        pal.setColor(QPalette::WindowText, themeType == DGuiApplicationHelper::DarkType
                     ? QColor::fromRgbF(1, 1, 1, alpha)
                     : QColor::fromRgbF(0, 0, 0, alpha));
        obj->setPalette(pal);
    });
}

void BluetoothTransDialog::setObjTextStyle(QWidget *obj, int size, bool bold)
{
    if (!obj)
        return;
    QFont f = obj->font();
    f.setFamily("SourceHanSansSC");
    f.setPixelSize(size);
    f.setWeight(bold ? QFont::Medium : QFont::Normal);
    f.setStyle(QFont::StyleNormal);
    obj->setFont(f);
}

QString BluetoothTransDialog::humanizedStrOfObexErrMsg(const QString &msg)
{
    if (msg.contains("Timed out")) {
        return tr("File sending request timed out");
    } else if (msg.contains("0x53")) {
        return tr("The service is busy and unable to process the request");
    } else { // ...TO BE CONTINUE
        return msg;
    }
}

bool BluetoothTransDialog::canSendFiles()
{
    return bluetoothManager->canSendBluetoothRequest();
}

void BluetoothTransDialog::initUI()
{
    setIcon(QIcon::fromTheme(ICON_CONNECT));
    setFixedSize(381, 271);
    layout()->setMargin(0);
    layout()->setSpacing(0);

    // main structure
    QFrame *mainFrame = new QFrame(this);
    QVBoxLayout *pLayout = new QVBoxLayout;
    pLayout->setSpacing(0);
    pLayout->setMargin(0);
    mainFrame->setLayout(pLayout);

    addContent(mainFrame);

    // public title
    m_titleOfDialog = new DLabel(TITLE_BT_TRANS_FILE, this);
    m_titleOfDialog->setAlignment(Qt::AlignCenter);
    setObjTextStyle(m_titleOfDialog, 14, true);
    changeLabelTheme(m_titleOfDialog, true);
    pLayout->addWidget(m_titleOfDialog);

    // stacked area
    m_stack = new QStackedWidget(this);
    m_stack->layout()->setMargin(0);
    m_stack->layout()->setSpacing(0);

    pLayout->addWidget(m_stack);

    // 以下顺序固定以便进行枚举遍历
    m_stack->addWidget(initDeviceSelectorPage());
    m_stack->addWidget(initNonDevicePage());
    m_stack->addWidget(initWaitForRecvPage());
    m_stack->addWidget(initTranferingPage());
    m_stack->addWidget(initFailedPage());
    m_stack->addWidget(initSuccessPage());

    setOnButtonClickedClose(false);

    // 防止窗口初始化的时候字体颜色不更改，手动触发一次该信号
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::instance()->themeType());
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

    connect(m_stack, &QStackedWidget::currentChanged, this, &BluetoothTransDialog::onPageChagned);
    connect(this, &BluetoothTransDialog::buttonClicked, this, &BluetoothTransDialog::onBtnClicked);

    connect(m_devicesList, &DListView::clicked, this, [this](const QModelIndex & curr) {
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

    connect(bluetoothManager->model(), &BluetoothModel::adapterAdded, this, [this](const BluetoothAdapter *adapter) {
        connectAdapter(adapter);
    });

    connect(bluetoothManager->model(), &BluetoothModel::adapterRemoved, this, [this](const BluetoothAdapter *adapter) {
        if (!adapter)
            return;
        if (m_connectedAdapter.contains(adapter->id()))
            m_connectedAdapter.removeAll(adapter->id());

        adapter->disconnect();
        QMap<QString, const BluetoothDevice *> devices = adapter->devices();
        QMapIterator<QString, const BluetoothDevice *> it(devices);
        while (it.hasNext()) {
            it.next();
            it.value()->disconnect();
        }
    });

    connect(bluetoothManager, &BluetoothManager::transferProgressUpdated, this, [this](const QString &sessionPath, qulonglong total, qulonglong transferred, int currFileIndex) {
        if (sessionPath != m_currSessionPath)
            return;

        if (transferred > total) // 过滤异常数据
            return;

        if (m_progressUpdateShouldBeIgnore) {
            m_firstTransSize = transferred; // fix bug 45296 记录被忽略的第一次的请求头的数据段长度，如果之后信号触发依然是这个数据长度则认为未接收传输请求。
            m_progressUpdateShouldBeIgnore = false;
            return;
        }
        if (m_firstTransSize == transferred)
            return;

        if (m_stack->currentIndex() != TransferPage && m_stack->currentIndex() != FailedPage)
            m_stack->setCurrentIndex(TransferPage);

        m_sendingStatus->setText(TXT_SEND_PROGRES.arg(currFileIndex - 1).arg(m_urls.count()));
        m_progressBar->setMaximum(static_cast<int>(total));
        m_progressBar->setValue(static_cast<int>(transferred));

        if (total == transferred && m_stack->currentIndex() == TransferPage) {
            m_sendingStatus->setText(TXT_SEND_PROGRES.arg(currFileIndex).arg(m_urls.count()));
            QPointer<QStackedWidget> stack(m_stack);
            QTimer::singleShot(1000, nullptr, [stack] { // 这里留一秒的时间用于显示完整的进度，避免进度满就直接跳转页面了
                if (!stack)
                    return;
                qDebug() << "delay switch page on trans success";
                stack->setCurrentIndex(SuccessPage);
            });
        }
    });

    connect(bluetoothManager, &BluetoothManager::transferCancledByRemote, this, [this](const QString &sessionPath) {
        if (sessionPath != m_currSessionPath)
            return;
        m_stack->setCurrentIndex(FailedPage);
        bluetoothManager->cancelTransfer(sessionPath);
    });

    connect(bluetoothManager, &BluetoothManager::transferFailed, this, [this](const QString &sessionPath, const QString &filePath, const QString &errMsg) {
        if (sessionPath != m_currSessionPath)
            return;
        m_stack->setCurrentIndex(FailedPage);
        bluetoothManager->cancelTransfer(sessionPath);
        qDebug() << "filePath: " << filePath
                 << "\nerrorMsg: " << errMsg;
    });

    connect(bluetoothManager, &BluetoothManager::fileTransferFinished, this, [this](const QString &sessionPath, const QString &filePath) {
        if (sessionPath != m_currSessionPath)
            return;
        m_finishedUrls << filePath;
        if (m_finishedUrls.count() == m_urls.count()) {
            m_stack->setCurrentIndex(SuccessPage);
        }
    });

    connect(bluetoothManager, &BluetoothManager::transferEstablishFinish, this, [this](const QString &sessionPath, const QString &errMsg, const QString &senderToken){
        if (m_token != senderToken)
            return;

        m_currSessionPath = sessionPath;
        if (!sessionPath.isEmpty())
            return;

        if (m_devModel->rowCount() != 0)
            m_stack->setCurrentIndex(SelectDevicePage);
        else
            m_stack->setCurrentIndex(NoneDevicePage);

        dialogManager->showErrorDialog(TITLE_BT_TRANS_FAIL, humanizedStrOfObexErrMsg(errMsg));
    });
}

QWidget *BluetoothTransDialog::initDeviceSelectorPage()
{
    // device selector page
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLayout = new QVBoxLayout(w);
    pLayout->setSpacing(0);
    pLayout->setMargin(0);
    w->setLayout(pLayout);
    // w->setStyleSheet("border: 1px solid red;");

    DLabel *statusTxt = new DLabel(TXT_SELECT_DEVIC, this);
    statusTxt->setAlignment(Qt::AlignCenter);
    setObjTextStyle(statusTxt, 14, false);
    changeLabelTheme(statusTxt);
    pLayout->addWidget(statusTxt);

    m_devicesList = new DListView(this);
    m_devModel = new QStandardItemModel(this);
    m_devicesList->setFixedHeight(88);
    m_devicesList->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_devicesList->setEditTriggers(QListView::NoEditTriggers);
    m_devicesList->setIconSize(QSize(32, 32));
    m_devicesList->setResizeMode(QListView::Adjust);
    m_devicesList->setMovement(QListView::Static);
    m_devicesList->setSelectionMode(QListView::NoSelection);
    m_devicesList->setFrameShape(QFrame::NoFrame);
    m_devicesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_devicesList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_devicesList->setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    m_devicesList->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    m_devicesList->setViewportMargins(0, 0, 0, 0);
    m_devicesList->setItemSpacing(1);
    m_devicesList->setModel(m_devModel);

    pLayout->addWidget(m_devicesList);
    DCommandLinkButton *linkBtn = new DCommandLinkButton(TXT_GOTO_BT_SETS, this);
    setObjTextStyle(linkBtn, 12, true);
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
    pLay->setSpacing(0);
    pLay->setMargin(0);
    w->setLayout(pLay);

    DLabel *statusTxt = new DLabel(TXT_NO_DEV_FOUND, this);
    statusTxt->setAlignment(Qt::AlignCenter);
    setObjTextStyle(statusTxt, 14, false);
    changeLabelTheme(statusTxt);
    pLay->addWidget(statusTxt);

    DCommandLinkButton *linkBtn = new DCommandLinkButton(TXT_GOTO_BT_SETS, this);
    setObjTextStyle(linkBtn, 12, true);
    connect(linkBtn, &DCommandLinkButton::clicked, this, &BluetoothTransDialog::showBluetoothSetting);
    QHBoxLayout *pHLay = new QHBoxLayout(w);
    pHLay->addStretch(1);
    pHLay->addWidget(linkBtn);
    pHLay->addStretch(1);
    pLay->addLayout(pHLay);

    QSvgWidget *pIconWid = new QSvgWidget(this);
    pIconWid->setFixedSize(80, 80);
    QWidget *pIconWidContainer = new QWidget(this);
    QHBoxLayout *iconLay = new QHBoxLayout(pIconWidContainer);
    iconLay->addStretch(1);
    iconLay->addWidget(pIconWid);
    iconLay->addStretch(1);
    iconLay->setMargin(0);
    iconLay->setSpacing(0);
    pLay->addWidget(pIconWidContainer);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [pIconWid](DGuiApplicationHelper::ColorType t) {
        switch (t) {
        case DGuiApplicationHelper::UnknownType:
        case DGuiApplicationHelper::LightType:
            pIconWid->load(PXMP_NO_DEV_LIGHT);
            break;
        case DGuiApplicationHelper::DarkType:
            pIconWid->load(PXMP_NO_DEV_DARKY);
            break;
        }
    });

    return w;
}

QWidget *BluetoothTransDialog::initWaitForRecvPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    pLay->setSpacing(0);
    pLay->setContentsMargins(0, 6, 0, 16);
    w->setLayout(pLay);
    // w->setStyleSheet("border: 1px solid red;");

    m_subTitleForWaitPage = new DLabel("Sending files to ...");
    m_subTitleForWaitPage->setAlignment(Qt::AlignCenter);
    m_subTitleForWaitPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    setObjTextStyle(m_subTitleForWaitPage, 14, false);
    changeLabelTheme(m_subTitleForWaitPage);
    pLay->addWidget(m_subTitleForWaitPage);

    QVBoxLayout *spinnerLayout = new QVBoxLayout;
    m_spinner = new DSpinner(this);
    m_spinner->setFixedHeight(48);
    spinnerLayout->addStretch(1);
    spinnerLayout->addWidget(m_spinner);
    spinnerLayout->addStretch(1);
    pLay->addLayout(spinnerLayout);

    DLabel *txt2 = new DLabel(TXT_WAIT_FOR_RCV, this);
    txt2->setAlignment(Qt::AlignCenter);
    txt2->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    setObjTextStyle(txt2, 12, false);
    changeLabelTheme(txt2);
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
    setObjTextStyle(m_subTitleOfTransPage, 14, false);
    changeLabelTheme(m_subTitleOfTransPage);
    pLay->addWidget(m_subTitleOfTransPage);

    m_progressBar = new DProgressBar(this);
    m_progressBar->setValue(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setMaximumHeight(8);
    pLay->addWidget(m_progressBar);

    m_sendingStatus = new DLabel(TXT_SEND_PROGRES, this);
    m_sendingStatus->setAlignment(Qt::AlignCenter);
    setObjTextStyle(m_sendingStatus, 12, false);
    changeLabelTheme(m_sendingStatus);
    pLay->addWidget(m_sendingStatus);

    return w;
}

QWidget *BluetoothTransDialog::initFailedPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    pLay->setSpacing(0);
    pLay->setMargin(0);
    // w->setStyleSheet("border: 1px solid red;");
    w->setLayout(pLay);

    m_subTitleOfFailedPage = new DLabel("Failed to send files to ...");
    m_subTitleOfFailedPage->setAlignment(Qt::AlignCenter);
    m_subTitleOfFailedPage->setContentsMargins(0, 46, 0, 10);
    setObjTextStyle(m_subTitleOfFailedPage, 14, false);
    changeLabelTheme(m_subTitleOfFailedPage);
    pLay->addWidget(m_subTitleOfFailedPage);

    DLabel *txt2 = new DLabel(TXT_ERROR_REASON, this);
    txt2->setMargin(0);
    txt2->setAlignment(Qt::AlignCenter);
    setObjTextStyle(txt2, 12, false);
    changeLabelTheme(txt2);
    pLay->addWidget(txt2);
    pLay->addStretch(1);

    return w;
}

QWidget *BluetoothTransDialog::initSuccessPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    w->setLayout(pLay);

    m_subTitleOfSuccessPage = new DLabel("Sent to ... successfully");
    m_subTitleOfSuccessPage->setAlignment(Qt::AlignCenter);
    setObjTextStyle(m_subTitleOfSuccessPage, 14, false);
    changeLabelTheme(m_subTitleOfSuccessPage);
    pLay->addWidget(m_subTitleOfSuccessPage);

    return w;
}

DStandardItem *BluetoothTransDialog::getStyledItem(const BluetoothDevice *dev)
{
    // 只有已配对、状态为已连接的设备才显示在设备列表中
    if (!dev || !(dev->paired() && dev->state() == BluetoothDevice::StateConnected))
        return nullptr;

    if (findItemByIdRole(dev)) // 列表中已有此设备
        return nullptr;

    DViewItemActionList actLst;
    DViewItemAction *act = new DViewItemAction(Qt::AlignVCenter | Qt::AlignLeft, QSize(22, 22), QSize(), false);
    actLst.append(act);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    act, [act, dev](DGuiApplicationHelper::ColorType themeType) {
        QString iconPath = QString("%1%2%3").arg(themeType == DGuiApplicationHelper::DarkType ? darkIcon : lightIcon)
                           .arg(dev->icon())
                           .arg(themeType == DGuiApplicationHelper::DarkType ? "_dark.svg" : "_light.svg");
        act->setIcon(QIcon(iconPath));
    });

    // 初始化一次主题变更以添加图标
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::instance()->themeType());


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
    return dev ? findItemByIdRole(dev->id()) : nullptr;
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
    if (!dev)
        return;
    // 根据设备的 uuid 或 icon 要对可接收文件的设备进行过滤
    static const QStringList deviceCanRecvFile {"computer", "phone"};
    if (!deviceCanRecvFile.contains(dev->icon())) // 暂时根据 icon 进行判定，以后或可根据 uuid 是否包含 obex 传输服务来判定设备能否接收文件
        return;

    DStandardItem *item = getStyledItem(dev);
    if (!item)
        return;

    m_devModel->appendRow(item);
    if (m_stack->currentIndex() == NoneDevicePage) // 仅当页面位于无设备页面时执行跳转
        m_stack->setCurrentIndex(SelectDevicePage);
}

void BluetoothTransDialog::removeDevice(const BluetoothDevice *dev)
{
    if (!dev)
        return;
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
    foreach (auto path, m_finishedUrls) { // 针对失败重试：之前已经发送成功的文件不再次发送
        m_urls.removeAll(path);
    }
    m_finishedUrls.clear();

    if (m_urls.count() == 0 || m_selectedDeviceId.isEmpty())
        return;

    // 无法发送文件尺寸大于 2GB 以及尺寸为 0 的文件，若包含则中止发送行为，文件不存在也一样
    foreach (auto u, m_urls) {
        DUrl url = DUrl::fromLocalFile(u);
        if (!url.isValid())
            continue;
        DAbstractFileInfoPointer info = fileService->createFileInfo(nullptr, url);
        if (info && !info->exists()) {
            dialogManager->showMessageDialog(DialogManager::msgErr, TXT_FILE_NOEXIST, "", TXT_OKAY);
            close(); // 与产品经理沟通后，为避免文件不存在时的retry可能引起的一系列问题，当用户点击retry的确认时，直接终止流程
            return;
        }
        if (info && info->size() > FILE_TRANSFER_LIMITS) {
            dialogManager->showMessageDialog(DialogManager::msgInfo, TXT_FILE_OVERSIZ, "", TXT_OKAY);
            return;
        } else if (info && info->size() == 0) {
            dialogManager->showMessageDialog(DialogManager::msgInfo, TXT_FILE_ZEROSIZ, "", TXT_OKAY);
            return;
        }
    }

    m_subTitleForWaitPage->setText(TXT_SENDING_FILE.arg(m_selectedDevice));
    m_subTitleOfTransPage->setText(TXT_SENDING_FILE.arg(m_selectedDevice));
    m_subTitleOfFailedPage->setText(TXT_SENDING_FAIL.arg(m_selectedDevice));
    m_subTitleOfSuccessPage->setText(TXT_SENDING_SUCC.arg(m_selectedDevice));

    m_progressUpdateShouldBeIgnore = true;
    m_firstTransSize = 0;
    m_progressBar->setValue(0); // retry 时需要重置进度

    bluetoothManager->sendFiles(m_selectedDeviceId, m_urls, m_token);

    m_stack->setCurrentIndex(WaitForRecvPage);
    m_spinner->start();
}

void BluetoothTransDialog::closeEvent(QCloseEvent *event)
{
    DDialog::closeEvent(event);

    if ((m_stack->currentIndex() == WaitForRecvPage
            || m_stack->currentIndex() == TransferPage
            || m_stack->currentIndex() == FailedPage)
            && !m_currSessionPath.isEmpty()) {
        bluetoothManager->cancelTransfer(m_currSessionPath);
    }
}

void BluetoothTransDialog::showBluetoothSetting()
{
    bluetoothManager->showBluetoothSettings();
}

void BluetoothTransDialog::onBtnClicked(const int &nIdx)
{
    static qint64 lastTriggerTime = 0;
    if (QDateTime::currentMSecsSinceEpoch() - lastTriggerTime <= 200) // 间隔 200ms 触发一次操作，限制操作频率
        return;

    Page currpage = static_cast<Page>(m_stack->currentIndex());
    switch (currpage) {
    case SelectDevicePage:
        if (m_selectedDevice.isEmpty() && nIdx == 1)
            return;
        if (nIdx != 1) { // 点击取消
            close();
            return;
        }

        if (canSendFiles()) {
            sendFiles();
        } else {
            dialogManager->showMessageDialog(DialogManager::messageType::msgInfo, DialogManager::tr("Sending files now, please try later"));
        }

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
    lastTriggerTime = QDateTime::currentMSecsSinceEpoch();
}

void BluetoothTransDialog::onPageChagned(const int &nIdx)
{
    if (!m_titleOfDialog || !m_spinner)
        return;
    m_spinner->stop();
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
}

void BluetoothTransDialog::connectAdapter(const BluetoothAdapter *adapter)
{
    if (!adapter || m_connectedAdapter.contains(adapter->id()))
        return;
    m_connectedAdapter.append(adapter->id());

    connect(adapter, &BluetoothAdapter::deviceAdded, this, [this](const BluetoothDevice * dev) {
        addDevice(dev);
        connectDevice(dev);
    });

    connect(adapter, &BluetoothAdapter::deviceRemoved, this, [this](const QString & deviceId) {
        removeDevice(deviceId);
    });
}

void BluetoothTransDialog::connectDevice(const BluetoothDevice *dev)
{
    if (!dev)
        return;
    connect(dev, &BluetoothDevice::stateChanged, this, [this](const BluetoothDevice::State & state) {
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
