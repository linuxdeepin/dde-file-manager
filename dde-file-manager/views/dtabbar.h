#ifndef DTABBAR_H
#define DTABBAR_H

#include <QTabBar>
#include <QWidget>
#include "../app/global.h"
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"
#include "../models/durl.h"

#include "widgets/singleton.h"
#include <QMap>
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QJsonObject>
#include <QPushButton>
#define MAX_TAB_COUNT 8

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
    void requestRemoveView(int index , FMEvent event);
    void tabAddableChanged(bool addbable);
    void requestCurrentFileViewChanged(int index, FMEvent envent);

public slots:
    void onCurrentIndexChanged(int index);
    void onTabCloseRequest(const int &index);
protected:
    QSize tabSizeHint(int index) const ;
    QSize minimumTabSizeHint(int index) const ;
    void mouseMoveEvent(QMouseEvent *event);
    bool event(QEvent *);
private:
};

#endif // DTABBAR_H
