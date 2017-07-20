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
#include "singleton.h"
#include "dfileservices.h"
#include "dfmgenericfactory.h"
#include "dialogs/filepreviewdialog.h"

#include <QTimer>
#include <QAction>
#define protected public
#include <QAbstractItemView>
#undef protected
#include <QLineEdit>
#include <QTextEdit>

#include <DApplication>

DWIDGET_USE_NAMESPACE

class DFileViewHelperPrivate
{
public:
    DFileViewHelperPrivate(DFileViewHelper *qq)
        : q_ptr(qq) {}

    void init();

    void _q_edit(const DFMUrlBaseEvent &event);
    void _q_selectAndRename(const DFMUrlBaseEvent &event);

    QByteArray keyboardSearchKeys;
    QTimer keyboardSearchTimer;

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
    keyboardSearchTimer.setInterval(500);

    // init connects
    QObject::connect(&keyboardSearchTimer, &QTimer::timeout,
                    q , [this] {
        keyboardSearchKeys.clear();
    });
    QObject::connect(qApp, &DApplication::iconThemeChanged, q->parent(), static_cast<void (QWidget::*)()>(&QWidget::update));
    QObject::connect(DFMGlobal::instance(), &DFMGlobal::clipboardDataChanged, q, [q] {
        for (const QModelIndex &index : q->itemDelegate()->hasWidgetIndexs()) {
            FileIconItem *item = qobject_cast<FileIconItem*>(q->indexWidget(index));

            if (item)
                item->setOpacity(q->isCut(index) ? 0.3 : 1);
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

    q->parent()->addAction(copy_action);
    q->parent()->addAction(cut_action);
    q->parent()->addAction(paste_action);

    q->connect(fileSignalManager, SIGNAL(requestRename(DFMUrlBaseEvent)), q, SLOT(_q_edit(DFMUrlBaseEvent)));
    q->connect(fileSignalManager, SIGNAL(requestSelectRenameFile(DFMUrlBaseEvent)), q, SLOT(_q_selectAndRename(DFMUrlBaseEvent)));
    // call later
    TIMER_SINGLESHOT(0, {
                         q->connect(fileSignalManager, SIGNAL(trashStateChanged()), q->model(), SLOT(update()));
                     }, q);

    // init plugin objects
    static bool initialized = false;

    if (!initialized) {
        initialized = true;

        pluginObjectList = DFMGenericFactory::createAll(QStringLiteral("fileinfo/additionalIcon"));
    }
}

void DFileViewHelperPrivate::_q_edit(const DFMUrlBaseEvent &event)
{
    Q_Q(DFileViewHelper);

    if (event.windowId() != q->windowId())
        return;

    DUrl fileUrl = event.url();

    if (!fileUrl.isValid())
        return;

    const QModelIndex &index = q->model()->index(fileUrl);

    if (q->parent()->isVisible())
        q->parent()->edit(index, QAbstractItemView::EditKeyPressed, 0);
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
                                                     Q_ARG(const DAbstractFileInfoPointer&, fileInfo));

        if (ok)
            list << plugin_list;
        else
            qWarning() << "call the fileAdditionalIcon slot failed";
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
    return qobject_cast<QAbstractItemView*>(QObject::parent());
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

    if (!fileInfo)
        return false;

    return DFMGlobal::instance()->clipboardAction() == DFMGlobal::CutAction
            && DFMGlobal::instance()->clipboardFileUrlList().contains(fileInfo->fileUrl());
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

    if (!fileInfo)
        return list;

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

    if (!fileInfo)
        return QString();

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

    if (!fileInfo)
        return DUrl();

    return fileInfo->fileUrl();
}

void DFileViewHelper::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    if (isSelected(index))
        option->state |= QStyle::State_Selected;
    else
        option->state &= QStyle::StateFlag(~QStyle::State_Selected);

    option->palette.setColor(QPalette::Text, QColor("#303030"));
    option->palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#797979"));
    if ((option->state & QStyle::State_Selected) && option->showDecorationSelected)
        option->palette.setColor(QPalette::Inactive, QPalette::Text, QColor("#e9e9e9"));
    else
        option->palette.setColor(QPalette::Inactive, QPalette::Text, QColor("#797979"));
    option->palette.setColor(QPalette::BrightText, Qt::white);
    option->palette.setBrush(QPalette::Shadow, QColor(0, 0, 0, 178));

    if ((option->state & QStyle::State_HasFocus) && option->showDecorationSelected && selectedIndexsCount() > 1) {
        option->backgroundBrush = QColor("#0076F9");
    } else {
        option->backgroundBrush = QColor("#2da6f7");
    }
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
    d->keyboardSearchTimer.start();

    int row_count = parent()->model()->rowCount(parent()->rootIndex());
    bool reverse_order = qApp->keyboardModifiers() == Qt::ShiftModifier;
    const QModelIndex &current_index = parent()->currentIndex();

    for (int i = 1; i < row_count; ++i) {
        int row = reverse_order ? row_count + current_index.row() - i : current_index.row() + i;

        row = row % row_count;

        const QModelIndex &index = parent()->model()->index(row, 0, parent()->rootIndex());

        if (index == current_index)
            continue;

        const QString &pinyin_name = parent()->model()->data(index, DFileSystemModel::FilePinyinName).toString();

        if (pinyin_name.startsWith(d->keyboardSearchKeys, Qt::CaseInsensitive)) {
            parent()->setCurrentIndex(index);
            parent()->scrollTo(index, reverse_order ? QAbstractItemView::PositionAtBottom : QAbstractItemView::PositionAtTop);

            return;
        }
    }
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

        if(!rect.contains(pos))
            return true;

        QStyleOptionViewItem option = parent()->viewOptions();

        option.rect = rect;

        const QList<QRect> &geometry_list = itemDelegate()->paintGeomertys(option, index);

        for(const QRect &rect : geometry_list) {
            if(rect.contains(pos)) {
                return false;
            }
        }
    }

