// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
