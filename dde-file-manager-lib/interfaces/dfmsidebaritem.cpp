/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "dfmsidebaritem.h"
#include "dfileservices.h"
#include "dfilemenu.h"
#include "dfilemenumanager.h"
#include "dfmsidebar.h"
#include "dstorageinfo.h"

#include "singleton.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "views/windowmanager.h"
#include "views/themeconfig.h"
#include "deviceinfo/udisklistener.h"

#include <QDrag>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMimeData>
#include <QPainter>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QtConcurrent>

#include <DSvgRenderer>

#include <views/dfilemanagerwindow.h>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

#define SIDEBAR_ITEM_HEIGHT 30
#define SIDEBAR_ITEM_PADDING 13
#define SIDEBAR_ICON_SIZE 16
#define SIDEBAR_ICON_TEXT_GAP_SIZE 8
#define SIDEBAR_CHECK_BORDER_SIZE 3
#define SIDEBAR_RENAMEEDIT_MARGIN 2

class DFMSideBarItemPrivate
{
    Q_DECLARE_PUBLIC(DFMSideBarItem)

public:
    DFMSideBarItemPrivate(DFMSideBarItem *qq);
    void init();
    QPixmap icon() const;
    QPixmap icon(ThemeConfig::State state) const;
    QPixmap reorderLine() const;
    ThemeConfig::State getState() const;

    void hideRenameEditor();
    void cdIfAutoOpenUrlOnClick();

    bool canDeleteViaDrag = false;
    bool reorderable = false;
    bool readOnly = true;
    bool itemVisible = true;

    DUrl url;
    QFont font;
    QWidget *contentWidget = nullptr;
    QLineEdit *renameLineEdit = nullptr;
    QHBoxLayout *contentLayout = nullptr;
    QSequentialAnimationGroup scaleAnimation;

    DFMSideBarItem *q_ptr = nullptr;

private:
    QString groupName = QString();
    QString displayText = "Placeholder";
    QString iconGroup, iconKey; // use `icon()` and you'll get proper QPixmap for drawing.

    bool pressed = false;
    bool checked = false;
    bool hovered = false;
    bool kbdFocused = false;
    bool autoOpenUrlOnClick = true;
};

DFMSideBarItemPrivate::DFMSideBarItemPrivate(DFMSideBarItem *qq)
    : q_ptr(qq)
{

}

void DFMSideBarItemPrivate::init()
{
    Q_Q(DFMSideBarItem);

    q->setFocusPolicy(Qt::TabFocus);
    q->setAcceptDrops(true);
    q->setMinimumSize(DFMSideBarItem::minimumWidth, SIDEBAR_ITEM_HEIGHT);
    q->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    q->setIconFromThemeConfig("BookmarkItem.BookMarks", "icon"); // Default icon

    // layout for contentWidget
    contentLayout = new QHBoxLayout(q);
    contentLayout->setAlignment(Qt::AlignVCenter);
    contentLayout->setAlignment(Qt::AlignRight);
    contentLayout->setContentsMargins(0, 0, SIDEBAR_ITEM_PADDING, 0);

    // leave url a default display name.
    DAbstractFileInfoPointer file_info = DFileService::instance()->createFileInfo(q, url);

    if (file_info) {
        displayText = file_info->fileDisplayName();
    }

    // Scale animation.
    QVariantAnimation *scaleBegin = new QVariantAnimation();
    QVariantAnimation *scaleEnd = new QVariantAnimation();

    scaleBegin->setDuration(150);
    scaleBegin->setEasingCurve(QEasingCurve::OutQuad);
    scaleBegin->setStartValue(QVariant(1.0f));
    scaleBegin->setEndValue(QVariant(1.13f));

    scaleEnd->setDuration(150);
    scaleEnd->setEasingCurve(QEasingCurve::OutQuad);
    scaleEnd->setStartValue(QVariant(1.13f));
    scaleEnd->setEndValue(QVariant(1.0f));

    scaleAnimation.addAnimation(scaleBegin);
    scaleAnimation.addAnimation(scaleEnd);

    q->connect(scaleBegin, &QVariantAnimation::valueChanged,
               q, static_cast<void(DFMSideBarItem::*)()>(&DFMSideBarItem::update));
    q->connect(scaleEnd, &QVariantAnimation::valueChanged,
               q, static_cast<void(DFMSideBarItem::*)()>(&DFMSideBarItem::update));

}

