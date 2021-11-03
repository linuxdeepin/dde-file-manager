/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QMenu>
#include <QDebug>
#include <QGuiApplication>
#include <QClipboard>
#include <dgiosettings.h>
#include <QPushButton>

#include "dfmapplication.h"
#include "dfmcrumbbar.h"
#include "private/dfmcrumbbar_p.h"
#include "dfmcrumbmanager.h"
#include "dfmsettings.h"
#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmaddressbar.h"
#include "dfmcrumbfactory.h"
#include "dfmcrumbinterface.h"
#include "dfmapplication.h"
#include "dfmevent.h"
#include "dlistview.h"
#include "dfmcrumblistviewmodel.h"
#include "app/define.h"
#include "singleton.h"
#include "interfaces/dfmstandardpaths.h"
#include "controllers/pathmanager.h"
#include "controllers/vaultcontroller.h"
#include "interfaces/dfilemenu.h"
#include "accessibility/ac-lib-file-manager.h"

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DFMCrumbBarPrivate::DFMCrumbBarPrivate(DFMCrumbBar *qq)
    : q_ptr(qq)
{
    initData();
    initUI();
    initConnections();
}

DFMCrumbBarPrivate::~DFMCrumbBarPrivate()
{
    if (crumbController) {
        crumbController->deleteLater();
        crumbController = nullptr;
    }
}

/*!
 * \brief Remove all crumbs List items inside crumb listView.
 */
void DFMCrumbBarPrivate::clearCrumbs()
{
    leftArrow.hide();
    rightArrow.hide();

    if (crumbListviewModel)
        crumbListviewModel->removeAll();
}

void DFMCrumbBarPrivate::checkArrowVisiable()
{
    QScrollBar *sb = crumbListView.horizontalScrollBar();
    if (!sb)
        return;
    AC_SET_OBJECT_NAME(sb, AC_COMPUTER_CRUMB_BAR_SCROLL_BAR);
    AC_SET_ACCESSIBLE_NAME(sb, AC_COMPUTER_CRUMB_BAR_SCROLL_BAR);

    leftArrow.setVisible(!addressBar->isVisible() && sb->maximum() > 0);
    rightArrow.setVisible(!addressBar->isVisible() && sb->maximum() > 0);

    leftArrow.setEnabled(sb->value() != sb->minimum());
    rightArrow.setEnabled(sb->value() != sb->maximum());
}

/*!
 * \brief Update the crumb controller hold by the crumb bar.
 *
 * \param url The url which controller should supported.
 */
void DFMCrumbBarPrivate::updateController(const DUrl &url)
{
    Q_Q(DFMCrumbBar);

    if (!crumbController || !crumbController->supportedUrl(url)) {
        if (crumbController) {
            crumbController->deleteLater();
        }
        // 修复bug41522-保险箱内，右键文件管理器打开文件夹，面包屑显示不对问题
        DUrl newurl;
        if (VaultController::isVaultFile(url.toString())) {
            newurl = VaultController::localUrlToVault(url);
        } else {
            newurl = url;
        }
        crumbController = DFMCrumbManager::instance()->createControllerByUrl(newurl, q);
        // Not found? Then nothing here...
        if (!crumbController) {
            qDebug() << "Unsupported url / scheme: " << newurl;
        } else
            crumbController->setParent(q_ptr);
    }
}

void DFMCrumbBarPrivate::setClickableAreaEnabled(bool enabled)
{
    Q_Q(DFMCrumbBar);

    if (clickableAreaEnabled == enabled) return;

    clickableAreaEnabled = enabled;
    //crumbListHolder->setContentsMargins(0, 0, (enabled ? 30 : 0), 0);

    q->update();
}

