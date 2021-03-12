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

#ifndef RECENTFILEINFO_H
#define RECENTFILEINFO_H

#include "dabstractfileinfo.h"
#include <QMutex>

class RecentFileInfo;
typedef QExplicitlySharedDataPointer<RecentFileInfo> RecentPointer;

class RecentFileInfo : public DAbstractFileInfo
{
public:
    explicit RecentFileInfo(const DUrl &url);
    ~RecentFileInfo() override;

    bool makeAbsolute() override;
    bool exists() const override;
    bool isDir() const override;
    bool isReadable() const override;
    bool isWritable() const override;
    bool canIteratorDir() const override;
    bool canDrop() const override;
    bool canRedirectionFileUrl() const override;
    bool canRename() const override;
    DUrl redirectedFileUrl() const override;
    DUrl getUrlByNewFileName(const QString &fileName) const override;

    QFileDevice::Permissions permissions() const override;
    QVector<MenuAction> menuActionList(MenuType type) const override;
    QSet<MenuAction> disableMenuActionList() const override;
    QList<int> userColumnRoles() const override;
    QVariant userColumnData(int userColumnRole) const override;
    QVariant userColumnDisplayName(int userColumnRole) const override;
    MenuAction menuActionByColumnRole(int userColumnRole) const override;
    int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const override;
    DUrl mimeDataUrl() const override;
    DUrl parentUrl() const override;

    CompareFunction compareFunByColumn(int columnRole) const override;

    QString toLocalFile() const override;
    QString subtitleForEmptyFloder() const override;
    DUrl goToUrlWhenDeleted() const override;
    virtual QString fileDisplayName() const override;
    virtual QString mimeTypeDisplayName() const override;
    virtual bool isVirtualEntry() const override;
    //获取最近访问时间
    virtual const QDateTime getReadTime() const override;
    //更新最近访问时间
    virtual void updateReadTime(const QDateTime &) override;

    void updateInfo();
    void setReadDateTime(const QString &time);
    QDateTime readDateTime() const;

private:
    QDateTime m_lastReadTime;
    QString m_lastReadTimeStr;
    QMutex m_mutex;
    qint8 m_isWriteAble = -1;
};

#endif // RECENTFILEINFO_H
