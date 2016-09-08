#include "dtabbar.h"
#include <QtCore>
#include <QDebug>
#include <QPropertyAnimation>
#include "app/global.h"
#include "app/fmevent.h"
#include "app/filemanagerapp.h"
#include "controllers/appcontroller.h"

#include <dutility.h>

DWIDGET_USE_NAMESPACE

Tab::Tab(QGraphicsObject *parent, int viewIndex, QString text):
    QGraphicsObject(parent)
{
    m_tabText = text;
    QJsonObject tabData;
    tabData["viewIndex"] = viewIndex;
    tabData["text"] = text;
    m_tabData = tabData;
    setAcceptHoverEvents(true);
    setFlags(ItemIsSelectable);
    setAcceptedMouseButtons(Qt::LeftButton);
}

Tab::~Tab()
{
}

void Tab::setTabIndex(int index)
{
    m_tabIndex = index;
}

int Tab::tabIndex()
{
    return m_tabIndex;
}

void Tab::setTabData(QVariant data)
{
    m_tabData = data;
}

QVariant Tab::tabData()
{
    return m_tabData;
}

void Tab::setTabText(QString text)
{
    m_tabText = text;
    update();
}

QString Tab::tabText()
{
    return m_tabText;
}

void Tab::setFixedSize(QSize size)
{
    prepareGeometryChange();
    m_width = size.width();
    m_height = size.height();
}

void Tab::setGeometry(QRect rect)
{
    prepareGeometryChange();
    setX(rect.x());
    setY(rect.y());
    m_width = rect.width();
    m_height = rect.height();
}

QRect Tab::geometry()
{
    return QRect(x(),y(),m_width,m_height);
}

int Tab::width()
{
    return m_width;
}

int Tab::height()
{
    return m_height;
}

bool Tab::isDragging()
{
    return m_isDragging;
}

void Tab::setHovered(bool hovered)
{
    m_hovered = hovered;
}

bool Tab::isDragOutSide()
{
    return m_dragOutSide;
}

QPixmap Tab::toPixmap(bool drawBorder)
{
    QImage img(300,m_height,QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter painter(&img);
    QPen pen;
    QColor color(Qt::yellow);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);

    //draw text
    QFont font;
    font.setPixelSize(12);
    painter.setFont(font);
    QFontMetrics fm(font);
    QString str = fm.elidedText(m_tabText,Qt::ElideRight,300-10);

    //draw backgound
    color.setNamedColor("#FFFFFF");
    painter.fillRect(boundingRect(),color);
    color.setNamedColor("#303030");
    pen.setColor(color);
    painter.setPen(pen);
    painter.drawText((300-fm.width(str))/2,(m_height-fm.height())/2,
                      fm.width(str),fm.height(),0,str);

    if (drawBorder) {
        QPainterPath path;
        path.addRect(0,0 ,300-1,m_height-1);
        color.setRgb(0,0,0,0.1*255);
        pen.setColor(color);
        painter.setPen(pen);
        painter.drawPath(path);
    }

    return QPixmap::fromImage(img);
}

bool Tab::isChecked() const
{
    return m_checked;
}

void Tab::setChecked(const bool check)
{
    m_checked = check;
    update();
}

bool Tab::borderLeft() const
{
    return m_borderLeft;
}

void Tab::setBorderLeft(const bool flag)
{
    m_borderLeft = flag;
}

QRectF Tab::boundingRect() const
{
    return QRectF(0,0,m_width,m_height);
}

