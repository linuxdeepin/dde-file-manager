// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bluetoothtransdialog.h"
#include "dguiapplicationhelper.h"
#include "bluetoothmanager.h"
#include "bluetoothadapter.h"
#include "bluetoothmodel.h"
#include "dfileservices.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/dialogmanager.h>

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
#include <QUuid>
#include <QDateTime>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QSvgWidget>
#else
#    include <QSvgWidget>
#endif

#define TITLE_BT_TRANS_FILE dfmplugin_utils::BluetoothTransDialog::tr("Bluetooth File Transfer")
#define TITLE_BT_TRANS_SUCC dfmplugin_utils::BluetoothTransDialog::tr("File Transfer Successful")
#define TITLE_BT_TRANS_FAIL dfmplugin_utils::BluetoothTransDialog::tr("File Transfer Failed")

#define TXT_SENDING_FILE dfmplugin_utils::BluetoothTransDialog::tr("Sending files to \"<b style=\"font-weight: 550;\">%1</b>\"")
#define TXT_SENDING_FAIL dfmplugin_utils::BluetoothTransDialog::tr("Failed to send files to \"<b style=\"font-weight: 550;\">%1</b>\"")
#define TXT_SENDING_SUCC dfmplugin_utils::BluetoothTransDialog::tr("Sent to \"<b style=\"font-weight: 550;\">%1</b>\" successfully")
#define TXT_SELECT_DEVIC dfmplugin_utils::BluetoothTransDialog::tr("Select a Bluetooth device to receive files")
#define TXT_NO_DEV_FOUND dfmplugin_utils::BluetoothTransDialog::tr("Cannot find the connected Bluetooth device")
#define TXT_WAIT_FOR_RCV dfmplugin_utils::BluetoothTransDialog::tr("Waiting to be received...")
#define TXT_GOTO_BT_SETS dfmplugin_utils::BluetoothTransDialog::tr("Go to Bluetooth Settings")
#define TXT_SEND_PROGRES dfmplugin_utils::BluetoothTransDialog::tr("%1/%2 Sent")
#define TXT_ERROR_REASON dfmplugin_utils::BluetoothTransDialog::tr("Error: the Bluetooth device is disconnected")
#define TXT_FILE_OVERSIZ dfmplugin_utils::BluetoothTransDialog::tr("Unable to send the file more than 2 GB")
#define TXT_FILE_ZEROSIZ dfmplugin_utils::BluetoothTransDialog::tr("Unable to send 0 KB files")
#define TXT_FILE_NOEXIST dfmplugin_utils::BluetoothTransDialog::tr("File doesn't exist")
#define TXT_DIR_SELECTED dfmplugin_utils::BluetoothTransDialog::tr("Transferring folders is not supported")
#define TXT_LONG_FILENAME dfmplugin_utils::BluetoothTransDialog::tr("Error: The filename is too long")

#define TXT_NEXT dfmplugin_utils::BluetoothTransDialog::tr("Next", "button")
#define TXT_CANC dfmplugin_utils::BluetoothTransDialog::tr("Cancel", "button")
#define TXT_DONE dfmplugin_utils::BluetoothTransDialog::tr("Done", "button")
#define TXT_RTRY dfmplugin_utils::BluetoothTransDialog::tr("Retry", "button")
#define TXT_OKAY dfmplugin_utils::BluetoothTransDialog::tr("OK", "button")

static constexpr char kIconConnect[] { "notification-bluetooth-connected" };
static constexpr char kIconDisconn[] { "notification-bluetooth-disconnected" };
static constexpr char kBluetoothIconLight[] { ":/icons/deepin/builtin/light/icons/bluetooth_" };
static constexpr char kBluetoothIconDark[] { ":/icons/deepin/builtin/dark/icons/bluetooth_" };
static constexpr char kNoDeviceLight[] { "://icons/deepin/builtin/light/icons/dfm_bluetooth_empty_light.svg" };
static constexpr char kNoDeviceDark[] { "://icons/deepin/builtin/dark/icons/dfm_bluetooth_empty_dark.svg" };

static constexpr long kFileTransferSizeLimits = 2147483648;   // 2GB = 2 * 1024 * 1024 * 1024 Bytes

using namespace dfmplugin_utils;

