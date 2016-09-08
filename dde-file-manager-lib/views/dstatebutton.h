#ifndef DSTATEBUTTON_H
#define DSTATEBUTTON_H

#include <QPushButton>

class DStateButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DStateButton(const QString& iconA, const QString& iconB, QWidget *parent = 0);
    explicit DStateButton(const QString& iconA, QWidget *parent = 0);
    ~DStateButton();

    enum ButtonState{
        stateA,
        stateB
    };

    ButtonState getButtonState();

signals:


public slots:
    void setAStateIcon();
    void setBStateIcon();
    void toogleStateICon();
    void setButtonState(ButtonState state);

private:
    QString m_aStateIcon = "";
    QString m_bStateIcon = "";
    ButtonState m_buttonState;
};

#endif // DSTATEBUTTON_H