QPainterPath Tab::shape()
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void Tab::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
//    if(tabIndex() == 0||tabIndex() ==1)
//        qDebug()<<tabIndex()<<","<<boundingRect()<<","<<shape();

    if(m_dragOutSide)
        return;
    QPen pen;
    QColor color(Qt::yellow);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(color);
    painter->setPen(pen);
    pen.setWidth(1);

    //draw text
    QFont font;
    font.setPixelSize(12);
    painter->setFont(font);
    QFontMetrics fm(font);
    QString str = fm.elidedText(m_tabText,Qt::ElideRight,m_width-10);

    //draw backgound
    if(isChecked()){
        color.setNamedColor("#FFFFFF");
        painter->fillRect(boundingRect(),color);
        color.setNamedColor("#303030");
        pen.setColor(color);
        painter->setPen(pen);
        painter->drawText((m_width-fm.width(str))/2,(m_height-fm.height())/2,
                          fm.width(str),fm.height(),0,str);
    }
    else if(m_hovered||(m_hovered&&!isChecked())){
        color.setNamedColor("#EDEDED");
        painter->fillRect(boundingRect(),color);
        color.setNamedColor("#303030");
        pen.setColor(color);
        painter->setPen(pen);
        painter->drawText((m_width-fm.width(str))/2,(m_height-fm.height())/2,
                          fm.width(str),fm.height(),0,str);
    }
    else{
        color.setNamedColor("#FAFAFA");
        painter->fillRect(boundingRect(),color);
        color.setNamedColor("#949494");
        pen.setColor(color);
        painter->setPen(pen);
        painter->drawText((m_width-fm.width(str))/2,(m_height-fm.height())/2,
                          fm.width(str),fm.height(),0,str);
    }

    //draw line
    color.setRgb(0,0,0,33);
    pen.setColor(color);
    painter->setPen(pen);
    painter->drawLine(QPointF(0,boundingRect().height()-1),
                      QPointF(boundingRect().width()-1,boundingRect().height()-1));

    painter->drawLine(QPointF(boundingRect().width()-1,0),
                      QPointF(boundingRect().width()-1,boundingRect().height()-2));
    //border left
    if(m_borderLeft){
        painter->drawLine(QPointF(0,0),
                          QPointF(0,boundingRect().height()-1));
    }
}

void Tab::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        m_pressed = true;
        m_originPos = pos();
        setZValue(3);
//        m_isDragging = true;
    }
    QGraphicsObject::mousePressEvent(event);
}

void Tab::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug()<<event->pos();
    if(m_isDragging&&!m_borderLeft){
        m_borderLeft = true;
        update();
    }

    if(event->pos().y()<-m_height || event->pos().y() > m_height*2){
        if(!m_dragOutSide){
            m_dragOutSide = true;
            update();
            emit aboutToNewWindow(tabIndex());
            emit draggingFinished();
            m_dragWidget = new DDragWidget((QObject*)event->widget());
            QMimeData *mimeData = new QMimeData;
            int radius = 20;

            const QPixmap &pixmap = toPixmap(true);
            QImage image = DUtility::dropShadow(pixmap, radius,QColor(0,0,0,0.2*255));
            QPainter pa(&image);

            pa.drawPixmap(radius, radius, pixmap);

            m_dragWidget->setPixmap(QPixmap::fromImage(image));
            m_dragWidget->setMimeData(mimeData);
            m_dragWidget->setHotSpot(QPoint(150+radius, 12+radius));
            m_dragWidget->startDrag();
            m_pressed = false;
            emit requestNewWindow(tabData().toJsonObject()["url"].toString());
        }
    }

    if(m_dragOutSide){
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    if(pos().x() == 0 && pos().x() == scene()->width() - m_width){
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }
    setPos(x() + event->pos().x() - event->lastPos().x(),0);
    draggingStarted();
    m_isDragging = true;
    if(pos().x()<0)
        setX(0);
    else if(pos().x()> scene()->width() - m_width)
        setX(scene()->width() - m_width);

    if(pos().x()> m_originPos.x()+m_width/2){
        emit moveNext(tabIndex());
        m_originPos.setX(m_originPos.x() + m_width);
    }
    else if(pos().x()< m_originPos.x() - m_width/2){
        emit movePrevius(tabIndex());
        m_originPos.setX(m_originPos.x() - m_width);
    }

    QGraphicsObject::mouseMoveEvent(event);
}

void Tab::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_dragOutSide){
        m_pressed = false;
        setZValue(1);
        QGraphicsObject::mouseReleaseEvent(event);

        m_dragOutSide = false;
        m_isDragging = false;
        if(m_dragWidget){
            m_dragWidget = NULL;
        }        return;
    }

    emit clicked();
    m_pressed = false;
    setZValue(1);
    m_isDragging = false;
    m_borderLeft = false;
    update();
    emit draggingFinished();
    QGraphicsObject::mouseReleaseEvent(event);
}

