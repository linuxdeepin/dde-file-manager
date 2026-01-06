// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabbar.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/event/event.h>
#include <dfm-framework/dpf.h>

#include <DIconButton>
#include <DWindowManagerHelper>
#include <DPaletteHelper>
#include <DPlatformWindowHandle>

#include <QMouseEvent>
#include <QBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCursor>
#include <QIcon>
#include <QUuid>
#include <QUrlQuery>
#include <QToolTip>

#include <unistd.h>
#include <functional>

inline constexpr int kTabMaxWidth { 240 };
inline constexpr int kTabMinWidth { 90 };
inline constexpr int kItemButtonSize { 20 };
inline constexpr int kItemButtonIconSize { 16 };
inline constexpr int kItemButtonMargin { 4 };

using namespace dfmplugin_titlebar;
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

struct Tab
{
    QUrl tabUrl;
    QString tabAlias;
    QString uniqueId;
    QString tabTipText;
    QVariantMap userData;
    bool isInactive { false };
    bool isPinned { false };
};

namespace dfmplugin_titlebar {
class TabBarPrivate : public QObject
{
public:
    explicit TabBarPrivate(TabBar *qq);

    void initUI();
    void initConnections();

    bool tabCloseable(const Tab &tab, const QUrl &targetUrl) const;
    void handleLastTabClose(const QUrl &currentUrl, const QUrl &targetUrl);
    bool isItemButtonHovered(int index);
    void handleTabReleased(int index);
    void handleDragActionChanged(Qt::DropAction action);
    void handleContextMenu(int index);
    void handleIndexChanged(int index);
    void updateToolTip(int index, bool visible);
    void handleTabDroped(int index, Qt::DropAction dropAction, QObject *target);
    void handlePinnedTabsChanged(const QString &config, const QString &key);
    void handleTabMoved(int from, int to);
    void updatePinnedTabsOrder();

    QString tabDisplayName(const QUrl &url) const;
    QUrl determineRedirectUrl(const QUrl &currentUrl, const QUrl &targetUrl) const;
    QUrl findValidParentPath(const QUrl &url) const;
    void paintTabBackground(QPainter *painter, const QStyleOptionTab &option);
    void paintTabLabel(QPainter *painter, int index, const QStyleOptionTab &option);
    void paintTabButton(DIconButton *btn);
    void paintTabItemButton(QPainter *painter, int index, const QStyleOptionTab &option);

    Tab tabInfo(int index) const;
    bool updateTabInfo(int index, std::function<void(Tab &)> modifier);
    bool canPinned(int index);
    void setTabPinned(int index, bool pinned, bool updateConfig = true);
    bool shouldCreateNewTabForPinnedTab(const QUrl &currentUrl, const QUrl &newUrl) const;
    QPixmap createColoredIcon(const QIcon &icon, const QColor &color, const QSize &size);
    QUrl getActualUrl(const QUrl &url) const;
    void closeLeftTabs(int index);
    void closeRightTabs(int index);
    void closeOtherTabs(int index);

public:
    TabBar *q;
    DIconButton *addBtn { nullptr };
    DIconButton *leftBtn { nullptr };
    QTabBar *tabBar { nullptr };

    int nextTabUniqueId { 0 };
    int currentTabIndex { -1 };
    bool isDragging { false };
    QTimer *updateConfigTimer { nullptr };
    int lastTooltipTabIndex { -1 };
    bool lastTooltipOnButton { false };
};
}

TabBarPrivate::TabBarPrivate(TabBar *qq)
    : q(qq)
{
}

void TabBarPrivate::initUI()
{
    q->setMovable(true);
    q->setTabsClosable(false);
    q->setVisibleAddButton(true);
    q->setDragable(true);
    q->setFocusPolicy(Qt::NoFocus);
    q->installEventFilter(q);

    const auto &pa = DPaletteHelper::instance()->palette(q);
    q->setMaskColor(pa.color(DPalette::ObviousBackground));

    // Initialize config update timer
    updateConfigTimer = new QTimer(q);
    updateConfigTimer->setSingleShot(true);
    updateConfigTimer->setInterval(500);

    addBtn = q->findChild<DIconButton *>("AddButton");
    Q_ASSERT(addBtn);

    addBtn->setFixedSize(30, 30);
    addBtn->setFlat(true);
    addBtn->setFocusPolicy(Qt::NoFocus);
    addBtn->installEventFilter(q);

    leftBtn = q->findChild<DIconButton *>("leftButton");
    Q_ASSERT(leftBtn);
    leftBtn->setFocusPolicy(Qt::NoFocus);
    leftBtn->setFixedSize(30, 30);
    leftBtn->setFlat(true);
    leftBtn->installEventFilter(q);

    auto rightBtn = q->findChild<DIconButton *>("rightButton");
    Q_ASSERT(rightBtn);
    rightBtn->setFocusPolicy(Qt::NoFocus);
    rightBtn->setFixedSize(30, 30);
    rightBtn->setFlat(true);
    rightBtn->installEventFilter(q);

    // for update close button state
    tabBar = q->findChild<QTabBar *>();
    if (tabBar) {
        tabBar->setMouseTracking(true);
        tabBar->installEventFilter(q);
    }
}

