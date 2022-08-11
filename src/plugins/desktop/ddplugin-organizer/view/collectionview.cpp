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
#include "models/fileproxymodel.h"
#include "delegate/collectionitemdelegate.h"
#include "mode/collectiondataprovider.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "utils/fileoperator.h"

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

using namespace ddplugin_organizer;
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static constexpr int kCollectionViewMargin = 4;
static constexpr int kCollectionItemVerticalMaxMargin = 5;
static constexpr int kCollectionItemVertiaclMinMargin = 1;

CollectionViewPrivate::CollectionViewPrivate(const QString &uuid, CollectionDataProvider *dataProvider, CollectionView *qq, QObject *parent)
    : QObject(parent)
    , q(qq)
    , id(uuid)
    , provider(dataProvider)
{
    touchDragTimer.setSingleShot(true);
    touchDragTimer.setTimerType(Qt::PreciseTimer);

    initUI();
    initConnect();
}

CollectionViewPrivate::~CollectionViewPrivate()
{

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

void CollectionViewPrivate::initConnect()
{
    connect(provider, &CollectionDataProvider::itemsChanged, this, &CollectionViewPrivate::onItemsChanged);
}

void CollectionViewPrivate::updateRegionView()
{
    q->itemDelegate()->updateItemSizeHint();
    auto itemSize = q->itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());

    const QMargins viewMargin(kCollectionViewMargin, kCollectionViewMargin, kCollectionViewMargin, kCollectionViewMargin);
    updateViewSizeData(q->geometry().size(), viewMargin, itemSize);
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
    updateViewMargins(viewSize, viewMargins);

    updateColumnCount(viewSize.width(), itemSize.width());

    updateRowCount(viewSize.height(), itemSize.height());

    updateCellMargins(itemSize, QSize(cellWidth, cellHeight));
}

