/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dfileviewhelper.h"
#include "dfmglobal.h"
#include "dfmstyleditemdelegate.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "views/windowmanager.h"
#include "dabstractfileinfo.h"
#include "dfilesystemmodel.h"
#include "views/fileitem.h"
#include "views/dtagactionwidget.h"
#include "singleton.h"
#include "dfileservices.h"
#include "dfmgenericfactory.h"
#include "dialogs/filepreviewdialog.h"
#include "controllers/appcontroller.h"
#include "controllers/mergeddesktopcontroller.h"
#include "dfmeventdispatcher.h"
#include "dfilemenu.h"
#include "tag/tagmanager.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "dstorageinfo.h"
#include "controllers/vaultcontroller.h"
#include "dfmstandardpaths.h"
#include "deviceinfo/udisklistener.h"

#include <sys/stat.h>

#include <QTimer>
#include <QAction>
#define protected public
#include <QAbstractItemView>
#undef protected
#include <QLineEdit>
#include <QTextEdit>
#include <QMenu>
#include <QWidgetAction>

#include <DApplication>

DWIDGET_USE_NAMESPACE

class MenuActionEventHandler : public DFMAbstractEventHandler
{
public:
    explicit MenuActionEventHandler(DFileViewHelper *helper)
        : DFMAbstractEventHandler(false)
        , viewHelper(helper)
    {
    }

    bool fmEventFilter(const QSharedPointer<DFMEvent> &event, DFMAbstractEventHandler *target, QVariant *resultData) override
    {
        Q_UNUSED(target)
        Q_UNUSED(resultData)

        if (event->type() != DFMEvent::MenuAction) {
            return false;
        }

        if (event->windowId() != viewHelper->windowId()) {
            return false;
        }

        const DFMMenuActionEvent &menu_event = dfmevent_cast<DFMMenuActionEvent>(*event.data());
        if (menu_event.action() == DFMGlobal::MenuAction::RefreshView) {
            viewHelper->viewFlicker();
            return true;
        }
        if (menu_event.action() != DFMGlobal::TagInfo)
            return false;

        if (menu_event.selectedUrls().isEmpty())
            return false;

        //在多标签的情况下只判断currentUrl无法区分同一个目录打开的多个标签
        //修改为判断事件发送者和viewhelper的持有者是否为同一个view对象
        if (menu_event.sender() && viewHelper->parent()) {
            if (menu_event.sender().data() != viewHelper->parent())
                return false;
        }

        //优先取当前点击的index作为tagedit的基点
        const QModelIndex &index = menu_event.clickedIndex() != QModelIndex() ? menu_event.clickedIndex() : viewHelper->model()->index(menu_event.selectedUrls().first());
        const QRect &rect = viewHelper->parent()->visualRect(index);
        QStyleOptionViewItem option = viewHelper->parent()->viewOptions();

        option.rect = rect;

        const QList<QRect> &geometry_list = viewHelper->itemDelegate()->paintGeomertys(option, index);
        const QRect &icon_rect = geometry_list.first();
        bool isCanvas = viewHelper->property("isCanvasViewHelper").toBool();
        int iconTopOffset = isCanvas ? 0 : (option.rect.height() - icon_rect.height()) / 3.0;

        const QPoint &edit_pos = QPoint(icon_rect.x() + icon_rect.width() / 2, icon_rect.bottom() + iconTopOffset);
        /****************************************************************************************************************************/
        //在靠近边框底部不够显示编辑框时，编辑框的箭头出现在底部
        //appController->showTagEdit(viewHelper->parent()->viewport()->mapToGlobal(edit_pos), menu_event.selectedUrls());
        const QRect &parentRect = viewHelper->parent()->geometry();
        appController->showTagEdit(parentRect, viewHelper->parent()->viewport()->mapToGlobal(edit_pos), menu_event.selectedUrls());
        /****************************************************************************************************************************/
        return true;
    }

private:
    DFileViewHelper *viewHelper = nullptr;
};

class DFileViewHelperPrivate
{
public:
    explicit DFileViewHelperPrivate(DFileViewHelper *qq)
        : menuEventHandler(new MenuActionEventHandler(qq))
        , q_ptr(qq)
    {
        DFMEventDispatcher::instance()->installEventFilter(menuEventHandler);
    }