void TabBarPrivate::initConnections()
{
    connect(DevProxyMng, &DeviceProxyManager::mountPointAboutToRemoved, this, [this](QStringView mpt) {
        fmDebug() << "Mount point about to be removed:" << mpt.toString();
        q->closeTab(QUrl::fromLocalFile(mpt.toString()));
    });
    connect(q, &TabBar::currentChanged, this, &TabBarPrivate::handleIndexChanged);
    connect(q, &TabBar::tabReleaseRequested, this, &TabBarPrivate::handleTabReleased);
    connect(q, &TabBar::dragActionChanged, this, &TabBarPrivate::handleDragActionChanged);
    connect(q, &DTabBar::tabDroped, this, &TabBarPrivate::handleTabDroped);
    connect(q, &DTabBar::tabMoved, this, &TabBarPrivate::handleTabMoved);

    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, &TabBarPrivate::handlePinnedTabsChanged);
    connect(updateConfigTimer, &QTimer::timeout, this, &TabBarPrivate::updatePinnedTabsOrder);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [this] {
        const auto &pa = DPaletteHelper::instance()->palette(q);
        q->setMaskColor(pa.color(DPalette::ObviousBackground));
    });
}

bool TabBarPrivate::tabCloseable(const Tab &tab, const QUrl &targetUrl) const
{
    QUrl currentUrl = tab.tabUrl;

    // Check if tab can be closed through hook
    bool closeable = dpfHookSequence->run("dfmplugin_titlebar", "hook_Tab_Closeable", currentUrl, targetUrl);
    if (closeable
        || UniversalUtils::urlEquals(currentUrl, targetUrl)
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

            if (!realTargetPath.endsWith('/'))
                realTargetPath.append('/');
            if (realCurrentPath.startsWith(realTargetPath))
                return true;
        }
    }

    return false;
}

void TabBarPrivate::handleLastTabClose(const QUrl &currentUrl, const QUrl &targetUrl)
{
    QUrl redirectUrl = determineRedirectUrl(currentUrl, targetUrl);
    auto winId = TitleBarHelper::windowId(q);
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, redirectUrl);
}

bool TabBarPrivate::isItemButtonHovered(int index)
{
    auto rect = q->tabRect(index);
    const QRect btnRect(rect.right() - kItemButtonSize - kItemButtonMargin,
                        rect.y() + (rect.height() - kItemButtonSize) / 2,
                        kItemButtonSize, kItemButtonSize);
    auto pos = q->mapFromGlobal(QCursor::pos());
    return btnRect.contains(pos);
}

void TabBarPrivate::handleTabReleased(int index)
{
    if (index == -1)
        index = 0;

    fmInfo() << "Tab released at index:" << index;

    const auto &url = q->tabUrl(index);
    if (!url.isValid()) {
        fmWarning() << "Cannot release tab: invalid URL";
        return;
    }

    if (q->count() == 1) {
        q->window()->show();
    } else {
        currentTabIndex = -1;

        // Check if this is a pinned tab
        bool isPinnedTab = q->isPinned(index);
        QUrl publishUrl = url;

        if (isPinnedTab) {
            // Get pinnedId for pinned tab
            QString pinnedId = q->tabUserData(index, TabDef::kPinnedId).toString();
            if (!pinnedId.isEmpty()) {
                // Create special URL with pinned scheme
                publishUrl.setScheme(TabDef::kTabScheme);
                publishUrl.setPath("/");
                QUrlQuery query;
                query.addQueryItem(TabDef::kPinnedId, pinnedId);
                publishUrl.setQuery(query);
                fmInfo() << "Releasing pinned tab with pinnedId:" << pinnedId;
            } else {
                fmWarning() << "Pinned tab missing pinnedId, using regular URL";
            }
        }

        // Send open new window event
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, publishUrl);

        // Remove tab from current window
        q->tabCloseRequested(index);
        fmInfo() << "Tab released and new window created for URL:" << publishUrl;
    }
}

void TabBarPrivate::handleDragActionChanged(Qt::DropAction action)
{
    // Reset cursor to Qt::ArrowCursor if drag tab.
    if (action == Qt::IgnoreAction) {
        if (q->dragIconWindow()) {
            QGuiApplication::changeOverrideCursor(Qt::ArrowCursor);
            DPlatformWindowHandle::setDisableWindowOverrideCursor(q->dragIconWindow(), true);
        }
    } else if (q->dragIconWindow()) {
        DPlatformWindowHandle::setDisableWindowOverrideCursor(q->dragIconWindow(), false);
        if (QGuiApplication::overrideCursor())
            QGuiApplication::changeOverrideCursor(QGuiApplication::overrideCursor()->shape());
    }
}

void TabBarPrivate::handleContextMenu(int index)
{
    QMenu menu;
    bool isPinned = q->isPinned(index);
    if (isPinned) {
        menu.addAction(TabBar::tr("Unpin tab"), this, std::bind(&TabBarPrivate::setTabPinned, this, index, false, true));
    } else {
        auto act = menu.addAction(TabBar::tr("Pin tab"), this, std::bind(&TabBarPrivate::setTabPinned, this, index, true, true));
        act->setEnabled(canPinned(index));
    }
    menu.addSeparator();

    auto act = menu.addAction(TabBar::tr("Close tab"), this, [this, index] { Q_EMIT q->tabCloseRequested(index); });
    act->setEnabled(!isPinned);

    menu.addAction(TabBar::tr("Close other tabs"), this, std::bind(&TabBarPrivate::closeOtherTabs, this, index));
    act = menu.addAction(TabBar::tr("Close tabs to the left"), this, std::bind(&TabBarPrivate::closeLeftTabs, this, index));
    act->setEnabled(index > 0);

    act = menu.addAction(TabBar::tr("Close tabs to the right"), this, std::bind(&TabBarPrivate::closeRightTabs, this, index));
    act->setEnabled(index < q->count() - 1);

    menu.exec(QCursor::pos());
    // 防止标签处于hover状态
    QEvent event(QEvent::HoverLeave);
    QApplication::sendEvent(tabBar, &event);
}

