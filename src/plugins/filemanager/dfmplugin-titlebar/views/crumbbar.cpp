// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/crumbbar_p.h"
#include "views/crumbbar.h"
#include "views/urlpushbutton.h"
#include "views/titlebarwidget.h"
#include "utils/crumbmanager.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"
#include "events/titlebareventreceiver.h"

#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/event/event.h>

#include <DListView>

#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QMenu>
#include <QDebug>
#include <QGuiApplication>
#include <QClipboard>
#include <QPushButton>
#include <QMouseEvent>
#include <QUrlQuery>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static constexpr int kCrumbBarRectRadius { 8 };

/*!
 * \class CrumbBarPrivate
 * \brief
 */

CrumbBarPrivate::CrumbBarPrivate(CrumbBar *qq)
    : q(qq)
{
    initData();
    initUI();
    initConnections();
}

CrumbBarPrivate::~CrumbBarPrivate()
{
}

/*!
 * \brief Remove all crumbs List items inside crumb listView.
 */
void CrumbBarPrivate::clearCrumbs()
{
    for (auto button : std::as_const(navButtons)) {
        button->hide();
        button->deleteLater();
    }
    navButtons.clear();
}

void CrumbBarPrivate::updateController(const QUrl &url)
{
    if (!crumbController || !crumbController->isSupportedScheme(url.scheme())) {
        if (crumbController) {
            crumbController->deleteLater();
        }
        crumbController = CrumbManager::instance()->createControllerByUrl(url);
        // Not found? Then nothing here...
        if (!crumbController) {
            fmWarning() << "Cannot create crumb controller for URL:" << url.toString();
            // always has default controller
            crumbController = new CrumbInterface;
        }
        crumbController->setParent(q);
        QObject::connect(crumbController, &CrumbInterface::hideAddressBar, q, &CrumbBar::hideAddressBar);
        QObject::connect(crumbController, &CrumbInterface::keepAddressBar, q, &CrumbBar::onKeepAddressBar);
    }
}

/*!
 * \brief Update the crumb controller hold by the crumb bar.
 *
 * \param url The url which controller should supported.
 */

void CrumbBarPrivate::setClickableAreaEnabled(bool enabled)
{
    if (clickableAreaEnabled == enabled) return;
    clickableAreaEnabled = enabled;
    q->update();
}

void CrumbBarPrivate::writeUrlToClipboard(const QUrl &url)
{
    QString copyPath;
    if (url.isLocalFile() || !UrlRoute::hasScheme(url.scheme())) {
        copyPath = url.toString(QUrl::RemoveQuery);
    } else {
        // why? The format of the custom scheme URL was incorrect when it was converted to a string
        // eg: QUrl("recent:///") -> "recent:/"
        QUrl tmpUrl(url);
        tmpUrl.setScheme(Global::Scheme::kFile);
        copyPath = tmpUrl.toString().replace(0, 4, url.scheme());
    }

    if (copyPath.isEmpty())
        return;

    QGuiApplication::clipboard()->setText(copyPath.replace(QString(Global::Scheme::kFile) + "://", ""));
}

UrlPushButton *CrumbBarPrivate::buttonAt(QPoint pos) const
{
    UrlPushButton *button = nullptr;
    for (int i = 0; i < navButtons.size(); ++i) {
        if (navButtons[i]->geometry().contains(pos)) {
            button = navButtons[i];
            break;
        }
    }
    return button;
}

void CrumbBarPrivate::initUI()
{
    // Crumb Bar Layout
    crumbBarLayout = new QHBoxLayout(q);
    crumbBarLayout->addStretch(1);
    crumbBarLayout->setContentsMargins(kItemMargin / 2, 3, kItemMargin * 2, 3);
    crumbBarLayout->setSpacing(0);
    q->setLayout(crumbBarLayout);

    return;
}

void CrumbBarPrivate::initData()
{
    clickableAreaEnabled = Application::instance()->genericAttribute(Application::kShowCsdCrumbBarClickableArea).toBool();
}

void CrumbBarPrivate::initConnections()
{
    if (Application::instance()) {
        q->connect(Application::instance(),
                   &Application::csdClickableAreaAttributeChanged,
                   q, [=](bool enabled) {
                       setClickableAreaEnabled(enabled);
                   });
    }

    q->connect(DevProxyMng, &DeviceProxyManager::mountPointAdded, q,
               [this](const QStringView &mpt) {
                   if (lastUrl.path() == mpt)
                       q->onUrlChanged(lastUrl);
               });
}

void CrumbBarPrivate::appendWidget(QWidget *widget, int stretch)
{
    crumbBarLayout->insertWidget(crumbBarLayout->count() - 1, widget, stretch);
}

