#ifndef CLOSEALLDIALOGINDICATOR_H
#define CLOSEALLDIALOGINDICATOR_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>

class CloseAllDialogIndicator : public QDialog
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
    void setTotalMessage(qint64 size, int count);

protected:
//    void showEvent(QShowEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    QLabel* m_messageLabel;
    QPushButton* m_closeButton;
};

#endif // CLOSEALLDIALOGINDICATOR_H
