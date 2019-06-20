/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "computerview.h"
#include "dfilemenu.h"
#include "dfilemenumanager.h"
#include "windowmanager.h"
#include "dfmevent.h"
#include "dfmeventdispatcher.h"
#include "dfmapplication.h"
#include "dfmsettings.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfileservices.h"
#include "controllers/pathmanager.h"
#include "controllers/appcontroller.h"
#include "deviceinfo/udisklistener.h"
#include "dabstractfileinfo.h"
#include "interfaces/dfmstandardpaths.h"
#include "gvfs/gvfsmountmanager.h"
#include "singleton.h"
#include "../shutil/fileutils.h"
#include "partman/partition.h"
#include "dabstractfilewatcher.h"

#include <dslider.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDebug>
#include <QTextEdit>
#include <QSizePolicy>
#include <QFile>
#include <QStorageInfo>
#include <QSettings>
#include <QUrlQuery>
#include <QScroller>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>

#include <DApplication>

DWIDGET_USE_NAMESPACE

#define CVI_PROGRESSLINE_HEIGHT 4

TitleLine::TitleLine(const QString &title, QWidget *parent):
    QFrame(parent),
    m_title(title)
{
    initUI();
}

void TitleLine::initUI()
{
    m_titleLable = new QLabel(m_title);
    m_titleLable->setObjectName("TitleLabel");
    m_lineLable = new QFrame();
    m_lineLable->setObjectName("LineLabel");
    m_lineLable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_lineLable->setFixedHeight(1);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(m_titleLable);
    contentLayout->addSpacing(20);
    contentLayout->addWidget(m_lineLable, 100, Qt::AlignCenter);
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(contentLayout);
}

void TitleLine::resizeEvent(QResizeEvent *event)
{
    m_lineLable->setFixedWidth(event->size().width() - m_titleLable->width() - 20);
}


ComputerViewItem::ComputerViewItem(QWidget *parent):
    FileIconItem(parent)
{
    getTextEdit()->setReadOnly(true);
    getTextEdit()->setAttribute(Qt::WA_TransparentForMouseEvents);
    getTextEdit()->setTextInteractionFlags(Qt::NoTextInteraction);
    getTextEdit()->setStyleSheet("border:1px solid red");
    getTextEdit()->setObjectName("CVI_TextEdit");
    getTextEdit()->installEventFilter(this);

    m_sizeLabel = new QLabel(this);
    m_sizeLabel->setObjectName("DiskSize");
    m_sizeLabel->hide();

    m_progressLine = new ProgressLine(this);
    m_progressLine->setAlignment(Qt::AlignHCenter);
    m_progressLine->setFrameShape(QFrame::NoFrame);
    m_progressLine->setFixedSize(width(), CVI_PROGRESSLINE_HEIGHT);
    m_progressLine->setRoundRadius(CVI_PROGRESSLINE_HEIGHT / 2);
    m_progressLine->hide();

    connect(qApp, &DApplication::iconThemeChanged, this, &ComputerViewItem::updateStatus);
    connect(this, &ComputerViewItem::inputFocusOut, this, [ = ]{
//        QString newName = getTextEdit()->toPlainText();
        getTextEdit()->setReadOnly(true);
        updateStatus();
    });
}

QIcon ComputerViewItem::getIcon(int size)
{
    if (m_info) {
        return m_info->fileIcon();
    } else if (m_deviceInfo) {
        return m_deviceInfo->fileIcon(size, size);
    }
    return QIcon();
}


DAbstractFileInfoPointer ComputerViewItem::info() const
{
    return m_info;
}

void ComputerViewItem::setInfo(const DAbstractFileInfoPointer &info)
{
    m_info = info;
}


UDiskDeviceInfoPointer ComputerViewItem::deviceInfo() const
{
    return m_deviceInfo;
}

void ComputerViewItem::setDeviceInfo(UDiskDeviceInfoPointer deviceInfo)
{
    m_deviceInfo = deviceInfo;

    QString fstype = PartMan::Partition::getPartitionByDevicePath(m_deviceInfo->getDiskInfo().unix_device()).fs();

    if (fstype == "crypto_LUKS") {
        m_isLocked = true;
    }
}

int ComputerViewItem::windowId()
{
    return WindowManager::getWindowId(this);
}