void CrumbBarPrivate::updateButtonVisibility()
{
    const int buttonsCount = navButtons.count();
    if (buttonsCount < 2) {
        return;
    }

    const auto &margins = crumbBarLayout->contentsMargins();
    int availableWidth = q->width() - margins.left() - margins.right();
    QList<UrlPushButton *> buttonsToShow;
    availableWidth -= navButtons[0]->minimumWidth();
    buttonsToShow.append(navButtons[0]);
    navButtons[1]->hide();
    int stackedWidth = navButtons[1]->minimumWidth();

    bool isLastButton = true;
    QList<CrumbData> stackedDatas;
    navButtons.last()->updateWidth();
    for (int i = navButtons.size() - 1; i > 1; --i) {
        UrlPushButton *button = navButtons[i];
        availableWidth -= button->minimumWidth();
        // 优先减去堆叠按钮宽度
        if ((availableWidth - stackedWidth) < 0)
            availableWidth -= stackedWidth;
        if (i == 0) {
            buttonsToShow.append(button);
        } else if ((availableWidth <= 0) && !isLastButton) {
            button->hide();
            for (auto data : button->crumbDatas()) {
                stackedDatas.push_front(data);
            }
        } else {
            buttonsToShow.append(button);
        }
        isLastButton = false;
    }

    // Calculate remaining width
    if (buttonsToShow.size() == 2) {
        availableWidth = q->width();
        if (q->parent()) {
            auto titleBar = qobject_cast<TitleBarWidget *>(q->parent());
            if (titleBar)
                availableWidth = titleBar->calculateRemainingWidth();
        }
        availableWidth -= kItemMargin * 3;
        if (!stackedDatas.isEmpty()) {
            availableWidth -= navButtons[1]->minimumWidth();
        }
        int lastButtonMinWidth = navButtons.last()->minimumWidth();
        if (lastButtonMinWidth > 50)
            lastButtonMinWidth = 50;
        if ((availableWidth - lastButtonMinWidth) < 0) {
            auto lastButton = buttonsToShow.takeLast();
            lastButton->hide();
            for (auto data : lastButton->crumbDatas()) {
                stackedDatas.append(data);
            }
        } else if (availableWidth < navButtons.last()->minimumWidth()) {
            // Compress current directory name
            navButtons.last()->setMinimumWidth(availableWidth);
        }
    }

    // All buttons have the correct activation state and
    // can be shown now
    for (UrlPushButton *button : std::as_const(buttonsToShow)) {
        button->show();
    }
    // 显示堆叠窗口
    if (!stackedDatas.isEmpty()) {
        navButtons[1]->setCrumbDatas(stackedDatas, true);
        navButtons[1]->show();
    }
}

/*!
 * \class CrumbBar
 * \inmodule dde-file-manager-lib
 *
 * \brief CrumbBar is the crumb bar widget of Deepin File Manager
 *
 * CrumbBar is the crumb bar widget of Deepin File Manager, provide the interface to manage
 * crumb bar state.
 *
 * \sa DFMCrumbInterface, DFMCrumbManager
 */

CrumbBar::CrumbBar(QWidget *parent)
    : QFrame(parent), d(new CrumbBarPrivate(this))
{
    setFrameShape(QFrame::NoFrame);
}

CrumbBar::~CrumbBar()
{
}

CrumbInterface *CrumbBar::controller() const
{
    return d->crumbController;
}

QUrl CrumbBar::lastUrl() const
{
    if (!d->lastUrl.isEmpty() && d->lastUrl.isValid()) {
        return d->lastUrl;
    } else {
        QString homePath { StandardPaths::location(StandardPaths::kHomePath) };
        return QUrl::fromLocalFile(homePath);
    }
}

void CrumbBar::customMenu(const QUrl &url, QMenu *menu)
{
    quint64 id { window()->internalWinId() };

    bool tabAddable { TitleBarEventReceiver::instance()->handleTabAddable(id) };
    bool displayIcon { false };   // TODO: use dde-dconfig
    bool displayNewWindowAndTab { TitleBarHelper::newWindowAndTabEnabled };

    QIcon copyIcon, newWndIcon, newTabIcon, editIcon;
    if (displayIcon) {
        copyIcon = QIcon::fromTheme("edit-copy");
        if (displayNewWindowAndTab) {
            newWndIcon = QIcon::fromTheme("window-new");
            newTabIcon = QIcon::fromTheme("tab-new");
        }
        editIcon = QIcon::fromTheme("entry-edit");
    }

    menu->addAction(copyIcon, QObject::tr("Copy path"), [this, url]() {
        QUrl u(url);
        if (dpfHookSequence->run("dfmplugin_titlebar", "hook_Copy_Addr", &u))
            d->writeUrlToClipboard(u);
        else
            d->writeUrlToClipboard(url);
    });

    if (displayNewWindowAndTab) {
        menu->addAction(newWndIcon, QObject::tr("Open in new window"), [url]() {
            TitleBarEventCaller::sendOpenWindow(url);
        });

        QAction *tabAct = menu->addAction(newTabIcon, QObject::tr("Open in new tab"), [url, id]() {
            TitleBarEventCaller::sendOpenTab(id, url);
        });
        tabAct->setDisabled(!tabAddable);
    }

    menu->addSeparator();

    QUrl fullUrl = d->lastUrl;
    menu->addAction(editIcon, QObject::tr("Edit address"), this, [this, fullUrl]() {
        emit this->editUrl(fullUrl);
    });
}