    return true;
}

void DFileViewHelper::showPreviewFileDialog()
{
    DUrlList list;

    for (const DUrl &url : selectedUrls()) {
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

        if (info && info->isFile() && !info->toLocalFile().isEmpty())
            list << DUrl::fromLocalFile(info->toLocalFile());
    }

    if (list.isEmpty())
        return;

    FilePreviewDialog *dialog = new FilePreviewDialog(list, parent());

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(true);
    dialog->show();
}

void DFileViewHelper::handleCommitData(QWidget *editor) const
{
    if (!editor)
        return;

    const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(itemDelegate()->editingIndex());

    if (!fileInfo)
        return;

    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    FileIconItem *item = qobject_cast<FileIconItem*>(editor);

    DFMEvent event(this);
    event.setData(fileInfo->fileUrl());
    event.setWindowId(windowId());

    QString new_file_name = lineEdit ? lineEdit->text() : item ? item->edit->toPlainText() : "";

    new_file_name.remove('/');
    new_file_name.remove(QChar(0));

    if (fileInfo->fileName() == new_file_name || new_file_name.isEmpty()) {
        return;
    }

    DUrl old_url = fileInfo->fileUrl();
    DUrl new_url = fileInfo->getUrlByNewFileName(new_file_name);

    const DAbstractFileInfoPointer &newFileInfo = DFileService::instance()->createFileInfo(this, new_url);

    if (newFileInfo && newFileInfo->baseName().isEmpty() && newFileInfo->suffix() == fileInfo->suffix()) {
        return;
    }

    if (lineEdit) {
        /// later rename file.
        TIMER_SINGLESHOT(0, {
                             fileService->renameFile(this, old_url, new_url);
                         }, old_url, new_url, this)
    } else {
        fileService->renameFile(this, old_url, new_url);
    }
}

#include "moc_dfileviewhelper.cpp"
