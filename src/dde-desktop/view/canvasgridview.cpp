/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include "canvasgridview.h"

#include <QDrag>
#include <QWindow>
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>
#include <QPainterPath>
#include <QScrollBar>
#include <QTextEdit>
#include <QUrlQuery>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMimeData>
#include <QProcess>
#include <QApplication>
#include <QScreen>
#include <QAction>
#include <QDir>
#include <QStandardPaths>
#include <QPropertyAnimation>
#include <danchors.h>
#include <DUtil>

#include <durl.h>
#include <dfmglobal.h>
#include <dfilesystemmodel.h>
#include <diconitemdelegate.h>
#include <dlistitemdelegate.h>
#include <dfileviewhelper.h>
#include <dfileservices.h>
#include <dfilemenu.h>
#include <dfilemenumanager.h>
#include <dfilewatcher.h>
#include <dfmapplication.h>
#include <dfmsettings.h>
#include <dgiosettings.h>
#include <dfiledragclient.h>
//dfmlib singleton
#include <../utils/singleton.h>
#include <DApplication>

#include "../util/dde/desktopinfo.h"
#include "../model/dfileselectionmodel.h"
#include "../presenter/gridmanager.h"
#include "../presenter/apppresenter.h"
#include "../presenter/dfmsocketinterface.h"
#include "../desktop.h"
#include "../config/config.h"
#include "screen/screenhelper.h"
#include "../models/desktopfileinfo.h"

#include "interfaces/private/mergeddesktop_common_p.h"
#include "private/canvasviewprivate.h"
#include "canvasviewhelper.h"
#include "watermaskframe.h"
#include "desktopitemdelegate.h"

#include "app/define.h"
#include "controllers/mergeddesktopcontroller.h"
#include "../dde-wallpaper-chooser/screensavercontrol.h"

#include "accessibility/ac-desktop-define.h"

#define DESKTOP_CAN_SCREENSAVER "DESKTOP_CAN_SCREENSAVER"
#define DRAGICON_SIZE 128       //拖拽聚合默认icon边长
#define DRAGICON_OUTLINE 30     //增加外圈范围，防止旋转后部分图片的角绘制不到
#define DRAGICON_MAX 4          //拖拽聚合最多绘制icon数量
#define DRAGICON_ROTATE 10.0    //拖拽聚合旋转角度
#define DRAGICON_OPACITY 0.1    //拖拽聚合透明度梯度
#define DRAGICON_MAX_COUNT 99   //最大显示计数

QMap<DMD_TYPES, bool> CanvasGridView::virtualEntryExpandState;

static const QMap<int, int> kSortActions = {{MenuAction::Name, DFileSystemModel::FileDisplayNameRole}
    , {MenuAction::Size, DFileSystemModel::FileSizeRole}
    , {MenuAction::Type, DFileSystemModel::FileMimeTypeRole}
    , {MenuAction::LastModifiedDate, DFileSystemModel::FileLastModifiedRole}
};

DWIDGET_USE_NAMESPACE

//candrop十分耗时,在不关心Qt::ItemDropEnable的调用时ignoreDropFlag为true，不调用candrop，节省时间,bug#10926
namespace  {
    class IgnoreDropFlag
    {
    public:
        explicit IgnoreDropFlag(DFileSystemModel *m) : model(m)
        {
            if (model)
                model->ignoreDropFlag = true;
        }
        ~IgnoreDropFlag()
        {
            if (model)
                model->ignoreDropFlag = false;
        }
    private:
        DFileSystemModel *model = nullptr;
    };

    static void setMenuActionsFilter()
    {
        DFileMenuManager::setActionWhitelist(QSet<MenuAction>());
        DFileMenuManager::setActionBlacklist(QSet<MenuAction>());
    }
    // end
}

CanvasGridView::CanvasGridView(const QString &screen, QWidget *parent)
    : QAbstractItemView(parent)
    , d(new CanvasViewPrivate)
    , m_screenName(screen)
    , m_currentTargetUrl(DUrl())
{
    AC_SET_OBJECT_NAME(this, AC_CANVAS_GRID_VIEW);
    AC_SET_ACCESSIBLE_NAME(this, AC_CANVAS_GRID_VIEW);
    initUI();
    initConnection();
}

CanvasGridView::~CanvasGridView()
{

}

QRect CanvasGridView::visualRect(const QModelIndex &index) const
{
    auto url = model()->getUrlByIndex(index);
    auto gridPos = GridManager::instance()->position(m_screenNum, url.toString());

    auto x = gridPos.x() * d->cellWidth + d->viewMargins.left();
    auto y = gridPos.y() * d->cellHeight + d->viewMargins.top();
    return QRect(x, y, d->cellWidth, d->cellHeight);
}

QModelIndex CanvasGridView::indexAt(const QPoint &point) const
{
    QPoint gridPos = gridAt(point);
    //如果本屏有堆叠就，并且point为堆叠点，那么修改点击的项目为堆叠项目的最后一个 for优先在最后一个网格显示堆叠的最后一个项目
    QString localFile = GridManager::instance()->itemTop(m_screenNum, gridPos.x(), gridPos.y());
    //GridManager::instance()->itemId(m_screenNum, gridPos.x(), gridPos.y());

    QModelIndex rowIndex = model()->index(DUrl(localFile));
    QPoint pos = QPoint(point.x() + horizontalOffset(), point.y() + verticalOffset());
    auto list = itemPaintGeomertys(rowIndex);


    for (QModelIndex &index : itemDelegate()->hasWidgetIndexs()) {
        if (index == itemDelegate()->editingIndex()) {
            QWidget *widget = itemDelegate()->editingIndexWidget();
            if (widget && widget->isVisible() && widget->geometry().contains(point)) {
                return index;
            }
            continue;
        }

        QWidget *widget = indexWidget(index);

        if (widget && widget->isVisible() && widget->geometry().contains(point)) {
            return index;
        }
    }

    auto ret = std::any_of(list.begin(), list.end(), [pos](const QRect & rect) {
        return rect.contains(pos);
    });

    if (ret)
        return rowIndex;

    return QModelIndex();
}

void CanvasGridView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    Q_UNUSED(hint);
    QRect area = viewport()->rect();
    QRect rect = visualRect(index);

    if (rect.left() < area.left()) {
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() + rect.left() - area.left());
    } else if (rect.right() > area.right()) {
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() + qMin(
                rect.right() - area.right(), rect.left() - area.left()));
    }

    if (rect.top() < area.top()) {
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() + rect.top() - area.top());
    } else if (rect.bottom() > area.bottom()) {
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() + qMin(
                rect.bottom() - area.bottom(), rect.top() - area.top()));
    }

    update();
}

QModelIndex CanvasGridView::moveCursorGrid(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
//    qDebug() << modifiers <<  d->currentCursorIndex;

    auto selectionModel = this->selectionModel();
    auto headIndex = firstIndex();
    auto tailIndex = lastIndex();

    QModelIndex current = d->currentCursorIndex;
    if (!current.isValid() || !selectionModel->isSelected(current)) {
        return headIndex;
    }
    auto url = model()->getUrlByIndex(current);

    auto pos = GridManager::instance()->position(m_screenNum, url.toString());
    auto newCoord = Coordinate(pos);

    switch (cursorAction) {
    case MoveLeft:
        while (pos.x() >= 0) {
            newCoord = newCoord.moveLeft();
            pos = newCoord.position();
            if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                break;
            }
        }
        break;
    case MoveRight:
        while (pos.x() < d->colCount) {
            newCoord = newCoord.moveRight();
            pos = newCoord.position();
            if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                break;
            }
        }
        break;
    case MovePrevious:
    case MoveUp:
        while (pos.y() >= 0 && pos.x() >= 0) {
            newCoord = newCoord.moveUp();
            pos = newCoord.position();
            if (pos.y() < 0) {
                newCoord = Coordinate(pos.x() - 1, d->rowCount - 1);
                pos = newCoord.position();
            }
            if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                break;
            }
        }
        break;
    case MoveNext:
    case MoveDown:
        while (pos.y() < d->rowCount && pos.x() < d->colCount) {
            newCoord = newCoord.moveDown();
            pos = newCoord.position();
            if (pos.y() >= d->rowCount) {
                newCoord = Coordinate(pos.x() + 1, 0);
                pos = newCoord.position();
            }
            if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                break;
            }
        }
        break;
    case MoveHome:
    case MovePageUp: {
        if (modifiers == Qt::ShiftModifier) {
            QItemSelection selection;
            while (pos.y() >= 0 && pos.x() >= 0) {
                newCoord = newCoord.moveUp();
                pos = newCoord.position();
                if (pos.y() < 0) {
                    newCoord = Coordinate(pos.x() - 1, d->rowCount - 1);
                    pos = newCoord.position();
                }
                if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                    auto localFile = GridManager::instance()->itemTop(m_screenNum, pos.x(), pos.y());
                    //GridManager::instance()->itemId(m_screenNum, pos.x(), pos.y());
                    auto index = model()->index(DUrl(localFile));

                    QItemSelectionRange selectionRange(index);
                    selection.push_back(selectionRange);
                }
            }
            QAbstractItemView::selectionModel()->select(selection, QItemSelectionModel::Select);
        }
        return headIndex;
    }
    case MoveEnd:
    case MovePageDown: {
        if (modifiers == Qt::ShiftModifier) {
            QItemSelection selection;
            while (pos.y() < d->rowCount && pos.x() < d->colCount) {
                newCoord = newCoord.moveDown();
                pos = newCoord.position();
                if (pos.y() >= d->rowCount) {
                    newCoord = Coordinate(pos.x() + 1, 0);
                    pos = newCoord.position();
                }
                if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                    auto localFile = GridManager::instance()->itemTop(m_screenNum, pos.x(), pos.y());
                    //GridManager::instance()->itemId(m_screenNum, pos.x(), pos.y());
                    auto index = model()->index(DUrl(localFile));

                    QItemSelectionRange selectionRange(index);
                    selection.push_back(selectionRange);
                }
            }

            QAbstractItemView::selectionModel()->select(selection, QItemSelectionModel::Select);
        }
        return tailIndex;
    }
    }

    if (!d->isVaildCoordinate(newCoord)) {
        return current;
    }

    auto localFile = GridManager::instance()->itemTop(m_screenNum, pos.x(), pos.y());
    //GridManager::instance()->itemId(m_screenNum, pos.x(), pos.y());
    auto newIndex = model()->index(DUrl(localFile));
    if (newIndex.isValid()) {
        return newIndex;
    }

    //qDebug() << selectedUrls();
    return current;
}

void CanvasGridView::updateHiddenItems()
{
    itemDelegate()->hideAllIIndexWidget();
    clearSelection();

    auto filters  = model()->filters();
    filters = GridManager::instance()->getWhetherShowHiddenFiles() ?
              filters | QDir::Hidden : filters & ~QDir::Hidden;
    qDebug() << "current filters" << GridManager::instance()->getWhetherShowHiddenFiles()
             << filters << m_screenName << m_screenNum;
    model()->setFilters(filters);
    delayModelRefresh(0);
    return;
}

void CanvasGridView::updateExpandItemGeometry()
{
    updateEditorGeometries();
}

void CanvasGridView::setGeometry(const QRect &rect)
{
    //!防止获取到的屏幕区域是0x0的时候崩溃
    if (rect.size().width() < 1 || rect.size().height() < 1) {
        return;
    } else {
        QAbstractItemView::setGeometry(rect);
        updateCanvas();
        if (d->waterMaskFrame)
            d->waterMaskFrame->updatePosition();
    }
}

bool CanvasGridView::fetchDragEventUrlsFromSharedMemory()
{
    QSharedMemory sm;
    sm.setKey(DRAG_EVENT_URLS);

    if (!sm.isAttached()) {
        if (!sm.attach()) {
            qWarning() << "FQSharedMemory detach failed.";
            return false;
        }
    }

    QBuffer buffer;
    QDataStream in(&buffer);

    sm.lock();
    //用缓冲区得到共享内存关联后得到的数据和数据大小

//    buffer.setData((char*)sm.constData(), sm.size());//解决警告采用下方方式
    buffer.setData(static_cast<char *>(const_cast<void *>(sm.constData())), sm.size());

    buffer.open(QBuffer::ReadOnly);     //设置读取模式
    in >> m_urlsForDragEvent;               //使用数据流从缓冲区获得共享内存的数据，然后输出到字符串中
    sm.unlock();    //解锁
    sm.detach();//与共享内存空间分离

    return true;
}

void CanvasGridView::setTargetUrlToApp(const QMimeData *data, const DUrl &url)
{
    //仅当target改变的时候才调用DFileDragClient::setTargetUrl
    if (!m_currentTargetUrl.isValid() || m_currentTargetUrl.path() != url.path()) {
        m_currentTargetUrl = url;
        DFileDragClient::setTargetUrl(data, url);
    }
}

void CanvasGridView::delayModelRefresh(int ms)
{
    if (m_refreshTimer != nullptr) {
        m_refreshTimer->stop();
        delete m_refreshTimer;
        m_refreshTimer = nullptr;
        qDebug() << "reset refresh timer" << m_screenNum;
    }

    if (ms < 1) {
        qDebug() << "now refresh " << m_refreshTimer << m_screenNum;
        m_rt.start();
        model()->refresh();
        return;
    }

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, [ = ]() {
        m_refreshTimer->stop();
        qDebug() << "beging refresh " << m_refreshTimer << m_screenNum;
        m_rt.start();
        model()->refresh();
    });

    m_refreshTimer->start(ms);
}

void CanvasGridView::delayArrage(int ms)
{
    static QTimer *arrangeTimer = nullptr;
    if (arrangeTimer != nullptr) {
        arrangeTimer->stop();
        delete arrangeTimer;
        arrangeTimer = nullptr;
        qDebug() << "reset timer" << m_screenNum;
    }
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
    if (ms < 1) {
        d->bReloadItem = true;
        qDebug() << "beging sort " << arrangeTimer << m_screenNum;
        model()->setEnabledSort(true);
        model()->sort();
        return;
    }

    arrangeTimer = new QTimer;
    connect(arrangeTimer, &QTimer::timeout, this, [ = ]() {
        arrangeTimer->stop();
        d->bReloadItem = true;
        qDebug() << "beging sort " << arrangeTimer << m_screenNum;
        model()->setEnabledSort(true);
        model()->sort();
    });
#else
    if (ms < 1) {
        qDebug() << "beging arrage " << arrangeTimer << m_screenNum;
        auto list = GridManager::instance()->allItems();
        qDebug() << "initArrage file count" << list.size();
        GridManager::instance()->initArrage(list);
        GridManager::instance()->delaySyncAllProfile();

        emit GridManager::instance()->sigSyncOperation(GridManager::soUpdate);
        return;
    }

    arrangeTimer = new QTimer;
    connect(arrangeTimer, &QTimer::timeout, this, [ = ]() {
        arrangeTimer->stop();
        auto list = GridManager::instance()->allItems();
        qDebug() << "initArrage file count" << list.size();
        GridManager::instance()->initArrage(list);
        GridManager::instance()->delaySyncAllProfile();

        emit GridManager::instance()->sigSyncOperation(GridManager::soUpdate);
    });
#endif
    arrangeTimer->start(ms);
}

