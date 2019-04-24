/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dfileviewhelper.h"
#include "dfmglobal.h"
#include "dstyleditemdelegate.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "views/windowmanager.h"
#include "dabstractfileinfo.h"
#include "dfilesystemmodel.h"
#include "views/fileitem.h"
#include "views/themeconfig.h"
#include "views/dtagactionwidget.h"
#include "singleton.h"
#include "dfileservices.h"
#include "dfmgenericfactory.h"
#include "dialogs/filepreviewdialog.h"
#include "controllers/appcontroller.h"
#include "dfmeventdispatcher.h"
#include "dfilemenu.h"
#include "tag/tagmanager.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "dstorageinfo.h"

#include "deviceinfo/udisklistener.h"

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
    MenuActionEventHandler(DFileViewHelper *helper)
        : DFMAbstractEventHandler(false)
        , viewHelper(helper) {}

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

        if (menu_event.action() != DFMGlobal::TagInfo)
            return false;

        if (menu_event.selectedUrls().isEmpty())
            return false;

        const QModelIndex &index = viewHelper->model()->index(menu_event.selectedUrls().first());
        const QRect &rect = viewHelper->parent()->visualRect(index);
        QStyleOptionViewItem option = viewHelper->parent()->viewOptions();

        option.rect = rect;

        const QList<QRect> &geometry_list = viewHelper->itemDelegate()->paintGeomertys(option, index);
        const QRect &icon_rect = geometry_list.first();
        const QPoint &edit_pos = QPoint(icon_rect.x() + icon_rect.width() / 2, icon_rect.bottom());

        appController->showTagEdit(viewHelper->parent()->viewport()->mapToGlobal(edit_pos), menu_event.selectedUrls());

        return true;
    }

private:
    DFileViewHelper *viewHelper;
};

class DFileViewHelperPrivate
{
public:
    DFileViewHelperPrivate(DFileViewHelper *qq)
        : menuEventHandler(new MenuActionEventHandler(qq))
        , q_ptr(qq)
    {
        DFMEventDispatcher::instance()->installEventFilter(menuEventHandler);
    }

    ~DFileViewHelperPrivate()
    {
        delete menuEventHandler;
    }

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
    keyboardSearchTimer.setInterval(1000);

