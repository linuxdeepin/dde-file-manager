#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include <ddialog.h>

DWIDGET_USE_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DTitlebar;
DWIDGET_END_NAMESPACE

class QShowEvent;
class QWidget;

class BaseDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit BaseDialog(QWidget *parent = 0);
    ~BaseDialog();

    void setTitle(const QString &title);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    DTitlebar *m_titlebar;
};

#endif // BASEDIALOG_H
