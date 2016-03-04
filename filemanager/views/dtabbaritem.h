#ifndef DTABBARITEM_H
#define DTABBARITEM_H

#include <QLabel>

class DTabBar;

class DTabBarItem : public QLabel
{
public:
    DTabBarItem(DTabBar * tabBar, const QString & text, int index);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void setDefaultColor(QColor color);
    void setSelectedColor(QColor color);
    void setBorderColor(QColor color);
    bool isSelected();
    void setSelected(bool select);
private:
    DTabBar * m_tabBar;
    int m_index;
    QColor m_backgroundColor;
    QColor m_selectedColor;
    QColor m_borderColor;
    bool m_selected;
};

#endif // DTABBARITEM_H