BluetoothTransDialog::BluetoothTransDialog(const QStringList &urls, QString targetDevId, QWidget *parent)
    : DDialog(parent), urlsWaitToSend(urls), dialogToken(QUuid::createUuid().toString())
{
    initUI();
    initConn();
    stackedWidget->setCurrentIndex(kNoneDevicePage);   // 初始界面为空界面

    updateDeviceList();   // 打开多个窗口的时候蓝牙设备不一定任何更新操作，因此这时依靠蓝牙状态的变更去更新列表不可取，手动获取一次列表
    BluetoothManagerInstance->refresh();

    if (!targetDevId.isEmpty())   // specify a device to receive the files.
        sendFilesToDevice(targetDevId);

    // 调试布局
    // setStyleSheet("border: 1px solid blue;");
}

/**
 * @brief sendFilesToDevice 直接发送文件到指定设备
 * @param devId 接收方id
 */
void BluetoothTransDialog::sendFilesToDevice(const QString &devId)
{
    const BluetoothDevice *dev = nullptr;
    QMapIterator<QString, const BluetoothAdapter *> iter(BluetoothManagerInstance->getAdapters());
    while (iter.hasNext()) {
        iter.next();
        dev = (iter.value()->deviceById(devId));
        if (dev)
            break;
    }

    if (!dev) {
        fmDebug() << "can not find device: " << devId;
    } else {
        selectedDeviceName = dev->getAlias();
        selectedDeviceId = devId;
        sendFiles();
    }
}

/**
 * @brief changeLabelTheme 跟随主题变换对话框部分字体文案的颜色
 * @param obj 需要控制颜色的QLabel对象
 * @param isTitle 是否是主标题，主标题的透明度与其他文案有差别
 */
void BluetoothTransDialog::changeLabelTheme(QLabel *obj, bool isTitle)
{
    if (!obj)
        return;

    double alpha = isTitle ? 0.9 : 0.7;
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            obj, [obj, alpha](DGuiApplicationHelper::ColorType themeType) {
                QPalette pal = obj->palette();
                pal.setColor(QPalette::WindowText, themeType == DGuiApplicationHelper::DarkType ? QColor::fromRgbF(1, 1, 1, alpha) : QColor::fromRgbF(0, 0, 0, alpha));
                obj->setPalette(pal);
            });
}

/**
 * @brief setObjTextStyle 设置各控件的字体属性
 * @param obj   目标控件
 * @param size  字体尺寸
 * @param bold  字体加粗
 */
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

/**
 * @brief humanizeObexErrMsg  处理 obex 的错误信息
 * @param msg                obex 返回的错误信息
 * @return                   可读性处理后的字符串
 */
QString BluetoothTransDialog::humanizeObexErrMsg(const QString &msg)
{
    if (msg.contains("Timed out")) {
        return tr("File sending request timed out");
    } else if (msg.contains("0x53")) {
        return tr("The service is busy and unable to process the request");
    } else if (msg.contains("Unable to find service record")) {
        return TXT_NO_DEV_FOUND;
    } else if (msg.contains("device not connected")
               || msg.contains("Connection refused")
               || msg.contains("Connection reset by peer")) {
        return TXT_ERROR_REASON;
    } else {   // ...TO BE CONTINUE
        fmWarning() << "bluetooth error message: " << msg;
        return "";
    }
}

void BluetoothTransDialog::setNextButtonEnable(bool enable)
{
    if (stackedWidget->currentIndex() != Page::kSelectDevicePage) {
        for (auto btn : getButtons())
            btn->setEnabled(true);
        return;
    }

    auto btns = getButtons();
    if (btns.count() == 2) {
        btns[1]->setEnabled(enable);
    }
}

bool BluetoothTransDialog::isBluetoothIdle()
{
    return BluetoothManagerInstance->canSendBluetoothRequest();
}

void BluetoothTransDialog::initUI()
{
    setIcon(QIcon::fromTheme(kIconConnect));
    setFixedSize(381, 271);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);

    // main structure
    QFrame *mainFrame = new QFrame(this);
    QVBoxLayout *pLayout = new QVBoxLayout;
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(0, 0, 0, 0);
    mainFrame->setLayout(pLayout);

    addContent(mainFrame);

    // public title
    titleOfDialog = new DLabel(TITLE_BT_TRANS_FILE, this);
    titleOfDialog->setAlignment(Qt::AlignCenter);
    setObjTextStyle(titleOfDialog, 14, true);
    changeLabelTheme(titleOfDialog, true);
    pLayout->addWidget(titleOfDialog);

    // stacked area
    stackedWidget = new QStackedWidget(this);
    stackedWidget->layout()->setContentsMargins(0, 0, 0, 0);
    stackedWidget->layout()->setSpacing(0);

    pLayout->addWidget(stackedWidget);

    // 以下顺序固定以便进行枚举遍历
    stackedWidget->addWidget(createDeviceSelectorPage());
    stackedWidget->addWidget(createNonDevicePage());
    stackedWidget->addWidget(createWaitForRecvPage());
    stackedWidget->addWidget(createTranferingPage());
    stackedWidget->addWidget(createFailedPage());
    stackedWidget->addWidget(createSuccessPage());

    setOnButtonClickedClose(false);

    // 防止窗口初始化的时候字体颜色不更改，手动触发一次该信号
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::instance()->themeType());
}