    // init connects
    QObject::connect(&keyboardSearchTimer, &QTimer::timeout,
    q, [this] {
        keyboardSearchKeys.clear();
    });
    QObject::connect(qApp, &DApplication::iconThemeChanged, q->parent(), static_cast<void (QWidget::*)()>(&QWidget::update));
    QObject::connect(DFMGlobal::instance(), &DFMGlobal::clipboardDataChanged, q, [q] {
        for (const QModelIndex &index : q->itemDelegate()->hasWidgetIndexs())
        {
            QWidget *item = q->indexWidget(index);

            if (item) {
                item->setProperty("opacity", q->isCut(index) ? 0.3 : 1);
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
        fileService->writeFilesToClipboard(q, DFMGlobal::CopyAction, q->selectedUrls());
    });

    QAction *cut_action = new QAction(q->parent());

    cut_action->setAutoRepeat(false);
    cut_action->setShortcut(QKeySequence::Cut);

    QObject::connect(cut_action, &QAction::triggered,
    q, [q] {
        fileService->writeFilesToClipboard(q, DFMGlobal::CutAction, q->selectedUrls());
    });

    QAction *paste_action = new QAction(q->parent());

    paste_action->setShortcut(QKeySequence::Paste);

    QObject::connect(paste_action, &QAction::triggered,
    q, [q] {
        fileService->pasteFileByClipboard(q->parent(), q->currentUrl());
    });

    QAction *revocation_action = new QAction(q->parent());

    revocation_action->setShortcut(QKeySequence::Undo);

    QObject::connect(revocation_action, &QAction::triggered,
    q, [q] {
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
 * \brief Return true if index is cut state; otherwise return false.
 * \param index
 * \return
 */
bool DFileViewHelper::isCut(const QModelIndex &index) const
{
    const DAbstractFileInfoPointer &fileInfo = this->fileInfo(index);

    if (!fileInfo) {
        return false;
    }
    DUrl fileUrl = fileInfo->fileUrl();
    if (fileInfo->fileUrl().isSearchFile()) {
        fileUrl = fileInfo->fileUrl().searchedFileUrl();
    }

    return DFMGlobal::instance()->clipboardAction() == DFMGlobal::CutAction
           && DFMGlobal::instance()->clipboardFileUrlList().contains(fileUrl);
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
    if (isSelected(index)) {
        option->state |= QStyle::State_Selected;
    } else {
        option->state &= QStyle::StateFlag(~QStyle::State_Selected);
    }

    option->palette.setColor(QPalette::Text, ThemeConfig::instace()->color("FileView", "color"));
    option->palette.setColor(QPalette::Disabled, QPalette::Text, ThemeConfig::instace()->color("FileView", "color", ThemeConfig::Dislable));
    if ((option->state & QStyle::State_Selected) && option->showDecorationSelected) {
        option->palette.setColor(QPalette::Inactive, QPalette::Text, ThemeConfig::instace()->color("FileView", "color", ThemeConfig::Checked | ThemeConfig::Inactive));
    } else {
        option->palette.setColor(QPalette::Inactive, QPalette::Text, ThemeConfig::instace()->color("FileView", "color", ThemeConfig::Inactive));
    }
    option->palette.setColor(QPalette::BrightText, Qt::white);
    option->palette.setBrush(QPalette::Shadow, ThemeConfig::instace()->color("FileView", "shadow"));

    bool cuted = isCut(index);

    if (cuted) {
        option->backgroundBrush = ThemeConfig::instace()->color("FileView", "background", ThemeConfig::Inactive);
    }

    if ((option->state & QStyle::State_HasFocus) && option->showDecorationSelected && selectedIndexsCount() > 1) {
        option->palette.setColor(QPalette::Background, ThemeConfig::instace()->color("FileView", "background", ThemeConfig::Focus));

        if (!cuted)
            option->backgroundBrush = ThemeConfig::instace()->color("FileView", "background", ThemeConfig::Focus);
    } else {
        option->palette.setColor(QPalette::Background, ThemeConfig::instace()->color("FileView", "background"));

        if (!cuted)
            option->backgroundBrush = ThemeConfig::instace()->color("FileView", "background");
    }

    option->textElideMode = Qt::ElideLeft;
}

void DFileViewHelper::handleMenu(QMenu *menu)
{
    DFileMenu *file_menu = qobject_cast<DFileMenu*>(menu);

    if (Q_UNLIKELY(!file_menu))
        return;

    QAction *tag_action = file_menu->actionAt("Add color tags");

    if (!tag_action)
        return;

    QWidgetAction *widget_action = qobject_cast<QWidgetAction*>(tag_action);

    if (Q_UNLIKELY(!widget_action))
        return;

    DTagActionWidget* tag_widget = qobject_cast<DTagActionWidget*>(widget_action->defaultWidget());

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

    connect(tag_widget, &DTagActionWidget::hoverColorChanged, menu, [tag_widget] (const QColor &color) {
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

    d->keyboardSearchKeys.append(key);

    bool reverse_order = qApp->keyboardModifiers() == Qt::ShiftModifier;
    const QModelIndex &current_index = parent()->currentIndex();

    QModelIndex index = d->findIndex(d->keyboardSearchKeys, true, current_index.row(), reverse_order, !d->keyboardSearchTimer.isActive());

    if (!index.isValid()) {
        // 使用 QString::contains 模式再次匹配
        index = d->findIndex(d->keyboardSearchKeys, false, current_index.row(), reverse_order, !d->keyboardSearchTimer.isActive());
    }

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

        for (const QRect &rect : geometry_list) {
            if (rect.contains(pos)) {
                return false;
            }
        }
    }

    return true;
}

void DFileViewHelper::preproccessDropEvent(QDropEvent *event) const
{
    if (event->source() == parent() && !DFMGlobal::keyCtrlIsPressed()) {
        event->setDropAction(Qt::MoveAction);
    } else {
        DAbstractFileInfoPointer info = model()->fileInfo(parent()->indexAt(event->pos()));

        if (!info)
            info = model()->fileInfo(parent()->rootIndex());

        if (!info) {
            return;
        }

        const QList<QUrl> &urls = event->mimeData()->urls();

        if (urls.isEmpty()) {
            return;
        }

        const DUrl from = urls.first();
        const DUrl to = info->fileUrl();
        Qt::DropAction default_action = Qt::CopyAction;

        if (qApp->keyboardModifiers() == Qt::AltModifier) {
            default_action = Qt::MoveAction;
        } else if (!DFMGlobal::keyCtrlIsPressed()) {
            // 如果文件和目标路径在同一个分区下，默认为移动文件，否则默认为复制文件
            if (DStorageInfo::inSameDevice(from, to)) {
                default_action = Qt::MoveAction;
            }
        }

        if (event->possibleActions().testFlag(default_action)) {
            event->setDropAction(default_action);
        }

        if (!info->supportedDropActions().testFlag(event->dropAction())) {
            QList<Qt::DropAction> actions;

            actions.reserve(3);
            actions << Qt::CopyAction << Qt::MoveAction << Qt::LinkAction;

            for (Qt::DropAction action : actions) {
                if (event->possibleActions().testFlag(action) && info->supportedDropActions().testFlag(action)) {
                    event->setDropAction(action);

                    break;
                }
            }
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
        new_file_name = new_file_name.trimmed();

        if (new_file_name.isEmpty()) {
            return;
        }
    }

    if (fileInfo->fileName() == new_file_name) {
        return;
    }

    DUrl old_url = fileInfo->fileUrl();
    DUrl new_url = fileInfo->getUrlByNewFileName(new_file_name);

    const DAbstractFileInfoPointer &newFileInfo = DFileService::instance()->createFileInfo(this, new_url);

    if (newFileInfo && newFileInfo->baseNameOfRename().isEmpty() && newFileInfo->suffixOfRename() == fileInfo->suffixOfRename()) {
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
