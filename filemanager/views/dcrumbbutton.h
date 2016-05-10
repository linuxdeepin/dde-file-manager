#ifndef DCRUMBBUTTON_H
#define DCRUMBBUTTON_H

#include <QPushButton>
#include <QListWidgetItem>

class DCrumbButton : public QPushButton
{
    Q_OBJECT
public:
    DCrumbButton(int index, const QString &text, QWidget *parent = 0);
    DCrumbButton(int index, const QIcon& icon, const QString &text, QWidget *parent = 0);
    int getIndex();
    QString getName();
    void setItem(QListWidgetItem * item);
    QListWidgetItem * getItem();
    QString path() const;
    void setPath(const QString &path);

private:
    int m_index;
    QString m_name;
    QString m_path;
    QListWidgetItem * m_item;
protected:
    void paintEvent(QPaintEvent *e);
};

class DCrumbIconButton : public DCrumbButton
{
    Q_OBJECT
public:
    DCrumbIconButton(int index, const QIcon& normalIcon, const QIcon& hoverIcon, const QIcon& checkedIcon, const QString &text, QWidget *parent = 0);
private:
    QIcon m_normalIcon;
    QIcon m_hoverIcon;
    QIcon m_checkedIcon;
protected:
    void checkStateSet();
    void nextCheckState();
};

#endif // DCRUMBBUTTON_H
