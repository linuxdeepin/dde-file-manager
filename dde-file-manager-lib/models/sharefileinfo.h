/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SHAREFILEINFO_H
#define SHAREFILEINFO_H

#include <QObject>
#include "dabstractfileinfo.h"
#include "dfmevent.h"

class ShareFileInfo : public DAbstractFileInfo
{
public:
    ShareFileInfo(const DUrl& url);
    ~ShareFileInfo();
    bool exists() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    bool canRename() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;

    QString fileDisplayName() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;

    bool columnDefaultVisibleForRole(int userColumnRole) const Q_DECL_OVERRIDE;

    MenuAction menuActionByColumnRole(int userColumnRole) const Q_DECL_OVERRIDE;

    bool canIteratorDir() const Q_DECL_OVERRIDE;
    bool makeAbsolute() Q_DECL_OVERRIDE;

    DUrl mimeDataUrl() const Q_DECL_OVERRIDE;
    DUrl parentUrl() const Q_DECL_OVERRIDE;

    bool isShared() const Q_DECL_OVERRIDE;

    Qt::ItemFlags fileItemDisableFlags() const Q_DECL_OVERRIDE;
    QList<QIcon> additionalIcon() const Q_DECL_OVERRIDE;

    virtual bool canRedirectionFileUrl() const Q_DECL_OVERRIDE;
    virtual DUrl redirectedFileUrl() const Q_DECL_OVERRIDE;

private:
    QString m_displayName;

};

#endif // SHAREFILEINFO_H
