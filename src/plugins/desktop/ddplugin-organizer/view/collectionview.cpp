/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "collectionview_p.h"
#include "dfm-base/utils/windowutils.h"
#include "models/fileproxymodel.h"
#include "delegate/collectionitemdelegate.h"

#include "dfm-base/utils/windowutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/base/standardpaths.h"

#include <DApplication>
#include <DFileDragClient>

#include <QScrollBar>
#include <QUrl>
#include <QDebug>
#include <QPainter>
#include <QDrag>
#include <QMimeData>

DDP_ORGANIZER_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

CollectionViewPrivate::CollectionViewPrivate(CollectionView *qq)
    : q(qq)
{
    touchDragTimer.setSingleShot(true);
    touchDragTimer.setTimerType(Qt::PreciseTimer);

    initUI();
}

void CollectionViewPrivate::initUI()
{
    q->setRootIndex(q->model()->rootIndex());
    q->setAttribute(Qt::WA_TranslucentBackground);

    q->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    q->viewport()->setAutoFillBackground(false);

    q->setFrameShape(QFrame::NoFrame);
    q->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

    q->setDragDropOverwriteMode(false);
    q->setDragDropMode(QAbstractItemView::DragDrop);
    q->setDefaultDropAction(Qt::CopyAction);

    auto delegate = new CollectionItemDelegate(q);
    q->setItemDelegate(delegate);

    // todo:disble selection???
}

void CollectionViewPrivate::updateRegionView()
{
    q->itemDelegate()->updateItemSizeHint();
    auto itemSize = q->itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());

    updateViewSizeData(q->geometry().size(), QMargins(0, 0, 0, 0), itemSize);
}

QList<QRect> CollectionViewPrivate::itemPaintGeomertys(const QModelIndex &index) const
{
    if (Q_UNLIKELY(!index.isValid()))
        return {};

    QStyleOptionViewItem option = q->viewOptions();
    option.rect = itemRect(index);
    return q->itemDelegate()->paintGeomertys(option, index);
}

QRect CollectionViewPrivate::itemRect(const QModelIndex &index) const
{
    return q->visualRect(index).marginsRemoved(cellMargins);
}

QRect CollectionViewPrivate::visualRect(const QPoint &pos) const
{
    const QPoint &&point = posToPoint(pos);

    QRect rect(point.x(), point.y(), cellWidth, cellHeight);
    rect.moveLeft(rect.left() - q->horizontalOffset());
    rect.moveTop(rect.top() - q->verticalOffset());

    return rect;
}

void CollectionViewPrivate::updateViewSizeData(const QSize &viewSize, const QMargins &viewMargins, const QSize &itemSize)
{
    // 固定行高与间隔边距,无行数限制
    // top margin is for icon top spacing
    // todo:define ICON_TOP_SPACE_DESKTOP 2
    static const QMargins minMargin(0, 2, 0, 0);
    int minCellWidth = itemSize.width() + minMargin.left() + minMargin.right();
    updateColumnCount(viewSize.width(), minCellWidth);

    int minCellHeight = itemSize.height() + minMargin.top() + minMargin.bottom();
    updateRowCount(viewSize.height(), minCellHeight);

    updateCellMargins(itemSize, QSize(cellWidth, cellHeight));

    updateViewMargins(viewSize, viewMargins);
}

void CollectionViewPrivate::updateVerticalBarRange()
{
    needUpdateVerticalBarRange = false;

    int dataRow = urls.count() / columnCount;
    if (0 != urls.count() % columnCount)
        dataRow += 1;

    int height = dataRow * cellHeight + viewMargins.top() + viewMargins.bottom() - q->viewport()->height();

    q->verticalScrollBar()->setRange(0, height);
    qDebug() << "update vertical scrollbar range to:" << q->verticalScrollBar()->maximum();
}

int CollectionViewPrivate::verticalScrollToValue(const QModelIndex &index, const QRect &rect, QAbstractItemView::ScrollHint hint) const
{
    Q_UNUSED(index)

    const QRect &&area = q->viewport()->rect();
    const bool above = (QAbstractItemView::EnsureVisible == hint && rect.top() < area.top());
    const bool below = (QAbstractItemView::EnsureVisible == hint && rect.bottom() > area.bottom());

    int verticalValue = q->verticalScrollBar()->value();
    QRect adjusted = rect.adjusted(-space, -space, space, space);
    if (QAbstractItemView::PositionAtTop == hint || above) {
        verticalValue += adjusted.top();
    } else if (QAbstractItemView::PositionAtBottom == hint || below) {
        verticalValue += qMin(adjusted.top(), adjusted.bottom() - area.height() + 1);
    } else if (QAbstractItemView::PositionAtCenter == hint) {
        verticalValue += adjusted.top() - ((area.height() - adjusted.height()) / 2);
    }

    return verticalValue;
}

