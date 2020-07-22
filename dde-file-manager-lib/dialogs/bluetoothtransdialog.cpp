#include "bluetoothtransdialog.h"
#include "dguiapplicationhelper.h"

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

BluetoothTransDialog::BluetoothTransDialog(QWidget *parent)
    : DDialog(parent)
{
    timer = new QTimer(this);
    setAttribute(Qt::WA_DeleteOnClose);

    initUI();

    UpdateDeviceList();

    //    setStyleSheet("border: 1px solid blue;");
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
    m_title = new DLabel(TITLE_BT_TRANS_FILE, this);
    QFont fnt = m_title->font();
    fnt.setBold(true);
    fnt.setPixelSize(14);
    m_title->setFont(fnt);
    m_title->setAlignment(Qt::AlignCenter);
    pLayout->addWidget(m_title);

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

    setOnButtonClickedClose(false);

    connect(this, &BluetoothTransDialog::buttonClicked, this, &BluetoothTransDialog::onBtnClicked);
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

    m_subTitle1 = new DLabel("Sending files to ...");
    m_subTitle1->setAlignment(Qt::AlignCenter);
    QFont f = m_subTitle1->font();
    f.setPixelSize(14);
    m_subTitle1->setFont(f);
    pLay->addWidget(m_subTitle1);

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

    m_subTitle2 = new DLabel("Sending files to ...");
    m_subTitle2->setAlignment(Qt::AlignCenter);
    QFont f = m_subTitle2->font();
    f.setPixelSize(14);
    m_subTitle2->setFont(f);
    pLay->addWidget(m_subTitle2);

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

    m_subTitle3 = new DLabel("Failed to send files to ...");
    m_subTitle3->setAlignment(Qt::AlignCenter);
    QFont f = m_subTitle3->font();
    f.setPixelSize(14);
    m_subTitle3->setFont(f);
    pLay->addWidget(m_subTitle3);

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

    m_subTitle4 = new DLabel("Sent to ... successfully");
    m_subTitle4->setAlignment(Qt::AlignCenter);
    QFont f = m_subTitle4->font();
    f.setPixelSize(14);
    m_subTitle4->setFont(f);
    pLay->addWidget(m_subTitle4);

    return w;
}

QList<QString> BluetoothTransDialog::listDevices()
{
    QList<QString> devices;
    devices << "Anna's phone"
            << "XuSt's phone"
            << "Test's phone";
    return devices;
}

void BluetoothTransDialog::UpdateDeviceList()
{
    if (!m_devicesList)
        return;

    m_stack->setCurrentIndex(NoneDevicePage);
    DViewItemActionList actLst;
    DViewItemAction *act = new DViewItemAction(Qt::AlignVCenter | Qt::AlignLeft, QSize(22, 22), QSize(), false);
    actLst.append(act);
    act->setIcon(QIcon::fromTheme(ICON_PHONE));
    foreach (auto dev, listDevices()) {
        if (m_stack->currentIndex() != SelectDevicePage)
            m_stack->setCurrentIndex(SelectDevicePage);
        DStandardItem *item = new DStandardItem();
        item->setText(dev);
        QFont f = item->font();
        f.setPixelSize(12);
        item->setFont(f);
        item->setActionList(Qt::LeftEdge, actLst);
        m_devModel->appendRow(item);
    }
}

void BluetoothTransDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
}

void BluetoothTransDialog::showBluetoothSetting()
{
}

void BluetoothTransDialog::updateProgress()
{
}

void BluetoothTransDialog::retry()
{
}

void BluetoothTransDialog::onBtnClicked(const int &nIdx)
{
    auto jumpPage = [nIdx, this] {
        if (nIdx == 0) {
            close();
            return;
        } else if (nIdx == 1) { // 下一步
            m_stack->setCurrentIndex(WaitForRecvPage);
            Q_EMIT startSpinner();
        }
    };
    Page currpage = static_cast<Page>(m_stack->currentIndex());
    switch (currpage) {
    case SelectDevicePage:
        if (m_selectedDevice.isEmpty() && nIdx == 1)
            return;
        m_subTitle1->setText(TXT_SENDING_FILE.arg(m_selectedDevice));
        m_subTitle2->setText(TXT_SENDING_FILE.arg(m_selectedDevice));
        m_subTitle3->setText(TXT_SENDING_FAIL.arg(m_selectedDevice));
        m_subTitle4->setText(TXT_SENDING_SUCC.arg(m_selectedDevice));
        jumpPage();
        qDebug() << "SendFiles: " << m_urls << "to" << m_selectedDevice;
        break;
    case FailedPage:
        jumpPage();
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
    m_title->setText(TITLE_BT_TRANS_FILE);
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
        m_title->setText(TITLE_BT_TRANS_FAIL);
        setIcon(QIcon::fromTheme(ICON_DISCONN));
        addButton(TXT_CANC);
        addButton(TXT_RTRY, true, ButtonType::ButtonRecommend);
        break;
    case SuccessPage:
        m_title->setText(TITLE_BT_TRANS_SUCC);
        addButton(TXT_DONE);
        break;
    }

    // 测试代码
    if (currpage == WaitForRecvPage) {
        QTimer::singleShot(3000, nullptr, [this] {
            if (!m_stack)
                return;
            m_stack->setCurrentIndex(TransferPage);
        });
    }
    if (currpage == TransferPage) {
        Q_EMIT resetProgress();
        timer->start(100);
    } else {
        timer->stop();
    }
}
