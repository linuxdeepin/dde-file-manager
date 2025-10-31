// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabbar.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/event/event.h>
#include <dfm-framework/dpf.h>

#include <DIconButton>
#include <DWindowManagerHelper>
#include <DPaletteHelper>
#include <DPlatformWindowHandle>

#include <QMouseEvent>
#include <QBoxLayout>

#include <unistd.h>
#include <functional>

inline constexpr int kTabMaxWidth { 240 };
inline constexpr int kTabMinWidth { 70 };
inline constexpr int kCloseButtonSize { 16 };
inline constexpr int kCloseButtonMargin { 4 };

using namespace dfmplugin_titlebar;
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QPixmap *TabBar::sm_pDragPixmap = nullptr;

struct Tab
{
    QUrl tabUrl;
    QString tabAlias;
    QString uniqueId;
    QVariantMap userData;
    bool isInactive { false };
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
    bool isCloseButtonHovered(int index);
    void handleTabReleased(int index);
    void handleDragActionChanged(Qt::DropAction action);
    void handleContextMenu(int index);
    void handleTabClicked(int index);
    void handleIndexChanged(int index);
    void updateToolTip(int index, const QString &tip);

    QString tabDisplayName(const QUrl &url) const;
    QUrl determineRedirectUrl(const QUrl &currentUrl, const QUrl &targetUrl) const;
    QUrl findValidParentPath(const QUrl &url) const;
    void paintTabBackground(QPainter *painter, const QStyleOptionTab &option);
    void paintTabLabel(QPainter *painter, int index, const QStyleOptionTab &option);
    void paintTabButton(DIconButton *btn);

    Tab tabInfo(int index) const;
    bool updateTabInfo(int index, std::function<void(Tab &)> modifier);

public:
    TabBar *q;
    DIconButton *addBtn { nullptr };
    DIconButton *leftBtn { nullptr };
    QTabBar *tabBar { nullptr };