    ~DFileViewHelperPrivate()
    {
        delete menuEventHandler;
        menuEventHandler = nullptr;
    }

    DFileViewHelperPrivate(DFileViewHelperPrivate &) = delete;
    DFileViewHelperPrivate &operator=(DFileViewHelperPrivate &) = delete;

    void init();
    QModelIndex findIndex(const QByteArray &keys, bool matchStart,
                          int current, bool reverse_order, bool excludeCurrent) const;

    void _q_edit(const DFMUrlBaseEvent &event);
    void _q_selectAndRename(const DFMUrlBaseEvent &event);

    QByteArray keyboardSearchKeys;
    QTimer keyboardSearchTimer;
    MenuActionEventHandler *menuEventHandler;

    static QObjectList pluginObjectList;
    static QList<QIcon> getAdditionalIconByPlugins(const DAbstractFileInfoPointer &fileInfo);

    DFileViewHelper *q_ptr;

    Q_DECLARE_PUBLIC(DFileViewHelper)
};

QObjectList DFileViewHelperPrivate::pluginObjectList;

void DFileViewHelperPrivate::init()
{
    Q_Q(DFileViewHelper);

    keyboardSearchTimer.setSingleShot(true);
    keyboardSearchTimer.setInterval(200);

    // init connects
    QObject::connect(&keyboardSearchTimer, &QTimer::timeout,
    q, [this] {
        keyboardSearchKeys.clear();
    });
    QObject::connect(qApp, &DApplication::iconThemeChanged, q->parent(), static_cast<void (QWidget::*)()>(&QWidget::update));
    QObject::connect(DFMGlobal::instance(), &DFMGlobal::clipboardDataChanged, q, [q] {
        if (q->itemDelegate())
        {
            for (const QModelIndex &index : q->itemDelegate()->hasWidgetIndexs()) {
                QWidget *item = q->indexWidget(index);

                if (item) {
                    item->setProperty("opacity", q->isTransparent(index) ? 0.3 : 1);
                }
            }
        }

        q->parent()->update();
    });

    // init actions
    QAction *copy_action = new QAction(q->parent());

    copy_action->setAutoRepeat(false);
    copy_action->setShortcut(QKeySequence::Copy);

    QObject::connect(copy_action, &QAction::triggered,
    q, [q] {
        // fix bug 62872
        // 与右键菜单保持一致，如果选中项只有一个且选中项不可读，则复制操作无效
        if (q->selectedUrls().size() == 1) {
            const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(nullptr, q->selectedUrls().first());
            if (!fileInfo || !fileInfo->isReadable())
                return;
        }
        DUrlList selectedUrls = q->selectedUrls();
        qInfo() << " ctrl C writeFilesToClipboard and selectedUrls = " << selectedUrls
                << " currentUrl = " << q->currentUrl();
        fileService->writeFilesToClipboard(q, DFMGlobal::CopyAction, selectedUrls);
    });

    QAction *cut_action = new QAction(q->parent());

    cut_action->setAutoRepeat(false);
    cut_action->setShortcut(QKeySequence::Cut);

    QObject::connect(cut_action, &QAction::triggered,
    q, [q] {
        // 只支持回收站根目录下的文件执行剪切
        const DAbstractFileInfoPointer &rootInfo = fileService->createFileInfo(q, q->currentUrl());
        if (!rootInfo || !rootInfo->isWritable()) {
            qInfo() << "Read only folders do not support Ctrl + X operations！folders = " << q->currentUrl();
            return;
        }

        if (!q->selectedUrls().isEmpty()) {
            DUrl url = q->selectedUrls().first();
            if (url.isTrashFile() && url.parentUrl() != DUrl::fromTrashFile("/"))
                return;
        }
        DUrlList selectedUrls = q->selectedUrls();
        qInfo() << " ctrl X writeFilesToClipboard and selectedUrls = " << selectedUrls
                << " currentUrl = " << q->currentUrl();
        fileService->writeFilesToClipboard(q, DFMGlobal::CutAction, selectedUrls);
    });

    QAction *paste_action = new QAction(q->parent());

    paste_action->setShortcut(QKeySequence::Paste);

    QObject::connect(paste_action, &QAction::triggered,
    q, [q] {
        qInfo() << " ctrl V pasteFileByClipboard and currentUrl = " << q->currentUrl();
        fileService->pasteFileByClipboard(q->parent(), q->currentUrl());
    });

    QAction *revocation_action = new QAction(q->parent());

    revocation_action->setShortcut(QKeySequence::Undo);

    QObject::connect(revocation_action, &QAction::triggered,
    q, [q] {
        qInfo() << " ctrl Z recovert operation !";
        DFMEventDispatcher::instance()->processEvent<DFMRevocationEvent>(q);
    });

    q->parent()->addAction(copy_action);
    q->parent()->addAction(cut_action);
    q->parent()->addAction(paste_action);
    q->parent()->addAction(revocation_action);

    q->connect(fileSignalManager, SIGNAL(requestRename(DFMUrlBaseEvent)), q, SLOT(_q_edit(DFMUrlBaseEvent)));
    q->connect(fileSignalManager, SIGNAL(requestSelectRenameFile(DFMUrlBaseEvent)), q, SLOT(_q_selectAndRename(DFMUrlBaseEvent)));
    // call later
    QTimer::singleShot(0, q, [q] {
        q->connect(fileSignalManager, SIGNAL(trashStateChanged()), q->model(), SLOT(update()));
    });

    // init plugin objects
    static bool initialized = false;

    if (!initialized) {
        initialized = true;

        pluginObjectList = DFMGenericFactory::createAll(QStringLiteral("fileinfo/additionalIcon"));
    }
}