QItemSelection CollectionViewPrivate::selection(const QRect &rect) const
{
    QItemSelection selection;
    const QRect actualRect(qMin(rect.left(), rect.right())
                           , qMin(rect.top(), rect.bottom()) + q->verticalOffset()
                           , abs(rect.width())
                           , abs(rect.height()));
    const QPoint offset(-q->horizontalOffset(), 0);
    const QPoint iconOffset(10, 10);    // todo(wangcl):define

    for (auto url : urls) {
        auto index = q->model()->index(url);
        const QRect &&itemRect = q->visualRect(index);
        QRect realItemRect(itemRect.topLeft() + offset + iconOffset, itemRect.bottomRight() + offset - iconOffset);

        // least 3 pixels
        static const int diff = 3;
        if (actualRect.left() > realItemRect.right() - diff
                || actualRect.top() > realItemRect.bottom() - diff
                || actualRect.right() < realItemRect.left() + diff
                || actualRect.bottom() < realItemRect.top() + diff)
            continue;

        if (!selection.contains(index)) {
            selection.push_back(QItemSelectionRange(index));
        }
    }

    return selection;
}

void CollectionViewPrivate::selectItems(const QList<QUrl> &fileUrl) const
{
    //fileUrl is file:///xxxx
    QItemSelection selection;
    for (const QUrl &url : fileUrl) {
        auto desktopUrl = url.toString();
        auto index = q->model()->index(desktopUrl);
        QItemSelectionRange selectionRange(index);
        if (!selection.contains(index)) {
            selection.push_back(selectionRange);
        }
    }

    if (!selection.isEmpty()) {
        q->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);

        // update new focus index.
        auto lastIndex = q->selectionModel()->selectedIndexes().last();
        q->setCurrentIndex(lastIndex);
    }
}

QPoint CollectionViewPrivate::pointToPos(const QPoint &point) const
{
    int column = (point.x() - viewMargins.left()) / cellWidth;
    int row = (point.y() - viewMargins.top()) / cellHeight;
    return QPoint(row, column);
}

QPoint CollectionViewPrivate::posToPoint(const QPoint &pos) const
{
    int pointX = pos.y() * cellWidth + viewMargins.left();
    int pointY = pos.x() * cellHeight + viewMargins.top();
    return QPoint(pointX, pointY);
}

int CollectionViewPrivate::posToNode(const QPoint &pos) const
{
    return pos.x() * columnCount + pos.y();
}

QPoint CollectionViewPrivate::nodeToPos(const int node) const
{
    return QPoint(node / columnCount, node % columnCount);
}

void CollectionViewPrivate::checkTouchDarg(QMouseEvent *event)
{
    if (!event)
        return;

    // delay 200ms to enable start drag on touch screen.
    // When the event source is mouseeventsynthesizedbyqt, it is considered that this event is converted from touchbegin
    if ((event->source() == Qt::MouseEventSynthesizedByQt) && (event->button() == Qt::LeftButton)) {
        // Read the pressing duration of DDE configuration
        QObject *themeSettings = reinterpret_cast<QObject *>(qvariant_cast<quintptr>(qApp->property("_d_theme_settings_object")));
        QVariant touchFlickBeginMoveDelay;
        if (themeSettings)
            touchFlickBeginMoveDelay = themeSettings->property("touchFlickBeginMoveDelay");

        touchDragTimer.setInterval(touchFlickBeginMoveDelay.isValid() ? touchFlickBeginMoveDelay.toInt() : 200);
        touchDragTimer.start();
    } else {
        touchDragTimer.stop();
    }
}

bool CollectionViewPrivate::isDelayDrag() const
{
    return touchDragTimer.isActive();
}

