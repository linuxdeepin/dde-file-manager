// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarwidget.h"
#include "sidebarview.h"
#include "treemodels/sidebarmodel.h"
#include "sidebaritem.h"
#include "utils/fileoperatorhelper.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarinfocachemananger.h"
#include "utils/devicemountsubscriber.h"
#include "private/sidebarview_p.h"
#include "events/sidebareventcaller.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/traversaldirthread.h>

#include <dfm-framework/dpf.h>

#include <QtConcurrent>
#include <QDebug>
#include <QCursor>
#include <QMimeData>
#include <QApplication>
#include <memory>
#include <QMouseEvent>
#include <QUrl>
#include <QProxyStyle>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>
#include <QStyledItemDelegate>
#include <QDrag>
#include <QPainter>
#include <QTextLayout>
#include <QScrollBar>
#include <QScroller>
#include <QVariantAnimation>
#include <QPointer>
#include <QDir>

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
    if (dynamic_cast<SideBarItemSeparator *>(item)) {
        fmDebug() << "Current changed to separator item, ignoring";
        return;
    }

    this->previous = current;
    current = curIndex;
    sidebarUrl = curIndex.data(SideBarItem::kItemUrlRole).toUrl();
}

void SideBarViewPrivate::onItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        fmWarning() << "Double clicked on invalid index";
        return;
    }
    SideBarItem *item = q->model()->itemFromIndex(index);

    // Handle group separator expand/collapse.
    if (dynamic_cast<SideBarItemSeparator *>(item)) {
        q->onChangeExpandState(index, !q->isExpanded(index));
        return;
    }

    // Handle partition sub-directory expand/collapse.
    if (item && item->group() == DefaultGroup::kDevice) {
        if (!SideBarHelper::partitionExpandable()) {
            fmDebug() << "Partition expansion is disabled";
            return;
        }

        QUrl finalUrl = item->itemInfo().finalUrl;
        QUrl nodeUrl = item->url();
        if (nodeUrl.scheme() == "file")
            finalUrl = nodeUrl;

        bool deviceMounted = !finalUrl.isEmpty() && finalUrl.isValid();

        fmDebug() << "is mounted?" << deviceMounted << finalUrl;
        if (deviceMounted) {
            // Prevent a white-background flash during expansion by setting the
            // transparent palette before the async expand path runs.  Qt's
            // QAbstractItemView::mouseDoubleClickEvent may force a viewport
            // repaint (executePostedLayout) after the doubleClicked signal
            // is emitted; without the transparent palette that repaint shows
            // a white flicker.  onChangeExpandState (called later in the
            // async callback) also sets the transient palette and restores it.
            setTransparentPalette();
            expandPartitionItem(index, finalUrl);
        } else {
            // Device not yet mounted; subscribe to mount event and auto-expand later.
            // Cancel any previous subscription for the same device to avoid duplicate toggling.
            cancelPendingMountSubscription(nodeUrl);

            fmDebug() << "SideBarViewPrivate: Device not mounted, subscribing to mount events:" << nodeUrl;

            QModelIndex capturedIndex = index;
            QPointer<SideBarView> view = q;
            QPointer<SideBarViewPrivate> guard(this);
            QUrl deviceUrl = nodeUrl;
            const QPersistentModelIndex persistentCapturedIndex(capturedIndex);

            int subId = DeviceMountSubscriber::instance()->subscribe(
                    nodeUrl,
                    [persistentCapturedIndex, view, guard, deviceUrl](const QUrl &mountedUrl) {
                        if (!view || !guard) {
                            fmDebug() << "SideBarViewPrivate: View destroyed before mount completed";
                            return;
                        }

                        guard->pendingMountSubs.remove(deviceUrl);

                        fmDebug() << "SideBarViewPrivate: Device mounted at" << mountedUrl
                                  << ", auto-expanding directory";

                        QTimer::singleShot(100, view, [persistentCapturedIndex, mountedUrl, view, guard]() {
                            if (!view || !guard) {
                                fmDebug() << "SideBarViewPrivate: View destroyed during delayed expansion";
                                return;
                            }

                            if (!persistentCapturedIndex.isValid()
                                || persistentCapturedIndex.model() != view->model()) {
                                fmDebug() << "SideBarViewPrivate: Stale index after mount, aborting auto-expand";
                                return;
                            }

                            if (mountedUrl.isValid() && mountedUrl.scheme() == "file"
                                && QDir(mountedUrl.path()).exists()) {
                                guard->expandPartitionItem(persistentCapturedIndex, mountedUrl);
                            } else {
                                fmDebug() << "SideBarViewPrivate: Unable to expand - invalid mounted URL:"
                                          << mountedUrl;
                            }
                        });
                    });
            if (subId >= 0)
                pendingMountSubs.insert(nodeUrl, subId);
        }
        return;
    }

    fmDebug() << "Double clicked on non-separator item, ignoring";
}

