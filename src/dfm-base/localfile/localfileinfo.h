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
#ifndef FILEINFO_H
#define FILEINFO_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/shutil/mimedatabase.h"
#include "dfm-base/base/abstractfileinfo.h"

#include <QIcon>
#include <QPointF>
#include <QMimeType>
#include <QMimeDatabase>

DFMBASE_BEGIN_NAMESPACE
class LocalFileInfoPrivate;
class LocalFileInfo : public AbstractFileInfo
{
    Q_DECLARE_PRIVATE(LocalFileInfo)
    LocalFileInfoPrivate * const d_ptr;

public:
    enum Icon {
        LinkIcon,           // 链接文件图标
        LockIcon,           // 带锁的文件图标
        UnreadableIcon,     // 不可读的文件图标
        ShareIcon,          // 共享的文件图标
    };

    enum FlagIcon {
        Writable,           // 是否可写
        SymLink,            // 是否是链接文件
        Readable,           // 是否可读
    };Q_ENUMS(FlagIcon)

    struct DFMEmblemInfo
    {
        FlagIcon flag;      // 图标标识
        QIcon icon;         // 图标
        QPointF point;      // 图标位置
    };

    typedef QList<DFMEmblemInfo> DFMEmblemInfos;
    explicit LocalFileInfo(const QUrl &url);
    virtual ~LocalFileInfo() override;

    LocalFileInfo& operator = (const LocalFileInfo &info);
    virtual bool isBlockDev() const;
    virtual QString mountPath() const;
    virtual bool isCharDev() const;
    virtual bool isFifo() const;
    virtual bool isSocket() const;
    virtual bool isRegular() const;
    virtual void setEmblems(const DFMEmblemInfos &infos);
    virtual void clearEmblems();
    virtual DFMEmblemInfos emblems() const;
    virtual void setIcon(const QIcon &icon);
    virtual QIcon icon() const;
    virtual MimeDatabase::FileType fileType() const;
    virtual QString linkTargetPath() const;
    virtual int countChildFile() const;
    virtual QString sizeFormat() const;
    virtual QString fileDisplayName() const;
    void refresh() override;
    virtual QFileInfo toQFileInfo() const;
    virtual QVariantHash extraProperties() const;
    virtual quint64 inode() const;

protected:
    explicit LocalFileInfo();
    explicit LocalFileInfo(const QString &filePath);
    explicit LocalFileInfo(const QFileInfo &fileInfo);
};
DFMBASE_END_NAMESPACE
typedef QSharedPointer<DFMBASE_NAMESPACE::LocalFileInfo> DFMLocalFileInfoPointer;

#endif // FILEINFO_H