QPixmap DFMSideBarItemPrivate::icon() const
{
    return ThemeConfig::instace()->pixmap(iconGroup, iconKey, getState());
}

QPixmap DFMSideBarItemPrivate::icon(ThemeConfig::State state) const
{
    return ThemeConfig::instace()->pixmap(iconGroup, iconKey, state);
}

QPixmap DFMSideBarItemPrivate::reorderLine() const
{
    DSvgRenderer renderer;

    renderer.load(QStringLiteral(":/icons/images/icons/reordering_line.svg"));

    QPainter painter;
    QImage image = QImage(200,
                          renderer.defaultSize().height() * 200.0 / renderer.defaultSize().width(),
                          QImage::Format_ARGB32);

    image.fill(Qt::transparent);
    painter.begin(&image);
    renderer.render(&painter, QRect(QPoint(0, 0), image.size()));
    painter.end();

    return QPixmap::fromImage(image);
}

ThemeConfig::State DFMSideBarItemPrivate::getState() const
{
    if (checked) {
        return ThemeConfig::Checked;
    }

    if (pressed) {
        return ThemeConfig::Pressed;
    }

    if (hovered) {
        return ThemeConfig::Hover;
    }

    if (kbdFocused) {
        return ThemeConfig::Focus;
    }

    return ThemeConfig::Normal;
}

void DFMSideBarItemPrivate::hideRenameEditor()
{
    if (renameLineEdit) {
        renameLineEdit->hide();
        renameLineEdit->deleteLater();
        renameLineEdit = nullptr;
    }
}

void DFMSideBarItemPrivate::cdIfAutoOpenUrlOnClick()
{
    Q_Q(DFMSideBarItem);

    if (autoOpenUrlOnClick) {
        DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(q->topLevelWidget());
        wnd->cd(q->url()); // don't `setChecked` here, wait for a signal.
    }
}

/*!
 * \class DFMSideBarItem
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMSideBarItem is the item inside DFMSideBar
 *
 * DFMSideBarItem is the base class for the sidebar items. All type of items inside
 * the sidebar is derived from DFMSideBarItem. DFMSideBarItem provide the interfaces
 * to manage the item state.
 *
 * \sa DFMSideBar
 */

DFMSideBarItem::DFMSideBarItem(const DUrl &url, QWidget *parent)
    : QWidget(parent)
    , d_ptr(new DFMSideBarItemPrivate(this))
{
    d_func()->url = url;

    d_func()->init();
}

DFMSideBarItem::~DFMSideBarItem()
{

}

const DUrl DFMSideBarItem::url() const
{
    Q_D(const DFMSideBarItem);

    return d->url;
}

bool DFMSideBarItem::reorderable() const
{
    Q_D(const DFMSideBarItem);

    return d->reorderable;
}

bool DFMSideBarItem::canDeleteViaDrag() const
{
    Q_D(const DFMSideBarItem);

    return d->canDeleteViaDrag;
}

bool DFMSideBarItem::readOnly() const
{
    Q_D(const DFMSideBarItem);

    return d->readOnly;
}

bool DFMSideBarItem::checked() const
{
    Q_D(const DFMSideBarItem);

    return d->checked;
}

bool DFMSideBarItem::autoOpenUrlOnClick() const
{
    Q_D(const DFMSideBarItem);

    return d->autoOpenUrlOnClick;
}

bool DFMSideBarItem::itemVisible() const
{
    Q_D(const DFMSideBarItem);

    return d->itemVisible;
}

QString DFMSideBarItem::groupName() const
{
    Q_D(const DFMSideBarItem);

    return d->groupName;
}

QString DFMSideBarItem::text() const
{
    Q_D(const DFMSideBarItem);

    return d->displayText;
}

