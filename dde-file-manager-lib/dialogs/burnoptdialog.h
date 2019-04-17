#ifndef BURNOPTDIALOG_H
#define BURNOPTDIALOG_H
#include "ddialog.h"

class BurnOptDialogPrivate;
class BurnOptDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    BurnOptDialog(QWidget *parent = 0);
    ~BurnOptDialog();
private:
    QScopedPointer<BurnOptDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE(BurnOptDialog)

};

#endif // BURNOPTDIALOG_H
