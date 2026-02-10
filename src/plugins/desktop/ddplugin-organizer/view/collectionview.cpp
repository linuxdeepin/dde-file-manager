// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectionview_p.h"
#include "models/collectionmodel.h"
#include "delegate/collectionitemdelegate.h"
#include "mode/collectiondataprovider.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmanagershell.h"
#include "utils/fileoperator.h"
#include "broker/collectionhookinterface.h"
#include "models/itemselectionmodel.h"
#include "config/configpresenter.h"
#include "mode/normalized/fileclassifier.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/sortutils.h>
#include <dfm-base/base/application/application.h>

#include <DApplication>
#include <DFileDragClient>
#include <DGuiApplicationHelper>

#include <QScrollBar>
#include <QUrl>
#include <QDebug>
#include <QPainter>
#include <QDrag>
#include <QMimeData>

using namespace ddplugin_organizer;
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

// no need view margin, this 2px used to draw inner and out order.
static constexpr int kCollectionViewMargin = 0;   // 2

static constexpr int kCollectionItemVerticalMargin = 2;
static constexpr int kIconOffset = 10;
static constexpr int kIconSelectMargin = 3;

CollectionViewPrivate::CollectionViewPrivate(const QString &uuid, CollectionDataProvider *dataProvider, CollectionView *qq, QObject *parent)
    : QObject(parent), q(qq), id(uuid), provider(dataProvider), menuProxy(new CollectionViewMenu(qq))
{
    touchDragTimer.setSingleShot(true);
    touchDragTimer.setTimerType(Qt::PreciseTimer);
}

CollectionViewPrivate::~CollectionViewPrivate()
{
}

void CollectionViewPrivate::initUI()
{
    q->setAttribute(Qt::WA_TranslucentBackground);
    q->setAttribute(Qt::WA_InputMethodEnabled);

#ifdef QT_SCROLL_WHEEL_ANI
    QScrollBar *bar = q->verticalScrollBar();
    bar->setSingleStep(1);
    q->setVerticalScrollBarPolicy(Qt::ScrollBarSlideAnimationOn);
#endif

    q->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    q->viewport()->setAutoFillBackground(false);

    q->setFrameShape(QFrame::NoFrame);
    q->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

    q->setDragDropOverwriteMode(false);
    q->setDragDropMode(QAbstractItemView::DragDrop);
    q->setDefaultDropAction(Qt::CopyAction);

    delegate = new CollectionItemDelegate(q);
    q->setItemDelegate(delegate);

    auto ef = new GraphicsEffect(q);
    q->viewport()->setGraphicsEffect(ef);
}

void CollectionViewPrivate::initConnect()
{
    connect(provider, &CollectionDataProvider::itemsChanged, this, &CollectionViewPrivate::onItemsChanged);

    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(200);
    connect(searchTimer, &QTimer::timeout, this, [this]() {
        searchKeys.clear();
    });

    using namespace std::placeholders;
    connect(q, &CollectionView::clicked, this, std::bind(&CollectionViewPrivate::openIndexByClicked, this, ClickedAction::kClicked, _1));
    connect(q, &CollectionView::doubleClicked, this, std::bind(&CollectionViewPrivate::openIndexByClicked, this, ClickedAction::kDoubleClicked, _1));
}

QList<QRect> CollectionViewPrivate::itemPaintGeomertys(const QModelIndex &index) const
{
    if (Q_UNLIKELY(!index.isValid()))
        return {};

    QStyleOptionViewItem option;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    option = q->viewOptions();
#else
    q->initViewItemOption(&option);
#endif
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

    QRect orgRect(point.x(), point.y(), cellWidth, cellHeight);

    // translate postion
    return orgRect.translated(-q->horizontalOffset(), -q->verticalOffset());
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

    q->verticalScrollBar()->setRange(0, qMax(0, height));
    q->verticalScrollBar()->setPageStep(q->viewport()->height());
    q->verticalScrollBar()->setSingleStep(1);
    // fmDebug() << "update vertical scrollbar range to:" << q->verticalScrollBar()->maximum();
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
    const QRect actualRect(qMin(rect.left(), rect.right()), qMin(rect.top(), rect.bottom()), abs(rect.width()), abs(rect.height()));
    const QPoint offset(q->horizontalOffset(), q->verticalOffset());
    const QPoint iconOffset(kIconOffset, kIconOffset);

    for (auto url : provider->items(id)) {
        auto index = q->model()->index(url);
        const QRect &&itRect = q->visualRect(index);
        QRect realItemRect(itRect.topLeft() + offset + iconOffset, itRect.bottomRight() + offset - iconOffset);

        // least 3 pixels
        if (actualRect.left() > realItemRect.right() - kIconSelectMargin
            || actualRect.top() > realItemRect.bottom() - kIconSelectMargin
            || actualRect.right() < realItemRect.left() + kIconSelectMargin
            || actualRect.bottom() < realItemRect.top() + kIconSelectMargin)
            continue;

        if (!selection.contains(index)) {
            selection.push_back(QItemSelectionRange(index));
        }
    }

    return selection;
}

void CollectionViewPrivate::selectItems(const QList<QUrl> &fileUrl) const
{
    // fileUrl is file:///xxxx
    QItemSelection seleted;
    for (const QUrl &url : fileUrl) {
        auto desktopUrl = url.toString();
        auto index = q->model()->index(desktopUrl);
        QItemSelectionRange selectionRange(index);
        if (!seleted.contains(index)) {
            seleted.push_back(selectionRange);
        }
    }

    if (!seleted.isEmpty())
        q->selectionModel()->select(seleted, QItemSelectionModel::ClearAndSelect);
}

void CollectionViewPrivate::selectRect(const QRect &rect) const
{
    auto selectModel = q->selectionModel();
    if (Q_UNLIKELY(!selectModel))
        return;

    auto rectSelection = selection(rect);
    if (qApp->keyboardModifiers() == Qt::ControlModifier)
        selectModel->select(rectSelection, QItemSelectionModel::ToggleCurrent);
    else if (qApp->keyboardModifiers() == Qt::ShiftModifier)
        selectModel->select(rectSelection, QItemSelectionModel::SelectCurrent);
    else
        selectModel->select(rectSelection, QItemSelectionModel::ClearAndSelect);
}

void CollectionViewPrivate::toggleSelect() const
{
    auto sel = q->selectionModel();
    auto model = q->model();
    if (!sel || !model)
        return;

    auto currentSelected = sel->selectedIndexes();
    if (currentSelected.isEmpty())
        return;

    QItemSelection selections;
    bool contain = false;
    for (int node = 0; node < provider->items(id).count(); ++node) {
        auto &&fileUrl = provider->items(id).at(node);
        auto &&index = q->model()->index(fileUrl);
        if (currentSelected.contains(index)) {
            contain = true;
        } else {
            selections.push_back(QItemSelectionRange(index));
        }
    }

    if (!contain)
        return;

    sel->select(selections, QItemSelectionModel::ClearAndSelect);
}

QPoint CollectionViewPrivate::pointToPos(const QPoint &point) const
{
    int column = (point.x() - viewMargins.left()) / cellWidth;
    int row = (point.y() - viewMargins.top()) / cellHeight;
    return QPoint(column, row);
}

QPoint CollectionViewPrivate::posToPoint(const QPoint &pos) const
{
    int pointX = pos.x() * cellWidth + viewMargins.left();
    int pointY = pos.y() * cellHeight + viewMargins.top();
    return QPoint(pointX, pointY);
}

int CollectionViewPrivate::posToNode(const QPoint &pos) const
{
    return pos.x() + pos.y() * columnCount;
}