void Tab::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_hovered = true;
    QGraphicsObject::hoverEnterEvent(event);
}

void Tab::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hovered = false;
    m_pressed = false;
    QGraphicsObject::hoverLeaveEvent(event);
}

TabCloseButton::TabCloseButton(QGraphicsItem *parent):
    QGraphicsObject(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

QRectF TabCloseButton::boundingRect() const
{
    return QRectF(0,0,23,23);

}

void TabCloseButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QString imgSrc;
    if(m_mousePressed){
        if(m_activeWidthTab)
            imgSrc = ":/icons/images/icons/active_tab_close_press.png";
        else
            imgSrc = ":/icons/images/icons/tab_close_press.png";
        QPixmap pixmap(imgSrc);
        painter->drawPixmap(boundingRect().toRect(),pixmap);
    }
    else if(m_mouseHovered){
        if(m_activeWidthTab)
            imgSrc = ":/icons/images/icons/active_tab_close_hover.png";
        else
            imgSrc = ":/icons/images/icons/tab_close_hover.png";
        QPixmap pixmap(imgSrc);
        painter->drawPixmap(boundingRect().toRect(),pixmap);
    }
    else{
        if(m_activeWidthTab)
            imgSrc = ":/icons/images/icons/active_tab_close_normal.png";
        else
            imgSrc = ":/icons/images/icons/tab_close_normal.png";
        QPixmap pixmap(imgSrc);
        painter->drawPixmap(boundingRect().toRect(),pixmap);
    }
}

int TabCloseButton::closingIndex()
{
    return m_closingIndex;
}

void TabCloseButton::setClosingIndex(int index)
{
    m_closingIndex = index;
}

void TabCloseButton::setActiveWidthTab(bool active)
{
    m_activeWidthTab = active;
    update();
}

void TabCloseButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePressed = true;
    if(m_mouseHovered)
        m_mouseHovered = false;
    update();
}

void TabCloseButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePressed = false;
    emit clicked();
    update();
}

void TabCloseButton::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    event->ignore();
    if(!m_mouseHovered)
        m_mouseHovered = true;
    update();
}

void TabCloseButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit unHovered(closingIndex());
    event->ignore();
    m_mouseHovered = false;
    update();
}

void TabCloseButton::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    event->ignore();
    m_mouseHovered = true;
    emit hovered(closingIndex());
    update();
}


TabBar::TabBar(QWidget *parent):QGraphicsView(parent){
    m_scene = new QGraphicsScene(this);
    setObjectName("TabBar");
    m_scene->setSceneRect(0,0,width(),height());
    setContentsMargins(0,0,0,0);
    setScene(m_scene);

    m_TabCloseButton = new TabCloseButton;
    m_TabCloseButton->setZValue(4);
    m_TabCloseButton->hide();
    m_scene->addItem(m_TabCloseButton);
    setMouseTracking(true);

    initConnections();
    hide();
}

int TabBar::addTabWithData(const int &viewIndex, const QString text, const QString url)
{
    Tab *tab = new Tab(0,viewIndex,text);
    m_tabs.append(tab);
    m_scene->addItem(tab);

    if(isHidden()&&count()>=2)
        show();

    int index = count() - 1;
    tab->setTabIndex(index);
    QJsonObject tabData;
    tabData["viewIndex"] = viewIndex;
    tabData["text"] = text;
    tabData["url"] = url;
    tab->setTabData(QVariant(tabData));

    connect(tab, &Tab::clicked, this, [=]{
       setCurrentIndex(tab->tabIndex());
       m_TabCloseButton->setActiveWidthTab(true);
    });
    connect(tab, &Tab::moveNext, this, &TabBar::onMoveNext);
    connect(tab, &Tab::movePrevius, this, &TabBar::onMovePrevius);
    connect(tab, &Tab::requestNewWindow, this, &TabBar::onRequestNewWindow);
    connect(tab, &Tab::aboutToNewWindow, this, &TabBar::onAboutToNewWindow);
    connect(tab, &Tab::draggingFinished, this, [=]{
        m_TabCloseButton->setZValue(2);
        if(tab->isDragOutSide())
            m_TabCloseButton->hide();
        m_lastDeleteState = false;
        updateScreen();

        //hide border left line
        for(auto it: m_tabs)
            if(it->borderLeft())
                it->setBorderLeft(false);
    });
    connect(tab, &Tab::draggingStarted, this, [=]{
       m_TabCloseButton->setZValue(0);
    });
    connect(tab, &Tab::draggingStarted, this, [=]{
        int pairIndex = tab->tabIndex() +1;
        int counter = 0;
        for(auto it: m_tabs){
            if(counter == tab->tabIndex() || counter == pairIndex)
                it->setBorderLeft(true);
            else
                it->setBorderLeft(false);
            counter ++;
        }
    });

    m_lastAddTabState = true;
    setCurrentIndex(index);
    m_lastAddTabState = false;

    tabAddableChanged(count()<8);

    return index;
}

