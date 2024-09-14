// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarwidget.h"
#include "sidebarview.h"
#include "sidebarmodel.h"
#include "sidebaritem.h"
#include "utils/sidebarinfocachemananger.h"
#include "utils/fileoperatorhelper.h"
#include "utils/sidebarhelper.h"
#include "private/sidebarview_p.h"
#include "events/sidebareventcaller.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/sysinfoutils.h>

#include <dfm-framework/dpf.h>

#include <QtConcurrent>
#include <QDebug>
#include <QMimeData>
#include <QApplication>
#include <QMouseEvent>
#include <QUrl>
#include <QProxyStyle>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>
#include <QStyledItemDelegate>
#include <QDrag>
#include <QTextLayout>
#include <QScrollBar>

#include <unistd.h>

Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(const char *)

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

SideBarViewPrivate::SideBarViewPrivate(SideBarView *qq)
    : QObject(qq), q(qq)
{
}

void SideBarViewPrivate::currentChanged(const QModelIndex &curIndex)
{
    SideBarModel *mod = q->model();
    Q_ASSERT(mod);
    SideBarItem *item = mod->itemFromIndex(curIndex);
    if (dynamic_cast<SideBarItemSeparator *>(item))
        return;

    this->previous = current;
    current = curIndex;
    sidebarUrl = curIndex.data(SideBarItem::kItemUrlRole).toUrl();
}

void SideBarViewPrivate::onItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    SideBarItem *item = q->model()->itemFromIndex(index);
    if (!dynamic_cast<SideBarItemSeparator *>(item))
        return;

    q->onChangeExpandState(index, !q->isExpanded(index));
}

void SideBarViewPrivate::notifyOrderChanged()
{
    if (draggedGroup.isEmpty())
        return;

    QTimer::singleShot(0, this, [=] {   // this must be invoked after items are sorted finished
        quint64 winId = FMWindowsIns.findWindowId(q);
        dpfSignalDispatcher->publish("dfmplugin_sidebar", "signal_Sidebar_Sorted", winId, draggedGroup);
        draggedGroup = "";
    });
}

void SideBarViewPrivate::updateDFMMimeData(const QDropEvent *event)
{
    dfmMimeData.clear();
    const QMimeData *data = event->mimeData();

    if (data && data->hasFormat(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey))
        dfmMimeData = DFMMimeData::fromByteArray(data->data(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey));
}

bool SideBarViewPrivate::checkTargetEnable(const QUrl &targetUrl)
{
    if (!dfmMimeData.isValid())
        return true;

    if (FileUtils::isTrashFile(targetUrl))
        return dfmMimeData.canTrash() || dfmMimeData.canDelete();

    return true;
}

bool SideBarViewPrivate::canEnter(QDragEnterEvent *event)
{
    Q_ASSERT(q);

    if (!event)
        return false;

    if (urlsForDragEvent.isEmpty() || FileUtils::isContainProhibitPath(urlsForDragEvent))
        return false;

    SideBarItem *item = q->itemAt(event->pos());
    if (item) {
        const QUrl &targetItemUrl { item->targetUrl() };
        if (!checkTargetEnable(targetItemUrl))
            return false;
    }

    return true;
}

bool SideBarViewPrivate::canMove(QDragMoveEvent *event)
{
    Q_ASSERT(q);

    if (!event)
        return false;

    const QList<QUrl> &urls = urlsForDragEvent.isEmpty()
            ? event->mimeData()->urls()
            : urlsForDragEvent;

    if (!urls.isEmpty()) {
        SideBarItem *item = q->itemAt(event->pos());
        if (!item)
            return false;

        const QUrl &targetItemUrl { item->targetUrl() };
        if (!checkTargetEnable(targetItemUrl))
            return false;

        Qt::DropAction action { Qt::CopyAction };
        if (dpfHookSequence->run("dfmplugin_sidebar", "hook_Item_DragMoveData", urls, item->url(), &action)) {
            if (action == Qt::IgnoreAction)
                return false;
        }
    }

    return true;
}

class SidebarViewStyle : public QProxyStyle
{
public:
    SidebarViewStyle(QStyle *style = nullptr);

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const;
};

SidebarViewStyle::SidebarViewStyle(QStyle *style)
    : QProxyStyle(style)
{
}

void SidebarViewStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::PE_IndicatorItemViewItemDrop && !option->rect.isNull()) {
        painter->setRenderHint(QPainter::Antialiasing);
        QStyleOption opt(*option);
        opt.rect.setLeft(0);
        if (widget)
            opt.rect.setRight(widget->width());
        DPalette pl(DPaletteHelper::instance()->palette(widget));
        QColor bgColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
        QPalette::ColorGroup colorGroup = QPalette::Normal;
        bgColor = opt.palette.color(colorGroup, QPalette::Highlight);
        QPen pen = painter->pen();
        pen.setColor(bgColor);
        pen.setWidth(2);
        painter->setPen(pen);

        QPoint posLeft = opt.rect.topLeft();
        QPoint posRight = opt.rect.bottomRight();
        painter->drawRoundedRect(QRect(posLeft + QPoint(10, 0), posRight + QPoint(-10, 0)), 8, 8);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

SideBarView::SideBarView(QWidget *parent)
    : DTreeView(parent), d(new SideBarViewPrivate(this))
{
    setRootIsDecorated(false);
    setIndentation(0);
#ifdef QT_SCROLL_WHEEL_ANI
    QScrollBar *bar = verticalScrollBar();
    bar->setSingleStep(1);
    setVerticalScrollBarPolicy(Qt::ScrollBarSlideAnimationOn);
#endif
    setVerticalScrollMode(ScrollPerPixel);
    setIconSize(QSize(16, 16));
    setHeaderHidden(true);
    setMouseTracking(true);   // sp3 feature 35，解除注释以便鼠标在移动时就能触发 mousemoveevent

    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(false);

    connect(this, &DTreeView::clicked, d, &SideBarViewPrivate::currentChanged);
    connect(this, &DTreeView::doubleClicked, d, &SideBarViewPrivate::onItemDoubleClicked);

    d->lastOpTime = 0;

    setStyle(new SidebarViewStyle(style()));
}

SideBarModel *SideBarView::model() const
{
    return qobject_cast<SideBarModel *>(QAbstractItemView::model());
}

void SideBarView::mousePressEvent(QMouseEvent *event)
{
    //频繁点击操作与网络或挂载设备的加载效率低两个因素的共同作用下 会导致侧边栏可能出现显示错误
    //暂时抛去部分频繁点击来规避这个问题
    if (!d->checkOpTime())
        return;

    d->draggedUrl = urlAt(event->pos());
    auto item = itemAt(event->pos());
    d->draggedGroup = item ? item->group() : "";

    if (event->button() == Qt::RightButton) {
        // fix bug#33502 鼠标挪动到侧边栏底部右键，滚动条滑动，不能定位到选中的栏目上
        event->accept();
        return;
    }

    DTreeView::mousePressEvent(event);
}

void SideBarView::mouseReleaseEvent(QMouseEvent *event)
{
    d->draggedUrl = QUrl {};
    d->draggedGroup.clear();

    QModelIndex index = indexAt(event->pos());
    if (index.isValid() && index.data(SideBarItem::kItemTypeRole) == SideBarItem::kSidebarItem) {
        const QUrl &url = index.data(SideBarItem::kItemUrlRole).toUrl();
        SideBarItem *item = itemAt(event->pos());
        if (url.isValid() && item) {
            auto info = item->itemInfo();
            QString reportName = info.reportName;
            QVariantMap data;
            data.insert("sidebar_item", reportName);

            dpfSignalDispatcher->publish("dfmplugin_sidebar", "signal_ReportLog_Commit", QString("Sidebar"), data);
        }
    }

    DTreeView::mouseReleaseEvent(event);
}

void SideBarView::dragEnterEvent(QDragEnterEvent *event)
{
    d->currentHoverIndex = QModelIndex();
    d->updateDFMMimeData(event);
    if (event->source() != this) {
        d->urlsForDragEvent = d->dfmMimeData.isValid() ? d->dfmMimeData.urls() : event->mimeData()->urls();
        // treeveiew drop urls
        if (event->mimeData()->formats().contains(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey)) {
            auto treeUrlsStr = QString(event->mimeData()->data(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey));
            auto treeUrlss = treeUrlsStr.split("\n");
            QList<QUrl> treeSelectUrl;
            for (const auto &url : treeUrlss) {
                if (url.isEmpty())
                    continue;
                treeSelectUrl.append(QUrl(url));
            }
            if (!treeUrlss.isEmpty())
                d->urlsForDragEvent = treeSelectUrl;
        }
        if (!d->canEnter(event)) {
            event->setDropAction(Qt::IgnoreAction);
            event->ignore();
            return;
        }

    } else {
        d->urlsForDragEvent.clear();
        d->isItemDragged = true;
    }

    d->previousRowCount = model()->rowCount();
    if (isAccepteDragEvent(event))
        return;

    DTreeView::dragEnterEvent(event);

    if (event->source() != this) {
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
    }
}

void SideBarView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->source() != this)
        d->currentHoverIndex = indexAt(event->pos());

    SideBarItem *item = itemAt(event->pos());
    if (item) {
        viewport()->update();
        if (!d->canMove(event)) {
            event->setDropAction(Qt::IgnoreAction);
            event->ignore();
            return;
        }
    }

    if (isAccepteDragEvent(event))
        return;

    DTreeView::dragMoveEvent(event);

    if (event->source() != this)
        event->ignore();
}

