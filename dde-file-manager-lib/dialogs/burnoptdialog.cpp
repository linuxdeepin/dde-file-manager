#include "burnoptdialog.h"
#include <DLineEdit>
#include "dlabel.h"
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

DWIDGET_USE_NAMESPACE

class BurnOptDialogPrivate
{
public:
    BurnOptDialogPrivate(BurnOptDialog *q);
    ~BurnOptDialogPrivate();
    void setupUi();
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

    Q_DECLARE_PUBLIC(BurnOptDialog)
};

BurnOptDialog::BurnOptDialog(QWidget *parent) :
    DDialog(parent),
    d_ptr(new BurnOptDialogPrivate(this))
{
    Q_D(BurnOptDialog);
    d->setupUi();
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

    QStringList sl;
    sl << "Cancel";
    sl << "Burn";
    q->addButtons(sl);
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