QPixmap CollectionViewPrivate::polymerizePixmap(QModelIndexList indexs) const
{
    if (indexs.isEmpty())
        return QPixmap();

    // get foucs item to set it on top.
    auto foucs = q->currentIndex();
    if (!foucs.isValid()) {
        qWarning() << "current index is invalid.";
        foucs = indexs.first();
    } else if (!indexs.contains(foucs)) {
        qWarning() << "current index is not in indexs.";
        foucs = indexs.first();
    }
    const int indexCount = indexs.count();
    // remove focus which will paint on top
    indexs.removeAll(foucs);

    static const int iconWidth = 128;
    static const int iconMargin = 30;    // add margin for showing ratoted item.
    static const int maxIconCount = 4;   // max painting item number.
    static const int maxTextCount = 99;  // max text number.
    static const qreal rotateBase = 10.0;
    static const qreal opacityBase = 0.1;
    static const int rectSzie = iconWidth + iconMargin * 2;
    const qreal scale = q->devicePixelRatioF();

    QRect pixRect(0, 0, rectSzie, rectSzie);
    QPixmap pixmap(pixRect.size() * scale);
    pixmap.setDevicePixelRatio(scale);
    pixmap.fill(Qt::transparent);

    const qreal offsetX = pixRect.width() / 2;
    const qreal offsetY = pixRect.height() / 2;
    const QSize iconSize(iconWidth, iconWidth);

    QStyleOptionViewItem option = q->viewOptions();
    option.state |= QStyle::State_Selected;
    // icon rect in pixmap.
    option.rect = pixRect.translated(iconMargin, iconMargin);
    option.rect.setSize(iconSize);

    QPainter painter(&pixmap);
    // paint items except focus
    for (int i = qMin(maxIconCount - 1, indexs.count() - 1); i >= 0 ; --i) {
        painter.save();

        //opacity 50% 40% 30% 20%
        painter.setOpacity(1.0 - (i + 5) * opacityBase);

        //rotate
        {
            qreal rotate = rotateBase * (qRound((i + 1.0) / 2.0) / 2.0 + 1.0) * (i % 2 == 1 ? -1 : 1);
            auto tf = painter.transform();

            // rotate on center
            tf = tf.translate(offsetX, offsetY).rotate(rotate).translate(-offsetX, -offsetY);
            painter.setTransform(tf);
        }

        //paint icon
        q->itemDelegate()->paintDragIcon(&painter, option, indexs.at(i));

        painter.restore();
    }

    // paint focus
    QSize topIconSize;
    {
        painter.save();
        painter.setOpacity(0.8);
        topIconSize = q->itemDelegate()->paintDragIcon(&painter, option, foucs);
        painter.restore();;
    }

    // paint text
    {
        int length = 0;
        QString text;
        if (indexCount > maxTextCount) {
            length = 28; //there are three characters showed.
            text = QString::number(maxTextCount).append("+");
        } else {
            length = 24; // one or two characters
            text = QString::number(indexCount);
        }

        // the text rect is on right bottom of top icon.
        // using actual size of top icon to calc postion.
        int x = iconMargin + (iconWidth + topIconSize.width() - length) / 2;
        int y = iconMargin + (iconWidth + topIconSize.height() - length) / 2;
        QRect textRect(x, y, length, length);

        // paint text background.
        drawEllipseBackground(&painter, textRect);
        drawDragText(&painter, text, textRect);
    }

    return pixmap;
}

bool CollectionViewPrivate::checkClientMimeData(QDragEnterEvent *event) const
{
    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        // set target dir to source app.
        // after setting, app will copy it's items to target.
        // and this action is start before dropEvent instead in or after dropEvent.
        DFileDragClient::setTargetUrl(event->mimeData(), dropTargetUrl);
        event->setDropAction(Qt::CopyAction);
        return true;
    }

    return false;
}

bool CollectionViewPrivate::checkXdndDirectSave(QDragEnterEvent *event) const
{
    if (event->mimeData()->hasFormat("XdndDirectSave0")) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return true;
    }

    return false;
}