void DFMCrumbBarPrivate::initUI()
{
    Q_Q(DFMCrumbBar);

    // Address Bar
    addressBar->hide();

    // Crumbbar Widget
    //q->setFixedHeight(24);

    // Arrows
    QSize size(24, 24), iconSize(16, 16);
    AC_SET_OBJECT_NAME((&leftArrow), AC_COMPUTER_CRUMB_BAR_LEFT_ARROW);
    AC_SET_ACCESSIBLE_NAME((&leftArrow), AC_COMPUTER_CRUMB_BAR_LEFT_ARROW);
    AC_SET_OBJECT_NAME((&rightArrow), AC_COMPUTER_CRUMB_BAR_RIGHT_ARROW);
    AC_SET_ACCESSIBLE_NAME((&rightArrow), AC_COMPUTER_CRUMB_BAR_RIGHT_ARROW);

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
    crumbListView.setObjectName("DCrumbListScrollArea");
    AC_SET_OBJECT_NAME((&crumbListView), AC_COMPUTER_CRUMB_BAR_LIST_VIEW);
    AC_SET_ACCESSIBLE_NAME((&crumbListView), AC_COMPUTER_CRUMB_BAR_LIST_VIEW);

    crumbListView.setItemSpacing(10);
    crumbListView.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbListView.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbListView.setFocusPolicy(Qt::NoFocus);
    crumbListView.setContentsMargins(0, 0, 0, 0);
    crumbListView.setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    crumbListView.setIconSize({16, 16});
    crumbListView.setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    crumbListView.setOrientation(QListView::LeftToRight, false);
    crumbListView.setEditTriggers(QAbstractItemView::NoEditTriggers);
    crumbListView.setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    crumbListviewModel = new DFMCrumbListviewModel(q);
    crumbListView.setModel(crumbListviewModel);
    crumbListView.setContextMenuPolicy(Qt::CustomContextMenu);

    // 点击listview空白可拖动窗口
    crumbListView.viewport()->installEventFilter(q);

    // for first icon item icon AlignCenter...
    class IconItemDelegate : public DStyledItemDelegate
    {
    public:
        explicit IconItemDelegate(QAbstractItemView *parent = nullptr): DStyledItemDelegate(parent)
        {
            setItemSpacing(10);
        }

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            QStyleOptionViewItem opt = option;
            opt.decorationAlignment = Qt::AlignCenter;
            DStyledItemDelegate::paint(painter, opt, index);
        }
    };
    crumbListView.setItemDelegateForRow(0, new IconItemDelegate(&crumbListView));

    // Crumb Bar Layout
    crumbBarLayout = new QHBoxLayout(q);
    crumbBarLayout->addWidget(&leftArrow);
    crumbBarLayout->addWidget(&crumbListView);
    crumbBarLayout->addWidget(&rightArrow);
    crumbBarLayout->setContentsMargins(0, 0, 0, 0);
    crumbBarLayout->setSpacing(0);
    q->setLayout(crumbBarLayout);

    return;
}

void DFMCrumbBarPrivate::initData()
{
    Q_Q(DFMCrumbBar);
    addressBar = new DFMAddressBar(q);
    clickableAreaEnabled = DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowCsdCrumbBarClickableArea).toBool();
}

void DFMCrumbBarPrivate::initConnections()
{
    Q_Q(DFMCrumbBar);
    QObject::connect(&crumbListView, &QListView::customContextMenuRequested, q, &DFMCrumbBar::onListViewContextMenu);

    QObject::connect(&crumbListView, &QListView::clicked, q, [q](const QModelIndex & index) {
        if (index.isValid()) {
            emit q->crumbListItemSelected(DUrl(index.data(DFMCrumbListviewModel::FileUrlRole).toUrl()));
        }
    });

    q->connect(&leftArrow, &QPushButton::clicked, q, [this]() {
        crumbListView.horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
    });

    q->connect(&rightArrow, &QPushButton::clicked, q, [this]() {
        crumbListView.horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    });

    q->connect(crumbListView.horizontalScrollBar(), &QScrollBar::valueChanged, q, [this]() {
        checkArrowVisiable();
    });

    q->connect(addressBar, &DFMAddressBar::returnPressed, q, [q, this]() {

        //! 如果为保险箱路径则进行路径转换
        QString str = VaultController::toInternalPath(addressBar->text());

        emit q->addressBarContentEntered(str);
    });

    q->connect(addressBar, &DFMAddressBar::escKeyPressed, q, [this]() {
        if (crumbController) {
            crumbController->processAction(DFMCrumbInterface::EscKeyPressed);
        }
    });

    q->connect(addressBar, &DFMAddressBar::lostFocus, q, [q, this]() {
        if (crumbController && !qobject_cast<DFileManagerWindow *>(q->window())->isAdvanceSearchBarVisible()) {
            crumbController->processAction(DFMCrumbInterface::AddressBarLostFocus);
        }
    });

    q->connect(addressBar, &DFMAddressBar::clearButtonPressed, q, [this] {
        if (crumbController && !addressBar->text().isEmpty())
        {
            crumbController->processAction(DFMCrumbInterface::ClearButtonPressed);
        }
    });

    q->connect(addressBar, &DFMAddressBar::pauseButtonClicked, q, [this]() {
        if (crumbController)
            crumbController->processAction(DFMCrumbInterface::PauseButtonClicked);
    });

    q->connect(DFMApplication::instance(), &DFMApplication::csdClickableAreaAttributeChanged, q, [this](bool enabled) {
        setClickableAreaEnabled(enabled);
    });
}