void CanvasGridView::delayCustom(int ms)
{
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
    if (GridManager::instance()->shouldArrange()) {
#else
    if (GridManager::instance()->autoMerge()) {
#endif
        return;
    }

    static QTimer *arrangeTimer = nullptr;
    if (arrangeTimer != nullptr) {
        arrangeTimer->stop();
        delete arrangeTimer;
        arrangeTimer = nullptr;
        qDebug() << "reset Custom timer" << m_screenNum;
    }

    if (ms < 1) {
        QStringList list;
        for (int i = 0; i < model()->rowCount(); ++i) {
            auto index = model()->index(i, 0);
            auto localFile = model()->getUrlByIndex(index).toString();
            list << localFile;
        }
        qDebug() << "initCustom file count" << list.size();
        GridManager::sortMainDesktopFile(list, model()->sortRole(), model()->sortOrder());
        GridManager::instance()->initCustom(list);
#ifndef USE_SP2_AUTOARRAGE   //sp3需求改动
        if (GridManager::instance()->autoArrange()) {
            delayArrage(0);
        }
#endif
        emit GridManager::instance()->sigSyncOperation(GridManager::soUpdate);
        return;
    }

    arrangeTimer = new QTimer;
    connect(arrangeTimer, &QTimer::timeout, this, [ = ]() {
        arrangeTimer->stop();
        QStringList list;
        for (int i = 0; i < model()->rowCount(); ++i) {
            auto index = model()->index(i, 0);
            auto localFile = model()->getUrlByIndex(index).toString();
            list << localFile;
        }
        GridManager::sortMainDesktopFile(list, model()->sortRole(), model()->sortOrder());
        auto oriItems = GridManager::instance()->allItems();
        qDebug() << "initCustom file count" << list.size() << " and oriItems count " << oriItems.count();
        GridManager::instance()->initCustom(list);
#ifndef USE_SP2_AUTOARRAGE   //sp3需求改动
        if (GridManager::instance()->autoArrange()) {
            delayArrage(0);
        }
#endif

        //fix bug #32527
        if (list.isEmpty() && !oriItems.isEmpty()) {
            delayModelRefresh(500);
        }

        emit GridManager::instance()->sigSyncOperation(GridManager::soUpdate);
    });
    arrangeTimer->start(ms);
}

void CanvasGridView::delayAutoMerge(int ms)
{
    if (!GridManager::instance()->autoMerge()) {
        return;
    }

    static QTimer *arrangeTimer = nullptr;
    if (arrangeTimer != nullptr) {
        arrangeTimer->stop();
        delete arrangeTimer;
        arrangeTimer = nullptr;
        qDebug() << "reset autoMerge timer" << m_screenNum;
    }

    if (ms < 1) {
        QStringList list;
        for (int i = 0; i < model()->rowCount(); ++i) {
            auto index = model()->index(i, 0);
            auto localFile = model()->getUrlByIndex(index).toString();
            list << localFile;
        }
        qDebug() << "now initArrage file count" << list.size()
                 << "expend" << currentUrl().fragment();
        GridManager::instance()->initArrage(list);
        return;
    }

    arrangeTimer = new QTimer;
    connect(arrangeTimer, &QTimer::timeout, this, [ = ]() {
        arrangeTimer->stop();
        QStringList list;
        for (int i = 0; i < model()->rowCount(); ++i) {
            auto index = model()->index(i, 0);
            auto localFile = model()->getUrlByIndex(index).toString();
            list << localFile;
        }
        qDebug() << "initArrage file count" << list.size()
                 << "expend" << currentUrl().fragment() << "screen" << m_screenNum;
        GridManager::instance()->initArrage(list);
    });
    arrangeTimer->start(ms);
}

DUrl CanvasGridView::currentCursorFile() const
{
    DUrl ret;
    DAbstractFileInfoPointer fp = model()->fileInfo(d->currentCursorIndex);
    if (fp) {
        ret = fp->fileUrl();
    }
    return ret;
}

void CanvasGridView::syncIconLevel(int level)
{
    if (itemDelegate()->iconSizeLevel() == level) {
        return;
    }

    itemDelegate()->setIconSizeByIconSizeLevel(level);
    updateCanvas();
}

WId CanvasGridView::winId() const
{
    if (isTopLevel()) {
        return QAbstractItemView::winId();
    } else {
        return topLevelWidget()->winId();
    }
}

void CanvasGridView::setAutoMerge(bool enabled)
{
    GridManager::instance()->setAutoMerge(enabled);
    if (enabled) {
        //this->setRootUrl(DUrl(DFMMD_ROOT MERGEDDESKTOP_FOLDER));
        //刷新虚拟路径时是先查看是否已有展开状态
        DUrl virtualExpandUrl = GridManager::instance()->getCurrentVirtualExpandUrl();
        if (virtualExpandUrl.fragment().isEmpty()) {
            this->setRootUrl(DUrl(DFMMD_ROOT MERGEDDESKTOP_FOLDER));
        } else {
            this->setRootUrl(virtualExpandUrl);
        }
    } else {
        // sa
        QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
        DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

        if (!QDir(desktopPath).exists()) {
            QDir::home().mkpath(desktopPath);
        }

        //复位成未展开的状态
        DUrl clearUrl;
        GridManager::instance()->setCurrentVirtualExpandUrl(DUrl(QUrl()));
        this->setRootUrl(desktopUrl);
    }
}

// please make sure the passed \a url argument is a valid virtual entry url.
void CanvasGridView::toggleEntryExpandedState(const DUrl &url)
{
    // just some simple check
    if (!url.isValid() || url.scheme() != DFMMD_SCHEME) {
        return;
    }

    clearSelection();

    // toggle expand state
    DMD_TYPES toggleType = MergedDesktopController::entryTypeByName(url.fileName());
    virtualEntryExpandState[toggleType] = !virtualEntryExpandState[toggleType];
    bool isExpand = virtualEntryExpandState[toggleType];

    // construct fragment which indicated the expanded entries
    QStringList expandedEntries;
    int possibleChildCount = 0;

    bool onlyExpandShowClickedEntry = false;
    for (unsigned int i = DMD_FIRST_TYPE; i <= DMD_ALL_TYPE; i++) {
        DMD_TYPES oneType = static_cast<DMD_TYPES>(i);
        if (oneType != DMD_FOLDER && virtualEntryExpandState[oneType]) {
            expandedEntries.append(MergedDesktopController::entryNameByEnum(oneType));
        }

        // check if icon cound is greater than desktop grid count
        possibleChildCount += 1; // 1: the virtual entry icon
        if (isExpand && virtualEntryExpandState[oneType]) {
            DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, MergedDesktopController::getVirtualEntryPath(oneType));
            if (info) {
                possibleChildCount += info->filesCount();
            }
        }
    }
    if (possibleChildCount > GridManager::instance()->gridCount()) {
        onlyExpandShowClickedEntry = true;
    }

    // prepare root url
    DUrl targetUrl(DFMMD_ROOT MERGEDDESKTOP_FOLDER);

    if (!expandedEntries.isEmpty()) {
        targetUrl.setFragment(onlyExpandShowClickedEntry ? MergedDesktopController::entryNameByEnum(toggleType) : expandedEntries.join(','));
    }

    // set root url (which will update the view)
    this->setRootUrl(targetUrl);
    //更新其他canvas的model自动整理数据。 todo:考虑优化没有扩展到的不用刷新
    QMap<QString, DUrl> mergeUpInfo;
    mergeUpInfo.insert(m_screenName, targetUrl);
    emit GridManager::instance()->sigSyncOperation(GridManager::soAutoMergeUpdate, QVariant::fromValue(mergeUpInfo));

}

void CanvasGridView::updateEntryExpandedState(const DUrl &url)
{
    this->setRootUrl(url);
}

QModelIndex CanvasGridView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    // Do not allow move when hold ctrl
    if (modifiers == Qt::ControlModifier) {
        return d->currentCursorIndex;
    }

    auto current = d->currentCursorIndex;
    if (!current.isValid()) {
        current = firstIndex();
        d->currentCursorIndex = current;
        d->m_oldCursorIndex = current;
        return current;
    }

    if (rectForIndex(current).isEmpty()) {
        qCritical() << "current never empty" << current;
        d->currentCursorIndex = firstIndex();
        d->m_oldCursorIndex = current;
        return d->currentCursorIndex;
    }

    QModelIndex index = moveCursorGrid(cursorAction, modifiers);

    if (index.isValid()) {
        d->currentCursorIndex = index;
        if (!DFMGlobal::keyShiftIsPressed())
            d->m_oldCursorIndex = index;
        return index;
    }

    d->currentCursorIndex = current;
    return current;
}

int CanvasGridView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int CanvasGridView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

bool CanvasGridView::isIndexHidden(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return false;
}

void CanvasGridView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    setSelection(rect, command, false);
}

QRegion CanvasGridView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;
    auto selectedList = selection.indexes();
    for (auto &index : selectedList) {
        region = region.united(QRegion(visualRect(index)));
    }
    return region;
}

void CanvasGridView::mouseMoveEvent(QMouseEvent *event)
{
    //不关心Dropflag，节省时间,bug#10926
    IgnoreDropFlag idf(model());

    if (event->buttons() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    QAbstractItemView::mouseMoveEvent(event);

    auto curPos = event->pos();
    QRect selectRect;

    if (d->showSelectRect) {
        selectRect.setLeft(qMin(curPos.x(), d->lastPos.x()));
        selectRect.setTop(qMin(curPos.y(), d->lastPos.y()));
        selectRect.setRight(qMax(curPos.x(), d->lastPos.x()));
        selectRect.setBottom(qMax(curPos.y(), d->lastPos.y()));
        d->selectRect = selectRect.normalized();
    }

    if (d->showSelectRect) {
        update();
        setState(DragSelectingState);
        auto command = QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect;
        setSelection(selectRect, command, true);
    }
}

void CanvasGridView::mousePressEvent(QMouseEvent *event)
{
    auto index = indexAt(event->pos());

    d->mousePressed = true;

    if (index.isValid()) {
        d->currentCursorIndex = index;
        //shift 开始index无效时更新无当前点击项，未按下shift点击时更新为当前点击项
        if (!d->m_oldCursorIndex.isValid() || !DFMGlobal::keyShiftIsPressed())
            d->m_oldCursorIndex = index;
    }

    bool leftButtonPressed = event->button() == Qt::LeftButton;
    bool showSelectFrame = leftButtonPressed;
    showSelectFrame &= !index.isValid();
    d->showSelectRect = showSelectFrame;
    d->lastPos = event->pos();

    //如果是触摸屏，就开启200ms计时，再响应drag
    //当事件source为MouseEventSynthesizedByQt，认为此事件为TouchBegin转换而来
    if ((event->source() == Qt::MouseEventSynthesizedByQt) && leftButtonPressed) {
        //读取dde配置的按压时长
        static QObject *theme_settings = reinterpret_cast<QObject *>(qvariant_cast<quintptr>(qApp->property("_d_theme_settings_object")));
        QVariant touchFlickBeginMoveDelay;
        if (theme_settings) {
            touchFlickBeginMoveDelay = theme_settings->property("touchFlickBeginMoveDelay");
        }
        //若dde配置了则使用dde的配置，若没有则使用默认的200ms
        d->touchTimer.setInterval(touchFlickBeginMoveDelay.isValid() ? touchFlickBeginMoveDelay.toInt() : 200);
        d->touchTimer.start();
    } else {
        d->touchTimer.stop();
    }

    bool isEmptyArea = !index.isValid();
    if (index.isValid() && itemDelegate()->editingIndex() == index) {
        //如果当前点击的是正在编辑的项目
        //不关闭编辑框,fix#bug25523 桌面文件进行重命名操作，在重命名输入框中右键，自动退出重命名状态
    } else
        itemDelegate()->commitDataAndCloseActiveEditor();

    if (isEmptyArea) {
        if (!DFMGlobal::keyCtrlIsPressed() && !DFMGlobal::keyShiftIsPressed()) {
            itemDelegate()->hideNotEditingIndexWidget();
            QAbstractItemView::setCurrentIndex(QModelIndex());
            clearSelection();
            d->currentCursorIndex = QModelIndex();
            d->m_oldCursorIndex = QModelIndex();
        }
    }

    d->beforeMoveSelection = selectionModel()->selection();
    //auto selectedIndexes = selectionModel()->selectedIndexes();
    bool isselected = isSelected(index);
//    QAbstractItemView::mousePressEvent(event);
    //fix 修改ctrl+左键取消选中状态导致所有选中文件被取消选中的问题。
    if (leftButtonPressed && isselected && event->modifiers() == Qt::ControlModifier) {
#if 0   //反选功能 暂不清楚是否需要，不开启
        selectedIndexes.removeOne(index);
        QItemSelection selection;
        for (const QModelIndex &mi : selectedIndexes) {
            selection << QItemSelectionRange(mi);
        }
        d->beforeMoveSelection = selection;
        selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
#else
        setProperty("lastPressedIndex", index);

        //fixbug39610:必须先取出已选择列表，再调用mousePressEvent(不调用会导致不能连续拖拽复制)，再设置选中列表为获取的项
        QItemSelection selection;
        for (const QModelIndex &mi : selectedIndexes()) {
            selection << QItemSelectionRange(mi);
        }

        QAbstractItemView::mousePressEvent(event);

        selectionModel()->select(selection, QItemSelectionModel::Select);
#endif
    } else {
        QAbstractItemView::mousePressEvent(event);
    }

    if (leftButtonPressed) {
        d->m_currentMousePressIndex = index;
        if (!isEmptyArea) {
            const DUrl &url = model()->getUrlByIndex(index);
            DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
            if (info && info->isVirtualEntry()) {
                toggleEntryExpandedState(url);
            }
        }
    }

    update();
}

void CanvasGridView::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractItemView::mouseReleaseEvent(event);
    d->mousePressed = false;
    if (d->showSelectRect && d->selectRect.isValid()) {
        d->showSelectRect = false;
        d->selectRect = QRect();

        auto releaseIndex = indexAt(event->pos());
        if (releaseIndex.isValid()) {
            //按照产品要求，框选时鼠标释放位置若为有效index时，设置该index为焦点index，
            //若释放时位置的index无效，则会在moveCursor中默认为左上角第一个
            d->currentCursorIndex = releaseIndex;
            d->m_oldCursorIndex = releaseIndex;
        }
    }

    QModelIndex index = property("lastPressedIndex").toModelIndex();
    if (index.isValid() && DFMGlobal::keyCtrlIsPressed() && index == indexAt(event->pos()) && isSelected(index)) {
        //fix 修改ctrl+左键取消选中状态导致所有选中文件被取消选中的问题。
        selectionModel()->select(QItemSelection(index, index), QItemSelectionModel::Deselect);
        setProperty("lastPressedIndex", QModelIndex());
    }

    update();
}

void CanvasGridView::mouseDoubleClickEvent(QMouseEvent *event)
{
    auto pos = event->pos();
    QModelIndex index = indexAt(pos);

    //bug59277桌面修改文件夹名称，双击进入文件夹，会进入桌面目录
    //若在编辑中则直接完成编辑，并在重名完成后使用新的名称发送doubleClicked信号
    if (itemDelegate()->editingIndexWidget() && (index == itemDelegate()->editingIndex())) {
        //结束重名
        itemDelegate()->commitDataAndCloseActiveEditor();
        QTimer::singleShot(200,this,[this,pos]() {
            //因为重名后原有的index会改变，但所处位置pos不变，因此使用pos重新获取index
            emit doubleClicked(indexAt(pos));
        });
        return;
    }

    QPersistentModelIndex persistent = index;
    emit doubleClicked(persistent);
    if ((event->button() == Qt::LeftButton) && !edit(persistent, DoubleClicked, event)
            && !style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, nullptr, this)) {
        emit activated(persistent);
    }
}

void CanvasGridView::wheelEvent(QWheelEvent *event)
{
    if (DFMGlobal::keyCtrlIsPressed()) {
        if (event->angleDelta().y() > 0) {
            increaseIcon();
        } else {
            decreaseIcon();
        }
        //QThread::msleep(100);
        event->accept();
    }
}

