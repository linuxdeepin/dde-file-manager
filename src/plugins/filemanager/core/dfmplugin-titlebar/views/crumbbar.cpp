// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/crumbbar_p.h"
#include "views/crumbbar.h"
#include "models/crumbmodel.h"
#include "utils/crumbmanager.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"

#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/event/event.h>

#include <DListView>

// #include <QGSettings>
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

static QString getIconName(const CrumbData &c)
{
    QString iconName = c.iconName;

    if (!iconName.isEmpty() && !iconName.startsWith("dfm_") && !iconName.contains("-symbolic"))
        iconName.append("-symbolic");

    return iconName;
}

/*!
 * \class IconItemDelegate
 * \brief
 */

IconItemDelegate::IconItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
    setItemSpacing(6);
    setMargins(QMargins(4, 0, 4, 0));
}

void IconItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.decorationAlignment = Qt::AlignCenter;
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    DStyledItemDelegate::paint(painter, opt, index);
}

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
    leftArrow.hide();
    rightArrow.hide();

    if (crumbModel)
        crumbModel->removeAll();
}

void CrumbBarPrivate::checkArrowVisiable()
{
    QScrollBar *scrollBar = crumbView.horizontalScrollBar();
    if (!scrollBar)
        return;

    leftArrow.setVisible(scrollBar->maximum() > 0);
    rightArrow.setVisible(scrollBar->maximum() > 0);

    leftArrow.setEnabled(scrollBar->value() != scrollBar->minimum());
    rightArrow.setEnabled(scrollBar->value() != scrollBar->maximum());
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
            fmWarning() << "Unsupported url / scheme: " << url;
            // always has default controller
            crumbController = new CrumbInterface;
        }
        crumbController->setParent(q);
        QObject::connect(crumbController, &CrumbInterface::hideAddressBar, q, &CrumbBar::hideAddressBar);
        QObject::connect(crumbController, &CrumbInterface::keepAddressBar, q, &CrumbBar::onKeepAddressBar);
        QObject::connect(crumbController, &CrumbInterface::hideAddrAndUpdateCrumbs, q, &CrumbBar::onHideAddrAndUpdateCrumbs);
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
    if (dfmbase::FileUtils::isLocalFile(url) || !UrlRoute::hasScheme(url.scheme())) {
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

void CrumbBarPrivate::initUI()
{
    // Arrows
    QSize size(24, 24), iconSize(16, 16);
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(&leftArrow), AcName::kAcComputerCrumbBarLeftArrow);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(&rightArrow), AcName::kAcComputerCrumbBarRightArrow);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(&crumbView), AcName::kAcComputerCrumbBarListView);
#endif

    leftArrow.setFocusPolicy(Qt::NoFocus);
    leftArrow.setIcon(QIcon::fromTheme("go-previous"));
    rightArrow.setIcon(QIcon::fromTheme("go-next"));
    rightArrow.setFocusPolicy(Qt::NoFocus);

    leftArrow.setFixedSize(size);
    leftArrow.setIconSize(iconSize);
    rightArrow.setFixedSize(size);
    rightArrow.setIconSize(iconSize);
    leftArrow.setFlat(true);
    rightArrow.setFlat(true);
    leftArrow.hide();
    rightArrow.hide();

    // Crumb List Layout
    crumbView.setObjectName("DCrumbListScrollArea");

    crumbView.setItemSpacing(6);
    crumbView.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbView.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbView.setFocusPolicy(Qt::NoFocus);
    crumbView.setContentsMargins(0, 0, 0, 0);
    crumbView.setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    crumbView.setIconSize({ 16, 16 });
    crumbView.setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    crumbView.setOrientation(QListView::LeftToRight, false);
    crumbView.setEditTriggers(QAbstractItemView::NoEditTriggers);
    crumbView.setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    crumbModel = new CrumbModel(q);
    crumbView.setModel(crumbModel);
    crumbView.setContextMenuPolicy(Qt::CustomContextMenu);

    // 点击listview空白可拖动窗口
    crumbView.viewport()->installEventFilter(q);
    crumbView.setItemDelegateForRow(0, new IconItemDelegate(&crumbView));
    crumbView.setItemMargins(QMargins(0, 0, 0, 0));

    // Crumb Bar Layout
    crumbBarLayout = new QHBoxLayout(q);
    crumbBarLayout->addWidget(&leftArrow);
    crumbBarLayout->addWidget(&crumbView);
    crumbBarLayout->addWidget(&rightArrow);
    crumbBarLayout->setContentsMargins(0, 0, 0, 0);
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
    QObject::connect(&crumbView, &QListView::customContextMenuRequested,
                     q, &CrumbBar::onCustomContextMenu);

    QObject::connect(&crumbView, &QListView::clicked,
                     q, [=](const QModelIndex &index) {
                         if (index.isValid()) {
                             fmInfo() << "sig send selectedUrl: " << index.data().toUrl();
                             emit q->selectedUrl(index.data(CrumbModel::FileUrlRole).toUrl());
                         }
                     });

    q->connect(&leftArrow, &DPushButton::clicked,
               q, [=]() {
                   crumbView.horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
               });

    q->connect(&rightArrow, &DPushButton::clicked,
               q, [=]() {
                   crumbView.horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
               });

    q->connect(crumbView.horizontalScrollBar(), &QScrollBar::valueChanged,
               q, [=]() {
                   checkArrowVisiable();
               });

    if (Application::instance()) {
        q->connect(Application::instance(),
                   &Application::csdClickableAreaAttributeChanged,
                   q, [=](bool enabled) {
                       setClickableAreaEnabled(enabled);
                   });
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

void CrumbBar::mousePressEvent(QMouseEvent *event)
{
    d->clickedPos = event->globalPos();

    if (event->button() == Qt::RightButton && d->clickableAreaEnabled) {
        event->accept();
        return;
    }

    QModelIndex index = d->crumbView.indexAt(event->pos());
    if (event->button() != Qt::RightButton || !index.isValid()) {
        QFrame::mousePressEvent(event);
    }
}

void CrumbBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (!d->clickableAreaEnabled) {
        return QFrame::mouseReleaseEvent(event);
    }

    QFrame::mouseReleaseEvent(event);
}