/*!
 * \class DFMCrumbBar
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMCrumbBar is the crumb bar widget of Deepin File Manager
 *
 * DFMCrumbBar is the crumb bar widget of Deepin File Manager, provide the interface to manage
 * crumb bar state.
 *
 * \sa DFMCrumbInterface, DFMCrumbManager
 */

DFMCrumbBar::DFMCrumbBar(QWidget *parent)
    : QFrame(parent)
    , d_ptr(new DFMCrumbBarPrivate(this))
{
    AC_SET_OBJECT_NAME(this, AC_COMPUTER_CRUMB_BAR);
    AC_SET_ACCESSIBLE_NAME(this, AC_COMPUTER_CRUMB_BAR);
    setFrameShape(QFrame::NoFrame);
}

DFMCrumbBar::~DFMCrumbBar()
{

}

/*!
 * \brief Toggle and show the address bar.
 *
 * \param text The text in the address bar.
 */
void DFMCrumbBar::showAddressBar(const QString &text)
{
    Q_D(DFMCrumbBar);

    d->leftArrow.hide();
    d->rightArrow.hide();
    d->crumbListView.hide();

    d->addressBar->show();
    d->addressBar->setText(text);
    //d->addressBar->setSelection(0, text.length());
    d->addressBar->setFocus();

    emit addressBarShown();

    return;
}

/*!
 * \brief Toggle and show the address bar.
 *
 * \param url The url in the address bar.
 */
void DFMCrumbBar::showAddressBar(const DUrl &url)
{
    Q_D(DFMCrumbBar);

    d->leftArrow.hide();
    d->rightArrow.hide();
    d->crumbListView.hide();

    d->addressBar->show();
    d->addressBar->setCurrentUrl(url);
    d->addressBar->setFocus();

    emit addressBarShown();

    return;
}

/*!
 * \brief Hide the address bar.
 */
void DFMCrumbBar::hideAddressBar()
{
    Q_D(DFMCrumbBar);

    d->addressBar->hide();

    d->crumbListView.show();
    d->checkArrowVisiable();

    emit addressBarHidden();

    return;
}

static QString getIconName(const CrumbData &c)
{
    QString iconName = c.iconName;
    if (c.url == DUrl(TRASH_ROOT)) {
        iconName = systemPathManager->getSystemPathIconName("Trash");
    } else if (c.url.isNetWorkFile() || c.url.isSMBFile() || c.url.isFTPFile() || c.url.isSFTPFile()) {
        iconName = systemPathManager->getSystemPathIconName("Network");
    }

    if (!iconName.isEmpty() && !iconName.startsWith("dfm_") && !iconName.contains("-symbolic"))
        iconName.append("-symbolic");

    return iconName;
}

/*!
 * \brief Update crumbs in crumb bar by the given \a url
 *
 * \param url The newly switched url.
 *
 * DFMCrumbBar holds an instance of crumb controller (derived from DFMCrumbInterface), and when
 * calling updateCrumb, it will check the current used crumb controller is supporting the given
 * \a url. if isn't, we will create a new crumb controller form the registed controllers or the
 * plugin list. Then we will call DFMCrumbInterface::seprateUrl to seprate the url
 */