void SideBarView::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    d->draggedUrl = QUrl("");
    d->isItemDragged = false;
    setState(State::NoState);

    if (d->currentHoverIndex.isValid()) {
        update(d->currentHoverIndex);
        d->currentHoverIndex = QModelIndex();
    }
}

void SideBarView::dropEvent(QDropEvent *event)
{
    d->currentHoverIndex = QModelIndex();
    d->isItemDragged = false;
    if (d->draggedUrl.isValid()) {   // select the dragged item when dropped.
        d->notifyOrderChanged();   // notify to update the persistence data
    }

    d->dropPos = event->pos();
    SideBarItem *item = itemAt(event->pos());
    if (!item)
        return DTreeView::dropEvent(event);

    const QUrl &targetItemUrl { item->targetUrl() };

    fmDebug() << "source: " << event->mimeData()->urls();
    fmDebug() << "target item: " << item->group() << "|" << item->text() << "|" << item->url();
    fmDebug() << "item->itemInfo().finalUrl: " << item->itemInfo().finalUrl;

    // wayland环境下QCursor::pos()在此场景中不能获取正确的光标当前位置，代替方案为直接使用QDropEvent::pos()
    // QDropEvent::pos() 实际上就是drop发生时光标在该widget坐标系中的position (mapFromGlobal(QCursor::pos()))
    //但rc本来就是由event->pos()计算item得出的Rect，这样判断似乎就没有意义了（虽然原来的逻辑感觉也没什么意义）
    QPoint pt = event->pos();   // mapFromGlobal(QCursor::pos());
    QRect rc = visualRect(indexAt(event->pos()));
    if (!rc.contains(pt)) {
        fmDebug() << "mouse not in my area";
        return DTreeView::dropEvent(event);
    }

    // bug case 24499, 这里需要区分哪些是可读的文件 或文件夹，因为其权限是不一样的，所以需要对不同权限的文件进行区分处理
    // 主要有4种场景：1.都是可读写的场景; 2.文件夹是只读属性，子集是可读写的; 3.文件夹或文件是可读写的; 4.拖动的包含 可读写的和只读的
    QList<QUrl> urls, copyUrls;
    for (const QUrl &url : d->urlsForDragEvent) {
        if (UrlRoute::isRootUrl(url)) {
            fmDebug() << "skip the same dir file..." << url;
        } else {
            if (dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileCanMove", url)) {
                urls << url;
                continue;
            }

            QString folderPath = UrlRoute::urlToPath(UrlRoute::urlParent(url));

            bool isFolderWritable = false;

            QFileInfo folderinfo(folderPath);   // 判断上层文件是否是只读，有可能上层是只读，而里面子文件或文件夾又是可以写

            isFolderWritable = folderinfo.isWritable();

            if (!isFolderWritable) {
                copyUrls << QUrl(url);
                fmDebug() << "this is a unwriteable case:" << url;
            } else {
                urls << QUrl(url);
            }
        }
    }

    bool isActionDone = false;
    if (!urls.isEmpty()) {
        Qt::DropAction action = canDropMimeData(item, event->mimeData(), Qt::MoveAction);
        if (action == Qt::IgnoreAction) {
            action = canDropMimeData(item, event->mimeData(), event->possibleActions());
        }
        event->setDropAction(action);
        action = event->dropAction();

        if (urls.size() > 0 && onDropData(urls, targetItemUrl, action)) {
            event->setDropAction(action);
            isActionDone = true;
        }
    }
    if (!copyUrls.isEmpty()) {
        if (onDropData(copyUrls, targetItemUrl, Qt::CopyAction)) {   // 对于只读权限的，只能进行 copy动作
            event->setDropAction(Qt::CopyAction);
            isActionDone = true;
        }
    }

    if (isActionDone) {
        // fix bug 24478,在drop事件完成时，设置当前窗口为激活窗口，crtl+z就能找到正确的回退
        QWidget *parentPtr = parentWidget();
        QWidget *curWindow = nullptr;
        while (parentPtr) {
            curWindow = parentPtr;
            parentPtr = parentPtr->parentWidget();
        }
        if (curWindow)
            qApp->setActiveWindow(curWindow);

        event->accept();
    } else {
        DTreeView::dropEvent(event);
    }
}