void BluetoothTransDialog::initConn()
{
    QMap<QString, const BluetoothAdapter *> adapters = BluetoothManagerInstance->getAdapters();
    QMapIterator<QString, const BluetoothAdapter *> iter(adapters);
    while (iter.hasNext()) {
        iter.next();
        const BluetoothAdapter *adapter = iter.value();
        connectAdapter(adapter);
    }

    connect(stackedWidget, &QStackedWidget::currentChanged, this, &BluetoothTransDialog::onPageChagned);
    connect(this, &BluetoothTransDialog::buttonClicked, this, &BluetoothTransDialog::onBtnClicked);

    connect(devicesListView, &DListView::clicked, this, [this](const QModelIndex &curr) {
        for (int i = 0; i < devModel->rowCount(); i++) {
            DStandardItem *item = dynamic_cast<DStandardItem *>(devModel->item(i));
            if (!item)
                continue;
            if (i == curr.row()) {
                item->setCheckState(Qt::Checked);
                selectedDeviceName = item->text();
                selectedDeviceId = item->data(kDevIdRole).toString();
                setNextButtonEnable(true);   // when an item is selected, the next button should be enable.
            } else {
                item->setCheckState(Qt::Unchecked);
            }
        }
    });

    connect(BluetoothManagerInstance, &BluetoothManager::adapterAdded, this, [this](const BluetoothAdapter *adapter) {
        connectAdapter(adapter);
    });

    connect(BluetoothManagerInstance, &BluetoothManager::adapterRemoved, this, [this](const BluetoothAdapter *adapter) {
        if (!adapter)
            return;
        if (connectedAdapters.contains(adapter->getId()))
            connectedAdapters.removeAll(adapter->getId());

        adapter->disconnect();
        QMap<QString, const BluetoothDevice *> devices = adapter->getDevices();
        QMapIterator<QString, const BluetoothDevice *> it(devices);
        while (it.hasNext()) {
            it.next();
            it.value()->disconnect();
        }
    });

    connect(BluetoothManagerInstance, &BluetoothManager::transferProgressUpdated, this, [this](const QString &sessionPath, qulonglong total, qulonglong transferred, int currFileIndex) {
        if (sessionPath != currSessionPath)
            return;

        if (transferred > total)   // 过滤异常数据
            return;

        if (ignoreProgress) {
            firstTransSize = transferred;   // fix bug 45296 记录被忽略的第一次的请求头的数据段长度，如果之后信号触发依然是这个数据长度则认为未接收传输请求。
            ignoreProgress = false;
            return;
        }
        if (firstTransSize == transferred)
            return;

        if (stackedWidget->currentIndex() != kTransferPage && stackedWidget->currentIndex() != kFailedPage)
            stackedWidget->setCurrentIndex(kTransferPage);

        sendingStatusLabel->setText(TXT_SEND_PROGRES.arg(currFileIndex - 1).arg(urlsWaitToSend.count()));
        progressBar->setMaximum(static_cast<int>(total));
        progressBar->setValue(static_cast<int>(transferred));

        if (total == transferred && stackedWidget->currentIndex() == kTransferPage) {
            sendingStatusLabel->setText(TXT_SEND_PROGRES.arg(currFileIndex).arg(urlsWaitToSend.count()));
            QPointer<QStackedWidget> stack(stackedWidget);
            QTimer::singleShot(1000, nullptr, [stack] {   // 这里留一秒的时间用于显示完整的进度，避免进度满就直接跳转页面了
                if (!stack)
                    return;
                fmDebug() << "delay switch page on trans success";
                stack->setCurrentIndex(kSuccessPage);
            });
        }
    });

    connect(BluetoothManagerInstance, &BluetoothManager::transferCancledByRemote, this, [this](const QString &sessionPath) {
        if (sessionPath != currSessionPath)
            return;

        if (BluetoothManagerInstance->isLongFilenameFailure(sessionPath)) {
            if (failedErrorMsgLabel) {
                failedErrorMsgLabel->setText(TXT_LONG_FILENAME);
            }
        } else {
            if (failedErrorMsgLabel) {
                failedErrorMsgLabel->setText(TXT_ERROR_REASON);
            }
        }

        stackedWidget->setCurrentIndex(kFailedPage);
        BluetoothManagerInstance->cancelTransfer(sessionPath);
    });

    connect(BluetoothManagerInstance, &BluetoothManager::transferFailed, this, [this](const QString &sessionPath, const QString &filePath, const QString &errMsg) {
        if (sessionPath != currSessionPath)
            return;
        stackedWidget->setCurrentIndex(kFailedPage);
        BluetoothManagerInstance->cancelTransfer(sessionPath);
        fmDebug() << "filePath: " << filePath
                  << "\nerrorMsg: " << errMsg;
    });

    connect(BluetoothManagerInstance, &BluetoothManager::fileTransferFinished, this, [this](const QString &sessionPath, const QString &filePath) {
        if (sessionPath != currSessionPath)
            return;
        finishedUrls << filePath;
        if (finishedUrls.count() == urlsWaitToSend.count()) {
            stackedWidget->setCurrentIndex(kSuccessPage);
        }
    });

    connect(BluetoothManagerInstance, &BluetoothManager::transferEstablishFinish, this, [this](const QString &sessionPath, const QString &errMsg, const QString &senderToken) {
        if (dialogToken != senderToken)
            return;

        currSessionPath = sessionPath;
        if (!sessionPath.isEmpty())
            return;

        if (devModel->rowCount() != 0)
            stackedWidget->setCurrentIndex(kSelectDevicePage);
        else
            stackedWidget->setCurrentIndex(kNoneDevicePage);

        DialogManagerInstance->showErrorDialog(TITLE_BT_TRANS_FAIL, humanizeObexErrMsg(errMsg));
    });
}