void CanvasGridView::keyPressEvent(QKeyEvent *event)
{
    if (Q_UNLIKELY(DFMApplication::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopShortcuts", false).toBool())) {
        bool specialShortcut = false;
        if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::KeypadModifier) {
            switch (event->key()) {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                specialShortcut = true;
                break;
            default:
                return;
            }
        }

        if (!specialShortcut) {
            return;
        }
    }

    QMap<QString, DUrl> selectUrlsMap;
    auto rootUrl = model()->rootUrl();
    bool canDeleted = true;
    for (const QModelIndex &index : selectionModel()->selectedIndexes()) {
        auto url = model()->getUrlByIndex(index);
        if (url.isEmpty()) {
            canDeleted = false;
            continue;
        }
        const DAbstractFileInfoPointer fileInfo = model()->fileInfo(index);
        if (fileInfo && !fileInfo->isVirtualEntry()) {
            selectUrlsMap.insert(url.toString(), url);
        }
    }
    selectUrlsMap.remove(rootUrl.toString());

    const DUrlList &selectUrls = selectUrlsMap.values();

    switch (event->modifiers()) {
    case Qt::NoModifier:
        switch (event->key()) {
        case Qt::Key_F1: {
            class PublicApplication : public DApplication
            {
            public:
                using  DApplication::handleHelpAction;
            };

            QString app_name = qApp->applicationName();
            qApp->setApplicationName("dde");
            reinterpret_cast<PublicApplication *>(DApplication::instance())->handleHelpAction();
            qApp->setApplicationName(app_name);
            break;
        }
        case Qt::Key_Tab: {
            this->selectionModel()->clear();
            QKeyEvent downKey(QEvent::KeyPress, Qt::Key_Down,  Qt::NoModifier);
            QCoreApplication::sendEvent(this, &downKey);
            break;
        }
        case Qt::Key_Escape: {
            auto tmpUrl = DFMGlobal::instance()->fetchUrlsFromClipboard();
            auto homePath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
            if (tmpUrl.size() > 0) {
                DAbstractFileInfoPointer tempInfo = DFileService::instance()->createFileInfo(nullptr, MergedDesktopController::convertToRealPath(DUrl(tmpUrl.first())));
                if (tempInfo->path() == homePath)
                    DFMGlobal::instance()->clearClipboard();
            }
            break;
        }
        default:
            break;
        }
        // fall through
        //为解决fall through警告而添加的下面一句
        Q_FALLTHROUGH();
    case Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (!itemDelegate()->editingIndex().isValid()) {
                DUrlList lst;
                for (const DUrl &url : selectUrlsMap) {
                    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
                    if (!info || info->isVirtualEntry()) {
                        // we do expand the virtual entry on single click, so no longer need to do that here.
                        // toggleEntryExpandedState(url);
                        continue;
                    }
                    lst << url;
                }
                if (1 == lst.count()) {
                    DFileService::instance()->openFile(this, lst.first());
                } else {
                    DFileService::instance()->openFiles(this, lst, true);
                }
                return;
            }
            break;
        case Qt::Key_F5:
            Refresh();
            return;
        case Qt::Key_Delete:
            if (canDeleted && !selectUrlsMap.contains(rootUrl.toString()) && !selectUrls.isEmpty()) {
                DFileService::instance()->moveToTrash(this, selectUrls);
            }
            break;
        case Qt::Key_Space: {
            QStringList urls = GridManager::instance()->itemIds(m_screenNum);
            DUrlList entryUrls;
            foreach (QString url, urls) {
                entryUrls << DUrl(url);
            }
            DUrlList selectUrlsActual = MergedDesktopController::convertToRealPaths(selectUrls);
            DFMGlobal::showFilePreviewDialog(selectUrlsActual, entryUrls);
        }
        break;
        default:
            break;
        }
        break;

    case Qt::ShiftModifier:
        if (event->key() == Qt::Key_Delete) {
            if (!canDeleted || selectUrls.isEmpty()) {
                return;
            }

            DFileService::instance()->deleteFiles(this, selectUrls);

            return;
        } else if (event->key() == Qt::Key_T) {
// Open           appController->actionOpenInTerminal(fmevent);
            return;
        }
        break;

    case Qt::ControlModifier:
        switch (event->key()) {
        case Qt::Key_Minus:
            decreaseIcon();
            return;
        case Qt::Key_Equal:
            increaseIcon();
            return;
        case Qt::Key_H: {
            //更改显示隐藏状态
            bool whetherShowHiddenFiles{ GridManager::instance()->getWhetherShowHiddenFiles() };
            GridManager::instance()->setWhetherShowHiddenFiles(!whetherShowHiddenFiles);

            //同步刷新,交由GridViewManager调用updateHiddenItems
            emit GridManager::instance()->sigSyncOperation(GridManager::soHidenSwitch);
            return;
        }
        case Qt::Key_I:
            DFMGlobal::showPropertyDialog(nullptr, selectUrls);
            return;

        default:
            break;
        }
        break;

    case Qt::ControlModifier | Qt::ShiftModifier:

        if (event->key() == Qt::Key_N) {
            if (itemDelegate()->editingIndex().isValid()) {
                return;
            }
//            clearSelection();
//            appController->actionNewFolder(fmevent);
            return;
        }

        if (event->key() == Qt::Key_Question) {
//            appController->actionShowHotkeyHelp(fmevent);
            return;
        }
        if (event->key() == Qt::Key_Plus) {
            increaseIcon();
            return;
        }
        break;

    case Qt::AltModifier:
        if (event->key() == Qt::Key_M) {
            //新需求gesetting控制右键菜单隐藏功能,和产品确认调整为gsetting高于本身配置文件，即gsetting有相关配置后本身的json相关配置失效
            auto tempGsetting = GridManager::instance()->isGsettingShow("context-menu", QVariant());
            if (tempGsetting.isValid()) {
                if (!tempGsetting.toBool())
                    return;
            } else {
                auto tempConfig = DFMApplication::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopContextMenu", QVariant());
                if (tempConfig.isValid())
                    if (!tempConfig.toBool())
                        return;
            }

            QModelIndexList indexList = selectionModel()->selectedIndexes();
            bool isEmptyArea = indexList.empty();
            Qt::ItemFlags flags;

            if (isEmptyArea) {
                flags = model()->flags(rootIndex());

                if (!flags.testFlag(Qt::ItemIsEnabled)) {
                    return;
                }
            } else {
                const QModelIndex &index = indexList.first();
                flags = model()->flags(index);

                if (!flags.testFlag(Qt::ItemIsEnabled)) {
                    isEmptyArea = true;
                    flags = rootIndex().flags();
                }
            }

            // fix bug94233 在桌面右键,右键菜单缺少选项
            if (!DFileMenuManager::actionWhitelist().isEmpty() || !DFileMenuManager::actionBlacklist().isEmpty()) {
                //! set menu actions filter
                setMenuActionsFilter();
            }

            if (isEmptyArea) {
                itemDelegate()->hideNotEditingIndexWidget();
                clearSelection();
                showEmptyAreaMenu(flags);
            } else {
                const QModelIndex &index = indexList.first();
                showNormalMenu(index, flags);
            }
            return;
        }
        break;

    default:
        break;
    }

    QAbstractItemView::keyPressEvent(event);

//    DUtil::TimerSingleShot(10, [this]() {
//        auto index = d->currentCursorIndex;
//        auto marginWidth = d->cellHeight;
//        auto rect = visualRect(index).marginsAdded(QMargins(marginWidth, marginWidth, marginWidth, marginWidth));
//        repaint(rect);
//    });
    update();
}

void CanvasGridView::dragEnterEvent(QDragEnterEvent *event)
{
    // 修复bug-65773
    m_currentTargetUrl.clear();
    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        setTargetUrlToApp(event->mimeData(), currentUrl());
        return;
    }
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
    if (event->source()) {
#else
    if (!GridManager::instance()->autoMerge()) {
#endif
        //从非桌面拖入时，不启动让位动画（同步dragMoveEvent的处理方式），否则将导致桌面已选择的图标在绘制时被隐藏
        CanvasGridView *view = dynamic_cast<CanvasGridView *>(event->source());
        if (view && event->mimeData()) {
            //拖拽复制时，不做让位处理
            if (!GridManager::instance()->shouldArrange() && !DFMGlobal::keyCtrlIsPressed()) {
                d->startDodge = true;
            }
        }
        itemDelegate()->hideNotEditingIndexWidget();
    }

    //由于普通用户无法访问root用户的共享内存，跨用户的情况使用从mimedata中取url的方式
    bool sameUser = DFMGlobal::isMimeDatafromCurrentUser(event->mimeData());
    if (sameUser) {
        fetchDragEventUrlsFromSharedMemory();
    } else {
        m_urlsForDragEvent = event->mimeData()->urls();
    }

    d->fileViewHelper->preproccessDropEvent(event, m_urlsForDragEvent);

    if (event->mimeData()->hasFormat("XdndDirectSave0")) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }

    update(); // bug 23351: while draging it, refresh it ASAP to remove the older icon
    QAbstractItemView::dragEnterEvent(event);
}

void CanvasGridView::dragMoveEvent(QDragMoveEvent *event)
{
    d->dodgeDelayTimer.stop();
    d->dragTargetGrid = QPoint(-1, -1);

    auto pos = event->pos();
    auto hoverIndex = indexAt(event->pos());

    auto startDodgeAnimation = [ = ]() {
        d->dragTargetGrid.setX((pos.x() - d->viewMargins.left()) / d->cellWidth);
        d->dragTargetGrid.setY((pos.y() - d->viewMargins.top()) / d->cellHeight);

        // FIXME: out of border???
        auto localeFile = GridManager::instance()->itemId(m_screenNum, d->dragTargetGrid);
        if (!localeFile.isEmpty() && !d->dodgeAnimationing) {
            d->dodgeDelayTimer.start();
        }

        d->fileViewHelper->preproccessDropEvent(event, m_urlsForDragEvent);
        if (!hoverIndex.isValid()) {
            if (DFileDragClient::checkMimeData(event->mimeData())) {
                event->acceptProposedAction();
                setTargetUrlToApp(event->mimeData(), currentUrl());
            } else {
                event->accept();
            }
        }
    };

    if (hoverIndex.isValid()) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(hoverIndex);
        CanvasGridView *view = dynamic_cast<CanvasGridView *>(event->source());
        if (fileInfo) {
            if (view && !DFMGlobal::keyCtrlIsPressed()) {
                event->setDropAction(Qt::MoveAction);
            }

            bool canDrop = fileInfo->canDrop();
            canDrop = fileInfo->isDir() && !fileInfo->isWritable();
            canDrop = fileInfo->supportedDropActions().testFlag(event->dropAction());
            //解决未使用警告，不清楚是否能直接删除上方canDrop相关，故采用宏
            Q_UNUSED(canDrop)
            if (!fileInfo->canDrop() || (fileInfo->isDir() && !fileInfo->isWritable()) ||
                    !fileInfo->supportedDropActions().testFlag(event->dropAction())) {
                // not support drag
                event->ignore();
            } else {
                if (DFileDragClient::checkMimeData(event->mimeData())) {
                    event->acceptProposedAction();
                    setTargetUrlToApp(event->mimeData(), fileInfo->fileUrl());
                } else {
                    event->accept();
                }
                // 当为追加压缩时
                if (fileInfo->canDrop() && fileInfo->canDragCompress()) {
                    // 设置当拖拽gvfs文件时，不支持追加压缩
                    if (!m_urlsForDragEvent.isEmpty()) {
                        event->setDropAction(Qt::CopyAction);
                        const DAbstractFileInfoPointer &dragfileInfo = DFileService::instance()->createFileInfo(this, DUrl(m_urlsForDragEvent.first()));
                        if (dragfileInfo->isGvfsMountFile()) {
                            event->setDropAction(Qt::MoveAction);
                            return event->ignore();
                        }
                    }
                }
                update();
                return;
            }
        }
    }

#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
    if (!GridManager::instance()->shouldArrange()) {   //自定义
#else
    //fixbug39610 区分图标拖拽移动和拖拽复制操作
    if (!GridManager::instance()->autoMerge() && !DFMGlobal::keyCtrlIsPressed()) {   //自定义
#endif
        CanvasGridView *view = dynamic_cast<CanvasGridView *>(event->source());
        if (view && event->mimeData()) {
            QPair<int, QPoint> orgpos;
            //找项目源
            auto urls = event->mimeData()->urls();
            if (!urls.isEmpty() && GridManager::instance()->find(urls.first().toString(), orgpos)) {
                if (orgpos.first == m_screenNum) { //同屏
                    startDodgeAnimation();
                    update();
                    return;
                }        //跨屏拖动，空间不足时先禁止拖动，todo 寻找更好的解决办法
                else if (GridManager::instance()->emptyPostionCount(m_screenNum) >= urls.size()) {
                    startDodgeAnimation();
                    update();
                    return;
                }
            } else {
                qWarning() << "not found items" << urls;
            }
        }
        //end
    }

    {
        //自动整理(sp2自动排列)以及不触发让位的drag处理
        d->fileViewHelper->preproccessDropEvent(event, m_urlsForDragEvent);
        if (!hoverIndex.isValid()) {
            if (DFileDragClient::checkMimeData(event->mimeData())) {
                event->acceptProposedAction();
                setTargetUrlToApp(event->mimeData(), currentUrl());
            } else {
                event->accept();
            }
        }
    }
    update();
}

void CanvasGridView::dragLeaveEvent(QDragLeaveEvent *event)
{
    d->dodgeDelayTimer.stop();
    d->startDodge = false;
    d->dragTargetGrid = QPoint(-1, -1);
    QAbstractItemView::dragLeaveEvent(event);
    update();
}

void CanvasGridView::dropEvent(QDropEvent *event)
{
    d->dodgeDelayTimer.stop();
    d->startDodge = false;
    d->dragTargetGrid = QPoint(-1, -1);

    QModelIndex targetIndex = indexAt(event->pos());

    //list保持顺序
    QList<QString> selectLocalFiles;
    //辅助selectLocalFiles做查找，判断是否重复
    QHash<QString, bool> selectLocalFileMap;

    auto selects = selectionModel()->selectedIndexes();
    bool dropOnSelf = false;
    for (auto index : selects) {
        auto info = model()->fileInfo(index);

        if (!info) {
            continue;
        }

        if (targetIndex == index) {
            dropOnSelf = true;
        }

        const QString fileUrl = info->fileUrl().toString();
        //重复的不再添加
        if (!selectLocalFileMap.contains(fileUrl)) {
            selectLocalFiles << fileUrl;
            selectLocalFileMap.insert(fileUrl, 0);
        }

    }

    qDebug() << "selectLocalFiles urls:" << selectLocalFiles;
    DAbstractFileInfoPointer targetInfo = model()->fileInfo(indexAt(event->pos()));
    if (!targetInfo || dropOnSelf) {
        targetInfo = model()->fileInfo(rootIndex());
    }

    //防止桌面的计算机/回收站/主目录被拖拽复制到其他目录中
    if (indexAt(event->pos()).isValid() &&
            (targetInfo->isDir() ||
             targetInfo->fileUrl() == DesktopFileInfo::homeDesktopFileUrl())) {
        for (QUrl url : m_urlsForDragEvent) {
            DUrl durl(url);
            if ((DesktopFileInfo::computerDesktopFileUrl() == durl) ||
                    (DesktopFileInfo::trashDesktopFileUrl() == durl) ||
                    (DesktopFileInfo::homeDesktopFileUrl() == durl)) {
                event->setDropAction(Qt::IgnoreAction);
                return;
            }
        }
    }

    CanvasGridView *sourceView = dynamic_cast<CanvasGridView *>(event->source());
    if (sourceView && !DFMGlobal::keyCtrlIsPressed()) {
        event->setDropAction(Qt::MoveAction);
    } else {
        d->fileViewHelper->preproccessDropEvent(event);
    }

    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        event->setDropAction(Qt::CopyAction);

        const QModelIndex &index = indexAt(event->pos());
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(index.isValid() ? index : rootIndex());

        if (fileInfo && fileInfo->fileUrl().isLocalFile()) {
            if (fileInfo->isDir()) {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->fileUrl());
            } else {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->parentUrl());
            }
        }

        event->accept(); // yeah! we've done with XDS so stop Qt from further event propagation.
    } else {
        if (event->dropAction() == Qt::MoveAction) {
            QModelIndex dropIndex = indexAt(gridRectAt(event->pos()).center());
            if (sourceView && (!dropIndex.isValid() || dropOnSelf)) {
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
                //自动排列和自动整理不允许屏幕间拖动
                if (GridManager::instance()->shouldArrange()) {
#else
                //自动整理不允许屏幕间拖动
                if (GridManager::instance()->autoMerge()) {
#endif
                    return;
                }

                auto point = event->pos();
                auto row = (point.x() - d->viewMargins.left()) / d->cellWidth;
                auto col = (point.y() - d->viewMargins.top()) / d->cellHeight;

                QList<QUrl> urls = event->mimeData()->urls();
                qDebug() << "event urls:" << urls;
                for (auto url : urls) {
                    const QString fielUrl = url.toString();
                    if (!selectLocalFileMap.contains(fielUrl)) {
                        selectLocalFiles << fielUrl;
                        selectLocalFileMap.insert(fielUrl, 0);
                    }
                }

                qDebug() << "selectLocalFiles2222 urls:" << selectLocalFiles;
                QPair<int, QPoint> orgpos;
                //找项目源
                if (!selectLocalFiles.isEmpty()
                        && GridManager::instance()->find(*selectLocalFiles.begin(), orgpos)) {
                    if (orgpos.first == m_screenNum) { //同屏
                        //获取焦点
                        QString current = sourceView->currentCursorFile().toString();
                        qDebug() << "move " << m_screenNum << "focus" << current << "count" << selectLocalFiles.size();
                        GridManager::instance()->move(m_screenNum, selectLocalFiles, current, row, col);
                    } else { //跨屏
                        //获取源屏幕的焦点
                        QString current = sourceView->currentCursorFile().toString();
                        qDebug() << "move form" << orgpos.first << "to" << m_screenNum
                                 << "focus" << current << selectLocalFiles.size();
                        GridManager::instance()->move(orgpos.first, m_screenNum, selectLocalFiles, current, row, col);
                    }
                }

                setState(NoState);
                itemDelegate()->hideNotEditingIndexWidget();
#ifndef USE_SP2_AUTOARRAGE   //sp3需求改动
                if (GridManager::instance()->autoArrange()) {
                    delayArrage(0);
                }
#endif
                emit GridManager::instance()->sigSyncOperation(GridManager::soUpdate);
                return;
            } else if (sourceView && !targetIndex.isValid()) { //桌面窗口上互相拖动到有文件的格子上，drop到的targetIndex无效时直接返回
                qInfo() << "drop on invaild target, skip. drop:" << dropIndex.row() << dropIndex.column();
                return;
            }
        }

        if (!targetIndex.isValid()) {
            targetIndex = rootIndex();
        }

        if (model()->supportedDropActions() & event->dropAction() && model()->flags(targetIndex) & Qt::ItemIsDropEnabled) {
            const Qt::DropAction action = event->dropAction();
            if (model()->dropMimeData(event->mimeData(), action, targetIndex.row(), targetIndex.column(), targetIndex)) {
                if (action != event->dropAction()) {
                    event->setDropAction(action);
                    event->accept();
                } else {
                    event->acceptProposedAction();
                }
            }
        }

        setState(NoState);
        viewport()->update();
    }
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
    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        setTargetUrlToApp(event->mimeData(), model()->getUrlByIndex(targetIndex));

        // DFileDragClient deletelater() will be called after connection destroyed
        DFileDragClient *c = new DFileDragClient(event->mimeData());
        DUrlList urlList = DUrl::fromQUrlList(event->mimeData()->urls());
        connect(c, &DFileDragClient::stateChanged, this, [this, urlList](DFileDragState state) {
            if (state == Finished) {
                select(urlList);
            }
        });
    }
}