void DFMSideBarItem::showRenameEditor()
{
    Q_D(DFMSideBarItem);

    if (d->renameLineEdit) {
        return;
    }

    int paddingLeft = SIDEBAR_ITEM_PADDING + SIDEBAR_ICON_SIZE + SIDEBAR_ICON_TEXT_GAP_SIZE;

    d->renameLineEdit = new QLineEdit(this);
    d->renameLineEdit->resize(width() - paddingLeft - SIDEBAR_RENAMEEDIT_MARGIN,
                              SIDEBAR_ITEM_HEIGHT - SIDEBAR_RENAMEEDIT_MARGIN * 2);
    d->renameLineEdit->move(paddingLeft, SIDEBAR_RENAMEEDIT_MARGIN);
    d->renameLineEdit->show();
    d->renameLineEdit->selectAll();
    d->renameLineEdit->setFocus(Qt::MouseFocusReason);

    const DAbstractFileInfoPointer infoPointer = fileService->createFileInfo(this, url());

    d->renameLineEdit->setText(infoPointer->fileName());

    connect(d->renameLineEdit, &QLineEdit::editingFinished,
            this, &DFMSideBarItem::onEditingFinished);
}

/*!
 * \return the sorting priority of this item. The smaller the value,
 *         the higher the priority.
 */
int DFMSideBarItem::sortingPriority() const
{
    return 0;
}

void DFMSideBarItem::setContentWidget(QWidget *widget)
{
    Q_D(DFMSideBarItem);

    d->contentLayout->addWidget(widget);
    d->contentWidget = widget;
}

QWidget *DFMSideBarItem::contentWidget() const
{
    Q_D(const DFMSideBarItem);

    return d->contentWidget;
}

void DFMSideBarItem::setIconFromThemeConfig(const QString &group, const QString &key)
{
    Q_D(DFMSideBarItem);

    d->iconGroup = group;
    d->iconKey = key;

    // Do widget UI update.
    update();
}

void DFMSideBarItem::setReorderable(bool reorderable)
{
    Q_D(DFMSideBarItem);

    d->reorderable = reorderable;
}

void DFMSideBarItem::setCanDeleteViaDrag(bool canDeleteViaDrag)
{
    Q_D(DFMSideBarItem);

    d->canDeleteViaDrag = canDeleteViaDrag;
}

void DFMSideBarItem::setReadOnly(bool readOnly)
{
    Q_D(DFMSideBarItem);

    d->readOnly = readOnly;
}

void DFMSideBarItem::setChecked(bool checked)
{
    Q_D(DFMSideBarItem);

    d->checked = checked;

    // Do widget UI update.
    update();
}

/*!
 * \brief Set the group name which the item is belones to.
 * \param groupName the group name.
 *
 * Notice: Don't manualy set the group name by hand. It should managed by
 * a `DFMSideBarItemGroup`.
 */
void DFMSideBarItem::setGroupName(QString groupName)
{
    Q_D(DFMSideBarItem);

    d->groupName = groupName;
}

void DFMSideBarItem::setText(QString text)
{
    Q_D(DFMSideBarItem);

    d->displayText = text;

    // Do widget UI update.
    update();
}

/*!
 * \brief Disable auto change directory (`cd`) on sidebar item is clicked
 * \param autoCd
 *
 * The `DFMSideBarItem`'s default click behavior is change directory to
 * `url()` path. If you are going to write a custom item and don't want
 * the default behavior, then set it to false and handle `clicked()` signal
 * by yourself.
 *
 * \sa url(), clicked()
 */
void DFMSideBarItem::setAutoOpenUrlOnClick(bool autoCd)
{
    Q_D(DFMSideBarItem);

    d->autoOpenUrlOnClick = autoCd;
}

void DFMSideBarItem::onEditingFinished()
{
    Q_D(DFMSideBarItem);

    Q_CHECK_PTR(d->renameLineEdit);
    QString text = d->renameLineEdit->text();

    QSignalBlocker blocker(d->renameLineEdit);
    Q_UNUSED(blocker)

    d->hideRenameEditor();

    if (text == d->displayText) {
        return;
    }

    DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(nullptr, url());
    DUrl tmpUrl = infoPointer->getUrlByNewFileName(text);

    fileService->renameFile(this, url(), tmpUrl, true);

    emit renameFinished(text);
}