void SideBarViewPrivate::setTransparentPalette()
{
    QPalette treePal = q->palette();
    treePal.setColor(QPalette::Base, Qt::transparent);
    q->setPalette(treePal);
}

void SideBarViewPrivate::restorePalette()
{
    q->setPalette(originPalette);
}

void SideBarViewPrivate::expandItem(const QModelIndex &index, const QList<QUrl> &subFolders)
{
    if (!index.isValid())
        return;

    q->model()->addSubItems(index, subFolders);
    q->onChangeExpandState(index, true);
    q->setCurrentUrl(sidebarUrl);
}

void SideBarViewPrivate::expandPartitionItem(const QModelIndex &index, const QUrl &url)
{
    // If already expanded, collapse it.
    if (q->isExpanded(index)) {
        q->onChangeExpandState(index, false);
        return;
    }

    auto filters = QDir::Dirs | QDir::NoDotAndDotDot;
    if (Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool())
        filters |= QDir::Hidden;

    // Show a wait cursor while traversing large directories (e.g. /proc with
    // hundreds of pid entries) so the user gets immediate feedback that the
    // expand is in progress. Restored exactly once when the traversal emits
    // updateChildren or finishes (whichever fires first).
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    TraversalDirThread *t = new TraversalDirThread(url, {}, filters, QDirIterator::FollowSymlinks);
    // Pre-sort the traversal results on the worker thread using FileNameSorter
    // (numeric-aware ICU collation) so the sidebar sub-tree ordering matches the
    // file view / title bar / desktop and the heavy sorting is offloaded from
    // the UI thread.
    t->setEnableSort(true);
    connect(t, &TraversalDirThread::finished, t, &TraversalDirThread::deleteLater);
    const QPersistentModelIndex persistentIndex(index);
    // Shared flag ensures restoreOverrideCursor is called exactly once per
    // setOverrideCursor, regardless of which signal fires first. The cursor is
    // a global QApplication resource, so restoring it does not depend on the
    // view being alive.
    auto cursorRestored = std::make_shared<bool>(false);
    auto restoreCursorOnce = [cursorRestored]() {
        if (*cursorRestored)
            return;
        *cursorRestored = true;
        QApplication::restoreOverrideCursor();
    };
    connect(t, &TraversalDirThread::updateChildren, this, [=](const QList<QUrl> &subs) {
        restoreCursorOnce();
        if (!persistentIndex.isValid() || persistentIndex.model() != q->model()) {
            fmDebug() << "SideBarViewPrivate: Stale index after traversal, aborting expand";
            return;
        }
        this->expandItem(persistentIndex, subs);
    });
    // Ensure the cursor is restored even if the thread finishes without emitting
    // updateChildren (e.g. empty directory or early cancellation).
    connect(t, &TraversalDirThread::finished, this, [restoreCursorOnce]() {
        restoreCursorOnce();
    });
    t->start();
}

void SideBarViewPrivate::cancelPendingMountSubscription(const QUrl &deviceUrl)
{
    auto it = pendingMountSubs.find(deviceUrl);
    if (it != pendingMountSubs.end()) {
        DeviceMountSubscriber::instance()->unsubscribe(it.value());
        pendingMountSubs.erase(it);
    }
}

void SideBarViewPrivate::onExpandableChanged()
{
    if (SideBarHelper::partitionExpandable()) {
        q->viewport()->update();
        return;
    }

    fmDebug() << "Partition expansion is disabled";
    SideBarModel *sidebarModel = q->model();
    if (!sidebarModel) {
        fmDebug() << "SideBarViewPrivate: Model is null, cannot collapse partitions";
        return;
    }

    auto groupIndex = sidebarModel->findGroupIndex(DefaultGroup::kDevice);
    if (!groupIndex.isValid()) {
        fmDebug() << "SideBarViewPrivate: Group index not found";
        return;
    }

    // Use rowCount(groupIndex) — not rowCount() — so we iterate device children, not top-level groups.
    for (int i = 0; i < sidebarModel->rowCount(groupIndex); ++i) {
        QModelIndex index = sidebarModel->index(i, 0, groupIndex);
        if (!index.isValid())
            continue;

        DStandardItem *item = sidebarModel->itemFromIndex(index);
        SideBarItem *sidebarItem = dynamic_cast<SideBarItem *>(item);

        if (sidebarItem && sidebarItem->group() == DefaultGroup::kDevice) {
            // Collapse the partition and remove its sub-items so file watchers are released.
            if (q->isExpanded(index)) {
                fmDebug() << "SideBarViewPrivate: Collapsing expanded partition:" << sidebarItem->url();
                q->onChangeExpandState(index, false);
            }
            // Remove dynamically-added sub-items (keep the partition item itself).
            while (sidebarItem->rowCount() > 0) {
                if (auto *child = static_cast<SideBarItem *>(sidebarItem->child(0)))
                    SideBarInfoCacheMananger::instance()->removeItemInfoCache(child->url());
                sidebarItem->removeRow(0);
            }
        }
    }

    q->viewport()->update();
}

