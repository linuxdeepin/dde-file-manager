// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabbar.h"
#include "tab.h"
#include "tabclosebutton.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"
#include "utils/filedatamanager.h"

#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/event/event.h>

#include <QUrl>
#include <QEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsSceneMouseEvent>

#include <unistd.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

TabBar::TabBar(QWidget *parent)
    : QGraphicsView(parent)
{
    setObjectName("TabBar");
    initializeUI();
}

TabBar::~TabBar()
{
    for (int index = tabList.count() - 1; index >= 0; --index) {
        removeTab(index);
    }
}

int TabBar::createTab()
{
    Tab *tab = new Tab();
    tabList.append(tab);
    scene->addItem(tab);

    quint64 thisWinID = WorkspaceHelper::instance()->windowId(qobject_cast<QWidget *>(parent()));
    WorkspaceEventCaller::sendTabAdded(thisWinID);

    if (isHidden() && count() >= 2) {
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

    lastAddTabState = true;
    setCurrentIndex(index);
    lastAddTabState = false;

    tabAddableChanged(count() < kMaxTabCount);

    return index;
}

void TabBar::removeTab(const int index, const bool &remainState)
{
    Tab *tab = tabList.at(index);

    tabList.removeAt(index);
    tab->deleteLater();

    quint64 thisWinID = WorkspaceHelper::instance()->windowId(qobject_cast<QWidget *>(parent()));
    WorkspaceEventCaller::sendTabRemoved(thisWinID, index);

    if (tabCloseButton->getClosingIndex() <= count() - 1
        && tabCloseButton->getClosingIndex() >= 0) {
        lastDeleteState = remainState;
    } else {
        lastAddTabState = false;
        // handle tab close button display position
        if (remainState) {
            QMouseEvent *event = new QMouseEvent(QMouseEvent::MouseMove,
                                                 mapFromGlobal(QCursor::pos()),
                                                 Qt::NoButton,
                                                 Qt::NoButton,
                                                 Qt::NoModifier);
            mouseMoveEvent(event);
        }
    }

    if (index < count())
        setCurrentIndex(index);
    else
        setCurrentIndex(count() - 1);
    emit tabAddableChanged(count() < kMaxTabCount);

    if (count() < 2) {
        lastDeleteState = false;
        hide();
        emit tabBarHidden();
    }
}

int TabBar::getCurrentIndex() const
{
    return currentIndex;
}

bool TabBar::tabAddable() const
{
    return count() < kMaxTabCount;
}

Tab *TabBar::currentTab() const
{
    if (currentIndex >= 0 && currentIndex < count())
        return tabList.at(currentIndex);
    return nullptr;
}

Tab *TabBar::tabAt(const int &index)
{
    return tabList.at(index);
}

void TabBar::setCurrentIndex(const int index)
{
    if (index < 0 || index >= tabList.count())
        return;

    currentIndex = index;

    int counter = 0;
    for (auto tab : tabList) {
        if (counter == index) {
            tab->setChecked(true);
        } else {
            tab->setChecked(false);
        }
        counter++;
    }

    emit currentChanged(index);
    updateScreen();
}

void TabBar::setCurrentUrl(const QUrl &url)
{
    Tab *tab = currentTab();
    if (!tab)
        createTab();

    tab->setCurrentUrl(url);
}

void TabBar::closeTab(quint64 winId, const QUrl &url)
{
    for (int i = count() - 1; i >= 0; --i) {
        Tab *tab = tabAt(i);
        if (!tab)
            continue;

        QUrl curUrl = tab->getCurrentUrl();
        // Some URLs cannot be compared universally
        bool closeable { dpfHookSequence->run("dfmplugin_workspace", "hook_Tab_Closeable",
                                              curUrl, url) };

        static const QUrl &kGotoWhenDevRemoved = QUrl("computer:///");
        if (closeable || DFMBASE_NAMESPACE::UniversalUtils::urlEquals(curUrl, url) || url.isParentOf(curUrl)) {
            if (count() == 1) {
                QUrl redirectToWhenDelete;
                if (isMountedDevPath(url) || url.scheme() != Global::Scheme::kFile) {
                    redirectToWhenDelete = kGotoWhenDevRemoved;
                } else {   // redirect to upper directory
                    QString localPath = url.path();
                    do {
                        QStringList pathFragment = localPath.split("/");
                        pathFragment.removeLast();
                        localPath = pathFragment.join("/");
                    } while (!QDir(localPath).exists());
                    redirectToWhenDelete.setScheme(Global::Scheme::kFile);
                    redirectToWhenDelete.setPath(localPath);

                    {
                        /* NOTE(xust)
                         * BUG-236625
                         * this is a workaround.
                         * when android phone mounted with MTP protocol, cd into the internal storage.
                         * eject the phone by sidebar button or dock widget
                         * the 'fileRemoved' signal with the internal storage path is emitted first
                         * and then runs into current 'else' branch, and try to find it's parent path to cd to
                         * and the gvfs mount root path was found
                         * while cd to 'computer:///' is expected.
                         * so if final cd path is gvfs root, then change it to computer root to solve this bug.
                         *
                         * but this solution would introduce another lower level bug.
                         * */
                        static const QStringList &kGvfsMpts {
                            QString("/run/user/%1/gvfs").arg(getuid()),
                            "/root/.gvfs"
                        };
                        if (kGvfsMpts.contains(localPath))
                            redirectToWhenDelete = kGotoWhenDevRemoved;
                    }
                }

                dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, redirectToWhenDelete);
            } else {
                removeTab(i);
            }
        }
    }
}

