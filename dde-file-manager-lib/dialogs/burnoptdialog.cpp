#include "burnoptdialog.h"
#include <DLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtConcurrent>
#include <QDebug>

#include <unistd.h>
#include <sys/wait.h>

#include "disomaster.h"
#include "app/define.h"
#include "fileoperations/filejob.h"
#include "dialogmanager.h"
#include "singleton.h"
#include "app/filesignalmanager.h"

DWIDGET_USE_NAMESPACE

class BurnOptDialogPrivate
{
public:
    BurnOptDialogPrivate(BurnOptDialog *q);
    ~BurnOptDialogPrivate();
    void setupUi();
    void setDevice(const QString &device);
private:
    BurnOptDialog *q_ptr;
    QWidget *w_content;
    QLabel *lb_volname;
    QLineEdit *le_volname;
    QLabel *lb_writespeed;
    QComboBox *cb_writespeed;
    QCheckBox *cb_iclose;
    QLabel *lb_postburn;
    QCheckBox *cb_checkdisc;
    QCheckBox *cb_eject;
    QString dev;
    QHash<QString, int> speedmap;
    DUrl image_file;
    int window_id;
    QString lastVolName;

    Q_DECLARE_PUBLIC(BurnOptDialog)
};

BurnOptDialog::BurnOptDialog(QString device, QWidget *parent) :
    DDialog(parent),
    d_ptr(new BurnOptDialogPrivate(this))
{
    Q_D(BurnOptDialog);
    d->setDevice(device);
    d->setupUi();
    connect(this, &BurnOptDialog::buttonClicked, this,
        [=](int index, const QString &text) {
            Q_UNUSED(text);
            if (index == 1) {
                emit fileSignalManager->stopCdScanTimer(device);
                if (d->image_file.path().length() == 0) {
                    QtConcurrent::run([=] {
                        FileJob *job = new FileJob(FileJob::OpticalBurn);
                        job->moveToThread(qApp->thread());
                        job->setWindowId(d->window_id);
                        dialogManager->addJob(job);

                        DUrl dev(device);

                        int flag = 0;
                        d->cb_checkdisc->isChecked() && (flag |= 4);
                        d->cb_eject->isChecked() && (flag |= 2);
                        !d->cb_iclose->isChecked() && (flag |= 1);

                        // fix: use fork() burn files
                        qDebug() << "start burn files";
                        const QString &volName = d->le_volname->text().trimmed().isEmpty()
                                                     ? d->lastVolName
                                                     : d->le_volname->text().trimmed();
                        job->doOpticalBurnByChildProcess(dev, volName, d->speedmap[d->cb_writespeed->currentText()], flag);
                        dialogManager->removeJob(job->getJobId(),true );
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

                        int flag = 0;
                        d->cb_checkdisc->isChecked() && (flag |= 4);
                        d->cb_eject->isChecked() && (flag |= 2);

                        // fix: use fork() burn image
                        qDebug() << "start burn image";
                        job->doOpticalImageBurnByChildProcess(dev, img, d->speedmap[d->cb_writespeed->currentText()], flag);
                        dialogManager->removeJob(job->getJobId(), true );
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
    d->cb_iclose->hide();

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
    q->setIcon(QIcon::fromTheme("media-optical").pixmap(96, 96), QSize(96, 96));

    q->addButton(QObject::tr("Cancel"));
    q->addButton(QObject::tr("Burn"), true, DDialog::ButtonType::ButtonRecommend);

    w_content = new QWidget(q);
    w_content->setLayout(new QVBoxLayout);
    q->addContent(w_content);


    lb_volname = new QLabel(QObject::tr("Disc name:"));
    w_content->layout()->addWidget(lb_volname);

    le_volname = new QLineEdit();
    QRegExp regx("[^\\\\/\':\\*\\?\"<>|%&.]+"); //屏蔽特殊字符
    QValidator *validator = new QRegExpValidator(regx, le_volname);
    le_volname->setValidator(validator);
    le_volname->setMaxLength(32);
    QObject::connect(le_volname, &QLineEdit::textChanged, [this] {
        while (le_volname->text().toUtf8().length() > 32) {
            le_volname->setText(le_volname->text().chopped(1));
        }
    });
    w_content->layout()->addWidget(le_volname);

    lb_writespeed = new QLabel(QObject::tr("Write speed:"));
    w_content->layout()->addWidget(lb_writespeed);

    cb_writespeed = new QComboBox();
    cb_writespeed->addItem(QObject::tr("Maximum"));
    w_content->layout()->addWidget(cb_writespeed);
    speedmap[QObject::tr("Maximum")] = 0;

    DISOMasterNS::DeviceProperty dp = ISOMaster->getDevicePropertyCached(dev);
    for (auto i : dp.writespeed) {
        float speed;
        int speedk;
        sscanf(i.toUtf8().data(), "%d%*c\t%f", &speedk, &speed);
        speedmap[QString::number(speed, 'f', 1) + 'x'] = speedk;
        cb_writespeed->addItem(QString::number(speed, 'f', 1) + 'x');
    }

    cb_iclose = new QCheckBox(QObject::tr("Allow files to be added later"));
    cb_iclose->setChecked(true);
    w_content->layout()->addWidget(cb_iclose);

    QWidget *wpostburn = new QWidget();
    wpostburn->setLayout(new QHBoxLayout);
    w_content->layout()->addWidget(wpostburn);
    wpostburn->layout()->setMargin(0);

    cb_checkdisc = new QCheckBox(QObject::tr("Verify data"));
    wpostburn->layout()->addWidget(cb_checkdisc);

    cb_eject = new QCheckBox(QObject::tr("Eject"));
    cb_eject->setChecked(true);
    wpostburn->layout()->addWidget(cb_eject);
    cb_eject->setVisible(false); // 20200430 xust 与产品沟通后决定隐藏弹出的配置项，默认刻录完成后弹出光盘仓
}

void BurnOptDialogPrivate::setDevice(const QString &device)
{
    dev = device;
}
