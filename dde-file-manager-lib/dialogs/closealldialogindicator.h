#ifndef CLOSEALLDIALOGINDICATOR_H
#define CLOSEALLDIALOGINDICATOR_H

#include <QLabel>
#include <QPushButton>

#include <dabstractdialog.h>

DWIDGET_USE_NAMESPACE

class CloseAllDialogIndicator : public DAbstractDialog
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
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel* m_messageLabel;
    QPushButton* m_closeButton;
};

#endif // CLOSEALLDIALOGINDICATOR_H
