#include "dbookmarkitem.h"
#include "dbookmarkitemgroup.h"

#include "dfilemenu.h"
#include "dfilemenumanager.h"
#include "windowmanager.h"
#include "ddragwidget.h"
#include "ddialog.h"
#include "dbookmarkmountedindicatoritem.h"
#include "deditorwidgetmenu.h"

#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QSvgRenderer>


#include "models/bookmark.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "app/filemanagerapp.h"

#include "shutil/iconprovider.h"

#include "controllers/bookmarkmanager.h"
#include "controllers/appcontroller.h"
#include "dfileservices.h"

#include "dialogs/dialogmanager.h"

#include "deviceinfo/udiskdeviceinfo.h"

#include "widgets/singleton.h"

DWIDGET_USE_NAMESPACE

DBookmarkItem::DBookmarkItem()
{
    init();
    m_isDefault = true;
}

DBookmarkItem::DBookmarkItem(UDiskDeviceInfo * deviceInfo)
{
    init();
    setDeviceInfo(deviceInfo);
}

DBookmarkItem::DBookmarkItem(BookMark *bookmark)
{
    init();
    m_url = bookmark->getUrl();
    m_textContent = bookmark->getName();
}

void DBookmarkItem::setDeviceInfo(UDiskDeviceInfo *deviceInfo)
{
    setHighlightDiskBackgroundEnable(true);
    m_isDisk = true;
    m_checkable = true;
    m_url = deviceInfo->getMountPointUrl();
    m_isDefault = true;
    m_sysPath = deviceInfo->getDiskInfo().ID;
    m_textContent = deviceInfo->fileDisplayName();
    m_isMounted = deviceInfo->getDiskInfo().CanUnmount;
    m_deviceInfo = deviceInfo;

    if (!m_mountBookmarkItem)
        m_mountBookmarkItem = makeMountBookmark(this);
    updateMountIndicator();
}

void DBookmarkItem::init()
{
    setAcceptHoverEvents(true);
    setReleaseBackgroundColor(QColor(238,232,205,0));
    setPressBackgroundColor(QColor(44,167,248,255));
    setHoverBackgroundColor(QColor(0, 0, 0, 12));
    setHighlightDiskBackgroundColor(QColor("#C0C0C0"));
    setPressBackgroundEnable(true);
    setReleaseBackgroundEnable(true);
    setHoverBackgroundEnable(true);
    setHighlightDiskBackgroundEnable(false);
    setTextColor(Qt::black);
    setAcceptDrops(true);
    m_checkable = true;
}

void DBookmarkItem::editFinished()
{
    if (!m_lineEdit || m_eidtMenu->isVisible())
        return;

    DFMEvent event;
    event << windowId();
    event << m_url;
    if(m_group)
        event.setBookmarkIndex(m_group->items()->indexOf(this));

    if (!m_lineEdit->text().isEmpty() && m_lineEdit->text() != m_textContent)
    {
        bookmarkManager->renameBookmark(getBookmarkModel(), m_lineEdit->text());
        emit fileSignalManager->bookmarkRenamed(m_lineEdit->text(), event);
        m_textContent = m_lineEdit->text();
    }

    m_widget->deleteLater();
    m_lineEdit = Q_NULLPTR;

    emit fileSignalManager->requestFoucsOnFileView(event);
}

void DBookmarkItem::checkMountedItem(const DFMEvent &event)
{
    if (event.windowId() != windowId()){
        return;
    }

    if (m_isDisk && m_deviceInfo){
        qDebug() << event;

        if(m_group)
        {
            m_group->deselectAll();
            setChecked(true);
        }
        m_pressed = false;
        update();

        DFMEvent e;
        e << windowId();
        e << m_url;
        e << DFMEvent::LeftSideBar;
        e.setBookmarkIndex(m_group->items()->indexOf(this));
        qDebug() << m_isDisk << m_deviceInfo << m_url;
        m_group->url(e);
    }
}

QRectF DBookmarkItem::boundingRect() const
{
    return QRectF(m_x_axis - m_adjust,
                  m_y_axis - m_adjust,
                  m_width + m_adjust,
                  m_height + m_adjust);
}

