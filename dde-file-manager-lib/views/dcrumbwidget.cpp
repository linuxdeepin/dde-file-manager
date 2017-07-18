#include <QDebug>
#include <QStandardPaths>
#include <QScrollBar>

#include <anchors.h>

#include "dcrumbwidget.h"
#include "dcrumbbutton.h"
#include "windowmanager.h"
#include "dstatebutton.h"

#include "dfmevent.h"
#include "app/define.h"

#include "controllers/pathmanager.h"

#include "deviceinfo/udiskdeviceinfo.h"
#include "deviceinfo/udisklistener.h"

#include "singleton.h"
#include "controllers/avfsfilecontroller.h"

#include "dfilemanagerwindow.h"
#include "view/viewinterface.h"
#include "plugins/pluginmanager.h"

#include <QApplication>

DWIDGET_USE_NAMESPACE

DCrumbWidget::DCrumbWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

void DCrumbWidget::initUI()
{
    Anchors<QWidget> background_widget(new QWidget(this));

    background_widget.setFill(this);
    background_widget->setObjectName("DCrumbBackgroundWidget");

    m_homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).last();
    createArrows();
    m_listWidget = new ListWidgetPrivate(this);
    m_listWidget->setObjectName("DCrumbList");
    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->addWidget(m_leftArrow);
    m_buttonLayout->addWidget(m_listWidget);
    m_buttonLayout->addWidget(m_rightArrow);
    m_buttonLayout->setContentsMargins(0,0,0,0);
    m_buttonLayout->setSpacing(0);
    setLayout(m_buttonLayout);
    setObjectName("DCrumbWidget");
    m_listWidget->setFlow(QListWidget::LeftToRight);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    setFixedHeight(24);
    setMinimumWidth(50);
    QTimer::singleShot(1000, this, [=](){
        setCrumb(m_url);
    });
}

void DCrumbWidget::addCrumb(const QStringList &list)
{
    qDebug() << list;
    for(int i = 0; i < list.size(); i++)
    {
        QString text = list.at(i);
        text.replace("&", "&&");

        DCrumbButton * button;
        if(isHomeFolder(text)){
            button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/leftsidebar/images/leftsidebar/home_normal_16px.svg"),
                    QIcon(":/icons/images/icons/home_hover_16px.svg"),
                    QIcon(":/icons/images/icons/home_checked_16px.svg"),
                    text, this);
        }else if(isDeviceFolder(text)){
            UDiskDeviceInfoPointer info = deviceListener->getDeviceByPath(text);
            if (info->getMediaType() == UDiskDeviceInfo::camera && info->getName() == "iPhone"){
                button = createDeviceCrumbButtonByType(UDiskDeviceInfo::iphone, text);
            }else{
                button = createDeviceCrumbButtonByType(info->getMediaType(), text);
            }
        }
        else{
            button = new DCrumbButton(m_group.buttons().size(), text, this);
        }

        if (button){
            QString path = list.at(0);
            if(path == "/")
                path = "";
            for(int j = 1; j <= i; j++)
            {
                path += "/" + list.at(j);
            }

            if (!path.startsWith("/"))
                path.prepend('/');

            if(m_url.isAVFSFile()){
                QString archRootPath = AVFSFileController::findArchFileRootPath(m_url);
                if(!archRootPath.startsWith(path) || archRootPath == path)
                    button->setUrl(DUrl::fromAVFSFile(path));
                else
                    button->setUrl(DUrl::fromLocalFile(path));
            } else if(m_url.isTrashFile()){
                button->setUrl(DUrl::fromTrashFile(path));
            }else{
                button->setUrl(DUrl::fromLocalFile(path));
            }

            if (systemPathManager->systemPathsMap().values().contains(path)){
                foreach (QString key, systemPathManager->systemPathsMap().keys()) {
                    if (systemPathManager->systemPathsMap().value(key) == path){
                           button->setText(systemPathManager->getSystemPathDisplayName(key));
                    }
                }
            }

            button->setFocusPolicy(Qt::NoFocus);
            button->adjustSize();
            m_group.addButton(button, button->getIndex());
            connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
        }
        if (i == 0){
            if (button){
                button->setObjectName("DCrumbIconButton");
            }
        }
    }
    if (m_group.buttons().count() > 0){
        m_group.buttons().last()->setChecked(true);
    }
}

