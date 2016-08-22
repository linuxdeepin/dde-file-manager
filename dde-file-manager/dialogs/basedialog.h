#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include <dwidget.h>

DWIDGET_USE_NAMESPACE

class QShowEvent;
class QWidget;

class BaseDialog : public DWidget
{
    Q_OBJECT
public:
    explicit BaseDialog(QWidget *parent = 0);
    ~BaseDialog();

signals:

public slots:

protected:
    void adjustPosition(QWidget* w);
    void showEvent(QShowEvent* event);
    void keyPressEvent(QKeyEvent* event);
};

#endif // BASEDIALOG_H
