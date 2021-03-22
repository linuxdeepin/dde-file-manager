/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
    explicit TrashFileInfo(const DUrl &url);

    bool exists() const override;
    bool canRename() const override;
    bool isReadable() const override;
    bool isWritable() const override;
    bool canShare() const override;
    bool isDir() const override;

    QString fileDisplayName() const override;
    QFile::Permissions permissions() const override;

    QVector<MenuAction> menuActionList(MenuType type) const override;
    QSet<MenuAction> disableMenuActionList() const override;

    void setColumnCompact(bool) override;
    QList<int> userColumnRoles() const override;
    QVariant userColumnData(int userColumnRole) const override;
    QVariant userColumnDisplayName(int userColumnRole) const override;

    QList<int> userColumnChildRoles(int column) const override;

    int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const override;
    QString subtitleForEmptyFloder() const override;
//    int userRowHeight(const QFontMetrics &fontMetrics) const override;

    CompareFunction compareFunByColumn(int columnRole) const override;
    bool columnDefaultVisibleForRole(int userColumnRole) const override;


    MenuAction menuActionByColumnRole(int userColumnRole) const override;
    QList<int> sortSubMenuActionUserColumnRoles() const override;

    bool canIteratorDir() const override;
    bool makeAbsolute() override;

    DUrl originUrl() const;
    DUrl mimeDataUrl() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    QIcon fileIcon() const override;
    QList<QIcon> additionalIcon() const override;

    DUrl goToUrlWhenDeleted() const override;

    bool restore(QSharedPointer<FileJob> job) const;
    QDateTime deletionDate() const;
    QString sourceFilePath() const;

private:
    Q_DECLARE_PRIVATE(TrashFileInfo)
};

#endif // TRASHFILEINFO_H
