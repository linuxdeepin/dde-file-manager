#include "dtabbar.h"


DTabCloseButton::DTabCloseButton(QWidget *parent):QPushButton(parent){

}


DTabBar::DTabBar(QWidget *parent) : QTabBar(parent)
{
    initConnections();
    setMovable(true);
    setAutoHide(true);
    setDrawBase(true);
    setUsesScrollButtons(false);
    setExpanding(true);
    installEventFilter(this);
    setMouseTracking(true);
    setElideMode(Qt::ElideMiddle);
}
void DTabBar::initConnections(){
    connect(this, &DTabBar::currentChanged , this, &DTabBar::onCurrentIndexChanged);
    connect(this, &DTabBar::tabCloseRequested, this, &DTabBar::onTabCloseRequest);
}

void DTabBar::onCurrentIndexChanged(int index){
    const int winId = window()->winId();
    FMEvent event;
    event = winId;
    emit requestCurrentFileViewChanged(tabData(index).toInt() , event);
}

void DTabBar::addTabWithData(const int &data, QString str){
    int index = addTab(str);
    setTabData(index,data);
    DTabCloseButton *closeButton = new DTabCloseButton(this);
    closeButton->setTabIndex(index);
    closeButton->setObjectName("TabCloseButton");
    closeButton->setFixedSize(16,25);
    QPushButton *emptyButton = new QPushButton(this);
    emptyButton->setFixedSize(16,25);
    setTabButton(index,QTabBar::RightSide,closeButton);
    setTabButton(index,QTabBar::LeftSide,emptyButton);
    emptyButton->setEnabled(false);
    emptyButton->setVisible(false);
    closeButton->hide();
    connect(closeButton, &QPushButton::clicked, this, [=]{
        emit QTabBar::tabCloseRequested(closeButton->tabIndex());
    });
    if(count()>=MAX_TAB_COUNT)
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
void DTabBar::onTabCloseRequest(const int &index){

    FMEvent event;
    event = window()->winId();
    int viewIndex = tabData(index).toInt();
    removeTab(index);
    emit requestRemoveView(viewIndex, event);
    if(count()<MAX_TAB_COUNT)
        emit tabAddableChanged(true);

    //recorrect view index
    for(int i = index; i<count(); i++){
        int newViewIndex = tabData(i).toInt();
        newViewIndex--;
        setTabData(i,newViewIndex);
    }

    //recorrect tab's button index
    for(int i = index; i<count(); i++){
        DTabCloseButton *closeButton = qobject_cast<DTabCloseButton*>(tabButton(i, QTabBar::RightSide));
        closeButton->setTabIndex(closeButton->tabIndex()-1);
    }
}
bool DTabBar::tabAddable(){
    return count()<MAX_TAB_COUNT;
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
