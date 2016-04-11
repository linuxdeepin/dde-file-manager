#ifndef MESSAGEWRONGDIALOG_H
#define MESSAGEWRONGDIALOG_H

#include "dbasedialog.h"

class MessageWrongDialog : public DBaseDialog
{
    Q_OBJECT
public:
    explicit MessageWrongDialog(const QString& msg, QWidget *parent = 0);
    ~MessageWrongDialog();

signals:

public slots:
};

#endif // MESSAGEWRONGDIALOG_H