void CanvasGridView::paintEvent(QPaintEvent *event)
{
    //不关心Dropflag，节省时间,bug#10926
    IgnoreDropFlag idf(model());

    QPainter painter(viewport());
    auto repaintRect = event->rect();
    painter.setRenderHints(QPainter::HighQualityAntialiasing);

    auto option = viewOptions();
    option.textElideMode = Qt::ElideMiddle;

    const QModelIndex current = d->currentCursorIndex;
    const QAbstractItemModel *itemModel = this->model();
    const DFileSelectionModel *selections = this->selectionModel();
    const bool focusEnabled = false;
    const bool focus = (hasFocus() || viewport()->hasFocus()) && current.isValid() && focusEnabled;
    const QStyle::State state = option.state;
    const QAbstractItemView::State viewState = this->state();
    const bool enabled = (state & QStyle::State_Enabled) != 0;

    painter.setBrush(QColor(255, 0, 0, 0));

    if (d->_debug_show_grid) {
        painter.save();
        if (model()) {
            for (int i = 0; i < d->colCount * d->rowCount; ++i) {
                auto  pos = d->indexCoordinate(i).position();
                auto x = pos.x() * d->cellWidth + d->viewMargins.left();
                auto y = pos.y() * d->cellHeight + d->viewMargins.top();

                auto rect =  QRect(x, y, d->cellWidth, d->cellHeight);

                int rowMode = pos.x() % 2;
                int colMode = pos.y() % 2;
                auto color = (colMode == rowMode) ? QColor(0, 0, 255, 32) : QColor(255, 0, 0, 32);
                painter.fillRect(rect, color);

                if (pos == d->dragTargetGrid) {
                    painter.fillRect(rect, Qt::green);
                }
                painter.setPen(QPen(Qt::red, 2));
                painter.drawText(rect, QString("%1-%2").arg(pos.x()).arg(pos.y()));

                //为测试开启debug模式下鼠标框选热区
                QMargins margins(10, 10, 10, 10);
                rect = rect.marginsRemoved(margins);
                painter.setPen(Qt::red);
                painter.drawRect(rect);
            }
        }
        painter.restore();
    }

    DUrlList selecteds;
    if (d->dodgeAnimationing || d->startDodge) {
        selecteds = selectedUrls();
    }

//    qDebug() << d->dragIn << d->dodgeAnimationing;
    if (d->startDodge) {
        auto currentMousePos = mapFromGlobal(QCursor::pos());
        auto hoverIndex = indexAt(currentMousePos);
        auto url = model()->getUrlByIndex(hoverIndex);

        if (selecteds.contains(url)
                || (d->dodgeAnimationing && d->dodgeItems.contains(url.toString()))) {

        } else {
            if (hoverIndex.isValid() && hoverIndex != d->currentCursorIndex) {
                QPainterPath path;
                auto lastRect = visualRect(hoverIndex);
                path.addRoundRect(lastRect, 4, 4);
                painter.fillPath(path, QColor(43, 167, 248, 255 * 3 / 10));
                painter.strokePath(path, QColor(30, 126, 255, 255 * 2 / 10));
            }
        }
    }

    QStringList repaintLocalFiles;
    if (d->fileViewHelper->isPaintFile()) {
        for (int x = 0; x < d->colCount; ++x) {
            for (int y = 0; y < d->rowCount; ++y) {
                auto localFile = GridManager::instance()->itemId(m_screenNum, x, y);
                if (!localFile.isEmpty()) {
                    repaintLocalFiles << localFile;
                }
            }
        }

        //放入堆叠
        auto overlayItems = GridManager::instance()->overlapItems(m_screenNum);
        for (int i = 0; i < overlayItems.length(); ++i) {
            auto localFile = overlayItems.value(i);
            if (!localFile.isEmpty()) {
                repaintLocalFiles << localFile;
            }
        }
    }

//    int drawCount = 0;
    for (auto &localFile : repaintLocalFiles) {
        auto url = DUrl(localFile);

        /* 按照产品要求，拖拽时，展示拖拽源位置图片
        // hide selected if draw animation
        //拖拽复制时，原图标保持
        if ((d->dodgeAnimationing || d->startDodge) && selecteds.contains(url) && !DFMGlobal::keyCtrlIsPressed()) {
//            qDebug() << "skip drag select" << url;
            continue;
        }
        */


        if (d->dodgeAnimationing && d->dodgeItems.contains(localFile)) {
//            qDebug() << "skip  dragMoveItems" << localFile;
            continue;
        }

        auto index = model()->index(url);
        if (!index.isValid()) {
//            qDebug() << "skip index.isValid";
            continue;
        }
        option.rect = visualRect(index);

        bool needflash = false;
        for (auto &rr : event->region().rects())
            if (rr.intersects(option.rect)) {
                needflash = true;
            }

        if (!needflash) {
//            qDebug() << "skip !needflash";
            continue;
        }


        if (!repaintRect.intersects(option.rect)) {
//            qDebug() << "skip !repaintRect.intersects(option.rect)";
            continue;
        }

        option.rect = option.rect.marginsRemoved(d->cellMargins);
        option.state = state;
        if (selections && selections->isSelected(index)) {
            option.state |= QStyle::State_Selected;
        }
        if (enabled) {
            QPalette::ColorGroup cg;
            if ((itemModel->flags(index) & Qt::ItemIsEnabled) == 0) {
                option.state &= ~QStyle::State_Enabled;
                cg = QPalette::Disabled;
            } else {
                cg = QPalette::Normal;
            }
            option.palette.setCurrentColorGroup(cg);
        }
        if (focus && current == index) {
            option.state |= QStyle::State_HasFocus;
            if (viewState == EditingState) {
                option.state |= QStyle::State_Editing;
            }
        }
        option.state &= ~QStyle::State_MouseOver;

        painter.save();
        if (d->_debug_show_grid) {
            for (auto rect : itemPaintGeomertys(index)) {
                painter.setPen(Qt::red);
                painter.drawRect(rect);
            }
        }

        this->itemDelegate()->paint(&painter, option, index);
        DAbstractFileInfoPointer info = model()->fileInfo(index);
        if (info && info->scheme() == DFMMD_SCHEME && info->isVirtualEntry()) {
            DMD_TYPES oneType = MergedDesktopController::entryTypeByName(info->fileName());
            if (virtualEntryExpandState[oneType]) {
                // do draw mask here
                static QIcon expandMaskIcon = QIcon::fromTheme("folder-stack-mask");
                const QRect itemRect = itemIconGeomerty(index);
                QPixmap pixmap = DFMStyledItemDelegate::getIconPixmap(expandMaskIcon, itemRect.size(), devicePixelRatioF());
                painter.drawPixmap(itemRect.topLeft(), pixmap);
            }
        }
        painter.restore();
    }

    // draw select rect copy from QListView::paintEvent
    if (d->showSelectRect && d->selectRect.isValid()) {
        QStyleOptionRubberBand opt;
        opt.initFrom(this);
        opt.shape = QRubberBand::Rectangle;
        opt.opaque = false;
        opt.rect = d->selectRect;
        painter.save();
        style()->drawControl(QStyle::CE_RubberBand, &opt, &painter);
        painter.restore();
    }

// draw dragMove animation
    if (d->dodgeAnimationing) {
        for (auto animatingItem : d->dodgeItems) {
            auto localFile = animatingItem;
            auto index = model()->index(DUrl(localFile));
            if (index.isValid()) {
                option.rect = visualRect(index).marginsRemoved(d->cellMargins);
            }

            auto gridPos = d->dodgeTargetGrid->pos(m_screenNum, localFile);
            auto x = gridPos.x() * d->cellWidth + d->viewMargins.left();
            auto y = gridPos.y() * d->cellHeight + d->viewMargins.top();

            QRect end = QRect(x, y, d->cellWidth, d->cellHeight).marginsRemoved(d->cellMargins);

            auto tempCurrent = dodgeDuration();
            auto nx = option.rect.x() + (end.x() - option.rect.x()) * tempCurrent;
            auto ny = option.rect.y() + (end.y() - option.rect.y()) * tempCurrent;
            option.rect.setX(static_cast<int>(nx));
            option.rect.setY(static_cast<int>(ny));
            option.rect.setSize(end.size());
            painter.save();
            itemDelegate()->paint(&painter, option, index);
            painter.restore();
        }
    }
}

void CanvasGridView::focusInEvent(QFocusEvent *event)
{
    QAbstractItemView::focusInEvent(event);
    itemDelegate()->commitDataAndCloseActiveEditor();

    /// set menu actions filter
    setMenuActionsFilter();
}

void CanvasGridView::focusOutEvent(QFocusEvent *event)
{
    QAbstractItemView::focusOutEvent(event);
    d->startDodge = false;
}

void CanvasGridView::contextMenuEvent(QContextMenuEvent *event)
{
    // fix bug94233 在桌面右键,右键菜单缺少选项
    if (!DFileMenuManager::actionWhitelist().isEmpty() || !DFileMenuManager::actionBlacklist().isEmpty()) {
        //! set menu actions filter
        setMenuActionsFilter();
    }

    //fix bug39609 选中桌面文件夹，进行右键操作，例如打开、重命名，然后再按快捷键home、left、right无效
    d->mousePressed = false;

    //新需求gesetting控制右键菜单隐藏功能,和产品确认调整为gsetting高于本身配置文件，即gsetting有相关配置后本身的json相关配置失效
    auto tempGsetting = GridManager::instance()->isGsettingShow("context-menu", QVariant());
    if (tempGsetting.isValid()) {
        if (!tempGsetting.toBool())
            return;
    } else {
        auto tempConfig = DFMApplication::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopContextMenu", QVariant());
        if (tempConfig.isValid())
            if (!tempConfig.toBool())
                return;
    }

    //关闭编辑框
    itemDelegate()->hideAllIIndexWidget();

    //fix buf#202007010011,忽略drop判断，提升响应速度
    IgnoreDropFlag idf(model());

    const QModelIndex &index = indexAt(event->pos());
    bool indexIsSelected = selectionModel()->isSelected(index);
    bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos()) && !indexIsSelected;
    Qt::ItemFlags flags;

    if (isEmptyArea) {
        flags = model()->flags(rootIndex());

        if (!flags.testFlag(Qt::ItemIsEnabled)) {
            return;
        }
    } else {
        flags = model()->flags(index);

        if (!flags.testFlag(Qt::ItemIsEnabled)) {
            isEmptyArea = true;
            flags = rootIndex().flags();
        }
    }

    if (isEmptyArea) {
        itemDelegate()->hideNotEditingIndexWidget();
        clearSelection();
        //*d->lastMenuPos() = qMakePair(m_screenNum,gridAt(event->pos()));
        d->lastMenuPos = event->pos();
        showEmptyAreaMenu(flags);
    } else {
        if (!selectionModel()->isSelected(index)) {
            setCurrentIndex(index);
        }

        showNormalMenu(index, flags);
    }
}

bool CanvasGridView::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        updateCanvas();
    }

    return QAbstractItemView::event(event);
}

void CanvasGridView::rowsInserted(const QModelIndex &parent, int first, int last)
{
    QAbstractItemView::rowsInserted(parent, first, last);

    for (int index = first; index <= last; ++index) {
        const QModelIndex &child = parent.child(index, 0);
        DAbstractFileInfoPointer info = model()->fileInfo(child);
        if (info) {
            info->makeToActive();
        } else {
            qCritical() << "CanvasGridView::rowsInserted(): Create file info failed!!!!!!!!!!!!!" << index;
        }
    }

    update();
}

void CanvasGridView::keyboardSearch(const QString &search)
{
    if (search.isEmpty())
        return;

    d->fileViewHelper->keyboardSearch(search.toLocal8Bit().at(0));
}

QPixmap CanvasGridView::renderToPixmap(const QModelIndexList &indexes) const
{
    qreal scale = 1;
    QWidget *window = this->window();
    if (window) {
        QWindow *windowHandle = window->windowHandle();
        if (windowHandle)
            scale = windowHandle->devicePixelRatio();
    }

    //将当前按住的index剔除
    QModelIndexList indexesWithoutPressed = indexes;
    auto needRemove = [=](const QModelIndex &index) {
        return index.row() == d->m_currentMousePressIndex.row();
    };
    indexesWithoutPressed.erase(std::remove_if(indexesWithoutPressed.begin(), indexesWithoutPressed.end(), needRemove),
                                indexesWithoutPressed.end());
    //拖拽聚合图标可绘制区域大小
    QRect pixRect(0, 0, DRAGICON_SIZE + DRAGICON_OUTLINE * 2, DRAGICON_SIZE + DRAGICON_OUTLINE * 2);
    QPixmap pixmap(pixRect.size() * scale);
    pixmap.setDevicePixelRatio(scale);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option = this->viewOptions();
    option.state |= QStyle::State_Selected;
    option.rect = option.rect.translated(DRAGICON_OUTLINE, DRAGICON_OUTLINE);

    qreal offsetX = pixRect.width() / 2;
    qreal offsetY = pixRect.height() / 2;
    for (int i = qMin(DRAGICON_MAX - 1, indexesWithoutPressed.length() - 1); i >= 0 ; --i) {
        //计算旋转角度
        qreal rotate = DRAGICON_ROTATE * (ceil((i + 1.0) / 2.0) / 2.0 + 1.0) * (i % 2 == 1 ? -1 : 1);
        //设置透明度 50% 40% 30% 20%
        painter.setOpacity(1.0 - (i + 5) * DRAGICON_OPACITY);

        //旋转
        painter.translate(offsetX, offsetY); //让图片的中心作为旋转的中心
        painter.rotate(rotate);
        painter.translate(-offsetX, -offsetY); //使原点复原

        //绘制icon
        this->itemDelegate()->paintDragIcon(&painter, option, indexesWithoutPressed.at(i), QSize(DRAGICON_SIZE, DRAGICON_SIZE));

        //旋转回原角度
        painter.translate(offsetX, offsetY);
        painter.rotate(-rotate);
        painter.translate(-offsetX, -offsetY);
    }

    //绘制当前按住的icon,顶层icon80%透明度
    painter.setOpacity(0.8);
    this->itemDelegate()->paintDragIcon(&painter, option, d->m_currentMousePressIndex, QSize(DRAGICON_SIZE, DRAGICON_SIZE));
    QSize iconSize = this->itemDelegate()->getIndexIconSize(option, d->m_currentMousePressIndex, QSize(DRAGICON_SIZE, DRAGICON_SIZE));

    //绘制数量提示原点，大于99个文件显示为99+
    int length = indexes.length() > DRAGICON_MAX_COUNT ? 28 : 24; //原点直径：1到2个字符直径为24，3个字符直径为28
    int x = DRAGICON_OUTLINE + (DRAGICON_SIZE + iconSize.width() - length) / 2;
    int y = DRAGICON_OUTLINE + (DRAGICON_SIZE + iconSize.height() - length) / 2;

    QColor pointColor(244, 74, 74);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setPen(pointColor);
    painter.setBrush(pointColor);
    painter.drawEllipse(x, y, length, length);

    painter.setPen(Qt::white);
    //按照设计的要求设置字体：Arial，12大小,粗体
    QFont ft("Arial");
    ft.setPixelSize(12);
    ft.setBold(true);
    painter.setFont(ft);
    QString countStr = indexes.length() > DRAGICON_MAX_COUNT ? QString::number(DRAGICON_MAX_COUNT).append("+") : QString::number(indexes.length());
    painter.drawText(QRect(x, y, length, length), Qt::AlignCenter, countStr);

    return pixmap;
}