void TabBarPrivate::handleIndexChanged(int index)
{
    if (currentTabIndex == index)
        return;

    const auto &tab = tabInfo(index);
    if (tab.isInactive) {
        Q_EMIT q->requestCreateView(tab.uniqueId);
        updateTabInfo(index, [](Tab &tab) {
            tab.isInactive = false;
        });
    }

    if (!isDragging)
        Q_EMIT q->currentTabChanged(currentTabIndex, index);
    currentTabIndex = index;
}

void TabBarPrivate::updateToolTip(int index, bool visible)
{
    // Hide tooltip if tab is not visible
    if (!visible) {
        if (lastTooltipTabIndex != -1) {
            QToolTip::hideText();
            lastTooltipTabIndex = -1;
            lastTooltipOnButton = false;
        }
        return;
    }

    bool isBtnHovered = isItemButtonHovered(index);
    const auto &tabTipText = tabInfo(index).tabTipText;

    // Update tooltip if: 1) hovering over a different tab, or 2) button hover state changed
    bool needUpdate = (lastTooltipTabIndex != index) || (lastTooltipOnButton != isBtnHovered) || !tabTipText.isEmpty();
    if (needUpdate) {
        QString tooltipText;
        if (isBtnHovered) {
            // Mouse on button - show button tooltip
            tooltipText = q->isPinned(index) ? TabBar::tr("Unpin tab") : TabBar::tr("Close tab");
        } else if (!tabTipText.isEmpty()) {
            // Mouse on tab but not on button - show tab tooltip
            tooltipText = tabTipText;
        }

        if (!tooltipText.isEmpty()) {
            QToolTip::showText(QCursor::pos(), tooltipText, q);
        } else {
            QToolTip::hideText();
        }
        lastTooltipTabIndex = index;
        lastTooltipOnButton = isBtnHovered;
    }
}

void TabBarPrivate::handleTabDroped(int index, Qt::DropAction dropAction, QObject *target)
{
    Q_UNUSED(dropAction)

    fmInfo() << "Tab dropped at index:" << index << "target:" << target;
    TabBar *targetTabBar = qobject_cast<TabBar *>(target);

    if (!targetTabBar) {
        // Dropped to empty area - create new window
        fmInfo() << "Tab dropped to empty area, creating new window";
        handleTabReleased(index);
    } else {
        currentTabIndex = -1;
        // Dropped to another window's TabBar
        fmInfo() << "Tab dropped to another window's TabBar";
        // Source window's tab has been created in target window via insertTabFromMimeData
        // Just close the tab in source window
        q->tabCloseRequested(index);
    }
}

void TabBarPrivate::handlePinnedTabsChanged(const QString &config, const QString &key)
{
    // Only handle pinned tabs configuration changes
    if (config != kViewDConfName || key != kPinnedTabs)
        return;

    // Get current pinned tabs from DConfig
    const auto &pinnedTabs = DConfigManager::instance()->value(kViewDConfName, kPinnedTabs, QVariantList()).toList();

    // Build a set of pinnedIds from DConfig
    QSet<QString> configPinnedIds;
    for (const auto &item : pinnedTabs) {
        auto tabData = item.toMap();
        QString pinnedId = tabData[TabDef::kPinnedId].toString();
        if (!pinnedId.isEmpty())
            configPinnedIds.insert(pinnedId);
    }

    // Check all tabs in this window and unpin those removed from config
    for (int i = 0; i < q->count(); ++i) {
        if (!q->isPinned(i))
            continue;

        QString pinnedId = q->tabUserData(i, TabDef::kPinnedId).toString();
        if (pinnedId.isEmpty())
            continue;

        // If this tab's pinnedId is not in the config anymore, unpin it
        if (!configPinnedIds.contains(pinnedId)) {
            fmInfo() << "Unpinning tab at index" << i << "due to config sync, pinnedId:" << pinnedId;
            setTabPinned(i, false, false);
        }
    }
}

void TabBarPrivate::handleTabMoved(int from, int to)
{
    // Only update config if the moved tab is a pinned tab
    if (!q->isPinned(to))
        return;

    // Start/restart timer to delay config update (avoid frequent updates during drag)
    updateConfigTimer->start();
}

void TabBarPrivate::updatePinnedTabsOrder()
{
    // Rebuild pinned tabs list in current tab order
    QVariantList pinnedTabs;
    for (int i = 0; i < q->count(); ++i) {
        if (!q->isPinned(i))
            continue;

        auto tab = tabInfo(i);
        QString pinnedId = tab.userData.value(TabDef::kPinnedId).toString();
        if (pinnedId.isEmpty()) {
            fmWarning() << "Pinned tab at index" << i << "missing pinnedId, skipping";
            continue;
        }

        QVariantMap pinnedData;
        pinnedData[TabDef::kPinnedId] = pinnedId;
        pinnedData[TabDef::kTabUrl] = getActualUrl(tab.tabUrl).toString();
        pinnedTabs.append(pinnedData);
    }

    // Update DConfig without triggering valueChanged signal
    DConfigManager::instance()->setValue(kViewDConfName, kPinnedTabs, pinnedTabs);
}

QString TabBarPrivate::tabDisplayName(const QUrl &url) const
{
    if (UrlRoute::isRootUrl(url))
        return UrlRoute::rootDisplayName(url.scheme());

    if (SystemPathUtil::instance()->isSystemPath(url.path()))
        return SystemPathUtil::instance()->systemPathDisplayNameByPath(url.path());

    auto info = InfoFactory::create<FileInfo>(url);
    if (info) {
        QString name = info->displayOf(DisPlayInfoType::kFileDisplayName);
        if (!name.isEmpty())
            return name;

        name = info->nameOf(DFMBASE_NAMESPACE::FileInfo::FileNameInfoType::kFileName);
        if (!name.isEmpty())
            return name;
    }

    return url.fileName();
}