void SideBarViewPrivate::updateHoverIndex(const QModelIndex &index)
{
    if (currentHoverIndex == index)
        return;

    const QModelIndex previousHoverIndex = currentHoverIndex;
    currentHoverIndex = index;

    auto safeUpdate = [this](const QModelIndex &idx) {
        if (idx.isValid() && idx.model() == q->model())
            q->update(idx);
    };
    safeUpdate(previousHoverIndex);
    safeUpdate(currentHoverIndex);
}

void SideBarViewPrivate::clearHoverIndex()
{
    updateHoverIndex(QModelIndex());
}

bool SideBarViewPrivate::isCursorInsideIndex(const QModelIndex &index, const QPoint &fallbackPos) const
{
    if (!index.isValid())
        return false;

    const QRect itemRect = q->visualRect(index);
    const QPoint globalPos = QCursor::pos();
    if (QWidget *windowWidget = q->window()) {
        const QPoint windowPos = windowWidget->mapFromGlobal(globalPos);
        if (QWidget *targetWidget = windowWidget->childAt(windowPos)) {
            if (!(targetWidget == q || q->isAncestorOf(targetWidget)))
                return false;

            const QPoint cursorPos = q->viewport()->mapFromGlobal(globalPos);
            return itemRect.contains(cursorPos);
        }
    }

    // Fallback for compositor paths where childAt/global lookup is unavailable
    // but the sidebar viewport still owns the active pointer hover state.
    return q->viewport()->underMouse() && itemRect.contains(fallbackPos);
}

void SideBarViewPrivate::setDragSourceIndex(const QModelIndex &index)
{
    const QModelIndex validIndex = index.isValid() && index.flags().testFlag(Qt::ItemIsDragEnabled)
            ? index
            : QModelIndex();
    if (dragSourceIndex == validIndex)
        return;

    dragSourceIndex = validIndex;
}

void SideBarViewPrivate::clearInternalDragState()
{
    if (placeholderAnimation)
        placeholderAnimation->stop();

    isRenderingDragPreview = false;
    dragSourceIndex = QModelIndex();
    previousPlaceholderParent = QModelIndex();
    placeholderParent = QModelIndex();
    previousPlaceholderRow = -1;
    placeholderRow = -1;
    placeholderAnimationProgress = 1.0;
    q->viewport()->update();
}

int SideBarViewPrivate::calculatePlaceholderRow(const QPoint &pos, const QMimeData *data) const
{
    if (!dragSourceIndex.isValid() || !data)
        return -1;

    const QModelIndex hoverIndex = q->indexAt(pos);
    if (!hoverIndex.isValid() || hoverIndex.parent() != dragSourceIndex.parent())
        return -1;

    int row = dragSourceIndex.row();
    if (hoverIndex.row() > dragSourceIndex.row())
        row = hoverIndex.row() + 1;
    else if (hoverIndex.row() < dragSourceIndex.row())
        row = hoverIndex.row();

    auto *sidebarModel = q->model();
    if (!sidebarModel)
        return -1;

    if (!sidebarModel->canDropMimeData(data, Qt::MoveAction, row, 0, hoverIndex.parent()))
        return -1;

    return row;
}

void SideBarViewPrivate::updatePlaceholderRow(int row, const QModelIndex &parent)
{
    if (!dragSourceIndex.isValid() || !parent.isValid() || row < 0) {
        previousPlaceholderParent = placeholderParent;
        previousPlaceholderRow = placeholderRow;
        placeholderParent = QModelIndex();
        placeholderRow = -1;
        placeholderAnimationProgress = 1.0;
        q->viewport()->update();
        return;
    }

    if (placeholderParent == parent && placeholderRow == row)
        return;

    previousPlaceholderParent = placeholderParent;
    previousPlaceholderRow = placeholderRow;
    placeholderParent = parent;
    placeholderRow = row;

    if (!placeholderAnimation) {
        placeholderAnimation = new QVariantAnimation(this);
        placeholderAnimation->setDuration(120);
        placeholderAnimation->setEasingCurve(QEasingCurve::OutCubic);
        connect(placeholderAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            placeholderAnimationProgress = value.toReal();
            q->viewport()->update();
        });
        connect(placeholderAnimation, &QVariantAnimation::finished, this, [this]() {
            placeholderAnimationProgress = 1.0;
            q->viewport()->update();
        });
    }

    if (previousPlaceholderRow < 0 || previousPlaceholderParent != placeholderParent) {
        placeholderAnimation->stop();
        placeholderAnimationProgress = 1.0;
        q->viewport()->update();
        return;
    }

    if (placeholderAnimation->state() == QAbstractAnimation::Running)
        placeholderAnimation->stop();

    placeholderAnimationProgress = 0.0;
    placeholderAnimation->setStartValue(0.0);
    placeholderAnimation->setEndValue(1.0);
    placeholderAnimation->start();
}