void DFMCrumbBar::updateCrumbs(const DUrl &url)
{
    Q_D(DFMCrumbBar);

    d->clearCrumbs();

    if (!d->crumbController) {
        qWarning("No controller found when trying to call DFMCrumbBar::updateCrumbs() !!!");
        qDebug() << "updateCrumbs (no controller) : " << url;
        return;
    }

    // 回收站预览打开文件夹时传过来的是真实路径，所以将其转换为虚拟路径
    DUrl fileUrl = url;
    if (url.path().startsWith(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))) {
        QString trashFilePath = url.path();
        if (trashFilePath == DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))
            trashFilePath = trashFilePath + "/";
        fileUrl = DUrl::fromTrashFile(trashFilePath.remove(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath)));
        d->updateController(fileUrl);
    } else if (VaultController::isVaultFile(url.toLocalFile())) {
        // 修复bug-60781
        // 如果是保险箱文件，转化为保险箱路径，以便创建保险箱的面包屑
        fileUrl = VaultController::localToVault(url.toLocalFile());
    }

    //NOTE [HMOE REN] 已注册的CrumbController(面包屑)创建的面包屑值
    QList<CrumbData> crumbDataList = d->crumbController->seprateUrl(fileUrl);
    for (const CrumbData &c : crumbDataList) {
        if (d->crumbListviewModel) {
            QString iconName = getIconName(c);
            QStandardItem *listitem = nullptr;
            if (!iconName.isEmpty()) {
                listitem = new QStandardItem(QIcon::fromTheme(iconName), QString());
            } else {
                listitem = new QStandardItem(c.displayText);
            }
            listitem->setTextAlignment(Qt::AlignCenter);//垂直与水平居中
            listitem->setCheckable(false);
            listitem->setData(c.url, DFMCrumbListviewModel::FileUrlRole);
            d->crumbListviewModel->appendRow(listitem);
        }
    }

    if (d->crumbListView.selectionModel() && d->crumbListviewModel)
        d->crumbListView.selectionModel()->select(d->crumbListviewModel->lastIndex(), QItemSelectionModel::Select);

    d->checkArrowVisiable();
    d->crumbListView.horizontalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);
}

void DFMCrumbBar::playAddressBarAnimation()
{
    Q_D(DFMCrumbBar);

    d->addressBar->playAnimation();
}

void DFMCrumbBar::stopAddressBarAnimation()
{
    Q_D(DFMCrumbBar);

    d->addressBar->stopAnimation();
}

/*!
 * \brief Call when toolbar url got changed.
 *
 * \param url Current url which we changed to.
 */
void DFMCrumbBar::updateCurrentUrl(const DUrl &url)
{
    Q_D(DFMCrumbBar);

    d->updateController(url);

    if (d->crumbController) {
        d->crumbController->crumbUrlChangedBehavior(url);
    }
}

void DFMCrumbBar::mousePressEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbBar);
    d->clickedPos = event->globalPos();

    if (event->button() == Qt::RightButton && d->clickableAreaEnabled) {
        event->accept();
        return;
    }

    QModelIndex index = d->crumbListView.indexAt(event->pos());
    if (event->button() != Qt::RightButton || !index.isValid()) {
        QFrame::mousePressEvent(event);
    }
}

void DFMCrumbBar::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbBar);

    if (!d->clickableAreaEnabled) {
        return QFrame::mouseReleaseEvent(event);;
    }

    // blumia: no need to check if it's clicked on other widgets
    //         since this will only happend when clicking empty.
    const QPoint pos_difference = d->clickedPos - event->globalPos();

    if (qAbs(pos_difference.x()) < 2 && qAbs(pos_difference.y()) < 2) {
        showAddressBar(qobject_cast<DFileManagerWindow *>(topLevelWidget())->currentUrl());
    }

    QFrame::mouseReleaseEvent(event);
}

void DFMCrumbBar::resizeEvent(QResizeEvent *event)
{
    Q_D(DFMCrumbBar);

    d->checkArrowVisiable();
    d->addressBar->resize(event->size());

    return QFrame::resizeEvent(event);
}

void DFMCrumbBar::showEvent(QShowEvent *event)
{
    Q_D(DFMCrumbBar);

    //d->crumbListScrollArea.horizontalScrollBar()->setPageStep(d->crumbListHolder->width());
    d->crumbListView.horizontalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);

    d->checkArrowVisiable();

    return QFrame::showEvent(event);
}

