#ifndef DTABBAR_H
#define DTABBAR_H

#include <QTabBar>
#include <QWidget>
#include "../app/global.h"
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"
#include "utils/durl.h"

#include "widgets/singleton.h"
#include <QDebug>
#include <QPushButton>

class DTabCloseButton : public QPushButton{
    Q_OBJECT
public:
    explicit DTabCloseButton(QWidget *parent = 0);
    int tabIndex(){return m_tabIndex;}
    void setTabIndex(int index){m_tabIndex = index;}
private:
    int m_tabIndex;
};

class DTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit DTabBar(QWidget *parent = 0);
    void initConnections();

    void addTabWithData(const int &data, QString str);
    bool tabAddable();
    void setTabText(const QString &text, const int &viewIndex, const FMEvent &event);

signals:
    void tabAddableChanged(bool addbable);

public slots:
    void onTabMoved(const int from, const int to);
protected:
    QSize tabSizeHint(int index) const ;
    QSize minimumTabSizeHint(int index) const ;
    void mouseMoveEvent(QMouseEvent *event);
    bool event(QEvent *);
private:
};

#endif // DTABBAR_H
