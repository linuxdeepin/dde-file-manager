#include <QDebug>
#include <QStandardPaths>
#include <QScrollBar>

#include "dcrumbwidget.h"
#include "dcrumbbutton.h"
#include "../app/fmevent.h"
#include "windowmanager.h"
#include "dstatebutton.h"
#include "../app/global.h"


DCrumbWidget::DCrumbWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

void DCrumbWidget::initUI()
{
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
    setFixedHeight(20);
    setMinimumWidth(50);
}

void DCrumbWidget::addCrumb(const QStringList &list)
{
//    qDebug() << list;
    for(int i = 0; i < list.size(); i++)
    {
        QString text = list.at(i);
        DCrumbButton * button;
        if(isHomeFolder(text)){
            button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/home_normal_16px.svg"),
                    QIcon(":/icons/images/icons/home_hover_16px.svg"),
                    QIcon(":/icons/images/icons/home_checked_16px.svg"),
                    text, this);
        }else if(isDeviceFolder(text)){
            UDiskDeviceInfo* info = deviceListener->getDeviceByPath(text);
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
            button->setPath(path);

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
    }
    m_group.buttons().last()->setChecked(true);
}

void DCrumbWidget::setCrumb(const DUrl &path)
{
//    qDebug() << path;
    if(path.isSearchFile())
        return;
    m_path = path;
    m_needArrows = false;
    clear();
    if(path.isRecentFile())
    {
        addRecentCrumb();
    }
    else if(path.isComputerFile())
    {
        addComputerCrumb();
        addLocalCrumbs(path);
    }
    else if(path.isTrashFile())
    {
        if (path == DUrl(TRASH_ROOT)){
            addTrashCrumb();
        }else{
            addTrashCrumb();
            addLocalCrumbs(path);
        }
    }else if(path.isSMBFile()){
        addNetworkCrumb();
        addCrumb(QStringList() << path.toString());
    }else if(path.isNetWorkFile()){
        addNetworkCrumb();
    }
    else
    {
        addLocalCrumbs(path);
    }
    createCrumbs();
    repaint();
}

void DCrumbWidget::clear()
{
    m_listWidget->clear();
    m_prevCheckedId = m_group.checkedId();
    qDeleteAll(m_group.buttons());
}

QString DCrumbWidget::path()
{
    return m_path.toLocalFile();
}

DUrl DCrumbWidget::getUrl()
{
    return m_path;
}

