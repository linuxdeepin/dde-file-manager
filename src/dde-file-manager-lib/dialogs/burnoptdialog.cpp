/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "burnoptdialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtConcurrent>
#include <QDebug>
#include <QWindow>
#include <DCommandLinkButton>

#include <unistd.h>
#include <sys/wait.h>

#include "disomaster.h"
#include "app/define.h"
#include "fileoperations/filejob.h"
#include "dialogmanager.h"
#include "singleton.h"
#include "app/filesignalmanager.h"
#include "utils/desktopinfo.h"
#include "burnoptdialog_p.h"

DWIDGET_USE_NAMESPACE

#define MAX_LABEL_LEN 30

BurnOptDialog::BurnOptDialog(QString device, QWidget *parent) :
    DDialog(parent),
    d_ptr(new BurnOptDialogPrivate(this))
{
    if (DFMGlobal::isWayLand()) {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    Q_D(BurnOptDialog);
    d->setDevice(device);
    d->setupUi();
    connect(this, &BurnOptDialog::buttonClicked, this,
    [ = ](int index, const QString & text) {
        Q_UNUSED(text);
        QFile opticalDevice(d->dev);
        if (!opticalDevice.exists()) {
            dialogManager->showErrorDialog(tr("Device error"), tr("Optical device %1 doesn't exist").arg(d->dev));
            return;
        }
        DISOMasterNS::BurnOptions opts;
        if (d->cb_checkdisc->isChecked())
            opts |= DISOMasterNS::VerifyDatas;
        if (d->cb_eject->isChecked())
            opts |= DISOMasterNS::EjectDisc;
        if (index == 1 && d->cb_donotclose->isChecked()) // 关闭 session 仅在数据刻录有效(index == 1)
            opts |= DISOMasterNS::KeepAppendable;

        // 文件系统
        if (d->cb_fs->currentIndex() == 0)
            opts |= DISOMasterNS::ISO9660Only;
        else if (d->cb_fs->currentIndex() == 1)
            opts |= DISOMasterNS::JolietSupport;
        else if (d->cb_fs->currentIndex() == 2)
            opts |= DISOMasterNS::RockRidgeSupport;
        else
            opts |= DISOMasterNS::JolietAndRockRidge; // not used yet

        int nSpeeds = d->speedmap[d->cb_writespeed->currentText()];
        const QString &volName = d->le_volname->text().trimmed().isEmpty()
                                 ? d->lastVolName
                                 : d->le_volname->text().trimmed();

        bool isUDBurn = d->cb_fs->currentIndex() == 3;

        if (index == 1) {
            emit fileSignalManager->stopCdScanTimer(device);
            if (d->image_file.path().length() == 0) {
                QtConcurrent::run([ = ] {
                    QSharedPointer<FileJob> job(new FileJob(FileJob::OpticalBurn));
                    job->moveToThread(qApp->thread());
                    job->setWindowId(d->window_id);
                    dialogManager->addJob(job);

                    DUrl dev(device);

                    // fix: use fork() burn files
                    qDebug() << "start burn files";
                    if (isUDBurn && DFMGlobal::isProfessional())
                        job->doUDBurn(dev, volName, nSpeeds, opts);
                    else
                        job->doISOBurn(dev, volName, nSpeeds, opts);
                    dialogManager->removeJob(job->getJobId(), true);  // 清除所有数据，防止脏数据出现
                    //job->deleteLater();
                });
            } else {
                QtConcurrent::run([ = ] {
                    QSharedPointer<FileJob> job(new FileJob(FileJob::OpticalImageBurn));
                    job->moveToThread(qApp->thread());
                    job->setWindowId(d->window_id);
                    dialogManager->addJob(job);

                    DUrl dev(device);
                    //just to ensure we still have access to the image url even after 'this' is deleted
                    DUrl img(d->image_file);

                    // fix: use fork() burn image
                    qDebug() << "start burn image";

                    job->doISOImageBurn(dev, img, nSpeeds, opts);
                    dialogManager->removeJob(job->getJobId(), true); // 清除所有数据，防止脏数据出现
                   // job->deleteLater();
                });
            }
        }
    });
}

void BurnOptDialog::setISOImage(DUrl image)
{
    Q_D(BurnOptDialog);

    d->image_file = image;
    d->cb_donotclose->hide();

    d->lb_fs->hide();
    d->cb_fs->hide();

    d->le_volname->setEnabled(false);

    //we are seemingly abusing DISOMaster here. However that's actually not the case.
    ISOMaster->acquireDevice(QString("stdio:") + image.toLocalFile());
    DISOMasterNS::DeviceProperty dp = ISOMaster->getDeviceProperty();
    d->le_volname->setText(dp.volid);
    ISOMaster->releaseDevice();
}

void BurnOptDialog::setJobWindowId(int wid)
{
    Q_D(BurnOptDialog);
    d->window_id = wid;
}

void BurnOptDialog::setDefaultVolName(const QString &volName)
{
    Q_D(BurnOptDialog);
    d->le_volname->clear();
    d->le_volname->setText(volName);
    d->le_volname->setSelection(0, volName.length());
    d->le_volname->setFocus();
    d->lastVolName = volName;
}

void BurnOptDialog::setDiscAndFsInfo(int type, QString filesystem, QString version)
{
    Q_D(BurnOptDialog);

    if (!DFMGlobal::isProfessional())
        return;

    auto *model = d->cb_fs->model();
    if (!model || model->rowCount() < 4)
        return;

    bool disableISOOpts = false;
    auto supportUD = [ & ]{
        if (!isSupportedUDMedium(type)) // non dvd+-r is obviously not supported yet
            return false;
        if (filesystem.isEmpty()) // empty dvd+-r is supported
            return true;
        if (filesystem != QString("u/d/f").remove("/")) // non-udf fs do not supported udf option
            return false;
        if (isSupportedUDVersion(version)) {
            disableISOOpts = true;
            return true;
        }
        return false;
    };

    bool enableUD = supportUD();
    if (!enableUD)
        model->setData(model->index(3, 0), 0, Qt::UserRole - 1);
    if (disableISOOpts) {
        model->setData(model->index(0, 0), 0, Qt::UserRole - 1);
        model->setData(model->index(1, 0), 0, Qt::UserRole - 1);
        model->setData(model->index(2, 0), 0, Qt::UserRole - 1);
        d->cb_fs->setCurrentIndex(3); // since all iso opts is disable, select UD default
    }
}

bool BurnOptDialog::isSupportedUDVersion(const QString &version)
{
    static const QStringList && supported = {
        "1.02"
    };
    return supported.contains(version);
}

bool BurnOptDialog::isSupportedUDMedium(int type)
{
    static const QList<DISOMasterNS::MediaType> &&supportedMedium = {
        DISOMasterNS::DVD_R,
        DISOMasterNS::DVD_PLUS_R,
        DISOMasterNS::CD_R,
        DISOMasterNS::CD_RW
    };
    return supportedMedium.contains(DISOMasterNS::MediaType(type));
}

BurnOptDialog::~BurnOptDialog()
{
}

BurnOptDialogPrivate::BurnOptDialogPrivate(BurnOptDialog *q) :
    q_ptr(q)
{
}

BurnOptDialogPrivate::~BurnOptDialogPrivate()
{
}

void BurnOptDialogPrivate::setupUi()
{
    Q_Q(BurnOptDialog);
    q->setModal(true);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    q->setIcon(QIcon::fromTheme("media-optical").pixmap(96, 96));

    q->addButton(QObject::tr("Cancel","button"));
    q->addButton(QObject::tr("Burn","button"), true, DDialog::ButtonType::ButtonRecommend);

    q->layout()->setContentsMargins(0, 0, 0, 0);
    w_content = new QWidget(q);
    QVBoxLayout *contentLay = new QVBoxLayout;
    QMargins mg(0, 15, 0, 0);
    contentLay->setContentsMargins(mg);
    w_content->setLayout(contentLay);
    q->addContent(w_content, Qt::AlignTop);

    // 光盘名称
    lb_volname = new QLabel(QObject::tr("Disc name:"));
    contentLay->addWidget(lb_volname, 0, Qt::AlignTop);
    QFont f13 = lb_volname->font();
    f13.setPixelSize(13);
    f13.setFamily("SourceHanSansSC");
    f13.setWeight(QFont::Medium);
    lb_volname->setFont(f13);

    le_volname = new QLineEdit();
    QRegExp regx("[^\\\\/\':\\*\\?\"<>|%&.]+"); //屏蔽特殊字符
    QValidator *validator = new QRegExpValidator(regx, le_volname);
    le_volname->setValidator(validator);
    le_volname->setMaxLength(MAX_LABEL_LEN);
    QObject::connect(le_volname, &QLineEdit::textChanged, [this] {
        while (le_volname->text().toUtf8().length() > MAX_LABEL_LEN)
        {
            le_volname->setText(le_volname->text().chopped(1));
        }
    });
    contentLay->addWidget(le_volname, 0, Qt::AlignTop);
    QFont f14 = le_volname->font();
    f14.setPixelSize(14);
    f14.setWeight(QFont::Medium);
    f14.setFamily("SourceHanSansSC");
    le_volname->setFont(f14);

    // 高级设置内容
    DCommandLinkButton *advanceBtn = new DCommandLinkButton(BurnOptDialog::tr("Advanced settings"), q);
    QFont f12 = advanceBtn->font();
    f12.setPixelSize(12);
    f12.setWeight(QFont::Normal);
    f12.setFamily("SourceHanSansSC");
    advanceBtn->setFont(f12);

    QHBoxLayout *advanceBtnLay = new QHBoxLayout(w_content);
    advanceBtnLay->setMargin(0);
    advanceBtnLay->setSpacing(0);
    advanceBtnLay->addStretch(1);
    advanceBtnLay->addWidget(advanceBtn);
    contentLay->addLayout(advanceBtnLay);

    QWidget *advancedSettings = new QWidget(q);
    contentLay->addWidget(advancedSettings, 0, Qt::AlignTop);
    advancedSettings->hide();

    QVBoxLayout *vLay = new QVBoxLayout(advancedSettings);
    vLay->setMargin(0);
    vLay->setSpacing(8);
    advancedSettings->setLayout(vLay);

    // 文件系统
    lb_fs = new QLabel(BurnOptDialog::tr("File system: "));
    vLay->addWidget(lb_fs);
    static const QStringList fsTypes {
        BurnOptDialog::tr("ISO9660 Only"),
        BurnOptDialog::tr("ISO9660/Joliet (For Windows)"),
        BurnOptDialog::tr("ISO9660/Rock Ridge (For Unix)")
    };

    cb_fs = new QComboBox;
    cb_fs->addItems(fsTypes);
    cb_fs->setCurrentIndex(1); // 默认使用 i + j 的方式刻录
    vLay->addWidget(cb_fs);
    lb_fs->setFont(f13);
    cb_fs->setFont(f14);

    static const QString &udItem =  BurnOptDialog::tr("%1 (Compatible with Windows CD/DVD mode)").arg(QString("U/D/F").remove("/"));
    if (cb_fs->count() == fsTypes.count() && DFMGlobal::isProfessional())
        cb_fs->addItem(udItem);

    // 控制间距
    vLay->addItem(new QSpacerItem(1, 20));

    // 刻录速度
    lb_writespeed = new QLabel(QObject::tr("Write speed:"));
    vLay->addWidget(lb_writespeed, 0, Qt::AlignTop);
    cb_writespeed = new QComboBox();
    cb_writespeed->addItem(QObject::tr("Maximum"));
    vLay->addWidget(cb_writespeed, 0, Qt::AlignTop);
    speedmap[QObject::tr("Maximum")] = 0;
    DISOMasterNS::DeviceProperty dp = ISOMaster->getDevicePropertyCached(dev);
    for (auto i : dp.writespeed) {
        float speed;
        int speedk;
        QByteArray iBytes(i.toUtf8());
        sscanf(iBytes.data(), "%d%*c\t%f", &speedk, &speed);
        speedmap[QString::number(speed, 'f', 1) + 'x'] = speedk;
        cb_writespeed->addItem(QString::number(speed, 'f', 1) + 'x');
    }
    lb_writespeed->setFont(f13);
    cb_writespeed->setFont(f14);

    // 刻录选项-允许追加
    cb_donotclose = new QCheckBox(QObject::tr("Allow files to be added later"));
    cb_donotclose->setChecked(true);
    vLay->addWidget(cb_donotclose, 0, Qt::AlignTop);
    QWidget *wpostburn = new QWidget();
    wpostburn->setLayout(new QHBoxLayout);
    vLay->addWidget(wpostburn, 0, Qt::AlignTop);
    wpostburn->layout()->setMargin(0);
    cb_donotclose->setFont(f12);

    // 刻录选项-校验数据
    cb_checkdisc = new QCheckBox(QObject::tr("Verify data"));
    cb_checkdisc->setFont(f12);
    wpostburn->layout()->addWidget(cb_checkdisc);
    // 刻录选项-弹出光盘（目前禁用）
    cb_eject = new QCheckBox(QObject::tr("Eject"));
    cb_eject->setFont(f12);
    cb_eject->setChecked(true);
    wpostburn->layout()->addWidget(cb_eject);
    cb_eject->setVisible(false); // 20200430 xust 与产品沟通后决定隐藏弹出的配置项，默认刻录完成后弹出光盘仓

    w_content->setFixedWidth(360);
    q->layout()->setSizeConstraint(QLayout::SetFixedSize);

    QObject::connect(advanceBtn, &DCommandLinkButton::clicked, q, [ = ] {
        advancedSettings->setHidden(!advancedSettings->isHidden());
    });

    QObject::connect(cb_fs, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), q, [this](int idx){
        if (idx == 3) {
            cb_checkdisc->setChecked(false);
            cb_checkdisc->setEnabled(false);
            cb_donotclose->setChecked(true);
            cb_donotclose->setEnabled(false);
            cb_writespeed->setCurrentIndex(0);
            cb_writespeed->setEnabled(false);
        } else {
            cb_checkdisc->setEnabled(true);
            cb_donotclose->setEnabled(true);
            cb_writespeed->setEnabled(true);
        }
    });

//    q->setStyleSheet("border: 1px solid blue;");
}

void BurnOptDialogPrivate::setDevice(const QString &device)
{
    dev = device;
}
