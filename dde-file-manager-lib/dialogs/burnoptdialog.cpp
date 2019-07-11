#include "burnoptdialog.h"
#include <DLineEdit>
#include "dlabel.h"
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtConcurrent>
#include "disomaster.h"
#include "app/define.h"
#include "fileoperations/filejob.h"
#include "dialogmanager.h"
#include <QDebug>

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
    DLabel *lb_volname;
    DLineEdit *le_volname;
    DLabel *lb_writespeed;
    QComboBox *cb_writespeed;
    QCheckBox *cb_iclose;
    DLabel *lb_postburn;
    QCheckBox *cb_checkdisc;
    QCheckBox *cb_eject;
    QString dev;
    QHash<QString, int> speedmap;
    DUrl image_file;
    int window_id;

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

                        job->doOpticalBurn(dev, d->le_volname->text(), d->speedmap[d->cb_writespeed->currentText()], flag);
                        dialogManager->removeJob(job->getJobId());
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

                        job->doOpticalImageBurn(dev, img, d->speedmap[d->cb_writespeed->currentText()], flag);
                        dialogManager->removeJob(job->getJobId());
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


    lb_volname = new DLabel(QObject::tr("Disc name:"));
    w_content->layout()->addWidget(lb_volname);

    le_volname = new DLineEdit();
    le_volname->setMaxLength(32);
    w_content->layout()->addWidget(le_volname);

    lb_writespeed = new DLabel(QObject::tr("Write speed:"));
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
}

void BurnOptDialogPrivate::setDevice(const QString &device)
{
    dev = device;
}