QModelIndex DFileViewHelperPrivate::findIndex(const QByteArray &keys, bool matchStart,
                                              int current, bool reverse_order, bool excludeCurrent) const
{
    Q_Q(const DFileViewHelper);

    int row_count = q->parent()->model()->rowCount(q->parent()->rootIndex());

    for (int i = excludeCurrent ? 1 : 0; i < row_count; ++i) {
        int row = reverse_order ? row_count + current - i : current + i;

        row = row % row_count;

        if (excludeCurrent && row == current) {
            continue;
        }

        const QModelIndex &index = q->parent()->model()->index(row, 0, q->parent()->rootIndex());
        const QString &pinyin_name = q->parent()->model()->data(index, DFileSystemModel::FilePinyinName).toString();

        if (matchStart ? pinyin_name.startsWith(keys, Qt::CaseInsensitive)
                : pinyin_name.contains(keys, Qt::CaseInsensitive)) {
            return index;
        }
    }

    return QModelIndex();
}

void DFileViewHelperPrivate::_q_edit(const DFMUrlBaseEvent &event)
{
    Q_Q(DFileViewHelper);

    if (event.windowId() != q->windowId()) {
        return;
    }

    DUrl fileUrl = event.url();

    if (!fileUrl.isValid()) {
        return;
    }

    const QModelIndex &index = q->model()->index(fileUrl);

    if (q->parent()->isVisible()) {
        q->parent()->edit(index, QAbstractItemView::EditKeyPressed, 0);
    }
}

void DFileViewHelperPrivate::_q_selectAndRename(const DFMUrlBaseEvent &event)
{
    Q_Q(DFileViewHelper);

    if (event.windowId() != q->windowId() || !q->parent()->isVisible()) {
        return;
    }

    q->select(DUrlList() << event.url());
    _q_edit(event);
}

QList<QIcon> DFileViewHelperPrivate::getAdditionalIconByPlugins(const DAbstractFileInfoPointer &fileInfo)
{
    QList<QIcon> list;

    for (QObject *object : pluginObjectList) {
        QList<QIcon> plugin_list;

        bool ok = object->metaObject()->invokeMethod(object, "fileAdditionalIcon",
                                                     Q_RETURN_ARG(QList<QIcon>, plugin_list),
                                                     Q_ARG(const DAbstractFileInfoPointer &, fileInfo));

        if (ok) {
            list << plugin_list;
        } else {
            qWarning() << "call the fileAdditionalIcon slot failed";
        }
    }

    return list;
}

