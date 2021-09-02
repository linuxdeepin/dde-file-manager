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

#ifndef ABSTRACTFILEINFO_H
#define ABSTRACTFILEINFO_H

#include "dfm-base/base/urlroute.h"

#include <QFileInfo>
#include <QDateTime>
#include <QFileSystemWatcher>

#include <dfm-io/core/dfileinfo.h>

class AbstractFileInfoPrivate;
class AbstractFileInfo: public QSharedData
{
    Q_DECLARE_PRIVATE(AbstractFileInfo)
    AbstractFileInfoPrivate * const d_ptr;
public:
    /*!
     * \enum FileInfoCacheType 文件缓存的key值
     * \brief 文件缓存的key值，不同的key对应不同的文件属性，用于在DAbstractFileInfoPrivate中缓存文件信息的key
     */
    enum FileInfoCacheType {
        TypeExists,                             // 文件存在
        TypeFilePath,                           // 文件路径
        TypeAbsoluteFilePath,                   // 文件绝对路径
        TypeFileName,                           // 文件名称
        TypeBaseName,                           // 文件基础名称
        TypeCompleteBaseName,                   // 文件完整的基础名称
        TypeSuffix,                             // 文件的suffix
        TypeCompleteSuffix,                     // 文件的完成suffix
        TypePath,                               // 路径
        TypeIsReadable,                         // 文件是否可读
        TypeIsWritable,                         // 文件是否可写
        TypeIsExecutable,                       // 文件是否可执行
        TypeIsHidden,                           // 文件是否隐藏
        TypeIsFile,                             // 是否是文件
        TypeIsDir,                              // 是否是目录
        TypeIsSymLink,                          // 是否是链接文件
        TypeSymLinkTarget,                      // 链接文件的目标文件
        TypeOwner,                              // 文件的拥有者
        TypeOwnerID,                            // 文件的拥有者的id
        TypeGroup,                              // 文件所在组
        TypeGroupID,                            // 文件所在组的id
        TypePermissions,                        // 文件的所有权限
        TypeSize,                               // 文件的大小
        TypeCreateTime,                         // 文件的创建时间
        TypeChangeTime,                         // 文件的改变时间
        TypeLastModifyTime,                     // 文件的最后修改时间
        TypeLastReadTime,                       // 文件的最后修改时间
        TypeUnknow = 255
    };

public:
    AbstractFileInfo();
    AbstractFileInfo(const QUrl &url);
    AbstractFileInfo(const QString &file);
    AbstractFileInfo(const QFile &file);
    AbstractFileInfo(const QDir &dir, const QString &file);
    AbstractFileInfo(const QFileInfo &fileinfo);
    virtual ~AbstractFileInfo();
    virtual AbstractFileInfo &operator=(const AbstractFileInfo &fileinfo);
    virtual bool operator==(const AbstractFileInfo &fileinfo) const;
    virtual bool operator!=(const AbstractFileInfo &fileinfo) const;
    virtual void setFile(const DFMIO::DFileInfo &file);
    virtual void setFile(const QFile &file);
    virtual void setFile(const QUrl &url);
    virtual bool exists() const;
    virtual void refresh();
    virtual QString filePath() const;
    virtual QString absoluteFilePath() const;    
    virtual QString fileName() const;    
    virtual QString baseName() const;    
    virtual QString completeBaseName() const;    
    virtual QString suffix() const;    
    virtual QString completeSuffix();    
    virtual QString path() const;    
    virtual QString absolutePath() const;    
    virtual QString canonicalPath() const;    
    virtual QDir dir() const;    
    virtual QDir absoluteDir() const;    
    virtual QUrl url() const;    
    virtual bool isReadable() const;    
    virtual bool isWritable() const;   
    virtual bool isExecutable() const;    
    virtual bool isHidden() const;   
    virtual bool isFile() const;   
    virtual bool isDir() const;   
    virtual bool isSymLink() const;   
    virtual bool isRoot() const;   
    virtual bool isBundle() const;    
    virtual QString symLinkTarget() const;    
    virtual QString owner() const;    
    virtual uint ownerId() const;    
    virtual QString group() const;   
    virtual uint groupId() const;   
    virtual bool permission(QFile::Permissions permissions) const;    
    virtual QFile::Permissions permissions() const;    
    virtual qint64 size() const; 
    virtual QDateTime created() const;
    virtual QDateTime birthTime() const;
    virtual QDateTime metadataChangeTime() const;
    virtual QDateTime lastModified() const;
    virtual QDateTime lastRead() const;
    virtual QDateTime fileTime(QFile::FileTime time) const;
};

typedef QSharedPointer<AbstractFileInfo> AbstractFileInfoPointer;

Q_DECLARE_METATYPE(AbstractFileInfoPointer)

#endif // ABSTRACTFILEINFO_H