QVariant TabBar::tabData(const int index)
{
    if(index>=count()||index<0)
        return QVariant();
    return m_tabs.at(index)->tabData();
}

int TabBar::count() const
{
    return m_tabs.count();
}

void TabBar::removeTab(const int index)
{
    Tab *tab = m_tabs.at(index);

    m_tabs.removeAt(index);
    tab->deleteLater();

    for(int i = index; i<count(); i++){
        tab = m_tabs.at(i);
        tab->setTabIndex(tab->tabIndex() - 1);
    }
    if(m_TabCloseButton->closingIndex() <=count()-1 &&m_TabCloseButton->closingIndex()>=0){
        m_lastDeleteState = true;
    }
    if(index<count())
        setCurrentIndex(index);
    else
        setCurrentIndex(count()-1);
    updateScreen();
}

int TabBar::currentIndex() const
{
    return m_currentIndex;
}

void TabBar::setCurrentIndex(const int index)
{
    m_currentIndex = index;

    int counter = 0;
    for(auto tab: m_tabs){
        if(counter == index){
            tab->setChecked(true);
        }
        else{
            tab->setChecked(false);
        }
        counter ++;
    }
    emit currentChanged(index);
    updateScreen();
}

bool TabBar::tabAddable()
{
    return count()<8;
}

void TabBar::setTabData(const int &index, const QVariant &tabData)
{
    if(index<0||index>=count())
        return;
    m_tabs.at(index)->setTabData(tabData);
}

void TabBar::setTabText(const int viewIndex, const QString text, const QString url)
{
    int counter = 0;
    for(auto it:m_tabs){
        if(it->tabData().toJsonObject()["viewIndex"].toInt() == viewIndex){
            it->setTabText(text);
            QJsonObject tabData = it->tabData().toJsonObject();
            tabData["text"] = text;
            tabData["url"] = url;
            it->setTabData(QVariant(tabData));
            it->update();
        }
        counter++;
    }
}

QSize TabBar::tabSizeHint(const int &index)
{

    if(m_lastDeleteState)
        return QSize(m_tabs.at(0)->width(),m_tabs.at(0)->height());

    int averageWidth = width()/count();

    if(index == count() -1)
        return (QSize(width() - averageWidth*(count()-1),24));
    else
        return (QSize(averageWidth,24));
}

void TabBar:: updateScreen()
{
    int counter = 0;
    int lastX = 0;
    for(auto tab:m_tabs){
        QRect rect(lastX,0,tabSizeHint(counter).width(),tabSizeHint(counter).height());
        lastX = rect.x() + rect.width();
        if(tab->isDragging()){
            counter ++ ;
            continue;
        }
        if(!m_lastAddTabState){
            QPropertyAnimation *animation = new QPropertyAnimation(tab,"geometry");
            animation->setDuration(100);
            animation->setStartValue(tab->geometry());
            animation->setEndValue(rect);
            animation->start();

            connect(animation,&QPropertyAnimation::finished,[=]{
                animation->deleteLater();
                if((m_TabCloseButton->closingIndex()>=count()||m_TabCloseButton->closingIndex()<0)&&m_lastDeleteState){
                    m_TabCloseButton->hide();
                    m_lastDeleteState = false;
                    updateScreen();
                }
            });
        }
        else
            tab->setGeometry(rect);
        counter ++;
    }
    updateSceneRect(m_scene->sceneRect());
}