QUrl TabBarPrivate::determineRedirectUrl(const QUrl &currentUrl, const QUrl &targetUrl) const
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

QUrl TabBarPrivate::findValidParentPath(const QUrl &url) const
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

void TabBarPrivate::paintTabBackground(QPainter *painter, const QStyleOptionTab &option)
{
    painter->save();

    const QRect rect = option.rect;
    const bool isSelected = option.state & QStyle::State_Selected;
    const bool isHovered = option.state & QStyle::State_MouseOver;

    // 获取主题颜色
    DPalette pal = DPaletteHelper::instance()->palette(q);
    const bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;

    // 绘制背景
    if (isHovered) {
        QColor bgColor = isDarkTheme ? QColor(255, 255, 255, qRound(255 * 0.05)) : QColor(0, 0, 0, qRound(255 * 0.05));
        painter->setBrush(bgColor);
    } else {
        painter->setBrush(pal.color(QPalette::Active, QPalette::Base));
    }

    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRect(rect);

    // 绘制边框
    QColor borderColor = isDarkTheme ? QColor(255, 255, 255, 20) : QColor(0, 0, 0, 20);
    painter->setPen(borderColor);
    painter->setBrush(Qt::NoBrush);
    if (isSelected) {
        QPainterPath path;
        path.moveTo(rect.bottomLeft());
        path.lineTo(rect.topLeft());
        path.lineTo(rect.topRight());
        path.lineTo(rect.bottomRight());
        painter->drawPath(path);
    } else {
        painter->drawRect(rect);
    }
    // 对中间的tabbar尾后加一根明显的线
    if (QStyleOptionTab::End != option.position && QStyleOptionTab::OnlyOneTab != option.position) {
        painter->drawLine(rect.topRight(), rect.bottomRight());
    }

    painter->restore();
}

void TabBarPrivate::paintTabLabel(QPainter *painter, int index, const QStyleOptionTab &option)
{
    painter->save();
    const QRect rect = option.rect;
    const bool isSelected = option.state & QStyle::State_Selected;
    const bool isItemBtnShowed = (option.state & QStyle::State_MouseOver) || q->isPinned(index);

    // 获取主题颜色
    DPalette pal = DPaletteHelper::instance()->palette(q);

    // 计算布局参数
    const int tabMargin = 10;
    const int blueMarkerWidth = isSelected ? 6 : 0;
    const int blueMarkerMargin = isSelected ? 4 : 0;
    const int btnSize = isItemBtnShowed ? kItemButtonSize : 0;
    const int btnMargin = isItemBtnShowed ? kItemButtonMargin : 0;

    // 计算文本可用宽度（考虑蓝色标记和关闭按钮）
    const int textMargin = blueMarkerWidth + blueMarkerMargin;
    const int leftSpace = tabMargin / 2 + textMargin;
    const int rightSpace = isItemBtnShowed ? (btnSize + btnMargin) : 0;
    const int availableTextWidth = rect.width() - leftSpace;

    // 截断文本以适应可用宽度
    QString elidedText = option.fontMetrics.elidedText(option.text, Qt::ElideRight, availableTextWidth);

    // 计算文本居中位置
    const int textWidth = option.fontMetrics.horizontalAdvance(elidedText);
    const int textX = rect.x() + (rect.width() - option.fontMetrics.horizontalAdvance(elidedText) - textMargin) / 2 + textMargin;
    const int textY = rect.y() + (rect.height() - option.fontMetrics.height()) / 2 + option.fontMetrics.ascent();

    if (isItemBtnShowed) {
        int textEndX = textX + textWidth;
        int buttonStartX = rect.right() - rightSpace;
        if (textEndX > buttonStartX) {
            // 重新计算文本宽度，确保不重叠
            elidedText = option.fontMetrics.elidedText(option.text, Qt::ElideRight, buttonStartX - textX);
        }
    }

    // 设置tooltip，只有省略时才显示
    const auto &tooltip = tabInfo(index).tabTipText;
    if (option.text != elidedText && tooltip != option.text) {
        updateTabInfo(index, [option](Tab &tab) {
            tab.tabTipText = option.text;
        });
    } else if (option.text == elidedText && !tooltip.isEmpty()) {
        updateTabInfo(index, [](Tab &tab) {
            tab.tabTipText.clear();
        });
    }

    // 绘制蓝色标记
    if (isSelected) {
        painter->save();
        QColor blueColor = pal.color(QPalette::Active, QPalette::Highlight);
        painter->setPen(Qt::NoPen);
        painter->setBrush(blueColor);

        const int markerY = rect.y() + (rect.height() - blueMarkerWidth) / 2;
        painter->drawRoundedRect(QRect(textX - textMargin, markerY,
                                       blueMarkerWidth, blueMarkerWidth),
                                 1, 1);
        painter->restore();
    }

    // 绘制文本
    QColor textColor = isSelected ? pal.color(QPalette::Active, QPalette::Text) : pal.color(QPalette::Inactive, QPalette::Text);
    painter->setPen(textColor);
    painter->drawText(textX, textY, elidedText);
    painter->restore();
}

