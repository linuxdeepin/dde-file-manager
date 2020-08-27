#ifndef DFMVAULTACTIVESTARTVIEW_H
#define DFMVAULTACTIVESTARTVIEW_H

#include <QWidget>
#include <dtkwidget_global.h>

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

class DFMVaultActiveStartView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultActiveStartView(QWidget *parent = nullptr);

signals:
    void sigAccepted();

private slots:
    void slotStartBtnClicked();

public slots:

private:
    QPushButton         *m_pStartBtn;   // 开启包厢按钮
};

#endif // DFMVAULTACTIVESTARTVIEW_H
