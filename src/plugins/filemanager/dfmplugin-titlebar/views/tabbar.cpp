// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabbar.h"
#include "tab.h"
#include "titlebarwidget.h"
#include "dfmplugin_titlebar_global.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"

#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/widgets/dfmcustombuttons/customiconbutton.h>

#include <dfm-framework/event/event.h>

#include <DIconButton>

#include <QDir>
#include <QUrl>
#include <QEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>

#include <unistd.h>

inline constexpr int kTabHeightScaling { 24 };
inline constexpr int kCloseButtonBigSize { 36 };
inline constexpr int kCloseButtonSmallSize { 30 };
inline constexpr int kTabMaxWidth { 240 };

DFMBASE_USE_NAMESPACE
DPTITLEBAR_USE_NAMESPACE

TabBar::TabBar(QWidget *parent)
    : QGraphicsView(parent)
{
    setObjectName("TabBar");
    initializeUI();
    initializeConnections();
}

TabBar::~TabBar()
{
}

int TabBar::createTab()
{
    Tab *tab = new Tab();
    tabList.append(tab);
    scene->addItem(tab);

    QString prefix = "tab_";
    QString uniqueId = prefix + QString::number(++nextTabUniqueId);
    tab->setUniqueId(uniqueId);

    Q_EMIT newTabCreated(uniqueId);

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
    connect(tab, &Tab::closeRequested, this, &TabBar::onTabCloseButtonClicked);

    playTabAnimation = false;
    setCurrentIndex(index);
    playTabAnimation = true;

    updateAddTabButtonState();
    updateTabsState();
    return index;
}

void TabBar::removeTab(const int index, const bool &remainState)
{
    int newIndex;
    if (currentIndex < index) {
        // Current tab is before the deleted tab, keep current index unchanged
        newIndex = currentIndex;
    } else if (currentIndex == index) {
        // Delete current tab, select next tab (if last tab then select previous)
        newIndex = (index == count() - 1) ? qMax(index - 1, 0) : index;
    } else {
        // Current tab is after deleted tab, decrease index by 1
        newIndex = currentIndex - 1;
    }

    Q_EMIT tabRemoved(index, newIndex);

    Tab *tab = tabList.at(index);
    tabList.removeAt(index);
    tab->deleteLater();

    playTabAnimation = true;
    setCurrentIndex(newIndex);

    updateAddTabButtonState();
    updateTabsState();
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
    if (index < 0 || index >= tabList.count()) {
        fmWarning() << "Cannot set current index - invalid index:" << index << "tab count:" << count();
        return;
    }

    int oldIndex = currentIndex;
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

    emit currentChanged(oldIndex, index);
    updateScreen();
}

void TabBar::setCurrentUrl(const QUrl &url)
{
    Tab *tab = currentTab();
    if (tab == nullptr)
        createTab();

    tab = currentTab();
    if (tab)
        tab->setCurrentUrl(url);
}

void TabBar::closeTab(const QUrl &url)
{
    for (int i = count() - 1; i >= 0; --i) {
        Tab *tab = tabAt(i);
        if (!tab)
            continue;

        if (!shouldCloseTab(tab, url))
            continue;

        if (count() == 1) {
            handleLastTabClose(tab->getCurrentUrl(), url);
        } else {
            removeTab(i);
        }
    }
}

void TabBar::onTabCloseButtonClicked()
{
    auto tab = dynamic_cast<Tab *>(sender());
    if (!tab) {
        fmWarning() << "Tab close button clicked but sender is not a Tab";
        return;
    }

    int closingIndex = tabList.indexOf(tab);

    // effect handler
    if (closingIndex == count() - 1) {
        historyWidth = count() * tabList.at(0)->width();
    } else {
        historyWidth = (count() - 1) * tabList.at(0)->width();
    }
    emit tabCloseRequested(closingIndex, true);
}

void TabBar::onMoveNext(Tab *tab)
{
    int tabIndex = tabList.indexOf(tab);
    if (tabIndex >= count() - 1)
        return;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    tabList.swap(tabIndex, tabIndex + 1);
#else
    tabList.swapItemsAt(tabIndex, tabIndex + 1);
#endif

    ++tabIndex;
    emit tabMoved(tabIndex - 1, tabIndex);

    setCurrentIndex(tabIndex);
}

void TabBar::onMovePrevius(Tab *tab)
{
    int tabIndex = tabList.indexOf(tab);
    if (tabIndex <= 0)
        return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    tabList.swap(tabIndex, tabIndex - 1);
#else
    tabList.swapItemsAt(tabIndex, tabIndex - 1);
#endif
    --tabIndex;
    emit tabMoved(tabIndex + 1, tabIndex);

    setCurrentIndex(tabIndex);
}