void SideBarView::startDrag(Qt::DropActions supportedActions)
{
    if (!d->draggedUrl.isValid())
        return;
    d->isItemDragged = true;
    DTreeView::startDrag(supportedActions);
}

QModelIndex SideBarView::indexAt(const QPoint &p) const
{
    return QTreeView::indexAt(p);
}

bool SideBarView::onDropData(QList<QUrl> srcUrls, QUrl dstUrl, Qt::DropAction action) const
{
    if (dpfHookSequence->run("dfmplugin_sidebar", "hook_Item_DropData", srcUrls, dstUrl, &action)) {
        if (action == Qt::IgnoreAction)
            return true;
    }

    auto dstInfo = InfoFactory::create<FileInfo>(dstUrl);

    // convert destnation url to real path if it's a symbol link.
    if (dstInfo->isAttributes(OptInfoType::kIsSymLink))
        dstUrl = QUrl::fromLocalFile(dstInfo->pathOf(PathInfoType::kSymLinkTarget));

    auto winId = SideBarHelper::windowId(qobject_cast<QWidget *>(parent()));

    switch (action) {
    case Qt::CopyAction:
        // blumia: should run in another thread or user won't do another DnD opreation unless the copy action done.
        QTimer::singleShot(0, const_cast<SideBarView *>(this), [=]() {
            if (srcUrls.count() > 0)
                FileOperatorHelperIns->pasteFiles(winId, srcUrls, dstUrl, action);
        });
        break;
    case Qt::LinkAction:
        break;
    case Qt::MoveAction:
        if (srcUrls.count() > 0)
            FileOperatorHelperIns->pasteFiles(winId, srcUrls, dstUrl, action);
        break;
    default:
        return false;
    }

    return true;
}

SideBarItem *SideBarView::itemAt(const QPoint &pt) const
{
    SideBarItem *item = nullptr;
    QModelIndex index = indexAt(pt);
    if (!index.isValid()) {
        return item;
    }

    SideBarModel *mod = model();
    Q_ASSERT(mod);
    item = mod->itemFromIndex(index);
    Q_ASSERT(item);

    return item;
}

QUrl SideBarView::urlAt(const QPoint &pt) const
{
    SideBarItem *item = itemAt(pt);
    if (!item)
        return QUrl("");
    return item->url();
}

void SideBarView::saveStateWhenClose()
{
    if (SideBarHelper::groupExpandRules().isEmpty())
        return;
    if (d->groupExpandState.isEmpty())
        d->groupExpandState = SideBarHelper::groupExpandRules();

    SideBarHelper::saveGroupsStateToConfig(d->groupExpandState);
}

void SideBarView::setCurrentUrl(const QUrl &url)
{
    d->sidebarUrl = url;
    bool urlNotChanged = UniversalUtils::urlEquals(d->current.data(SideBarItem::kItemUrlRole).toUrl(), url);
    const QModelIndex &index = urlNotChanged ? d->current : findItemIndex(url);

    if (!index.isValid()) {
        const QModelIndex &checkIndex = findItemIndex(url);
        if (checkIndex.isValid()) {
            d->current = checkIndex;
        } else {
            this->clearSelection();
            return;
        }
    }
    SideBarModel *sidebarModel = dynamic_cast<SideBarModel *>(model());
    if (!sidebarModel)
        return;
    SideBarItem *currentItem = sidebarModel->itemFromIndex(index);
    if (currentItem && currentItem->parent()) {
        SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(currentItem->parent());
        // If the current item's group is not expanded, do not set current index, otherwise
        // the unexpanded group would be expaned again.
        if (groupItem && !groupItem->isExpanded())
            return;
    }

    this->setCurrentIndex(index);
    d->current = index;
    if (!d->previous.isValid()) {
        d->previous = index;
    }
}

QUrl SideBarView::currentUrl() const
{
    return d->sidebarUrl;
}