void DBookmarkItem::setTightMode(bool v)
{
    m_isTightMode = v;
}

bool DBookmarkItem::isTightModel()
{
    return m_isTightMode;
}

void DBookmarkItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->setClipRect(option->exposedRect);
//    double w = m_width;
    QColor textColor;
    double leftPadding = 13;
    double dy;
    QPixmap press;
    QPixmap checked;
    QPixmap release;
    QPixmap hover;
    if(m_isTightMode)
    {
        press = m_pressImageBig;
        if (m_checkedImageBig.isNull())
            checked = press;
        else
            checked = m_checkedImageBig;
        release = m_releaseImageBig;
        hover = m_hoverImageBig;

        dy = m_height/2 - m_releaseImageBig.height()/2;
    }
    else
    {
        press = m_pressImage;
        if (m_checkedImage.isNull())
            checked = press;
        else
            checked = m_checkedImage;
        release = m_releaseImage;
        hover = m_hoverImage;
        dy = m_height/2 - m_releaseImage.height()/2;
    }

    if(m_hovered && !m_isHighlightDisk)
    {
        if(m_hoverBackgroundEnabled)
        {
            painter->setBrush(m_hoverBackgroundColor);
            painter->setPen(QColor(0,0,0,0));
            painter->drawRect(m_x_axis, m_y_axis, m_width, m_height);
        }
        painter->drawPixmap(leftPadding, dy,  hover.width(), hover.height(), hover);
    }
    else if(m_pressed)
    {
        if(m_pressBackgroundEnabled)
        {
            painter->setPen(m_pressBackgroundColor);
            painter->setBrush(m_pressBackgroundColor);
            painter->drawRect(m_x_axis, m_y_axis, m_width, m_height);
            textColor = Qt::white;
        }
        painter->drawPixmap(leftPadding, dy, press.width(), press.height(), press);
    }else if(!m_hovered && (m_checked && m_checkable))
    {
        if(m_pressBackgroundEnabled)
        {
            painter->setPen(m_pressBackgroundColor);
            painter->setBrush(m_pressBackgroundColor);
            painter->drawRect(m_x_axis, m_y_axis, m_width, m_height);
            textColor = Qt::white;
        }
        painter->drawPixmap(leftPadding, dy, checked.width(), checked.height(), checked);
    }else if (m_isHighlightDisk){
        if(m_highlightDiskBackgroundEnabled)
        {
            painter->setPen(m_highlightDiskBackgroundColor);
            painter->setBrush(m_highlightDiskBackgroundColor);
            painter->drawRect(m_x_axis, m_y_axis, m_width, m_height);
            textColor = Qt::white;
        }
        painter->drawPixmap(leftPadding, dy, release.width(), release.height(), checked);
    }
    else
    {
        if(m_releaseBackgroundEnabled)
        {
            painter->setPen(m_releaseBackgroundColor);
            painter->setBrush(m_releaseBackgroundColor);
            painter->drawRect(m_x_axis, m_y_axis, m_width, m_height);
        }

        painter->drawPixmap(leftPadding, dy, release.width(), release.height(), release);
        textColor = Qt::black;
    }

    double textTopPadding = 4;
    if(!m_isTightMode)
    {
        painter->setPen(textColor);
        painter->setFont(m_font);
        QRect rect(leftPadding + 24, m_y_axis, m_width - 25, m_height);
        QFontMetrics metrics(m_font);
        QString elidedText = metrics.elidedText(m_textContent, Qt::ElideMiddle, m_width - leftPadding - 60);
        painter->drawText(rect,Qt::TextWordWrap|Qt::AlignLeft| Qt::AlignVCenter, elidedText);
    }
    else
    {
        if(!m_isDefault && !m_textContent.isEmpty())
        {
            QString text = m_textContent.at(0);
            painter->setPen(QColor(0x2CA7F8));
            QFont font;
            font.setPointSize(8);
            painter->setFont(font);

            painter->drawText(leftPadding + 6, m_y_axis + m_height*3/4 - textTopPadding, text);
        }
    }