void TabBar::onTabCloseButtonHovered(int closingIndex)
{
    if (closingIndex < 0 || closingIndex >= count())
        return;
    Tab *tab = tabList.at(closingIndex);
    if (!tab)
        return;
    tab->setHovered(true);
    tab->update();
}

void TabBar::onTabCloseButtonUnHovered(int closingIndex)
{
    if (closingIndex < 0 || closingIndex >= count())
        return;
    Tab *tab = tabList.at(closingIndex);
    tab->setHovered(false);
    tab->update();
}

void TabBar::onTabCloseButtonClicked()
{
    int closingIndex = tabCloseButton->getClosingIndex();

    // effect handler
    if (closingIndex == count() - 1) {
        historyWidth = count() * tabList.at(0)->width();
    } else {
        historyWidth = (count() - 1) * tabList.at(0)->width();
    }
    emit tabCloseRequested(closingIndex, true);

    // redirect tab close button's closingIndex
    if (closingIndex >= count())
        tabCloseButton->setClosingIndex(--closingIndex);
}

void TabBar::onMoveNext(Tab *tab)
{
    int tabIndex = tabList.indexOf(tab);
    if (tabIndex >= count() - 1)
        return;

    tabList.swapItemsAt(tabIndex, tabIndex + 1);

    ++tabIndex;
    quint64 thisWinID = WorkspaceHelper::instance()->windowId(qobject_cast<QWidget *>(parent()));
    WorkspaceEventCaller::sendTabMoved(thisWinID, tabIndex - 1, tabIndex);
    emit tabMoved(tabIndex - 1, tabIndex);

    setCurrentIndex(tabIndex);
}

void TabBar::onMovePrevius(Tab *tab)
{
    int tabIndex = tabList.indexOf(tab);
    if (tabIndex <= 0)
        return;

    tabList.swapItemsAt(tabIndex, tabIndex - 1);

    --tabIndex;
    quint64 thisWinID = WorkspaceHelper::instance()->windowId(qobject_cast<QWidget *>(parent()));
    WorkspaceEventCaller::sendTabMoved(thisWinID, tabIndex + 1, tabIndex);
    emit tabMoved(tabIndex + 1, tabIndex);

    setCurrentIndex(tabIndex);
}

void TabBar::onRequestNewWindow(const QUrl url)
{
    WorkspaceEventCaller::sendOpenWindow(QList<QUrl>() << url);
}

void TabBar::onAboutToNewWindow(Tab *tab)
{
    emit tabCloseRequested(tabList.indexOf(tab));
}

void TabBar::onTabClicked()
{
    Tab *tab = qobject_cast<Tab *>(sender());
    if (!tab)
        return;
    setCurrentIndex(tabList.indexOf(tab));
    tabCloseButton->setActiveWidthTab(true);
}

void TabBar::onTabDragFinished()
{
    Tab *tab = qobject_cast<Tab *>(sender());
    if (!tab)
        return;
    tabCloseButton->setZValue(2);
    if (tab->isDragOutSide())
        tabCloseButton->hide();
    lastDeleteState = false;
    updateScreen();

    //hide border left line
    for (auto it : tabList)
        if (it->borderLeft())
            it->setBorderLeft(false);
}

