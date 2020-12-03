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
    QWidget *w_content = nullptr;
    QLabel *lb_volname = nullptr;
    QLineEdit *le_volname = nullptr;
    QLabel *lb_writespeed = nullptr;
    QComboBox *cb_writespeed = nullptr;
    QCheckBox *cb_iclose = nullptr;
    QLabel *lb_postburn = nullptr;
    QCheckBox *cb_checkdisc = nullptr;
    QCheckBox *cb_eject = nullptr;
    QString dev{""};
    QHash<QString, int> speedmap;
    DUrl image_file;
    int window_id = 0;
    QString lastVolName;

    Q_DECLARE_PUBLIC(BurnOptDialog)
};

#endif // BURNOPTDIALOG_H