void TabBarPrivate::paintTabButton(DIconButton *btn)
{
    if (btn->isEnabled() && !btn->isChecked()) {
        QPainter painter(btn);
        painter.setRenderHint(QPainter::Antialiasing);
        bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
        DStyleHelper dstyle(btn->style());
        const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);

        QColor bgColor;
        if (btn->isDown()) {
            // 按下状态 - 20%不透明度
            bgColor = isDarkTheme ? QColor(255, 255, 255, 51)
                                  : QColor(0, 0, 0, 51);
        } else if (btn->underMouse()) {
            // 悬浮状态 - 10%不透明度
            bgColor = isDarkTheme ? QColor(255, 255, 255, 26)
                                  : QColor(0, 0, 0, 26);
        }

        if (bgColor.isValid()) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(bgColor);
            painter.drawRoundedRect(btn->rect(), radius, radius);
        }
    }
}

void TabBarPrivate::paintTabItemButton(QPainter *painter, int index, const QStyleOptionTab &option)
{
    QIcon btnIcon;
    if (q->isPinned(index)) {
        btnIcon = QIcon::fromTheme("dfm_pin");
    } else if (option.state & QStyle::State_MouseOver) {
        btnIcon = QIcon::fromTheme("dfm_close");
    }

    if (!btnIcon.isNull()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        const QRect rect = option.rect;
        const QRect btnRect(rect.right() - kItemButtonMargin - kItemButtonSize,
                            rect.y() + (rect.height() - kItemButtonSize) / 2,
                            kItemButtonSize, kItemButtonSize);

        // 检测鼠标是否在按钮上
        const bool btnHovered = isItemButtonHovered(index);
        // 在 hover 时绘制圆形背景
        if (btnHovered) {
            bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
            QColor bgColor = isDarkTheme ? QColor(255, 255, 255, 26) : QColor(0, 0, 0, 26);
            painter->setPen(Qt::NoPen);
            painter->setBrush(bgColor);
            painter->drawEllipse(btnRect);
        }

        // 绘制图标
        DPalette pal = DPaletteHelper::instance()->palette(q);
        QColor iconColor = pal.color(QPalette::Active, QPalette::Text);
        QPixmap iconPixmap = createColoredIcon(btnIcon, iconColor, QSize(kItemButtonIconSize, kItemButtonIconSize));

        // 将图标居中绘制在背景中
        const int iconOffset = (kItemButtonSize - kItemButtonIconSize) / 2;
        QRect iconRect(btnRect.x() + iconOffset, btnRect.y() + iconOffset,
                       kItemButtonIconSize, kItemButtonIconSize);
        painter->setOpacity(0.7);
        painter->drawPixmap(iconRect, iconPixmap);
        painter->restore();
    }
}

Tab TabBarPrivate::tabInfo(int index) const
{
    Tab tab;
    if (index < 0 || index >= q->count())
        return tab;

    auto data = q->tabData(index);
    if (!data.isValid())
        return tab;

    return data.value<Tab>();
}

bool TabBarPrivate::updateTabInfo(int index, std::function<void(Tab &)> modifier)
{
    if (index < 0 || index >= q->count())
        return false;

    auto data = q->tabData(index);
    if (!data.isValid())
        return false;

    auto tab = data.value<Tab>();
    modifier(tab);
    q->setTabData(index, QVariant::fromValue(tab));
    return true;
}

bool TabBarPrivate::canPinned(int index)
{
    if (qAppName() != "dde-file-manager")
        return false;

    const auto &url = q->tabUrl(index);
    if (!url.isValid())
        return false;

    if (UrlRoute::isVirtual(url)) {
        static QStringList supportedSchemes {
            Global::Scheme::kRecent,
            Global::Scheme::kTrash,
            Global::Scheme::kComputer
        };
        return supportedSchemes.contains(url.scheme());
    }

    return ProtocolUtils::isLocalFile(url);
}

void TabBarPrivate::setTabPinned(int index, bool pinned, bool updateConfig)
{
    QString pinnedId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    if (updateConfig) {
        // Update DConfig
        auto pinnedTabs = DConfigManager::instance()->value(kViewDConfName, kPinnedTabs, QVariantList()).toList();
        const auto &tab = tabInfo(index);
        bool pinnedTabChanged = false;
        if (pinned) {
            // Add to DConfig
            QVariantMap pinnedData;
            pinnedData[TabDef::kPinnedId] = pinnedId;
            pinnedData[TabDef::kTabUrl] = getActualUrl(tab.tabUrl).toString();
            pinnedTabs.append(pinnedData);
            pinnedTabChanged = true;
            fmInfo() << "Pin tab:" << pinnedData[TabDef::kTabUrl] << "with pinnedId:" << pinnedId;
        } else {
            pinnedId = tab.userData.value(TabDef::kPinnedId).toString();
            if (!pinnedId.isEmpty()) {
                // Remove from DConfig
                for (int i = 0; i < pinnedTabs.size(); ++i) {
                    auto item = pinnedTabs[i].toMap();
                    if (item[TabDef::kPinnedId].toString() == pinnedId) {
                        pinnedTabs.removeAt(i);
                        pinnedTabChanged = true;
                        fmInfo() << "Unpin tab:" << tab.tabUrl << "with pinnedId:" << pinnedId;
                        break;
                    }
                }
            }
        }

        if (pinnedTabChanged)
            DConfigManager::instance()->setValue(kViewDConfName, kPinnedTabs, pinnedTabs);
    }

    updateTabInfo(index, [&](Tab &tab) {
        tab.isPinned = pinned;
        if (pinned)
            tab.userData[TabDef::kPinnedId] = pinnedId;
        else
            tab.userData.remove(TabDef::kPinnedId);
    });
    q->update(q->tabRect(index));
}

