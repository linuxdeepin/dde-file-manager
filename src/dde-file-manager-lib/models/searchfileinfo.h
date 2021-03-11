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

#ifndef SEARCHFILEINFO_H
#define SEARCHFILEINFO_H

#include "dfileinfo.h"

class SearchFileInfo : public DAbstractFileInfo
{
public:
    explicit SearchFileInfo(const DUrl &url);

    bool exists() const override;
    bool isReadable() const override;
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

private:
    DUrl m_parentUrl;

};

#endif // SEARCHFILEINFO_H