DFileViewHelper::DFileViewHelper(QAbstractItemView *parent)
    : QObject(parent)
    , d_ptr(new DFileViewHelperPrivate(this))
{
    Q_ASSERT(parent);

    d_func()->init();
}

DFileViewHelper::~DFileViewHelper()
{

}

QAbstractItemView *DFileViewHelper::parent() const
{
    return qobject_cast<QAbstractItemView *>(QObject::parent());
}

/*!
 * \brief Returns the window id of the top-level window in the parent widget.
 * \return
 */
quint64 DFileViewHelper::windowId() const
{
    return WindowManager::getWindowId(parent());
}

/*!
 * \brief Return true if index is transparent; otherwise return false.
 * \param index
 * \return
 */
bool DFileViewHelper::isTransparent(const QModelIndex &index) const
{
    const DAbstractFileInfoPointer &fileInfo = this->fileInfo(index);

    if (!fileInfo) {
        return false;
    }
    DUrl fileUrl = fileInfo->fileUrl();

    if (fileInfo->fileUrl().isSearchFile()) {
        fileUrl = fileInfo->fileUrl().searchedFileUrl();
    }

    //如果在tag的view下，获取的url是tag的，需要转为localfile的url
    if (fileInfo->fileUrl().isTaggedFile()) {
        fileUrl = DUrl::fromLocalFile(fileInfo->fileUrl().taggedLocalFilePath());
    }

    //staging files are transparent
    if (currentUrl().scheme() == BURN_SCHEME && fileUrl.scheme() == BURN_SCHEME && !fileUrl.burnIsOnDisc()) {
        return true;
    }

    // 解决在保险箱中执行剪切时，图标不灰显的问题
    if (fileUrl.scheme() == DFMVAULT_SCHEME) {
        fileUrl = VaultController::vaultToLocalUrl(fileUrl);
    }

    // 将回收站路径转化成真实路径，解决在回收站中执行剪切时，图标不灰显的问题
    bool isTrashFile = false;
    if (fileUrl.scheme() == TRASH_SCHEME) {
        isTrashFile = true;
        const QString &path = fileUrl.path();
        fileUrl = DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath) + path);
    }

    //为了防止自动整理下剪切与分类名相同的文件夹,如果是分类就不转真实路径了
    auto isVPath = MergedDesktopController::isVirtualEntryPaths(fileUrl);
    if (fileUrl.scheme() == DFMMD_SCHEME && !isVPath)
        fileUrl = MergedDesktopController::convertToRealPath(fileUrl);

    //判断该文件是否被剪切
    if (DFMGlobal::instance()->clipboardAction() == DFMGlobal::CutAction && (isTrashFile || fileInfo->canRename())) {
        if (DFMGlobal::instance()->clipboardFileUrlList().contains(fileUrl))
            return true;

        //链接文件只判断url，不判断inode，因为链接文件的inode与源文件的inode是一致的
        if (!fileInfo->isSymLink()) {
            if (DFMGlobal::instance()->clipboardFileInodeList().contains(fileInfo->inode()))
                return true;
        }
    }

    return false;
}

/*!
 * \brief Return true if index is selected; otherwise return false.
 * \param index
 * \return
 */
bool DFileViewHelper::isSelected(const QModelIndex &index) const
{
    return parent()->selectionModel()->isSelected(index);
}

/*!
 * \brief Return true if file view drag moveing and mouse hover on index; otherwise return false.
 * \param index
 * \return
 */
bool DFileViewHelper::isDropTarget(const QModelIndex &index) const
{
    Q_UNUSED(index)

    return false;
}

/*!
 * \brief Return the view of selected index count.
 * \return
 */
int DFileViewHelper::selectedIndexsCount() const
{
    return parent()->selectionModel()->selectedIndexes().count();
}

/*!
 * \brief Return the view row count.
 * \return
 */
int DFileViewHelper::rowCount() const
{
    return parent()->model()->rowCount(parent()->rootIndex());
}

/*!
 * \brief Return the index row index.
 * \param index
 * \return
 */
int DFileViewHelper::indexOfRow(const QModelIndex &index) const
{
    Q_UNUSED(index)

    return -1;
}

