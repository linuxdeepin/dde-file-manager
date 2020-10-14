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

#ifndef SEARCHFILEINFO_H
#define SEARCHFILEINFO_H

#include "dfileinfo.h"

class SearchFileInfo : public DAbstractFileInfo
{
public:
    explicit SearchFileInfo(const DUrl &url);

    bool exists() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    virtual bool isVirtualEntry() const Q_DECL_OVERRIDE;

    int filesCount() const Q_DECL_OVERRIDE;

    DUrl parentUrl() const Q_DECL_OVERRIDE;

    void setColumnCompact(bool) Q_DECL_OVERRIDE;
    QList<int> userColumnRoles() const Q_DECL_OVERRIDE;
    QVariant userColumnDisplayName(int userColumnRole) const Q_DECL_OVERRIDE;
    /// get custom column data
    QVariant userColumnData(int userColumnRole) const Q_DECL_OVERRIDE;
    QList<int> userColumnChildRoles(int column) const Q_DECL_OVERRIDE;
    bool columnDefaultVisibleForRole(int role) const Q_DECL_OVERRIDE;

    bool canRedirectionFileUrl() const Q_DECL_OVERRIDE;
    DUrl redirectedFileUrl() const Q_DECL_OVERRIDE;

    bool canIteratorDir() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;
    int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const Q_DECL_OVERRIDE;
//    int userRowHeight(const QFontMetrics &fontMetrics) const Q_DECL_OVERRIDE;

    MenuAction menuActionByColumnRole(int userColumnRole) const Q_DECL_OVERRIDE;
    QList<int> sortSubMenuActionUserColumnRoles() const Q_DECL_OVERRIDE;

    bool isEmptyFloder(const QDir::Filters &filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System) const Q_DECL_OVERRIDE;

    CompareFunction compareFunByColumn(int columnRole) const Q_DECL_OVERRIDE;
    bool hasOrderly() const Q_DECL_OVERRIDE;

    DUrl getUrlByNewFileName(const QString &fileName) const Q_DECL_OVERRIDE;

    QString loadingTip() const Q_DECL_OVERRIDE;
    QString subtitleForEmptyFloder() const Q_DECL_OVERRIDE;

    QString fileDisplayName() const Q_DECL_OVERRIDE;

    DUrl mimeDataUrl() const Q_DECL_OVERRIDE;

    QString toLocalFile() const Q_DECL_OVERRIDE;
    virtual QIcon fileIcon() const Q_DECL_OVERRIDE;

private:
    DUrl m_parentUrl;
};

#endif // SEARCHFILEINFO_H
