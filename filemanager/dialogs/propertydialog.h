#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include "basedialog.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QTextEdit;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DExpandGroup;
DWIDGET_END_NAMESPACE

class AbstractFileInfo;
class DUrl;

DWIDGET_USE_NAMESPACE

class PropertyDialog : public BaseDialog
{
    Q_OBJECT

public:
    explicit PropertyDialog(const DUrl &url, QWidget *parent = 0);

private:
    QLabel *m_icon;
    QTextEdit *m_edit;

    DExpandGroup *addExpandWidget(const QStringList &titleList);
};

#endif // PROPERTYDIALOG_H
