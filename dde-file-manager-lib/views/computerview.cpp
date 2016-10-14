#include "computerview.h"
#include "flowlayout.h"
#include "dfilemenu.h"
#include "filemenumanager.h"
#include "windowmanager.h"

#include "app/global.h"
#include "app/filesignalmanager.h"
#include "fmevent.h"
#include "app/filemanagerapp.h"
#include "fileservices.h"
#include "controllers/pathmanager.h"
#include "controllers/appcontroller.h"
#include "deviceinfo/udisklistener.h"
#include "abstractfileinfo.h"
#include "shutil/standardpath.h"
#include "widgets/singleton.h"

#include <dscrollbar.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDebug>
#include <QTextEdit>
#include <QSizePolicy>
#include <QFile>

DWIDGET_USE_NAMESPACE


TitleLine::TitleLine(const QString &title, QWidget *parent):
    QFrame(parent),
    m_title(title)
{
    initUI();
}

void TitleLine::initUI()
{
    m_titleLable = new QLabel(m_title);
    m_titleLable->setStyleSheet("color: rgba(0, 0, 0, 0.5)");
    m_lineLable = new QLabel;
    m_lineLable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_lineLable->setFixedHeight(1);
    m_lineLable->setStyleSheet("background-color:rgba(0, 0, 0, 0.1)");

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_titleLable);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_lineLable, 100, Qt::AlignCenter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
}

void TitleLine::resizeEvent(QResizeEvent *event)
{
    m_lineLable->setFixedWidth(event->size().width() - m_titleLable->width() - 20);
}


ComputerViewItem::ComputerViewItem(QWidget *parent):
    FileIconItem(parent)
{
    getTextEdit()->setReadOnly(true);
    getTextEdit()->setTextInteractionFlags(Qt::NoTextInteraction);
    getTextEdit()->setStyleSheet("border:1px solid red");
}

QIcon ComputerViewItem::getIcon(int size)
{
    if (m_info){
        return m_info->fileIcon();
    }else if (m_deviceInfo){
        return m_deviceInfo->fileIcon(size, size);
    }
    return QIcon();
}


AbstractFileInfoPointer ComputerViewItem::info() const
{
    return m_info;
}

void ComputerViewItem::setInfo(const AbstractFileInfoPointer &info)
{
    m_info = info;
}


UDiskDeviceInfo *ComputerViewItem::deviceInfo() const
{
    return m_deviceInfo;
}

void ComputerViewItem::setDeviceInfo(UDiskDeviceInfo *deviceInfo)
{
    m_deviceInfo = deviceInfo;
}

int ComputerViewItem::windowId()
{
    return WindowManager::getWindowId(this);
}

void ComputerViewItem::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event)
    setChecked(true);
    DFileMenu *menu;
    DUrl url;
    QSet<MenuAction> disableList;
    const bool& tabAddable = WindowManager::tabAddableByWinId(windowId());
    if(!tabAddable)
        disableList << MenuAction::OpenInNewTab;
    if (m_info){
        menu = FileMenuManager::createDefaultBookMarkMenu(disableList);
        url = m_info->fileUrl();
    }else if (m_deviceInfo){
        if (m_deviceInfo->getMountPoint() == "/" && !m_deviceInfo->getDiskInfo().isNativeCustom){
            menu = FileMenuManager::createDefaultBookMarkMenu(disableList);
            url = m_deviceInfo->getMountPointUrl();
        }else if (m_deviceInfo->getDiskInfo().isNativeCustom){
            menu = FileMenuManager::createDefaultBookMarkMenu(disableList);
            url = m_deviceInfo->getMountPointUrl();
        }
        else{
            disableList |= m_deviceInfo->disableMenuActionList();
            menu = FileMenuManager::genereteMenuByKeys(
                        m_deviceInfo->menuActionList(AbstractFileInfo::SingleFile),
                        disableList);
            url = m_deviceInfo->getMountPointUrl();
            url.setQuery(m_deviceInfo->getId());
        }
    }

    DUrlList urls;
    urls.append(url);

    FMEvent fmEvent;
    fmEvent << url;
    fmEvent << urls;
    fmEvent << windowId();

    menu->setEvent(fmEvent);
    menu->exec();
    menu->deleteLater();

}