QString CanvasGridView::canvansScreenName() const
{
    return m_screenName;
}

QRect CanvasGridView::rectForIndex(const QModelIndex &index) const
{
    return visualRect(index);
}

DUrl CanvasGridView::currentUrl() const
{
    return model()->getUrlByIndex(rootIndex());
}

DFileSystemModel *CanvasGridView::model() const
{
    return qobject_cast<DFileSystemModel *>(QAbstractItemView::model());
}

DFileSelectionModel *CanvasGridView::selectionModel() const
{
    return static_cast<DFileSelectionModel *>(QAbstractItemView::selectionModel());
}

DesktopItemDelegate *CanvasGridView::itemDelegate() const
{
    return qobject_cast<DesktopItemDelegate *>(QAbstractItemView::itemDelegate());
}

void CanvasGridView::setItemDelegate(DesktopItemDelegate *delegate)
{
    QAbstractItemDelegate *dg = QAbstractItemView::itemDelegate();

    if (dg) {
        dg->deleteLater();
    }

    QAbstractItemView::setItemDelegate(delegate);
}

double CanvasGridView::dodgeDuration() const
{
    return d->dodgeDuration;
}

QMargins CanvasGridView::cellMargins() const
{
    return d->cellMargins;
}

QSize CanvasGridView::cellSize() const
{
    return QSize(d->cellWidth, d->cellHeight);
}

void CanvasGridView::openUrl(const DUrl &url)
{
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    if (!info || info->isVirtualEntry()) {
        // we do expand the virtual entry on single click, so no longer need to do that here.
        // toggleEntryExpandedState(url);
        return;
    }

    DFileService::instance()->openFile(this, url);
}

void CanvasGridView::openUrls(const QList<DUrl> &urls)
{
    if (urls.isEmpty())
        return;
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, urls.at(0));
    if (!info || info->isVirtualEntry()) {
        return;
    }

    DFileService::instance()->openFiles(this, urls);
}

bool CanvasGridView::setCurrentUrl(const DUrl &url)
{
    DUrl fileUrl = url;
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, fileUrl);
    if (!info) {
        qWarning() << "This scheme isn't support";
        return false;
    }

    const DUrl &checkUrl = currentUrl();

    if (checkUrl == fileUrl) {
        return false;
    }

    QModelIndex index = model()->setRootUrl(fileUrl);
    setRootIndex(index);

    if (focusWidget() && focusWidget()->window() == window() && fileUrl.isLocalFile()) {
        QDir::setCurrent(fileUrl.toLocalFile());
    }

    QAbstractItemView::setCurrentIndex(QModelIndex());

    model()->setFilters(model()->filters());

    if (d->filesystemWatcher) {
        disconnect(d->filesystemWatcher, nullptr, this, nullptr);
        d->filesystemWatcher->deleteLater();
    }

    if (!model()->canFetchMore(index)) {

        // TODO: updateContentLabel
        qDebug() << "TODO: updateContentLabel()";
    } else { //if (GridManager::instance()->autoMerge()){
        m_rt.restart();
        model()->refresh();
        qDebug() << "refresh" << m_screenNum << fileUrl;
    }

    d->filesystemWatcher = model()->fileWatcher();

#if 0 //!由于model中已做了刷新，故无需下面代码 task#40201
    //fix bug#30019 当.hidden文件改变时刷新model,用于实时更新隐藏文件
    connect(d->filesystemWatcher, &DAbstractFileWatcher::fileModified, this, [this](const DUrl & url) {
        if (url.fileName() == ".hidden" && !(model()->filters() & QDir::Hidden))
            delayModelRefresh();
    });
    //end
#endif

    connect(d->filesystemWatcher, &DAbstractFileWatcher::subfileCreated,
    this, [ = ](const DUrl & url) {
        Q_EMIT itemCreated(url);
        update();
    });

    connect(d->filesystemWatcher, &DAbstractFileWatcher::fileDeleted,
    this, [ = ](const DUrl & url) {
        Q_EMIT itemDeleted(url);
        update();
    });

    connect(d->filesystemWatcher, &DAbstractFileWatcher::fileMoved,
    this, [ = ](const DUrl & oriUrl, const DUrl & dstUrl) {

        bool findOldPos = false;
        QPoint oldPos = QPoint(-1, -1);

        if (GridManager::instance()->contains(m_screenNum, oriUrl.toString()) && !oriUrl.fileName().isEmpty()) {
            oldPos = GridManager::instance()->position(m_screenNum, oriUrl.toString());
            findOldPos = true;
        }

        bool findNewPos = false;
        if (dstUrl.parentUrl() == oriUrl.parentUrl() && !dstUrl.fileName().isEmpty()) {
            findNewPos = true;
        }

        findNewPos &= !GridManager::instance()->contains(m_screenNum, dstUrl.toString());

        if (!findNewPos) {
            Q_EMIT itemDeleted(oriUrl);
        } else {
            if (findOldPos) {
                GridManager::instance()->remove(m_screenNum, oriUrl.toString());

                //文件被重名后变成隐藏文件
                DAbstractFileInfoPointer file = fileService->createFileInfo(nullptr, dstUrl);
                //判断文件是否为隐藏文件，当前若不显示隐藏文件，则跳过
                if (file && file->isHidden() && !(model()->filters() & QDir::Hidden)) {
                    qDebug() << dstUrl << "is hidden file，do not show";

                    //变成隐藏文件，出现了空位，需要执行一次排列
                    if (GridManager::instance()->autoArrange())
                        this->delayArrage();
                } else
                    GridManager::instance()->add(m_screenNum, oldPos, dstUrl.toString());

                if (GridManager::instance()->autoMerge())
                    this->delayModelRefresh();
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
                else if (GridManager::instance()->autoArrange())
                    this->delayArrage();
#endif
            } else {
                //在多屏情况下，该信号会导致文件改名后被移动位置（原因是每个画布对象都有一个watcher，文件改名时每个画布都会处理，从而非文件所在的画布就会发送该信号）
                //同步，查找old文件名所在屏和所在位置，仅在old名图标所在的屏处理，避免重命名时被非old名图标所在屏处理
                QPair<int, QPoint> pos;
                bool isFound = GridManager::instance()->find(oriUrl.toString(), pos);
                if (isFound && pos.first != m_screenNum) {
                    qDebug() << QString("item: %1 is found, but it doesn't belong to %2 %3, it's belong %4").
                               arg(oriUrl.toString()).
                               arg(m_screenNum).
                               arg(m_screenName).
                               arg(pos.first)
                            << pos.second;
                    return ;
                } else {
                    auto screenNumbers = GridManager::instance()->allScreenNum();
                    for (auto num : screenNumbers) {
                        if (num < m_screenNum && !GridManager::instance()->getCanvasFullStatus(num)) {
                            qDebug() << QString("The previous Canvas(%1)is not full!!! so, current Canvas(%2) don't emit itemCreated signal !").
                                        arg(num).
                                        arg(m_screenNum);
                            return;
                        }
                    }
                }
                Q_EMIT itemCreated(dstUrl);
                qDebug() << QString("%1 isFound = %2 ,findOldPos = %3, findNewPos = %4, emit itemCreated signal!").
                            arg(oriUrl.toString()).
                            arg(isFound).
                            arg(findOldPos).
                            arg(findNewPos);
            }
        }

        update();
    });
    return true;
}

void CanvasGridView::initRootUrl()
{
    setAutoMerge(GridManager::instance()->autoMerge());
}

bool CanvasGridView::setRootUrl(const DUrl &url)
{
    if (url.isEmpty()) {
        return false;
    }

    if (url.scheme() == DFMMD_SCHEME) {
        for (unsigned int oneType = DMD_PICTURE; oneType <= DMD_OTHER; oneType++) {
            virtualEntryExpandState[static_cast<DMD_TYPES>(oneType)] = false;
        }

        QString frag = url.fragment();
        if (!frag.isEmpty()) {
            QStringList entryNameList = frag.split(',', QString::SkipEmptyParts);
            for (const QString &oneEntry : entryNameList) {
                virtualEntryExpandState[MergedDesktopController::entryTypeByName(oneEntry)] = true;
            }
        }
        //虚拟路径展开后立即保存
        if (GridManager::instance()->doneInit())
            GridManager::instance()->setCurrentVirtualExpandUrl(url);
    }

    itemDelegate()->hideAllIIndexWidget();

    clearSelection();

    return setCurrentUrl(url);
}

const DUrlList CanvasGridView::autoMergeSelectedUrls() const
{
    auto selects = selectionModel()->selectedIndexes();
    DUrlList urls;
    for (auto index : selects) {
        auto info = model()->fileInfo(index);

        //为了满足自定义模式下对多个文件的重命名时的replaceText里的判断
        //这里提前改成对应的Durl
        if (info && !info->isVirtualEntry()) {
            if (GridManager::instance()->autoMerge()) {
                if (!info)
                    continue ;
                QString fileBaseName{ info->baseName() };
                const QString &suffix = info->suffix().isEmpty() ? QString() : QString(".") + info->suffix();
                if (fileBaseName.trimmed().isEmpty()) {
                    qWarning() << "replace fileBaseName(not include suffix) trimmed is empty string";
                    continue;
                }
                int max_length = MAX_FILE_NAME_CHAR_COUNT - suffix.toLocal8Bit().size();
                if (fileBaseName.toLocal8Bit().size() > max_length) {
                    fileBaseName = DFMGlobal::cutString(fileBaseName, max_length, QTextCodec::codecForLocale());
                }
                DUrl vUrl{ info->getUrlByNewFileName(fileBaseName + suffix) };
                urls << vUrl;
            }
        }
    }
    return urls;
}

const DUrlList CanvasGridView::selectedUrls() const
{
    auto selects = selectionModel()->selectedIndexes();
    DUrlList urls;
    for (auto index : selects) {
        auto info = model()->fileInfo(index);
        if (info && !info->isVirtualEntry()) {
            urls << info->fileUrl();
        }
    }
    return urls;
}

void CanvasGridView::viewSelectedUrls(DUrlList &validSel, QModelIndexList &validIndexes)
{
    auto selects = selectionModel()->selectedIndexes();
    for (auto index : selects) {
        auto info = model()->fileInfo(index);
        if (info && !info->isVirtualEntry() && GridManager::instance()->contains(m_screenNum, info->fileUrl().toString())) {
            validSel << info->fileUrl();
            validIndexes << index;
        }
    }
}

void CanvasGridView::setScreenNum(int num)
{
    m_screenNum = num;
}

void CanvasGridView::setScreenName(const QString &name)
{
    m_screenName = name;
}

bool CanvasGridView::isSelected(const QModelIndex &index) const
{
    return static_cast<DFileSelectionModel *>(selectionModel())->isSelected(index);
}

void CanvasGridView::select(const QList<DUrl> &list)
{
    //不关心Dropflag，节省时间 bug#202007010011
    IgnoreDropFlag idf(model());

    QModelIndex lastIndex;
    QItemSelection selection;

    clearSelection();

    for (auto &url : list) {
        auto index = model()->index(url);
        QItemSelectionRange selectionRange(index);
        if (!selection.contains(index)) {
            selection.push_back(selectionRange);
        }
        lastIndex = index;
    }

    if (!selection.isEmpty()) {
        auto selectModel = static_cast<DFileSelectionModel *>(selectionModel());
        selectModel->select(selection, QItemSelectionModel::Select);
    }

    if (lastIndex.isValid()) {
        selectionModel()->setCurrentIndex(lastIndex, QItemSelectionModel::Select);

        //fix #bug100789 根据产品要求，在粘贴导致原有选中项中的焦点项被取消选中状态时，已粘贴后的扩展方向的最后一个为焦点
        d->currentCursorIndex = lastIndex;
        d->m_oldCursorIndex = lastIndex;
    }
}

int CanvasGridView::selectedIndexCount() const
{
    return static_cast<const DFileSelectionModel *>(selectionModel())->selectedCount();
}

bool CanvasGridView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    DUrl fileUrl = model()->getUrlByIndex(index);
    //解决自动整理只包含一个有效图标时全选重命名无效问题
    auto validSelectCount = selectedUrls().size();
//    if (fileUrl.isEmpty() || selectedIndexCount() > 1 || (trigger == SelectedClicked && DFMGlobal::keyShiftIsPressed())) {
    if (fileUrl.isEmpty() || validSelectCount > 1 || (trigger == SelectedClicked && DFMGlobal::keyShiftIsPressed())) {
        return false;
    }

    if (trigger == SelectedClicked) {
        QStyleOptionViewItem option = viewOptions();

        option.rect = visualRect(index).marginsRemoved(d->cellMargins);

        const QRect &file_name_rect = itemDelegate()->fileNameRect(option, index);

        if (!file_name_rect.contains(static_cast<QMouseEvent *>(event)->pos())) {
            return false;
        }
    }

    if (QWidget *w = indexWidget(index)) {
        Qt::ItemFlags flags = model()->flags(index);
        if (((flags & Qt::ItemIsEditable) == 0) || ((flags & Qt::ItemIsEnabled) == 0)) {
            return false;
        }
        if (state() == QAbstractItemView::EditingState) {
            return false;
        }
        if (trigger == QAbstractItemView::AllEditTriggers) { // force editing
            d->fileViewHelper->triggerEdit(index);
            return true;
        }
        if ((trigger & editTriggers()) == QAbstractItemView::SelectedClicked
                && !selectionModel()->isSelected(index)) {
            return false;
        }

        if (trigger & editTriggers()) {
            w->setFocus();
            d->fileViewHelper->triggerEdit(index);
            return true;
        }
    }
    bool tmp = QAbstractItemView::edit(index, trigger, event);
    if (tmp) {
        d->fileViewHelper->triggerEdit(index);
    }

    return tmp;
}

void CanvasGridView::setDodgeDuration(double dodgeDuration)
{
    if (qFuzzyCompare(d->dodgeDuration, dodgeDuration)) {
        return;
    }

    d->dodgeDuration = dodgeDuration;
    emit dodgeDurationChanged(d->dodgeDuration);
}

void CanvasGridView::selectAll()
{
    //全选按桌面显示顺序对文件进行选择
    DUrlList all = DUrl::fromStringList(GridManager::instance()->allItems());
    select(all);
}

void CanvasGridView::onRefreshFinished()
{
    qDebug() << "fresh ending spend " << m_rt.elapsed() << m_screenNum;
    model()->setEnabledSort(false);
    if (GridManager::instance()->autoMerge()) {
        delayAutoMerge();
    }
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
    else if (GridManager::instance()->autoArrange()) {
        delayArrage();
    }
#endif
    else {  //自定义
        delayCustom();
    }
}

void CanvasGridView::EnableUIDebug(bool enable)
{
    d->_debug_log = enable;
    d->_debug_show_grid = enable;
}