QWidget *BluetoothTransDialog::createDeviceSelectorPage()
{
    // device selector page
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLayout = new QVBoxLayout(w);
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(0, 0, 0, 0);
    w->setLayout(pLayout);
    // w->setStyleSheet("border: 1px solid red;");

    DLabel *statusTxt = new DLabel(TXT_SELECT_DEVIC, this);
    statusTxt->setAlignment(Qt::AlignCenter);
    setObjTextStyle(statusTxt, 14, false);
    changeLabelTheme(statusTxt);
    pLayout->addWidget(statusTxt);

    devicesListView = new DListView(this);
    devModel = new QStandardItemModel(this);
    devicesListView->setFixedHeight(88);
    devicesListView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    devicesListView->setEditTriggers(QListView::NoEditTriggers);
    devicesListView->setIconSize(QSize(32, 32));
    devicesListView->setResizeMode(QListView::Adjust);
    devicesListView->setMovement(QListView::Static);
    devicesListView->setSelectionMode(QListView::NoSelection);
    devicesListView->setFrameShape(QFrame::NoFrame);
    devicesListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    devicesListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    devicesListView->setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    devicesListView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    devicesListView->setViewportMargins(0, 0, 0, 0);
    devicesListView->setItemSpacing(1);
    devicesListView->setModel(devModel);

    pLayout->addWidget(devicesListView);
    DCommandLinkButton *linkBtn = new DCommandLinkButton(TXT_GOTO_BT_SETS, this);
    setObjTextStyle(linkBtn, 12, true);
    connect(linkBtn, &DCommandLinkButton::clicked, this, &BluetoothTransDialog::showBluetoothSetting);
    QHBoxLayout *pLay = new QHBoxLayout(this);
    pLay->setContentsMargins(0, 0, 0, 0);
    pLay->setSpacing(0);
    pLay->addStretch(1);
    pLay->addWidget(linkBtn);
    pLayout->addLayout(pLay);
    pLayout->setStretch(1, 1);

    return w;
}

QWidget *BluetoothTransDialog::createNonDevicePage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    pLay->setSpacing(0);
    pLay->setContentsMargins(0, 0, 0, 0);
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
    iconLay->setContentsMargins(0, 0, 0, 0);
    iconLay->setSpacing(0);
    pLay->addWidget(pIconWidContainer);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [pIconWid](DGuiApplicationHelper::ColorType t) {
        switch (t) {
        case DGuiApplicationHelper::UnknownType:
        case DGuiApplicationHelper::LightType:
            pIconWid->load(QString(kNoDeviceLight));
            break;
        case DGuiApplicationHelper::DarkType:
            pIconWid->load(QString(kNoDeviceDark));
            break;
        }
    });

    return w;
}