void ComputerViewItem::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    setChecked(true);
    FileIconItem::mousePressEvent(event);
}

void ComputerViewItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton){
        FMEvent fevent;
        fevent << windowId();
        if (m_info){
            fevent << m_info->fileUrl();
            emit fileSignalManager->requestChangeCurrentUrl(fevent);
        }else if (m_deviceInfo){
            DUrl url = m_deviceInfo->getMountPointUrl();
            if (!m_deviceInfo->getDiskInfo().CanUnmount){
                url.setQuery(m_deviceInfo->getId());
                fevent << url;
                DUrlList urls;
                urls.append(url);
                fevent << urls;
                appController->actionOpenDisk(fevent);
            }else{
                fevent << url;
                emit fileSignalManager->requestChangeCurrentUrl(fevent);
            }

        }
    }
    FileIconItem::mouseDoubleClickEvent(event);

}

bool ComputerViewItem::checked() const
{
    return m_checked;
}

void ComputerViewItem::setChecked(bool checked)
{
    if (checked != m_checked){
        m_checked = checked;
        updateStatus();
        emit checkChanged(checked);
    }
}

void ComputerViewItem::updateStatus()
{
    if (m_checked){
        setIconSizeState(m_iconSize, QIcon::Selected);
        setDisplayName(m_name);
        if (fontMetrics().width(m_name) < width()){
            getTextEdit()->setFixedWidth(fontMetrics().width(m_name) + 10);
        }
        getTextEdit()->setStyleSheet("border-radius:4px;background-color: #2da6f7; color:white");
    }else{
        setIconSizeState(m_iconSize, QIcon::Normal);
        QString ds = Global::elideText(m_name,
                          QSize(width(), 40),
                          fontMetrics(),
                          QTextOption::WrapAtWordBoundaryOrAnywhere,
                          Qt::ElideMiddle);
        getTextEdit()->setStyleSheet("background-color: transparent");
        setDisplayName(ds.remove('\n'));
    }
}


int ComputerViewItem::iconSize() const
{
    return m_iconSize;
}

void ComputerViewItem::setIconSize(int size)
{
    m_iconSize = size;
}

void ComputerViewItem::setIconSizeState(int iconSize, QIcon::Mode mode)
{
    getIconLabel()->setFixedSize(QSize(iconSize, iconSize));
    if (m_info){
        getIconLabel()->setPixmap(getIcon(iconSize).pixmap(iconSize, iconSize, mode));
    }else if (m_deviceInfo){
        getIconLabel()->setPixmap(getIcon(iconSize).pixmap(iconSize, iconSize, mode));
    }
}
QString ComputerViewItem::name() const
{
    return m_name;
}

void ComputerViewItem::setName(const QString &name)
{
    m_name = name;
}

QString ComputerViewItem::displayName() const
{
    return m_displayName;
}

void ComputerViewItem::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
    getTextEdit()->setText(m_displayName);
}



ComputerView::ComputerView(QWidget *parent) : QScrollArea(parent)
{
    initData();
    initUI();
    initConnect();
}

ComputerView::~ComputerView()
{

}

DUrl ComputerView::url()
{
    return DUrl::fromComputerFile("/");
}

void ComputerView::initData()
{
    m_iconSizes << 48 << 64 << 96 << 128 << 256;
    m_systemPathKeys << "Desktop"
                     << "Videos"
                     << "Music"
                     << "Pictures"
                     << "Documents"
                     << "Downloads";

}