int SideBarViewPrivate::dragItemOffset(const QModelIndex &index, int rowHeight) const
{
    if (!isItemDragged || !dragSourceIndex.isValid() || !index.isValid() || rowHeight <= 0)
        return 0;

    auto calcOffset = [this, rowHeight, &index](const QPersistentModelIndex &parent, int row) -> int {
        if (!parent.isValid() || row < 0 || index.parent() != parent || dragSourceIndex.parent() != parent)
            return 0;

        const int sourceRow = dragSourceIndex.row();
        const int currentRow = index.row();
        if (currentRow == sourceRow) {
            if (sourceRow < row)
                return (row - sourceRow - 1) * rowHeight;
            if (sourceRow > row)
                return (row - sourceRow) * rowHeight;
            return 0;
        }

        if (sourceRow < row && currentRow > sourceRow && currentRow < row)
            return -rowHeight;

        if (sourceRow > row && currentRow >= row && currentRow < sourceRow)
            return rowHeight;

        return 0;
    };

    const int startOffset = calcOffset(previousPlaceholderParent, previousPlaceholderRow);
    const int endOffset = calcOffset(placeholderParent, placeholderRow);
    if (qFuzzyCompare(placeholderAnimationProgress, 1.0))
        return endOffset;

    return qRound(startOffset + (endOffset - startOffset) * placeholderAnimationProgress);
}

