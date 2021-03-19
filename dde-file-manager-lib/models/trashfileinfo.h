/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef TRASHFILEINFO_H
#define TRASHFILEINFO_H

#include "dabstractfileinfo.h"

class FileJob;
class DFMEvent;
class TrashFileInfoPrivate;
class TrashFileInfo : public DAbstractFileInfo
{
public:
    TrashFileInfo(const DUrl &url);

    bool exists() const Q_DECL_OVERRIDE;
    bool canRename() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool canShare() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;

    QString fileDisplayName() const Q_DECL_OVERRIDE;
    QFile::Permissions permissions() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;

    void setColumnCompact(bool) Q_DECL_OVERRIDE;
    QList<int> userColumnRoles() const Q_DECL_OVERRIDE;
    QVariant userColumnData(int userColumnRole) const Q_DECL_OVERRIDE;
    QVariant userColumnDisplayName(int userColumnRole) const Q_DECL_OVERRIDE;

    QList<int> userColumnChildRoles(int column) const Q_DECL_OVERRIDE;

    int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const Q_DECL_OVERRIDE;
    QString subtitleForEmptyFloder() const override;
//    int userRowHeight(const QFontMetrics &fontMetrics) const Q_DECL_OVERRIDE;

    CompareFunction compareFunByColumn(int columnRole) const Q_DECL_OVERRIDE;
    bool columnDefaultVisibleForRole(int userColumnRole) const Q_DECL_OVERRIDE;


    MenuAction menuActionByColumnRole(int userColumnRole) const Q_DECL_OVERRIDE;
    QList<int> sortSubMenuActionUserColumnRoles() const Q_DECL_OVERRIDE;

    bool canIteratorDir() const Q_DECL_OVERRIDE;
    bool makeAbsolute() Q_DECL_OVERRIDE;

    DUrl originUrl() const;
    DUrl mimeDataUrl() const Q_DECL_OVERRIDE;
    Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;
    Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;

    QIcon fileIcon() const Q_DECL_OVERRIDE;
    QList<QIcon> additionalIcon() const Q_DECL_OVERRIDE;

    DUrl goToUrlWhenDeleted() const Q_DECL_OVERRIDE;

    bool restore(QSharedPointer<FileJob> job) const;
    QDateTime deletionDate() const;
    QString sourceFilePath() const;

private:
    Q_DECLARE_PRIVATE(TrashFileInfo)
};

#endif // TRASHFILEINFO_H