void ComputerView::initUI()
{
    setObjectName("ComputerView");
    setWidgetResizable(true);
    QFrame* contentFrame = new QFrame(this);
    contentFrame->setStyleSheet("background-color: transparent");

    m_systemTitleLine = new TitleLine(tr("My Directories"));
    m_systemFlowLayout = new FlowLayout();
    m_systemFlowLayout->setContentsMargins(20, 20, 20, 20);
    m_systemFlowLayout->setHorizontalSpacing(40);
    m_systemFlowLayout->setVorizontalSpacing(40);

    m_nativeTitleLine = new TitleLine(tr("Internal Disk"));
    m_nativeFlowLayout = new FlowLayout();
    m_nativeFlowLayout->setContentsMargins(20, 20, 20, 20);
    m_nativeFlowLayout->setHorizontalSpacing(40);
    m_nativeFlowLayout->setVorizontalSpacing(40);

    m_removableTitleLine = new TitleLine(tr("External Disk"));
    m_removableFlowLayout = new FlowLayout();
    m_removableFlowLayout->setContentsMargins(20, 20, 20, 20);
    m_removableFlowLayout->setHorizontalSpacing(40);
    m_removableFlowLayout->setVorizontalSpacing(40);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_systemTitleLine);
    mainLayout->addLayout(m_systemFlowLayout);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_nativeTitleLine);
    mainLayout->addLayout(m_nativeFlowLayout);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_removableTitleLine);
    mainLayout->addLayout(m_removableFlowLayout);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(20, 20, 20, 20);
    contentFrame->setLayout(mainLayout);

    setWidget(contentFrame);
    setVerticalScrollBar(new DScrollBar);

    loadSystemItems();

    if (isDiskConfExisted()){
        loadCustomItems();
    }else{
        loadNativeItems();
    }

    resizeItemBySizeIndex(m_currentIconSizeIndex);

    if (m_removableItems.count() == 0){
        m_removableTitleLine->hide();
    }
}

void ComputerView::initConnect()
{
    connect(deviceListener, &UDiskListener::volumeRemoved, this, &ComputerView::volumeRemoved);
    connect(deviceListener, &UDiskListener::mountAdded, this, &ComputerView::mountAdded);
    connect(deviceListener, &UDiskListener::mountRemoved, this, &ComputerView::mountRemoved);
}

void ComputerView::loadSystemItems()
{
    foreach (QString key, m_systemPathKeys) {
        QString path = systemPathManager->getSystemPath(key);
        DUrl url = DUrl::fromLocalFile(path);
        const AbstractFileInfoPointer &fileInfo = FileServices::instance()->createFileInfo(url);
        ComputerViewItem* item = new ComputerViewItem;
        item->setInfo(fileInfo);
        item->setName(fileInfo->displayName());
        m_systemFlowLayout->addWidget(item);
        m_systemItems.insert(path, item);
    }
}

void ComputerView::loadNativeItems()
{
    UDiskDeviceInfo* device = new UDiskDeviceInfo();
    DiskInfo info;
    info.ID = "/";
    info.CanEject = false;
    info.CanUnmount = false;
    info.Type = "native";
    info.Name = tr("Disk");
    info.MountPoint = "/";
    info.Total = 0;
    info.Used = 0;
    info.MountPointUrl = DUrl::fromLocalFile("/");
    device->setDiskInfo(info);
    mountAdded(device);
    foreach (UDiskDeviceInfo* device, deviceListener->getDeviceList()) {
        mountAdded(device);
    }
}

void ComputerView::loadCustomItems()
{
    QSettings diskSettings(getDiskConfPath(), QSettings::IniFormat);
    diskSettings.beginGroup("Disk");
    foreach (QString key, diskSettings.childKeys()) {
        loadCustomItemsByNameUrl(key, diskSettings.value(key).toString());
    }
    diskSettings.endGroup();
}

void ComputerView::loadCustomItemsByNameUrl(const QString &id, const QString &url)
{
    UDiskDeviceInfo* device = new UDiskDeviceInfo();
    DiskInfo info;
    info.ID = id;
    info.CanEject = false;
    info.CanUnmount = false;
    info.Type = "native";
    info.Name = id;
    info.MountPoint = url;
    info.Total = 0;
    info.Used = 0;
    info.MountPointUrl = DUrl::fromLocalFile(url);
    info.isNativeCustom = true;
    device->setDiskInfo(info);
    mountAdded(device);
}

bool ComputerView::isDiskConfExisted()
{
    if (QFile(getDiskConfPath()).exists()){
        return true;
    }
    return false;
}

QString ComputerView::getDiskConfPath()
{
    return QString("%1/%2").arg(StandardPath::getConfigPath(), "disk.conf");
}

void ComputerView::volumeAdded(UDiskDeviceInfo *device)
{
    Q_UNUSED(device)
}