void CrumbBar::setPopupVisible(bool visible)
{
    if (d->popupVisible != visible) {
        d->popupVisible = visible;
        update();
    }
}

void CrumbBar::mousePressEvent(QMouseEvent *event)
{
    d->clickedPos = event->globalPosition().toPoint();

    if (event->button() == Qt::RightButton && d->clickableAreaEnabled) {
        event->accept();
        return;
    }

    auto button = d->buttonAt(event->pos());
    if (event->button() != Qt::RightButton || !button) {
        QFrame::mousePressEvent(event);
    }
}

void CrumbBar::mouseReleaseEvent(QMouseEvent *event)
{
    QFrame::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton) {
        QTimer::singleShot(0, this, [this]() {
            editUrl(d->lastUrl);
        });
    }
}

void CrumbBar::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(0, this, [this]() {
        d->updateButtonVisibility();
    });

    return QFrame::resizeEvent(event);
}

void CrumbBar::showEvent(QShowEvent *event)
{
    return QFrame::showEvent(event);
}

bool CrumbBar::eventFilter(QObject *watched, QEvent *event)
{
    return QFrame::eventFilter(watched, event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void CrumbBar::enterEvent(QEnterEvent *event)
#else
void CrumbBar::enterEvent(QEvent *event)
#endif
{
    QFrame::enterEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void CrumbBar::leaveEvent(QEvent *event)
{
    QFrame::leaveEvent(event);
    if (d->hoverFlag) {
        d->hoverFlag = false;
        update();
    }
}

void CrumbBar::onUrlChanged(const QUrl &url)
{
    auto sourceUrl = url;
    if (TitleBarHelper::checkKeepTitleStatus(url)) {
        QUrlQuery query { url.query() };
        QString sourcePath { query.queryItemValue("url", QUrl::FullyDecoded) };
        if (!sourcePath.isEmpty())
            sourceUrl = QUrl(sourcePath);
    }

    d->updateController(sourceUrl);
    onHideAddrAndUpdateCrumbs(sourceUrl);
}

void CrumbBar::onKeepAddressBar(const QUrl &url)
{
    QUrlQuery query { url.query() };
    QString searchKey { query.queryItemValue("keyword", QUrl::FullyDecoded) };
    emit showAddressBarText(searchKey);
}

void CrumbBar::onHideAddrAndUpdateCrumbs(const QUrl &url)
{
    emit hideAddressBar();

    d->clearCrumbs();
    bool updataFlag = false;
    if (d->popupVisible) {
        d->popupVisible = false;
        updataFlag = true;
    }
    if (d->hoverFlag) {
        d->hoverFlag = false;
        updataFlag = true;
    }
    if (updataFlag)
        update();
    if (!d->crumbController) {
        fmCritical() << "Cannot update crumbs: no controller available for URL" << url.toString();
        return;
    }

    d->lastUrl = url;
    QList<CrumbData> &&crumbDataList = d->crumbController->seprateUrl(url);
    for (int i = 0; i < crumbDataList.size(); ++i) {
        auto button = new UrlPushButton(this);
        button->setCrumbDatas(QList<CrumbData>() << crumbDataList[i]);
        if (i < (crumbDataList.size() - 1)) {
            QString subText = crumbDataList[i + 1].displayText;
            button->setActiveSubDirectory(subText);
        }
        connect(button, &UrlPushButton::urlButtonActivated, this, &CrumbBar::selectedUrl);
        button->installEventFilter(this);
        d->appendWidget(button);
        d->navButtons.append(button);
        // 增加堆叠按钮
        if (i == 0) {
            button = new UrlPushButton(this);
            button->hide();
            button->setCrumbDatas(QList<CrumbData>(), true);
            connect(button, &UrlPushButton::urlButtonActivated, this, &CrumbBar::selectedUrl);
            button->installEventFilter(this);
            d->appendWidget(button);
            d->navButtons.append(button);
        }
    }
    d->updateButtonVisibility();
}

void CrumbBar::paintEvent(QPaintEvent *event)
{
    if (d->hoverFlag || d->popupVisible) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.save();
        painter.setPen(Qt::NoPen);
        auto color = palette().color(QPalette::Button);
        color.setAlphaF(0.3);
        painter.setBrush(color);
        painter.drawRoundedRect(rect(), kCrumbBarRectRadius, kCrumbBarRectRadius);
        painter.restore();
    }
}