QWidget *BluetoothTransDialog::createWaitForRecvPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    pLay->setSpacing(0);
    pLay->setContentsMargins(0, 6, 0, 16);
    w->setLayout(pLay);
    // w->setStyleSheet("border: 1px solid red;");

    subTitleForWaitPage = new DLabel("Sending files to ...");
    subTitleForWaitPage->setAlignment(Qt::AlignCenter);
    subTitleForWaitPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    setObjTextStyle(subTitleForWaitPage, 14, false);
    changeLabelTheme(subTitleForWaitPage);
    pLay->addWidget(subTitleForWaitPage);

    QVBoxLayout *spinnerLayout = new QVBoxLayout;
    spinner = new DSpinner(this);
    spinner->setFixedHeight(48);
    spinnerLayout->addStretch(1);
    spinnerLayout->addWidget(spinner);
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

QWidget *BluetoothTransDialog::createTranferingPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    w->setLayout(pLay);

    subTitleOfTransPage = new DLabel("Sending files to ...");
    subTitleOfTransPage->setAlignment(Qt::AlignCenter);
    setObjTextStyle(subTitleOfTransPage, 14, false);
    changeLabelTheme(subTitleOfTransPage);
    pLay->addWidget(subTitleOfTransPage);

    progressBar = new DProgressBar(this);
    progressBar->setValue(0);
    progressBar->setMaximum(100);
    progressBar->setMaximumHeight(8);
    pLay->addWidget(progressBar);

    sendingStatusLabel = new DLabel(TXT_SEND_PROGRES, this);
    sendingStatusLabel->setAlignment(Qt::AlignCenter);
    setObjTextStyle(sendingStatusLabel, 12, false);
    changeLabelTheme(sendingStatusLabel);
    pLay->addWidget(sendingStatusLabel);

    return w;
}

QWidget *BluetoothTransDialog::createFailedPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    pLay->setSpacing(0);
    pLay->setContentsMargins(0, 0, 0, 0);
    // w->setStyleSheet("border: 1px solid red;");
    w->setLayout(pLay);

    subTitleOfFailedPage = new DLabel("Failed to send files to ...");
    subTitleOfFailedPage->setAlignment(Qt::AlignCenter);
    subTitleOfFailedPage->setContentsMargins(0, 46, 0, 10);
    setObjTextStyle(subTitleOfFailedPage, 14, false);
    changeLabelTheme(subTitleOfFailedPage);
    pLay->addWidget(subTitleOfFailedPage);

    failedErrorMsgLabel = new DLabel(TXT_ERROR_REASON, this);
    failedErrorMsgLabel->setMargin(0);
    failedErrorMsgLabel->setAlignment(Qt::AlignCenter);
    setObjTextStyle(failedErrorMsgLabel, 12, false);
    changeLabelTheme(failedErrorMsgLabel);
    pLay->addWidget(failedErrorMsgLabel);
    pLay->addStretch(1);

    return w;
}

QWidget *BluetoothTransDialog::createSuccessPage()
{
    QWidget *w = new QWidget(this);
    QVBoxLayout *pLay = new QVBoxLayout(w);
    w->setLayout(pLay);

    subTitleOfSuccessPage = new DLabel("Sent to ... successfully");
    subTitleOfSuccessPage->setAlignment(Qt::AlignCenter);
    setObjTextStyle(subTitleOfSuccessPage, 14, false);
    changeLabelTheme(subTitleOfSuccessPage);
    pLay->addWidget(subTitleOfSuccessPage);

    return w;
}

/**
 * @brief getStyledItem 根据蓝牙设备对象获取一个处理后的列表 item
 * @param dev 蓝牙设备对象
 * @return 返回一个列表 item 用于添加到列表中
 */