/*!
 * \brief Returns corner icon by file info(file permission...).
 * \param index
 * \return
 */
QList<QIcon> DFileViewHelper::additionalIcon(const QModelIndex &index) const
{
    QList<QIcon> list;
    const DAbstractFileInfoPointer &fileInfo = this->fileInfo(index);

    if (!fileInfo || !fileInfo->exists()) {
        return list;
    }

    list << fileInfo->additionalIcon();
    list << DFileViewHelperPrivate::getAdditionalIconByPlugins(fileInfo);

    return list;
}

/*!
 * \brief Returns the complete base name of the file without the path.
          The complete base name consists of all characters in the file up to (but not including) the file suffix.
 * \param index
 * \return
 */
QString DFileViewHelper::baseName(const QModelIndex &index) const
{
    const DAbstractFileInfoPointer &fileInfo = this->fileInfo(index);

    if (!fileInfo) {
        return QString();
    }

    return fileInfo->baseName();
}

/*!
 * \brief Return the view role list by column
 * \return
 */
QList<int> DFileViewHelper::columnRoleList() const
{
    return QList<int>();
}

/*!
 * \brief Return column width by column index
 * \param columnIndex
 * \return
 */
int DFileViewHelper::columnWidth(int columnIndex) const
{
    Q_UNUSED(columnIndex)

    return -1;
}

DUrl DFileViewHelper::currentUrl() const
{
    const DAbstractFileInfoPointer &fileInfo = this->fileInfo(parent()->rootIndex());

    if (!fileInfo) {
        return DUrl();
    }

    return fileInfo->fileUrl();
}

void DFileViewHelper::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    if (isSelected(index) && index.flags().testFlag(Qt::ItemFlag::ItemIsSelectable)) {
        option->state |= QStyle::State_Selected;
    } else {
        option->state &= QStyle::StateFlag(~QStyle::State_Selected);
    }

    QPalette appPalette = QGuiApplication::palette();

    auto setcolor1 = [](QPalette & p1, QPalette & p2, QPalette::ColorRole role) {
        p1.setColor(role, p2.color(role));
    };

    auto setcolor2 = [](QPalette & p1, QPalette & p2, QPalette::ColorGroup group, QPalette::ColorRole role) {
        p1.setColor(group, role, p2.color(group, role));
    };

    setcolor1(option->palette, appPalette, QPalette::Text);
    setcolor2(option->palette, appPalette, QPalette::Disabled, QPalette::Text);

    if ((option->state & QStyle::State_Selected) && option->showDecorationSelected) {
        setcolor2(option->palette, appPalette, QPalette::Inactive, QPalette::Text);
    } else {
        setcolor2(option->palette, appPalette, QPalette::Inactive, QPalette::Text);
    }

    option->palette.setColor(QPalette::BrightText, Qt::white);

    setcolor1(option->palette, appPalette, QPalette::Shadow);

    if ((option->state & QStyle::State_HasFocus) && option->showDecorationSelected && selectedIndexsCount() > 1) {
        setcolor2(option->palette, appPalette, QPalette::Current, QPalette::Background);
    } else {
        setcolor2(option->palette, appPalette, QPalette::Normal, QPalette::Background);
    }

    bool transp = isTransparent(index);
    option->backgroundBrush = appPalette.brush(transp ? QPalette::Inactive : QPalette::Current, QPalette::Background);

    option->textElideMode = Qt::ElideLeft;
}