void SideBarViewPrivate::notifyOrderChanged()
{
    if (draggedGroup.isEmpty()) {
        fmDebug() << "No dragged group to notify order change";
        return;
    }

    QTimer::singleShot(0, this, [this] {   // this must be invoked after items are sorted finished
        QList<QUrl> ret;
        QList<SideBarItem *> items { q->model()->subItems(draggedGroup) };
        std::for_each(items.begin(), items.end(), [&ret](SideBarItem *item) {
            if (!item)
                return;
            ret.append(item->url());
        });

        quint64 winId = FMWindowsIns.findWindowId(q);
        dpfSignalDispatcher->publish("dfmplugin_sidebar", "signal_Sidebar_Sorted", winId, draggedGroup, ret);
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

    if (!event) {
        fmWarning() << "Drag enter event is null";
        return false;
    }

    if (urlsForDragEvent.isEmpty() || FileUtils::isContainProhibitPath(urlsForDragEvent)) {
        fmDebug() << "Drag enter rejected: empty URLs or prohibited path";
        return false;
    }

    SideBarItem *item = q->itemAt(event->position().toPoint());
    if (item) {
        const QUrl &targetItemUrl { item->targetUrl() };
        if (!checkTargetEnable(targetItemUrl)) {
            fmDebug() << "Drag enter rejected: target not enabled, URL:" << targetItemUrl;
            return false;
        }
    }

    return true;
}

bool SideBarViewPrivate::canMove(QDragMoveEvent *event)
{
    Q_ASSERT(q);

    if (!event) {
        fmWarning() << "Drag move event is null";
        return false;
    }

    const QList<QUrl> &urls = urlsForDragEvent.isEmpty()
            ? event->mimeData()->urls()
            : urlsForDragEvent;

    if (!urls.isEmpty()) {
        SideBarItem *item = q->itemAt(event->position().toPoint());
        if (!item) {
            fmDebug() << "Drag move rejected: no item at position";
            return false;
        }

        const QUrl &targetItemUrl { item->targetUrl() };
        if (!checkTargetEnable(targetItemUrl)) {
            fmDebug() << "Drag move rejected: target not enabled, URL:" << targetItemUrl;
            return false;
        }

        Qt::DropAction action { Qt::CopyAction };
        if (dpfHookSequence->run("dfmplugin_sidebar", "hook_Item_DragMoveData", urls, item->url(), &action)) {
            if (action == Qt::IgnoreAction) {
                fmDebug() << "Drag move rejected: hook returned IgnoreAction";
                return false;
            }
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
        // 不绘制拖拽指示线，改为拖动时选项位移动画
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
    setAnimated(true);
    setIconSize(QSize(16, 16));
    setHeaderHidden(true);
    setMouseTracking(true);   // sp3 feature 35，解除注释以便鼠标在移动时就能触发 mousemoveevent
    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(false);
    setFrameShape(QFrame::Shape::NoFrame);
    setContextMenuPolicy(Qt::CustomContextMenu);

    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAutoFillBackground(false);

    // Enable touch-based kinetic scrolling for touchscreen devices.
    // QScroller intercepts touch events before they are synthesized into mouse events,
    // preventing the InternalMove drag state machine from consuming swipe gestures.
    QScroller::grabGesture(viewport(), QScroller::TouchGesture);

    connect(this, &DTreeView::clicked, d, &SideBarViewPrivate::currentChanged);
    connect(this, &DTreeView::doubleClicked, d, &SideBarViewPrivate::onItemDoubleClicked);
    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, [=](const QString &cfg, const QString &key) {
        if (cfg == ConfigInfos::kConfName && key == ConfigInfos::kPartitionExpandableKey) {
            m_partitionExpandableCache.reset();
            d->onExpandableChanged();
            updateEditTriggers();
        }
    });

    d->originPalette = palette();
    d->lastOpTimer.start();

    setDropIndicatorShown(false);
    auto *sidebarStyle = new SidebarViewStyle(style());
    setStyle(sidebarStyle);
    viewport()->setStyle(sidebarStyle);

    // Apply the initial edit-trigger policy according to the tree-view mode.
    updateEditTriggers();
}

SideBarView::~SideBarView()
{
    // Cancel any pending device-mount subscriptions to avoid lingering callbacks.
    for (auto it = d->pendingMountSubs.begin(); it != d->pendingMountSubs.end(); ++it)
        DeviceMountSubscriber::instance()->unsubscribe(it.value());
    d->pendingMountSubs.clear();

    QScroller::ungrabGesture(viewport());
    setStyle(nullptr);
}

SideBarModel *SideBarView::model() const
{
    return qobject_cast<SideBarModel *>(QAbstractItemView::model());
}

void SideBarView::mousePressEvent(QMouseEvent *event)
{
    // 频繁点击操作与网络或挂载设备的加载效率低两个因素的共同作用下 会导致侧边栏可能出现显示错误
    // 暂时抛去部分频繁点击来规避这个问题
    if (!d->checkOpTime())
        return;

    d->dragPressPos = event->pos();
    d->setDragSourceIndex(indexAt(event->pos()));
    d->draggedUrl = urlAt(event->pos());
    auto item = itemAt(event->pos());
    d->draggedGroup = item ? item->group() : "";

    auto index = indexAt(event->pos());
    if (event->button() == Qt::LeftButton && index.isValid()
        && item && item->group() == DefaultGroup::kDevice) {
        if (item->isExpandable() && SideBarHelper::partitionExpandable()) {
            int layer = 0;
            auto parentIdx = index;
            while (parentIdx.parent().isValid()) {
                parentIdx = parentIdx.parent();
                layer++;
            }
            // see @SideBarItemDelegate::drawExpandIndicator
            // The indicator is painted at (layer * kExpandIndentPerLayer + kExpandIconOffset).
            // Expand the hit area by kExpandHitMargin on both sides for easier clicking.
            int iconLeft = layer * ExpandIndicatorGeometry::kIndentPerLayer + ExpandIndicatorGeometry::kIconOffset;
            int collapseIconLeft = iconLeft - ExpandIndicatorGeometry::kHitMargin;
            int collapseIconRight = iconLeft + ExpandIndicatorGeometry::kIconSize + ExpandIndicatorGeometry::kHitMargin;
            if (event->pos().x() >= collapseIconLeft && event->pos().x() <= collapseIconRight) {
                d->onItemDoubleClicked(index);
                d->ignoreNextMouseRelease = true;
                return;   // do not select the item.
            }
        }
    }

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
            data.insert("sidebar_item", reportName.isEmpty() ? info.displayName : reportName);

            dpfSignalDispatcher->publish("dfmplugin_sidebar", "signal_ReportLog_Commit", QString("Sidebar"), data);
        }
    }

    // Prevent selecting an item when the mouse press was consumed by the
    // expand-indicator click handler (see mousePressEvent).
    if (d->ignoreNextMouseRelease) {
        d->ignoreNextMouseRelease = false;
        return;
    }

    DTreeView::mouseReleaseEvent(event);
}

void SideBarView::dragEnterEvent(QDragEnterEvent *event)
{
    d->clearHoverIndex();
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
        if (!d->dragSourceIndex.isValid())
            d->setDragSourceIndex(currentIndex());
        d->isItemDragged = d->dragSourceIndex.isValid();
        if (d->dragSourceIndex.isValid())
            d->updatePlaceholderRow(d->dragSourceIndex.row(), d->dragSourceIndex.parent());
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
    const QPoint eventPos = event->position().toPoint();
    const QModelIndex hoverIndex = indexAt(eventPos);

    if (event->source() != this)
        d->updateHoverIndex(hoverIndex);
    else if (d->dragSourceIndex.isValid()) {
        const int placeholderRow = d->calculatePlaceholderRow(eventPos, event->mimeData());
        if (placeholderRow >= 0) {
            d->updatePlaceholderRow(placeholderRow, hoverIndex.parent());
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            d->updatePlaceholderRow(d->dragSourceIndex.row(), d->dragSourceIndex.parent());
        }
    }

    if (event->source() != this && !d->isCursorInsideIndex(hoverIndex, eventPos)) {
        d->clearHoverIndex();
        event->ignore();
        return;
    }

    SideBarItem *item = itemAt(eventPos);
    if (item) {
        if (!d->canMove(event)) {
            event->setDropAction(Qt::IgnoreAction);
            event->ignore();
            return;
        }
    }

    if (isAccepteDragEvent(event))
        return;

    // DTreeView::dragMoveEvent(event);  // 禁用拖拽指示线

    if (event->source() != this)
        event->ignore();
}

void SideBarView::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    d->draggedUrl = QUrl("");
    d->isItemDragged = false;
    setState(State::NoState);
    d->clearHoverIndex();
    d->clearInternalDragState();
}

