#include "dtabbar.h"


DTabCloseButton::DTabCloseButton(QWidget *parent):QPushButton(parent){

}


DTabBar::DTabBar(QWidget *parent) : QTabBar(parent)
{
    initConnections();
    setMovable(true);
    setAutoHide(true);
//    setDrawBase(false);
    setUsesScrollButtons(false);
    installEventFilter(this);
    setMouseTracking(true);
    setElideMode(Qt::ElideMiddle);
//    setDocumentMode(true);
}
void DTabBar::initConnections(){
    connect(this, &DTabBar::tabMoved, this, &DTabBar::onTabMoved);
}

void DTabBar::addTabWithData(const int &data, QString str){
    int index = addTab(str);
    setTabData(index,data);
    DTabCloseButton *closeButton = new DTabCloseButton(this);
    closeButton->setTabIndex(index);
    closeButton->setObjectName("TabCloseButton");
    closeButton->setFixedSize(24,24);
    QPushButton *emptyButton = new QPushButton(this);
    emptyButton->setFixedSize(24,24);
    setTabButton(index,QTabBar::RightSide,closeButton);
    setTabButton(index,QTabBar::LeftSide,emptyButton);
    emptyButton->setEnabled(false);
    emptyButton->setVisible(false);
    closeButton->hide();
    connect(closeButton, &QPushButton::clicked, this, [=]{
//        qDebug()<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< tab close bnt clicke:"<<closeButton->tabIndex();
        emit QTabBar::tabCloseRequested(closeButton->tabIndex());
    });
    if(count()>=8)
        emit tabAddableChanged(false);
}

void DTabBar::setTabText(const QString &text, const int &viewIndex, const FMEvent &event){
    if(event.windowId() != window()->winId())
        return;

    for(int i = 0; i<count(); i++){
        if(tabData(i).toInt() == viewIndex){
            QTabBar::setTabText(i,text);
        }
    }
}

QSize DTabBar::tabSizeHint(int index) const{
    int averageWidth = width()/count();
    if(index == count()-1)
        return QSize(width()-(averageWidth*(count()-1)),height());
    else
        return QSize(averageWidth,height());
}
QSize DTabBar::minimumTabSizeHint(int index) const{
    int averageWidth = width()/count();
    if(index == count()-1)
        return QSize(width()-(averageWidth*(count()-1)),height());
    else
        return QSize(averageWidth,height());
}

bool DTabBar::tabAddable(){
    return count()<8;
}
void DTabBar::mouseMoveEvent(QMouseEvent *event){
    QTabBar::mouseMoveEvent(event);
    int averageWidth = width()/count();
    int currentHoverTabIndex = event->x()/averageWidth;

    for(int i = 0; i<count();i++){
        DTabCloseButton *closeButton = qobject_cast<DTabCloseButton*>(tabButton(i, QTabBar::RightSide));
        if(i == currentHoverTabIndex && closeButton->isHidden())
            closeButton->show();
        else if(i != currentHoverTabIndex && !closeButton->isHidden())
            closeButton->hide();
    }
}
bool DTabBar::event(QEvent *event){
    if(event->type() == event->HoverLeave||event->type() == event->Leave){
        for(int i = 0; i<count(); i++){
            DTabCloseButton *closeButton =qobject_cast<DTabCloseButton*>(tabButton(i,QTabBar::RightSide)) ;
            if(!closeButton->isHidden())
                closeButton->hide();
        }
    }
    QTabBar::event(event);
}

void DTabBar::onTabMoved(const int from, const int to){
    DTabCloseButton *fromCloseBnt = qobject_cast<DTabCloseButton*>(tabButton(from, QTabBar::RightSide));
    DTabCloseButton *toCloseBnt = qobject_cast<DTabCloseButton*>(tabButton(to, QTabBar::RightSide));
    fromCloseBnt->setTabIndex(from);
    toCloseBnt->setTabIndex(to);
//    qDebug()<<"tab move , from:"<<from<<","<<"from view index:"<<tabData(from).toInt()<<
//              ","<<"to:"<<to<<", to view idnex:"<<tabData(to).toInt();
}