QPoint CollectionViewPrivate::nodeToPos(const int node) const
{
    return QPoint(node % columnCount, node / columnCount);
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
        fmWarning() << "current index is invalid.";
        foucs = indexs.first();
    } else if (!indexs.contains(foucs)) {
        fmWarning() << "current index is not in indexs.";
        foucs = indexs.first();
    }
    const int indexCount = indexs.count();
    // remove focus which will paint on top
    indexs.removeAll(foucs);

    static const int iconWidth = 128;
    static const int iconMargin = 30;   // add margin for showing ratoted item.
    static const int maxIconCount = 4;   // max painting item number.
    static const int maxTextCount = 99;   // max text number.
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

    QStyleOptionViewItem option;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    option = q->viewOptions();
#else
    q->initViewItemOption(&option);
#endif

    option.state |= QStyle::State_Selected;
    // icon rect in pixmap.
    option.rect = pixRect.translated(iconMargin, iconMargin);
    option.rect.setSize(iconSize);

    QPainter painter(&pixmap);
    // paint items except focus
    for (int i = qMin(maxIconCount - 1, indexs.count() - 1); i >= 0; --i) {
        painter.save();

        // opacity 50% 40% 30% 20%
        painter.setOpacity(1.0 - (i + 5) * opacityBase);

        // rotate
        {
            qreal rotate = rotateBase * (qRound((i + 1.0) / 2.0) / 2.0 + 1.0) * (i % 2 == 1 ? -1 : 1);
            auto tf = painter.transform();

            // rotate on center
            tf = tf.translate(offsetX, offsetY).rotate(rotate).translate(-offsetX, -offsetY);
            painter.setTransform(tf);
        }

        // paint icon
        q->itemDelegate()->paintDragIcon(&painter, option, indexs.at(i));

        painter.restore();
    }

    // paint focus
    QSize topIconSize;
    {
        painter.save();
        painter.setOpacity(0.8);
        topIconSize = q->itemDelegate()->paintDragIcon(&painter, option, foucs);
        painter.restore();
    }

    // paint text
    {
        int length = 0;
        QString text;
        if (indexCount > maxTextCount) {
            length = 28;   // there are three characters showed.
            text = QString::number(maxTextCount).append("+");
        } else {
            length = 24;   // one or two characters
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

bool CollectionViewPrivate::checkProhibitPaths(QDragEnterEvent *event) const
{
    auto urlsForDragEvent = event->mimeData()->urls();

    // Filter the event that cannot be dragged
    if (!urlsForDragEvent.isEmpty() && FileUtils::isContainProhibitPath(urlsForDragEvent)) {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
        return true;
    }

    return false;
}

void CollectionViewPrivate::preproccessDropEvent(QDropEvent *event, const QUrl &targetUrl) const
{
    //! event->mimeData()->urls() may change to be empty after checking empty.
    //! so the following code must use variable urls instead of event->mimeData()->urls().
    auto urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    // in collection
    if (qobject_cast<CollectionView *>(event->source())) {
        auto action = WindowUtils::keyCtrlIsPressed() ? Qt::CopyAction : Qt::MoveAction;
        event->setDropAction(action);
        return;
    }

    QString errString;
    auto itemInfo = InfoFactory::create<FileInfo>(targetUrl, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        fmWarning() << "create FileInfo error: " << errString << targetUrl;
        return;
    }

    if (event->mimeData() && !event->mimeData()->hasFormat(DFMGLOBAL_NAMESPACE::Mime::kDFMAppTypeKey))
        return;

    auto defaultAction = Qt::CopyAction;
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
    if (FileUtils::isTrashFile(from))
        defaultAction = Qt::MoveAction;

    const bool sameUser = SysInfoUtils::isSameUser(event->mimeData());
    if (event->possibleActions().testFlag(defaultAction))
        event->setDropAction((defaultAction == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : defaultAction);

    if (!itemInfo->supportedOfAttributes(SupportedType::kDrop).testFlag(event->dropAction())) {
        QList<Qt::DropAction> actions { Qt::CopyAction, Qt::MoveAction, Qt::LinkAction };
        for (auto action : actions) {
            if (event->possibleActions().testFlag(action) && itemInfo->supportedOfAttributes(SupportedType::kDrop).testFlag(action)) {
                event->setDropAction((action == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : action);
                break;
            }
        }
    }

    event->setDropAction(defaultAction);
}

void CollectionViewPrivate::handleMoveMimeData(QDropEvent *event, const QUrl &url)
{
    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        // update target url if mouse focus is on file which can drop.
        // drag file from compressor
        updateTarget(event->mimeData(), url);
    } else {
        event->accept();
    }
}

bool CollectionViewPrivate::drop(QDropEvent *event)
{
    // extend
    {
        QVariantHash ext;
        ext.insert("QDropEvent", reinterpret_cast<qlonglong>(event));
        QUrl dropUrl;
        QModelIndex dropIndex = q->indexAt(event->pos());
        if (dropIndex.isValid())
            dropUrl = q->model()->fileUrl(dropIndex);
        else
            dropUrl = q->model()->rootUrl();

        ext.insert("dropUrl", QVariant(dropUrl));
        if (CollectionHookInterface::dropData(id, event->mimeData(), event->pos(), &ext)) {
            fmDebug() << "droped by extend";
            return true;
        }
    }

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

void CollectionViewPrivate::helpAction()
{
    QString appName = qApp->applicationName();
    qApp->setApplicationName("dde");
    DGuiApplicationHelper::instance()->handleHelpAction();
    qApp->setApplicationName(appName);
}

void CollectionViewPrivate::openFiles()
{
    FileOperatorIns->openFiles(q);
}

void CollectionViewPrivate::moveToTrash()
{
    FileOperatorIns->moveToTrash(q);
}

void CollectionViewPrivate::showMenu()
{
    // same as canvas
    if (CollectionViewMenu::disableMenu())
        return;

    QModelIndexList indexList = q->selectionModel()->selectedIndexes();
    bool isEmptyArea = indexList.isEmpty();
    Qt::ItemFlags flags;
    QModelIndex index;
    if (isEmptyArea) {
        index = q->rootIndex();
        flags = q->model()->flags(index);
        if (!flags.testFlag(Qt::ItemIsEnabled))
            return;
    } else {
        index = q->currentIndex();

        // the current index may be not in selected indexs."
        if (!indexList.contains(index)) {
            fmDebug() << "current index is not selected.";
            index = indexList.last();
        }

        flags = q->model()->flags(index);
        if (!flags.testFlag(Qt::ItemIsEnabled)) {
            fmInfo() << "file is disbale, switch to empty area" << q->model()->fileUrl(index);
            isEmptyArea = true;
            flags = q->rootIndex().flags();
        }
    }

    q->itemDelegate()->revertAndcloseEditor();
    if (isEmptyArea) {
        q->selectionModel()->clear();
        menuProxy->emptyAreaMenu();
    } else {
        auto gridPos = pointToPos(q->visualRect(index).center());
        menuProxy->normalMenu(index, q->model()->flags(index), gridPos);
    }
}

void CollectionViewPrivate::deleteFiles()
{
    FileOperatorIns->deleteFiles(q);
}

void CollectionViewPrivate::clearClipBoard()
{
    auto urls = ClipBoard::instance()->clipboardFileUrlList();
    if (!urls.isEmpty()) {
        QString errString;
        auto itemInfo = InfoFactory::create<FileInfo>(urls.first(), Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (Q_UNLIKELY(!itemInfo)) {
            fmInfo() << "create FileInfo error: " << errString << urls.first();
            return;
        }
        auto homePath = q->model()->rootUrl().toLocalFile();
        if (itemInfo->pathOf(PathInfoType::kAbsolutePath) == homePath)
            ClipBoard::instance()->clearClipboard();
    }
}

void CollectionViewPrivate::selectCollection()
{
    QItemSelection selections;
    for (int node = 0; node < provider->items(id).count(); ++node) {
        auto &&fileUrl = provider->items(id).at(node);
        auto &&index = q->model()->index(fileUrl);
        if (!selections.contains(index)) {
            selections.push_back(QItemSelectionRange(index));
        }
    }
    q->selectionModel()->select(selections, QItemSelectionModel::ClearAndSelect);
}

void CollectionViewPrivate::copyFiles()
{
    FileOperatorIns->copyFiles(q);
}

void CollectionViewPrivate::copyFilePath()
{
    FileOperatorIns->copyFilePath(q);
}

void CollectionViewPrivate::cutFiles()
{
    FileOperatorIns->cutFiles(q);
}

void CollectionViewPrivate::pasteFiles()
{
    FileOperatorIns->pasteFiles(q, "");
}

void CollectionViewPrivate::undoFiles()
{
    FileOperatorIns->undoFiles(q);
}

void CollectionViewPrivate::previewFiles()
{
    FileOperatorIns->previewFiles(q);
}

void CollectionViewPrivate::showFilesProperty()
{
    FileOperatorIns->showFilesProperty(q);
}

bool CollectionViewPrivate::dropFilter(QDropEvent *event)
{
    // Prevent the desktop's computer/recycle bin/home directory from being dragged and copied to other directories
    {
        QModelIndex index = q->indexAt(event->pos());
        if (index.isValid()) {
            QUrl targetItem = q->model()->fileUrl(index);
            QString errString;
            auto itemInfo = InfoFactory::create<FileInfo>(targetItem, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
            if (Q_UNLIKELY(!itemInfo)) {
                fmWarning() << "create FileInfo error: " << errString << targetItem;
                return false;
            }
            if (itemInfo->isAttributes(OptInfoType::kIsDir) || itemInfo->urlOf(UrlInfoType::kUrl) == DesktopAppUrl::homeDesktopFileUrl()) {
                auto sourceUrls = event->mimeData()->urls();
                bool find = std::any_of(sourceUrls.begin(), sourceUrls.end(), [](const QUrl &url) {
                    return (DesktopAppUrl::computerDesktopFileUrl() == url)
                            || (DesktopAppUrl::trashDesktopFileUrl() == url)
                            || (DesktopAppUrl::homeDesktopFileUrl() == url);
                });
                if (find) {
                    event->setDropAction(Qt::IgnoreAction);
                    return true;
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
        fmInfo() << "Client drop operation detected, target URL:" << dropTargetUrl;

        QList<QUrl> urlList = data->urls();
        if (!urlList.isEmpty()) {
            // follow canvas dropClientDownload
            DFileDragClient *client = new DFileDragClient(data, q);
            fmDebug() << "Starting drag client download with" << urlList.size() << "URLs";
            connect(client, &DFileDragClient::stateChanged, this, [this, urlList](DFileDragState state) {
                if (state == Finished) {
                    selectItems(urlList);
                    fmInfo() << "Client download finished, selected" << urlList.size() << "items";
                }
                fmDebug() << "Client state changed to:" << state;
            });

            connect(client, &DFileDragClient::serverDestroyed, client, &DFileDragClient::deleteLater);
            connect(client, &DFileDragClient::destroyed, []() {
                fmDebug() << "Drag client deleted";
            });
        }

        return true;
    }

    return false;
}

bool CollectionViewPrivate::dropDirectSaveMode(QDropEvent *event) const
{
    // NOTE: The following code sets the properties that will be used
    // in the project `linuxdeepin/qt5platform-plugins`.
    // The purpose is to support dragging a file from a archive to extract it to the dde-filemanager
    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        event->setDropAction(Qt::CopyAction);
        const QModelIndex &index = q->indexAt(event->pos());
        auto fileInfo = q->model()->fileInfo(index.isValid() ? index : q->rootIndex());

        if (fileInfo && fileInfo->urlOf(UrlInfoType::kUrl).isLocalFile()) {
            if (fileInfo->isAttributes(OptInfoType::kIsDir))
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->urlOf(UrlInfoType::kUrl));
            else
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->urlOf(UrlInfoType::kParentUrl));
        }

        event->accept();   // yeah! we've done with XDS so stop Qt from further event propagation.
        return true;
    }

    return false;
}

bool CollectionViewPrivate::dropBetweenCollection(QDropEvent *event) const
{
    auto urls = event->mimeData()->urls();
    if (WindowUtils::keyCtrlIsPressed() || urls.isEmpty())
        return false;

    CollectionView *fromView = qobject_cast<CollectionView *>(event->source());
    if (!fromView) {
        // source file does not belong to collection
        return false;
    }

    QPoint viewPoint(event->pos().x() + q->horizontalOffset(), event->pos().y() + q->verticalOffset());
    auto dropPos = pointToPos(viewPoint);
    auto targetIndex = q->indexAt(event->pos());
    bool dropOnSelf = targetIndex.isValid() ? q->selectionModel()->selectedIndexes().contains(targetIndex) : false;

    if (dropOnSelf) {
        fmDebug() << "Drop on self detected, skipping operation at position:" << dropPos.x() << "," << dropPos.y();
        return true;
    }

    if (targetIndex.isValid()) {
        fmDebug() << "Drop on target item detected:" << q->model()->fileUrl(targetIndex);
        return false;
    }

    if (!fileShiftable) {
        // disbale shift file from other collection
        for (const QUrl &url : urls) {
            auto key = provider->key(url);
            if (id != key) {
                fmDebug() << "File shifting disabled from other collection, source key:" << key;
                return true;
            }
        }
    }

    auto index = posToNode(dropPos);
    provider->moveUrls(urls, id, index);

    return true;
}

bool CollectionViewPrivate::dropFromCanvas(QDropEvent *event) const
{
    auto urls = event->mimeData()->urls();
    if (WindowUtils::keyCtrlIsPressed() || urls.isEmpty())
        return false;

    auto firstUrl = urls.first();
    // auto firstIndex = q->model()->index(firstUrl);
    // if (firstIndex.isValid()) {
    //     fmWarning() << "source file belong collection:" << firstUrl;
    //     return false;
    // }

    QString errString;
    auto itemInfo = InfoFactory::create<FileInfo>(firstUrl, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        fmWarning() << "create FileInfo error: " << errString << firstUrl;
        return false;
    }

    if (itemInfo->pathOf(PathInfoType::kAbsolutePath) != q->model()->fileUrl(q->model()->rootIndex()).toLocalFile()) {
        fmWarning() << "source file not belong desktop:" << event->mimeData()->urls();
        return false;
    }

    auto targetIndex = q->indexAt(event->pos());
    if (targetIndex.isValid()) {
        fmDebug() << "Drop from canvas on target item:" << q->model()->fileUrl(targetIndex);
        return false;
    }

    QPoint viewPoint(event->pos().x() + q->horizontalOffset(), event->pos().y() + q->verticalOffset());
    auto dropPos = pointToPos(viewPoint);
    auto index = posToNode(dropPos);

    provider->addPreItems(id, urls, index);

    for (auto url : urls)
        provider->prepend(url);
    selectItems(urls);

    for (auto url : urls)
        canvasModelShell->take(url);

    // q->model()->fetch(urls);

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
            fmDebug() << "drop files to collection.";
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
    } else if (WindowUtils::isWayLand()) {
        // Bug-209635，wayland下wine应用拖拽时，QDragEvent中的action为Qt::IgnoreActon
        // 当拖拽事件action无效时，判断文件来源，wine应用默认做CopyAction处理
        QList<QUrl> urls = event->mimeData()->urls();

        if (!urls.isEmpty()) {
            const QUrl from = QUrl(urls.first());
            if (!from.path().contains("/.deepinwine/"))
                return false;
            if (model->dropMimeData(event->mimeData(), Qt::CopyAction, targetIndex.row(), targetIndex.column(), targetIndex))
                event->acceptProposedAction();
            return true;
        }
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

void CollectionViewPrivate::continuousSelection(const QPersistentModelIndex &newCurrent)
{
    if (!currentSelectionStartIndex.isValid()) {
        if (newCurrent.isValid()) {
            currentSelectionStartIndex = newCurrent;
            q->selectionModel()->select(newCurrent, QItemSelectionModel::ClearAndSelect);
            q->setCurrentIndex(newCurrent);
        }
        return;
    }

    if (!newCurrent.isValid())
        return;

    q->selectionModel()->setCurrentIndex(newCurrent, QItemSelectionModel::NoUpdate);

    auto &&currentSelectionStartFile = q->model()->fileUrl(currentSelectionStartIndex);
    auto &&currentSelectionStartNode = provider->items(id).indexOf(currentSelectionStartFile);
    if (Q_UNLIKELY(-1 == currentSelectionStartNode)) {
        fmWarning() << "warning:can not find file:" << currentSelectionStartFile << " in collection:" << id
                    << ".Or no file is selected.So fix to 0.";
        currentSelectionStartNode = 0;
    }

    auto &&currentSelectionEndFile = q->model()->fileUrl(newCurrent);
    auto &&currentSelectionEndNode = provider->items(id).indexOf(currentSelectionEndFile);
    if (Q_UNLIKELY(-1 == currentSelectionEndNode)) {
        fmWarning() << "warning:can not find file:" << currentSelectionEndFile << " in collection:" << id
                    << ".Give up switch selection!";
        return;
    }

    int minNode = qMin(currentSelectionStartNode, currentSelectionEndNode);
    int maxNode = qMax(currentSelectionStartNode, currentSelectionEndNode);

    if (Q_UNLIKELY(minNode < 0)) {
        fmWarning() << "warning:minNode error:" << minNode << " and fix to 0";
        minNode = 0;
    }
    if (Q_UNLIKELY(maxNode >= provider->items(id).count())) {
        fmWarning() << "warning:maxNode error:" << maxNode << "and fix to " << provider->items(id).count() - 1;
        maxNode = provider->items(id).count() - 1;
    }

    QItemSelection selections;
    for (int node = minNode; node <= maxNode; ++node) {
        auto &&fileUrl = provider->items(id).at(node);
        auto &&index = q->model()->index(fileUrl);
        if (!selections.contains(index)) {
            selections.push_back(QItemSelectionRange(index));
        }
    }
    q->selectionModel()->select(selections, QItemSelectionModel::ClearAndSelect);
    return;
}

QModelIndex CollectionViewPrivate::findIndex(const QString &key, bool matchStart, const QModelIndex &current, bool reverseOrder, bool excludeCurrent) const
{
    int start = -1;
    auto itemList = provider->items(id);
    if (current.isValid()) {
        auto curUrl = q->model()->fileUrl(current);
        start = itemList.indexOf(curUrl);
    }

    // current index is invalid.
    if (start < 0) {
        start = 0;
        excludeCurrent = false;
    }

    const int count = itemList.size();
    for (int i = excludeCurrent ? 1 : 0; i < count; ++i) {
        int next = reverseOrder ? count + start - i : start + i;
        next = next % count;
        if (excludeCurrent && next == start)
            continue;

        auto item = itemList.at(next);
        QModelIndex index = q->model()->index(item);
        if (!index.isValid())
            continue;

        const QString &pinyinName = q->model()->data(index, Global::ItemRoles::kItemFilePinyinNameRole).toString();

        if (matchStart ? pinyinName.startsWith(key, Qt::CaseInsensitive)
                       : pinyinName.contains(key, Qt::CaseInsensitive)) {
            return index;
        }
    }

    return QModelIndex();
}

void CollectionViewPrivate::updateDFMMimeData(QDropEvent *event)
{
    dfmmimeData.clear();
    const QMimeData *data = event->mimeData();

    if (data && data->hasFormat(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey))
        dfmmimeData = DFMMimeData::fromByteArray(data->data(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey));
}

bool CollectionViewPrivate::checkTargetEnable(QDropEvent *event, const QUrl &targetUrl)
{
    auto rootUrl = q->model()->rootUrl();
    if (rootUrl == targetUrl) {
        auto classfier = dynamic_cast<FileClassifier *>(provider.data());
        if (classfier) {
            auto dragUrls = event->mimeData()->urls();
            for (auto url : dragUrls) {
                auto type = classfier->classify(url);
                if (type != id)
                    return false;
            }
        }
    }

    if (!dfmmimeData.isValid())
        return true;

    if (FileUtils::isTrashDesktopFile(targetUrl))
        return dfmmimeData.canTrash() || dfmmimeData.canDelete();

    return true;
}

void CollectionViewPrivate::openIndexByClicked(const ClickedAction action, const QModelIndex &index)
{
    ClickedAction configAction = static_cast<ClickedAction>(Application::instance()->appAttribute(Application::kOpenFileMode).toInt());
    if (action == configAction) {
        Qt::ItemFlags flags = q->model()->flags(index);
        if (!flags.testFlag(Qt::ItemIsEnabled))
            return;

        if (!WindowUtils::keyCtrlIsPressed() && !WindowUtils::keyShiftIsPressed())
            openIndex(index);
    }
}

void CollectionViewPrivate::openIndex(const QModelIndex &index)
{
    const FileInfoPointer &info = q->model()->fileInfo(index);

    if (!info) {
        fmWarning() << "Cannot open index: file info is null";
        return;
    }

    QUrl fileUrl = info->urlOf(UrlInfoType::kUrl);
    fmDebug() << "Opening file:" << fileUrl;
    FileOperatorIns->openFiles(q, { fileUrl });
}

void CollectionViewPrivate::redoFiles()
{
    FileOperatorIns->undoFiles(q);
}

void CollectionViewPrivate::updateRowCount(const int &viewHeight, const int &itemHeight)
{
    const int availableHeight = viewHeight - viewMargins.top() - viewMargins.bottom();
    rowCount = availableHeight / itemHeight;
    if (Q_UNLIKELY(rowCount < 1)) {
        fmWarning() << "Row count is 0!Fix it to 1,and set cell height to:" << itemHeight;
        cellHeight = itemHeight;
        rowCount = 1;
    } else {
        // fixed value,required by design
        cellHeight = itemHeight + kCollectionItemVerticalMargin;

        // view's top and bottom margins is fixed,not need update

        // there is a scroll bar in the vertical direction, so you don't need to care about the unused height
    }

    if (Q_UNLIKELY(cellHeight < 1)) {
        fmWarning() << "Cell height is:" << cellHeight << "!Fix it to 1";
        cellHeight = 1;
    }
}

void CollectionViewPrivate::updateColumnCount(const int &viewWidth, const int &itemWidth)
{
    const int availableWidth = viewWidth - viewMargins.left() - viewMargins.right();
    columnCount = availableWidth / itemWidth;
    if (Q_UNLIKELY(columnCount < 1)) {
        fmWarning() << "Column count is 0!Fix it to 1,and set cell width to:" << viewWidth;
        cellWidth = viewWidth;
        columnCount = 1;
    } else {
        cellWidth = itemWidth;
    }

    if (Q_UNLIKELY(cellWidth < 1)) {
        fmWarning() << "Cell width is:" << cellWidth << "!Fix it to 1";
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
    : QAbstractItemView(parent), d(new CollectionViewPrivate(uuid, dataProvider, this))
{
#ifdef QT_DEBUG
    // d->showGrid = true;
#endif

    d->initUI();
    d->initConnect();
    setObjectName("dd_collection_view");
}

CollectionView::~CollectionView()
{
}

QString CollectionView::id() const
{
    return d->id;
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

void CollectionView::setCanvasManagerShell(CanvasManagerShell *sh)
{
    if (sh == d->canvasManagerShell)
        return;

    if (d->canvasManagerShell)
        disconnect(d->canvasManagerShell, nullptr, this, nullptr);

    d->canvasManagerShell = sh;
    if (!d->canvasManagerShell)
        return;

    const int level = d->canvasManagerShell->iconLevel();
    d->delegate->setIconLevel(level);
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

CollectionModel *CollectionView::model() const
{
    return qobject_cast<CollectionModel *>(QAbstractItemView::model());
}

CollectionItemDelegate *CollectionView::itemDelegate() const
{
    return qobject_cast<CollectionItemDelegate *>(QAbstractItemView::itemDelegate());
}

CollectionDataProvider *CollectionView::dataProvider() const
{
    return d->provider;
}

WId CollectionView::winId() const
{
    // If it not the top widget and QAbstractItemView::winId() is called,that will cause errors in window system coordinates and graphics.
    if (isTopLevel()) {
        return QAbstractItemView::winId();
    } else {
        return topLevelWidget()->winId();
    }
}

void CollectionView::updateRegionView()
{
    itemDelegate()->updateItemSizeHint();
    auto itemSize = itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());

    const QMargins viewMargin(kCollectionViewMargin, kCollectionViewMargin, kCollectionViewMargin, kCollectionViewMargin);
    d->updateViewSizeData(geometry().size(), viewMargin, itemSize);
    d->updateVerticalBarRange();
}

void CollectionView::refresh(bool silence)
{
    if (verticalScrollBar())
        verticalScrollBar()->setValue(0);

    if (silence)
        return;

    d->flicker = true;
    repaint();
    update();
    d->flicker = false;
}

void CollectionView::setFreeze(bool freeze)
{
    d->freeze = freeze;
}

void CollectionView::openEditor(const QUrl &url)
{
    QModelIndex index = model()->index(url);
    if (Q_UNLIKELY(!index.isValid())) {
        fmWarning() << "Cannot open editor for invalid URL:" << url.toString();
        return;
    }

    fmInfo() << "Opening editor for URL:" << url.toString();
    selectionModel()->select(index, QItemSelectionModel::Select);
    this->setCurrentIndex(index);
    this->edit(index, QAbstractItemView::AllEditTriggers, nullptr);
    this->activateWindow();
}

void CollectionView::selectUrl(const QUrl &url, const QItemSelectionModel::SelectionFlag &flags)
{
    auto index = model()->index(url);
    if (Q_UNLIKELY(!index.isValid())) {
        fmWarning() << "warning:can not find index for:" << url;
        return;
    }

    selectionModel()->select(index, flags);
    // can not setting repeated,becase QAbstractItemView::setCurrentIndex will be cleared select items
    if (!currentIndex().isValid())
        this->setCurrentIndex(index);
    this->activateWindow();
    this->update();
}

void CollectionView::selectUrls(const QList<QUrl> &urls)
{
    if (d)
        d->selectItems(urls);
}

void CollectionView::setModel(QAbstractItemModel *model)
{
    QAbstractItemView::setModel(model);
    // must update root index for view
    setRootIndex(this->model()->rootIndex());
}

void CollectionView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    QItemSelectionModel *oldSelectionModel = QAbstractItemView::selectionModel();
    QAbstractItemView::setSelectionModel(selectionModel);
    if (oldSelectionModel)
        oldSelectionModel->deleteLater();

    // the default selection model is setted after QAbstractItemView::setModel.
    Q_ASSERT(selectionModel);

    //! when selection changed, we need to update all view.
    //! otherwise the expanded text will partly remain.
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, (void(QWidget::*)()) & QWidget::update);
}

void CollectionView::reset()
{
    QAbstractItemView::reset();
    // the reset will be called on model()->endResetModel().
    // all data and state will be cleared in QAbstractItemView::reset.
    // it need to reset root index there.
    setRootIndex(model()->rootIndex());
}

void CollectionView::selectAll()
{
    d->selectCollection();
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

    if (Qt::ControlModifier == modifiers) {
        // Do not allow move when hold ctrl
        return current;
    }

    auto currentUrl = model()->fileUrl(current);
    auto node = d->provider->items(d->id).indexOf(currentUrl);
    if (Q_UNLIKELY(-1 == node)) {
        fmWarning() << "current url not belong to me." << currentUrl << d->provider->items(d->id);
        return QModelIndex();
    }

    switch (cursorAction) {
    case MovePrevious:
    case MoveLeft: {

        if (node > 0) {
            --node;
        } else {
            // first index
            return current;
        }
    } break;
    case MoveNext:
    case MoveRight: {

        if (node < d->provider->items(d->id).count() - 1) {
            ++node;
        } else {
            // last index
            return current;
        }
    } break;
    case MoveUp: {

        if (node >= d->columnCount) {
            // not first row
            node -= d->columnCount;
        } else {
            // first row
            return current;
        }
    } break;
    case MoveDown: {

        if (node < d->provider->items(d->id).count() - d->columnCount) {
            node += d->columnCount;
        } else {
            auto &&pos = d->nodeToPos(node);
            auto &&lastNode = d->provider->items(d->id).count() - 1;
            auto &&lastPos = d->nodeToPos(lastNode);
            if (pos.x() != lastPos.x()) {
                // there are some files in the next row,select last file
                node = lastNode;
            } else {
                // ignore
                return current;
            }
        }
    } break;
    case MoveHome: {
        node = 0;
    } break;
    case MoveEnd: {
        node = d->provider->items(d->id).count() - 1;
    } break;
    case MovePageUp: {
        auto &&pos = d->nodeToPos(node);
        pos.setX(pos.x() - d->rowCount);
        if (pos.x() < 0)
            pos.setX(0);
        node = d->posToNode(pos);
    } break;
    case MovePageDown: {
        int pageFileCount = d->rowCount * d->columnCount;
        if (node + pageFileCount < d->provider->items(d->id).count()) {
            // The following file more than one page,turn the page
            node += pageFileCount;
        } else {
            // The following file is less than one page
            auto &&pos = d->nodeToPos(node);
            auto &&lastNode = d->provider->items(d->id).count() - 1;
            auto &&lastPos = d->nodeToPos(lastNode);
            if (pos.x() != lastPos.x()) {
                // select last file
                node = lastNode;
            } else {
                // already on the last line,ignore
                return current;
            }
        }
    } break;
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
    auto url = model()->fileUrl(index);
    return !d->provider->contains(d->id, url);
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

bool CollectionView::lessThan(const QUrl &left, const QUrl &right) const
{
    auto fileSortOrder = d->sortOrder;
    int fileSortRole = d->sortRole;
    auto m = model();
    QModelIndex leftIdx = m->index(left);
    QModelIndex rightIdx = m->index(right);

    if (!leftIdx.isValid() || !rightIdx.isValid())
        return false;

    FileInfoPointer leftInfo = m->fileInfo(leftIdx);
    FileInfoPointer rightInfo = m->fileInfo(rightIdx);

    // The folder is fixed in the front position
    if (leftInfo->isAttributes(OptInfoType::kIsDir)) {
        if (!rightInfo->isAttributes(OptInfoType::kIsDir))
            return true;
    } else {
        if (rightInfo->isAttributes(OptInfoType::kIsDir))
            return false;
    }

    QVariant leftData = m->data(leftIdx, fileSortRole);
    QVariant rightData = m->data(rightIdx, fileSortRole);

    // When the selected sort attribute value is the same, sort by file name
    auto compareByName = [fileSortOrder, m, leftIdx, rightIdx]() {
        QString leftName = m->data(leftIdx, kItemFileDisplayNameRole).toString();
        QString rightName = m->data(rightIdx, kItemFileDisplayNameRole).toString();
        return SortUtils::compareString(leftName, rightName, fileSortOrder);
    };

    switch (fileSortRole) {
    case kItemFileCreatedRole:
    case kItemFileLastModifiedRole:
    case kItemFileMimeTypeRole:
    case kItemFileDisplayNameRole: {
        QString leftString = leftData.toString();
        QString rightString = rightData.toString();
        return leftString == rightString ? compareByName() : SortUtils::compareString(leftString, rightString, fileSortOrder);
    }
    case kItemFileSizeRole: {
        qint64 leftSize = leftData.toLongLong();
        qint64 rightSize = rightData.toLongLong();
        return leftSize == rightSize ? compareByName() : ((fileSortOrder == Qt::DescendingOrder) ^ (leftSize < rightSize)) == 0x01;
    }
    default:
        return false;
    }
}

void CollectionView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (d->freeze)
        return;

    if (Q_UNLIKELY(!itemDelegate()))
        return;

    if (d->flicker)
        return;

    QStyleOptionViewItem option;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    option = viewOptions();
#else
    initViewItemOption(&option);
#endif
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    auto repaintRect = viewport()->geometry().translated(horizontalOffset(), verticalOffset());
    auto topLeft = repaintRect.topLeft();
    auto bottomRight = repaintRect.bottomRight();
    auto startPoint = d->pointToPos(topLeft);
    auto startNode = d->posToNode(startPoint);
    auto endPoint = d->pointToPos(bottomRight);
    auto endNode = d->posToNode(endPoint);

    if (d->showGrid) {
        painter.save();
        for (auto node = startNode; node <= endNode; ++node) {
            auto pos = d->nodeToPos(node);
            auto point = d->posToPoint(pos);

            // translate postion
            auto rect = QRect(point.x(), point.y(), d->cellWidth, d->cellHeight).translated(-horizontalOffset(), -verticalOffset());

            auto rowMode = pos.x() % 2;
            auto colMode = pos.y() % 2;
            auto color = (colMode == rowMode) ? QColor(0, 0, 255, 32) : QColor(255, 0, 0, 32);
            painter.setPen(Qt::darkGray);
            painter.drawRect(rect);
            painter.fillRect(rect, color);

            auto itemSize = rect.marginsRemoved(d->cellMargins);
            painter.setPen(QPen(Qt::yellow, 1, Qt::DotLine));
            painter.drawRect(itemSize);

            painter.setPen(QPen(Qt::red, 2));
            painter.drawText(rect, QString("%1-%2").arg(d->nodeToPos(node + startNode).x()).arg(d->nodeToPos(node + startNode).y()));
        }
        painter.restore();
    }

    {
#ifdef QT_DEBUG
        painter.save();
        auto rect = viewport()->geometry();
        rect = rect.marginsAdded({ 0, 20, 0, 0 });
        painter.drawLine(rect.topLeft(), rect.bottomRight());
        painter.drawLine(rect.topRight(), rect.bottomLeft());

        painter.restore();
#endif
    }

    // draw files
    {
        QModelIndex expandItem;
        // item may need expand.
        // the expand item need to draw at last. otherwise other item will overlap the expeand text.
        itemDelegate()->mayExpand(&expandItem);

        for (auto node = startNode; node <= endNode; ++node) {
            if (node >= d->provider->items(d->id).count())
                break;

            auto url = d->provider->items(d->id).at(node);
            auto index = model()->index(url);

            // drawed in end.
            if (index == expandItem)
                continue;

            option.rect = visualRect(index).marginsRemoved(d->cellMargins);
            painter.save();
            if (!CollectionHookInterface::drawFile(id(), url, &painter, &option))
                itemDelegate()->paint(&painter, option, index);
            painter.restore();
        }

        if (expandItem.isValid()) {
            option.rect = visualRect(expandItem).marginsRemoved(d->cellMargins);
            painter.save();
            itemDelegate()->paint(&painter, option, expandItem);
            painter.restore();
        }
    }

    if (d->elasticBand.isValid()) {
        QStyleOptionRubberBand opt;
        opt.initFrom(this);
        opt.shape = QRubberBand::Rectangle;
        opt.opaque = false;
        opt.rect = d->elasticBand.translated(-horizontalOffset(), -verticalOffset());
        painter.save();
        style()->drawControl(QStyle::CE_RubberBand, &opt, &painter);
        painter.restore();
    }
}

void CollectionView::wheelEvent(QWheelEvent *event)
{
#ifdef QT_SCROLL_WHEEL_ANI
    return QAbstractItemView::wheelEvent(event);
#else
    int currentPosition = verticalScrollBar()->sliderPosition();
    int scrollValue = event->angleDelta().y();
    int targetValue = currentPosition - scrollValue;
    verticalScrollBar()->setSliderPosition(targetValue);
#endif
}

void CollectionView::mousePressEvent(QMouseEvent *event)
{
    auto pos = event->pos();
    bool leftButtonPressed = event->buttons().testFlag(Qt::LeftButton);

    if (pos.x() < kCollectionStretchThreshold
        || pos.x() > this->width() - kCollectionStretchThreshold
        || pos.y() < kCollectionStretchThreshold
        || pos.y() > this->height() - kCollectionStretchThreshold) {
        if (leftButtonPressed) {
            d->ignoreMouseEvent = true;
            return;
        }
    }

    if (leftButtonPressed) {
        d->canUpdateVerticalBarRange = false;
    }

    d->checkTouchDarg(event);

    auto index = indexAt(pos);
    if (index.isValid() && isPersistentEditorOpen(index)) {
        itemDelegate()->commitDataAndCloseEditor();
        return;
    }

    d->pressedModifiers = event->modifiers();
    d->pressedAlreadySelected = selectionModel()->isSelected(index);
    d->pressedIndex = index;

    QAbstractItemView::mousePressEvent(event);

    if (Qt::ShiftModifier == d->pressedModifiers) {
        // Qabstractitemview will select the elements within the rectangle
        // Special treatment: select continuous elements

        QPersistentModelIndex newCurrent(index);
        d->continuousSelection(newCurrent);
    } else {
        if (leftButtonPressed && d->pressedAlreadySelected && Qt::ControlModifier == d->pressedModifiers) {
            // reselect index(maybe the user wants to drag and copy by Ctrl)
            selectionModel()->select(d->pressedIndex, QItemSelectionModel::Select);
        } else if (!index.isValid() && Qt::ControlModifier != d->pressedModifiers) {   // pressed on blank space.
            setCurrentIndex(QModelIndex());
        }

        // reset start index
        d->currentSelectionStartIndex = selectionModel()->isSelected(index) ? index : QModelIndex();
    }

    QPoint viewPoint(pos.x() + horizontalOffset(), pos.y() + verticalOffset());
    d->pressedPosition = viewPoint;
}

void CollectionView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        d->ignoreMouseEvent = false;
    if (d->elasticBand.isValid()) {
        // clear elasticBand
        d->elasticBand = QRect();

        update();
    }

    d->canUpdateVerticalBarRange = true;
    if (d->needUpdateVerticalBarRange) {
        d->updateVerticalBarRange();
    }

    if (d->pressedIndex.isValid() && d->pressedIndex == indexAt(event->pos())
        && d->pressedAlreadySelected && Qt::ControlModifier == d->pressedModifiers) {
        // not drag and copy by Ctrl,so deselect index
        selectionModel()->select(d->pressedIndex, QItemSelectionModel::Deselect);
    }

    QAbstractItemView::mouseReleaseEvent(event);
}

void CollectionView::mouseMoveEvent(QMouseEvent *event)
{
    if (d->ignoreMouseEvent)
        return;
    QAbstractItemView::mouseMoveEvent(event);

    // left button pressed on empty area.
    if (event->buttons().testFlag(Qt::LeftButton) && !d->pressedIndex.isValid()) {
        QRect rect(d->pressedPosition, event->pos() + QPoint(horizontalOffset(), verticalOffset()));
        d->elasticBand = rect.normalized();

        // update selection
        d->selectRect(d->elasticBand);
        update();
    } else {
        d->elasticBand = QRect();
    }
}

void CollectionView::resizeEvent(QResizeEvent *event)
{
    QAbstractItemView::resizeEvent(event);

    updateRegionView();

    if (d->canUpdateVerticalBarRange) {
        d->updateVerticalBarRange();
    } else {
        d->needUpdateVerticalBarRange = true;
    }
}

void CollectionView::keyPressEvent(QKeyEvent *event)
{
    {
        QVariantHash extData;
        extData.insert("QKeyEvent", (qlonglong)event);
        if (CollectionHookInterface::keyPress(id(), event->key(), event->modifiers(), &extData))
            return;
    }

    switch (event->modifiers()) {
    case Qt::NoModifier:
        switch (event->key()) {
        case Qt::Key_F1: {
            d->helpAction();
            return;
        }
        case Qt::Key_F2: {
            // this is a cheat, use canvas key event handler to do batch rename.
            if (d->canvasViewShell->filterKeyPress(0, Qt::Key_F2, Qt::NoModifier))
                return;
            break;
        }
        case Qt::Key_Escape: {
            d->clearClipBoard();
            return;
        }
        default:
            break;
        }
        Q_FALLTHROUGH();
    case Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            d->openFiles();
            return;
        case Qt::Key_Space:
            if (!event->isAutoRepeat())
                d->previewFiles();
        case Qt::Key_F5:
            // use canvas refresh
            return QAbstractItemView::keyPressEvent(event);
        case Qt::Key_Delete:
            d->moveToTrash();
            return;
        default:
            break;
        }
        break;
    case Qt::AltModifier:
        if (event->key() == Qt::Key_M) {
            d->showMenu();
            return;
        }
        break;
    case Qt::ShiftModifier: {
        if (event->key() == Qt::Key_Delete) {
            d->deleteFiles();
            return;
        }
    } break;
    case Qt::ControlModifier: {
        switch (event->key()) {
        case Qt::Key_A:
            //  dynamic_cast<ItemSelectionModel *>(selectionModel())->selectAll();
            return QAbstractItemView::keyPressEvent(event);
        case Qt::Key_C:
            d->copyFiles();
            return;
        case Qt::Key_I:
            d->showFilesProperty();
            return;
        case Qt::Key_X:
            d->cutFiles();
            return;
        case Qt::Key_V:
            d->pasteFiles();   // paste files to canvas not collection
            return;
        case Qt::Key_Z:
            d->undoFiles();
            return;
        case Qt::Key_Y:
            // redo
            d->redoFiles();
            return;
        case Qt::Key_Minus:
            [[fallthrough]];
        case Qt::Key_Equal:
            return QAbstractItemView::keyPressEvent(event);
        default:
            break;
        }
    } break;
    case (Qt::ControlModifier | Qt::ShiftModifier): {
        if (event->key() == Qt::Key_I) {
            d->toggleSelect();
            return;
        } else if (event->key() == Qt::Key_C) {
            d->copyFilePath();
            return;
        }
    } break;
    default:
        break;
    }

    {
        const QKeySequence &seq { static_cast<int>(event->modifiers()) | event->key() };
        if (CfgPresenter->isEnableVisibility() && CfgPresenter->hideAllKeySequence() == seq)
            return QAbstractItemView::keyPressEvent(event);
    }

    {
        // Qabstractitemview will select the elements within the rectangle
        // Special treatment: select continuous elements
        QPersistentModelIndex newCurrent;
        bool move = true;
        switch (event->key()) {
        case Qt::Key_Down:
            newCurrent = moveCursor(MoveDown, event->modifiers());
            break;
        case Qt::Key_Up:
            newCurrent = moveCursor(MoveUp, event->modifiers());
            break;
        case Qt::Key_Left:
            newCurrent = moveCursor(MoveLeft, event->modifiers());
            break;
        case Qt::Key_Right:
            newCurrent = moveCursor(MoveRight, event->modifiers());
            break;
        case Qt::Key_Home:
            newCurrent = moveCursor(MoveHome, event->modifiers());
            break;
        case Qt::Key_End:
            newCurrent = moveCursor(MoveEnd, event->modifiers());
            break;
        case Qt::Key_PageUp:
            newCurrent = moveCursor(MovePageUp, event->modifiers());
            break;
        case Qt::Key_PageDown:
            newCurrent = moveCursor(MovePageDown, event->modifiers());
            break;
        case Qt::Key_Tab:
            newCurrent = moveCursor(MoveNext, event->modifiers());
            break;
        case Qt::Key_Backtab:
            newCurrent = moveCursor(MovePrevious, event->modifiers());
            break;
        default:
            move = false;
            break;
        }

        if (move) {
            if (event->modifiers() == Qt::NoModifier) {
                d->currentSelectionStartIndex = newCurrent;
                selectionModel()->select(newCurrent, QItemSelectionModel::ClearAndSelect);
                setCurrentIndex(newCurrent);
            } else if (event->modifiers() == Qt::ShiftModifier) {
                d->continuousSelection(newCurrent);
            }
            event->accept();
            return;
        }
    }

    QAbstractItemView::keyPressEvent(event);

    // must accept event
    event->accept();
}

void CollectionView::contextMenuEvent(QContextMenuEvent *event)
{
    if (this->property(kCollectionPropertyEditing).toBool())
        return;
    if (CollectionViewMenu::disableMenu())
        return;

    const QModelIndex &index = indexAt(event->pos());
    itemDelegate()->revertAndcloseEditor();

    if (!index.isValid())
        d->menuProxy->emptyAreaMenu();
    else {
        // menu focus is on the index that is not selected
        if (!selectionModel()->isSelected(index)) {
            selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            d->currentSelectionStartIndex = QModelIndex();
        }

        d->menuProxy->normalMenu(index, model()->flags(index), d->pointToPos(event->pos()));
    }

    event->accept();
}

void CollectionView::startDrag(Qt::DropActions supportedActions)
{
    if (d->isDelayDrag()) {
        fmDebug() << "Drag delayed due to touch screen";
        return;
    }

    // close editor before drag.
    // normally, items in editing status do not enter startDrag.
    // but if drag and drope one item before editing it, then draging it, startDrag will be called.
    // the reason is that when one item droped, the d->pressedIndex is setted to invaild.
    // then in mousePressEvent d->pressedIndex is not updating to preesed index because the state is EditingState.
    // finally, in mouseMoveEvent state is changed from EditingState to DragSelectingState because d->pressedInde is invaild.
    if (isPersistentEditorOpen(currentIndex()))
        closePersistentEditor(currentIndex());

    if (CollectionHookInterface::startDrag(id(), supportedActions)) {
        fmDebug() << "Drag handled by extension interface";
        return;
    }

    // Drag all selected items instead of just the selected items in the current view
    QModelIndexList validIndexes = selectionModel()->selectedIndexes();
    if (validIndexes.count() > 1) {
        QMimeData *data = model()->mimeData(validIndexes);
        if (!data) {
            fmWarning() << "Failed to create mime data for drag operation";
            return;
        }

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
    // Filter the event that cannot be dragged
    if (d->checkProhibitPaths(event))
        return;

    d->updateDFMMimeData(event);
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
    if (!d->checkTargetEnable(event, currentUrl)) {
        event->ignore();
        return;
    } else if (hoverIndex.isValid()) {
        if (auto fileInfo = model()->fileInfo(hoverIndex)) {
            // hook
            {
                Qt::DropAction dropAction = Qt::IgnoreAction;
                QVariantHash ext;
                ext.insert("hoverUrl", QVariant(currentUrl));
                ext.insert("dropAction", qlonglong(&dropAction));
                if (CollectionHookInterface::dragMove(id(), event->mimeData(), event->pos(), &ext)) {
                    if (dropAction != Qt::IgnoreAction) {
                        event->setDropAction(dropAction);
                        event->accept();
                        return;
                    }
                }
            }

            bool canDrop = !fileInfo->canAttributes(CanableInfoType::kCanDrop)
                    || (fileInfo->isAttributes(OptInfoType::kIsDir) && !fileInfo->isAttributes(OptInfoType::kIsWritable))
                    || !fileInfo->supportedOfAttributes(SupportedType::kDrop).testFlag(event->dropAction());
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

void CollectionView::focusInEvent(QFocusEvent *event)
{
    QAbstractItemView::focusInEvent(event);

    // WA_InputMethodEnabled will be set to false if current index is invalid in QAbstractItemView::focusInEvent
    // To enable WA_InputMethodEnabled no matter whether the current index is valid or not.
    if (!testAttribute(Qt::WA_InputMethodEnabled))
        setAttribute(Qt::WA_InputMethodEnabled, true);
}

void CollectionView::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::FontChange:
        updateRegionView();
        break;
    default:
        break;
    }

    QAbstractItemView::changeEvent(event);
}

void CollectionView::scrollContentsBy(int dx, int dy)
{
    // scroll the viewport to let the editor scrolled with item.
    viewport()->scroll(dx, dy);

    // just update viewport.
    QAbstractItemView::scrollContentsBy(dx, dy);
}

bool CollectionView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    if (WindowUtils::keyCtrlIsPressed() || WindowUtils::keyShiftIsPressed())
        return false;

    return QAbstractItemView::edit(index, trigger, event);
}

void CollectionView::keyboardSearch(const QString &search)
{
    if (search.isEmpty())
        return;

    if (CollectionHookInterface::keyboardSearch(id(), search))
        return;

    bool reverseOrder = qApp->keyboardModifiers() == Qt::ShiftModifier;
    d->searchKeys.append(search);
    QModelIndex current = currentIndex();
    QModelIndex index = d->findIndex(d->searchKeys, true, current, reverseOrder, !d->searchTimer->isActive());

    if (index.isValid()) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        setCurrentIndex(index);
    }

    d->searchTimer->start();
}

QVariant CollectionView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    // When no item is selected, return input method area where the current mouse is located
    if (query == Qt::ImCursorRectangle && !currentIndex().isValid())
        return QRect(mapFromGlobal(QCursor::pos()), iconSize());

    return QAbstractItemView::inputMethodQuery(query);
}

void CollectionView::sort(int role)
{
    // toggle order if resort
    if (role == d->sortRole)
        d->sortOrder = d->sortOrder == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
    else
        d->sortOrder = Qt::AscendingOrder;

    d->sortRole = role;
    auto items = d->provider->items(d->id);
    if (items.isEmpty()) {
        fmDebug() << "No items to sort in collection:" << d->id;
        return;
    }

    std::sort(items.begin(), items.end(), [this](const QUrl &left, const QUrl &right) {
        return lessThan(left, right);
    });

    d->provider->sorted(d->id, items);
    return;
}

void CollectionView::toggleSelect()
{
    d->toggleSelect();
}

void CollectionView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QAbstractItemView::currentChanged(current, previous);

    // WA_InputMethodEnabled will be set to false if current index is invalid in QAbstractItemView::currentChanged
    // To enable WA_InputMethodEnabled no matter whether the current index is valid or not.
    if (!testAttribute(Qt::WA_InputMethodEnabled))
        setAttribute(Qt::WA_InputMethodEnabled, true);
}

GraphicsEffect::GraphicsEffect(CollectionView *parent)
    : QGraphicsEffect(parent), view(parent)
{
}

void GraphicsEffect::draw(QPainter *painter)
{
    const int opacity = view->viewport()->height() * view->viewport()->devicePixelRatioF() * 0.05;

    bool drawTop = false;
    bool drawBottom = false;
    {
        const int currentScroll = view->verticalScrollBar()->value();
        const int maxScroll = view->verticalScrollBar()->maximum();
        const int minScroll = view->verticalScrollBar()->minimum();
        if (opacity > 1) {
            if (minScroll != maxScroll) {
                if (currentScroll == maxScroll)
                    drawTop = true;
                else if (currentScroll == minScroll)
                    drawBottom = true;
                else {
                    drawTop = true;
                    drawBottom = true;
                }
            }
        }
    }

    if (!drawTop && !drawBottom) {
        drawSource(painter);
        return;
    }

    QPoint offset;
    const QPixmap source = sourcePixmap(Qt::LogicalCoordinates, &offset, QGraphicsEffect::NoPad);
    if (source.isNull())
        return;

    const auto size = source.size();
    painter->save();

    QPixmap target(source.size());
    target.fill(Qt::transparent);
    QPainter pixmapPainter(&target);
    pixmapPainter.setRenderHints(painter->renderHints());

    int begin = 0;
    if (drawTop) {
        pixmapPainter.save();
        for (; begin < opacity; ++begin) {
            QRect rect(0, begin, size.width(), 1);
            pixmapPainter.setOpacity((begin) / qreal(opacity));
            pixmapPainter.drawPixmap(rect, source, rect);
        }
        pixmapPainter.restore();
    }

    if (drawBottom) {
        pixmapPainter.save();

        const QRect content(0, begin, size.width(), size.height() - begin - opacity);
        pixmapPainter.drawPixmap(content, source, content);

        int bottom = size.height() - opacity;
        for (int i = 0; i < opacity; ++i) {
            QRect rect(0, bottom + i, size.width(), 1);
            pixmapPainter.setOpacity((opacity - i) / qreal(opacity));
            pixmapPainter.drawPixmap(rect, source, rect);
        }

        pixmapPainter.restore();
    } else {
        const QRect content(0, begin, size.width(), size.height() - begin);
        pixmapPainter.drawPixmap(content, source, content);
    }

    target.setDevicePixelRatio(source.devicePixelRatioF());
    painter->drawPixmap(offset, target);
    painter->restore();
}

void GraphicsEffect::sourceChanged(QGraphicsEffect::ChangeFlags flags)
{
    updateBoundingRect();
    update();
}

QRectF GraphicsEffect::boundingRectFor(const QRectF &sourceRect) const
{
    return sourceRect;
}