QString CanvasGridView::Size()
{
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream debug(&buffer);

    debug << GridManager::instance()->gridSize(m_screenNum);

    return QString::fromUtf8(buffer.buffer());
}

QString CanvasGridView::Dump()
{
    GridManager::instance()->dump();
    return "";
}

#include <QJsonObject>

QString CanvasGridView::DumpPos(qint32 x, qint32 y)
{
    QJsonObject debug;

    auto px = x * d->cellWidth + d->cellWidth / 2 + d->viewMargins.left();
    auto py = y * d->cellHeight + d->cellHeight / 2 + d->viewMargins.top();

    QModelIndex index = indexAt(QPoint(px, py));

    debug.insert("checkPoint:", QJsonValue::fromVariant(QList<QVariant>({px, py})));
    debug.insert("index", QJsonValue::fromVariant(QList<QVariant>({index.row(), index.column()})));
    debug.insert("url", model()->getUrlByIndex(index).toString());
    debug.insert("grid content", GridManager::instance()->itemTop(m_screenNum, x, y));

    return QJsonDocument(debug).toJson();
}

void CanvasGridView::Refresh()
{
    d->fileViewHelper->viewFlicker();
}

void CanvasGridView::initUI()
{
#ifdef QT_DEBUG
    EnableUIDebug(true);
#endif

    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAutoFillBackground(false);
    setFrameShape(QFrame::NoFrame); // TODO: using QWidget instead of QFrame?

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    setDefaultDropAction(Qt::CopyAction);

    d->fileViewHelper = new CanvasViewHelper(this);
    d->fileViewHelper->setProperty("isCanvasViewHelper", true);

    setModel(new DFileSystemModel(d->fileViewHelper));
    model()->isDesktop = true;//紧急修复，由于修复bug#33209添加了一次事件循环的处理，导致桌面的自动排列在删除，恢复文件时显示异常

    //默认按类型排序
    model()->setSortRole(DFileSystemModel::FileMimeTypeRole);
    model()->setEnabledSort(true);

    //设置是否显示隐藏文件
    auto filters = model()->filters();
    filters = GridManager::instance()->getWhetherShowHiddenFiles() ?
              filters | QDir::Hidden : filters & ~QDir::Hidden;
    model()->setFilters(filters);

    setSelectionModel(new DFileSelectionModel(model(), this));
    auto delegate = new DesktopItemDelegate(d->fileViewHelper);
    delegate->setEnabledTextShadow(true);
    delegate->setFocusTextBackgroundBorderColor(Qt::white);
    setItemDelegate(delegate);

    QVariant iconSizeLevel = 1;
    iconSizeLevel = Config::instance()->getConfig(Config::groupGeneral, Config::keyIconLevel, iconSizeLevel);
    itemDelegate()->setIconSizeByIconSizeLevel(iconSizeLevel.toInt());
    qDebug() << "current icon size level" << itemDelegate()->iconSizeLevel();

    DFMSocketInterface::instance();
    DGioSettings desktopSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
    if (desktopSettings.keys().contains("water-mask") && desktopSettings.value("water-mask").toBool()) {
        d->waterMaskFrame = new WaterMaskFrame("/usr/share/deepin/dde-desktop-watermask.json", this);
        d->waterMaskFrame->lower();
        d->waterMaskFrame->updatePosition();
    }
}


void CanvasGridView::initConnection()
{
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
    [this](const QItemSelection & selected, const QItemSelection & deselected) {

        Q_UNUSED(selected)
        QModelIndex index = property("lastPressedIndex").toModelIndex();
        if (index.isValid() && deselected.contains(index)) {
            setProperty("lastPressedIndex", QModelIndex());
        }

        //同步选择状态
        emit GridManager::instance()->sigSyncSelection(this, selectedUrls());
    });

    connect(&d->dodgeDelayTimer, &QTimer::timeout,
    this, [ = ]() {
//        qDebug() << "start animation";
        d->dodgeAnimationing = true;
        auto animation = new QPropertyAnimation(this, "dodgeDuration");
        animation->setDuration(300);
        animation->setEasingCurve(QEasingCurve::InOutCubic);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);

        connect(animation, &QPropertyAnimation::valueChanged,
        this, [ = ]() {
            update();
        });

        connect(animation, &QPropertyAnimation::finished,
        this, [ = ]() {
            DUrlList selecteds = selectedUrls();
            QMap<DUrl, QPair<int, QPoint>> removed; //记录移除的
            for (auto relocateItem : selecteds) {
                auto localFile = relocateItem.toString();
                QPair<int, QPoint> pos;
                bool ret = GridManager::instance()->find(localFile, pos);
                //处理跨屏，如果是跨屏的remove失败，后面就无需再add
                if (ret && GridManager::instance()->remove(pos.first, localFile))
                    removed.insert(relocateItem, pos);
            }

            // commit dodgeTargetGrid
            for (auto relocateItem : d->dodgeItems) {
                QPoint orgPos = GridManager::instance()->position(m_screenNum, relocateItem);
                bool rmRet = GridManager::instance()->remove(m_screenNum, relocateItem);
                auto pos = d->dodgeTargetGrid->pos(m_screenNum, relocateItem);
                bool addRet = GridManager::instance()->add(m_screenNum, pos, relocateItem);
                //add失败，还原。修复bug#21943
                if (!addRet && rmRet) {
                    qWarning() << "error move!!!" << relocateItem << "from" << orgPos
                               << "to" << pos << "fail." << "put it on" << orgPos;
                    bool ret = GridManager::instance()->add(m_screenNum, orgPos, relocateItem);
                    if (!ret) {
                        qWarning() << "error resotre " << relocateItem << " to" << orgPos << "fail.";
                    }
                }
            }

            //只有被remove的才add。用于处理跨屏拖动
            for (auto relocateItem : removed.keys()) {
                QString localFile = relocateItem.toString();
                QPoint tarPos = d->dodgeTargetGrid->pos(m_screenNum, localFile);
                bool ret = GridManager::instance()->add(m_screenNum, tarPos, localFile);
                //用于修复drag在多屏之间拖动触发动画后的bug
                if (!ret) {
                    auto orgPos = removed.value(relocateItem);
                    qWarning() << "error move!!!" << localFile << "from" << orgPos
                               << "to" << tarPos << "fail." << "put it on" << orgPos;
                    ret = GridManager::instance()->add(orgPos.first, orgPos.second, localFile);
                    //还原失败，放入堆叠
                    if (!ret) {
                        int iret = GridManager::instance()->addToOverlap(localFile);
                        qWarning() << "error put " << localFile << " on" << orgPos << "fail. move to overlaps" << iret;
                    }
                }
            }
            d->dodgeAnimationing = false;
            delete d->dodgeTargetGrid;
            d->dodgeTargetGrid = nullptr;

            //隐藏选中框
            emit GridManager::instance()->sigSyncOperation(GridManager::soHideEditing);
        });
        animation->start();
        d->dodgeDelayTimer.stop();

        auto selURLs = selectedUrls();
        QStringList selLocalFiles;

        d->dodgeTargetGrid = GridManager::instance()->core();
        auto grid = d->dodgeTargetGrid;

        int emptyBefore = 0;
        int emptyAfter = 0;
        GIndex targetIndex = grid->toIndex(m_screenNum, d->dragTargetGrid);

        //获取所有的空位
        QList<GIndex> empty = grid->emptyPostion(m_screenNum);
        auto sortIndex = [targetIndex](const GIndex & i1, const GIndex & i2) ->bool{
            return qAbs(i1 - targetIndex) < qAbs(i2 - targetIndex);
        };
        qSort(empty.begin(), empty.end(), sortIndex);
        GIndex index = 0;
        for (auto sel : selURLs) {
            QString localFile = sel.toString();
            selLocalFiles << localFile;
            //抓取的项目在本屏
            if (GridManager::instance()->contains(m_screenNum, localFile)) {
                GPos pos = grid->pos(m_screenNum, localFile);
                index = grid->toIndex(m_screenNum, pos);
                grid->removeItem(m_screenNum, localFile);
            } else { //跨屏
                if (!empty.isEmpty())
                    index = empty.takeFirst();
            }

            if (index < targetIndex) {
                ++emptyBefore;
            } else {
                ++emptyAfter;
            }
        }

        if (0 == emptyAfter) {
            ++targetIndex;
        }

        d->dodgeItems = grid->reloacle(m_screenNum, targetIndex, emptyBefore, emptyAfter);
        for (auto i = 0; i < selLocalFiles.length(); ++i) {
            grid->addItem(m_screenNum, targetIndex - emptyBefore + i, selLocalFiles.value(i));
        }
    });

    //model刷新完毕
    connect(model(), &DFileSystemModel::sigJobFinished, this, &CanvasGridView::onRefreshFinished);
    connect(model(), &DFileSystemModel::requestSelectFiles,this,[this](const QList<DUrl> &urls){
        //fix bug39609 选中桌面文件夹，通过按键F2重命名，然后再按快捷键home、left、right均变为桌面第一个图标为选中
        //重设当前新文件名的index为当前index
        if (!urls.isEmpty()) {
            d->currentCursorIndex = model()->index(urls.first());
            d->m_oldCursorIndex = d->currentCursorIndex;
        }
        d->fileViewHelper->onRequestSelectFiles(urls);
    });

    connect(this->model(), &DFileSystemModel::newFileByInternal,
    this, [ = ](const DUrl & fileUrl) {
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
        if (GridManager::instance()->shouldArrange()) {
#else
        if (GridManager::instance()->autoMerge()) {
#endif
            //排完序后交由GridManager::initArrage触发重命名
            GridManager::instance()->m_needRenameItem = fileUrl.toString();
            return ;
        }
#ifndef USE_SP2_AUTOARRAGE   //sp3需求改动
        else if (GridManager::instance()->autoArrange()) {
            //开启编辑框
            emit GridManager::instance()->sigSyncOperation(GridManager::soRename, fileUrl.toString());
            return;
        }
#endif

        QString localFile = fileUrl.toString();
        int itemScreen;
        QPair<int, QPoint> orgPos;
        if (GridManager::instance()->find(localFile, orgPos)) {
            itemScreen = orgPos.first;
        } else {
            qCritical() << "cannot find item" << localFile << "screen" << m_screenNum;
            return ;
        }

        qDebug() << "newFileByInternal item" << localFile << "screen" << itemScreen
                 << "pos" << orgPos.second << "to screen" << m_screenNum;

        QPair<int, QPoint> gridPos;
        gridPos.first = m_screenNum;
        gridPos.second = gridAt(d->lastMenuPos);
        if (d->lastMenuNewFilepath == localFile) {
            if (gridPos == orgPos) {
                //开启编辑框
                goto openEditor;
            }
        }

        gridPos = GridManager::instance()->forwardFindEmpty(m_screenNum, gridPos.second);

        //无需move
        if (gridPos == orgPos) {
            goto openEditor;
        }

        //目标位置在当前屏才move
        if (gridPos.first == m_screenNum) {
            //同屏
            if (m_screenNum == itemScreen) {
                GridManager::instance()->move(m_screenNum, QStringList() << localFile,
                                              localFile, gridPos.second.x(), gridPos.second.y());
            } else { //跨屏
                GridManager::instance()->move(itemScreen, m_screenNum, QStringList() << localFile,
                                              localFile, gridPos.second.x(), gridPos.second.y());
            }
        }
        //else{} //目标位置无法在m_screenNum放下，就不处理

    openEditor:
        emit GridManager::instance()->sigSyncOperation(GridManager::soRename, fileUrl.toString());
        return ;
    });

    connect(this, &CanvasGridView::itemCreated, this, [ = ](const DUrl & url) {
        qDebug() << "CanvasGridView::itemCreated" << url << m_screenNum;
        d->lastMenuNewFilepath = url.toString();
        //!在这里add会导致自动整理在快速创建文件时crash，与DAbstractFileInfoPrivate中
        //! 的cache机制 urlToFileInfoMap 在异步时有关 关联bug#24855
        //GridManager::instance()->add(m_screenNum, d->lastMenuNewFilepath);
        //!end

        //创建或者粘贴时保持之前的状态
        if (GridManager::instance()->autoMerge()) {
            this->delayModelRefresh(100);
            return ;
        }

        //判断文件是否为隐藏文件，当前若不显示隐藏文件，则跳过
        DAbstractFileInfoPointer file = fileService->createFileInfo(nullptr, url);
        if (file && file->isHidden() && !(model()->filters() & QDir::Hidden)) {
            qDebug() << url << "is hidden file，do not show";
            return;
        }

        GridManager::instance()->add(m_screenNum, d->lastMenuNewFilepath);
#ifdef USE_SP2_AUTOARRAGE   //sp3需求改动
        if (GridManager::instance()->autoArrange()) { //重新排列
            this->delayArrage();
        }
#endif
    });

    connect(this, &CanvasGridView::itemDeleted, this, [ = ](const DUrl & url) {
        auto index = model()->index(url);
        //当选中的文件中有文件被移除时就清空所有选中项
        if (d->currentCursorIndex == index || selectionModel()->selectedIndexes().contains(index)) {
            d->currentCursorIndex = QModelIndex();
            selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Clear);
            setCurrentIndex(QModelIndex());
        }
        //fix bug55930 桌面新建文本文件和文件夹，选中文本文件剪切到文件夹 ，重命名新的文件夹 ，弹出重命名窗口
        //clear cache selected items,because index data updated
        selectionModel()->clearSelectedCaches();

        if (!GridManager::instance()->remove(m_screenNum, url.toString()))
            return;

        //自动整理
        if (GridManager::instance()->autoMerge()) {
            delayModelRefresh();
        } else if (GridManager::instance()->autoArrange()) { //重新排列
            this->delayArrage();
        } else {
            GridManager::instance()->popOverlap(); //弹出堆叠
        }
    });

    connect(this->model(), &DFileSystemModel::requestSelectFiles,
            d->fileViewHelper, &CanvasViewHelper::onRequestSelectFiles);

    connect(this->model(), &QAbstractItemModel::dataChanged,
    this, [ = ](const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> &roles) {
        qDebug() << "dataChanged" << roles << d->bReloadItem;
        if (d->bReloadItem) {
            d->bReloadItem = false;
            qDebug() << "dataChanged" << topLeft << bottomRight << roles;
            qDebug() << "resort desktop icons ";
            model()->setEnabledSort(false);

            QStringList list;
            for (int i = 0; i < model()->rowCount(); ++i) {
                auto index = model()->index(i, 0);
                auto localFile = model()->getUrlByIndex(index).toString();
                list << localFile;
            }

            GridManager::sortMainDesktopFile(list, model()->sortRole(), model()->sortOrder());
#ifdef USE_SP2_AUTOARRAGE
            //自动排列和整理
            if (GridManager::instance()->shouldArrange()) {
                GridManager::instance()->initArrage(list);
                return ;
            }

            GridManager::instance()->clear();
            //自定义
            for (auto lf : list) {
                GridManager::instance()->add(m_screenNum, lf);
            }
#else
            GridManager::instance()->initArrage(list);

            //自动整理
            if (GridManager::instance()->autoMerge()) {
                return ;
            }

            GridManager::instance()->delaySyncAllProfile();
#endif
        }
    });

    connect(this, &CanvasGridView::doubleClicked, this, [this](const QModelIndex &index) {
        DUrl url = model()->getUrlByIndex(index);
        openUrl(url);
    }, Qt::QueuedConnection);


    connect(this, &CanvasGridView::autoAlignToggled, this, [this]() {
        Presenter::instance()->onAutoAlignToggled();
        if (GridManager::instance()->autoArrange()) {
            this->delayArrage();
        } else {
            GridManager::instance()->delaySyncAllProfile(0);
        }
    });

#ifdef ENABLE_AUTOMERGE  //sp2需求调整，屏蔽自动整理
    connect(this, &CanvasGridView::autoMergeToggled, this, []() {
        bool enable = !GridManager::instance()->autoMerge();
        GridManager::instance()->setAutoMerge(enable);
        Presenter::instance()->onAutoMergeToggled();

        emit GridManager::instance()->sigSyncOperation(GridManager::soAutoMerge, enable);
    });
