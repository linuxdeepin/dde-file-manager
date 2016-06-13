#ifndef CLOSEALLDIALOGINDICATOR_H
#define CLOSEALLDIALOGINDICATOR_H

#include "basedialog.h"
#include <QLabel>
#include <QPushButton>

class CloseAllDialogIndicator : public BaseDialog
{
    Q_OBJECT
public:
    explicit CloseAllDialogIndicator(QWidget *parent = 0);
    ~CloseAllDialogIndicator();

    void initUI();
    void initConnect();

signals:
    void allClosed();

public slots:
    void setTotalMessage(int size, int count);

protected:
//    void showEvent(QShowEvent* event);

private:
    QLabel* m_messageLabel;
    QPushButton* m_closeButton;
};

#endif // CLOSEALLDIALOGINDICATOR_H
