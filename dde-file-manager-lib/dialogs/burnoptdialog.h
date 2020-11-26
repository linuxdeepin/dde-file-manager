#ifndef BURNOPTDIALOG_H
#define BURNOPTDIALOG_H
#include "ddialog.h"
#include "durl.h"

#include <QCheckBox>
#include <DLineEdit>
#include <QComboBox>

class BurnOptDialogPrivate;
class BurnOptDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    BurnOptDialog(QString device, QWidget *parent = 0);
    ~BurnOptDialog();

    void setISOImage(DUrl image);
    void setJobWindowId(int wid);
    void setDefaultVolName(const QString &volName);

private:
    QScopedPointer<BurnOptDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE(BurnOptDialog)

};

class BurnOptDialogPrivate
{
public:
    explicit BurnOptDialogPrivate(BurnOptDialog *q);
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
    QString dev{""};
    QHash<QString, int> speedmap;
    DUrl image_file;
    int window_id;
    QString lastVolName;

    Q_DECLARE_PUBLIC(BurnOptDialog)
};

#endif // BURNOPTDIALOG_H
