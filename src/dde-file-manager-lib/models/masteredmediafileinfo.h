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