void CollectionViewPrivate::preproccessDropEvent(QDropEvent *event, const QUrl &targetUrl) const
{
    if (!event || event->mimeData()->urls().isEmpty())
        return;

    // in collection
    if (qobject_cast<CollectionView *>(event->source())) {
        auto action = WindowUtils::keyCtrlIsPressed() ? Qt::CopyAction : Qt::MoveAction;
        event->setDropAction(action);
        return;
    }

    QString errString;
    auto itemInfo =  InfoFactory::create<LocalFileInfo>(targetUrl, true, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        qWarning() << "create LocalFileInfo error: " << errString << targetUrl;
        return;
    }

    auto defaultAction = Qt::CopyAction;
    auto urls = event->mimeData()->urls();
    const QUrl from = urls.first();

    // using MoveAction if alt key is pressed.
    // CopyAction if ctrl key is pressed.
    if (WindowUtils::keyAltIsPressed()) {
        defaultAction = Qt::MoveAction;
    } else if (!WindowUtils::keyCtrlIsPressed()) {
        if (FileUtils::isSameDevice(targetUrl, from)) {
            defaultAction = Qt::MoveAction;
        }
    }

    // is from or to trash
    {
        bool isFromTrash = from.toLocalFile().startsWith(StandardPaths::location(StandardPaths::kTrashPath));
        bool isToTrash = false; // there is no trash dir on desktop

        if (Q_UNLIKELY(isFromTrash && isToTrash)) {
            event->setDropAction(Qt::IgnoreAction);
            return;
        } else if (isFromTrash || isToTrash) {
            defaultAction = Qt::MoveAction;
        }
    }

    const bool sameUser = SysInfoUtils::isSameUser(event->mimeData());
    if (event->possibleActions().testFlag(defaultAction))
        event->setDropAction((defaultAction == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : defaultAction);

    // todo,from vault???

    if (!itemInfo->supportedDropActions().testFlag(event->dropAction())) {
        QList<Qt::DropAction> actions{Qt::CopyAction, Qt::MoveAction, Qt::LinkAction};
        for (auto action : actions) {
            if (event->possibleActions().testFlag(action) && itemInfo->supportedDropActions().testFlag(action)) {
                event->setDropAction((action == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : action);
                break;
            }
        }
    }

    // todo,from recent???

    event->setDropAction(defaultAction);
}

void CollectionViewPrivate::handleMoveMimeData(QDropEvent *event, const QUrl &url)
{
    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        // update target url if mouse focus is on file which can drop.
        updateTarget(event->mimeData(), url);
        qWarning() << "drop by app " << dropTargetUrl;
    } else {
        event->accept();
    }
}

bool CollectionViewPrivate::dropFilter(QDropEvent *event)
{
    //Prevent the desktop's computer/recycle bin/home directory from being dragged and copied to other directories
    {
        QModelIndex index = q->indexAt(event->pos());
        if (index.isValid()) {
            QUrl targetItem = q->model()->fileUrl(index);
            QString errString;
            auto itemInfo =  InfoFactory::create<LocalFileInfo>(targetItem, true, &errString);
            if (Q_UNLIKELY(!itemInfo)) {
                qWarning() << "create LocalFileInfo error: " << errString << targetItem;
                return false;
            }
            if (itemInfo->isDir() || itemInfo->url() == DesktopAppUrl::homeDesktopFileUrl()) {
                auto sourceUrls = event->mimeData()->urls();
                for (const QUrl &url : sourceUrls) {
                    if ((DesktopAppUrl::computerDesktopFileUrl() == url)
                         || (DesktopAppUrl::trashDesktopFileUrl() == url)
                         || (DesktopAppUrl::homeDesktopFileUrl() == url)) {
                        event->setDropAction(Qt::IgnoreAction);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool CollectionViewPrivate::dropClientDownload(QDropEvent *event) const
{
    auto data = event->mimeData();
    if (DFileDragClient::checkMimeData(data)) {
        event->acceptProposedAction();
        qWarning() << "drop on" << dropTargetUrl;

        QList<QUrl> urlList = data->urls();
        if (!urlList.isEmpty()) {
            // follow canvas dropClientDownload
            DFileDragClient *client = new DFileDragClient(data, q);
            qDebug() << "dragClientDownload" << client << data << urlList;
            connect(client, &DFileDragClient::stateChanged, this, [this, urlList](DFileDragState state) {
                if (state == Finished)
                    selectItems(urlList);
                qDebug() << "stateChanged" << state << urlList;
            });

            connect(client, &DFileDragClient::serverDestroyed, client, &DFileDragClient::deleteLater);
            connect(client, &DFileDragClient::destroyed, []() {
                qDebug() << "drag client deleted";
            });
        }

        return true;
    }

    return false;
}

bool CollectionViewPrivate::dropDirectSaveMode(QDropEvent *event) const
{
    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        event->setDropAction(Qt::CopyAction);
        const QModelIndex &index = q->indexAt(event->pos());
        auto fileInfo = q->model()->fileInfo(index.isValid() ? index : q->rootIndex());

        if (fileInfo && fileInfo->url().isLocalFile()) {
            if (fileInfo->isDir())
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->url());
            else
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->parentUrl());
        }

        event->accept();   // yeah! we've done with XDS so stop Qt from further event propagation.
        return true;
    }

    return false;
}

bool CollectionViewPrivate::dropBetweenView(QDropEvent *event) const
{
    if (WindowUtils::keyCtrlIsPressed() || !event->mimeData()->property("DragFromDesktop").toBool())
        return false;

    auto dropPos = pointToPos(event->pos());
    auto dropRect = visualRect(dropPos);
    auto dropIndex = q->indexAt(dropRect.center());
    auto targetIndex = q->indexAt(event->pos());
    bool dropOnSelf = targetIndex.isValid() ? q->selectionModel()->selectedIndexes().contains(targetIndex) : false;

    // process this case in other drop function(e.g. move) if targetGridPos is used and it is not drop-needed.
    if (dropIndex.isValid() && !dropOnSelf) {
        if (!targetIndex.isValid()) {
            qInfo() << "drop on invaild target, skip. drop:" << dropPos.x() << dropPos.y();
            return true;
        }
        return false;
    }

    auto urls = event->mimeData()->urls();
    auto index = posToNode(dropPos);
    // todo 要求在index位置开始插入urls


    return true;
}

bool CollectionViewPrivate::dropMimeData(QDropEvent *event) const
{
    auto model = q->model();
    auto targetIndex = q->indexAt(event->pos());
    bool enableDrop = targetIndex.isValid() ? model->flags(targetIndex) & Qt::ItemIsDropEnabled : model->flags(model->rootIndex()) & Qt::ItemIsDropEnabled;
    if (model->supportedDropActions() & event->dropAction() && enableDrop) {
        preproccessDropEvent(event, targetIndex.isValid() ? model->fileUrl(targetIndex) : model->fileUrl(model->rootIndex()));
        const Qt::DropAction action = event->dropAction();
        if (model->dropMimeData(event->mimeData(), action, targetIndex.row(), targetIndex.column(), targetIndex)) {
            if (action != event->dropAction()) {
                event->setDropAction(action);
                event->accept();
            } else {
                event->acceptProposedAction();
            }
        }
        return true;
    }
    return false;
}

void CollectionViewPrivate::updateRowCount(const int &viewHeight, const int &minCellHeight)
{
    rowCount = viewHeight / minCellHeight;
    if (Q_UNLIKELY(rowCount < 1)) {
        qWarning() << "Row count is 0!Fix it to 1,and set cell height to:" << minCellHeight;
        cellHeight = minCellHeight;
        rowCount = 1;
    } else {
        cellHeight = viewHeight / rowCount;
    }

    if (Q_UNLIKELY(cellHeight < 1)) {
        qWarning() << "Cell height is:" << cellHeight << "!Fix it to 1";
        cellHeight = 1;
    }
}

void CollectionViewPrivate::updateColumnCount(const int &viewWidth, const int &minCellWidth)
{
    columnCount = viewWidth / minCellWidth;
    if (Q_UNLIKELY(columnCount < 1)) {
        qWarning() << "Column count is 0!Fix it to 1,and set cell width to:" << viewWidth;
        cellWidth = viewWidth;
        columnCount = 1;
    } else {
        cellWidth = viewWidth / columnCount;
    }

    if (Q_UNLIKELY(cellWidth < 1)) {
        qWarning() << "Cell width is:" << cellWidth << "!Fix it to 1";
        cellWidth = 1;
    }
}

void CollectionViewPrivate::updateCellMargins(const QSize &itemSize, const QSize &cellSize)
{
    // todo:UI不合理，待确认
    const int horizontalMargin = cellSize.width() - itemSize.width();
    const int verticalMargin = cellSize.height() - itemSize.height();
    const int leftMargin = horizontalMargin / 2;
    const int rightMargin = horizontalMargin - leftMargin;
    const int topMargin = verticalMargin / 2;
    const int bottomMargin = verticalMargin - topMargin;

    cellMargins = QMargins(leftMargin, topMargin, rightMargin, bottomMargin);
}

void CollectionViewPrivate::updateViewMargins(const QSize &viewSize, const QMargins &oldMargins)
{
    const int horizontalMargin = (viewSize.width() - cellWidth * columnCount);
    const int leftMargin = horizontalMargin / 2;
    const int rightMargin = horizontalMargin - leftMargin;
    const int topMargin = 0;
    const int bottomMargin = 0;

    viewMargins = oldMargins + QMargins(leftMargin, topMargin, rightMargin, bottomMargin);
}

void CollectionViewPrivate::drawDragText(QPainter *painter, const QString &str, const QRect &rect) const
{
    painter->save();
    painter->setPen(Qt::white);
    QFont ft(q->font());
    ft.setPixelSize(12);
    ft.setBold(true);
    painter->setFont(ft);
    painter->drawText(rect, Qt::AlignCenter, str);
    painter->restore();
}

void CollectionViewPrivate::drawEllipseBackground(QPainter *painter, const QRect &rect) const
{
    painter->save();
    QColor pointColor(244, 74, 74);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(1);
    painter->setPen(pointColor);
    painter->setBrush(pointColor);
    painter->drawEllipse(rect);
    painter->restore();
}

void CollectionViewPrivate::updateTarget(const QMimeData *data, const QUrl &url)
{
    if (url == dropTargetUrl)
        return;
    dropTargetUrl = url;
    DFileDragClient::setTargetUrl(data, dropTargetUrl);
}

CollectionView::CollectionView(QWidget *parent)
    : QAbstractItemView(parent)
    , d(new CollectionViewPrivate(this))
{

}

CollectionView::~CollectionView()
{

}

QList<QUrl> CollectionView::urls() const
{
    return d->urls;
}

void CollectionView::setUrls(const QList<QUrl> &urls)
{
    d->urls.clear();
    for (auto url : urls) {
        if (url.fileName().startsWith("."))
            continue;
        d->urls.append(url);
    }

    d->updateVerticalBarRange();
    update();
}

QMargins CollectionView::cellMargins() const
{
    return d->cellMargins;
}

FileProxyModel *CollectionView::model() const
{
    return qobject_cast<FileProxyModel *>(QAbstractItemView::model());
}

CollectionItemDelegate *CollectionView::itemDelegate() const
{
    return qobject_cast<CollectionItemDelegate *>(QAbstractItemView::itemDelegate());
}

QRect CollectionView::visualRect(const QModelIndex &index) const
{
    if (!index.isValid())
        return QRect();

    QUrl url = model()->fileUrl(index);
    if (!d->urls.contains(url))
        return QRect();

    int node = d->urls.indexOf(url);
    const QPoint &&pos = d->nodeToPos(node);

    return d->visualRect(pos);
}

void CollectionView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    const QRect &&rect = visualRect(index);
    if (!rect.isValid())
        return;

    if (EnsureVisible == hint && viewport()->rect().contains(rect)) {
        viewport()->update(rect);
        return;
    }

    // vertical
    verticalScrollBar()->setValue(d->verticalScrollToValue(index, rect, hint));

    // disable horizontal
}

QModelIndex CollectionView::indexAt(const QPoint &point) const
{
    auto checkRect = [](const QList<QRect> &listRect, const QPoint &point) -> bool {
        // icon rect
        if (listRect.size() > 0 && listRect.at(0).contains(point))
            return true;

        if (listRect.size() > 1) {
            QRect identify = listRect.at(1);
            if (identify.contains(point))
                return true;
        }
        return false;
    };

    QModelIndex rowIndex = currentIndex();
    // first check the editing item or the expended item.
    // the editing item and the expended item must be one item.
    if (rowIndex.isValid() && isPersistentEditorOpen(rowIndex)) {
        QList<QRect> identify;
        // editor area that the height is higher than visualRect.
        if (QWidget *editor = indexWidget(rowIndex))
            identify << editor->geometry();
        if (checkRect(identify, point)) {
            return rowIndex;
        }
    } else if (itemDelegate()->mayExpand(&rowIndex)) {   // second
        // get the expended rect.
        auto listRect = d->itemPaintGeomertys(rowIndex);
        if (checkRect(listRect, point)) {
            return rowIndex;
        }
    }

    // then check the item on the point.
    {
        int node = d->posToNode(d->pointToPos(point));
        if (node >= d->urls.count())
            return QModelIndex();
        rowIndex = model()->index(d->urls.at(node));
        if (!rowIndex.isValid())
            return rowIndex;

        auto listRect = d->itemPaintGeomertys(rowIndex);
        if (checkRect(listRect, point)) {
            return rowIndex;
        }
    }

    return QModelIndex();
}

QModelIndex CollectionView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    auto findAvailableRowBackward = [=](int row) {
        if (row >= 0)
            --row;
        return row;
    };

    auto findAvailableRowForward = [=](int row) {
        if (row < d->rowCount)
            ++row;
        return row;
    };

    QModelIndex current = currentIndex();
    if (!current.isValid()) {
        int row = findAvailableRowForward(0);   // select first
        if (-1 == row || d->urls.isEmpty())
            return QModelIndex();
        const QUrl &url = d->urls.first();
        return model()->index(url);
    }

    switch (cursorAction) {
    case MoveLeft : {
        // todo:通过current从model中获取url，再从gridManager中获取该url的上一个url，再从model中获取对应的index
        return QModelIndex();
    }
    case MoveRight : {
        // todo:通过current从model中获取url，再从gridManager中获取该url的下一个url，再从model中获取对应的index
        return QModelIndex();
    }
    case MoveUp :
    case MovePrevious : {
        // todo:通过current从model中获取url,再从gridManager中获取该url的网格位置,获取其上一行的位置处的url,再从model获取该url对应的index
        return QModelIndex();
    }
    case MoveDown :
    case MoveNext : {
        // todo:通过current从model中获取url,再从gridManager中获取该url的网格位置,获取其下一行的位置处的url,再从model获取该url对应的index
        return QModelIndex();
    }
    case MoveHome : {
        // todo:返回第一个
        return QModelIndex();
    }
    case MoveEnd : {
        // todo:返回最后一个
        return QModelIndex();
    }
    case MovePageUp : {
        // todo:返回上一页的第一个
        // todo:shift?范围选择
        return QModelIndex();
    }
    case MovePageDown : {
        // todo:返回下一页的最后一个
        // todo:shift?范围选择
        return QModelIndex();
    }
    }
}

int CollectionView::horizontalOffset() const
{
    if (Q_UNLIKELY(isRightToLeft()))
        return horizontalScrollBar()->maximum() - horizontalScrollBar()->value();

    return horizontalScrollBar()->value();
}

int CollectionView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

bool CollectionView::isIndexHidden(const QModelIndex &index) const
{
    Q_UNUSED(index)

    // disable hidden index
    return false;
}

void CollectionView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    if (!selectionModel())
        return;

    QItemSelection selection;

    if (rect.width() == 1 && rect.height() == 1) {
        QModelIndex currentIndex = indexAt(rect.center());
        if (currentIndex.isValid())
            selection.select(currentIndex, currentIndex);
    } else {
        auto realRect = rect.translated(horizontalOffset(), verticalOffset());
        selection = d->selection(realRect);
    }

    selectionModel()->select(selection, command);
}

QRegion CollectionView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;
    auto selectedList = selection.indexes();
    for (auto &index : selectedList)
        region = region.united(QRegion(visualRect(index)));

    return region;
}