DStandardItem *BluetoothTransDialog::createStyledItem(const BluetoothDevice *dev)
{
    // 只有已配对、状态为已连接的设备才显示在设备列表中
    if (!dev || !(dev->isPaired() && dev->getState() == BluetoothDevice::kStateConnected))
        return nullptr;

    if (findItemByIdRole(dev))   // 列表中已有此设备
        return nullptr;

    DViewItemActionList actLst;
    DViewItemAction *act = new DViewItemAction(Qt::AlignVCenter | Qt::AlignLeft, QSize(22, 22), QSize(), false);
    actLst.append(act);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            act, [act, dev](DGuiApplicationHelper::ColorType themeType) {
                QString iconPath = QString("%1%2%3").arg(themeType == DGuiApplicationHelper::DarkType ? kBluetoothIconDark : kBluetoothIconLight).arg(dev->getIcon()).arg(themeType == DGuiApplicationHelper::DarkType ? "_dark.svg" : "_light.svg");
                act->setIcon(QIcon(iconPath));
            });

    // 初始化一次主题变更以添加图标
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::instance()->themeType());

    DStandardItem *item = new DStandardItem();
    item->setData(dev->getId(), kDevIdRole);
    item->setText(dev->getAlias());
    item->setActionList(Qt::LeftEdge, actLst);
    QFont f = item->font();
    f.setPixelSize(12);
    item->setFont(f);
    return item;
}

/**
 * @brief findItemByIdRole 从列表中获取对应蓝牙设备的 item
 * @param dev
 * @return
 */
DStandardItem *BluetoothTransDialog::findItemByIdRole(const BluetoothDevice *dev)
{
    return dev ? findItemByIdRole(dev->getId()) : nullptr;
}

DStandardItem *BluetoothTransDialog::findItemByIdRole(const QString &devId)
{
    const QString &id = devId;
    for (int i = 0; i < devModel->rowCount(); i++) {
        if (id == devModel->data(devModel->index(i, 0), kDevIdRole).toString())
            return dynamic_cast<DStandardItem *>(devModel->item(i));
    }
    return nullptr;
}

/**
 * @brief updateDeviceList 初始化加载设备列表
 */
void BluetoothTransDialog::updateDeviceList()
{
    if (!devicesListView)
        return;

    QMap<QString, const BluetoothAdapter *> adapters = BluetoothManagerInstance->getAdapters();
    QMapIterator<QString, const BluetoothAdapter *> iter(adapters);
    while (iter.hasNext()) {
        iter.next();
        const BluetoothAdapter *adapter = iter.value();
        QMap<QString, const BluetoothDevice *> devices = adapter->getDevices();
        QMapIterator<QString, const BluetoothDevice *> iterOfDev(devices);
        while (iterOfDev.hasNext()) {
            iterOfDev.next();
            const BluetoothDevice *dev = iterOfDev.value();
            connectDevice(dev);
            addDevice(dev);
        }
    }
}

/**
 * @brief addDevice 添加设备到设备列表
 * @param dev 蓝牙设备对象
 */
void BluetoothTransDialog::addDevice(const BluetoothDevice *dev)
{
    if (!dev)
        return;
    // 根据设备的 uuid 或 icon 要对可接收文件的设备进行过滤
    static const QStringList deviceCanRecvFile { "computer", "phone" };
    if (!deviceCanRecvFile.contains(dev->getIcon()))   // 暂时根据 icon 进行判定，以后或可根据 uuid 是否包含 obex 传输服务来判定设备能否接收文件
        return;

    DStandardItem *item = createStyledItem(dev);
    if (!item)
        return;

    devModel->appendRow(item);
    if (stackedWidget->currentIndex() == kNoneDevicePage)   // 仅当页面位于无设备页面时执行跳转
        stackedWidget->setCurrentIndex(kSelectDevicePage);
}

/**
 * @brief removeDevice 从列表中移除相应设备
 * @param dev 蓝牙设备对象
 */
void BluetoothTransDialog::removeDevice(const BluetoothDevice *dev)
{
    if (!dev)
        return;
    removeDevice(dev->getId());
}

void BluetoothTransDialog::removeDevice(const QString &id)
{
    for (int i = 0; i < devModel->rowCount(); i++) {
        if (devModel->data(devModel->index(i, 0), kDevIdRole).toString() == id) {
            auto item = devModel->item(i);
            if (item && item->checkState() == Qt::Checked)
                setNextButtonEnable(false);   // set the next button disable when the selected item is removed.

            devModel->removeRow(i);
            if (devModel->rowCount() == 0 && stackedWidget->currentIndex() == kSelectDevicePage)
                stackedWidget->setCurrentIndex(kNoneDevicePage);
            return;
        }
    }
}