QModelIndex SideBarView::findItemIndex(const QUrl &url) const
{
    SideBarModel *sidebarModel = dynamic_cast<SideBarModel *>(model());
    if (!sidebarModel)
        return QModelIndex();

    int count = sidebarModel->rowCount();
    for (int i = 0; i < count; i++) {
        SideBarItem *topItem = sidebarModel->itemFromIndex(i);
        SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(topItem);
        if (groupItem) {
            int childCount = groupItem->rowCount();
            for (int j = 0; j < childCount; j++) {
                QStandardItem *childItem = groupItem->child(j);
                SideBarItem *item = static_cast<SideBarItem *>(childItem);
                if (!item)
                    continue;
                bool foundByCb = item->itemInfo().findMeCb && item->itemInfo().findMeCb(item->url(), url);
                if (foundByCb || UniversalUtils::urlEquals(item->url(), url))
                    return item->index();
            }
        }
    }

    return QModelIndex();
}

QVariantMap SideBarView::groupExpandState() const
{
    return d->groupExpandState;
}

QModelIndex SideBarView::previousIndex() const
{
    return d->previous;
}

void SideBarView::setPreviousIndex(const QModelIndex &index)
{
    d->previous = index;
}

bool SideBarView::isDropTarget(const QModelIndex &index)
{
    return index == d->currentHoverIndex;
}

bool SideBarView::isSideBarItemDragged()
{
    return d->isItemDragged;
}

Qt::DropAction SideBarView::canDropMimeData(SideBarItem *item, const QMimeData *data, Qt::DropActions actions) const
{
    // Got a copy of urls so whatever data was changed, it won't affact the following code.
    QList<QUrl> urls = d->urlsForDragEvent;
    if (urls.empty()) {
        return Qt::IgnoreAction;
    }

    QUrl targetItemUrl;
    if (!item->itemInfo().finalUrl.isEmpty()) {
        targetItemUrl = item->itemInfo().finalUrl;
    } else {
        targetItemUrl = item->url();
    }

    if (!targetItemUrl.isValid())
        return Qt::IgnoreAction;

    auto itemInfo = InfoFactory::create<FileInfo>(targetItemUrl);
    if (!itemInfo || !itemInfo->canAttributes(CanableInfoType::kCanDrop)) {
        return Qt::IgnoreAction;
    }
    if (itemInfo->fileType() == FileInfo::FileType::kDirectory) {
        // when the dir not have writeable and executable permissions, then can not drop
        if (!itemInfo->isAttributes(OptInfoType::kIsExecutable) || !itemInfo->isAttributes(OptInfoType::kIsWritable))
            return Qt::IgnoreAction;
    }

    for (const QUrl &url : urls) {
        if (UniversalUtils::urlEquals(targetItemUrl, UrlRoute::urlParent(url)))
            return Qt::IgnoreAction;

        auto fileInfo = InfoFactory::create<FileInfo>(url);
        if (!fileInfo)
            return Qt::IgnoreAction;

        if (!fileInfo->isAttributes(OptInfoType::kIsReadable)) {
            return Qt::IgnoreAction;
        }
        //部分文件不能复制或剪切，需要在拖拽时忽略
        if (!fileInfo->canAttributes(CanableInfoType::kCanMoveOrCopy)) {
            return Qt::IgnoreAction;
        }
    }

    Qt::DropAction action = Qt::IgnoreAction;
    const Qt::DropActions supportActions = itemInfo->supportedOfAttributes(SupportedType::kDrop) & actions;

    if (supportActions.testFlag(Qt::CopyAction)) {
        action = Qt::CopyAction;
    }

    if (supportActions.testFlag(Qt::MoveAction)) {
        action = Qt::MoveAction;
    }

    if (supportActions.testFlag(Qt::LinkAction)) {
        action = Qt::LinkAction;
    }

    if (qApp->keyboardModifiers() == Qt::AltModifier) {
        action = Qt::MoveAction;
    } else if (qApp->queryKeyboardModifiers() == Qt::ControlModifier) {
        if (action == Qt::MoveAction)
            action = Qt::CopyAction;
    } else if (FileUtils::isSameDevice(urls.first(), targetItemUrl)) {
        action = Qt::MoveAction;
    } else if (action == Qt::MoveAction && !FileUtils::isSameDevice(urls.first(), targetItemUrl)) {
        action = Qt::CopyAction;
    }

    if (FileUtils::isTrashFile(targetItemUrl) && !SysInfoUtils::isSameUser(data))
        action = Qt::IgnoreAction;

    return action;
}