void CollectionView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (Q_UNLIKELY(!itemDelegate()))
        return;

    auto option = viewOptions();
    QPainter painter(viewport());

    auto repaintRect = viewport()->geometry().translated(horizontalOffset(), verticalOffset());
    auto topLeft = repaintRect.topLeft();
    auto bottomRight = repaintRect.bottomRight();
    auto startPoint = d->pointToPos(topLeft);
    auto startNode = d->posToNode(startPoint);
    auto endPoint = d->pointToPos(bottomRight);
    auto endNode = d->posToNode(endPoint);

    if (d->showGrid) {
        painter.save();
        for (auto node = 0 ; node <= endNode - startNode; ++node) {
            auto pos = d->nodeToPos(node);
            auto point = d->posToPoint(pos);
            auto rect = QRect(point.x(), point.y(), d->cellWidth, d->cellHeight);

            auto rowMode = pos.x() % 2;
            auto colMode = pos.y() % 2;
            auto color = (colMode == rowMode) ? QColor(0, 0, 255, 32) : QColor(255, 0, 0, 32);
            painter.setPen(Qt::darkGray);
            painter.drawRect(rect);
            painter.fillRect(rect, color);

            auto itemSize = rect.marginsRemoved(d->cellMargins);
            painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
            painter.drawRect(itemSize);

            painter.setPen(QPen(Qt::red, 2));
            painter.drawText(rect, QString("%1-%2").arg(d->nodeToPos(node + startNode).x()).arg(d->nodeToPos(node + startNode).y()));
        }
        painter.restore();
    }

    for (auto node = startNode ; node <= endNode ; ++node) {
        if (node >= d->urls.count())
            break;

        auto url = d->urls.at(node);
        auto index = model()->index(url);
        option.rect = visualRect(index).marginsRemoved(d->cellMargins);
        painter.save();
        itemDelegate()->paint(&painter, option, index);
        painter.restore();
    }
}

