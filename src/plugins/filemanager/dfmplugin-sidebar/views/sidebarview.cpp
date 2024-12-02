// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarview.h"
#include "models/sidebarmodel.h"
#include "views/sidebaritem.h"
#include "views/private/sidebarview_p.h"
#include "utils/fileoperatorhelper.h"
#include "utils/sidebarhelper.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/dpf.h>

#include <QtConcurrent>
#include <QDebug>
#include <QMimeData>
#include <QApplication>
#include <QMouseEvent>

#include <unistd.h>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#define DRAG_EVENT_URLS ((getuid() == 0) ? (QString(getlogin()) + "_RootUrlsInDragEvent") : (QString(getlogin()) + "_UrlsInDragEvent"))

SideBarViewPrivate::SideBarViewPrivate(SideBarView *qq)
    : QObject(qq), q(qq)
{
}

void SideBarViewPrivate::currentChanged(const QModelIndex &previous)
{
    current = q->currentIndex();
    this->previous = previous;
}

void SideBarViewPrivate::highlightAfterDraggedToSort()
{
    QTimer::singleShot(0, this, [=] {   // this must be invoked after items are sorted finished
        auto ret = q->model()->findRowByUrl(draggedUrl);
        if (ret >= 0)
            q->setCurrentIndex(q->model()->index(ret, 0));
        draggedUrl = QUrl {};
    });
}

void SideBarViewPrivate::notifyOrderChanged()
{
    if (draggedGroup.isEmpty())
        return;

    QTimer::singleShot(0, this, [=] {   // this must be invoked after items are sorted finished
        quint64 winId = FMWindowsIns.findWindowId(q);
        dpfSignalDispatcher->publish("dfmplugin_sidebar", "signal_Sidebar_Sorted", winId, draggedGroup);
        SideBarHelper::updateSideBarSelection(winId);
        draggedGroup = "";
    });
}

SideBarView::SideBarView(QWidget *parent)
    : DListView(parent), d(new SideBarViewPrivate(this))
{
    setVerticalScrollMode(ScrollPerPixel);
    setIconSize(QSize(16, 16));
    //    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setMouseTracking(true);   // sp3 feature 35，解除注释以便鼠标在移动时就能触发 mousemoveevent

    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(false);
    //QListView拖拽时会先插入后删除，于是可以通过rowCountChanged()信号来判断拖拽操作是否结束
    QObject::connect(this, static_cast<void (DListView::*)(const QModelIndex &)>(&DListView::currentChanged),
                     d, &SideBarViewPrivate::currentChanged);

    d->lastOpTime = 0;
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
        //fix bug#33502 鼠标挪动到侧边栏底部右键，滚动条滑动，不能定位到选中的栏目上
        event->accept();
        return;
    }
    DListView::mousePressEvent(event);
}

void SideBarView::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);
#if QT_CONFIG(draganddrop)
    if (state() == DraggingState) {
        startDrag(Qt::MoveAction);
        setState(NoState);   // the startDrag will return when the dnd operation is done
        stopAutoScroll();
        QPoint pt = mapFromGlobal(QCursor::pos());
        QRect rc = geometry();
        if (!rc.contains(pt)) {
            Q_EMIT requestRemoveItem();   // model()->removeRow(currentIndex().row());
        }
    }
#endif   // QT_CONFIG(draganddrop)
}

void SideBarView::mouseReleaseEvent(QMouseEvent *event)
{
    d->draggedUrl = QUrl {};
    d->draggedGroup.clear();
    DListView::mouseReleaseEvent(event);
}

void SideBarView::dragEnterEvent(QDragEnterEvent *event)
{
    d->previousRowCount = model()->rowCount();
    d->urlsForDragEvent = event->mimeData()->urls();

    if (isAccepteDragEvent(event)) {
        return;
    }

    DListView::dragEnterEvent(event);

    if (event->source() != this) {
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
    }
}