#endif

    connect(this, &CanvasGridView::sortRoleChanged,
            Presenter::instance(), &Presenter::onSortRoleChanged);
    connect(this, &CanvasGridView::changeIconLevel,
            Presenter::instance(), &Presenter::OnIconLevelChanged);

    connect(this, &CanvasGridView::changeIconLevel, this,
    [](int level) {
        emit GridManager::instance()->sigSyncOperation(GridManager::soIconSize, level);
    });

    connect(this, &CanvasGridView::sortRoleChanged, this,
    [](int role, Qt::SortOrder order) {
        QPoint sort(role, order);
        emit GridManager::instance()->sigSyncOperation(GridManager::soSort, sort);
    });

    connect(DFMApplication::instance(), &DFMApplication::showedHiddenFilesChanged, this, [ = ](bool isShowedHiddenFile) {
        GridManager::instance()->setWhetherShowHiddenFiles(isShowedHiddenFile);
        updateHiddenItems();
    });

    connect(DFMApplication::instance(), &DFMApplication::previewAttributeChanged, this->model(), &DFileSystemModel::update);
}

void CanvasGridView::updateCanvas()
{
    //todo计算margin
    itemDelegate()->updateItemSizeHint();
    auto itemSize = itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());
    QMargins geometryMargins = QMargins(0, 0, 0, 0);
    d->updateCanvasSize(this->geometry().size(), this->geometry().size(), geometryMargins, itemSize);

    GridManager::instance()->updateGridSize(m_screenNum, d->colCount, d->rowCount);

    auto expandedWidget = reinterpret_cast<QWidget *>(itemDelegate()->expandedIndexWidget());
    if (expandedWidget) {
        int offset = -1 * ((d->cellWidth - itemSize.width()) % 2);
        QMargins margins(offset, d->cellMargins.top(), 0, 0);
        expandedWidget->setContentsMargins(margins);
    }

    //需在expandedWidget->setContentsMargins(margins)之后在更新，否则在计算时没有将Margins纳入计算，导致显示错误
    updateEditorGeometries();
    update();
}

void CanvasGridView::setIconByLevel(int level)
{
    if (itemDelegate()->iconSizeLevel() == level) {
        return;
    }
    itemDelegate()->setIconSizeByIconSizeLevel(level);
    emit this->changeIconLevel(itemDelegate()->iconSizeLevel());
    updateCanvas();
}

void CanvasGridView::increaseIcon()
{
    // TODO: 3 is 128*128, 0,1,2,3
    if (itemDelegate()->iconSizeLevel() >= 4) {
        return;
    }
    itemDelegate()->increaseIcon();
    emit this->changeIconLevel(itemDelegate()->iconSizeLevel());
    updateCanvas();
}

void CanvasGridView::decreaseIcon()
{
    itemDelegate()->decreaseIcon();
    emit this->changeIconLevel(itemDelegate()->iconSizeLevel());
    updateCanvas();
}

inline QPoint CanvasGridView::gridAt(const QPoint &pos) const
{
    auto row = (pos.x() - d->viewMargins.left()) / d->cellWidth;
    auto col = (pos.y() - d->viewMargins.top()) / d->cellHeight;
    return QPoint(row, col);
}

inline QRect CanvasGridView::gridRectAt(const QPoint &pos) const
{
    auto row = (pos.x() - d->viewMargins.left()) / d->cellWidth;
    auto col = (pos.y() - d->viewMargins.top()) / d->cellHeight;

    auto x = row * d->cellWidth + d->viewMargins.left();
    auto y = col * d->cellHeight + d->viewMargins.top();
    return QRect(x, y, d->cellWidth, d->cellHeight).marginsRemoved(d->cellMargins);
}

inline QList<QRect> CanvasGridView::itemPaintGeomertys(const QModelIndex &index) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect = visualRect(index).marginsRemoved(d->cellMargins);
    return itemDelegate()->paintGeomertys(option, index);
}

inline QRect CanvasGridView::itemIconGeomerty(const QModelIndex &index) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect = visualRect(index).marginsRemoved(d->cellMargins);
    auto rects = itemDelegate()->paintGeomertys(option, index);
    if (rects.isEmpty()) {
        return option.rect;
    }

    return rects.value(0);
}


inline QModelIndex CanvasGridView::firstIndex()
{
    auto localFile = GridManager::instance()->firstItemId(m_screenNum);
    return model()->index(DUrl(localFile));
}

inline QModelIndex CanvasGridView::lastIndex()
{
    //使用堆叠
    auto localFile = GridManager::instance()->lastItemTop(m_screenNum);//lastItemId(m_screenNum);
    return model()->index(DUrl(localFile));
}

void CanvasGridView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command, bool byIconRect)
{
    Q_UNUSED(byIconRect)

    auto selectRect = rect.normalized();
    auto topLeftGridPos = gridAt(selectRect.topLeft());
    auto bottomRightGridPos = gridAt(selectRect.bottomRight());

    QItemSelection oldSelection;
    //  keep old selection if mouse press
    bool ctrlShiftPress = DFMGlobal::keyShiftIsPressed() || DFMGlobal::keyCtrlIsPressed();
    if (ctrlShiftPress) {
        oldSelection = selectionModel()->selection();
        if (0 == selectionModel()->selection().indexes().size() && DFMGlobal::keyShiftIsPressed())
            d->beginPos = topLeftGridPos;
    } else {
        d->beginPos = QPoint(-1, -1);
    }
    // select by  key board, so mouse not pressed
    if (!d->mousePressed && d->currentCursorIndex.isValid() && !ctrlShiftPress) {
        QItemSelectionRange selectionRange(d->currentCursorIndex);
        if (!oldSelection.contains(d->currentCursorIndex)) {
            oldSelection.push_back(selectionRange);
        }
        QAbstractItemView::selectionModel()->select(oldSelection, command);
        return;
    }

    //get current Point
    QPoint currentPoint(-1, -1);
    if (d->mousePressed) {
        auto clickIndex = indexAt(d->lastPos);
        if (clickIndex.isValid() && !d->showSelectRect) { //如果在框选就不进去
            QPoint tempClickIndex = visualRect(clickIndex).center();
            QPoint tempLastPoint = visualRect(d->currentCursorIndex).center();
            if (!d->currentCursorIndex.isValid())
                tempLastPoint = tempClickIndex + QPoint(1, 1);
            selectRect = QRect(tempClickIndex, tempLastPoint);

            topLeftGridPos = gridAt(selectRect.topLeft());
            bottomRightGridPos = gridAt(selectRect.bottomRight());
            currentPoint = topLeftGridPos;
            if (QPoint(-1, -1) == currentPoint)
                return;
        } else {
            // TODO: what?
            if (!d->showSelectRect) {
                return;
            }
            oldSelection = d->beforeMoveSelection;
            topLeftGridPos = gridAt(d->selectRect.topLeft());
            bottomRightGridPos = gridAt(d->selectRect.bottomRight());

            QItemSelection rectSelection;
            QMargins margins(10, 10, 10, 10);
            for (auto x = topLeftGridPos.x(); x <= bottomRightGridPos.x(); ++x) {
                for (auto y = topLeftGridPos.y(); y <= bottomRightGridPos.y(); ++y) {
                    auto localFile = GridManager::instance()->itemTop(m_screenNum, x, y);
                    //GridManager::instance()->itemId(m_screenNum, x, y);
                    if (localFile.isEmpty()) {
                        continue;
                    }
                    auto index = model()->index(DUrl(localFile));

                    //需求，桌面图标向内收缩10个像素为框选触发范围
                    auto tempVr = visualRect(index);
                    tempVr = tempVr.marginsRemoved(margins);
                    if (tempVr.intersects(selectRect)) {
                        QItemSelectionRange selectionRange(index);
                        if (!rectSelection.contains(index)) {
                            rectSelection.push_back(selectionRange);
                        }
                    }
                }
            }
            if (command != QItemSelectionModel::Deselect) {
                QItemSelection lastAllSelection;
                if (DFMGlobal::keyShiftIsPressed()) {
                    //shitf键增量
                    for (auto &sel : rectSelection) {
                        for (auto &index : sel.indexes())
                            if (!oldSelection.contains(index)) {
                                oldSelection.push_back(sel);
                            }
                    }
                    lastAllSelection.merge(oldSelection, QItemSelectionModel::Select);
                } else {
                    // Remove dump select
                    auto tempNewSelection = rectSelection;
                    auto tempOldSelection = oldSelection;
                    for (auto &sel : rectSelection) {
                        for (auto &index : sel.indexes())
                            if (oldSelection.contains(index)) {
                                //bug87509，产品要求按住ctrl鼠标框选效果与文管保持一直
                                tempNewSelection.removeOne(sel);
                                tempOldSelection.removeOne(sel);
                            }
                    }
                    // merge
                    lastAllSelection.merge(tempNewSelection, QItemSelectionModel::Select);
                    lastAllSelection.merge(tempOldSelection, QItemSelectionModel::Select);
                }

                QAbstractItemView::selectionModel()->select(lastAllSelection, command);
            } else {
                QAbstractItemView::selectionModel()->select(rectSelection, command);
            }
            return;
        }
    }

    if (DFMGlobal::keyShiftIsPressed()) {
        QModelIndexList needSelectIndex;
        QModelIndexList selectIndexs;
        QSize coordSize = GridManager::instance()->gridSize(m_screenNum);

        QItemSelection rectSelection;
        DUrl oldRet;
        DAbstractFileInfoPointer oldFp = model()->fileInfo(d->m_oldCursorIndex);
        if (!oldFp)
            return;
        oldRet = oldFp->fileUrl();
        QPair<int, QPoint> oldPosInfo;
        GridManager::instance()->find(oldRet.toString(), oldPosInfo);

        DUrl currentRet;
        DAbstractFileInfoPointer currentFp = model()->fileInfo(d->currentCursorIndex);
        if (currentFp) {
            currentRet = currentFp->fileUrl();
        }
        QPair<int, QPoint> currentPosInfo;
        GridManager::instance()->find(currentRet.toString(), currentPosInfo);

        topLeftGridPos = oldPosInfo.second;
        bottomRightGridPos = currentPosInfo.second;

        QPoint temp;
        if (topLeftGridPos.y() > bottomRightGridPos.y()
                || (topLeftGridPos.x() > bottomRightGridPos.x()
                && topLeftGridPos.y() == bottomRightGridPos.y())) {
            temp = topLeftGridPos;
            topLeftGridPos = bottomRightGridPos;
            bottomRightGridPos = temp;
        }

        int rowBegin = topLeftGridPos.y();
        int rowEnd = bottomRightGridPos.y();
        QPoint nextItem;
        for (; rowBegin <= rowEnd; ++ rowBegin) {
            nextItem.setX(0);
            nextItem.setY(rowBegin);
            while (nextItem != bottomRightGridPos) {
                if (nextItem.y() == topLeftGridPos.y() && nextItem.x() < topLeftGridPos.x()) {
                    nextItem = QPoint((nextItem.x() + 1), rowBegin);
                    continue;
                }

                auto nextUrl = GridManager::instance()->itemTop(m_screenNum, nextItem.x(), nextItem.y());
                auto nextIndex = model()->index(nextUrl);
                QItemSelectionRange selectionRange(nextIndex);
                rectSelection.append(selectionRange);
                nextItem = QPoint((nextItem.x() + 1), rowBegin);
                bool inLayoutX = 0 <= nextItem.x() && nextItem.x()  <= (coordSize.width() - 1);
                if (!inLayoutX) {
                    break;
                }
            }
            auto nextUrl = GridManager::instance()->itemTop(m_screenNum, nextItem.x(), nextItem.y());
            auto nextIndex = model()->index(nextUrl);
            QItemSelectionRange selectionRange(nextIndex);
            rectSelection.append(selectionRange);
        }

        QAbstractItemView::selectionModel()->clear();
        QAbstractItemView::selectionModel()->select(rectSelection, command);
    } else if (DFMGlobal::keyCtrlIsPressed()) {
        //just Ctrl
        auto localFile = GridManager::instance()->itemTop(m_screenNum, topLeftGridPos.x(), topLeftGridPos.y());
        //GridManager::instance()->itemId(m_screenNum, topLeftGridPos.x(), topLeftGridPos.y());
        if (localFile.isEmpty()) {
            return;
        }
        auto index = model()->index(DUrl(localFile));
        QItemSelectionRange selectionRange(index);
        if (!oldSelection.contains(index)) {
            oldSelection.push_back(selectionRange);
        } else {
            oldSelection.removeOne(selectionRange);
        }
        QAbstractItemView::selectionModel()->select(oldSelection, command);
        d->beginPos = topLeftGridPos;
    } else {
        //just click or mouseleft select
        auto localFile = GridManager::instance()->itemTop(m_screenNum, topLeftGridPos.x(), topLeftGridPos.y());
        //GridManager::instance()->itemId(m_screenNum, topLeftGridPos.x(), topLeftGridPos.y());
        if (localFile.isEmpty()) {
            return;
        }
        auto index = model()->index(DUrl(localFile));

        //end
        QItemSelectionRange selectionRange(index);
        QItemSelection rectSelection;
        rectSelection.push_back(selectionRange);
        QAbstractItemView::selectionModel()->select(rectSelection, command);
        d->beginPos = topLeftGridPos;
    }
}

void CanvasGridView::handleContextMenuAction(int action)
{
    bool changeSort  = false;

    switch (action) {
    case DisplaySettings: {
        QDBusInterface interface("com.deepin.dde.ControlCenter","/com/deepin/dde/ControlCenter","com.deepin.dde.ControlCenter");
        interface.asyncCall("ShowModule",QVariant::fromValue(QString("display")));
        break;
    }
    case CornerSettings:
        Desktop::instance()->showZoneSettings();
        break;
    case WallpaperSettings:
        Desktop::instance()->ShowWallpaperChooser(m_screenName);
        break;
    case MenuAction::SelectAll:
        this->selectAll();
        break;
//    case FileManagerProperty: {
//        QStringList localFiles;
//        localFiles << currentUrl().toLocalFile();
//        qDebug() << localFiles;
//        DFMSocketInterface::instance()->showProperty(localFiles);
//        break;
//    }
    case AutoMerge:
        emit autoMergeToggled();
        break;
    case AutoSort:
        emit autoAlignToggled();
        break;
    case IconSize0:
    case IconSize1:
    case IconSize2:
    case IconSize3:
    case IconSize4:
        setIconByLevel(action - IconSize0);
        break;

    case MenuAction::Name:
    case MenuAction::Size:
    case MenuAction::Type:
    case MenuAction::LastModifiedDate:
        changeSort = true;
        break;

    default:
        qInfo() << action;
    }

    if (changeSort) {
        model()->setEnabledSort(true);
        d->bReloadItem = true;
//        QMap<int, int> sortActions;
//        sortActions.insert(MenuAction::Name, DFileSystemModel::FileDisplayNameRole);
//        sortActions.insert(MenuAction::Size, DFileSystemModel::FileSizeRole);
//        sortActions.insert(MenuAction::Type, DFileSystemModel::FileMimeTypeRole);
//        sortActions.insert(MenuAction::LastModifiedDate, DFileSystemModel::FileLastModifiedRole);

        int sortRole = kSortActions.value(action);
        Qt::SortOrder sortOrder = model()->sortOrder();
        //若排序模式一样，则改变升降
        if (sortRole == model()->sortRole())
            sortOrder = sortOrder == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;

        model()->setSortRole(sortRole, sortOrder);
        model()->sort();
        Q_EMIT sortRoleChanged(sortRole, sortOrder);
    }
}

