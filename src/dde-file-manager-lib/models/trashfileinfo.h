// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