void BluetoothTransDialog::sendFiles()
{
    foreach (auto path, finishedUrls) {   // 针对失败重试：之前已经发送成功的文件不再次发送
        urlsWaitToSend.removeAll(path);
    }
    finishedUrls.clear();

    if (urlsWaitToSend.count() == 0 || selectedDeviceId.isEmpty())
        return;

    // 无法发送文件尺寸大于 2GB 以及尺寸为 0 的文件，若包含则中止发送行为，文件不存在也一样
    foreach (auto u, urlsWaitToSend) {
        QUrl url = DFMBASE_NAMESPACE::UrlRoute::pathToReal(u);
        if (!url.isValid())
            continue;

        auto info = DFMBASE_NAMESPACE::InfoFactory::create<DFMBASE_NAMESPACE::FileInfo>(url);
        if (!info) {
            fmWarning() << "cannot create file info: " << url;
            close();
            return;
        }

        if (!info->exists()) {
            close();   // 与产品经理沟通后，为避免文件不存在时的retry可能引起的一系列问题，当用户点击retry的确认时，直接终止流程
            DialogManagerInstance->showMessageDialog(TXT_FILE_NOEXIST, "", TXT_OKAY);
            return;
        } else if (info->size() > kFileTransferSizeLimits) {
            DialogManagerInstance->showMessageDialog(TXT_FILE_OVERSIZ, "", TXT_OKAY);
            return;
        } else if (info->size() == 0) {
            DialogManagerInstance->showMessageDialog(TXT_FILE_ZEROSIZ, "", TXT_OKAY);
            return;
        } else if (info->isAttributes(dfmbase::OptInfoType::kIsDir)) {
            close();   // 与产品经理沟通后，为避免文件不存在时的retry可能引起的一系列问题，当用户点击retry的确认时，直接终止流程
            DialogManagerInstance->showMessageDialog(TXT_DIR_SELECTED, "", TXT_OKAY);
            return;
        }
    }

    QFontMetrics fm = subTitleForWaitPage->fontMetrics();
    auto shrinkLabel = [=](const QString &format, QString msg) -> QString {
        QString pureFormat = format;
        pureFormat.remove(QRegularExpression(R"(<.*>)"));   // remove the html tags in label text.
        int fixedWidth = fm.horizontalAdvance(pureFormat);
        int freeWidth = this->width() - fixedWidth - 40;   // 40 is side margin.
        msg = fm.elidedText(msg, Qt::ElideRight, freeWidth);
        return format.arg(msg);
    };

    auto setToolTipAsNeeded = [](QLabel *lab, const QString &fullMsg, const QString &shortMsg, const QString &tooltip) {
        if (Q_UNLIKELY(fullMsg != shortMsg) && lab)
            lab->setToolTip(tooltip);
    };

    const auto &&sendingInfo = shrinkLabel(TXT_SENDING_FILE, selectedDeviceName);
    const auto &&sendingFailed = shrinkLabel(TXT_SENDING_FAIL, selectedDeviceName);
    const auto &&sendingSuccess = shrinkLabel(TXT_SENDING_SUCC, selectedDeviceName);

    subTitleForWaitPage->setText(sendingInfo);
    subTitleOfTransPage->setText(sendingInfo);
    subTitleOfFailedPage->setText(sendingFailed);
    subTitleOfSuccessPage->setText(sendingSuccess);

    setToolTipAsNeeded(subTitleForWaitPage, TXT_SENDING_FILE.arg(selectedDeviceName), sendingInfo, selectedDeviceName);
    setToolTipAsNeeded(subTitleOfTransPage, TXT_SENDING_FILE.arg(selectedDeviceName), sendingInfo, selectedDeviceName);
    setToolTipAsNeeded(subTitleOfFailedPage, TXT_SENDING_FAIL.arg(selectedDeviceName), sendingInfo, selectedDeviceName);
    setToolTipAsNeeded(subTitleOfSuccessPage, TXT_SENDING_SUCC.arg(selectedDeviceName), sendingInfo, selectedDeviceName);

    ignoreProgress = true;
    firstTransSize = 0;
    progressBar->setValue(0);   // retry 时需要重置进度

    BluetoothManagerInstance->sendFiles(selectedDeviceId, urlsWaitToSend, dialogToken);

    stackedWidget->setCurrentIndex(kWaitForRecvPage);
    spinner->start();
}

void BluetoothTransDialog::closeEvent(QCloseEvent *event)
{
    DDialog::closeEvent(event);

    if ((stackedWidget->currentIndex() == kWaitForRecvPage
         || stackedWidget->currentIndex() == kTransferPage
         || stackedWidget->currentIndex() == kFailedPage)
        && !currSessionPath.isEmpty()) {
        BluetoothManagerInstance->cancelTransfer(currSessionPath);
    }
}