    int nextTabUniqueId { 0 };
    int currentTabIndex { -1 };
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
    q->setAutoHide(true);
    q->setProperty("_d_dtk_tabbartab_type", true);
    q->installEventFilter(q);

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
    connect(q, &TabBar::tabBarClicked, this, &TabBarPrivate::handleTabClicked);
    connect(q, &TabBar::tabReleaseRequested, this, &TabBarPrivate::handleTabReleased);
    connect(q, &TabBar::dragActionChanged, this, &TabBarPrivate::handleDragActionChanged);
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

bool TabBarPrivate::isCloseButtonHovered(int index)
{
    auto rect = q->tabRect(index);
    const QRect closeButtonRect(rect.right() - kCloseButtonSize - kCloseButtonMargin,
                                rect.y() + (rect.height() - kCloseButtonSize) / 2,
                                kCloseButtonSize, kCloseButtonSize);
    auto pos = q->mapFromGlobal(QCursor::pos());
    return closeButtonRect.contains(pos);
}

void TabBarPrivate::handleTabReleased(int index)
{
    if (index == -1)
        index = 0;

    const auto &url = q->tabUrl(index);
    if (!url.isValid())
        return;

    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
    q->removeTab(index);
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
    menu.addAction(TabBar::tr("Close tab"), this, [this, index] { Q_EMIT q->tabCloseRequested(index); });
    menu.addAction(TabBar::tr("Close other tabs"), this, [this, index] {
        for (int i = q->count() - 1; i > index; --i) {
            Q_EMIT q->tabCloseRequested(i);
        }
        for (int i = 0; i < index; ++i) {
            Q_EMIT q->tabCloseRequested(0);
        }
    });
    auto act = menu.addAction(TabBar::tr("Close tabs to the left"), this, [this, index] {
        for (int i = 0; i < index; ++i) {
            Q_EMIT q->tabCloseRequested(0);
        }
    });
    if (index == 0)
        act->setEnabled(false);

    act = menu.addAction(TabBar::tr("Close tabs to the right"), this, [this, index] {
        for (int i = q->count() - 1; i > index; --i) {
            Q_EMIT q->tabCloseRequested(i);
        }
    });
    if (index == q->count() - 1)
        act->setEnabled(false);

    menu.exec(QCursor::pos());
    // 防止标签处于hover状态
    QEvent event(QEvent::HoverLeave);
    QApplication::sendEvent(tabBar, &event);
}

void TabBarPrivate::handleTabClicked(int index)
{
    if (isCloseButtonHovered(index))
        Q_EMIT q->tabCloseRequested(index);
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

    Q_EMIT q->currentTabChanged(currentTabIndex, index);
    currentTabIndex = index;
}

void TabBarPrivate::updateToolTip(int index, const QString &tip)
{
    q->setTabToolTip(index, tip);
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
        QColor bgColor = isDarkTheme ? QColor(255, 255, 255, 15) : QColor(0, 0, 0, 26);
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
    const bool isHovered = option.state & QStyle::State_MouseOver;

    // 获取主题颜色
    DPalette pal = DPaletteHelper::instance()->palette(q);

    // 计算布局参数
    const int tabMargin = 10;
    const int blueMarkerWidth = isSelected ? 6 : 0;
    const int blueMarkerMargin = isSelected ? 4 : 0;
    const int closeButtonSize = isHovered ? kCloseButtonSize : 0;
    const int closeButtonMargin = isHovered ? kCloseButtonMargin : 0;

    // 计算文本可用宽度（考虑蓝色标记和关闭按钮）
    const int textMargin = blueMarkerWidth + blueMarkerMargin;
    const int leftSpace = tabMargin / 2 + textMargin;
    const int rightSpace = isHovered ? (closeButtonSize + closeButtonMargin) : 0;
    const int availableTextWidth = rect.width() - leftSpace;

    // 截断文本以适应可用宽度
    QString elidedText = option.fontMetrics.elidedText(option.text, Qt::ElideRight, availableTextWidth);

    // 计算文本居中位置
    const int textWidth = option.fontMetrics.horizontalAdvance(elidedText);
    const int textX = rect.x() + (rect.width() - option.fontMetrics.horizontalAdvance(elidedText) - textMargin) / 2 + textMargin;
    const int textY = rect.y() + (rect.height() - option.fontMetrics.height()) / 2 + option.fontMetrics.ascent();

    if (isHovered) {
        int textEndX = textX + textWidth;
        int buttonStartX = rect.right() - rightSpace;
        if (textEndX > buttonStartX) {
            // 重新计算文本宽度，确保不重叠
            elidedText = option.fontMetrics.elidedText(option.text, Qt::ElideRight, buttonStartX - textX);
        }
    }

    // 设置tooltip，只有省略时才显示
    const auto &tooltip = q->tabToolTip(index);
    if (option.text != elidedText && tooltip != option.text)
        updateToolTip(index, option.text);
    else if (option.text == elidedText && !tooltip.isEmpty())
        updateToolTip(index, "");

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

    // 绘制关闭按钮
    if (isHovered) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        const QRect closeButtonRect(rect.right() - rightSpace,
                                    rect.y() + (rect.height() - closeButtonSize) / 2,
                                    closeButtonSize, closeButtonSize);

        // 检测鼠标是否在关闭按钮上
        const bool closeButtonHovered = isCloseButtonHovered(index);
        QColor buttonColor = closeButtonHovered ? pal.color(QPalette::Highlight) : pal.color(QPalette::Text);
        painter->setPen(QPen(buttonColor, 1.5));

        // 绘制X形关闭按钮
        const int offset = 4;
        painter->drawLine(closeButtonRect.topLeft() + QPoint(offset, offset),
                          closeButtonRect.bottomRight() + QPoint(-offset, -offset));
        painter->drawLine(closeButtonRect.topRight() + QPoint(-offset, offset),
                          closeButtonRect.bottomLeft() + QPoint(offset, -offset));
        painter->restore();
    }

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

int TabBar::createTab()
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

int TabBar::createInactiveTab(const QUrl &url, const QVariantMap &userData)
{
    QSignalBlocker blk(this);
    int index = addTab("");
    blk.unblock();

    Tab tab;
    QString prefix = "tab_";
    tab.uniqueId = prefix + QString::number(++d->nextTabUniqueId);
    tab.tabUrl = url;
    tab.userData = userData;
    tab.isInactive = true;
    dpfHookSequence->run("dfmplugin_titlebar", "hook_Tab_SetTabName", url, &tab.tabAlias);
    setTabData(index, QVariant::fromValue(tab));

    updateTabName(index);
    Q_EMIT newTabCreated();
    return index;
}

void TabBar::removeTab(int index, int selectIndex)
{
    int newIndex = selectIndex;
    if (newIndex == -1) {
        int curIndex = currentIndex();
        if (curIndex < index) {
            // Current tab is before the deleted tab, keep current index unchanged
            newIndex = curIndex;
        } else if (curIndex == index) {
            // Delete current tab, select next tab (if last tab then select previous)
            newIndex = (index == count() - 1) ? qMax(index - 1, 0) : index;
        } else {
            // Current tab is after deleted tab, decrease index by 1
            newIndex = curIndex - 1;
        }
    }

    Q_EMIT tabHasRemoved(index, newIndex);
    setCurrentIndex(newIndex);
    DTabBar::removeTab(index);
}

void TabBar::setCurrentUrl(const QUrl &url)
{
    int index = currentIndex();
    auto data = tabData(index);
    if (!data.isValid()) {
        createTab();
        index = currentIndex();
        data = tabData(index);
    }

    auto tab = data.value<Tab>();
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
            d->handleLastTabClose(tab.tabUrl, url);
        } else {
            removeTab(i);
        }
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
    QMimeData *data = new QMimeData;
    data->setParent(window());
    data->removeFormat("text/plain");
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
    backgroundImage.fill(QColor(palette().color(QPalette::Base)));
    // clip screenshot image with window radius.
    QPainter painter(&backgroundImage);
    painter.drawImage(5, 5, scaledImage);
    painter.setRenderHint(QPainter::Antialiasing, true);

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