//    if(m_isMounted)
//    {
//        QPixmap pressPic(":/icons/images/icons/unmount_press.svg");
//        QPixmap normalPic(":/icons/images/icons/unmount_normal.svg");
//        QPixmap hoverPic(":/icons/images/icons/unmount_press.svg");
//        if(m_pressed || (m_checked && m_checkable))
//            painter->drawPixmap(w - 20, pressPic.height()/2,
//                                pressPic.width(), pressPic.height(), pressPic);
//        else if(m_hovered)
//            painter->drawPixmap(w - 20, hoverPic.height()/2,
//                                hoverPic.width(), hoverPic.height(), hoverPic);
//        else
//            painter->drawPixmap(w - 20, normalPic.height()/2,
//                                normalPic.width(), normalPic.height(), normalPic);
//    }
}

void DBookmarkItem::boundImageToPress(QString imagePath)
{
    m_pressImage.load(imagePath);
}

void DBookmarkItem::boundImageToRelease(QString imagePath)
{
    m_releaseImage.load(imagePath);
}

void DBookmarkItem::boundImageToHover(QString imagePath)
{
    m_hoverImage.load(imagePath);
}

void DBookmarkItem::boundImageToChecked(QString imagePath)
{
    m_checkedImage.load(imagePath);
}

QPixmap DBookmarkItem::getCheckedPixmap()
{
    return m_checkedImage;
}

void DBookmarkItem::boundBigImageToPress(QString imagePath)
{
    m_pressImageBig.load(imagePath);
}

void DBookmarkItem::boundBigImageToRelease(QString imagePath)
{
    m_releaseImageBig.load(imagePath);
}

void DBookmarkItem::boundBigImageToHover(QString imagePath)
{
    m_hoverImageBig.load(imagePath);
}

void DBookmarkItem::boundBigImageToChecked(QString imagePath)
{
    m_checkedImageBig.load(imagePath);
}

void DBookmarkItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(m_isDraggable)
        {
            m_xPress = event->pos().x();
            m_yPress = event->pos().y();
        }
        m_pressed = true;
        m_hovered = false;
        update();
    }
}

void DBookmarkItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_pressed&&m_isDraggable)
    {
        m_xOffset = event->pos().x() - m_xPress;
        m_xPos += m_xOffset;
        m_xPress = event->pos().x();

        m_yOffset = event->pos().y() - m_yPress;
        m_yPos += m_yOffset;
        m_yPress = event->pos().y();
        moveBy(m_xPos,m_yPos);
        update();
    }

    if(m_pressed && !m_isDefault)
    {
        if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton))
                .length() < QApplication::startDragDistance()) {
                return;
            }

        drag = new DDragWidget((QObject*)event->widget());
        QMimeData *mimeData = new QMimeData;
        drag->setPixmap(toPixmap());
        drag->setMimeData(mimeData);
        drag->setHotSpot(QPoint(m_width/4, 4));
        drag->startDrag();
        m_pressed = false;
        drag = NULL;
        QPoint point = scene()->views().at(0)->mapFromGlobal(QCursor::pos());
        QPointF point1 = scene()->views().at(0)->mapToScene(point);
        emit dragFinished(QCursor::pos(), point1, this);
    }
}

QPixmap DBookmarkItem::toPixmap()
{
    if (!scene()) {
        return QPixmap();
    }

    QSvgRenderer renderer;

    renderer.load(QString(":/icons/images/icons/reordering_line.svg"));

    QImage image;
    QPainter painter;

    if (m_isTightMode)
        image = QImage(60, renderer.defaultSize().height(), QImage::Format_ARGB32);
    else
        image = QImage(200, renderer.defaultSize().height() * 200.0 / renderer.defaultSize().width(), QImage::Format_ARGB32);

    image.fill(Qt::transparent);
    painter.begin(&image);
    renderer.render(&painter, QRect(QPoint(0, 0), image.size()));

    return QPixmap::fromImage(image);
}

bool DBookmarkItem::isMounted()
{
    return m_isMounted;
}

void DBookmarkItem::setMounted(bool v)
{
    m_isMounted = v;
    update();
    updateMountIndicator();
}

void DBookmarkItem::setDeviceLabel(const QString &label)
{
    m_deviceLabel = label;
}