void DFMSideBarItem::playAnimation()
{
    Q_D(DFMSideBarItem);

    d->scaleAnimation.start();
}

void DFMSideBarItem::setUrl(DUrl url)
{
    Q_D(DFMSideBarItem);

    if (d->url == url) {
        return;
    }

    d->url = url;

    DAbstractFileInfoPointer file_info = DFileService::instance()->createFileInfo(this, url);

    if (file_info) {
        d->displayText = file_info->fileDisplayName();
        update();
    }

    emit urlChanged(url);
}

QMenu *DFMSideBarItem::createStandardContextMenu() const
{
    QMenu *menu = new QMenu();

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());

    menu->addAction(QObject::tr("Open in new window"), [this]() {
        WindowManager::instance()->showNewWindow(url(), true);
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, this]() {
        wnd->openNewTab(url());
    })->setDisabled(shouldDisable);

    menu->addAction(QObject::tr("Properties"), [this]() {
        DUrlList list;
        list.append(url());
        fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(this, list));
    });

    return menu;
}

Qt::DropAction DFMSideBarItem::canDropMimeData(const QMimeData *data, Qt::DropActions actions) const
{
    // Got a copy of urls so whatever data was changed, it won't affact the following code.
    QList<QUrl> urls = data->urls();

    if (urls.empty()) {
        return Qt::IgnoreAction;
    }

    for (const DUrl &url : urls) {
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(this, url);
        if (!fileInfo || !fileInfo->isReadable()) {
            return Qt::IgnoreAction;
        }
    }

    const DAbstractFileInfoPointer &info = fileService->createFileInfo(this, url());

    if (!info || !info->canDrop()) {
        return Qt::IgnoreAction;
    }

    const Qt::DropActions support_actions = info->supportedDropActions() & actions;

    if (DStorageInfo::inSameDevice(urls.first(), url())) {
        if (support_actions.testFlag(Qt::MoveAction)) {
            return Qt::MoveAction;
        }
    }

    if (support_actions.testFlag(Qt::CopyAction)) {
        return Qt::CopyAction;
    }

    if (support_actions.testFlag(Qt::MoveAction)) {
        return Qt::MoveAction;
    }

    if (support_actions.testFlag(Qt::LinkAction)) {
        return Qt::LinkAction;
    }

    return Qt::IgnoreAction;
}

bool DFMSideBarItem::dropMimeData(const QMimeData *data, Qt::DropAction action) const
{
    Q_D(const DFMSideBarItem);

    DUrl destUrl = url();
    DUrlList oriUrlList = DUrl::fromQUrlList(data->urls());
    const DAbstractFileInfoPointer &destInfo = fileService->createFileInfo(this, destUrl);

    for (DUrl &u : oriUrlList) {
        // we only do redirection for burn:// urls for the fear of screwing everything up again
        if (u.scheme() == BURN_SCHEME) {
            for (DAbstractFileInfoPointer fi = fileService->createFileInfo(nullptr, u); fi->canRedirectionFileUrl(); fi = fileService->createFileInfo(nullptr, u)) {
                u = fi->redirectedFileUrl();
            }
        }
    }

    // convert destnation url to real path if it's a symbol link.
    if (destInfo->isSymLink()) {
        destUrl = destInfo->rootSymLinkTarget();
    }

    switch (action) {
    case Qt::CopyAction:
        // blumia: should run in another thread or user won't do another DnD opreation unless the copy action done.
        QtConcurrent::run([=](){
            fileService->pasteFile(this, DFMGlobal::CopyAction, destUrl, oriUrlList);
        });
        break;
    case Qt::LinkAction:
        break;
    case Qt::MoveAction:
        fileService->pasteFile(this, DFMGlobal::CutAction, destUrl, oriUrlList);
        break;
    default:
        return false;
    }

    return true;
}