void SideBarView::dropEvent(QDropEvent *event)
{
    const bool isInternalDrag = event->source() == this;
    const QPoint eventPos = event->position().toPoint();
    const QModelIndex hoverIndex = indexAt(eventPos);
    d->clearHoverIndex();
    d->isItemDragged = false;
    if (d->draggedUrl.isValid()) {   // select the dragged item when dropped.
        d->notifyOrderChanged();   // notify to update the persistence data
    }
    d->dropPos = eventPos;

    if (isInternalDrag) {
        // Use calculatePlaceholderRow to ensure the actual drop row matches
        // the visual placeholder shown during dragMoveEvent. The base class
        // dropEvent relies on dropIndicatorPosition which is never updated
        // because dragMoveEvent does not call the base class.
        const int targetRow = d->calculatePlaceholderRow(eventPos, event->mimeData());
        if (targetRow >= 0 && d->dragSourceIndex.isValid()) {
            const QModelIndex parent = d->dragSourceIndex.parent();
            if (auto *sidebarModel = model()) {
                if (sidebarModel->dropMimeData(event->mimeData(), Qt::MoveAction,
                                               targetRow, 0, parent)) {
                    event->setDropAction(Qt::MoveAction);
                    event->accept();
                }
            }
        }
        d->clearInternalDragState();
        return;
    }

    SideBarItem *item = itemAt(eventPos);
    if (!item)
        return DTreeView::dropEvent(event);

    const QUrl &targetItemUrl { item->targetUrl() };

    fmDebug() << "source: " << event->mimeData()->urls();
    fmDebug() << "target item: " << item->group() << "|" << item->text() << "|" << item->url();
    fmDebug() << "item->itemInfo().finalUrl: " << item->itemInfo().finalUrl;
    fmDebug() << "item flags:" << item->flags();
    if (!d->isCursorInsideIndex(hoverIndex, eventPos)) {
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
            curWindow->activateWindow();

        event->accept();
    } else {
        DTreeView::dropEvent(event);
    }
}