void SideBarView::dragMoveEvent(QDragMoveEvent *event)
{
    if (isAccepteDragEvent(event)) {
        return;
    }

    DListView::dragMoveEvent(event);

    if (event->source() != this) {
        event->ignore();
    }
}

void SideBarView::dropEvent(QDropEvent *event)
{
    if (d->draggedUrl.isValid()) {   // select the dragged item when dropped.
        d->notifyOrderChanged();   // notify to update the persistence data
        d->highlightAfterDraggedToSort();
    }

    d->dropPos = event->pos();
    SideBarItem *item = itemAt(event->pos());
    if (!item) {
        return DListView::dropEvent(event);
    }

    qDebug() << "source: " << event->mimeData()->urls();
    qDebug() << "target item: " << item->group() << "|" << item->text() << "|" << item->url();

    //wayland环境下QCursor::pos()在此场景中不能获取正确的光标当前位置，代替方案为直接使用QDropEvent::pos()
    //QDropEvent::pos() 实际上就是drop发生时光标在该widget坐标系中的position (mapFromGlobal(QCursor::pos()))
    //但rc本来就是由event->pos()计算item得出的Rect，这样判断似乎就没有意义了（虽然原来的逻辑感觉也没什么意义）
    QPoint pt = event->pos();   //mapFromGlobal(QCursor::pos());
    QRect rc = visualRect(indexAt(event->pos()));
    if (!rc.contains(pt)) {
        qDebug() << "mouse not in my area";
        return DListView::dropEvent(event);
    }

    // bug case 24499, 这里需要区分哪些是可读的文件 或文件夹，因为其权限是不一样的，所以需要对不同权限的文件进行区分处理
    // 主要有4种场景：1.都是可读写的场景; 2.文件夹是只读属性，子集是可读写的; 3.文件夹或文件是可读写的; 4.拖动的包含 可读写的和只读的
    QList<QUrl> urls, copyUrls;
    for (const QUrl &url : d->urlsForDragEvent) {
        if (UrlRoute::isRootUrl(url)) {
            qDebug() << "skip the same dir file..." << url;
        } else {
            QString folderPath = UrlRoute::urlToPath(UrlRoute::urlParent(url));

            bool isFolderWritable = false;

            QFileInfo folderinfo(folderPath);   // 判断上层文件是否是只读，有可能上层是只读，而里面子文件或文件夾又是可以写

            isFolderWritable = folderinfo.isWritable();

            if (!isFolderWritable) {
                copyUrls << QUrl(url);
                qDebug() << "this is a unwriteable case:" << url;
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

        if (urls.size() > 0 && onDropData(urls, item->url(), action)) {
            event->setDropAction(action);
            isActionDone = true;
        }
    }
    if (!copyUrls.isEmpty()) {
        if (onDropData(copyUrls, item->url(), Qt::CopyAction)) {   // 对于只读权限的，只能进行 copy动作
            event->setDropAction(Qt::CopyAction);
            isActionDone = true;
        }
    }

    if (isActionDone) {
        //fix bug 24478,在drop事件完成时，设置当前窗口为激活窗口，crtl+z就能找到正确的回退
        QWidget *parentptr = parentWidget();
        QWidget *curwindow = nullptr;
        while (parentptr) {
            curwindow = parentptr;
            parentptr = parentptr->parentWidget();
        }
        if (curwindow) {
            qApp->setActiveWindow(curwindow);
        }

        event->accept();
    } else {
        DListView::dropEvent(event);
    }
}

QModelIndex SideBarView::indexAt(const QPoint &p) const
{
    return QListView::indexAt(p);
}

QModelIndex SideBarView::getPreviousIndex() const
{
    return d->previous;
}

QModelIndex SideBarView::getCurrentIndex() const
{
    return d->current;
}

bool SideBarView::onDropData(QList<QUrl> srcUrls, QUrl dstUrl, Qt::DropAction action) const
{
    if (dpfHookSequence->run("dfmplugin_sidebar", "hook_Item_DropData", srcUrls, dstUrl, action))
        return true;

    auto dstInfo = InfoFactory::create<AbstractFileInfo>(dstUrl);

    // convert destnation url to real path if it's a symbol link.
    if (dstInfo->isSymLink())
        dstUrl = dstInfo->symLinkTarget();

    auto winId = SideBarHelper::windowId(qobject_cast<QWidget *>(parent()));

    switch (action) {
    case Qt::CopyAction:
        // blumia: should run in another thread or user won't do another DnD opreation unless the copy action done.
        QtConcurrent::run([=]() {
            FileOperatorHelperIns->pasteFiles(winId, srcUrls, dstUrl, action);
            //            DFileService::instance()->pasteFile(this, DFMGlobal::CopyAction, dstUrl, srcUrls);
        });
        break;
    case Qt::LinkAction:
        break;
    case Qt::MoveAction:
        FileOperatorHelperIns->pasteFiles(winId, srcUrls, dstUrl, action);
        //        DFileService::instance()->pasteFile(this, DFMGlobal::CutAction, dstUrl, srcUrls);
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

Qt::DropAction SideBarView::canDropMimeData(SideBarItem *item, const QMimeData *data, Qt::DropActions actions) const
{
    Q_UNUSED(data)
    // Got a copy of urls so whatever data was changed, it won't affact the following code.
    QList<QUrl> urls = d->urlsForDragEvent;

    if (urls.empty()) {
        return Qt::IgnoreAction;
    }

    // TODO(zhangs): impl me!

    auto itemInfo = InfoFactory::create<AbstractFileInfo>(item->url());
    if (!itemInfo || !itemInfo->canDrop()) {
        return Qt::IgnoreAction;
    }

    for (const QUrl &url : urls) {
        auto fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo->isReadable()) {
            return Qt::IgnoreAction;
        }
        //部分文件不能复制或剪切，需要在拖拽时忽略
        if (!fileInfo->canMoveOrCopy()) {
            return Qt::IgnoreAction;
        }
    }

    Qt::DropAction action = Qt::IgnoreAction;
    const Qt::DropActions support_actions = itemInfo->supportedDropActions() & actions;

    if (support_actions.testFlag(Qt::CopyAction)) {
        action = Qt::CopyAction;
    }

    if (support_actions.testFlag(Qt::MoveAction)) {
        action = Qt::MoveAction;
    }

    if (support_actions.testFlag(Qt::LinkAction)) {
        action = Qt::LinkAction;
    }

    if ((action == Qt::MoveAction) && qApp->keyboardModifiers() == Qt::ControlModifier) {
        action = Qt::CopyAction;
    }

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

bool SideBarViewPrivate::fetchDragEventUrlsFromSharedMemory()
{
    QSharedMemory sm;
    sm.setKey(DRAG_EVENT_URLS);

    if (!sm.isAttached()) {
        if (!sm.attach()) {
            qDebug() << "FQSharedMemory detach failed.";
            return false;
        }
    }

    QBuffer buffer;
    QDataStream in(&buffer);
    QList<QUrl> urls;

    sm.lock();
    //用缓冲区得到共享内存关联后得到的数据和数据大小
    buffer.setData((char *)sm.constData(), sm.size());
    buffer.open(QBuffer::ReadOnly);   //设置读取模式
    in >> urlsForDragEvent;   //使用数据流从缓冲区获得共享内存的数据，然后输出到字符串中
    sm.unlock();   //解锁
    sm.detach();   //与共享内存空间分离

    return true;
}

bool SideBarViewPrivate::checkOpTime()
{
    //如果两次操作时间间隔足够长，则返回true
    if (QDateTime::currentDateTime().toMSecsSinceEpoch() - lastOpTime > 200) {
        lastOpTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        return true;
    }

    return false;
}