void DCrumbWidget::setCrumb(const DUrl &url)
{
    if(!url.isValid())
        return;
    m_url = url;
    if(url.isSearchFile())
        return;
    m_needArrows = false;
    clear();
    if(url.isRecentFile())
    {
        addRecentCrumb();
    }
    else if(url.isComputerFile())
    {
        addComputerCrumb();
    }
    else if(url.isTrashFile())
    {
        if (url.path().isEmpty()){
            addTrashCrumb();
        }else{
            addTrashCrumb();
            addCrumbs(url);
        }
    }else if(url.isSMBFile()){
        addSmbCrumb();
    }else if(url.isNetWorkFile()){
        addNetworkCrumb();
    }else if(url.isUserShareFile()){
        addUserShareCrumb();
    }/*else if (w->getViewManager()->isSchemeRegistered(url.scheme())){
        addPluginViewCrumb(url);
    }*/
    else
    {
        addCrumbs(url);
    }
    createCrumbs();
    update();
}

DUrl DCrumbWidget::backUrl()
{
    int backId = m_group.checkedId() - 1;
    if(backId >= 0){
        DCrumbButton* bnt = qobject_cast<DCrumbButton*>(m_group.button(backId));
        return bnt->url();
    }
    return DUrl::fromLocalFile(QDir::homePath());
}

void DCrumbWidget::clear()
{
    m_listWidget->clear();
    m_prevCheckedId = m_group.checkedId();
    qDeleteAll(m_group.buttons());
}

QString DCrumbWidget::path()
{
    return m_url.toLocalFile();
}

DUrl DCrumbWidget::getUrl()
{
    return m_url;
}

DUrl DCrumbWidget::getCurrentUrl()
{
    DUrl result;

    const DCrumbButton *button = qobject_cast<DCrumbButton*>(m_group.checkedButton());
    const QString &path = button ? button->url().path() : QString();
    if (m_url.isLocalFile()){
        result = DUrl::fromLocalFile(path);
    }else if (m_url.isTrashFile()){
        result = DUrl::fromTrashFile(path);
    }else if (m_url.isComputerFile()){
        result = DUrl::fromComputerFile("/");
    }else if(m_url.isSearchFile()){
        result = m_url.searchTargetUrl();
    }else
    {
        result = m_url;
    }

    return result;
}

