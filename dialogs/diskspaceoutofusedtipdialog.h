#ifndef DISKSPACEOUTOFUSEDTIPDIALOG_H
#define DISKSPACEOUTOFUSEDTIPDIALOG_H

#include <QObject>
#include "ddialog.h"

DWIDGET_USE_NAMESPACE

class DiskSpaceOutOfUsedTipDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DiskSpaceOutOfUsedTipDialog(QWidget *parent = 0);

signals:

public slots:
};

#endif // DISKSPACEOUTOFUSEDTIPDIALOG_H