QString DBookmarkItem::getDeviceLabel()
{
    return m_deviceLabel;
}

void DBookmarkItem::setSysPath(const QString &path)
{
    m_sysPath = path;
}

QString DBookmarkItem::getSysPath()
{
    return m_sysPath;
}

int DBookmarkItem::windowId()
{
    return WindowManager::getWindowId(scene()->views().at(0));
}

BookMark *DBookmarkItem::getBookmarkModel()
{
    return m_bookmarkModel;
}

void DBookmarkItem::setBookmarkModel(BookMark *bookmark)
{
    m_bookmarkModel = bookmark;
}

DBookmarkItem *DBookmarkItem::makeBookmark(const QString &name, const DUrl &url)
{
    DBookmarkItem * item = new DBookmarkItem;
    item->setDefaultItem(false);
    item->setText(name);
    item->setUrl(url);
    item->boundImageToHover(":/icons/images/icons/bookmarks_normal_16px.svg");
    item->boundImageToPress(":/icons/images/icons/bookmarks_checked_16px.svg");
    item->boundImageToRelease(":/icons/images/icons/bookmarks_normal_16px.svg");
    item->boundBigImageToHover(":/icons/images/icons/favourite_hover.svg");
    item->boundBigImageToPress(":/icons/images/icons/favourite_checked.svg");
    item->boundBigImageToRelease(":/icons/images/icons/favourite_normal.svg");
    return item;
}

DBookmarkMountedIndicatorItem *DBookmarkItem::makeMountBookmark(DBookmarkItem *parentItem)
{
    DBookmarkMountedIndicatorItem * item = new DBookmarkMountedIndicatorItem(parentItem);
    return item;
}

void DBookmarkItem::editMode()
{
    m_lineEdit = new QLineEdit;
//    m_lineEdit->setContextMenuPolicy(Qt::NoContextMenu);

    m_eidtMenu = new DEditorWidgetMenu(m_lineEdit);

    connect(m_lineEdit, &QLineEdit::editingFinished, this, &DBookmarkItem::editFinished);
    m_widget = scene()->addWidget(m_lineEdit);
    m_lineEdit->setGeometry(37 + geometry().x(), geometry().y(), m_width - 37, m_height);
    m_lineEdit->setText(m_textContent);
    m_lineEdit->setSelection(0, m_textContent.length());
    m_lineEdit->setFocus();
    m_lineEdit->installEventFilter(this);
    m_lineEdit->setStyleSheet("QLineEdit {\
                              background: #0b8ade;\
                              color:white;\
                              selection-background-color: #70bfff;\
                              padding-left: 6px;\
                              border-radius: 4px;\
                          }");
    m_lineEdit->show();

}

void DBookmarkItem::updateMountIndicator()
{
    qDebug() << m_isMounted << m_mountBookmarkItem;
    if (m_isMounted && m_mountBookmarkItem){
        m_mountBookmarkItem->show();
    }else{
        m_mountBookmarkItem->hide();
    }
}

QSizeF DBookmarkItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);
    return QSizeF(m_width, m_height);
}

void DBookmarkItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if(m_group && m_pressed)
    {
        emit clicked();

        if(m_group)
        {
            m_group->deselectAll();
            setChecked(true);
        }
        m_pressed = false;
        update();

        QDir dir(m_url.path());
        if(!dir.exists() && !m_isDefault)
        {
            DFMEvent event;
            event << m_url;
            event << DFMEvent::LeftSideBar;
            event << windowId();
            if(m_group)
                event.setBookmarkIndex(m_group->items()->indexOf(this));
            int result = dialogManager->showRemoveBookMarkDialog(event);
            if (result == DDialog::Accepted)
            {
                fileSignalManager->requestBookmarkRemove(event);
            }
        }
        else
        {
            DFMEvent e;
            e << windowId();
            if(m_url.isBookMarkFile())
                e << DUrl::fromLocalFile(m_url.path());
            else
                e << m_url;
            e << DFMEvent::LeftSideBar;
            qDebug() << m_isDisk << m_deviceInfo << m_url;

            if (m_isDisk){
                if (m_deviceInfo){
                    DiskInfo info = m_deviceInfo->getDiskInfo();
                    qDebug() << info << m_url << m_isMounted;
                    if (!m_isMounted){
                        m_url.setQuery(m_sysPath);
                        e << m_url;
                        DUrlList urls;
                        urls.append(m_url);
                        e << urls;
                        appController->actionOpenDisk(e);
                    }else{
                        qDebug() << e;
                        emit m_group->url(e);
                    }
                }
            }else{
                emit m_group->url(e);
            }
            scene()->views().at(0)->ensureVisible(this, -10, 0);
        }
    }
}

void DBookmarkItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    emit doubleClicked();
}

void DBookmarkItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    m_hovered = true;
    event->accept();
    emit dragEntered();
    update();
}

void DBookmarkItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event)
    m_hovered = false;
    emit dragLeft();
    QGraphicsItem::dragLeaveEvent(event);
    update();
}

void DBookmarkItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    m_hovered = true;
    event->accept();
}

void DBookmarkItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    event->acceptProposedAction();
    m_hovered = false;
    update();
    emit dropped();
    if(!event->mimeData()->hasUrls())
        return;
    if(m_isDisk)
        return;
    DFMEvent e;
    e << DFMEvent::LeftSideBar;
    e << m_url;
    e << DUrl::fromQUrlList(event->mimeData()->urls());
    e << windowId();

    fileService->pasteFile(DAbstractFileController::CopyType, m_url, e);
    QGraphicsItem::dropEvent(event);
}

bool DBookmarkItem::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_lineEdit) {
        if (e->type() == QEvent::FocusOut) {
            editFinished();

            return false;
        } else if (e->type() == QEvent::KeyPress) {
            QKeyEvent *key_event = static_cast<QKeyEvent*>(e);

            if (key_event->key() == Qt::Key_Escape) {
                m_widget->deleteLater();
                m_lineEdit = Q_NULLPTR;
            }
        }
    }

    return false;
}

void DBookmarkItem::keyPressEvent(QKeyEvent *event)
{
    QGraphicsItem::keyPressEvent(event);
}

void DBookmarkItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    event->accept();
    if (m_checked){

    }else{
        m_hovered = true;
        update();
    }
}

void DBookmarkItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    event->accept();
    if(!m_isMenuOpened)
    {
        m_hovered = false;
        update();
    }
}

void DBookmarkItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
    m_isMenuOpened = true;
    DFileMenu *menu;

    QSet<MenuAction> disableList;

    const bool tabAddable = WindowManager::tabAddableByWinId(windowId());
    if(!tabAddable || !QFile::exists(m_url.toLocalFile()))
        disableList << MenuAction::OpenInNewTab;

    if (m_url.isRecentFile()){
        menu = DFileMenuManager::createRecentLeftBarMenu(disableList);
    }else if (m_url.isTrashFile()){
        menu = DFileMenuManager::createTrashLeftBarMenu(disableList);
    }else if(m_isDisk && m_deviceInfo)
    {
        disableList |= m_deviceInfo->disableMenuActionList();
        m_url.setQuery(m_sysPath);

        m_deviceInfo->canUnmount();

        menu = DFileMenuManager::genereteMenuByKeys(
                    m_deviceInfo->menuActionList(DAbstractFileInfo::SingleFile),
                    disableList);
    }else if (m_url.isNetWorkFile()){
        menu = DFileMenuManager::createNetworkMarkMenu(disableList);
    }else if(m_url.isUserShareFile()){
        menu = DFileMenuManager::createUserShareMarkMenu(disableList);
    }else if(m_isDefault)
        menu = DFileMenuManager::createDefaultBookMarkMenu(disableList);
    else
        menu = DFileMenuManager::createCustomBookMarkMenu(m_url, disableList);

    QPointer<DBookmarkItem> me = this;


    DUrlList urls;
    urls.append(m_url);

    DFMEvent fmEvent;
    fmEvent << m_url;
    fmEvent << urls;
    fmEvent << windowId();
    fmEvent << DFMEvent::LeftSideBar;
    if(m_group)
        fmEvent.setBookmarkIndex(m_group->items()->indexOf(this));

    menu->setEvent(fmEvent);

    menu->exec();
    menu->deleteLater();

    if(me) {
        m_isMenuOpened = false;
        m_hovered = false;
        update();
    }
}

