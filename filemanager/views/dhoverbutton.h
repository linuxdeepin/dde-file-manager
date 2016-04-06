#ifndef DHOVERBUTTON_H
#define DHOVERBUTTON_H

#include <QPushButton>
#include <QLabel>
#include <QIcon>

class DFileMenu;

class DHoverButton : public QPushButton
{
    Q_OBJECT
public:
    DHoverButton(const QString &normal, const QString &hover, QWidget * parent = 0);
    void setMenu(DFileMenu *menu);
protected:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
private:
    void initUI();
    QLabel* m_iconLabel;
    QIcon m_normal;
    QIcon m_hover;
    DFileMenu * m_menu = NULL;
};

#endif // DHOVERBUTTON_H