bool TabBarPrivate::shouldCreateNewTabForPinnedTab(const QUrl &currentUrl, const QUrl &newUrl) const
{
    // URLs are the same, no need to create new tab
    if (UniversalUtils::urlEquals(currentUrl, newUrl))
        return false;

    // Entering search from pinned tab - allow in same tab
    if (newUrl.scheme() == "search")
        return false;

    // Exiting search - check if returning to search target directory
    if (currentUrl.scheme() == "search") {
        QUrlQuery query(currentUrl.query());
        QUrl targetUrl(query.queryItemValue("url", QUrl::FullyDecoded));
        if (UniversalUtils::urlEquals(targetUrl, newUrl))
            return false;
    }

    // For all other cases, create new tab
    return true;
}

QPixmap TabBarPrivate::createColoredIcon(const QIcon &icon, const QColor &color, const QSize &size)
{
    QPixmap pixmap = icon.pixmap(size);
    pixmap.setDevicePixelRatio(qApp->devicePixelRatio());

    // Apply color to icon using composition mode
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();

    return pixmap;
}

QUrl TabBarPrivate::getActualUrl(const QUrl &url) const
{
    if (url.scheme() == "search") {
        QUrlQuery query(url.query());
        QString targetUrlStr = query.queryItemValue("url", QUrl::FullyDecoded);
        if (!targetUrlStr.isEmpty()) {
            return QUrl(targetUrlStr);
        }
    }
    return url;
}

void TabBarPrivate::closeLeftTabs(int index)
{
    // Close tabs to the left
    for (int i = index - 1; i >= 0; --i) {
        if (!q->isPinned(i))
            Q_EMIT q->tabCloseRequested(i);
    }
}

void TabBarPrivate::closeRightTabs(int index)
{
    // Close tabs to the right (reverse order to maintain indices)
    for (int i = q->count() - 1; i > index; --i) {
        if (!q->isPinned(i))
            Q_EMIT q->tabCloseRequested(i);
    }
}

void TabBarPrivate::closeOtherTabs(int index)
{
    closeRightTabs(index);
    closeLeftTabs(index);
}

TabBar::TabBar(QWidget *parent)
    : DTabBar(parent),
      d(new TabBarPrivate(this))
{
    d->initUI();
    d->initConnections();
}

TabBar::~TabBar()
{
    delete d;
}

int TabBar::appendTab()
{
    QSignalBlocker blk(this);
    int index = addTab("");
    blk.unblock();

    Tab tab;
    QString prefix = "tab_";
    tab.uniqueId = prefix + QString::number(++d->nextTabUniqueId);
    setTabData(index, QVariant::fromValue(tab));

    Q_EMIT newTabCreated();
    Q_EMIT requestCreateView(tab.uniqueId);
    setCurrentIndex(index);
    Q_EMIT currentChanged(index);
    return index;
}

int TabBar::appendInactiveTab(const QUrl &url, bool pinned)
{
    return insertInactiveTab(count(), url, pinned);
}

int TabBar::insertInactiveTab(int index, const QUrl &url, bool pinned)
{
    QSignalBlocker blk(this);
    int newIndex = insertTab(index, "");
    blk.unblock();

    Tab tab;
    QString prefix = "tab_";
    tab.uniqueId = prefix + QString::number(++d->nextTabUniqueId);
    tab.tabUrl = url;
    tab.isInactive = true;
    tab.isPinned = pinned;
    dpfHookSequence->run("dfmplugin_titlebar", "hook_Tab_SetTabName", url, &tab.tabAlias);
    setTabData(newIndex, QVariant::fromValue(tab));

    if (newIndex <= d->currentTabIndex)
        d->currentTabIndex++;

    updateTabName(newIndex);
    Q_EMIT newTabCreated();
    return newIndex;
}

void TabBar::removeTab(int index, int selectIndex)
{
    if (isPinned(index))
        return;

    if (count() == 1) {
        window()->close();
        return;
    }

    int newIndex = selectIndex;
    if (newIndex == -1) {
        int curIndex = currentIndex();
        if (curIndex == index) {
            // Delete current tab, select next tab (if last tab then select previous)
            newIndex = (index == count() - 1) ? qMax(index - 1, 0) : index + 1;
        } else {
            // Keep current index unchanged
            newIndex = curIndex;
        }
    }

    Q_EMIT tabAboutToRemove(index, newIndex);
    setCurrentIndex(newIndex);
    DTabBar::removeTab(index);
}

void TabBar::forceRemoveTab(int index)
{
    if (isPinned(index))
        d->setTabPinned(index, false);
    removeTab(index);
}

void TabBar::setCurrentUrl(const QUrl &url)
{
    int index = currentIndex();
    auto data = tabData(index);
    if (!data.isValid()) {
        appendTab();
        index = currentIndex();
        data = tabData(index);
    }

    auto tab = data.value<Tab>();

    // If current tab is pinned and URL is changing, check if need to create new tab
    if (tab.isPinned && d->shouldCreateNewTabForPinnedTab(tab.tabUrl, url)) {
        fmInfo() << "Pinned tab detected, creating new tab for URL:" << url;
        int newIndex = appendTab();
        index = newIndex;
        data = tabData(index);
        tab = data.value<Tab>();
    }

    tab.tabUrl = url;
    tab.tabAlias.clear();
    dpfHookSequence->run("dfmplugin_titlebar", "hook_Tab_SetTabName", url, &tab.tabAlias);
    setTabData(index, QVariant::fromValue(tab));

    updateTabName(index);
}

void TabBar::closeTab(const QUrl &url)
{
    for (int i = count() - 1; i >= 0; --i) {
        auto tab = tabData(i).value<Tab>();
        if (!d->tabCloseable(tab, url))
            continue;

        if (count() == 1) {
            if (isPinned(i))
                d->setTabPinned(i, false);

            d->handleLastTabClose(tab.tabUrl, url);
            return;
        }

        forceRemoveTab(i);
    }
}