void ComputerViewItem::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event)
    setChecked(true);
    DFileMenu *menu = nullptr;
    DUrl url;
    QSet<MenuAction> disableList;
    const bool &tabAddable = WindowManager::tabAddableByWinId(windowId());
    if (!tabAddable) {
        disableList << MenuAction::OpenInNewTab << MenuAction::OpenDiskInNewTab;
    }
    if (m_info) {
        menu = DFileMenuManager::createDefaultBookMarkMenu(disableList);
        url = m_info->fileUrl();
    } else if (m_deviceInfo) {
        if (m_deviceInfo->getMountPoint() == "/" && m_deviceInfo->getDiskInfo().isNativeCustom()) {
            menu = DFileMenuManager::createDefaultBookMarkMenu(disableList);
            url =  m_deviceInfo->getMountPointUrl();
        } else if (m_deviceInfo->getDiskInfo().isNativeCustom()) {
            menu = DFileMenuManager::createDefaultBookMarkMenu(disableList);
            url = m_deviceInfo->getMountPointUrl();
        } else {
            disableList |= m_deviceInfo->disableMenuActionList();
            menu = DFileMenuManager::genereteMenuByKeys(
                       m_deviceInfo->menuActionList(DAbstractFileInfo::SingleFile),
                       disableList);
            url = m_deviceInfo->getMountPointUrl();
            url.setQuery(m_deviceInfo->getId());
        }
    }

    if (menu) {
        menu->setEventData(DUrl(), DUrlList() << url, windowId(), this);
        menu->exec();
        menu->deleteLater();
    }
}

void ComputerViewItem::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    setChecked(true);

    if (event->button() == Qt::LeftButton) {
        if (DFMApplication::instance()->appAttribute(DFMApplication::AA_OpenFileMode).toInt() == 0) {
            openUrl();
        }
    }

    FileIconItem::mousePressEvent(event);
}

void ComputerViewItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (DFMApplication::instance()->appAttribute(DFMApplication::AA_OpenFileMode).toInt() == 1) {
            openUrl();
        }
    }

    FileIconItem::mouseDoubleClickEvent(event);
}

bool ComputerViewItem::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (!m_deviceInfo) {
                qWarning() << "device info not valid, can be a bug." << obj;
                break;
            }
            event->accept();
            // do rename here
            {
                DUrl url;
                url.setScheme(DEVICE_SCHEME);
                url.setPath(m_deviceInfo->getId());
                DUrl urlWithNewName = url;
                QUrlQuery query;
                query.addQueryItem("new_name", getTextEdit()->toPlainText());
                urlWithNewName.setQuery(query);
                fileService->renameFile(this, url, urlWithNewName, true);
            }
            // done
            // 重设显示的名字
            setName(m_deviceInfo->fileDisplayName());
            emit inputFocusOut();
            return true;
        case Qt::Key_Escape:
            // 重设显示的名字
            setName(m_deviceInfo->fileDisplayName());
            emit inputFocusOut();
            return true;
        default:
            break;
        }
    }
    return FileIconItem::eventFilter(obj, event);
}

bool ComputerViewItem::event(QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        resize(width(), getIconLabel()->height() + getTextEdit()->height() + ICON_MODE_ICON_SPACING + 45);
        adjustPosition();
        return true;
    }
    return FileIconItem::event(event);
}

void ComputerViewItem::adjustPosition()
{
    m_sizeLabel->setFixedWidth(this->width());
    m_sizeLabel->setAlignment(Qt::AlignCenter);
    m_sizeLabel->move(0, getTextEdit()->y() + getTextEdit()->height());
    m_progressLine->move((this->width() - m_progressLine->width()) / 2, m_sizeLabel->y() + m_sizeLabel->height() + 3);
}

bool ComputerViewItem::checked() const
{
    return m_checked;
}

void ComputerViewItem::setChecked(bool checked)
{
    if (checked != m_checked) {
        m_checked = checked;
        updateStatus();
        emit checkChanged(checked);
    }
}

