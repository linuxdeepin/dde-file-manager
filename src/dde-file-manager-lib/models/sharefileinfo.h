// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREFILEINFO_H
#define SHAREFILEINFO_H

#include <QObject>
#include "dabstractfileinfo.h"

class ShareFileInfo : public DAbstractFileInfo
{
public:
    explicit ShareFileInfo(const DUrl &url);
    ~ShareFileInfo() override;
    bool exists() const override;
    bool isDir() const override;
    bool canRename() const override;
    bool isReadable() const override;
    bool isWritable() const override;

    QString fileDisplayName() const override;

    QVector<MenuAction> menuActionList(MenuType type) const override;
    QSet<MenuAction> disableMenuActionList() const override;

    bool columnDefaultVisibleForRole(int userColumnRole) const override;

    MenuAction menuActionByColumnRole(int userColumnRole) const override;

    bool canIteratorDir() const override;
    bool makeAbsolute() override;

    DUrl mimeDataUrl() const override;
    DUrl parentUrl() const override;

    bool isShared() const override;
    bool canTag() const override;
    bool isVirtualEntry() const override;
    virtual bool canDrop() const override;

    Qt::ItemFlags fileItemDisableFlags() const override;
    QList<QIcon> additionalIcon() const override;

    virtual bool canRedirectionFileUrl() const override;
    virtual DUrl redirectedFileUrl() const override;

private:
    QString m_displayName;

};

#endif // SHAREFILEINFO_H
