#ifndef CLEARTRASHDIALOG_H
#define CLEARTRASHDIALOG_H

#include "dbasedialog.h"

class ClearTrashDialog : public DBaseDialog
{
    Q_OBJECT
public:
    explicit ClearTrashDialog(QWidget *parent = 0);
    ~ClearTrashDialog();

signals:

public slots:
    void handleKeyEnter();
};

#endif // CLEARTRASHDIALOG_H