void ComputerViewItem::updateStatus()
{
    if (m_checked) {
        setIconSizeState(m_iconSize, QIcon::Selected);
        setDisplayName(m_name);
        if (fontMetrics().width(m_name) < width()) {
            getTextEdit()->setFixedWidth(fontMetrics().width(m_name) + 10);
        }
        if (getTextEdit()->isReadOnly()) {
            getTextEdit()->setStyleSheet("border-radius:4px; background-color:#2da6f7; color:white");
        } else {
            getTextEdit()->setStyleSheet("");
        }
    } else {
        setIconSizeState(m_iconSize, QIcon::Normal);
        QString ds = DFMGlobal::elideText(m_name,
                                          QSize(width(), 40),
                                          QTextOption::WrapAtWordBoundaryOrAnywhere,
                                          font(),
                                          Qt::ElideMiddle, TEXT_LINE_HEIGHT);
        getTextEdit()->setStyleSheet("background-color: transparent");
        setDisplayName(ds.remove('\n'));
    }

    if (getHasMemoryInfo()) {
        updateIconPixelWidth();
        m_progressLine->setFixedSize(getPixelWidth(), CVI_PROGRESSLINE_HEIGHT);
        const qlonglong total = m_deviceInfo->getTotal();
        const qlonglong used = total - m_deviceInfo->getFree();
        m_progressLine->setMax(total);
        m_progressLine->setValue(used);
        if (m_progressLine->isHidden()) {
            m_progressLine->show();
        }

        m_sizeLabel->setText(FileUtils::diskUsageString(used, total));

        m_sizeLabel->show();
        adjustPosition();
    } else {
        m_progressLine->setFixedHeight(0);
    }

    if (m_isLocked) {
        if (m_lockedLabel) {
            m_lockedLabel->deleteLater();
        }
        m_lockedLabel = new QLabel(getIconLabel());
        m_lockedLabel->setPixmap(QIcon::fromTheme("emblem-encrypted-locked").pixmap(m_iconSize / 3, m_iconSize / 3));
        m_lockedLabel->move(m_iconSize * 2 / 3, m_iconSize * 2 / 3);
        m_lockedLabel->raise();
        m_lockedLabel->show();
    }

    if (!getTextEdit()->isReadOnly()) {
        // Editing name
        this->setBorderColor(QColor("#2da6f7"));
        getTextEdit()->setFixedWidth(width());
        getTextEdit()->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        getTextEdit()->setFocus();
    } else {
        this->setBorderColor(QColor("#002da6f7")); // argb
        getTextEdit()->setAttribute(Qt::WA_TransparentForMouseEvents);
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

void ComputerViewItem::setIconIndex(int index)
{
    m_iconIndex = index;
}

void ComputerViewItem::setIconSizeState(int iconSize, QIcon::Mode mode)
{
    getIconLabel()->setFixedSize(QSize(iconSize, iconSize));
    if (m_info) {
        getIconLabel()->setPixmap(getIcon(iconSize).pixmap(iconSize, iconSize, mode));
    } else if (m_deviceInfo) {
        getIconLabel()->setPixmap(getIcon(iconSize).pixmap(iconSize, iconSize, mode));
    }
}

void ComputerViewItem::openUrl()
{
    DUrl url;

    if (m_info) {
        url = m_info->fileUrl();
    } else if (m_deviceInfo) {
        url = m_deviceInfo->getMountPointUrl();

        QDiskInfo diskInfo = m_deviceInfo->getDiskInfo();

        if (diskInfo.can_mount() && !diskInfo.can_unmount()) {
            url.setQuery(m_deviceInfo->getId());
            appController->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(this, url));
            return;
        }
    }

    DFMOpenUrlEvent::DirOpenMode mode = DFMApplication::instance()->appAttribute(DFMApplication::AA_AllwayOpenOnNewWindow).toBool()
                                        ? DFMOpenUrlEvent::ForceOpenNewWindow
                                        : DFMOpenUrlEvent::OpenInCurrentWindow;

    if (mode == DFMOpenUrlEvent::OpenInCurrentWindow)
        DFMEventDispatcher::instance()->processEventAsync<DFMOpenUrlEvent>(this, DUrlList() << url, mode);
    else
        DFMEventDispatcher::instance()->processEvent<DFMOpenUrlEvent>(this, DUrlList() << url, mode);
}

int ComputerViewItem::getPixelWidth() const
{
    return m_pixelWidth;
}

void ComputerViewItem::setPixelWidth(int pixelWidth)
{
    m_pixelWidth = pixelWidth;
}

void ComputerViewItem::updateIconPixelWidth()
{
    const QImage img = getIconLabel()->pixmap()->toImage();
    int pixelWidth = 0;
    for (int i = 0; i < img.width(); i++) {
        QColor color = img.pixelColor(i, (int)img.height() / 2);
        if (color.alpha() > 0) {
            pixelWidth ++;
        }
    }
    setPixelWidth(pixelWidth);
}

DUrl ComputerViewItem::getUrl() const
{
    if (m_info) {
        return m_info->fileUrl();
    } else if (m_deviceInfo) {
        QDiskInfo diskInfo = m_deviceInfo->getDiskInfo();

        if (diskInfo.can_mount() && !diskInfo.can_unmount()) {
            DUrl url("mount:");
            QUrlQuery query(url);

            query.addQueryItem("id", m_deviceInfo->getId());
            url.setQuery(query);

            return url;
        } else {
            return m_deviceInfo->getMountPointUrl();
        }
    }

    return DUrl();
}

bool ComputerViewItem::getHasMemoryInfo() const
{
    return m_hasMemoryInfo;
}

void ComputerViewItem::setHasMemoryInfo(bool hasMemoryInfo)
{
    m_hasMemoryInfo = hasMemoryInfo;
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
    getTextEdit()->setPlainText(m_displayName);
}


int ComputerView::ViewInstanceCount = -1;

ComputerView::ComputerView(QWidget *parent) : QFrame(parent)
{
    initData();
    initUI();
    initConnect();
}

ComputerView::~ComputerView()
{
}

QString ComputerView::scheme()
{
    return COMPUTER_SCHEME;
}

QString ComputerView::viewId() const
{
    return m_viewId;
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
    ViewInstanceCount += 1;
    m_viewId = QString("ComputerView%1").arg(QString::number(ViewInstanceCount));

}

void ComputerView::initUI()
{
    m_contentArea = new DScrollArea(this);
    m_contentArea->setObjectName("ComputerView");
    m_contentArea->setWidgetResizable(true);

    // 支持触屏滚动，直接使用TouchGesture会导致鼠标双击事件丢失
    QScroller::grabGesture(m_contentArea, QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(m_contentArea);
    auto scroller_properties = scroller->scrollerProperties();
#if QT_VERSION < QT_VERSION_CHECK(5,9,0)
    int touchTapDistance = 15;
#else
    int touchTapDistance = QGuiApplicationPrivate::platformTheme()->themeHint(QPlatformTheme::TouchDoubleTapDistance).toInt();
#endif
    scroller_properties.setScrollMetric(QScrollerProperties::DragStartDistance, touchTapDistance / 1000);
    scroller->setScrollerProperties(scroller_properties);

    m_statusBar = new DStatusBar(this);
    m_statusBar->setFixedHeight(22);
    m_statusBar->scalingSlider()->setMaximum(m_iconSizes.count() - 1);
    m_statusBar->scalingSlider()->setMinimum(0);
    setIconSizeBySizeIndex(m_currentIconSizeIndex);
    m_statusBar->scalingSlider()->setTickInterval(1);
    m_statusBar->scalingSlider()->setPageStep(1);

    QFrame *contentFrame = new QFrame(this);

    m_systemTitleLine = new TitleLine(tr("My Directories"), this);
    m_systemFlowLayout = new DFlowLayout();
    m_systemFlowLayout->setContentsMargins(20, 20, 20, 20);
    m_systemFlowLayout->setHorizontalSpacing(40);
    m_systemFlowLayout->setVerticalSpacing(40);

    m_nativeTitleLine = new TitleLine(tr("Internal Disk"), this);
    m_nativeFlowLayout = new DFlowLayout();
    m_nativeFlowLayout->setContentsMargins(20, 20, 20, 20);
    m_nativeFlowLayout->setHorizontalSpacing(40);
    m_nativeFlowLayout->setVerticalSpacing(50);

    m_removableTitleLine = new TitleLine(tr("External Disk"), this);
    m_removableFlowLayout = new DFlowLayout();
    m_removableFlowLayout->setContentsMargins(20, 20, 20, 20);
    m_removableFlowLayout->setHorizontalSpacing(40);
    m_removableFlowLayout->setVerticalSpacing(50);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->addWidget(m_systemTitleLine);
    contentLayout->addLayout(m_systemFlowLayout);
    contentLayout->addSpacing(20);
    contentLayout->addWidget(m_nativeTitleLine);
    contentLayout->addLayout(m_nativeFlowLayout);
    contentLayout->addSpacing(20);
    contentLayout->addWidget(m_removableTitleLine);
    contentLayout->addLayout(m_removableFlowLayout);
    contentLayout->addSpacing(20);
    contentLayout->addStretch();
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentFrame->setLayout(contentLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_contentArea);
    mainLayout->addWidget(m_statusBar);
    setLayout(mainLayout);

    m_contentArea->setWidget(contentFrame);

    loadSystemItems();
    loadCustomItems();
    loadNativeItems();

    if (m_nativeItems.count() == 0) {
        m_nativeTitleLine->hide();
    }

    if (m_removableItems.count() == 0) {
        m_removableTitleLine->hide();
    }

    DFMEvent event(this);
    event.setWindowId(window()->internalWinId());
    const int number = m_systemItems.count() + m_nativeItems.count() + m_removableItems.count();
    m_statusBar->itemCounted(event, number);

    //icon size
    resizeAllItemsBySizeIndex(DFMApplication::instance()->appAttribute(DFMApplication::AA_IconSizeLevel).toInt());
    loadViewState();
}

void ComputerView::initConnect()
{
    DAbstractFileWatcher *devices_watcher = fileService->createFileWatcher(this, DUrl(DEVICE_ROOT), this);
    Q_CHECK_PTR(devices_watcher);
    devices_watcher->startWatcher();

    // Patition get renamed.
    connect(devices_watcher, &DAbstractFileWatcher::fileMoved, this, [ = ](const DUrl & fromUrl, const DUrl & toUrl) {
        Q_UNUSED(toUrl)
        const DAbstractFileInfoPointer &deviceInfoPointer = DFileService::instance()->createFileInfo(nullptr, fromUrl);
        const UDiskDeviceInfo* info = dynamic_cast<UDiskDeviceInfo*>(deviceInfoPointer.data());
        if (!deviceInfoPointer) {
            return;
        }

        DUrl url;
        if (info->getDiskInfo().isNativeCustom()) {
            url = info->getMountPointUrl();
        } else {
            url = info->getMountPointUrl();
            url.setQuery(info->getId());
        }

        ComputerViewItem *cvi = findDeviceViewItemByUrl(url);

        if (!cvi) {
            return;
        }

        QUrlQuery query(toUrl);
        QString newName = query.queryItemValue("new_name");
        if (!newName.isEmpty()) {
            cvi->setName(newName);
        } else {
            cvi->setName(deviceInfoPointer->fileDisplayName());
        }
        cvi->updateStatus();
    });

    connect(devices_watcher, &DAbstractFileWatcher::fileAttributeChanged, this, [ = ](const DUrl & url) {
        ComputerViewItem *cvi = findDeviceViewItemByUrl(url);

        if (!cvi) {
            return;
        }

        cvi->updateStatus();
    });

    connect(deviceListener, &UDiskListener::mountAdded, this, &ComputerView::mountAdded);
    connect(deviceListener, &UDiskListener::mountRemoved, this, &ComputerView::mountRemoved);
    connect(deviceListener, &UDiskListener::volumeAdded, this, &ComputerView::volumeAdded);
    connect(deviceListener, &UDiskListener::volumeRemoved, this, &ComputerView::volumeRemoved);
    connect(m_statusBar->scalingSlider(), &DSlider::valueChanged, this, &ComputerView::resizeAllItemsBySizeIndex);
    connect(m_statusBar->scalingSlider(), &DSlider::valueChanged, this, &ComputerView::saveViewState);
    connect(DFMApplication::instance(), &DFMApplication::iconSizeLevelChanged, this, &ComputerView::resizeAllItemsBySizeIndex);
    connect(fileSignalManager, &FileSignalManager::requestRename, this, &ComputerView::onRequestEdit);

}

void ComputerView::loadSystemItems()
{
    foreach (QString key, m_systemPathKeys) {
        QString path = systemPathManager->getSystemPath(key);
        DUrl url = DUrl::fromLocalFile(path);
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, url);
        ComputerViewItem *item = new ComputerViewItem;
        item->setInfo(fileInfo);
        item->setName(fileInfo->fileDisplayName());
        m_systemFlowLayout->addWidget(item);
        m_systemItems.insert(path, item);
    }
}

void ComputerView::loadNativeItems()
{
    QStorageInfo storageInfo("/");
    QDiskInfo diskInfo;
    diskInfo.setId("/");
    diskInfo.setType("native");
    diskInfo.setName(systemPathManager->getSystemPathDisplayName("System Disk"));
    diskInfo.setMounted_root_uri("/");
    diskInfo.setCan_mount(false);
    diskInfo.setCan_unmount(false);
    diskInfo.setIsNativeCustom(true);
    diskInfo.setTotal(storageInfo.bytesTotal());
    diskInfo.setFree(storageInfo.bytesFree());
    diskInfo.setUsed(diskInfo.total() - diskInfo.free());
//    diskInfo.setMounted_url(DUrl::fromLocalFile("/"));

    UDiskDeviceInfo *deviceInfo = new UDiskDeviceInfo;
    deviceInfo->setDiskInfo(diskInfo);
    UDiskDeviceInfoPointer device(deviceInfo);

    if (DFMApplication::instance()->genericAttribute(DFMApplication::GA_HiddenSystemPartition).toBool()) {
        qDebug() << "hide root sytem";
    } else {
        volumeAdded(device);
    }

    foreach (UDiskDeviceInfoPointer device, deviceListener->getDeviceList()) {
        volumeAdded(device);
    }
}

void ComputerView::loadCustomItems()
{
    const QStringList &keys = DFMApplication::genericObtuselySetting()->keyList("Disk/Custom");

    for (const QString &key : keys) {
        loadCustomItemsByNameUrl(key, DFMApplication::genericObtuselySetting()->urlValue("Disk/Custom", key).toString());
    }
}

void ComputerView::loadCustomItemsByNameUrl(const QString &id, const QString &url)
{
    QDiskInfo diskInfo;
    diskInfo.setId(id);
    diskInfo.setType("native");
    diskInfo.setName(id);
    diskInfo.setCan_mount(false);
    diskInfo.setCan_unmount(false);
    diskInfo.setMounted_root_uri(url);
//    diskInfo.setMounted_url(DUrl::fromLocalFile(url));
    diskInfo.setIsNativeCustom(true);

    UDiskDeviceInfo *deviceInfo = new UDiskDeviceInfo;
    deviceInfo->setDiskInfo(diskInfo);
    UDiskDeviceInfoPointer device(deviceInfo);
    volumeAdded(device);
}

void ComputerView::updateStatusBar()
{
    ComputerViewItem *checkedItem = NULL;
    foreach (ComputerViewItem *item, m_systemItems) {
        if (item->checked()) {
            checkedItem = item;
        }
    }
    foreach (ComputerViewItem *item, m_nativeItems) {
        if (item->checked()) {
            checkedItem = item;
        }
    }
    foreach (ComputerViewItem *item, m_removableItems) {
        if (item->checked()) {
            checkedItem = item;
        }
    }

    if (checkedItem) {
        DFMEvent event(this);
        DUrlList urlList;
        if (checkedItem->info()) {
            urlList << checkedItem->info()->fileUrl();
        }
        event.setWindowId(window()->internalWinId());
        event.setData(urlList);
        m_statusBar->itemSelected(event, 1);
    } else {
        DFMEvent event(this);
        event.setWindowId(window()->internalWinId());
        const int number = m_systemItems.count() + m_nativeItems.count() + m_removableItems.count();
        m_statusBar->itemCounted(event, number);
    }
}

void ComputerView::loadViewState()
{
    const QVariantMap &value = DFMApplication::appObtuselySetting()->value("FileViewState", COMPUTER_ROOT).toMap();

    if (!value.contains("iconSizeLevel")) {
        return;
    }

    resizeAllItemsBySizeIndex(value.value("iconSizeLevel").toInt());
}

void ComputerView::saveViewState()
{
    m_currentIconSizeIndex = m_statusBar->scalingSlider()->value();

    DFMApplication::appObtuselySetting()->setValue("FileViewState", COMPUTER_ROOT, QVariantMap {
                                                       {"iconSizeLevel", m_currentIconSizeIndex}
                                                   });
}

ComputerViewItem *ComputerView::findDeviceViewItemByUrl(const DUrl &url)
{    
    auto getContextMenuUrl = [](ComputerViewItem *oneItem) {
        DUrl url;
        UDiskDeviceInfoPointer m_deviceInfo = oneItem->deviceInfo();
        if (m_deviceInfo->getDiskInfo().isNativeCustom()) {
            url = m_deviceInfo->getMountPointUrl();
        } else {
            url = m_deviceInfo->getMountPointUrl();
            url.setQuery(m_deviceInfo->getId());
        }
        return url;
    };

    auto getDeviceId = [](ComputerViewItem *oneItem) {
        UDiskDeviceInfoPointer m_deviceInfo = oneItem->deviceInfo();
        return m_deviceInfo->getId(); // something like "/dev/sdb1"
    };

    // Find the item we want to rename in the `Internam devices` section
    foreach (ComputerViewItem *oneItem, m_nativeItems) {
        qDebug() << oneItem->getUrl() << url;
        if (getContextMenuUrl(oneItem) == url) {
            return oneItem;
        }
    }

    // not found? try `removeable devices` section
    foreach (ComputerViewItem *oneItem, m_removableItems) {
        if (getContextMenuUrl(oneItem) == url) {
            return oneItem;
        }

        if (url.scheme() == DEVICE_SCHEME && getDeviceId(oneItem) == url.path()) {
            return oneItem;
        }
    }

    // return if still not found
    return nullptr;
}

QWidget *ComputerView::widget() const
{
    return const_cast<ComputerView *>(this);
}

DUrl ComputerView::rootUrl() const
{
    return DUrl::fromComputerFile("/");
}

bool ComputerView::setRootUrl(const DUrl &url)
{
    return DUrl::fromComputerFile("/") == url;
}

void ComputerView::volumeAdded(UDiskDeviceInfoPointer device)
{
    qDebug() << "===========volumeAdded=============" << device->getId() << m_nativeItems.contains(device->getId()) << m_removableItems.contains(device->getId());
    qDebug() << device->getDiskInfo();
    ComputerViewItem *item = NULL;
    QString id = device->getId();
    if (m_nativeItems.contains(id)) {
        item = m_nativeItems.value(id);
    } else if (m_removableItems.contains(id)) {
        item = m_removableItems.value(id);
    } else {
        item = new ComputerViewItem;
    }
    item->setHasMemoryInfo(true);
    item->setDeviceInfo(device);
    item->setName(device->fileDisplayName());

    if (device->getMediaType() == UDiskDeviceInfo::native) {
        m_nativeFlowLayout->addWidget(item);
        m_nativeItems.insert(device->getId(), item);
        if (m_nativeItems.count() > 0) {
            m_nativeTitleLine->show();
        }
    } else {
        m_removableFlowLayout->addWidget(item);
        m_removableItems.insert(device->getId(), item);
        if (m_removableItems.count() > 0) {
            m_removableTitleLine->show();
        }
    }
    updateItemBySizeIndex(m_currentIconSizeIndex, item);

    updateStatusBar();
}

void ComputerView::volumeRemoved(UDiskDeviceInfoPointer device)
{
    QString deviceId = device->getId();
    QString deviceUuid = device->getDiskInfo().uuid();

    qDebug() << "===========volumeRemoved=============" << deviceId << m_nativeItems.contains(deviceId) << m_removableItems.contains(deviceId);
//    qDebug() << device->getDiskInfo();

    foreach (UDiskDeviceInfoPointer d, deviceListener->getDeviceList()) {
//        qDebug() << d->getDiskInfo().id() << deviceId << d->getDiskInfo().uuid() << deviceUuid;
        if (d->getDiskInfo().id() != deviceId && !deviceUuid.isEmpty() && d->getDiskInfo().uuid() == deviceUuid) {
            deviceId = d->getDiskInfo().id();
        }
    }


    if (m_nativeItems.contains(deviceId)) {
        ComputerViewItem *item = m_nativeItems.value(deviceId);
        m_nativeFlowLayout->removeWidget(item);
        m_nativeItems.remove(deviceId);
        item->setParent(NULL);
        delete item;
        if (m_nativeItems.count() == 0) {
            m_nativeTitleLine->hide();
        }
    } else if (m_removableItems.contains(deviceId)) {
        ComputerViewItem *item = m_removableItems.value(deviceId);
        m_removableFlowLayout->removeWidget(item);
        m_removableItems.remove(deviceId);
        item->setParent(NULL);
        delete item;
        if (m_removableItems.count() == 0) {
            m_removableTitleLine->hide();
        }
    }

    updateStatusBar();
}

void ComputerView::mountAdded(UDiskDeviceInfoPointer device)
{
    qDebug() << "===========mountAdded=============" << device->getId() << m_nativeItems.contains(device->getId()) << m_removableItems.contains(device->getId());
    if (m_nativeItems.contains(device->getId())) {
        m_nativeItems.value(device->getId())->setDeviceInfo(device);
        m_nativeItems.value(device->getId())->updateStatus();
        return;
    } else if (m_removableItems.contains(device->getId())) {
        m_removableItems.value(device->getId())->setDeviceInfo(device);
        return;
    } else {
        volumeAdded(device);
        return;
    }
    updateStatusBar();
}

void ComputerView::mountRemoved(UDiskDeviceInfoPointer device)
{
    qDebug() << "===========mountRemoved=============" << device->getId() << m_nativeItems.contains(device->getId()) << m_removableItems.contains(device->getId());
    qDebug() << device->getDiskInfo();

    if (m_nativeItems.contains(device->getId())) {
        m_nativeItems.value(device->getId())->setDeviceInfo(device);
        return;
    } else if (m_removableItems.contains(device->getId())) {

        if (device->getDiskInfo().has_volume()) {
            m_removableItems.value(device->getId())->setDeviceInfo(device);
        } else {
            ComputerViewItem *item = m_removableItems.value(device->getId());
            m_removableFlowLayout->removeWidget(item);
            m_removableItems.remove(device->getId());
            item->setParent(NULL);
            delete item;
            if (m_removableItems.count() == 0) {
                m_removableTitleLine->hide();
            }
        }
    }
    updateStatusBar();
}

void ComputerView::updateComputerItemByDevice(UDiskDeviceInfoPointer device)
{
    foreach (ComputerViewItem *item, m_nativeItems) {
        if (item->deviceInfo() == device) {
            item->updateStatus();
            return;
        }
    }
    foreach (ComputerViewItem *item, m_removableItems) {
        if (item->deviceInfo() == device) {
            item->updateStatus();
            return;
        }
    }
}

void ComputerView::enlargeIcon()
{
    if (m_currentIconSizeIndex < m_iconSizes.count() - 1) {
        ++m_currentIconSizeIndex;
    }
    resizeAllItemsBySizeIndex(m_currentIconSizeIndex);
}

void ComputerView::shrinkIcon()
{
    if (m_currentIconSizeIndex > 0) {
        --m_currentIconSizeIndex;
    }
    resizeAllItemsBySizeIndex(m_currentIconSizeIndex);
}

void ComputerView::resizeAllItemsBySizeIndex(int index)
{
    for (ComputerViewItem *item : m_systemItems) {
        updateItemBySizeIndex(index, item);
    }
    for (ComputerViewItem *item : m_nativeItems) {
        updateItemBySizeIndex(index, item);
    }
    for (ComputerViewItem *item : m_removableItems) {
        updateItemBySizeIndex(index, item);
    }

    m_currentIconSizeIndex = index;
    setIconSizeBySizeIndex(index);
}

void ComputerView::updateItemBySizeIndex(const int &index, ComputerViewItem *item)
{
    int size = m_iconSizes.at(index);
    item->setFixedWidth(size + 30);
    item->setIconIndex(index);
    item->setIconSize(size);
    item->updateStatus();
}

void ComputerView::onRequestEdit(const DFMUrlBaseEvent &event)
{
    // find item and toggle rename
    DUrl url = event.url();
    ComputerViewItem *item = findDeviceViewItemByUrl(url);

    // return if still not found
    if (!item) {
        return;
    }

    if (event.windowId() != item->windowId()) {
        return;
    }

    item->getTextEdit()->setReadOnly(false);
    item->setName(item->deviceInfo()->fileName());
    item->updateStatus();
    item->getTextEdit()->selectAll();
}

void ComputerView::resizeEvent(QResizeEvent *event)
{
    m_contentArea->widget()->setFixedWidth(event->size().width() - 16);
    QFrame::resizeEvent(event);
}

void ComputerView::wheelEvent(QWheelEvent *event)
{
    if (DFMGlobal::keyCtrlIsPressed()) {
        if (event->angleDelta().y() > 0) {
            enlargeIcon();
        } else {
            shrinkIcon();
        }
    }
    QFrame::wheelEvent(event);
}

void ComputerView::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = m_contentArea->widget()->mapFromParent(event->pos());

    foreach (ComputerViewItem *item, m_systemItems) {

        if (item->geometry().contains(pos)) {
            item->setChecked(true);
        } else {
            item->setChecked(false);
        }
    }
    foreach (ComputerViewItem *item, m_nativeItems) {

        if (item->geometry().contains(pos)) {
            item->setChecked(true);
        } else {
            item->setChecked(false);
        }
    }
    foreach (ComputerViewItem *item, m_removableItems) {

        if (item->geometry().contains(pos)) {
            item->setChecked(true);
        } else {
            item->setChecked(false);
        }
    }

    updateStatusBar();
}