void DFMSideBarItem::dragEnterEvent(QDragEnterEvent *event)
{
    Q_D(DFMSideBarItem);

    Qt::DropAction action = canDropMimeData(event->mimeData(), event->proposedAction());

    if (action == Qt::IgnoreAction) {
        action = canDropMimeData(event->mimeData(), event->possibleActions());
    }

    if (action != Qt::IgnoreAction) {
        event->setDropAction(action);
        event->accept();
        d->hovered = true;
        update();
        return;
    }

    if (event->source() == this) {
        return;
    }

    if (DFMSideBarItem *item = qobject_cast<DFMSideBarItem *>(event->source())) {
        if (d->reorderable && this->groupName() == item->groupName()) {
            event->acceptProposedAction();
        }
    }

    if (event->isAccepted()) {
        d->hovered = true;
        update();
    }
}

void DFMSideBarItem::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    Q_D(DFMSideBarItem);

    d->hovered = false;
    update();
}

void DFMSideBarItem::dropEvent(QDropEvent *event)
{
    Q_D(DFMSideBarItem);

    // do it first to avoid unexpect return.
    d->hovered = false;
    update();

    Qt::DropAction action = canDropMimeData(event->mimeData(), event->proposedAction());

    // for robust, we should filter the drop action and only process *file* drop action here.
    // other drop event like DFMSideBarItem reorder and etc should be handle after `dropMimeData()`
    // so here we should simply check `canDropMimeData()` again.
    if (action == Qt::IgnoreAction) {
        action = canDropMimeData(event->mimeData(), event->possibleActions());
    }

    if (action != Qt::IgnoreAction) {
        if (dropMimeData(event->mimeData(), action)) {
            return;
        }
    }

    // If drop a sidebar item:
    if (DFMSideBarItem *item = qobject_cast<DFMSideBarItem *>(event->source())) {
        // do position change
        emit reorder(item, this, event->pos().y() < (SIDEBAR_ITEM_HEIGHT / 2));
    }
}

void DFMSideBarItem::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    Q_D(DFMSideBarItem);

    d->pressed = false; // clear click state
    d->hovered = true;
    update();
}

void DFMSideBarItem::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    Q_D(DFMSideBarItem);

    d->pressed = false; // only able to press when mouse is on this widget
    d->hovered = false;
    update();
}

void DFMSideBarItem::contextMenuEvent(QContextMenuEvent *event)
{
    DFMSideBar *sb = qobject_cast<DFMSideBar*>(this->parentWidget()->parentWidget()->parentWidget());
    if (!sb || !sb->contextMenuEnabled()) {
        return;
    }

    QMenu *menu = createStandardContextMenu();
    menu->exec(event->globalPos());
    menu->deleteLater();
}

void DFMSideBarItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(DFMSideBarItem);

    if (d->pressed && reorderable()) {
        QDrag *drag = new QDrag(this);
        Qt::DropAction dropAction;
        QMimeData *mimeData = new QMimeData;

        drag->setPixmap(d->reorderLine());
        drag->setHotSpot(QPoint(event->x(), 4));
        drag->setMimeData(mimeData);
        dropAction = drag->exec();
        drag->deleteLater();

        emit itemDragReleased(QCursor::pos(), dropAction);
    }

    return;
}

void DFMSideBarItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(DFMSideBarItem);

    if (event->button() == Qt::MouseButton::LeftButton) {
        d->pressed = true;
    }

    update();
    return QWidget::mousePressEvent(event);
}

void DFMSideBarItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(DFMSideBarItem);

    bool mouseStillOnWidget = rect().contains(event->localPos().toPoint());
    d->pressed = false;

    if (event->button() == Qt::MouseButton::LeftButton && mouseStillOnWidget) {
        emit clicked();
        // mouse click: cd to `url()`
        d->cdIfAutoOpenUrlOnClick();
    }

    update();
    return QWidget::mouseReleaseEvent(event);
}