bool DFMCrumbBar::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(DFMCrumbBar);
    QMouseEvent *me = nullptr;
    if (watched && watched->parent() == &d->crumbListView && (me = dynamic_cast<QMouseEvent *>(event))) {
        QEvent::Type type = event->type();
        bool isMousePressed = type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick;
        static QPoint pos;
        if (isMousePressed) {
            pos = QCursor::pos();
        }

        bool isIgnore = isMousePressed ||  type == QEvent::MouseMove;
        if (isIgnore) {
            event->ignore();
            return true;
        }

        bool isDragging = (pos - QCursor::pos()).manhattanLength() > QApplication::startDragDistance();
        if (type == QEvent::MouseButtonRelease && me->button() == Qt::LeftButton && !isDragging) {
            QModelIndex index = d->crumbListView.indexAt(me->pos());
            if (index.isValid() && index != d->crumbListView.currentIndex()) {
                d->crumbListView.clicked(index);
                return true;
            }
        }
    }

    return QFrame::eventFilter(watched, event);
}

void DFMCrumbBar::onListViewContextMenu(const QPoint &point)
{
    Q_D(DFMCrumbBar);
    QModelIndex index = d->crumbListView.indexAt(point);
    if (!index.isValid())
        return ;

    DFileMenu *menu = new DFileMenu();
    DUrl url = DUrl(index.data(DFMCrumbListviewModel::FileUrlRole).toUrl());
    menu->setAccessibleInfo(AC_FILE_MENU_CRUMB_BAR);
    DGioSettings settings("com.deepin.dde.filemanager.general", "/com/deepin/dde/filemanager/general/");
    bool displayIcon = settings.value("context-menu-icons").toBool();
    QIcon copyIcon, newWndIcon,  newTabIcon, editIcon;
    if (displayIcon) {
        copyIcon = QIcon::fromTheme("edit-copy");
        newWndIcon = QIcon::fromTheme("window-new");
        newTabIcon = QIcon::fromTheme("tab-new");
        editIcon = QIcon::fromTheme("entry-edit");
    }

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(window());
    if (!wnd) {
        menu->deleteLater();
        menu = nullptr;
        return;
    }

    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());
    menu->addAction(copyIcon, QObject::tr("Copy path"), [ = ]() {
        // 如果为保险箱路径则进行路径转换
        QString virtualUrl(url.toString());
        if (VaultController::isVaultFile(virtualUrl))
            virtualUrl = VaultController::localPathToVirtualPath(url.toLocalFile());
        QGuiApplication::clipboard()->setText(virtualUrl);
    });

    menu->addAction(newWndIcon, QObject::tr("Open in new window"), [url]() {
        WindowManager::instance()->showNewWindow(url, true);
    });

    menu->addAction(newTabIcon, QObject::tr("Open in new tab"), [wnd, url]() {
        wnd->openNewTab(url);
    })->setDisabled(shouldDisable);

    menu->addSeparator();

    menu->addAction(editIcon, QObject::tr("Edit address"), this, [ = ]() {

        DUrl url = wnd->currentUrl();
        // 如果为保险箱路径则进行路径转换
        QString realUrl(url.toString());
        if (VaultController::isVaultFile(realUrl)) {
            realUrl = VaultController::localPathToVirtualPath(url.toLocalFile());
        } else if (url.path().startsWith(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))) {
            QString trashFilePath = url.path();
            if (trashFilePath == DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))
                trashFilePath = trashFilePath + "/";
            realUrl = trashFilePath.replace(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath) + "/", TRASH_ROOT);
        }
        showAddressBar(realUrl);
    });
    //fix bug 33305 在用右键菜单复制大量文件时，在复制过程中，关闭窗口这时this释放了，在关闭拷贝menu的exec退出，menu的deleteLater崩溃
    QPointer<DFMCrumbBar> me = this;
    menu->exec(QCursor::pos());
    menu->deleteLater(me);
}

/*!
 * \fn DFMCrumbBar::addressBarShown()
 *
 * \brief Emit when the address bar got shown.
 *
 * \sa DFMAddressBar
 */

/*!
 * \fn DFMCrumbBar::addressBarHidden()
 *
 * \brief Emit when the address bar got hidden.
 *
 * \sa DFMAddressBar
 */

DFM_END_NAMESPACE
