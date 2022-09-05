// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSFILEINFO_H
#define AVFSFILEINFO_H

#include "interfaces/dabstractfileinfo.h"

class AVFSFileInfoPrivate;
class AVFSFileInfo : public DAbstractFileInfo
{
public:
    explicit AVFSFileInfo(const DUrl &avfsUrl);

    bool canRename() const override;
    bool isWritable() const override;
    bool canShare() const override;
    bool canIteratorDir() const override;
    bool isDir() const override;
    bool canManageAuth() const override;

    QString toLocalFile() const override;
    DUrl parentUrl() const override;
    QVector<MenuAction> menuActionList(MenuType type) const override;

    static DUrl realFileUrl(const DUrl &avfsUrl);
    static DUrl realDirUrl(const DUrl &avfsUrl);
protected:
    explicit AVFSFileInfo(AVFSFileInfoPrivate &dd);

    Q_DECLARE_PRIVATE(AVFSFileInfo)
};

#endif // AVFSFILEINFO_H