void DCrumbWidget::addRecentCrumb()
{
    QString text = RECENT_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/leftsidebar/images/leftsidebar/recent_normal_16px.svg"),
                QIcon(":/icons/images/icons/recent_hover_16px.svg"),
                QIcon(":/icons/images/icons/recent_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();

    button->setUrl(DUrl::fromRecentFile("/"));
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addComputerCrumb()
{
    QString text = COMPUTER_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/leftsidebar/images/leftsidebar/computer_normal_16px.svg"),
                QIcon(":/icons/images/icons/computer_hover_16px.svg"),
                QIcon(":/icons/images/icons/computer_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    button->setUrl(DUrl::fromComputerFile("/"));
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addTrashCrumb()
{
    QString text = TRASH_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/leftsidebar/images/leftsidebar/trash_normal_16px.svg"),
                QIcon(":/icons/images/icons/trash_hover_16px.svg"),
                QIcon(":/icons/images/icons/trash_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    button->setUrl(DUrl::fromTrashFile("/"));
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addNetworkCrumb()
{
    QString text = NETWORK_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/leftsidebar/images/leftsidebar/network_normal_16px.svg"),
                QIcon(":/icons/images/icons/network_hover_16px.svg"),
                QIcon(":/icons/images/icons/network_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    button->setUrl(DUrl::fromNetworkFile("/"));
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addSmbCrumb()
{
    QString text = NETWORK_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/leftsidebar/images/leftsidebar/network_normal_16px.svg"),
                QIcon(":/icons/images/icons/network_hover_16px.svg"),
                QIcon(":/icons/images/icons/network_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->setUrl(m_url);
    button->setText(m_url.toString());
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addUserShareCrumb()
{
    QString text = USERSHARE_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/leftsidebar/images/leftsidebar/usershare_normal_16px.svg"),
                QIcon(":/icons/images/icons/usershare_hover_16px.svg"),
                QIcon(":/icons/images/icons/usershare_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    button->setUrl(DUrl::fromUserShareFile("/"));
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addPluginViewCrumb(const DUrl &url)
{
    QString text = url.toString();
    ViewInterface* viewInterface = PluginManager::instance()->getViewInterfaceByScheme(url.scheme());
    if (viewInterface){
        DCrumbIconButton* button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    viewInterface->crumbNormalIcon(),
                    viewInterface->crumbHoverIcon(),
                    viewInterface->crumbCheckedIcon(),
                    text, this);
        button->setText(viewInterface->crumbText());

        button->setFocusPolicy(Qt::NoFocus);
        button->adjustSize();
        button->setUrl(url);
        m_group.addButton(button, button->getIndex());
        button->setChecked(true);
        connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
    }
}


DCrumbButton *DCrumbWidget::createDeviceCrumbButtonByType(UDiskDeviceInfo::MediaType type, const QString &mountPoint)
{
    DCrumbButton * button = NULL;
    switch (type) {
    case UDiskDeviceInfo::native:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/leftsidebar/images/leftsidebar/disk_normal_16px.svg"),
                    QIcon(":/icons/images/icons/disk_hover_16px.svg"),
                    QIcon(":/icons/images/icons/disk_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    case UDiskDeviceInfo::phone:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/android_normal_16px.svg"),
                    QIcon(":/icons/images/icons/android_hover_16px.svg"),
                    QIcon(":/icons/images/icons/android_checked_16px.svg"),
                    mountPoint, this);
        break;
    }case UDiskDeviceInfo::camera:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/leftsidebar/images/leftsidebar/android_normal_16px.svg"),
                    QIcon(":/icons/images/icons/android_hover_16px.svg"),
                    QIcon(":/icons/images/icons/android_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    case UDiskDeviceInfo::iphone:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/leftsidebar/images/leftsidebar/iphone_normal_16px.svg"),
                    QIcon(":/icons/images/icons/iphone_hover_16px.svg"),
                    QIcon(":/icons/images/icons/iphone_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    case UDiskDeviceInfo::removable:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/leftsidebar/images/leftsidebar/usb_normal_16px.svg"),
                    QIcon(":/icons/images/icons/usb_hover_16px.svg"),
                    QIcon(":/icons/images/icons/usb_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    case UDiskDeviceInfo::network:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/network_normal_16px.svg"),
                    QIcon(":/icons/images/icons/network_hover_16px.svg"),
                    QIcon(":/icons/images/icons/network_checked_16px.svg"),
                    mountPoint, this);
        break;
    }case UDiskDeviceInfo::dvd:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/leftsidebar/images/leftsidebar/dvd_normal_16px.svg"),
                    QIcon(":/icons/images/icons/dvd_hover_16px.svg"),
                    QIcon(":/icons/images/icons/dvd_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    default:
        qWarning() << "unknown type";
        break;
    }

    return button;
}

void DCrumbWidget::addCrumbs(const DUrl & url)
{
    QStringList list;
    const QString &path = url.isLocalFile() ? url.toLocalFile() : url.path();
    qDebug() << path << isInHome(path) << isInDevice(path);
    if (path.isEmpty())
        return;
    if(isInHome(path))
    {
        QString tmpPath = url.toLocalFile();
        tmpPath.replace(m_homePath, "");
        list.append(tmpPath.split("/"));
        list.insert(0, m_homePath);
        list.removeAll("");
    }else if (url == DUrl(FILE_ROOT)){
        list.insert(0, "/");
    }else if(isInDevice(path)){
        UDiskDeviceInfoPointer info;
        if (deviceListener->isDeviceFolder(path)){
            info = deviceListener->getDeviceByPath(path);
        }else{
            info = deviceListener->getDeviceByFilePath(path);
        }
        if (info){
            QString mountPoint = info->getMountPointUrl().toLocalFile();
            qDebug() << mountPoint << info << info->getDiskInfo();
            QString tmpPath = path;
            tmpPath.replace(mountPoint, "");
            list.append(tmpPath.split("/"));
            list.insert(0, mountPoint);
            list.removeAll("");
        }
    }
    else
    {
        list.append(path.split("/"));
        if(url.isLocalFile())
            list.replace(0, "/");
        list.removeAll("");
    }
    if (!list.isEmpty())
        addCrumb(list);
}

bool DCrumbWidget::hasPath(const QString &path)
{
    return m_url.toLocalFile().contains(path);
}

bool DCrumbWidget::isInHome(const QString& path)
{
    return DUrl::childrenList(DUrl(path)).contains(DUrl(m_homePath));
}

bool DCrumbWidget::isHomeFolder(const QString& path)
{
    return path == m_homePath;
}

bool DCrumbWidget::isInDevice(const QString& path)
{
    return deviceListener->isInDeviceFolder(path);
}

bool DCrumbWidget::isDeviceFolder(const QString &path)
{
    return deviceListener->isDeviceFolder(path);
}

bool DCrumbWidget::isRootFolder(QString path)
{
    return path == "/";
}

void DCrumbWidget::createCrumbs()
{
    m_crumbTotalLen = 0;
    m_items.clear();
    foreach(QAbstractButton * button, m_group.buttons())
    {
        QListWidgetItem* item = new QListWidgetItem(m_listWidget);
        item->setSizeHint(QSize(button->size().width(), 18));
        m_listWidget->setItemWidget(item, button);
        DCrumbButton * localButton = (DCrumbButton *)button;
        localButton->setItem(item);
        localButton->setListWidget(m_listWidget);
        m_items.append(item);
        m_crumbTotalLen += button->size().width();
    }

    if (!m_items.isEmpty()){
        m_listWidget->scrollToItem(m_items.last(), QListWidget::PositionAtBottom);
        m_listWidget->setHorizontalScrollMode(QListWidget::ScrollPerPixel);
        m_listWidget->horizontalScrollBar()->setPageStep(m_listWidget->width());
        checkArrows();
        m_listWidget->scrollToItem(m_items.last(), QListWidget::PositionAtBottom);
    }
}

void DCrumbWidget::createArrows()
{
    m_leftArrow = new QPushButton();
    m_leftArrow->setObjectName("backButton");
    m_leftArrow->setFixedWidth(26);
    m_leftArrow->setFixedHeight(24);
    m_leftArrow->setFocusPolicy(Qt::NoFocus);

    m_rightArrow = new QPushButton();
    m_rightArrow->setObjectName("forwardButton");
    m_rightArrow->setFixedWidth(26);
    m_rightArrow->setFixedHeight(24);
    m_rightArrow->setFocusPolicy(Qt::NoFocus);
    connect(m_leftArrow, &DStateButton::clicked, this, &DCrumbWidget::crumbMoveToLeft);
    connect(m_rightArrow, &DStateButton::clicked, this, &DCrumbWidget::crumbMoveToRight);
}

void DCrumbWidget::checkArrows()
{
    if(m_crumbTotalLen < m_listWidget->width())
    {
        m_leftArrow->hide();
        m_rightArrow->hide();
    }
    else
    {
        QListWidgetItem *head = m_listWidget->itemAt(1,1);
        QListWidgetItem *end = m_listWidget->itemAt(m_listWidget->width() - 5,5);
        m_leftArrow->show();
        m_rightArrow->show();
        if(head == m_items.first())
        {
            m_leftArrow->setDisabled(true);
            m_rightArrow->setEnabled(true);
        }
        else if(end == m_items.last())
        {
            m_leftArrow->setEnabled(true);
            m_rightArrow->setDisabled(true);
        }
        else
        {
            m_leftArrow->setEnabled(true);
            m_rightArrow->setEnabled(true);
        }
    }
}

void DCrumbWidget::buttonPressed()
{
    DCrumbButton * button = static_cast<DCrumbButton*>(sender());

    DFMEvent event(this);
    event.setData(button->url());

    m_listWidget->scrollToItem(button->getItem());
    emit crumbSelected(event);
    m_listWidget->update();
}

void DCrumbWidget::crumbMoveToLeft()
{
    m_listWidget->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
    m_listWidget->scrollToItem(m_listWidget->itemAt(0,0), QAbstractItemView::PositionAtTop);
    checkArrows();
}

void DCrumbWidget::crumbMoveToRight()
{
    m_listWidget->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    QListWidgetItem* item = m_listWidget->itemAt(m_listWidget->width() - 10,10);
    if (m_listWidget->itemWidget(item)->width() > m_listWidget->width()){
        int row = m_listWidget->row(item);
        if (row == (m_listWidget->count() - 1)){
            m_listWidget->scrollToBottom();
        }else{
            m_listWidget->scrollToItem(m_listWidget->item(row + 1), QAbstractItemView::PositionAtBottom);
        }
    }else{
        m_listWidget->scrollToItem(item, QAbstractItemView::PositionAtBottom);
    }
    checkArrows();
}

void DCrumbWidget::resizeEvent(QResizeEvent *e)
{
    checkArrows();
    QFrame::resizeEvent(e);
}

ListWidgetPrivate::ListWidgetPrivate(DCrumbWidget *crumbWidget)
    :QListWidget(crumbWidget)
{
    m_crumbWidget = crumbWidget;
}

void ListWidgetPrivate::mousePressEvent(QMouseEvent *event)
{
    oldGlobalPos = event->globalPos();

    QListWidget::mousePressEvent(event);
}

void ListWidgetPrivate::mouseReleaseEvent(QMouseEvent *event)
{
    QListWidget::mouseReleaseEvent(event);

    if (oldGlobalPos == event->globalPos() && itemAt(event->pos()) == NULL) {
        emit m_crumbWidget->searchBarActivated();
    }

    oldGlobalPos = event->globalPos();
}

void ListWidgetPrivate::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}