void DCrumbWidget::addRecentCrumb()
{
    QString text = RECENT_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/recent_normal_16px.svg"),
                QIcon(":/icons/images/icons/recent_hover_16px.svg"),
                QIcon(":/icons/images/icons/recent_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addComputerCrumb()
{
    QString text = COMPUTER_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/disk_normal_16px.svg"),
                QIcon(":/icons/images/icons/disk_hover_16px.svg"),
                QIcon(":/icons/images/icons/disk_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addTrashCrumb()
{
    QString text = TRASH_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/trash_normal_16px.svg"),
                QIcon(":/icons/images/icons/trash_hover_16px.svg"),
                QIcon(":/icons/images/icons/trash_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addHomeCrumb()
{
    QString text = m_homePath;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/home_normal_16px.svg"),
                QIcon(":/icons/images/icons/home_hover_16px.svg"),
                QIcon(":/icons/images/icons/home_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addNetworkCrumb()
{
    QString text = NETWORK_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/network_normal_16px.svg"),
                QIcon(":/icons/images/icons/network_hover_16px.svg"),
                QIcon(":/icons/images/icons/network_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

DCrumbButton *DCrumbWidget::createDeviceCrumbButtonByType(UDiskDeviceInfo::MediaType type, const QString &mountPoint)
{
    DCrumbButton * button = NULL;
    switch (type) {
    case UDiskDeviceInfo::native:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/disk_normal_16px.svg"),
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
                    QIcon(":/icons/images/icons/android_normal_16px.svg"),
                    QIcon(":/icons/images/icons/android_hover_16px.svg"),
                    QIcon(":/icons/images/icons/android_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    case UDiskDeviceInfo::iphone:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/iphone_normal_16px.svg"),
                    QIcon(":/icons/images/icons/iphone_hover_16px.svg"),
                    QIcon(":/icons/images/icons/iphone_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    case UDiskDeviceInfo::removable:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/usb_normal_16px.svg"),
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
    }
    default:
        qWarning() << "unknown type";
        break;
    }

    return button;
}

void DCrumbWidget::addLocalCrumbs(const DUrl & url)
{
    QStringList list;
    QString path = url.path();
//    qDebug() << path << isInDevice(path);
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
        UDiskDeviceInfo* info = deviceListener->getDeviceByPath(path);
        QString mountPoint = info->getMountPoint();
        qDebug() << mountPoint << info << info->getDiskInfo();
        QString tmpPath = url.toLocalFile();
        tmpPath.replace(mountPoint, "");
        list.append(tmpPath.split("/"));
        list.insert(0, mountPoint);
        list.removeAll("");
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
    return m_path.toLocalFile().contains(path);
}

bool DCrumbWidget::isInHome(const QString& path)
{
    return path.startsWith(m_homePath);
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
        QListWidgetItem * item = new QListWidgetItem(m_listWidget);
        item->setSizeHint(QSize(button->size().width(), 18));
        m_listWidget->setItemWidget(item, button);
        DCrumbButton * localButton = (DCrumbButton *)button;
        localButton->setItem(item);
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
    m_leftArrow = new DStateButton(":/icons/images/icons/backward_normal.png");
    m_leftArrow->setObjectName("leftArrow");
    m_leftArrow->setFixedWidth(25);
    m_leftArrow->setFixedHeight(20);
    m_leftArrow->setFocusPolicy(Qt::NoFocus);

    m_rightArrow = new DStateButton(":/icons/images/icons/forward_normal.png");
    m_rightArrow->setObjectName("rightArrow");
    m_rightArrow->setFixedWidth(25);
    m_rightArrow->setFixedHeight(20);
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

    FMEvent event;
    event = WindowManager::getWindowId(window());
    event = FMEvent::CrumbButton;
    QString text = button->path();
    DCrumbButton * localButton = (DCrumbButton *)m_group.buttons().at(0);
    qDebug() << text << localButton->getName();

    if(localButton->getName() == RECENT_ROOT)
    {
        event = DUrl::fromRecentFile(text.isEmpty() ? "/":text);
    }
    else if(localButton->getName() == COMPUTER_ROOT)
    {
        event = DUrl::fromComputerFile(text.isEmpty() ? "/":text);
    }
    else if(localButton->getName() == TRASH_ROOT)
    {
        event = DUrl::fromTrashFile(text.isEmpty() ? "/":text);
    }else if(localButton->getName() == NETWORK_ROOT)
    {
        if (!text.isEmpty()){
            event = DUrl(text);
        }else{
            event = DUrl(NETWORK_ROOT);
        }
    }
    else if(localButton->getName() == m_homePath)
    {
        event = DUrl::fromLocalFile(text);
    }
    else
    {
        event = DUrl::fromLocalFile(text.isEmpty() ? "/":text);
    }

    m_listWidget->scrollToItem(button->getItem());
    emit crumbSelected(event);
}

void DCrumbWidget::crumbModified()
{
    setCrumb(m_path);
}

void DCrumbWidget::crumbMoveToLeft()
{
    m_listWidget->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
    m_listWidget->scrollToItem(m_listWidget->itemAt(0,0));
    checkArrows();
}

void DCrumbWidget::crumbMoveToRight()
{
    m_listWidget->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    m_listWidget->scrollToItem(m_listWidget->itemAt(m_listWidget->width() - 10,10));
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
    QListWidget::mousePressEvent(event);
    if(itemAt(event->pos()) == NULL)
    {
        emit m_crumbWidget->searchBarActivated();
    }
}
