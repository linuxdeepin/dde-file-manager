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

DWIDGET_USE_NAMESPACE

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
    [ = ](int index, const QString &text) {
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

        if (index == 1) {
            emit fileSignalManager->stopCdScanTimer(device);
            if (d->image_file.path().length() == 0) {
                QtConcurrent::run([ = ] {
                    FileJob *job = new FileJob(FileJob::OpticalBurn);
                    job->moveToThread(qApp->thread());
                    job->setWindowId(d->window_id);
                    dialogManager->addJob(job);

                    DUrl dev(device);

                    // fix: use fork() burn files
                    qDebug() << "start burn files";

                    job->doOpticalBurnByChildProcess(dev, volName, nSpeeds, opts);
                    dialogManager->removeJob(job->getJobId(), true ); // 清除所有数据，防止脏数据出现
                    job->deleteLater();
                });
            } else {
                QtConcurrent::run([=] {
                    FileJob *job = new FileJob(FileJob::OpticalImageBurn);
                    job->moveToThread(qApp->thread());
                    job->setWindowId(d->window_id);
                    dialogManager->addJob(job);

                    DUrl dev(device);
                    //just to ensure we still have access to the image url even after 'this' is deleted
                    DUrl img(d->image_file);

                    // fix: use fork() burn image
                    qDebug() << "start burn image";

                    job->doOpticalImageBurnByChildProcess(dev, img, nSpeeds, opts);
                    dialogManager->removeJob(job->getJobId(), true );// 清除所有数据，防止脏数据出现
                    job->deleteLater();
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
    if (DesktopInfo().waylandDectected()) { // 该需求仅针对华为分支
        Q_D(BurnOptDialog);
        d->le_volname->clear();
        d->le_volname->setText(volName);
        d->le_volname->setSelection(0, volName.length());
        d->le_volname->setFocus();
        d->lastVolName = volName;
    }
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
    q->setIcon(QIcon::fromTheme("media-optical").pixmap(96, 96), QSize(96, 96));

    q->addButton(QObject::tr("Cancel"));
    q->addButton(QObject::tr("Burn"), true, DDialog::ButtonType::ButtonRecommend);

    q->layout()->setContentsMargins(0, 10, 0, 0);
    w_content = new QWidget(q);
    QVBoxLayout *contentLay = new QVBoxLayout;
    QMargins mg(0, 10, 0, 0);
    contentLay->setContentsMargins(mg);
    w_content->setLayout(contentLay);
    q->addContent(w_content, Qt::AlignTop);

    // 光盘名称
    lb_volname = new QLabel(QObject::tr("Disc name:"));
    contentLay->addWidget(lb_volname, 0, Qt::AlignTop);

    le_volname = new QLineEdit();
    QRegExp regx("[^\\\\/\':\\*\\?\"<>|%&.]+"); //屏蔽特殊字符
    QValidator *validator = new QRegExpValidator(regx, le_volname);
    le_volname->setValidator(validator);
    le_volname->setMaxLength(32);
    QObject::connect(le_volname, &QLineEdit::textChanged, [this] {
        while (le_volname->text().toUtf8().length() > 32)
        {
            le_volname->setText(le_volname->text().chopped(1));
        }
    });
    contentLay->addWidget(le_volname, 0, Qt::AlignTop);

    // 高级设置内容
    DCommandLinkButton *advanceBtn = new DCommandLinkButton(BurnOptDialog::tr("Advanced settings"), q);
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
    advancedSettings->setLayout(vLay);

    // 文件系统
    lb_fs = new QLabel(BurnOptDialog::tr("File System: "));
    vLay->addWidget(lb_fs);
    static QStringList fsTypes{BurnOptDialog::tr("ISO9660 Only"), BurnOptDialog::tr("ISO9660 / Joliet (For Windows)"), BurnOptDialog::tr("ISO9660 / RockRidge (For Unix)")};
    cb_fs = new QComboBox;
    cb_fs->addItems(fsTypes);
    cb_fs->setCurrentIndex(1); // 默认使用 i + j 的方式刻录
    vLay->addWidget(cb_fs);

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
        sscanf(i.toUtf8().data(), "%d%*c\t%f", &speedk, &speed);
        speedmap[QString::number(speed, 'f', 1) + 'x'] = speedk;
        cb_writespeed->addItem(QString::number(speed, 'f', 1) + 'x');
    }

    // 刻录选项-允许追加
    cb_donotclose = new QCheckBox(QObject::tr("Allow files to be added later"));
    cb_donotclose->setChecked(true);
    vLay->addWidget(cb_donotclose, 0, Qt::AlignTop);
    QWidget *wpostburn = new QWidget();
    wpostburn->setLayout(new QHBoxLayout);
    vLay->addWidget(wpostburn, 0, Qt::AlignTop);
    wpostburn->layout()->setMargin(0);
    // 刻录选项-校验数据
    cb_checkdisc = new QCheckBox(QObject::tr("Verify data"));
    wpostburn->layout()->addWidget(cb_checkdisc);
    // 刻录选项-弹出光盘（目前禁用）
    cb_eject = new QCheckBox(QObject::tr("Eject"));
    cb_eject->setChecked(true);
    wpostburn->layout()->addWidget(cb_eject);
    cb_eject->setVisible(false); // 20200430 xust 与产品沟通后决定隐藏弹出的配置项，默认刻录完成后弹出光盘仓

    w_content->setFixedWidth(360);
    q->layout()->setSizeConstraint(QLayout::SetFixedSize);

    QObject::connect(advanceBtn, &DCommandLinkButton::clicked, q, [ = ]{
        advancedSettings->setHidden(!advancedSettings->isHidden());
    });

//    q->setStyleSheet("border: 1px solid blue;");
}

void BurnOptDialogPrivate::setDevice(const QString &device)
{
    dev = device;
}
