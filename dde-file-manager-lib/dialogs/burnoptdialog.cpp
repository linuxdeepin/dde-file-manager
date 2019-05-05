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
                QtConcurrent::run([=] {
                    FileJob job(FileJob::OpticalBurn);
                    job.moveToThread(qApp->thread());
                    job.setWindowId(this->parentWidget()->window()->winId());
                    dialogManager->addJob(&job);

                    DUrl dev(device);
                    dev.setScheme(BURN_SCHEME);

                    job.doOpticalBurn(dev, d->le_volname->text(), 0, 0);
                    dialogManager->removeJob(job.getJobId());
                });
            }
    });
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


    lb_volname = new DLabel(QObject::tr("Volume label:"));
    w_content->layout()->addWidget(lb_volname);

    le_volname = new DLineEdit();
    w_content->layout()->addWidget(le_volname);

    lb_writespeed = new DLabel(QObject::tr("Write speed:"));
    w_content->layout()->addWidget(lb_writespeed);

    cb_writespeed = new QComboBox();
    cb_writespeed->addItem(QObject::tr("Max speed"));
    w_content->layout()->addWidget(cb_writespeed);

    DISOMasterNS::DeviceProperty dp = ISOMaster->getDevicePropertyCached(dev);
    for (auto i : dp.writespeed) {
        float speed;
        sscanf(i.toUtf8().data(), "%*d%*c\t%f", &speed);
        cb_writespeed->addItem(QString::number((int)speed) + 'x');
    }

    cb_iclose = new QCheckBox(QObject::tr("Allow more data to be append to the disc"));
    w_content->layout()->addWidget(cb_iclose);

    lb_postburn = new DLabel(QObject::tr("After the files have been burned:"));
    w_content->layout()->addWidget(lb_postburn);

    QWidget *wpostburn = new QWidget();
    wpostburn->setLayout(new QHBoxLayout);
    w_content->layout()->addWidget(wpostburn);
    wpostburn->layout()->setMargin(0);

    cb_checkdisc = new QCheckBox(QObject::tr("Check data"));
    wpostburn->layout()->addWidget(cb_checkdisc);

    cb_eject = new QCheckBox(QObject::tr("Eject"));
    wpostburn->layout()->addWidget(cb_eject);
}

void BurnOptDialogPrivate::setDevice(const QString &device)
{
    dev = device;
}
