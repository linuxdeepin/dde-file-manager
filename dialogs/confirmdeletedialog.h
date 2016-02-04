#ifndef CONFIRMDELETEDIALOG_H
#define CONFIRMDELETEDIALOG_H

#include "dbasedialog.h"

class ConfirmDeleteDialog : public DBaseDialog
{
    Q_OBJECT
public:
    explicit ConfirmDeleteDialog(QWidget *parent = 0);
    ~ConfirmDeleteDialog();

signals:

public slots:
    void handleKeyEnter();
};

#endif // CONFIRMDELETEDIALOG_H
