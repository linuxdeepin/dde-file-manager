#include "dtabbar.h"
#include "windowmanager.h"

#include "app/define.h"
#include "dfmevent.h"
#include "controllers/appcontroller.h"

#include <dutility.h>

#include <QPropertyAnimation>
#include <QDebug>
#include <QDrag>

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "deviceinfo/udisklistener.h"
#include "usershare/usersharemanager.h"
#include "controllers/pathmanager.h"
#include "interfaces/dfileservices.h"
#include "plugins/pluginmanager.h"
#include "view/viewinterface.h"
#include "interfaces/dfmstandardpaths.h"

DWIDGET_USE_NAMESPACE

Tab::Tab(QGraphicsObject *parent, DFMBaseView *view):
    QGraphicsObject(parent)
{
    m_fileView = view;

    if (view)
        setCurrentUrl(view->rootUrl());

    initConnect();
    setAcceptHoverEvents(true);
    setFlags(ItemIsSelectable);
    setAcceptedMouseButtons(Qt::LeftButton);
}

Tab::~Tab()
{
}

void Tab::initConnect()
{

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

DFMBaseView *Tab::fileView()
{
    return m_fileView;
}

void Tab::setFileView(DFMBaseView *view)
{
    if (m_fileView == view)
        return;

    m_fileView = view;

    if (view)
        setCurrentUrl(view->rootUrl());
}

DUrl Tab::currentUrl() const
{
    return m_url;
}

void Tab::setCurrentUrl(const DUrl &url)
{
    m_url = url;
    setTabText(getDisplayNameByUrl(url));
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

QString Tab::getDisplayNameByUrl(const DUrl &url) const
{
    QString urlDisplayName;
    if(url.isComputerFile()){
        if(systemPathManager->isSystemPath(url.toString()))
            urlDisplayName = systemPathManager->getSystemPathDisplayNameByPath(url.toString());
    } else if(url == DUrl::fromTrashFile("/")){        urlDisplayName = systemPathManager->getSystemPathDisplayNameByPath(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath));
    } else if (PluginManager::instance()->getViewInterfacesMap().keys().contains(url.scheme())){
        urlDisplayName = PluginManager::instance()->getViewInterfaceByScheme(url.scheme())->bookMarkText();
    } else{
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(this, url);

        if (fileInfo)
            urlDisplayName = fileInfo->fileDisplayName();
    }
    return urlDisplayName;
}

QRectF Tab::boundingRect() const
{
    return QRectF(0,0,m_width,m_height);
}

QPainterPath Tab::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void Tab::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

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

    if (isChecked()){
        font.setWeight(QFont::Normal + 10);
    }else{
        font.setWeight(QFont::Normal);
    }

    painter->setFont(font);
    QFontMetrics fm(font);
    QString str = fm.elidedText(m_tabText,Qt::ElideRight,m_width-10);

    //draw backgound
    if(isChecked()){
        color.setNamedColor("#FFFFFF");
        painter->fillRect(boundingRect(),color);
        color.setNamedColor("#2ca7f8");
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

void Tab::onFileRootUrlChanged(const DUrl &url)
{
    setCurrentUrl(url);
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
    if(m_isDragging&&!m_borderLeft){
        m_borderLeft = true;
        update();
    }

    if(event->pos().y()<-m_height || event->pos().y() > m_height*2){
        if(!m_dragOutSide){
            m_dragOutSide = true;
            update();
            emit aboutToNewWindow(this);
            emit draggingFinished();
            m_dragObject = new QDrag(this);
            QMimeData *mimeData = new QMimeData;
            int radius = 20;

            const QPixmap &pixmap = toPixmap(true);
            QImage image = DUtility::dropShadow(pixmap, radius,QColor(0,0,0,0.2*255));
            QPainter pa(&image);

            pa.drawPixmap(radius, radius, pixmap);

            m_dragObject->setPixmap(QPixmap::fromImage(image));
            m_dragObject->setMimeData(mimeData);
            m_dragObject->setHotSpot(QPoint(150+radius, 12+radius));
            m_dragObject->exec();
            m_dragObject->deleteLater();
            m_pressed = false;

            emit requestNewWindow(currentUrl());
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
        emit moveNext(this);
        m_originPos.setX(m_originPos.x() + m_width);
    }
    else if(pos().x()< m_originPos.x() - m_width/2){
        emit movePrevius(this);
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
        return;
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
    Q_UNUSED(option);
    Q_UNUSED(widget);

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
    Q_UNUSED(event);

    m_mousePressed = true;
    if(m_mouseHovered)
        m_mouseHovered = false;
    update();
}

void TabCloseButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

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

void TabBar::initConnections()
{
    connect(m_TabCloseButton, &TabCloseButton::hovered, this, &TabBar::onTabCloseButtonHovered);
    connect(m_TabCloseButton, &TabCloseButton::unHovered, this, &TabBar::onTabCloseButtonUnHovered);
    connect(m_TabCloseButton, &TabCloseButton::clicked, this, [=]{
        int closingIndex = m_TabCloseButton->closingIndex();

        //effect handler
        if(closingIndex == count() - 1){
            m_historyWidth = count() * m_tabs.at(0)->width();
        }
        else{
            m_historyWidth = (count() - 1) * m_tabs.at(0)->width();
        }
        emit tabCloseRequested(closingIndex, true);

        //redirect tab close button's closingIndex
        if(closingIndex>=count())
            m_TabCloseButton->setClosingIndex(--closingIndex);
    });
}

int TabBar::createTab(DFMBaseView *view)
{
    Tab *tab = new Tab(0,view);
    m_tabs.append(tab);
    m_scene->addItem(tab);

    if(isHidden()&&count()>=2){
        show();
        emit tabBarShown();
    }

    int index = count() - 1;

    connect(tab, &Tab::clicked, this, &TabBar::onTabClicked);
    connect(tab, &Tab::moveNext, this, &TabBar::onMoveNext);
    connect(tab, &Tab::movePrevius, this, &TabBar::onMovePrevius);
    connect(tab, &Tab::requestNewWindow, this, &TabBar::onRequestNewWindow);
    connect(tab, &Tab::aboutToNewWindow, this, &TabBar::onAboutToNewWindow);
    connect(tab, &Tab::draggingFinished, this, &TabBar::onTabDragFinished);
    connect(tab, &Tab::draggingStarted, this, &TabBar::onTabDragStarted);
    connect(tab, &Tab::requestActiveNextTab, this, &TabBar::activateNextTab);
    connect(tab, &Tab::requestActivePreviousTab, this, &TabBar::activatePreviousTab);

    m_lastAddTabState = true;
    setCurrentIndex(index);
    m_lastAddTabState = false;

    tabAddableChanged(count()<TAB_MAX_COUNT);

    return index;
}

void TabBar::removeTab(const int index, const bool &remainState)
{
    Tab *tab = m_tabs.at(index);

    m_tabs.removeAt(index);
    tab->deleteLater();

    if(m_TabCloseButton->closingIndex() <=count()-1 &&
            m_TabCloseButton->closingIndex()>=0){
        m_lastDeleteState = remainState;
    }
    else{
        m_lastAddTabState = false;
        // handle tab close button display position
        if(remainState){
            QMouseEvent * event = new QMouseEvent(QMouseEvent::MouseMove,
                                                  mapFromGlobal(QCursor::pos()),
                                                  Qt::NoButton,
                                                  Qt::NoButton,
                                                  Qt::NoModifier);
            mouseMoveEvent(event);
        }
    }

    if(index<count())
        setCurrentIndex(index);
    else
        setCurrentIndex(count()-1);
    emit tabAddableChanged(count() < TAB_MAX_COUNT);

    if(count() <2){
        m_lastDeleteState = false;
        hide();
        emit tabBarHidden();
    }
}

void TabBar::setCurrentIndex(const int index)
{
    if (index < 0 || index >= m_tabs.count())
        return;

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

void TabBar::setTabText(const int &index, const QString &text)
{
    if(index>0 && index<count()){
        m_tabs.at(index)->setTabText(text);
    }
}

int TabBar::count() const
{
    return m_tabs.count();
}

int TabBar::currentIndex() const
{
    return m_currentIndex;
}

bool TabBar::tabAddable() const
{
    return count() < TAB_MAX_COUNT;
}

Tab *TabBar::currentTab()
{
    if(m_currentIndex>=0 && m_currentIndex < count())
        return m_tabs.at(currentIndex());
    return NULL;
}

Tab *TabBar::tabAt(const int &index)
{
    if(index>=0 && index < count())
        return m_tabs.at(index);
    return NULL;
}

void TabBar::onMoveNext(Tab *who)
{
    if(m_tabs.indexOf(who) >= count()-1)
        return;
    m_tabs.swap(m_tabs.indexOf(who),m_tabs.indexOf(who)+1);
    emit tabMoved(m_tabs.indexOf(who) -1,m_tabs.indexOf(who));
    setCurrentIndex(m_tabs.indexOf(who));
}

void TabBar::onMovePrevius(Tab* who)
{
    if(m_tabs.indexOf(who) <= 0)
        return;
    m_tabs.swap(m_tabs.indexOf(who),m_tabs.indexOf(who)-1);
    emit tabMoved(m_tabs.indexOf(who)+1,m_tabs.indexOf(who));
    setCurrentIndex(m_tabs.indexOf(who));
}

void TabBar::onRequestNewWindow(const DUrl url)
{
    appController->actionNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(this, DUrlList() << url));
}

void TabBar::onAboutToNewWindow(Tab* who)
{
    emit tabCloseRequested(m_tabs.indexOf(who));
}

void TabBar::onTabClicked()
{
    Tab *tab = qobject_cast<Tab*>(sender());
    if(!tab)
        return;
    setCurrentIndex(m_tabs.indexOf(tab));
    m_TabCloseButton->setActiveWidthTab(true);
}

void TabBar::onTabDragFinished()
{
    Tab* tab = qobject_cast<Tab*>(sender());
    if(!tab)
        return;
    m_TabCloseButton->setZValue(2);
    if(tab->isDragOutSide())
        m_TabCloseButton->hide();
    m_lastDeleteState = false;
    updateScreen();

    //hide border left line
    for(auto it: m_tabs)
        if(it->borderLeft())
            it->setBorderLeft(false);
}

void TabBar::onTabDragStarted()
{
    m_TabCloseButton->setZValue(0);

    Tab* tab = qobject_cast<Tab*>(sender());
    if(!tab)
        return;

    int pairIndex = m_tabs.indexOf(tab) +1;
    int counter = 0;
    for(auto it: m_tabs){
        if(counter == m_tabs.indexOf(tab) || counter == pairIndex)
            it->setBorderLeft(true);
        else
            it->setBorderLeft(false);
        counter ++;
    }
}

void TabBar::activateNextTab()
{
    if(m_currentIndex == count() - 1)
        setCurrentIndex(0);
    else
        setCurrentIndex(currentIndex() +1);

}

void TabBar::activatePreviousTab()
{
    if(m_currentIndex == 0)
        setCurrentIndex(count() - 1);
    else
        setCurrentIndex(currentIndex() - 1);
}

void TabBar::onTabCloseButtonUnHovered(int closingIndex)
{
    if(closingIndex<0 || closingIndex >= count())
        return;
    Tab *tab = m_tabs.at(closingIndex);
    tab->setHovered(false);
    tab->update();
}

void TabBar::onTabCloseButtonHovered(int closingIndex)
{
    if(closingIndex < 0 || closingIndex >= count())
        return;
    Tab *tab = m_tabs.at(closingIndex);
    if(!tab)
        return;
    tab->setHovered(true);
    tab->update();
}
void TabBar::resizeEvent(QResizeEvent *event)
{
    m_scene->setSceneRect(0,0,width(),height());
    m_historyWidth = width();
    updateScreen();
    QGraphicsView::resizeEvent(event);
}

bool TabBar::event(QEvent *event)
{
    if(event->type() == event->Leave){
        m_TabCloseButton->hide();
        m_lastDeleteState = false;
        m_historyWidth = width();
        updateScreen();
    }
    return QGraphicsView::event(event);
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_TabCloseButton->isVisible())
        m_TabCloseButton->show();

    int closingIndex = -1;
    for(int i =0; i<m_tabs.count(); i++){
        Tab *tab = m_tabs.at(i);
        if(tab->sceneBoundingRect().contains(event->pos()))
            closingIndex = i;
        else{
            tab->setHovered(false);
            tab->update();
        }
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

QSize TabBar::tabSizeHint(const int &index)
{

    if(m_lastDeleteState)
        return QSize(m_tabs.at(0)->width(),m_tabs.at(0)->height());

    int averageWidth = m_historyWidth/count();

    if(index == count() -1)
        return (QSize(m_historyWidth - averageWidth*(count()-1),24));
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

                if(m_TabCloseButton->closingIndex() == counter){
                    m_TabCloseButton->setPos(tab->x()+tab->width()-26,0);
                }
                if((m_TabCloseButton->closingIndex()>=count()||m_TabCloseButton->closingIndex()<0)
                        &&m_lastDeleteState){
                    m_lastDeleteState = false;
                }

            });
        }
        else
            tab->setGeometry(rect);
        counter ++;
    }

    updateSceneRect(m_scene->sceneRect());
}