    QColor shadowColor = QColor(palette().color(QPalette::BrightText));
    shadowColor.setAlpha(80);

    painter.end();
    if (sm_pDragPixmap) delete sm_pDragPixmap;
    sm_pDragPixmap = new QPixmap(QPixmap::fromImage(backgroundImage));
    return QPixmap::fromImage(backgroundImage);
}

bool TabBar::canInsertFromMimeData(int index, const QMimeData *source) const
{
    return false;
}

bool TabBar::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == this && e->type() == QEvent::Paint) {
        // 为了禁止DTabBar绘制延长线
        return true;
    } else if (e->type() == QEvent::Paint) {
        auto btn = qobject_cast<DIconButton *>(obj);
        if (btn)
            d->paintTabButton(btn);
    } else if (obj == d->tabBar && e->type() == QEvent::MouseMove) {
        int index = tabAt(mapFromGlobal(QCursor::pos()));
        if (index != -1)
            update(tabRect(index));
    } else if (e->type() == QEvent::MouseButtonPress) {
        auto me = static_cast<QMouseEvent *>(e);
        if (me->button() == Qt::RightButton) {
            int index = tabAt(mapFromGlobal(QCursor::pos()));
            if (index != -1 && count() > 1) {
                d->handleContextMenu(index);
                return true;
            }
        }
    } else if (obj == d->leftBtn && e->type() == QEvent::Show) {
        auto margin = layout()->contentsMargins();
        margin.setLeft(4);
        layout()->setContentsMargins(margin);
    } else if (obj == d->leftBtn && e->type() == QEvent::Hide) {
        auto margin = layout()->contentsMargins();
        margin.setLeft(0);
        layout()->setContentsMargins(margin);
    }

    return DTabBar::eventFilter(obj, e);
}

void TabBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MiddleButton) {
        int index = tabAt(e->pos());
        if (-1 != index && count() > 1)
            Q_EMIT tabCloseRequested(index);
    }

    DTabBar::mousePressEvent(e);
}

void TabBar::resizeEvent(QResizeEvent *e)
{
    // 临时修改方案：通过调用setIconSize()，更新内部的layoutDirty标识，强制重新刷新标签页布局
    setIconSize(iconSize());
    DTabBar::resizeEvent(e);
}
