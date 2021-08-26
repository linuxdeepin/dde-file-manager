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

#include "dfm-base/shutil/dmimedatabase.h"
#include "dfm-base/base/dabstractfileinfo.h"

#include <QIcon>
#include <QPointF>
#include <QMimeType>
#include <QMimeDatabase>

class DFMLocalFileInfoPrivate;
class DFMLocalFileInfo : public DAbstractFileInfo
{
    Q_DECLARE_PRIVATE(DFMLocalFileInfo)
    DFMLocalFileInfoPrivate * const d_ptr;

public:
    enum Icon {
        LinkIcon,
        LockIcon,
        UnreadableIcon,
        ShareIcon,
    };

    enum FlagIcon {
        Writable,
        SymLink,
        Readable,
    };Q_ENUMS(FlagIcon)

    struct DFMEmblemInfo
    {
        FlagIcon flag;
        QIcon icon;
        QPointF point;
    };

    typedef QList<DFMEmblemInfo> DFMEmblemInfos;

    explicit DFMLocalFileInfo(const QUrl &url);

    virtual ~DFMLocalFileInfo() override;

    DFMLocalFileInfo& operator = (const DFMLocalFileInfo &info);

    //获取当前是为否块设备
    virtual bool isBlockDev() const;

    //获取挂载路径
    virtual QString mountPath() const;

    //获取当前是否为字符设备
    virtual bool isCharDev() const;

    //获取当前是否为管道文件
    virtual bool isFifo() const;

    //获取当前是否为套接字文件
    virtual bool isSocket() const;

    //获取当前是否是常规文件(与isFile一致)
    virtual bool isRegular() const;

    //设置自定义角标信息
    virtual void setEmblems(const DFMEmblemInfos &infos);

    //清除自定义角标
    virtual void clearEmblems();

    //角标信息设置
    virtual DFMEmblemInfos emblems() const;

    //设置文件图标
    virtual void setIcon(const QIcon &icon);

    //获取文件图标
    virtual QIcon icon() const;

    //Mime文件类型
    virtual DMimeDatabase::FileType fileType() const;

    //获取链接文件的目标路径
    virtual QString linkTargetPath() const;

    //文件夹下子文件的个数
    virtual int countChildFile() const;

    //格式化大小
    virtual QString sizeFormat() const;

    virtual QString fileDisplayName() const;

    void refresh() override;

    virtual QFileInfo toQFileInfo() const;

    virtual QVariantHash extraProperties() const;

    virtual quint64 inode() const;

protected:
    explicit DFMLocalFileInfo();
    explicit DFMLocalFileInfo(const QString &filePath);
    explicit DFMLocalFileInfo(const QFileInfo &fileInfo);
};

typedef QSharedPointer<DFMLocalFileInfo> DFMLocalFileInfoPointer;

#endif // FILEINFO_H