void TabBar::onTabDragStarted()
{
    tabCloseButton->setZValue(0);

    Tab *tab = qobject_cast<Tab *>(sender());
    if (!tab)
        return;

    int pairIndex = tabList.indexOf(tab) + 1;
    int counter = 0;
    for (auto it : tabList) {
        if (counter == tabList.indexOf(tab) || counter == pairIndex)
            it->setBorderLeft(true);
        else
            it->setBorderLeft(false);
        counter++;
    }
}

void TabBar::activateNextTab()
{
    if (currentIndex == count() - 1)
        setCurrentIndex(0);
    else
        setCurrentIndex(currentIndex + 1);
}

void TabBar::activatePreviousTab()
{
    if (currentIndex == 0)
        setCurrentIndex(count() - 1);
    else
        setCurrentIndex(currentIndex - 1);
}

void TabBar::closeTabAndRemoveCachedMnts(const QString &id)
{
    if (!allMntedDevs.contains(id))
        return;
    for (const auto &url : allMntedDevs.values(id)) {
        this->closeTab(WorkspaceHelper::instance()->windowId(this), url);
        FileDataManager::instance()->cleanRoot(url);
        emit InfoCacheController::instance().removeCacheFileInfo({ url });
        WatcherCache::instance().removeCacheWatcherByParent(url);
    }
    allMntedDevs.remove(id);
}

void TabBar::cacheMnt(const QString &id, const QString &mnt)
{
    if (!mnt.isEmpty())
        allMntedDevs.insert(id, QUrl::fromLocalFile(mnt));
}

void TabBar::resizeEvent(QResizeEvent *event)
{
    scene->setSceneRect(0, 0, width(), height());
    historyWidth = width();
    updateScreen();
    QGraphicsView::resizeEvent(event);
}

bool TabBar::event(QEvent *event)
{
    if (event->type() == event->Leave) {
        tabCloseButton->hide();
        lastDeleteState = false;
        historyWidth = width();
        updateScreen();
    }
    return QGraphicsView::event(event);
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!tabCloseButton->isVisible())
        tabCloseButton->show();

    int closingIndex = -1;
    for (int i = 0; i < tabList.count(); i++) {
        Tab *tab = tabList.at(i);
        if (tab->sceneBoundingRect().contains(event->pos())) {
            closingIndex = i;
        } else {
            tab->setHovered(false);
            tab->update();
        }
    }

    if (closingIndex < count() && closingIndex >= 0) {
        Tab *tab = tabList.at(closingIndex);
        tabCloseButton->setClosingIndex(closingIndex);
        int btnSize = height() > 24 ? 36 : 32;
        tabCloseButton->setSize(btnSize);
        tabCloseButton->setPos(tab->x() + tab->width() - btnSize - 4, btnSize - 32);

        if (closingIndex == currentIndex)
            tabCloseButton->setActiveWidthTab(true);
        else
            tabCloseButton->setActiveWidthTab(false);

    } else {
        if (lastDeleteState) {
            lastDeleteState = false;
            updateScreen();
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

bool TabBar::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent *>(event);
        if (e->button() == Qt::MiddleButton) {
            if (QGraphicsItem *item = itemAt(mapFromGlobal(QCursor::pos()))) {
                Tab *t = static_cast<Tab *>(item);
                if (tabList.contains(t))
                    emit tabCloseRequested(tabList.indexOf(t));
            } else {
                fmDebug("You didn't click on an item.");
            }
        }
    }
    return QGraphicsView::eventFilter(obj, event);
}

void TabBar::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        activateNextTab();
    } else {
        activatePreviousTab();
    }

    QGraphicsView::wheelEvent(event);
}

void TabBar::initializeUI()
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, width(), height());
    setContentsMargins(0, 0, 0, 0);
    setScene(scene);
    scene->installEventFilter(this);

    tabCloseButton = new TabCloseButton;
    tabCloseButton->setZValue(4);
    tabCloseButton->hide();
    scene->addItem(tabCloseButton);

    setMouseTracking(true);
    setFrameShape(QFrame::NoFrame);

    initializeConnections();
    hide();
}

