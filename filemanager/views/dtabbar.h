#ifndef DTABBAR_H
#define DTABBAR_H

#include <QFrame>
#include <QHBoxLayout>

class DTabBarItem;

class DTabBar : public QFrame
{
    Q_OBJECT
public:
    explicit DTabBar(QWidget *parent = 0);
    void addTab(const QString &text);
    void removeTab(int index);
    int count();
    void doLayout();
    void clearAllTabs();
    void setMaximumTabCount(int count);
    void setTabs(QStringList list);
    QString shrinkToIndex(int index);
    QString moveToIndex(int index);
    void setTabSelected(int index);
    int getSelectedIndex();
private:
    void deSelectAll();
    QList<DTabBarItem *> m_items;
    QString m_currentAddress;
    int m_addressOffset;
    int m_count;
signals:
    void tabBarClicked(int index);
    void tabBarChanged(QString path);
public slots:
    void setTabBarPath(const QString & text);
};

#endif // DTABBAR_H