void DBookmarkItem::setDraggable(bool b)
{
    m_isDraggable = b;
}

void DBookmarkItem::setBounds(int x, int y, int w, int h)
{
    m_x_axis = x;
    m_y_axis = y;
    m_width = w;
    m_height = h;
}

void DBookmarkItem::setPressBackgroundColor(const QColor &color)
{
    m_pressBackgroundColor = color;
}

void DBookmarkItem::setReleaseBackgroundColor(const QColor &color)
{
   m_releaseBackgroundColor = color;
}

void DBookmarkItem::setHoverBackgroundColor(const QColor &color)
{
    m_hoverBackgroundColor = color;
}

void DBookmarkItem::setHighlightDiskBackgroundColor(const QColor &color)
{
    m_highlightDiskBackgroundColor = color;
}

void DBookmarkItem::setHoverEnableFlag(bool flag)
{
     setAcceptHoverEvents(flag);
}

void DBookmarkItem::setAdjust(qreal value)
{
    m_adjust = value;
}

bool DBookmarkItem::isPressed()
{
    return m_pressed;
}

bool DBookmarkItem::isHovered()
{
    return m_hovered;
}

void DBookmarkItem::setHovered(bool flag)
{
    m_hovered = flag;
}

void DBookmarkItem::setBackgroundEnable(bool flag)
{
    m_backgroundEnabled = flag;
}

int DBookmarkItem::boundX()
{
    return m_x_axis;
}

int DBookmarkItem::boundY()
{
    return m_y_axis;
}

int DBookmarkItem::boundWidth()
{
    return m_width;
}

int DBookmarkItem::boundHeight()
{
    return m_height;
}

void DBookmarkItem::setText(const QString & text)
{
    m_textContent = text;
    update();
}

QString DBookmarkItem::text()
{
    return m_textContent;
}

void DBookmarkItem::setTextColor(const QColor &color)
{
    m_textColor = color;
}

QColor DBookmarkItem::getTextColor()
{
    return m_textColor;
}

void DBookmarkItem::setPress(bool b)
{
    m_pressed = b;
}

void DBookmarkItem::setPressBackgroundEnable(bool b)
{
    m_pressBackgroundEnabled = b;
}

void DBookmarkItem::setReleaseBackgroundEnable(bool b)
{
    m_releaseBackgroundEnabled = b;
}

void DBookmarkItem::setHoverBackgroundEnable(bool b)
{
    m_hoverBackgroundEnabled = b;
}

void DBookmarkItem::setHighlightDiskBackgroundEnable(bool b)
{
    m_highlightDiskBackgroundEnabled = b;
}

void DBookmarkItem::setHighlightDisk(bool isHighlight)
{
    m_isHighlightDisk = isHighlight;
    if (!m_checked){
        update();
    }
}

bool DBookmarkItem::isHighlightDisk()
{
    return m_isHighlightDisk;
}

void DBookmarkItem::setUrl(const DUrl &url)
{
    m_url = url;
}

DUrl DBookmarkItem::getUrl()
{
    return m_url;
}

void DBookmarkItem::setCheckable(bool b)
{
    m_checkable = b;
}

void DBookmarkItem::setChecked(bool b)
{
    m_checked = b;
    if (m_mountBookmarkItem){
        m_mountBookmarkItem->setChecked(b);
    }
    update();
}

bool DBookmarkItem::isChecked()
{
    return m_checked;
}

void DBookmarkItem::setItemGroup(DBookmarkItemGroup *group)
{
    m_group = group;
}

void DBookmarkItem::setWidth(double w)
{
    m_width = w;
}

void DBookmarkItem::setHeight(double h)
{
    m_height = h;
}

void DBookmarkItem::setDefaultItem(bool v)
{
    m_isDefault = v;
}

bool DBookmarkItem::isDefaultItem()
{
    return m_isDefault;
}

bool DBookmarkItem::isDiskItem()
{
    return m_isDisk;
}
