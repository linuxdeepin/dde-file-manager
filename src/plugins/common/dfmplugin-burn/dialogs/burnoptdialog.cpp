// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "burnoptdialog.h"
#include "utils/burnjobmanager.h"
#include "utils/burnhelper.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-burn/dopticaldiscmanager.h>
#include <dfm-burn/dopticaldiscinfo.h>

#include <DSysInfo>
#include <QWindow>
#include <QLabel>
#include <QFile>
#include <QtConcurrent>

using namespace dfmplugin_burn;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

static constexpr int kMaxLabelLen { 30 };

BurnOptDialog::BurnOptDialog(const QString &dev, QWidget *parent)
    : DDialog(parent), curDev(dev)
{
    initializeUi();
    initConnect();
}

void BurnOptDialog::setUDFSupported(bool supported, bool disableISOOpts)
{
    isSupportedUDF = supported;

    auto *model = fsComb->model();
    if (!model || model->rowCount() < 4)
        return;

    if (!supported)
        model->setData(model->index(3, 0), 0, Qt::UserRole - 1);
    if (disableISOOpts) {
        model->setData(model->index(0, 0), 0, Qt::UserRole - 1);
        model->setData(model->index(1, 0), 0, Qt::UserRole - 1);
        model->setData(model->index(2, 0), 0, Qt::UserRole - 1);
        fsComb->setCurrentIndex(3);   // since all iso opts is disable, select UD default
    }
}

void BurnOptDialog::setISOImage(const QUrl &image)
{
    DFM_BURN_USE_NS

    imageFile = image;
    donotcloseComb->hide();

    fsLabel->hide();
    fsComb->hide();

    volnameEdit->setEnabled(false);

    // we are seemingly abusing dfm-burn here. However that's actually not the case.
    QScopedPointer<DFMBURN::DOpticalDiscInfo> info { DFMBURN::DOpticalDiscManager::createOpticalInfo(QString("stdio:") + image.toLocalFile()) };
    if (info)
        volnameEdit->setText(info->volumeName());
}

void BurnOptDialog::setDefaultVolName(const QString &volName)
{
    volnameEdit->clear();
    volnameEdit->setText(volName);
    volnameEdit->setSelection(0, volName.length());
    volnameEdit->setFocus();
    lastVolName = volName;
}

void BurnOptDialog::setWriteSpeedInfo(const QStringList &writespeed)
{
    for (const auto &i : writespeed) {
        double speed;
        int speedk;
        QByteArray iBytes(i.toUtf8());
        sscanf(iBytes.data(), "%d%*c\t%lf", &speedk, &speed);
        speedMap[QString::number(speed, 'f', 1) + 'x'] = speedk;
        writespeedComb->addItem(QString::number(speed, 'f', 1) + 'x');
    }
}

