#ifndef DHOVERBUTTON_H
#define DHOVERBUTTON_H

#include <QPushButton>

class DHoverButton : public QPushButton
{
    Q_OBJECT
public:
    DHoverButton(const QString &normal, const QString &hover, QWidget * parent = 0);
private:
    void initUI();
    QString m_normal;
    QString m_hover;
};

#endif // DHOVERBUTTON_H