void TabBar::initConnections()
{
    connect(m_TabCloseButton, &TabCloseButton::hovered, this, &TabBar::onTabCloseButtonHovered);
    connect(m_TabCloseButton, &TabCloseButton::unHovered, this, &TabBar::onTabCloseButtonUnHovered);
    connect(m_TabCloseButton, &TabCloseButton::clicked, this, [=]{
        int closingIndex = m_TabCloseButton->closingIndex();
        emit tabCloseRequested(closingIndex);

        //redirect tab close button's closingIndex
        if(closingIndex>=count())
            m_TabCloseButton->setClosingIndex(--closingIndex);
    });
}

void TabBar::onMoveNext(const int fromTabIndex)
{
    Tab *fromTab = m_tabs.at(fromTabIndex);
    Tab *toTab = m_tabs.at(fromTabIndex+1);
    m_tabs.swap(fromTabIndex,fromTabIndex+1);
    fromTab->setTabIndex(fromTabIndex +1);
    toTab->setTabIndex(fromTabIndex);
    emit tabMoved(fromTabIndex,fromTabIndex+1);
    setCurrentIndex(fromTabIndex+1);
}

void TabBar::onMovePrevius(const int fromTabIndex)
{
    Tab *fromTab = m_tabs.at(fromTabIndex);
    Tab *toTab = m_tabs.at(fromTabIndex-1);
    m_tabs.swap(fromTabIndex,fromTabIndex-1);
    fromTab->setTabIndex(fromTabIndex -1);
    toTab->setTabIndex(fromTabIndex);
    emit tabMoved(fromTabIndex,fromTabIndex-1);
    setCurrentIndex(fromTabIndex-1);
}

void TabBar::onRequestNewWindow(const QString closeUrl)
{
    DUrl url = DUrl::fromUserInput(closeUrl);
    FMEvent event;
    event = FMEvent::FileView;
    event = window()->winId();
    event = url;
    appController->actionNewWindow(event);
}

void TabBar::onAboutToNewWindow(const int tabIndex)
{
    emit tabCloseRequested(tabIndex);
}

void TabBar::onTabCloseButtonUnHovered(int closingIndex)
{
    if(closingIndex<0 || closingIndex>= count())
        return;
    Tab *tab = m_tabs.at(closingIndex);
    tab->setHovered(false);
    tab->update();
}

void TabBar::onTabCloseButtonHovered(int closingIndex)
{
    Tab *tab = m_tabs.at(closingIndex);
    if(!tab)
        return;
    tab->setHovered(true);
    tab->update();
}
void TabBar::resizeEvent(QResizeEvent *event)
{
    m_scene->setSceneRect(0,0,width(),height());
    updateScreen();
    QGraphicsView::resizeEvent(event);
}

bool TabBar::event(QEvent *event)
{
    if(event->type() == event->Leave){
        m_TabCloseButton->hide();
        m_lastDeleteState = false;
        updateScreen();
    }
    return QGraphicsView::event(event);
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_TabCloseButton->isVisible())
        m_TabCloseButton->show();
    int closingIndex = 0;
    closingIndex = event->pos().x()/m_tabs.at(0)->width();

    int counter = 0;
    for(auto it:m_tabs){
        if(counter != closingIndex){
            it->setHovered(false);
            it->update();
        }
        counter ++;
    }

    if(closingIndex<count() && closingIndex>=0){
        Tab *tab = m_tabs.at(closingIndex);
        m_TabCloseButton->setClosingIndex(closingIndex);
        m_TabCloseButton->setPos(tab->x()+tab->width()-26,0);

        if(closingIndex == currentIndex())
            m_TabCloseButton->setActiveWidthTab(true);
        else
            m_TabCloseButton->setActiveWidthTab(false);

    }
    else{
        if(m_lastDeleteState){
            m_lastDeleteState = false;
            updateScreen();
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}