void ComputerView::volumeRemoved(UDiskDeviceInfo *device)
{
    if (m_nativeItems.contains(device->getId())){
        ComputerViewItem* item = m_nativeItems.value(device->getId());
        m_nativeFlowLayout->removeWidget(item);
        m_nativeItems.remove(device->getId());
        item->deleteLater();
    }else if (m_removableItems.contains(device->getId())){
        ComputerViewItem* item = m_removableItems.value(device->getId());
        m_removableFlowLayout->removeWidget(item);
        m_removableItems.remove(device->getId());
        item->deleteLater();
        if (m_removableItems.count() == 0){
            m_removableTitleLine->hide();
        }
    }
}

void ComputerView::mountAdded(UDiskDeviceInfo *device)
{
    if (m_nativeItems.contains(device->getId())){
        m_nativeItems.value(device->getId())->setDeviceInfo(device);
        return;
    }else if (m_removableItems.contains(device->getId())){
        m_removableItems.value(device->getId())->setDeviceInfo(device);
        return;
    }
    else{
        qDebug() << device->getDiskInfo() << device->displayName();
        ComputerViewItem* item = new ComputerViewItem;
        item->setDeviceInfo(device);
        item->setName(device->displayName());

        if (device->getMediaType() == UDiskDeviceInfo::native){
            m_nativeFlowLayout->addWidget(item);
            m_nativeItems.insert(device->getId(), item);
        }else{
            m_removableFlowLayout->addWidget(item);
            m_removableItems.insert(device->getId(), item);
            if (m_removableItems.count() > 0){
                m_removableTitleLine->show();
            }
        }
        resizeItemBySizeIndex(m_currentIconSizeIndex);
    }
}

void ComputerView::mountRemoved(UDiskDeviceInfo *device)
{
    Q_UNUSED(device);
}

void ComputerView::enlargeIcon()
{

    if(m_currentIconSizeIndex < m_iconSizes.count() - 1)
        ++m_currentIconSizeIndex;
    resizeItemBySizeIndex(m_currentIconSizeIndex);
}

void ComputerView::shrinkIcon()
{
    if(m_currentIconSizeIndex > 0)
        --m_currentIconSizeIndex;

    resizeItemBySizeIndex(m_currentIconSizeIndex);
}

void ComputerView::resizeItemBySizeIndex(int index)
{
    int size = m_iconSizes.at(index);
    foreach (ComputerViewItem* item, m_systemItems) {
        item->setFixedWidth(size+30);
        item->setIconSize(size);
        item->updateStatus();
    }
    foreach (ComputerViewItem* item, m_nativeItems) {
        item->setFixedWidth(size+30);
        item->setIconSize(size);
        item->updateStatus();
    }
    foreach (ComputerViewItem* item, m_removableItems) {
        item->setFixedWidth(size+30);
        item->setIconSize(size);
        item->updateStatus();
    }
}

void ComputerView::resizeEvent(QResizeEvent *event)
{
    widget()->setFixedWidth(event->size().width());
    QScrollArea::resizeEvent(event);
}

void ComputerView::wheelEvent(QWheelEvent *event)
{
    if(Global::keyCtrlIsPressed()) {
        if(event->angleDelta().y() > 0) {
            enlargeIcon();
        } else {
            shrinkIcon();
        }
    }else{
        verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition() - event->angleDelta().y());
    }
}

void ComputerView::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = widget()->mapFromParent(event->pos());

    foreach (ComputerViewItem* item, m_systemItems) {

        if (item->geometry().contains(pos)){
            item->setChecked(true);
        }else{
            item->setChecked(false);
        }
    }
    foreach (ComputerViewItem* item, m_nativeItems) {
        if (item->geometry().contains(pos)){
            item->setChecked(true);
        }else{
            item->setChecked(false);
        }
    }
    foreach (ComputerViewItem* item, m_removableItems) {
        if (item->geometry().contains(pos)){
            item->setChecked(true);
        }else{
            item->setChecked(false);
        }
    }
}

void ComputerView::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    foreach (ComputerViewItem* item, m_systemItems) {
        item->setChecked(false);
    }
    foreach (ComputerViewItem* item, m_nativeItems) {
        item->setChecked(false);
    }
    foreach (ComputerViewItem* item, m_removableItems) {
        item->setChecked(false);
    }
}