void CrumbBar::resizeEvent(QResizeEvent *event)
{
    d->checkArrowVisiable();

    return QFrame::resizeEvent(event);
}

void CrumbBar::showEvent(QShowEvent *event)
{
    // d->crumbListScrollArea.horizontalScrollBar()->setPageStep(d->crumbListHolder->width());
    d->crumbView.horizontalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);

    d->checkArrowVisiable();

    return QFrame::showEvent(event);
}

bool CrumbBar::eventFilter(QObject *watched, QEvent *event)
{
    QMouseEvent *me = nullptr;
    if (watched && watched->parent() == &d->crumbView && (me = dynamic_cast<QMouseEvent *>(event))) {
        QEvent::Type type = event->type();
        bool isMousePressed = type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick;
        static QPoint pos;
        if (isMousePressed) {
            pos = QCursor::pos();
        }

        bool isIgnore = isMousePressed || type == QEvent::MouseMove;
        if (isIgnore) {
            event->ignore();
            return true;
        }

        bool isDragging = (pos - QCursor::pos()).manhattanLength() > QApplication::startDragDistance();
        if (type == QEvent::MouseButtonRelease && me->button() == Qt::LeftButton) {
            event->ignore();
            QModelIndex index = d->crumbView.indexAt(me->pos());
            if (index.isValid() && index != d->crumbView.currentIndex() && !isDragging) {
                d->crumbView.clicked(index);
                return true;
            }
        }
    }

    return QFrame::eventFilter(watched, event);
}

void CrumbBar::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = d->crumbView.indexAt(point);
    if (!index.isValid())
        return;

    quint64 id { window()->internalWinId() };
    bool tabAddable { TitleBarEventCaller::sendCheckTabAddable(id) };
    bool displayIcon { TitleBarHelper::displayIcon() };
    bool displayNewWindowAndTab { TitleBarHelper::newWindowAndTabEnabled };
    QMenu *menu { new QMenu() };
    QUrl url { index.data(CrumbModel::FileUrlRole).toUrl() };
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

    QUrl fullUrl { index.data(CrumbModel::FullUrlRole).toUrl() };
    menu->addAction(editIcon, QObject::tr("Edit address"), this, [this, fullUrl]() {
        emit this->editUrl(fullUrl);
    });

    menu->exec(QCursor::pos());
    delete menu;
}

void CrumbBar::onUrlChanged(const QUrl &url)
{
    d->updateController(url);

    if (d->crumbController)
        d->crumbController->crumbUrlChangedBehavior(url);
}

void CrumbBar::onKeepAddressBar(const QUrl &url)
{
    QUrlQuery query { url.query() };
    QString searchKey { query.queryItemValue("keyword", QUrl::FullyDecoded) };
    emit showAddressBarText(searchKey);
}

void CrumbBar::onHideAddrAndUpdateCrumbs(const QUrl &url)
{
    emit hideAddressBar(false);

    d->clearCrumbs();

    if (!d->crumbController) {
        fmWarning("No controller found when trying to call DFMCrumbBar::updateCrumbs() !!!");
        fmDebug() << "updateCrumbs (no controller) : " << url;
        return;
    }

    d->lastUrl = url;
    QList<CrumbData> &&crumbDataList = d->crumbController->seprateUrl(url);

    // create QStandardItem by crumb data
    for (const CrumbData &c : crumbDataList) {
        if (d->crumbModel) {
            QStandardItem *listitem = nullptr;
            if (c.iconName.isEmpty()) {
                listitem = new QStandardItem(c.displayText);
            } else {
                listitem = new QStandardItem(QIcon::fromTheme(getIconName(c)), QString());
            }

            listitem->setTextAlignment(Qt::AlignCenter);
            listitem->setCheckable(false);
            Q_ASSERT(c.url.isValid());
            listitem->setData(c.url, CrumbModel::Roles::FileUrlRole);
            listitem->setData(url, CrumbModel::Roles::FullUrlRole);
            d->crumbModel->appendRow(listitem);
        }
    }

    if (d->crumbView.selectionModel() && d->crumbModel)
        d->crumbView.selectionModel()->select(d->crumbModel->lastIndex(), QItemSelectionModel::Select);

    d->checkArrowVisiable();
    d->crumbView.horizontalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);
}
