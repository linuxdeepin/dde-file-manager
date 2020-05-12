#ifndef WIDGETSTARTVAULT_H
#define WIDGETSTARTVAULT_H

#include <QWidget>

class QPushButton;

class WidgetStartVault : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetStartVault(QWidget *parent = nullptr);
    ~WidgetStartVault();

signals:
    void sigAccepted();

private slots:
    void slotStartBtnClicked();

public slots:

private:
    QPushButton         *m_pStartBtn;   // 开启包厢按钮
};

#endif // WIDGETSTARTVAULT_H