void CollectionView::wheelEvent(QWheelEvent *event)
{
    int currentPosition = verticalScrollBar()->sliderPosition();
    int scrollValue = event->angleDelta().y();
    int targetValue = currentPosition - scrollValue;
    verticalScrollBar()->setSliderPosition(targetValue);
}

void CollectionView::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton)) {
        d->canUpdateVerticalBarRange = false;
    }

    d->checkTouchDarg(event);

    return QAbstractItemView::mousePressEvent(event);
}

void CollectionView::mouseReleaseEvent(QMouseEvent *event)
{
    d->canUpdateVerticalBarRange = true;
    if (d->needUpdateVerticalBarRange) {
        d->updateVerticalBarRange();
    }

    return QAbstractItemView::mouseReleaseEvent(event);
}

void CollectionView::mouseMoveEvent(QMouseEvent *event)
{
    return QAbstractItemView::mouseMoveEvent(event);
}

void CollectionView::resizeEvent(QResizeEvent *event)
{
    QAbstractItemView::resizeEvent(event);

    d->updateRegionView();

    if (d->canUpdateVerticalBarRange) {
        d->updateVerticalBarRange();
    } else {
        d->needUpdateVerticalBarRange = true;
    }
}

void CollectionView::startDrag(Qt::DropActions supportedActions)
{
    qDebug() << "view start drag:" << supportedActions;
    if (d->isDelayDrag())
        return;

    // close editor before drag.
    // normally, items in editing status do not enter startDrag.
    // but if drag and drope one item before editing it, then draging it, startDrag will be called.
    // the reason is that when one item droped, the d->pressedIndex is setted to invaild.
    // then in mousePressEvent d->pressedIndex is not updating to preesed index because the state is EditingState.
    // finally, in mouseMoveEvent state is changed from EditingState to DragSelectingState because d->pressedInde is invaild.
    if (isPersistentEditorOpen(currentIndex()))
        closePersistentEditor(currentIndex());

    QModelIndexList validIndexes = selectionModel()->selectedIndexes();
    if (validIndexes.count() > 1) {
        QMimeData *data = model()->mimeData(validIndexes);
        if (!data)
            return;
        data->setProperty("DragFromDesktop", true);

        QPixmap pixmap = d->polymerizePixmap(validIndexes);
        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->setHotSpot(QPoint(static_cast<int>(pixmap.size().width() / (2 * pixmap.devicePixelRatio())),
                                static_cast<int>(pixmap.size().height() / (2 * pixmap.devicePixelRatio()))));
        Qt::DropAction dropAction = Qt::IgnoreAction;
        Qt::DropAction defaultDropAction = QAbstractItemView::defaultDropAction();
        if (defaultDropAction != Qt::IgnoreAction && (supportedActions & defaultDropAction))
            dropAction = defaultDropAction;
        else if (supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove)
            dropAction = Qt::CopyAction;
        drag->exec(supportedActions, dropAction);
    } else {
        QAbstractItemView::startDrag(supportedActions);
    }
}