void SideBarView::startDrag(Qt::DropActions supportedActions)
{
    if (!d->draggedUrl.isValid())
        return;

    // Prevent spurious drag during touch scrolling: QScroller and InternalMove
    // both react to the same touch sequence, causing duplicate items on drop.
    // Only allow dragging if the scroller is completely inactive.
    if (QScroller::hasScroller(viewport())) {
        QScroller *scroller = QScroller::scroller(viewport());
        if (scroller && scroller->state() != QScroller::Inactive) {
            d->isItemDragged = false;
            return;
        }
    }

    QModelIndex sourceIndex = d->dragSourceIndex;
    if (!sourceIndex.isValid())
        sourceIndex = currentIndex();
    if (!sourceIndex.isValid() || !sourceIndex.flags().testFlag(Qt::ItemIsDragEnabled))
        return;
    const QPersistentModelIndex sourcePersistentIndex(sourceIndex);

    QMimeData *mimeData = model() ? model()->mimeData({ sourceIndex }) : nullptr;
    if (!mimeData)
        return;

    QRect sourceRect = visualRect(sourceIndex);
    auto *drag = new QDrag(this);
    drag->setMimeData(mimeData);

    auto *delegate = itemDelegateForIndex(sourceIndex);
    if (!delegate)
        delegate = itemDelegate();
    if (delegate && sourceRect.isValid()) {
        QStyleOptionViewItem option;
        initViewItemOption(&option);
        option.widget = this;
        option.rect = QRect(QPoint(0, 0), sourceRect.size());
        option.state |= QStyle::State_Enabled;
        option.state &= ~(QStyle::State_MouseOver | QStyle::State_Sunken | QStyle::State_HasFocus);

        const qreal dpr = devicePixelRatioF();
        QPixmap pixmap(option.rect.size() * dpr);
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);

        d->isRenderingDragPreview = true;
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setOpacity(0.9);
        delegate->paint(&painter, option, sourceIndex);
        d->isRenderingDragPreview = false;

        drag->setPixmap(pixmap);

        QPoint hotSpot = d->dragPressPos - sourceRect.topLeft();
        hotSpot.setX(qBound(0, hotSpot.x(), qMax(0, option.rect.width() - 1)));
        hotSpot.setY(qBound(0, hotSpot.y(), qMax(0, option.rect.height() - 1)));
        drag->setHotSpot(hotSpot);
    }

    d->isItemDragged = true;
    d->updatePlaceholderRow(sourceIndex.row(), sourceIndex.parent());

    Qt::DropAction defaultAction = defaultDropAction();
    if (defaultAction == Qt::IgnoreAction || !(supportedActions & defaultAction)) {
        if (dragDropMode() == QAbstractItemView::InternalMove)
            defaultAction = Qt::MoveAction;
        else if (supportedActions & Qt::CopyAction)
            defaultAction = Qt::CopyAction;
    }

    const Qt::DropAction resultAction = drag->exec(supportedActions, defaultAction);
    const QObject *dropTarget = drag->target();

    // QStandardItemModel inserts the moved row on drop, and the default
    // view implementation removes the original row afterwards. Since we
    // render a custom drag preview here, we need to perform that cleanup.
    if (resultAction == Qt::MoveAction
        && sourcePersistentIndex.isValid()
        && (dropTarget == this || dropTarget == viewport())) {
        if (auto *sidebarModel = model())
            sidebarModel->removeRows(sourcePersistentIndex.row(), 1, sourcePersistentIndex.parent());
    }

    d->isItemDragged = false;
    d->clearInternalDragState();
}

QModelIndex SideBarView::indexAt(const QPoint &p) const
{
    return DTreeView::indexAt(p);
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
    if (SideBarHelper::groupExpandRules().isEmpty()) {
        fmDebug() << "No group expand rules to save";
        return;
    }
    if (d->groupExpandState.isEmpty())
        d->groupExpandState = SideBarHelper::groupExpandRules();

    SideBarHelper::saveGroupsStateToConfig(d->groupExpandState);
}

