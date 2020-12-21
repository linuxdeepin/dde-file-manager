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

#endif // BURNOPTDIALOG_H