void TabBar::onRequestNewWindow(const QUrl url)
{
    TitleBarEventCaller::sendOpenWindow(url);
}

void TabBar::onAboutToNewWindow(Tab *tab)
{
    emit tabCloseRequested(tabList.indexOf(tab));
}

void TabBar::onTabClicked()
{
    Tab *tab = qobject_cast<Tab *>(sender());
    if (!tab) {
        fmWarning() << "Tab clicked but sender is not a Tab";
        return;
    }
    setCurrentIndex(tabList.indexOf(tab));
}

void TabBar::onTabDragFinished()
{
    Tab *tab = qobject_cast<Tab *>(sender());
    if (!tab)
        return;

    updateScreen();

    // hide border left line
    for (auto it : tabList)
        if (it->borderLeft())
            it->setBorderLeft(false);
}

void TabBar::onTabDragStarted()
{
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

void TabBar::resizeEvent(QResizeEvent *event)
{
    scene->setSceneRect(0, 0, width(), height());
    historyWidth = getTabAreaWidth();
    updateScreen();
    QGraphicsView::resizeEvent(event);
}

bool TabBar::event(QEvent *event)
{
    if (event->type() == event->Leave) {
        historyWidth = getTabAreaWidth();
        updateScreen();
    }
    return QGraphicsView::event(event);
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    // 检查是否点击在标签上
    bool onTab = false;
    for (Tab *tab : tabList) {
        if (tab->sceneBoundingRect().contains(event->pos())) {
            onTab = true;
            break;
        }
    }

    if (!onTab) {
        // 如果不在标签上,开始窗口拖拽
        isDragging = true;
        event->ignore();   // 让事件继续传播给父窗口
    } else {
        if (event->button() == Qt::RightButton) {
            // don't show titlebar context menu
            return;
        }
        QGraphicsView::mousePressEvent(event);
    }
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging) {
        // 如果正在拖拽,让事件继续传播
        event->ignore();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }

    // 更新标签悬停状态
    for (Tab *tab : tabList) {
        bool hovered = tab->sceneBoundingRect().contains(event->pos());
        tab->setHovered(hovered);
        tab->update();
    }
}

void TabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (isDragging) {
        isDragging = false;
        event->ignore();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
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

    tabAddButton = new CustomDIconButton(DStyle::SP_IncreaseElement, this);
    tabAddButton->setObjectName("NewTabButton");
    tabAddButton->setFixedSize(kToolButtonSize, kToolButtonSize);
    tabAddButton->setFlat(true);
    scene->addWidget(tabAddButton);

#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<DIconButton *>(tabAddButton), AcName::kAcViewTabBarNewButton);
#endif

    setMouseTracking(true);
    setFrameShape(QFrame::NoFrame);
}

void TabBar::initializeConnections()
{
    connect(tabAddButton, &DIconButton::clicked, this, &TabBar::tabAddButtonClicked);
    connect(DevProxyMng, &DeviceProxyManager::mountPointAboutToRemoved, this, [this](QStringView mpt) {
        fmDebug() << "Mount point about to be removed:" << mpt.toString();
        closeTab(QUrl::fromLocalFile(mpt.toString()));
    });
}