void ComputerView::showEvent(QShowEvent *event)
{
//    deviceListener->refreshAsycGetAllDeviceUsage();

    foreach (ComputerViewItem *item, m_systemItems) {
        item->setChecked(false);
    }
    foreach (ComputerViewItem *item, m_nativeItems) {
        item->setChecked(false);
    }
    foreach (ComputerViewItem *item, m_removableItems) {
        item->setChecked(false);
    }

    setFocus();

    QFrame::showEvent(event);
}

void ComputerView::keyPressEvent(QKeyEvent *event)
{
    DUrlList urls;

    foreach (const ComputerViewItem *item, m_systemItems) {
        if (item->checked()) {
            urls << item->getUrl();
        }
    }
    foreach (const ComputerViewItem *item, m_nativeItems) {
        if (item->checked()) {
            urls << item->getUrl();
        }
    }
    foreach (const ComputerViewItem *item, m_removableItems) {
        if (item->checked()) {
            urls << item->getUrl();
        }
    }

    switch (event->modifiers()) {
    case Qt::NoModifier:
    case Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            appController->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));

            return;
        default: break;
        }

        break;
    case Qt::ControlModifier:
        switch (event->key()) {
        case Qt::Key_N:
            appController->actionNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls.isEmpty() ? DUrlList() << DUrl() : urls));
            return;
        case Qt::Key_I:
            appController->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));

            return;
        case Qt::Key_Down:
            appController->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));

            return;
        case Qt::Key_T: {
            //do not handle key press event of autoRepeat type
            if (event->isAutoRepeat()) {
                return;
            }

            DUrl url;

            if (urls.count() == 1) {
                url = urls.first();
            } else {
                url = DFMApplication::instance()->appUrlAttribute(DFMApplication::AA_UrlOfNewTab);

                if (!url.isValid()) {
                    url = rootUrl();
                }
            }
            DFMEventDispatcher::instance()->processEvent<DFMOpenNewTabEvent>(this, url);
            return;
        }
        default: break;
        }

        break;
    case Qt::ShiftModifier:
        if (event->key() == Qt::Key_T) {
            appController->actionOpenInTerminal(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));

            return;
        }

        break;
    case Qt::AltModifier:
    case Qt::AltModifier | Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Home:
            urls.clear();
            urls << DUrl::fromLocalFile(QDir::homePath());
            // fall through
        case Qt::Key_Down:
            appController->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));
            return;
        }
        break;

    default: break;
    }

    QFrame::keyPressEvent(event);
}

void ComputerView::mouseMoveEvent(QMouseEvent *event)
{
    // 防止在使用触屏滚动视图的同时导致窗口移动
    event->accept();

    return;
}

void ComputerView::setIconSizeBySizeIndex(int index)
{
    QSignalBlocker blocker(m_statusBar->scalingSlider());
    Q_UNUSED(blocker)
    m_statusBar->scalingSlider()->setValue(index);
}

DScrollArea::DScrollArea(QWidget *parent):
    QScrollArea(parent)
{

}

void DScrollArea::wheelEvent(QWheelEvent *event)
{
    if (DFMGlobal::keyCtrlIsPressed()) {
        return;
    }
    QScrollArea::wheelEvent(event);
}