void DFMSideBarItem::paintEvent(QPaintEvent *event)
{
    Q_D(const DFMSideBarItem);
    Q_UNUSED(event);

    QPainter painter(this);
    ThemeConfig::State curState = d->getState();

    bool animationPlaying = d->scaleAnimation.state() == QAbstractAnimation::Running;
    QVariantAnimation *p = static_cast<QVariantAnimation *>(d->scaleAnimation.currentAnimation());
    float curValue = p->currentValue().toFloat();

    // Const variables
    const int textPaddingLeft = SIDEBAR_ITEM_PADDING + SIDEBAR_ICON_SIZE + SIDEBAR_ICON_TEXT_GAP_SIZE;
    int iconPaddingTop = (height() - SIDEBAR_ICON_SIZE) / 2;
    QColor backgroundColor, textPenColor, iconBrushColor;
    QRect iconRect(SIDEBAR_ITEM_PADDING, iconPaddingTop, SIDEBAR_ICON_SIZE, SIDEBAR_ICON_SIZE);
    QRect textRect(textPaddingLeft, 0, width() - textPaddingLeft, height());

    backgroundColor = ThemeConfig::instace()->color("BookmarkItem", "background", curState);
    textPenColor = ThemeConfig::instace()->color("BookmarkItem", "color", curState);
    iconBrushColor = ThemeConfig::instace()->color("BookmarkItem", "color", curState); // what color?

    // Draw Background
    painter.fillRect(rect(), QBrush(backgroundColor));

    // Begin: Icon and text will be scaled when scale animation is playing
    if (animationPlaying) {
        painter.scale(curValue, curValue);
        iconRect.adjust(0, (curValue - 1.0f) * height() / -2.0f, 0, 0);
        textRect.adjust(0, (curValue - 1.0f) * height() / -2.0f, 0, 0);
    }

    // Draw Icon
    painter.drawPixmap(iconRect, curState == ThemeConfig::Pressed ? d->icon(ThemeConfig::Checked) : d->icon());

    // Draw Text (if no renameLineEdit is shown)
    if (!d->renameLineEdit) {
        painter.setPen(textPenColor);
        QFontMetrics metrics(d->font);
        int contentWidgetWidth = d->contentWidget && d->contentWidget->isVisible() ? d->contentWidget->width() + SIDEBAR_ITEM_PADDING : 0;
        QString elidedText = metrics.elidedText(d->displayText, Qt::ElideRight, textRect.width() - contentWidgetWidth - SIDEBAR_ITEM_PADDING);
        painter.drawText(textRect, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignVCenter, elidedText);
    }

    // End: Icon and text will be scaled when scale animation is playing
    painter.scale(1, 1);

    // Right border line
    if (curState == ThemeConfig::Checked) {
        painter.setBrush(iconBrushColor);
        painter.drawRect(width() - SIDEBAR_CHECK_BORDER_SIZE, 0, SIDEBAR_CHECK_BORDER_SIZE, height());
    }
}

void DFMSideBarItem::focusInEvent(QFocusEvent *event)
{
    Q_D(DFMSideBarItem);

    switch (event->reason()) {
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
        d->kbdFocused = true;
        update();
        break;
    default:
        break;
    }

    QWidget::focusInEvent(event);
}

void DFMSideBarItem::focusOutEvent(QFocusEvent *event)
{
    Q_D(DFMSideBarItem);

    d->kbdFocused = false;
    update();

    QWidget::focusOutEvent(event);
}

void DFMSideBarItem::setVisible(bool visible)
{
    Q_D(DFMSideBarItem);

    d->itemVisible = visible;
    QWidget::setVisible(visible);
}

bool DFMSideBarItem::event(QEvent *event)
{
    Q_D(DFMSideBarItem);

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        switch (keyEvent->key()) {
        case Qt::Key_Escape:
            if (!d->renameLineEdit) {
                keyEvent->accept();
                d->hideRenameEditor();
            }
            return true;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (!d->renameLineEdit) {
                emit clicked();
                d->cdIfAutoOpenUrlOnClick();
            }
            return true;
        default:
            break;
        }
    }

    return QWidget::event(event);
}

DFM_END_NAMESPACE