void SideBarView::setCurrentUrl(const QUrl &url)
{
    // The sidebar highlight is URL-based (see SideBarItemDelegate::paint): an item is
    // highlighted when its URL matches currentUrl(), and the highlight only refreshes
    // when the item is repainted. setCurrentIndex() is meant to repaint the newly
    // current item via currentChanged(), but that repaint is unreliable here:
    //   - QItemSelectionModel::setCurrentIndex() is a no-op (no currentChanged) when
    //     Qt's current index already equals the target, e.g. returning to a sidebar
    //     item visited before, or coming back after navigating into a sub-folder that
    //     has no sidebar item (which never changes the current index);
    //   - the collapsed-group early-return and the not-found branch below skip
    //     setCurrentIndex() entirely.
    // The sidebar-click path works only because SideBarWidget::onItemActived explicitly
    // update()s the previous/current items afterwards; the file-view navigation path
    // (cd() -> setCurrentUrl) has no such repaint, so the highlight does not refresh
    // until a hover triggers a viewport repaint.
    //
    // Fix: whenever the URL basis actually changes, repaint the whole viewport so every
    // visible item re-evaluates its URL-based highlight against the new url -- the
    // previously highlighted item drops its stale highlight and the newly matching
    // item gains it, immediately, without depending on setCurrentIndex(). This is safe
    // because the highlight background is purely URL-matched, so at most one item is
    // highlighted (no "two items selected").
    const QUrl previousUrl = d->sidebarUrl;
    d->sidebarUrl = url;
    if (!UniversalUtils::urlEquals(previousUrl, url))
        viewport()->update();   // refresh URL-based highlight for all visible items immediately

    bool urlNotChanged = UniversalUtils::urlEquals(d->current.data(SideBarItem::kItemUrlRole).toUrl(), url);
    const QModelIndex &index = urlNotChanged ? d->current : findItemIndex(url);

    if (!index.isValid()) {
        const QModelIndex &checkIndex = findItemIndex(url);
        if (checkIndex.isValid()) {
            d->current = checkIndex;
        } else {
            fmWarning() << "URL not found in sidebar, clearing selection:" << url;
            this->clearSelection();
            return;
        }
    }
    SideBarModel *sidebarModel = dynamic_cast<SideBarModel *>(model());
    if (!sidebarModel) {
        fmWarning() << "Sidebar model is null, cannot set current URL";
        return;
    }
    SideBarItem *currentItem = sidebarModel->itemFromIndex(index);
    if (currentItem && currentItem->parent()) {
        SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(currentItem->parent());
        // If the current item's group is not expanded, do not set current index, otherwise
        // the unexpanded group would be expaned again.
        if (groupItem && !groupItem->isExpanded()) {
            fmDebug() << "Group not expanded, skipping current index set for URL:" << url;
            d->current = index;
            return;
        }
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

QModelIndex SideBarView::currentTrackedIndex() const
{
    return d->current;
}

QModelIndex SideBarView::findItemIndex(const QUrl &url) const
{
    SideBarModel *sidebarModel = dynamic_cast<SideBarModel *>(model());
    if (!sidebarModel) {
        fmWarning() << "Sidebar model is null, cannot find item index";
        return QModelIndex();
    }

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

    fmDebug() << "Item index not found for URL:" << url;
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

bool SideBarView::isDropTarget(const QModelIndex &index) const
{
    return index == d->currentHoverIndex;
}

bool SideBarView::isSideBarItemDragged() const
{
    return d->isItemDragged;
}

bool SideBarView::isDraggedSource(const QModelIndex &index) const
{
    return d->dragSourceIndex == index;
}

bool SideBarView::isRenderingDragPreview() const
{
    return d->isRenderingDragPreview;
}

int SideBarView::dragItemVerticalOffset(const QModelIndex &index, int rowHeight) const
{
    return d->dragItemOffset(index, rowHeight);
}

bool SideBarView::isPartitionExpandable() const
{
    if (m_partitionExpandableCache.has_value())
        return *m_partitionExpandableCache;
    m_partitionExpandableCache = SideBarHelper::partitionExpandable();
    return *m_partitionExpandableCache;
}

void SideBarView::updateEditTriggers()
{
    // The sidebar tree view (partitionExpandable) reserves double-click for
    // expanding/collapsing a partition, so inline renaming on double-click must be
    // disabled in that mode and restored when the tree view is turned off.
    // F2 (EditKeyPressed) and the context-menu rename entry are kept in both modes.
    if (isPartitionExpandable())
        setEditTriggers(QAbstractItemView::EditKeyPressed);
    else
        setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
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

    // do not check the permissions when the dir is not a real dir
    if (itemInfo->fileType() == FileInfo::FileType::kDirectory
        && UniversalUtils::urlEquals(targetItemUrl, itemInfo->urlOf(UrlInfoType::kOriginalUrl))) {
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
        // 部分文件不能复制或剪切，需要在拖拽时忽略
        if (!fileInfo->canAttributes(CanableInfoType::kCanMoveOrCopy) && !fileInfo->canAttributes(CanableInfoType::kCanRename)) {
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

bool SideBarView::isAccepteDragEvent(QDropEvent *event) const
{
    SideBarItem *item = itemAt(event->position().toPoint());
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
    if (!sidebarModel) {
        fmWarning() << "Sidebar model is null, cannot update separator visible state";
        return;
    }

    for (int i = 0; i < sidebarModel->rowCount(); i++) {
        SideBarItem *item = sidebarModel->itemFromIndex(i);   // top item
        if (item)
            allItemsInvisiable = false;
        if (item && item->group() != lastGroupName) {
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
    if (!sidebarModel) {
        fmWarning() << "Sidebar model is null, cannot change expand state";
        return;
    }
    SideBarItem *item = sidebarModel->itemFromIndex(index);
    if (!item) {
        fmWarning() << "Item is null, cannot change expand state";
        return;
    }

    SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(item);

    d->setTransparentPalette();
    setExpanded(index, expand);
    d->restorePalette();

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

    // Notify SideBarModel to start/stop file watching for partition items.
    if (expand) {
        sidebarModel->onItemExpanded(index);
    } else {
        sidebarModel->onItemCollapsed(index);
    }

    if (index.isValid() && index.model() == sidebarModel)
        update(index);
}

void SideBarView::onRequestCollapseItem(const QModelIndex &index)
{
    if (index.isValid() && index.model() == model()) {
        collapse(index);
        onChangeExpandState(index, false);
        fmDebug() << "Collapsed item per model request:" << index.data(Qt::DisplayRole).toString();
    }
}

bool SideBarViewPrivate::checkOpTime()
{
    // If the interval between twice checking, then return true.
    if (lastOpTimer.elapsed() > 200) {
        lastOpTimer.restart();
        return true;
    }

    return false;
}