bool TabBar::isTabValid(int index) const
{
    auto data = tabData(index);
    return data.isValid();
}

QUrl TabBar::tabUrl(int index) const
{
    return d->tabInfo(index).tabUrl;
}

QString TabBar::tabAlias(int index) const
{
    return d->tabInfo(index).tabAlias;
}

void TabBar::setTabAlias(int index, const QString &alias)
{
    if (d->updateTabInfo(index, [&alias](Tab &tab) {
            tab.tabAlias = alias;
        })) {
        // 更新标签显示文本
        updateTabName(index);
    }
}

QString TabBar::tabUniqueId(int index) const
{
    return d->tabInfo(index).uniqueId;
}

QVariant TabBar::tabUserData(int index, const QString &key) const
{
    return d->tabInfo(index).userData.value(key);
}

void TabBar::setTabUserData(int index, const QString &key, const QVariant &userData)
{
    d->updateTabInfo(index, [&](Tab &tab) {
        tab.userData[key] = userData;
    });
}

bool TabBar::isInactiveTab(int index) const
{
    return d->tabInfo(index).isInactive;
}

bool TabBar::isPinned(int index) const
{
    return d->tabInfo(index).isPinned;
}

void TabBar::activateNextTab()
{
    if (currentIndex() == count() - 1)
        setCurrentIndex(0);
    else
        setCurrentIndex(currentIndex() + 1);
}

void TabBar::activatePreviousTab()
{
    if (currentIndex() == 0)
        setCurrentIndex(count() - 1);
    else
        setCurrentIndex(currentIndex() - 1);
}

void TabBar::updateTabName(int index)
{
    auto tab = tabData(index).value<Tab>();
    setTabText(index, tab.tabAlias.isEmpty() ? d->tabDisplayName(tab.tabUrl) : tab.tabAlias);
}

void TabBar::paintTab(QPainter *painter, int index, const QStyleOptionTab &option) const
{
    d->paintTabBackground(painter, option);
    d->paintTabLabel(painter, index, option);
    d->paintTabItemButton(painter, index, option);
}

QSize TabBar::tabSizeHint(int index) const
{
    if (index < 0)
        return DTabBar::tabSizeHint(index);

    int total = this->width();
    int aveargeWidth = kTabMaxWidth;
    aveargeWidth = total / DTabBar::count();
    if (aveargeWidth >= kTabMaxWidth)
        aveargeWidth = kTabMaxWidth;
    else if (aveargeWidth <= kTabMinWidth)
        aveargeWidth = kTabMinWidth;

    return QSize(aveargeWidth, height());
}

QSize TabBar::minimumTabSizeHint(int index) const
{
    return { kTabMinWidth, height() };
}

QSize TabBar::maximumTabSizeHint(int index) const
{
    return { kTabMaxWidth, height() };
}

QMimeData *TabBar::createMimeDataFromTab(int index, const QStyleOptionTab &option) const
{
    Q_UNUSED(option)

    if (!isTabValid(index)) {
        fmWarning() << "Create MIME data failed: invalid tab index" << index;
        return nullptr;
    }

    QMimeData *data = new QMimeData;

    // Save tab metadata to QMimeData
    const QUrl url = tabUrl(index);
    const QString alias = tabAlias(index);

    // Build JSON data structure
    QJsonObject tabDataObj;
    tabDataObj[TabDef::kTabUrl] = url.toString();
    tabDataObj[TabDef::kTabAlias] = alias;
    tabDataObj[TabDef::kProcessId] = QApplication::applicationPid();

    QJsonDocument doc(tabDataObj);
    data->setData("application/x-dde-filemanager-tab", doc.toJson());
    return data;
}

QPixmap TabBar::createDragPixmapFromTab(int index, const QStyleOptionTab &option, QPoint *hotspot) const
{
    auto winid = FMWindowsIns.findWindowId(this);
    auto window = FMWindowsIns.findWindowById(winid);
    if (!window)
        return DTabBar::createDragPixmapFromTab(index, option, hotspot);

    int width = window->width();
    int height = window->height();
    QImage screenshotImage(width, height, QImage::Format_ARGB32_Premultiplied);
    window->render(&screenshotImage, QPoint(), QRegion(0, 0, width, height));

    // scaled image to smaller.
    int scaledWidth = width / 5;
    int scaledHeight = height / 5;
    auto scaledImage = screenshotImage.scaled(scaledWidth, scaledHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QImage backgroundImage(scaledWidth + 10, scaledHeight + 10, QImage::Format_ARGB32_Premultiplied);
    backgroundImage.fill(palette().color(QPalette::Base));
    // clip screenshot image with window radius.
    QPainter painter(&backgroundImage);
    painter.drawImage(5, 5, scaledImage);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Hide window during drag: single tab in non-Wayland environment
    if (!WindowUtils::isWayLand() && 1 == count()) {
        this->window()->hide();
        fmDebug() << "Hide window during drag: single tab in non-Wayland environment";
    }

    // adjust offset.
    hotspot->setX(scaledWidth / 2);
    hotspot->setY(scaledHeight / 2);

    QPainterPath rectPath;
    QPainterPath roundedRectPath;
    rectPath.addRect(0, 0, scaledWidth + 10, scaledHeight + 10);
    roundedRectPath.addRoundedRect(QRect(0, 0, scaledWidth + 10, scaledHeight + 10), 6, 6);
    rectPath -= roundedRectPath;

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillPath(rectPath, Qt::transparent);
    painter.end();
    return QPixmap::fromImage(backgroundImage);
}

bool TabBar::canInsertFromMimeData(int index, const QMimeData *source) const
{
    Q_UNUSED(index)
    if (!source || !source->hasFormat("application/x-dde-filemanager-tab"))
        return false;

    // Parse JSON data to check process ID
    QByteArray jsonData = source->data("application/x-dde-filemanager-tab");
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isObject()) {
        fmWarning() << "Invalid MIME data: not a JSON object";
        return false;
    }

    QJsonObject tabDataObj = doc.object();
    qint64 srcProcessId = tabDataObj[TabDef::kProcessId].toInt();
    return srcProcessId == QApplication::applicationPid();
}

