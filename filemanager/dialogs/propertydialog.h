#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include <dwidget.h>

QT_BEGIN_NAMESPACE
class QLabel;
class QTextEdit;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DExpandGroup;
DWIDGET_END_NAMESPACE

class AbstractFileInfo;

DWIDGET_USE_NAMESPACE

class PropertyDialog : public DWidget
{
    Q_OBJECT

public:
    explicit PropertyDialog(AbstractFileInfo *info, const QIcon &icon,
                            QWidget *parent = 0);

private:
    QLabel *m_icon;
    QTextEdit *m_edit;

    DExpandGroup *addExpandWidget(const QStringList &titleList);
};

#endif // PROPERTYDIALOG_H