void CollectionView::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "view drag enter:" << event->pos() << event->mimeData() << event->dropAction() << event->possibleActions() << event->proposedAction();;

    d->dropTargetUrl = model()->fileUrl(model()->rootIndex());

    if (d->checkClientMimeData(event))
        return;

    if (d->checkXdndDirectSave(event))
        return;

    d->preproccessDropEvent(event, model()->fileUrl(model()->rootIndex()));

    return QAbstractItemView::dragEnterEvent(event);
}

void CollectionView::dragMoveEvent(QDragMoveEvent *event)
{
    qDebug() << "view drag move:" << event->pos() << event->mimeData() << event->dropAction() << event->possibleActions() << event->proposedAction();

    auto pos = event->pos();
    auto hoverIndex = indexAt(pos);
    auto currentUrl = hoverIndex.isValid() ? model()->fileUrl(hoverIndex) : model()->fileUrl(model()->rootIndex());
    if (hoverIndex.isValid()) {
        if (auto fileInfo = model()->fileInfo(hoverIndex)) {
            bool canDrop = !fileInfo->canDrop()
                           || (fileInfo->isDir() && !fileInfo->isWritable())
                           || !fileInfo->supportedDropActions().testFlag(event->dropAction());
            if (!canDrop) {
                d->handleMoveMimeData(event, currentUrl);
                return;
            } else {
                // not support drop
                event->ignore();
            }
        }
    }

    // hover
    d->preproccessDropEvent(event, currentUrl);
    if (!hoverIndex.isValid())
        d->handleMoveMimeData(event, currentUrl);
}

void CollectionView::dragLeaveEvent(QDragLeaveEvent *event)
{
    qDebug() << "view drag leave";

    d->dropTargetUrl.clear();

    return QAbstractItemView::dragLeaveEvent(event);
}

void CollectionView::dropEvent(QDropEvent *event)
{
    qDebug() << "view drop:" << event->pos() << event->mimeData() << event->dropAction() << event->possibleActions() << event->proposedAction();

    // some special case
    if (d->dropFilter(event))
        return;

    // copy file by other app
    if (d->dropClientDownload(event))
        return;

    // directSaveMode
    if (d->dropDirectSaveMode(event))
        return;

    // move file betewwn collection or view
    if (d->dropBetweenView(event))
        return;

    // drop mime data by model
    if (d->dropMimeData(event))
        return;

    event->ignore();
}