void CanvasGridView::showEmptyAreaMenu(const Qt::ItemFlags &/*indexFlags*/)
{
    const QModelIndex &index = rootIndex();
    const DAbstractFileInfoPointer &info = model()->fileInfo(index);
    QVector<MenuAction> actions;
    actions << MenuAction::NewFolder << MenuAction::NewDocument;
    if (!GridManager::instance()->autoMerge()) {
        actions << MenuAction::SortBy;
    }
    actions << MenuAction::Paste
            << MenuAction::SelectAll << MenuAction::OpenInTerminal;
    // 右键刷新
    {
        static const DGioSettings menuSwitch("com.deepin.dde.filemanager.contextmenu",
                                         "/com/deepin/dde/filemanager/contextmenu/");
        auto showRefreh = menuSwitch.value("Refresh");
        if (showRefreh.isValid() && showRefreh.toBool())
            actions << MenuAction::RefreshView;
    }

    actions << MenuAction::Property << MenuAction::Separator;
    if (actions.isEmpty()) {
        return;
    }

    const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();

    QSet<MenuAction> disableList = DFileMenuManager::getDisableActionList(model()->getUrlByIndex(index));

    if (model()->state() != DFileSystemModel::Idle) {
        disableList << MenuAction::SortBy;
    }
    if (!model()->rowCount()) {
        disableList << MenuAction::SelectAll;
    }

    DFileMenu *menu = DFileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    if (!menu) {
        return;
    }

    auto *pasteAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::Paste));

    DFileMenu iconSizeMenu;

    for (int i = itemDelegate()->minimumIconSizeLevel(); i <= itemDelegate()->maximumIconSizeLevel(); ++i) {
        auto iconSize = new QAction(&iconSizeMenu);
        iconSize->setText(itemDelegate()->iconSizeLevelDescription(i));
        iconSize->setData(IconSize + i);
        iconSize->setCheckable(true);
        iconSize->setChecked(i == itemDelegate()->iconSizeLevel());
        iconSizeMenu.addAction(iconSize);
    }
    //图标大小排列
    QAction iconSizeAction(menu);
    iconSizeAction.setText(tr("Icon size"));
    iconSizeAction.setData(IconSize);
    iconSizeAction.setMenu(&iconSizeMenu);
    menu->insertAction(pasteAction, &iconSizeAction);

#ifdef ENABLE_AUTOMERGE  //sp2需求调整，屏蔽自动整理
    //自动整理
    QAction menuAutoMerge(menu);
    menuAutoMerge.setText(tr("Auto merge"));
    menuAutoMerge.setData(AutoMerge);
    menuAutoMerge.setCheckable(true);
    menuAutoMerge.setChecked(GridManager::instance()->autoMerge());
    DGioSettings settings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
    if (settings.value("auto-merge").toBool()) {
        menu->insertAction(pasteAction, &menuAutoMerge);
    }
#endif

    //自动排序
    QAction autoSort(menu);
    autoSort.setText(tr("Auto arrange"));
    autoSort.setData(AutoSort);
    autoSort.setCheckable(true);
    autoSort.setChecked(GridManager::instance()->autoArrange());
    if (!GridManager::instance()->autoMerge())
        menu->insertAction(pasteAction, &autoSort);

    //DFileView中会修改排序项的选中属性，需重设状态。
    if (QAction *sortAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::SortBy))) {
        if (QMenu *roleMenu = sortAction->menu()) {
            for (QAction *action : roleMenu->actions()) {
                action->setCheckable(false);
                action->setChecked(false);
            }
        }
    }

    auto *propertyAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::Property));
    QList<QAction *> pluginActions = DFileMenuManager::loadEmptyAreaPluginMenu(menu, model()->rootUrl(), true);

    if (pluginActions.count() > 0) {
        QAction *separator = new QAction(menu);
        separator->setSeparator(true);
        menu->insertAction(pluginActions.at(0), separator);
    }

    //显示设置
    QAction display(menu);
    display.setText(tr("Display Settings"));
    display.setData(DisplaySettings);
    menu->addAction(&display);

    //热区设置，新版无热区设置需求
#ifndef DISABLE_ZONE
    QAction corner(menu);
    DGioSettings gsetting("com.deepin.dde.desktop", "/com/deepin/dde/desktop/");
    if (gsetting.keys().contains("enable-hotzone-settings") && gsetting.value("enable-hotzone-settings").toBool()) {
        corner.setText(tr("Corner Settings"));
        corner.setData(CornerSettings);
        menu->addAction(&corner);
    }
#endif

    //壁纸和屏保设置
    QAction wallpaper(menu);
#ifdef DISABLE_SCREENSAVER
    wallpaper.setText(tr("Set Wallpaper"));
#else
    if (ScreenSaverCtrlFunction::needShowScreensaver()) {
        wallpaper.setText(tr("Wallpaper and Screensaver"));
    } else {
        wallpaper.setText(tr("Set Wallpaper"));
    }
#endif
    wallpaper.setData(WallpaperSettings);
    menu->addAction(&wallpaper);

    menu->removeAction(propertyAction);
    menu->setEventData(model()->rootUrl(), selectedUrls(), winId(), this);

    //扩展菜单
    DFileMenuManager::extendCustomMenu(menu, false, currentUrl() , {}, {}, true);

    connect(menu, &DFileMenu::triggered, this, [ = ](QAction * action) {
        qDebug() << "trigger action" << action->data();
        if (!action->data().isValid()) {
            return;
        }
        handleContextMenuAction(action->data().toInt());
    });

    d->fileViewHelper->handleMenu(menu);

    if (DesktopInfo().waylandDectected()) {

        QPoint t_tmpPoint = QCursor::pos();
        QRect t_tmpRect;
        if (parentWidget())
            t_tmpRect = parentWidget()->geometry();
        else {
            auto screen = ScreenMrg->primaryScreen();
            if (screen)
                t_tmpRect = screen->geometry();
            else {
                qCritical() << "get primary geometry fail" << m_screenName << m_screenNum;
                menu->exec();
                menu->deleteLater();
                return;
            }
        }

        if (t_tmpPoint.x() + menu->sizeHint().width() > t_tmpRect.right())
            t_tmpPoint.setX(t_tmpPoint.x() - menu->sizeHint().width());

        if (t_tmpPoint.y() + menu->sizeHint().height() > t_tmpRect.bottom())
            t_tmpPoint.setY(t_tmpPoint.y() - menu->sizeHint().height());
//        menu->exec(t_tmpPoint);
        QEventLoop eventLoop;
        d->menuLoop = &eventLoop;
        connect(menu, &QMenu::aboutToHide, this, [ = ] {
            if (d->menuLoop)
                d->menuLoop->exit();
        });
        menu->popup(t_tmpPoint);
        menu->setGeometry(t_tmpPoint.x(), t_tmpPoint.y(), menu->sizeHint().width(), menu->sizeHint().height());
        eventLoop.exec();
        d->menuLoop = nullptr;
        menu->deleteLater();
        return;
    }
    //fix bug 33305 在用右键菜单复制大量文件时，在复制过程中，关闭窗口这时this释放了，在关闭拷贝menu的exec退出，menu的deleteLater崩溃
    QPointer<CanvasGridView> me = this;
    menu->exec();
    menu->deleteLater(me);
}

void CanvasGridView::showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags)
{
    if (!index.isValid()) {
        return;
    }

    const DUrlList list = selectedUrls();
    const DAbstractFileInfoPointer &info = model()->fileInfo(index);
    if (!info || info->isVirtualEntry()) {
        return;
    }

    QSet<MenuAction> disableList;
    QSet<MenuAction> unusedList;

    // blumia: when touching this part, do the same change in dfileview.cpp
    if (list.size() == 1) {
        if (!info->isReadable() && !info->isSymLink()) {
            disableList << MenuAction::Copy;
        }

        if (!info->isWritable() && !info->isFile() && !info->isSymLink()) {
            disableList << MenuAction::Delete;
        }

        if (!indexFlags.testFlag(Qt::ItemIsEditable)) {
            disableList << MenuAction::Rename;
        }
    }

    if (list.length() == 1) {
        unusedList << MenuAction::OpenInNewWindow
                   << MenuAction::OpenInNewTab
                   << MenuAction::SendToDesktop
                   << MenuAction::AddToBookMark;
    } else {
        unusedList << MenuAction::SendToDesktop;
    }

    /********************************************/
    //桌面的菜单action有些是通过filemanager lis管理的
    //里面的判断都是i基于真实路径，所以在自动整理的虚拟路径就不适用
    //故重新更改成真实路径传递，可能对action后续的操作是否有影响
    auto curUrl = info->fileUrl();
    DUrlList realList;
    DFileMenu *menu = nullptr;

    //是否显示自定义菜单
    bool customMenu = true;

    auto dirUrl = currentUrl();
    if (curUrl.scheme() == DFMMD_SCHEME) {
        curUrl = MergedDesktopController::convertToRealPath(curUrl);
        dirUrl = MergedDesktopController::convertToRealPath(dirUrl);

        //判断计算机 回收站 主目录
        const DUrl &computerDesktopFile = DesktopFileInfo::computerDesktopFileUrl();
        const DUrl &trashDesktopFile = DesktopFileInfo::trashDesktopFileUrl();
        const DUrl &homeDesktopFile = DesktopFileInfo::homeDesktopFileUrl();

        for (const DUrl &url : list) {
            auto cUrl = MergedDesktopController::convertToRealPath(url);
            realList.append(cUrl);
            //多选文件中包含以下文件时 则不展示扩展菜单项
            if (customMenu && (cUrl == computerDesktopFile
                    || cUrl == trashDesktopFile
                    || cUrl == homeDesktopFile)) {
                customMenu = false;
            }
        }
        menu = DFileMenuManager::createNormalMenu(curUrl, realList, disableList, unusedList, static_cast<int>(winId()), true);
    } else {
        //realList给后面的扩展菜单使用
        realList = list;

        //判断计算机 回收站 主目录
        const DUrl &computerDesktopFile = DesktopFileInfo::computerDesktopFileUrl();
        const DUrl &trashDesktopFile = DesktopFileInfo::trashDesktopFileUrl();
        const DUrl &homeDesktopFile = DesktopFileInfo::homeDesktopFileUrl();

        //多选文件中包含以下文件时 则不展示扩展菜单项
        for (const DUrl &cUrl : realList) {
            if (cUrl == computerDesktopFile
                    || cUrl == trashDesktopFile
                    || cUrl == homeDesktopFile) {
                customMenu = false;
                break;
            }
        }
        menu = DFileMenuManager::createNormalMenu(info->fileUrl(), list, disableList, unusedList, static_cast<int>(winId()), true);
    }

    if (!menu) {
        return;
    }

    //totally use dde file manager libs for menu actions
    //    auto *menu = DFileMenuManager::createNormalMenu(info->fileUrl(), list, disableList, unusedList, winId(), true);
    /********************************************/

    QSet<MenuAction> ignoreActions;
    ignoreActions  << MenuAction::Open;
    menu->setIgnoreMenuActions(ignoreActions);
    menu->setAccessibleInfo(AC_FILE_MENU_DESKTOP);

    auto *propertyAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::Property));
    if (propertyAction) {
        menu->removeAction(propertyAction);
    }

    QAction *property = new QAction(menu);
    property->setText(tr("Properties"));
    property->setData(FileManagerProperty);
    menu->addAction(property);

    menu->setEventData(model()->rootUrl(), selectedUrls(), winId(), this, index);

    //扩展菜单
    if (customMenu)
        DFileMenuManager::extendCustomMenu(menu, true, dirUrl, curUrl, realList, true);

    //断开连接，桌面优先处理
    //为了保证自动整理下右键菜单标记信息（需要虚拟路径）与右键取消共享文件夹（需要真是路径）无有冲突，
    //故在connect(menu, &DFileMenu::triggered, this, [ = ](QAction * action) {}相应之后再相应actionTriggered的连接
    //所以这里先断开
    disconnect(menu, &DFileMenu::triggered, fileMenuManger, &DFileMenuManager::actionTriggered);
    connect(menu, &DFileMenu::triggered, this, [ = ](QAction * action) {
        if (!action->data().isValid()) {
            return;
        }
        switch (action->data().toInt()) {
        case MenuAction::Open: {
//            for (auto &url : list) {
//                openUrl(url);
//            }
            qDebug() << "open urls" << list;
            openUrls(list);
        }
        break;
        case FileManagerProperty: {
            //解决自动整理修改文件属性不生效问题
            QList<DUrl> selectedUrlLst{};
            if (GridManager::instance()->autoMerge()) {
                QList<DUrl> t_selectedUrls{};
                t_selectedUrls = this->autoMergeSelectedUrls();
                for (auto temp : t_selectedUrls) {
                    if (DFMMD_SCHEME == temp.scheme())
                        selectedUrlLst.append(MergedDesktopController::convertToRealPath(temp));
                }
            } else {
                selectedUrlLst = this->selectedUrls();
            }

            DFMGlobal::showPropertyDialog(this, selectedUrlLst);
            //DFMGlobal::showPropertyDialog(this, this->selectedUrls());
            break;
        }
        case MenuAction::Rename: {
            if (list.size() == 1) {
                QAbstractItemView::edit(index); //###: select one file on desktop.

            } else { //###: select more than one files.
                QList<DUrl> selectedUrls{};
                if (GridManager::instance()->autoMerge()) {
                    selectedUrls = this->autoMergeSelectedUrls();
                } else {
                    selectedUrls = this->selectedUrls();
                }
                DFMGlobal::showMultiFilesRenameDialog(selectedUrls);
            }
            break;
        }
        //为了保证自动整理下右键菜单标记信息（需要虚拟路径）与右键取消共享文件夹（需要真是路径）无有冲突
        //将取消共享文件夹的选中路径提到里面
        case MenuAction::Share: {
            if (info->fileUrl().scheme() == DFMMD_SCHEME) {
                menu->setEventData(curUrl, realList, winId(), this, index);
            } else {
                menu->setEventData(model()->rootUrl(), selectedUrls(), winId(), this, index);
            }
            break;
        }

        default:
            break;
        }
    }, Qt::QueuedConnection);

    //使用队列连接，防止menu的exec无法退出
    //为了保证自动整理下右键菜单标记信息（需要虚拟路径）与右键取消共享文件夹（需要真是路径）无有冲突，
    //故在connect(menu, &DFileMenu::triggered, this, [ = ](QAction * action) {}相应之后再相应actionTriggered的连接
    //这里再重新连接保证在triggered之后相应
    connect(menu, &DFileMenu::triggered, fileMenuManger, &DFileMenuManager::actionTriggered, Qt::QueuedConnection);
    d->fileViewHelper->handleMenu(menu);

    if (DesktopInfo().waylandDectected()) {

        QPoint t_tmpPoint = QCursor::pos();
        QRect t_tmpRect;
        if (parentWidget())
            t_tmpRect = parentWidget()->geometry();
        else {
            auto screen = ScreenMrg->primaryScreen();
            if (screen)
                t_tmpRect = screen->geometry();
            else {
                qCritical() << "get primary geometry fail" << m_screenName << m_screenNum;
                menu->exec();
                menu->deleteLater();
                return;
            }
        }

        if (t_tmpPoint.x() + menu->sizeHint().width() > t_tmpRect.right())
            t_tmpPoint.setX(t_tmpPoint.x() - menu->sizeHint().width());

        if (t_tmpPoint.y() + menu->sizeHint().height() > t_tmpRect.bottom())
            t_tmpPoint.setY(t_tmpPoint.y() - menu->sizeHint().height());
//        menu->exec(t_tmpPoint);
        QEventLoop eventLoop;
        d->menuLoop = &eventLoop;
        connect(menu, &QMenu::aboutToHide, this, [ = ] {
            if (d->menuLoop)
                d->menuLoop->exit();
        });
        menu->popup(t_tmpPoint);
        menu->setGeometry(t_tmpPoint.x(), t_tmpPoint.y(), menu->sizeHint().width(), menu->sizeHint().height());
        eventLoop.exec();
        d->menuLoop = nullptr;
        menu->deleteLater();
        return;
    }
    //fix bug 33305 在用右键菜单复制大量文件时，在复制过程中，关闭窗口这时this释放了，在关闭拷贝menu的exec退出，menu的deleteLater崩溃
    QPointer<CanvasGridView> me = this;
    menu->exec();
    menu->deleteLater(me);
    //断连，防止menu没有释放再次触发信号
    disconnect(menu, nullptr, this, nullptr);
}

void CanvasGridView::startDrag(Qt::DropActions supportedActions)
{
    // 在定时器期间收到鼠标move事件低于配置时间，不触发drag
    if (d->touchTimer.isActive())
        return;
    //drop后该变量依然为true，先将其置为false
    d->mousePressed = false;

    //drag优化，只抓起本屏幕上的图标
    DUrlList validSel;
    QModelIndexList validIndexes;
    viewSelectedUrls(validSel, validIndexes);
    select(validSel);
    itemDelegate()->hideAllIIndexWidget();

    //数量大于1, 聚合图标
    if (validIndexes.count() > 1) {
        QMimeData *data = model()->mimeData(validIndexes);
        if (!data)
            return;

        QPixmap pixmap = this->renderToPixmap(validIndexes);
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