void DFileViewHelper::handleMenu(QMenu *menu)
{
    DFileMenu *file_menu = qobject_cast<DFileMenu *>(menu);

    if (Q_UNLIKELY(!file_menu))
        return;


    QAction *tag_action = file_menu->actionAt("Add color tags");

    if (!tag_action)
        return;

    QWidgetAction *widget_action = qobject_cast<QWidgetAction *>(tag_action);

    if (Q_UNLIKELY(!widget_action))
        return;

    DTagActionWidget *tag_widget = qobject_cast<DTagActionWidget *>(widget_action->defaultWidget());

    if (Q_UNLIKELY(!tag_widget))
        return;

    const QStringList &tag_names = DFileService::instance()->getTagsThroughFiles(parent(), file_menu->selectedUrls());
    QList<QColor> colors;

    for (const QString &tag : tag_names) {
        const QString &color_display_name = TagManager::instance()->getColorByDisplayName(tag);

        if (color_display_name.isEmpty())
            continue;

        const QColor &color = TagManager::instance()->getColorByColorName(color_display_name);

        if (Q_LIKELY(color.isValid()))
            colors << color;
    }

    tag_widget->setCheckedColorList(colors);

    connect(tag_widget, &DTagActionWidget::hoverColorChanged, menu, [tag_widget](const QColor & color) {
        if (color.isValid()) {
            const QString &tag_name = TagManager::instance()->getTagNameThroughColor(color);

            tag_widget->setToolTipText(tr("Add tag \"%1\"").arg(tag_name));
        } else {
            tag_widget->clearToolTipText();
        }
    });
}

/*!
 * \brief Sets the given widget on the item at the given index, passing the ownership of the widget to the view viewport.
 * If index is invalid (e.g., if you pass the root index), this function will do nothing.
 * \param index
 * \param widget
 * \sa indexWidget()
 */
void DFileViewHelper::setIndexWidget(const QModelIndex &index, QWidget *widget)
{
    parent()->setIndexWidget(index, widget);
}

/*!
 * \brief Returns the widget for the item at the given index.
 * \param index
 * \return
 * \sa setIndexWidget()
 */
QWidget *DFileViewHelper::indexWidget(const QModelIndex &index) const
{
    return parent()->indexWidget(index);
}

/*!
 * \brief Updates the geometry of the child widgets of the view.
 */
void DFileViewHelper::updateGeometries()
{
    parent()->updateGeometries();
}

/*!
 * \brief Returns the margins around the file view. By default all the margins are zero.
 * \return
 */
QMargins DFileViewHelper::fileViewViewportMargins() const
{
    return parent()->viewportMargins();
}

/*!
 * \brief DFileViewHelper::keyboardSearch
 * \param key
 */
void DFileViewHelper::keyboardSearch(char key)
{
    Q_D(DFileViewHelper);
    QByteArray indexChar;
    d->keyboardSearchKeys.append(key);
    bool reverse_order = qApp->keyboardModifiers() == Qt::ShiftModifier;
    const QModelIndex &current_index = parent()->currentIndex();

    QModelIndex index = d->findIndex(d->keyboardSearchKeys, true, current_index.row(), reverse_order, !d->keyboardSearchTimer.isActive());
//    if (!index.isValid()) {
//        // 使用 QString::contains 模式再次匹配
//        index = d->findIndex(d->keyboardSearchKeys, false, current_index.row(), reverse_order, !d->keyboardSearchTimer.isActive());
//    }

    if (index.isValid()) {
        parent()->setCurrentIndex(index);
        parent()->scrollTo(index, reverse_order ? QAbstractItemView::PositionAtBottom : QAbstractItemView::PositionAtTop);
    }
    // 开始计时，超过此时间后的搜索将清空之前输入的关键字
    d->keyboardSearchTimer.start();
}

/*!
 * \brief Return true if the position is empty area; otherwise return false.
 * \param pos
 * \return
 */
bool DFileViewHelper::isEmptyArea(const QPoint &pos) const
{
    const QModelIndex &index = parent()->indexAt(pos);

    if (index.isValid() && isSelected(index)) {
        return false;
    } else {
        const QRect &rect = parent()->visualRect(index);

        if (!rect.contains(pos)) {
            return true;
        }

        QStyleOptionViewItem option = parent()->viewOptions();

        option.rect = rect;

        const QList<QRect> &geometry_list = itemDelegate()->paintGeomertys(option, index);
        auto ret = std::any_of(geometry_list.begin(), geometry_list.end(), [pos](const QRect & rect) {
            return rect.contains(pos);
        });
        if (ret)
            return false;
    }

    return index.isValid();//true;
}

