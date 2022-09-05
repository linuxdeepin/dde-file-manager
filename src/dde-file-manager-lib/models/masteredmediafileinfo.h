// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MASTEREDMEDIAFILEINFO_H
#define MASTEREDMEDIAFILEINFO_H

#include "dfileinfo.h"

class MasteredMediaFileInfo : public DAbstractFileInfo
{
public:
    explicit MasteredMediaFileInfo(const DUrl &url);

    bool exists() const override;
    bool isReadable() const override;
    bool isWritable() const override;
    bool isDir() const override;

    int filesCount() const override;
    QString fileDisplayName() const override;

    QVariantHash extraProperties() const override;
    QVector<MenuAction> menuActionList(MenuType type) const override;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;
    DUrl mimeDataUrl() const override;

    bool canIteratorDir() const override;
    DUrl parentUrl() const override;
    DUrl goToUrlWhenDeleted() const override;
    QString toLocalFile() const override;

    bool canDrop() const override;
    bool canTag() const override;
    bool canRename() const override;
    QSet<MenuAction> disableMenuActionList() const override;
    void refresh(const bool isForce = false) override;
    bool canDragCompress() const override;

    virtual QString subtitleForEmptyFloder() const override;

private:
    QString getVolTag(const QString &burnPath) const;

    void backupInfo(const DUrl &url);

private:
    DUrl m_backerUrl;
};

#endif // MASTEREDMEDIAFILEINFO_H