void BurnOptDialog::initializeUi()
{
    if (WindowUtils::isWayLand()) {
        // 设置对话框窗口最大最小化按钮隐藏
        setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        setAttribute(Qt::WA_NativeWindow);
        windowHandle()->setProperty("_d_dwayland_minimizable", false);
        windowHandle()->setProperty("_d_dwayland_maximizable", false);
        windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    setModal(true);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setIcon(QIcon::fromTheme("media-optical").pixmap(96, 96));

    addButton(QObject::tr("Cancel", "button"));
    addButton(QObject::tr("Burn", "button"), true, DDialog::ButtonType::ButtonRecommend);
    layout()->setContentsMargins(0, 0, 0, 0);

    content = new QWidget(this);
    QVBoxLayout *contentLay = new QVBoxLayout;
    QMargins mg(0, 15, 0, 0);
    contentLay->setContentsMargins(mg);
    content->setLayout(contentLay);
    addContent(content, Qt::AlignTop);

    // 光盘名称
    volnameLabel = new QLabel(QObject::tr("Disc name:"));
    contentLay->addWidget(volnameLabel, 0, Qt::AlignTop);
    QFont f13 = volnameLabel->font();
    f13.setPixelSize(13);
    f13.setFamily("SourceHanSansSC");
    f13.setWeight(QFont::Medium);
    volnameLabel->setFont(f13);

    volnameEdit = new QLineEdit();
    QRegularExpression regx("[^\\\\/\':\\*\\?\"<>|%&.]+");   //屏蔽特殊字符
    QValidator *validator = new QRegularExpressionValidator(regx, volnameEdit);
    volnameEdit->setValidator(validator);
    volnameEdit->setMaxLength(kMaxLabelLen);

    contentLay->addWidget(volnameEdit, 0, Qt::AlignTop);
    QFont f14 = volnameEdit->font();
    f14.setPixelSize(14);
    f14.setWeight(QFont::Medium);
    f14.setFamily("SourceHanSansSC");
    volnameEdit->setFont(f14);

    // 高级设置内容
    advanceBtn = new DCommandLinkButton(BurnOptDialog::tr("Advanced settings"), this);
    QFont f12 = advanceBtn->font();
    f12.setPixelSize(12);
    f12.setWeight(QFont::Normal);
    f12.setFamily("SourceHanSansSC");
    advanceBtn->setFont(f12);

    QHBoxLayout *advanceBtnLay = new QHBoxLayout(content);
    advanceBtnLay->setContentsMargins(0, 0, 0, 0);
    advanceBtnLay->setSpacing(0);
    advanceBtnLay->addStretch(1);
    advanceBtnLay->addWidget(advanceBtn);
    contentLay->addLayout(advanceBtnLay);

    advancedSettings = new QWidget(this);
    contentLay->addWidget(advancedSettings, 0, Qt::AlignTop);
    advancedSettings->hide();

    QVBoxLayout *vLay = new QVBoxLayout(advancedSettings);
    vLay->setContentsMargins(0, 0, 0, 0);
    vLay->setSpacing(8);
    advancedSettings->setLayout(vLay);

    // 文件系统
    fsLabel = new QLabel(BurnOptDialog::tr("File system: "));
    vLay->addWidget(fsLabel);
    static const QStringList fsTypes {
        BurnOptDialog::tr("ISO9660 Only"),
        BurnOptDialog::tr("ISO9660/Joliet (For Windows)"),
        BurnOptDialog::tr("ISO9660/Rock Ridge (For Unix)")
    };

    fsComb = new QComboBox;
    fsComb->addItems(fsTypes);
    fsComb->setCurrentIndex(1);   // 默认使用 i + j 的方式刻录
    vLay->addWidget(fsComb);
    fsLabel->setFont(f13);
    fsComb->setFont(f14);

    static const QString &udItem = BurnOptDialog::tr("%1 (Compatible with Windows CD/DVD mode)").arg(QString("U/D/F").remove("/"));
    if (fsComb->count() == fsTypes.count() && DSysInfo::deepinType() == DSysInfo::DeepinProfessional)
        fsComb->addItem(udItem);

    // 控制间距
    vLay->addItem(new QSpacerItem(1, 20));

    // 刻录速度
    writespeedLabel = new QLabel(QObject::tr("Write speed:"));
    vLay->addWidget(writespeedLabel, 0, Qt::AlignTop);
    writespeedComb = new QComboBox();
    writespeedComb->addItem(QObject::tr("Maximum"));
    vLay->addWidget(writespeedComb, 0, Qt::AlignTop);
    speedMap[QObject::tr("Maximum")] = 0;
    writespeedLabel->setFont(f13);
    writespeedComb->setFont(f14);

    // 刻录选项-允许追加
    donotcloseComb = new QCheckBox(QObject::tr("Allow files to be added later"));
    donotcloseComb->setChecked(true);
    vLay->addWidget(donotcloseComb, 0, Qt::AlignTop);
    QWidget *wpostburn = new QWidget();
    wpostburn->setLayout(new QHBoxLayout);
    vLay->addWidget(wpostburn, 0, Qt::AlignTop);
    wpostburn->layout()->setContentsMargins(0, 0, 0, 0);
    donotcloseComb->setFont(f12);

    // 刻录选项-校验数据
    checkdiscCheckbox = new QCheckBox(QObject::tr("Verify data"));
    checkdiscCheckbox->setFont(f12);
    wpostburn->layout()->addWidget(checkdiscCheckbox);
    // 刻录选项-弹出光盘（目前禁用）
    ejectCheckbox = new QCheckBox(QObject::tr("Eject"));
    ejectCheckbox->setFont(f12);
    ejectCheckbox->setChecked(true);
    wpostburn->layout()->addWidget(ejectCheckbox);
    ejectCheckbox->setVisible(false);   // 20200430 xust 与产品沟通后决定隐藏弹出的配置项，默认刻录完成后弹出光盘仓

    content->setFixedWidth(360);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void BurnOptDialog::initConnect()
{
    connect(volnameEdit, &QLineEdit::textChanged, [this] {
        while (volnameEdit->text().toUtf8().length() > kMaxLabelLen) {
            volnameEdit->setText(volnameEdit->text().chopped(1));
        }
    });

    connect(advanceBtn, &DCommandLinkButton::clicked, this, [=] {
        advancedSettings->setHidden(!advancedSettings->isHidden());
    });

    connect(fsComb, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &BurnOptDialog::onIndexChanged);
    connect(this, &BurnOptDialog::buttonClicked, this, &BurnOptDialog::onButnBtnClicked);
}

DFMBURN::BurnOptions BurnOptDialog::currentBurnOptions()
{
    DFMBURN::BurnOptions opts;

    if (checkdiscCheckbox->isChecked())
        opts |= DFMBURN::BurnOption::kVerifyDatas;
    if (ejectCheckbox->isChecked())
        opts |= DFMBURN::BurnOption::kEjectDisc;
    if (donotcloseComb->isChecked())
        opts |= DFMBURN::BurnOption::kKeepAppendable;

    // 文件系统
    int fsIndex { fsComb->currentIndex() };
    if (fsIndex == 0)
        opts |= DFMBURN::BurnOption::kISO9660Only;
    else if (fsIndex == 1)
        opts |= DFMBURN::BurnOption::kJolietSupport;
    else if (fsIndex == 2)
        opts |= DFMBURN::BurnOption::kRockRidgeSupport;
    else if (fsIndex == 3)
        opts |= DFMBURN::BurnOption::kUDF102Supported;
    else
        opts |= DFMBURN::BurnOption::kJolietAndRockRidge;   // not used yet

    return opts;
}

void BurnOptDialog::startDataBurn()
{
    fmInfo() << "Start Burn files";
    const QString &volName = volnameEdit->text().trimmed().isEmpty()
            ? lastVolName
            : volnameEdit->text().trimmed();

    bool isUDFBurn = fsComb->currentIndex() == 3;
    BurnJobManager::Config conf;
    conf.speeds = speedMap[writespeedComb->currentText()];
    conf.opts = currentBurnOptions();
    conf.volName = volName;

    if (isUDFBurn)
        BurnJobManager::instance()->startBurnUDFFiles(curDev, BurnHelper::localStagingFile(curDev), conf);
    else
        BurnJobManager::instance()->startBurnISOFiles(curDev, BurnHelper::localStagingFile(curDev), conf);
}

void BurnOptDialog::startImageBurn()
{
    fmInfo() << "Start burn image";
    BurnJobManager::Config conf;
    conf.speeds = speedMap[writespeedComb->currentText()];
    conf.opts = currentBurnOptions();

    BurnJobManager::instance()->startBurnISOImage(curDev, imageFile, conf);
}

void BurnOptDialog::onIndexChanged(int index)
{
    if (index == 3) {   // 3 is UDF
        checkdiscCheckbox->setChecked(false);
        checkdiscCheckbox->setEnabled(false);
        donotcloseComb->setChecked(true);
        // donotcloseComb->setEnabled(false);
        writespeedComb->setCurrentIndex(0);
        writespeedComb->setEnabled(false);
    } else {
        checkdiscCheckbox->setEnabled(true);
        donotcloseComb->setEnabled(true);
        writespeedComb->setEnabled(true);
    }
}

void BurnOptDialog::onButnBtnClicked(int index, const QString &text)
{
    Q_UNUSED(text);
    QFile opticalDevice(curDev);
    if (!opticalDevice.exists()) {
        DialogManagerInstance->showErrorDialog(tr("Device error"), tr("Optical device %1 doesn't exist").arg(curDev));
        return;
    }

    if (index == 1) {
        bool isDataBurn { imageFile.path().length() == 0 };
        if (isDataBurn)
            startDataBurn();
        else
            startImageBurn();
    }
}