void DFileViewHelper::preproccessDropEvent(QDropEvent *event) const
{
    bool sameUser = DFMGlobal::isMimeDatafromCurrentUser(event->mimeData());
    if (event->source() == parent() && !DFMGlobal::keyCtrlIsPressed()) {
        event->setDropAction(sameUser ? Qt::MoveAction : Qt::IgnoreAction);
    } else {
        DAbstractFileInfoPointer info = model()->fileInfo(parent()->indexAt(event->pos()));

        if (!info)
            info = model()->fileInfo(parent()->rootIndex());

        if (!info) {
            return;
        }

        QList<QUrl> urls = event->mimeData()->urls();

        if (urls.empty())
            return;

        Qt::DropAction default_action = Qt::CopyAction;
        const DUrl from = DUrl(urls.first());
        DUrl to = info->fileUrl();

        //fix bug#23703勾选自动整理，拖拽其他目录文件到桌面做得是复制操作
        //因为自动整理的路径被DStorageInfo::inSameDevice判断为false，这里做转化
        if (to.scheme() == DFMMD_SCHEME) {
            to = DUrl(info->absoluteFilePath());
            to.setScheme(FILE_SCHEME);
        }
        //end

        if (qApp->keyboardModifiers() == Qt::AltModifier) {
            default_action = Qt::MoveAction;
        } else if (!DFMGlobal::keyCtrlIsPressed()) {
            // 如果文件和目标路径在同一个分区下，默认为移动文件，否则默认为复制文件
            if (DStorageInfo::inSameDevice(from, to)) {
                default_action = Qt::MoveAction;
            }
        }

        //任意来源为回收站的drop操作均为move(统一回收站拖拽标准)
        bool isFromTrash = from.url().contains(".local/share/Trash/");
        bool isToTrash = to.isTrashFile();
        if (isFromTrash || isToTrash) {
            if (!isFromTrash || !isToTrash) {
                default_action = Qt::MoveAction;
            } else {
                return event->setDropAction(Qt::IgnoreAction);
            }
        }

        if (event->possibleActions().testFlag(default_action)) {
            event->setDropAction((default_action == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : default_action);
        }

        if (!info->supportedDropActions().testFlag(event->dropAction())) {
            QList<Qt::DropAction> actions;

            actions.reserve(3);
            actions << Qt::CopyAction << Qt::MoveAction << Qt::LinkAction;

            for (Qt::DropAction action : actions) {

                if (event->possibleActions().testFlag(action) && info->supportedDropActions().testFlag(action)) {
                    event->setDropAction((action == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : action);
                    break;
                }
            }
        }

        // 保险箱时，修改DropAction为Qt::CopyAction
        if ((VaultController::isVaultFile(info->fileUrl().toString())
             || VaultController::isVaultFile(urls[0].toString()))
            && !isToTrash) {
            event->setDropAction(Qt::CopyAction);
        }

        // 最近使用目录下的文件，只有拖拽到回收站为剪切，其他都为拷贝
        if (from.isRecentFile()) {
            default_action = isToTrash ? Qt::MoveAction : Qt::CopyAction;
            event->setDropAction(default_action);
        }
    }
}

void DFileViewHelper::preproccessDropEvent(QDropEvent *event, const QList<QUrl> &urls) const
{
    bool sameUser = DFMGlobal::isMimeDatafromCurrentUser(event->mimeData());
    if (event->source() == parent() && !DFMGlobal::keyCtrlIsPressed()) {
        event->setDropAction(sameUser ? Qt::MoveAction : Qt::IgnoreAction);
    } else {
        DAbstractFileInfoPointer info = model()->fileInfo(parent()->indexAt(event->pos()));
        if (!info)
            info = model()->fileInfo(parent()->rootIndex());

        if (!info) {
            return;
        }

        if (urls.empty())
            return;

        const DUrl from = DUrl(urls.first());
        DUrl to = info->fileUrl();
        Qt::DropAction default_action = Qt::CopyAction;

        //fix bug#23703勾选自动整理，拖拽其他目录文件到桌面做得是复制操作
        //因为自动整理的路径被DStorageInfo::inSameDevice判断为false，这里做转化
        if (to.scheme() == DFMMD_SCHEME) {
            to = DUrl(info->absoluteFilePath());
            to.setScheme(FILE_SCHEME);
        }
        //end

        if (qApp->keyboardModifiers() == Qt::AltModifier) {
            default_action = Qt::MoveAction;
        } else if (!DFMGlobal::keyCtrlIsPressed()) {
            // 如果文件和目标路径在同一个分区下，默认为移动文件，否则默认为复制文件
            if (DStorageInfo::inSameDevice(from, to)) {
                default_action = Qt::MoveAction;
            }
        }

        //任意来源为回收站的drop操作均为move(统一回收站拖拽标准)
        bool isFromTrash = from.url().contains(".local/share/Trash/");
        bool isToTrash = to.isTrashFile();

        if (isFromTrash || isToTrash) {
            if (!isFromTrash || !isToTrash) {
                default_action = Qt::MoveAction;
            } else {
                return event->setDropAction(Qt::IgnoreAction);
            }
        }

        if (event->possibleActions().testFlag(default_action)) {
            event->setDropAction((default_action == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : default_action);
        }

        // 保险箱时，修改DropAction为Qt::CopyAction
        if (VaultController::isVaultFile(info->fileUrl().toString())
            || VaultController::isVaultFile(urls[0].toString())) {
            event->setDropAction(Qt::CopyAction);
        }

        if (!info->supportedDropActions().testFlag(event->dropAction())) {
            QList<Qt::DropAction> actions;

            actions.reserve(3);
            actions << Qt::CopyAction << Qt::MoveAction << Qt::LinkAction;

            for (Qt::DropAction action : actions) {

                if (event->possibleActions().testFlag(action) && info->supportedDropActions().testFlag(action)) {
                    event->setDropAction((action == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : action);
                    break;
                }
            }
        }

        // 最近使用目录下的文件，只有拖拽到回收站为剪切，其他都为拷贝
        if (from.isRecentFile()) {
            default_action = isToTrash ? Qt::MoveAction : Qt::CopyAction;
            event->setDropAction(default_action);
        }
    }
}

void DFileViewHelper::handleCommitData(QWidget *editor) const
{
    if (!editor) {
        return;
    }

    const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(itemDelegate()->editingIndex());

    if (!fileInfo) {
        return;
    }

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    FileIconItem *item = qobject_cast<FileIconItem *>(editor);

    DFMEvent event(this);
    event.setData(fileInfo->fileUrl());
    event.setWindowId(windowId());

    QString new_file_name = lineEdit ? lineEdit->text() : item ? item->edit->toPlainText() : "";

    if (new_file_name.isEmpty()) {
        return;
    }

    QString suffix_str_as_var{ editor->property("_d_whether_show_suffix").toString() };

    if (!suffix_str_as_var.isEmpty()) {
        new_file_name += QString{"."};
        new_file_name += suffix_str_as_var;
    } else if (DFMApplication::genericObtuselySetting()->value("FileName", "non-allowableEmptyCharactersOfEnd").toBool()) {
        //保留文件名称中的空格符号
        //new_file_name = new_file_name.trimmed();

        if (new_file_name.isEmpty()) {
            return;
        }
    }

    if (fileInfo->fileName() == new_file_name) {
        return;
    }

    DUrl old_url = fileInfo->fileUrl();
    //处理tag目录重命名了逻辑
    if (old_url.isTaggedFile() && old_url.taggedLocalFilePath().isEmpty()) {
        TagManager::instance()->changeTagName(qMakePair(old_url.tagName(), new_file_name));
        return;
    }

    DUrl new_url = fileInfo->getUrlByNewFileName(new_file_name);

    const DAbstractFileInfoPointer &newFileInfo = DFileService::instance()->createFileInfo(this, new_url);

    if (newFileInfo && newFileInfo->fileName().isEmpty() && newFileInfo->suffixOfRename() == fileInfo->suffixOfRename()) {
        return;
    }
    AppController::selectionFile = qMakePair(new_url, windowId());

    // 重命名文件完成后会删除view中的文件所在行，此时会触发销毁文件名编辑控件的动作。
    // 但是QAbstractItemView再调用commitEditorData之后紧接着还会调用closeEditor
    // 此时又会尝试销毁编辑控件，导致应用程序崩溃
    TIMER_SINGLESHOT(0, {
        fileService->renameFile(this, old_url, new_url);
    }, old_url, new_url, this)
}

#include "moc_dfileviewhelper.cpp"
