#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QWidget>
#include "ddialog.h"

DWIDGET_USE_NAMESPACE

class MessageDialog : public DDialog
{
    Q_OBJECT
public:
    explicit MessageDialog(QWidget *parent = 0);
    explicit MessageDialog(const QString& message = "", QWidget *parent = 0);
    void initUI();

signals:

public slots:

private:
    QString m_msg;
};

#endif // MESSAGEDIALOG_H