void TabBar::updateScreen()
{
    int counter = 0;
    int lastX = 0;
    historyWidth = getTabAreaWidth();

    for (Tab *tab : tabList) {
        QSize tabSize = tabSizeHint(counter);
        QRect rect(lastX, 0, tabSize.width(), tabSize.height());
        lastX = rect.x() + rect.width();

        if (tab->isDragging()) {
            counter++;
            continue;
        }

        if (playTabAnimation) {
            QPropertyAnimation *animation = new QPropertyAnimation(tab, "geometry");
            animation->setDuration(50);
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

    // update tabAddButton position
    if (tabAddButton) {
        int btnX = count() > 1 ? lastX + 10 : 10;
        int btnY = (height() - tabAddButton->height()) / 2;
        QRect rect(btnX, btnY, tabAddButton->size().width(), tabAddButton->size().height());
        if (playTabAnimation) {
            QPropertyAnimation *animation = new QPropertyAnimation(tabAddButton, "geometry");
            animation->setDuration(50);
            animation->setStartValue(tabAddButton->geometry());
            animation->setEndValue(rect);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            tabAddButton->setGeometry(rect);
        }
    }

    updateSceneRect(scene->sceneRect());
}

QSize TabBar::tabSizeHint(const int &index)
{
    int averageWidth = qMin(kTabMaxWidth, historyWidth / count());

    if (index == count() - 1)
        return (QSize(qMin(kTabMaxWidth, historyWidth - averageWidth * (count() - 1)), height()));
    else
        return (QSize(averageWidth, height()));
}

int TabBar::count() const
{
    return tabList.count();
}

void TabBar::handleTabAnimationFinished(const int index)
{
    // Fix for tab hover issue: After tab removal animation completes,
    // check if mouse cursor is over any tab and update hover state accordingly.
    // This ensures that when a tab is closed and other tabs move to new positions,
    // the hover state is correctly updated for the tab now under the mouse cursor.
    for (Tab *tab : tabList) {
        auto rect = tab->geometry();
        auto pos = mapFromGlobal(QCursor::pos());
        if (rect.contains(pos)) {
            tab->setHovered(true);
            break;
        }
    }
}

void TabBar::updateTabsState()
{
    int tabCount = tabList.count();
    for (Tab *tab : tabList) {
        tab->setShowCloseButton(tabCount > 1);
        tab->setCanDrag(tabCount > 1);
        tab->setVisible(tabCount > 1);
    }
}

void TabBar::updateAddTabButtonState()
{
    tabAddButton->setEnabled(count() < kMaxTabCount);
}

bool TabBar::shouldCloseTab(Tab *tab, const QUrl &targetUrl) const
{
    QUrl currentUrl = tab->getCurrentUrl();

    // Check if tab can be closed through hook
    bool closeable = dpfHookSequence->run("dfmplugin_titlebar", "hook_Tab_Closeable",
                                          currentUrl, targetUrl);

    if (closeable
        || DFMBASE_NAMESPACE::UniversalUtils::urlEquals(currentUrl, targetUrl)
        || targetUrl.isParentOf(currentUrl)) {
        return true;
    }

    if (currentUrl.isLocalFile() && targetUrl.isLocalFile()) {
        // 软链接场景下，如果是CIFS下的符号链接, canonicalFilePath 无法解析
        QString realCurrentPath = SystemPathUtil::instance()->getRealpathSafely(currentUrl.toLocalFile());
        QString realTargetPath = SystemPathUtil::instance()->getRealpathSafely(targetUrl.toLocalFile());

        if (!realCurrentPath.isEmpty() && !realTargetPath.isEmpty()) {
            if (realCurrentPath == realTargetPath)
                return true;
            if (realCurrentPath.startsWith(realTargetPath))
                return true;
        }
    }

    return false;
}

void TabBar::handleLastTabClose(const QUrl &currentUrl, const QUrl &targetUrl)
{
    QUrl redirectUrl = determineRedirectUrl(currentUrl, targetUrl);
    auto winId = TitleBarHelper::windowId(this);
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, redirectUrl);
}

QUrl TabBar::determineRedirectUrl(const QUrl &currentUrl, const QUrl &targetUrl) const
{
    const QUrl &defaultUrl = Application::instance()->appAttribute(Application::kUrlOfNewWindow).toUrl();

    // BUG: 303643
    QString targetPath = targetUrl.toLocalFile();
    targetPath = SystemPathUtil::instance()->getRealpathSafely(targetPath);
    if (DevProxyMng->isFileOfExternalMounts(targetPath))
        return defaultUrl;

    QUrl redirectUrl;
    if (dpfHookSequence->run("dfmplugin_titlebar", "hook_Tab_FileDeleteNotCdComputer",
                             currentUrl, &redirectUrl)) {
        return redirectUrl.isValid() ? redirectUrl : defaultUrl;
    }

    if (targetUrl.isLocalFile()) {
        redirectUrl = findValidParentPath(targetUrl);
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
         * but this solution would introduce another lower level bug.
         * */
        static const QStringList &kGvfsMpts {
            QString("/run/user/%1/gvfs").arg(getuid()),
            "/root/.gvfs"
        };
        if (kGvfsMpts.contains(redirectUrl.toLocalFile()))
            return defaultUrl;

        return redirectUrl;
    }

    return defaultUrl;
}

QUrl TabBar::findValidParentPath(const QUrl &url) const
{
    QString localPath = url.path();

    do {
        QStringList pathFragment = localPath.split("/");
        pathFragment.removeLast();
        localPath = pathFragment.join("/");
    } while (!QDir(localPath).exists());

    QUrl parentUrl;
    parentUrl.setScheme(Global::Scheme::kFile);
    parentUrl.setPath(localPath);
    return parentUrl;
}