bool TabBar::eventFilter(QObject *obj, QEvent *e)
{
    const QEvent::Type eventType = e->type();

    // Handle Paint event for DIconButton
    if (eventType == QEvent::Paint) {
        if (auto btn = qobject_cast<DIconButton *>(obj)) {
            d->paintTabButton(btn);
        }
        return DTabBar::eventFilter(obj, e);
    }

    // Handle drag events on TabBar itself
    if (obj == this) {
        if (eventType == QEvent::DragEnter) {
            d->isDragging = true;
        } else if (eventType == QEvent::DragLeave || eventType == QEvent::Drop) {
            d->isDragging = false;
        }
        return DTabBar::eventFilter(obj, e);
    }

    // Handle events on internal tabBar widget
    if (obj == d->tabBar) {
        if (eventType == QEvent::MouseMove) {
            const int index = tabAt(mapFromGlobal(QCursor::pos()));
            if (index != -1) {
                update(tabRect(index));
                d->updateToolTip(index, true);
            } else {
                d->updateToolTip(-1, false);
            }
        } else if (eventType == QEvent::Leave) {
            d->updateToolTip(-1, false);
        } else if (eventType == QEvent::MouseButtonPress) {
            d->updateToolTip(-1, false);
            const int index = tabAt(mapFromGlobal(QCursor::pos()));
            if (index == -1)
                return DTabBar::eventFilter(obj, e);

            auto me = static_cast<QMouseEvent *>(e);
            if (me->button() == Qt::RightButton) {
                d->handleContextMenu(index);
                return true;
            } else if (me->button() == Qt::LeftButton && d->isItemButtonHovered(index)) {
                if (isPinned(index))
                    d->setTabPinned(index, false);
                else
                    Q_EMIT tabCloseRequested(index);
                return true;
            }
        }
        return DTabBar::eventFilter(obj, e);
    }

    // Handle Show/Hide events for left scroll button
    if (obj == d->leftBtn && (eventType == QEvent::Show || eventType == QEvent::Hide)) {
        auto margin = layout()->contentsMargins();
        margin.setLeft(eventType == QEvent::Show ? 4 : 0);
        layout()->setContentsMargins(margin);
    }

    return DTabBar::eventFilter(obj, e);
}

void TabBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MiddleButton) {
        int index = tabAt(e->pos());
        if (-1 != index)
            Q_EMIT tabCloseRequested(index);
    }

    DTabBar::mousePressEvent(e);
}

void TabBar::insertFromMimeData(int index, const QMimeData *source)
{
    if (!source || !source->hasFormat("application/x-dde-filemanager-tab")) {
        fmWarning() << "Insert tab failed: invalid MIME data";
        return;
    }

    // Parse JSON data from MIME
    QByteArray jsonData = source->data("application/x-dde-filemanager-tab");
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isObject()) {
        fmWarning() << "Insert tab failed: invalid JSON data";
        return;
    }

    QJsonObject tabDataObj = doc.object();
    QUrl url = QUrl(tabDataObj[TabDef::kTabUrl].toString());
    QString alias = tabDataObj[TabDef::kTabAlias].toString();

    fmInfo() << "Inserting tab from MIME data at index" << index << "url:" << url;

    // Create inactive tab at specified index
    // Insert at specific position
    QSignalBlocker blk(this);
    int insertedIndex = insertTab(index, "");
    blk.unblock();

    // Update currentTabIndex value
    if (insertedIndex <= d->currentTabIndex)
        d->currentTabIndex++;

    Tab tab;
    QString prefix = "tab_";
    tab.uniqueId = prefix + QString::number(++d->nextTabUniqueId);
    tab.tabUrl = url;
    tab.tabAlias = alias;
    tab.isInactive = true;

    setTabData(insertedIndex, QVariant::fromValue(tab));
    updateTabName(insertedIndex);
    Q_EMIT newTabCreated();

    setCurrentIndex(index);
    fmInfo() << "Tab inserted and activated at index" << insertedIndex;
}

void TabBar::insertFromMimeDataOnDragEnter(int index, const QMimeData *source)
{
    if (!source || !source->hasFormat("application/x-dde-filemanager-tab")) {
        fmWarning() << "Insert tab failed: invalid MIME data";
        return;
    }

    // Parse JSON data from MIME
    QByteArray jsonData = source->data("application/x-dde-filemanager-tab");
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isObject()) {
        fmWarning() << "Insert tab failed: invalid JSON data";
        return;
    }

    QJsonObject tabDataObj = doc.object();
    QUrl url = QUrl(tabDataObj[TabDef::kTabUrl].toString());
    QString alias = tabDataObj[TabDef::kTabAlias].toString();

    fmInfo() << "Inserting tab from MIME data at index" << index << "url:" << url;

    // Create inactive tab at specified index
    // Insert at specific position
    QSignalBlocker blk(this);
    insertTab(index, alias.isEmpty() ? d->tabDisplayName(url) : alias);
}

void TabBar::resizeEvent(QResizeEvent *e)
{
    // 临时修改方案：通过调用setIconSize()，更新内部的layoutDirty标识，强制重新刷新标签页布局
    setIconSize(iconSize());
    DTabBar::resizeEvent(e);
}