void CollectionViewPrivate::updateVerticalBarRange()
{
    needUpdateVerticalBarRange = false;

    int dataRow = provider->items(id).count() / columnCount;
    if (0 != provider->items(id).count() % columnCount)
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

    for (auto url : provider->items(id)) {
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

bool CollectionViewPrivate::drop(QDropEvent *event)
{
    // todo:从文管的桌面目录，同时拖拽集合与非集合的文件到集合中?

    // some special case
    if (dropFilter(event))
        return true;

    // copy file by other app
    if (dropClientDownload(event))
        return true;

    // directSaveMode
    if (dropDirectSaveMode(event))
        return true;

    // move file betewwn collection or view
    if (dropBetweenCollection(event))
        return true;

    // from canvas move file to collection
    if (dropFromCanvas(event))
        return true;

    // drop mime data by model
    if (dropMimeData(event))
        return true;

    event->ignore();
    return true;
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

bool CollectionViewPrivate::dropBetweenCollection(QDropEvent *event) const
{
    if (WindowUtils::keyCtrlIsPressed() || event->mimeData()->urls().isEmpty())
        return false;

    auto firstUrl = event->mimeData()->urls().first();
    auto firstIndex = q->model()->index(firstUrl);
    if (!firstIndex.isValid()) {
        // source file does not belong to collection
        return false;
    }

    QPoint viewPoint(event->pos().x() + q->horizontalOffset(), event->pos().y() + q->verticalOffset());
    auto dropPos = pointToPos(viewPoint);
    auto targetIndex = q->indexAt(event->pos());
    bool dropOnSelf = targetIndex.isValid() ? q->selectionModel()->selectedIndexes().contains(targetIndex) : false;

    if (dropOnSelf) {
        qInfo() << "drop on self, skip. drop:" << dropPos.x() << dropPos.y();
        return true;
    }

    if (targetIndex.isValid()) {
        qDebug() << "drop on target:" << targetIndex << q->model()->fileUrl(targetIndex);
        return false;
    }

    if (!fileShiftable) {
        // disbale shift file from other collection
        auto key = provider->key(firstUrl);
        if (id != key) {
            qDebug() << "disbale shift file from other collection.";
            return true;
        }
    }

    auto urls = event->mimeData()->urls();
    auto index = posToNode(dropPos);
    provider->moveUrls(urls, id, index);

    return true;
}

bool CollectionViewPrivate::dropFromCanvas(QDropEvent *event) const
{
    if (WindowUtils::keyCtrlIsPressed() || event->mimeData()->urls().isEmpty())
        return false;

    auto firstUrl = event->mimeData()->urls().first();
    auto firstIndex = q->model()->index(firstUrl);
    if (firstIndex.isValid()) {
        qWarning() << "source file belong collection:" << firstUrl;
        return false;
    }

    QString errString;
    auto itemInfo =  InfoFactory::create<LocalFileInfo>(firstUrl, true, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        qWarning() << "create LocalFileInfo error: " << errString << firstUrl;
        return false;
    }

    if (itemInfo->absolutePath() != q->model()->fileUrl(q->model()->rootIndex()).toLocalFile()) {
        qWarning() << "source file not belong desktop:" << event->mimeData()->urls();
        return false;
    }

    auto targetIndex = q->indexAt(event->pos());
    if (targetIndex.isValid()) {
        qDebug() << "drop on target:" << targetIndex << q->model()->fileUrl(targetIndex);
        return false;
    }

    auto urls = event->mimeData()->urls();
    QPoint viewPoint(event->pos().x() + q->horizontalOffset(), event->pos().y() + q->verticalOffset());
    auto dropPos = pointToPos(viewPoint);
    auto index = posToNode(dropPos);

    provider->addPreItems(id, urls, index);

    for (auto url : urls)
        canvasModelShell->take(url);

    q->model()->fetch(urls);

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

        if (!targetIndex.isValid()) {
            qDebug() << "drop files to collection.";
            return dropFiles(event);
        }

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

bool CollectionViewPrivate::dropFiles(QDropEvent *event) const
{
    // drop files to collection
    auto urls = event->mimeData()->urls();
    QPoint viewPoint(event->pos().x() + q->horizontalOffset(), event->pos().y() + q->verticalOffset());
    auto dropPos = pointToPos(viewPoint);
    auto index = posToNode(dropPos);
    QUrl targetUrl = q->model()->fileUrl(q->model()->rootIndex());

    FileOperatorIns->dropFilesToCollection(event->dropAction(), targetUrl, urls, id, index);

    event->acceptProposedAction();
    return true;
}

void CollectionViewPrivate::updateRowCount(const int &viewHeight, const int &itemHeight)
{
    const int availableHeight = viewHeight - viewMargins.top() - viewMargins.bottom();
    rowCount = availableHeight / itemHeight;
    if (Q_UNLIKELY(rowCount < 1)) {
        qWarning() << "Row count is 0!Fix it to 1,and set cell height to:" << itemHeight;
        cellHeight = itemHeight;
        rowCount = 1;
    } else {
        int margin = (availableHeight - rowCount * itemHeight) / (rowCount + 1) / 2;
        if (margin > kCollectionItemVerticalMaxMargin)
            margin = kCollectionItemVerticalMaxMargin;
        else if (margin < kCollectionItemVertiaclMinMargin)
            margin = kCollectionItemVertiaclMinMargin;

        cellHeight = itemHeight + 2 * margin;

        // update viewMargins
        viewMargins.setTop(viewMargins.top() + margin);
        viewMargins.setBottom(viewMargins.bottom() + margin);
        // there is a scroll bar in the vertical direction, so you don't need to care about the unused height
    }

    if (Q_UNLIKELY(cellHeight < 1)) {
        qWarning() << "Cell height is:" << cellHeight << "!Fix it to 1";
        cellHeight = 1;
    }
}

void CollectionViewPrivate::updateColumnCount(const int &viewWidth, const int &itemWidth)
{
    const int availableWidth = viewWidth - viewMargins.left() - viewMargins.right();
    columnCount = availableWidth / itemWidth;
    if (Q_UNLIKELY(columnCount < 1)) {
        qWarning() << "Column count is 0!Fix it to 1,and set cell width to:" << viewWidth;
        cellWidth = viewWidth;
        columnCount = 1;
    } else {
        int margin = (availableWidth - columnCount * itemWidth) / (columnCount + 1) / 2;
        cellWidth = itemWidth + 2 * margin;

        // update viewMargins
        int leftViewMargin = viewMargins.left() + margin;
        int rightViewMargin = viewMargins.right() + margin;
        int unUsedWidth = viewWidth - leftViewMargin - rightViewMargin - columnCount * cellWidth;
        // try to divide equally
        leftViewMargin += unUsedWidth / 2;
        rightViewMargin += unUsedWidth - unUsedWidth / 2;

        viewMargins.setLeft(leftViewMargin);
        viewMargins.setRight(rightViewMargin);
    }

    if (Q_UNLIKELY(cellWidth < 1)) {
        qWarning() << "Cell width is:" << cellWidth << "!Fix it to 1";
        cellWidth = 1;
    }
}

void CollectionViewPrivate::updateCellMargins(const QSize &itemSize, const QSize &cellSize)
{
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
    Q_UNUSED(viewSize)

    // fixed value,required by design
    viewMargins = oldMargins;
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

void CollectionViewPrivate::onItemsChanged(const QString &key)
{
    if (id != key)
        return;

    updateVerticalBarRange();
    q->update();
}

CollectionView::CollectionView(const QString &uuid, CollectionDataProvider *dataProvider, QWidget *parent)
    : QAbstractItemView(parent)
    , d(new CollectionViewPrivate(uuid, dataProvider, this))
{
#ifdef QT_DEBUG
    d->showGrid = true;
#endif
}

CollectionView::~CollectionView()
{

}

void CollectionView::setCanvasModelShell(CanvasModelShell *sh)
{
    d->canvasModelShell = sh;
}

void CollectionView::setCanvasViewShell(CanvasViewShell *sh)
{
    d->canvasViewShell = sh;
}

void CollectionView::setCanvasGridShell(CanvasGridShell *sh)
{
    d->canvasGridShell = sh;
}

void CollectionView::setFileShiftable(const bool enable)
{
    d->fileShiftable = enable;
}

bool CollectionView::fileShiftable() const
{
    return d->fileShiftable;
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
    if (!d->provider->items(d->id).contains(url))
        return QRect();

    int node = d->provider->items(d->id).indexOf(url);
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

    // then check the item on the viewPoint.
    {
        QPoint viewPoint(point.x() + horizontalOffset(), point.y() + verticalOffset());
        int node = d->posToNode(d->pointToPos(viewPoint));
        if (node < 0 || node >= d->provider->items(d->id).count())
            return QModelIndex();
        rowIndex = model()->index(d->provider->items(d->id).at(node));
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
    if (d->provider->items(d->id).isEmpty())
        return QModelIndex();

    const QModelIndex &&current = currentIndex();
    if (!current.isValid()) {
        const QUrl &url = d->provider->items(d->id).first();
        return model()->index(url);
    }

    auto currentUrl = model()->fileUrl(current);
    auto node = d->provider->items(d->id).indexOf(currentUrl);
    if (Q_UNLIKELY(-1 == node)) {
        qWarning() << "current url not belong to me." << currentUrl << d->provider->items(d->id);
        return QModelIndex();
    }

    // todo(wangcl) hand modifiers(ctrl and shift)

    switch (cursorAction) {
    case MovePrevious :
    case MoveLeft : {

        if (node > 0) {
            --node;
        } else {
            // first index
            return current;
        }
    }
        break;
    case MoveNext :
    case MoveRight : {

        if (node < d->provider->items(d->id).count() - 1) {
            ++node;
        } else {
            // last index
            return current;
        }
    }
        break;
    case MoveUp : {

        if (node > d->columnCount) {
            // not first row
            node -= d->columnCount;
        } else {
            // first row
            return current;
        }
    }
        break;
    case MoveDown : {

        if (node < d->provider->items(d->id).count() - d->columnCount) {
            node += d->columnCount;
        } else {
            auto &&pos = d->nodeToPos(node);
            auto lastNode = d->provider->items(d->id).count() - 1;
            auto &&lastPos = d->nodeToPos(lastNode);
            if (pos.x() != lastPos.x()) {
                // there are some files in the next row,select last file
                node = lastNode;
            } else {
                // ignore
                return current;
            }
        }
    }
        break;
    case MoveHome : {
        node = 0;
    }
        break;
    case MoveEnd : {
        node = d->provider->items(d->id).count() - 1;
    }
        break;
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

    auto &&afterUrl = d->provider->items(d->id).at(node);
    auto &&afterIndex = model()->index(afterUrl);

    return afterIndex;
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
        if (node >= d->provider->items(d->id).count())
            break;

        auto url = d->provider->items(d->id).at(node);
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
    bool leftButtonPressed = event->buttons().testFlag(Qt::LeftButton);
    if (leftButtonPressed) {
        d->canUpdateVerticalBarRange = false;
    }

    d->checkTouchDarg(event);

    auto pos = event->pos();
    auto index = indexAt(pos);
    if (index.isValid() && isPersistentEditorOpen(index))
        return;

    d->pressedAlreadySelected = selectionModel()->isSelected(index);
    d->pressedIndex = index;
    d->pressedModifiers = event->modifiers();

    QAbstractItemView::mousePressEvent(event);
    if (leftButtonPressed && d->pressedAlreadySelected && Qt::ControlModifier == d->pressedModifiers) {
        // reselect index(maybe the user wants to drag and copy by Ctrl)
        selectionModel()->select(d->pressedIndex, QItemSelectionModel::Select);
    }
}

void CollectionView::mouseReleaseEvent(QMouseEvent *event)
{
    d->canUpdateVerticalBarRange = true;
    if (d->needUpdateVerticalBarRange) {
        d->updateVerticalBarRange();
    }

    if (d->pressedIndex.isValid() && d->pressedIndex == indexAt(event->pos())
            && d->pressedAlreadySelected && Qt::ControlModifier == d->pressedModifiers) {
        // not drag and cot by Ctrl,so deselect index
        selectionModel()->select(d->pressedIndex, QItemSelectionModel::Deselect);
    }

    QAbstractItemView::mouseReleaseEvent(event);
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

void CollectionView::keyPressEvent(QKeyEvent *event)
{
    QAbstractItemView::keyPressEvent(event);

    // must accept event
    event->accept();
}

void CollectionView::startDrag(Qt::DropActions supportedActions)
{
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
    d->dropTargetUrl.clear();

    return QAbstractItemView::dragLeaveEvent(event);
}

void CollectionView::dropEvent(QDropEvent *event)
{
    if (d->drop(event)) {
        setState(NoState);
        return;
    }

    QAbstractItemView::dropEvent(event);
}

bool CollectionView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    return QAbstractItemView::edit(index, trigger, event);
}
