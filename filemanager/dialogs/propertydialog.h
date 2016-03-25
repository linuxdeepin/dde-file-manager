#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include <dwidget.h>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class PropertyDialog : public DWidget
{
    Q_OBJECT

public:
    explicit PropertyDialog(QWidget *parent = 0);

private:
    QLabel *m_icon;
};

#endif // PROPERTYDIALOG_H