void BluetoothTransDialog::showBluetoothSetting()
{
    BluetoothManagerInstance->showBluetoothSettings();
}

/**
 * @brief onBtnClicked 对话框按钮槽函数，根据页面及触发按钮的索引执行页面间的跳转
 * @param nIdx 按钮的索引
 */
void BluetoothTransDialog::onBtnClicked(const int &nIdx)
{
    static qint64 lastTriggerTime = 0;
    if (QDateTime::currentMSecsSinceEpoch() - lastTriggerTime <= 200)   // 间隔 200ms 触发一次操作，限制操作频率
        return;

    Page currpage = static_cast<Page>(stackedWidget->currentIndex());
    switch (currpage) {
    case kSelectDevicePage:
        if (selectedDeviceName.isEmpty() && nIdx == 1)
            return;
        if (nIdx != 1) {   // 点击取消
            close();
            return;
        }

        if (isBluetoothIdle()) {
            sendFiles();
        } else {
            //            dialogManager->showMessageDialog(DialogManager::messageType::msgInfo, DialogManager::tr("Sending files now, please try later")); // TODO(xust)
        }

        break;
    case kFailedPage:
        if (nIdx == 1)
            sendFiles();
        else
            close();
        break;
    case kWaitForRecvPage:
    case kNoneDevicePage:
    case kTransferPage:
    case kSuccessPage:
        close();
        break;
    }
    lastTriggerTime = QDateTime::currentMSecsSinceEpoch();
}

/**
 * @brief onPageChagned 处理页面跳转后界面按钮的更新等操作
 * @param nIdx 页面的索引，索引值与枚举 Page 一致
 */
void BluetoothTransDialog::onPageChagned(const int &nIdx)
{
    if (!titleOfDialog || !spinner)
        return;
    spinner->stop();
    setIcon(QIcon::fromTheme(kIconConnect));
    titleOfDialog->setText(TITLE_BT_TRANS_FILE);
    clearButtons();

    Page currpage = static_cast<Page>(nIdx);
    switch (currpage) {
    case kSelectDevicePage:
        addButton(TXT_CANC);
        addButton(TXT_NEXT, true, ButtonType::ButtonRecommend);
        setNextButtonEnable(false);
        for (int i = 0; i < devModel->rowCount(); ++i) {
            auto item = devModel->item(i);
            if (item && item->checkState() == Qt::Checked) {
                setNextButtonEnable(true);
                break;
            }
        }
        break;
    case kNoneDevicePage:
    case kWaitForRecvPage:
    case kTransferPage:
        addButton(TXT_CANC);
        break;
    case kFailedPage:
        titleOfDialog->setText(TITLE_BT_TRANS_FAIL);
        setIcon(QIcon::fromTheme(kIconDisconn));
        addButton(TXT_CANC);
        addButton(TXT_RTRY, true, ButtonType::ButtonRecommend);
        break;
    case kSuccessPage:
        titleOfDialog->setText(TITLE_BT_TRANS_SUCC);
        addButton(TXT_DONE);
        break;
    }
}

/**
 * @brief connectAdapter 连接每个 adapter 的信号
 */
void BluetoothTransDialog::connectAdapter(const BluetoothAdapter *adapter)
{
    if (!adapter || connectedAdapters.contains(adapter->getId()))
        return;
    connectedAdapters.append(adapter->getId());

    connect(adapter, &BluetoothAdapter::deviceAdded, this, [this](const BluetoothDevice *dev) {
        addDevice(dev);
        connectDevice(dev);
    });

    connect(adapter, &BluetoothAdapter::deviceRemoved, this, [this](const QString &deviceId) {
        removeDevice(deviceId);
    });
}

/**
 * @brief connectDevice 连接每个设备的信号
 */
void BluetoothTransDialog::connectDevice(const BluetoothDevice *dev)
{
    if (!dev)
        return;
    connect(dev, &BluetoothDevice::stateChanged, this, [this](const BluetoothDevice::State &state) {
        BluetoothDevice *device = dynamic_cast<BluetoothDevice *>(sender());
        if (!device)
            return;

        switch (state) {
        case BluetoothDevice::kStateConnected:
            addDevice(device);
            break;
        default:
            removeDevice(device);
            break;
        }
    });
}