bool SideBarView::isAccepteDragEvent(QDropEvent *event)
{
    SideBarItem *item = itemAt(event->pos());
    if (!item) {
        return false;
    }

    bool accept = false;
    Qt::DropAction action = canDropMimeData(item, event->mimeData(), event->proposedAction());
    if (action == Qt::IgnoreAction) {
        action = canDropMimeData(item, event->mimeData(), event->possibleActions());
    }

    if (action != Qt::IgnoreAction) {
        event->setDropAction(action);
        event->accept();
        accept = true;
    }

    return accept;
}

QString SideBarView::dragEventUrls() const
{
    return ((getuid() == 0) ? (QString(getlogin()) + "_RootUrlsInDragEvent") : (QString(getlogin()) + "_UrlsInDragEvent"));
}

void SideBarView::updateSeparatorVisibleState()
{
    QVariantMap lastWindowGroupState;
    //`groupExpandState` is empty means that user have not operated the group expanding state and
    // the group expanding state must be got from the previous actived window.
    if (d->groupExpandState.isEmpty()) {
        auto lastActivedWinId = FMWindowsIns.previousActivedWindowId();
        auto win = FMWindowsIns.findWindowById(lastActivedWinId);
        if (win) {
            SideBarWidget *sb = dynamic_cast<SideBarWidget *>(win->sideBar());
            if (sb) {
                SideBarView *view = dynamic_cast<SideBarView *>(sb->view());
                if (view) {
                    lastWindowGroupState = view->groupExpandState();
                    if (!lastWindowGroupState.isEmpty())
                        d->groupExpandState = lastWindowGroupState;   // synchronize the sidebar group expanding state from the previous actived window
                }
            }
        }
    }

    QString lastGroupName = DefaultGroup::kNotExistedGroup;
    bool allItemsInvisiable = true;
    SideBarModel *sidebarModel = dynamic_cast<SideBarModel *>(model());
    if (!sidebarModel)
        return;
    for (int i = 0; i < sidebarModel->rowCount(); i++) {
        SideBarItem *item = sidebarModel->itemFromIndex(i);   // top item
        if (item)
            allItemsInvisiable = false;
        if (item->group() != lastGroupName) {
            SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(item);
            if (groupItem) {   // Separator
                QVariantMap temGroupExpandState;
                if (d->groupExpandState.isEmpty())
                    temGroupExpandState = SideBarHelper::groupExpandRules();
                else
                    temGroupExpandState = d->groupExpandState;

                bool groupExpaned = temGroupExpandState.value(groupItem->group(), true).toBool();
                groupItem->setExpanded(groupExpaned);
                this->setExpanded(groupItem->index(), groupExpaned);
                int childCount = item->rowCount();
                bool allChildIsHiden = true;
                for (int j = 0; j < childCount; j++) {
                    if (!groupItem->index().isValid())
                        continue;
                    bool hiden = this->isRowHidden(j, groupItem->index());
                    if (!hiden) {
                        allChildIsHiden = false;
                        break;
                    }
                }
                if (allChildIsHiden || childCount <= 0)   // The top item dont have child item or they are hiden
                    this->setRowHidden(i, QModelIndex(), true);
                else   // sub item
                    this->setRowHidden(i, QModelIndex(), false);   // The other top be shown include its children

                lastGroupName = item->group();
            }
        }
    }

    // when no item is visiable in sidebar, do something, such as hide sidebar?
    if (allItemsInvisiable)
        fmDebug() << "nothing in sidebar is visiable, maybe hide sidebar?";
}

void SideBarView::onChangeExpandState(const QModelIndex &index, bool expand)
{
    SideBarModel *sidebarModel = dynamic_cast<SideBarModel *>(model());
    if (!sidebarModel)
        return;
    SideBarItem *item = sidebarModel->itemFromIndex(index);
    if (!item)
        return;

    this->setExpanded(index, expand);

    SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(item);
    if (groupItem) {
        groupItem->setExpanded(expand);
        const QVariantMap &gMap = SideBarHelper::groupExpandRules();
        if (d->groupExpandState.isEmpty() && !gMap.isEmpty())
            d->groupExpandState = gMap;

        if (d->groupExpandState.keys().contains(groupItem->group()))
            d->groupExpandState[groupItem->group()] = expand;

        if (expand)
            setCurrentUrl(d->sidebarUrl);   // To make sure, when expand the group item, the current item is highlighted.
    }
    update(index);
}

bool SideBarViewPrivate::checkOpTime()
{
    // If the interval between twice checking, then return true.
    if (QDateTime::currentDateTime().toMSecsSinceEpoch() - lastOpTime > 200) {
        lastOpTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        return true;
    }

    return false;
}
