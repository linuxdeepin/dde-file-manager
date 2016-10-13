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
#include "views/windowmanager.h"

#define protected public
#include "models/abstractfileinfo.h"
#undef protected

DFileViewHelper::DFileViewHelper(QAbstractItemView *parent)
    : QObject(parent)
{
    Q_ASSERT(parent);
}

QAbstractItemView *DFileViewHelper::parent() const
{
    return qobject_cast<QAbstractItemView*>(QObject::parent());
}

/*!
 * \brief Returns the window id of the top-level window in the parent widget.
 * \return
 */
int DFileViewHelper::windowId() const
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
    const AbstractFileInfo *fileInfo = this->fileInfo(index);

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
    QList<QIcon> icons;
    const AbstractFileInfo *fileInfo = this->fileInfo(index);

    if (!fileInfo)
        return icons;

    if (fileInfo->isSymLink()) {
        icons << DFMGlobal::instance()->standardIcon(DFMGlobal::LinkIcon);
    }

    if (!fileInfo->isWritable())
        icons << DFMGlobal::instance()->standardIcon(DFMGlobal::LockIcon);

    if (!fileInfo->isReadable())
        icons << DFMGlobal::instance()->standardIcon(DFMGlobal::UnreadableIcon);

    if (fileInfo->isShared())
        icons << DFMGlobal::instance()->standardIcon(DFMGlobal::ShareIcon);

    return icons;
}

/*!
 * \brief Returns the file name that needs to be selected when renaming the file.
 * \param index
 * \return
 */
QString DFileViewHelper::selectionWhenEditing(const QModelIndex &index) const
{
    const AbstractFileInfo *fileInfo = this->fileInfo(index);

    if (!fileInfo)
        return QString();

    const QString &fileName = fileInfo->displayName();

    if (fileInfo->isFile()) {
        const QString &suffix = fileInfo->suffix();

        if (fileName.endsWith(suffix)) {
            return fileName.left(fileName.length() - suffix.length() - 1);
        }
    }

    return fileName;
}

/*!
 * \brief Return file info by index.
 * \param index
 * \return
 */
const AbstractFileInfo *DFileViewHelper::fileInfo(const QModelIndex &index) const
{
    Q_UNUSED(index)

    return 0;
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
