// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHFILEINFO_H
#define SEARCHFILEINFO_H

#include "dfileinfo.h"

class SearchFileInfo : public DAbstractFileInfo
{
public:
    explicit SearchFileInfo(const DUrl &url);

    bool exists() const override;
    bool isReadable() const override;
    bool isWritable() const override;
    bool isDir() const override;
    virtual bool isVirtualEntry() const override;

    int filesCount() const override;

    DUrl parentUrl() const override;

    void setColumnCompact(bool) override;
    QList<int> userColumnRoles() const override;
    QVariant userColumnDisplayName(int userColumnRole) const override;
    /// get custom column data
    QVariant userColumnData(int userColumnRole) const override;
    QList<int> userColumnChildRoles(int column) const override;
    bool columnDefaultVisibleForRole(int role) const override;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;

    bool canIteratorDir() const override;

    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const override;
    QSet<MenuAction> disableMenuActionList() const override;
    int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const override;
//    int userRowHeight(const QFontMetrics &fontMetrics) const override;

    MenuAction menuActionByColumnRole(int userColumnRole) const override;
    QList<int> sortSubMenuActionUserColumnRoles() const override;

    bool isEmptyFloder(const QDir::Filters &filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System) const override;

    CompareFunction compareFunByColumn(int columnRole) const override;
    bool hasOrderly() const override;

    DUrl getUrlByNewFileName(const QString &fileName) const override;

    QString loadingTip() const override;
    QString subtitleForEmptyFloder() const override;

    QString fileDisplayName() const override;

    DUrl mimeDataUrl() const override;

    QString toLocalFile() const override;
    virtual QIcon fileIcon() const override;
    virtual bool canDrop() const override;

    virtual bool canRename() const override;

    bool hasSpecialColumn() const;
    QDateTime recentReadDateTime() const;
    QString recentFilePath() const;
    QDateTime trashDeletionDate() const;
    QString trashSourceFilePath() const;

private:
    DUrl m_parentUrl;
    DAbstractFileInfoPointer m_targetFileInfo;

};

#endif // SEARCHFILEINFO_H