void TabBar::initializeConnections()
{
    QObject::connect(tabCloseButton, &TabCloseButton::hovered, this, &TabBar::onTabCloseButtonHovered);
    QObject::connect(tabCloseButton, &TabCloseButton::unHovered, this, &TabBar::onTabCloseButtonUnHovered);
    QObject::connect(tabCloseButton, &TabCloseButton::clicked, this, &TabBar::onTabCloseButtonClicked);

    QObject::connect(DevProxyMng, &DeviceProxyManager::blockDevMounted, this, &TabBar::cacheMnt);
    QObject::connect(DevProxyMng, &DeviceProxyManager::protocolDevMounted, this, &TabBar::cacheMnt);
    QObject::connect(DevProxyMng, &DeviceProxyManager::blockDevAdded, this, [this](const QString &id) { cacheMnt(id, ""); });
    QObject::connect(DevProxyMng, &DeviceProxyManager::blockDevUnmounted, this, &TabBar::closeTabAndRemoveCachedMnts);
    QObject::connect(DevProxyMng, &DeviceProxyManager::blockDevRemoved, this, &TabBar::closeTabAndRemoveCachedMnts);
    QObject::connect(DevProxyMng, &DeviceProxyManager::protocolDevUnmounted, this, &TabBar::closeTabAndRemoveCachedMnts);

    for (auto id : DevProxyMng->getAllBlockIds()) {
        auto datas = DevProxyMng->queryBlockInfo(id);
        const QString &&mntPath = datas.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString();
        cacheMnt(id, mntPath);
    }
    for (auto id : DevProxyMng->getAllProtocolIds()) {
        auto datas = DevProxyMng->queryProtocolInfo(id);
        const QString &&mntPath = datas.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString();
        if (!mntPath.isEmpty())
            allMntedDevs.insert(id, QUrl::fromLocalFile(mntPath));
    }
}

void TabBar::updateScreen()
{
    int counter = 0;
    int lastX = 0;
    historyWidth = width();
    for (Tab *tab : tabList) {
        QSize tabSize = tabSizeHint(counter);
        QRect rect(lastX, 0, tabSize.width(), tabSize.height());
        lastX = rect.x() + rect.width();

        if (tab->isDragging()) {
            counter++;
            continue;
        }

        if (!lastAddTabState) {
            QPropertyAnimation *animation = new QPropertyAnimation(tab, "geometry");
            animation->setDuration(100);
            animation->setStartValue(tab->geometry());
            animation->setEndValue(rect);
            animation->start(QAbstractAnimation::DeleteWhenStopped);

            connect(animation, &QPropertyAnimation::finished, tab, [=] {
                handleTabAnimationFinished(counter);
            });
        } else {
            tab->setGeometry(rect);
        }

        counter++;
    }

    updateSceneRect(scene->sceneRect());
}

QSize TabBar::tabSizeHint(const int &index)
{
    if (lastDeleteState)
        return QSize(tabList.at(0)->width(), tabList.at(0)->height());

    int averageWidth = historyWidth / count();

    if (index == count() - 1)
        return (QSize(historyWidth - averageWidth * (count() - 1), height()));
    else
        return (QSize(averageWidth, height()));
}

int TabBar::count() const
{
    return tabList.count();
}

void TabBar::handleTabAnimationFinished(const int index)
{
    if (tabCloseButton->getClosingIndex() == index) {
        Tab *tab = tabList.at(index);
        int btnSize = height() > 24 ? 36 : 32;
        tabCloseButton->setSize(btnSize);
        tabCloseButton->setPos(tab->x() + tab->width() - btnSize - 4, btnSize - 32);
    }

    if ((tabCloseButton->getClosingIndex() >= count()
         || tabCloseButton->getClosingIndex() < 0)
        && lastDeleteState) {
        lastDeleteState = false;
    }
}

bool TabBar::isMountedDevPath(const QUrl &url)
{
    for (auto iter = allMntedDevs.cbegin(); iter != allMntedDevs.cend(); ++iter) {
        auto urls = allMntedDevs.values(iter.key());
        auto ret = std::find_if(urls.cbegin(), urls.cend(), [url](const QUrl &u) { return DFMBASE_NAMESPACE::UniversalUtils::urlEquals(u, url); });
        if (ret != urls.cend())
            return true;
    }
    return false;
}
