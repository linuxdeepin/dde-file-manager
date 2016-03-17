#ifndef DCRUMBBUTTON_H
#define DCRUMBBUTTON_H

#include <QPushButton>

class DCrumbButton : public QPushButton
{
    Q_OBJECT
public:
    DCrumbButton(int index, const QString &text, QWidget *parent = 0);
    DCrumbButton(int index, const QIcon& icon, const QString &text, QWidget *parent = 0);
    int getIndex();
    QString getName();
private:
    int m_index;
    QString m_name;
protected:
    void paintEvent(QPaintEvent *e);
};

#endif // DCRUMBBUTTON_H
