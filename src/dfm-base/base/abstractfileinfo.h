/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "dfm-base/dfm_base_global.h"

#include <dfm-io/core/dfileinfo.h>

#include <QSharedData>
#include <QFile>



class QDir;
class QDateTime;

DFMBASE_BEGIN_NAMESPACE

namespace SuffixInfo {
extern const QString USER_DIR;
extern const QString PROTOCOL;
extern const QString BLOCK;
extern const QString STASHED_REMOTE;
} // namespace SuffixInfo

class AbstractFileInfoPrivate;
class AbstractFileInfo : public QSharedData
{
    QScopedPointer<AbstractFileInfoPrivate> d;
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
    explicit AbstractFileInfo() = delete;
    explicit AbstractFileInfo(const QUrl &url);
    virtual ~AbstractFileInfo();
    virtual AbstractFileInfo &operator=(const AbstractFileInfo &fileinfo);
    virtual bool operator==(const AbstractFileInfo &fileinfo) const;
    virtual bool operator!=(const AbstractFileInfo &fileinfo) const;

    virtual void setFile(const QUrl &url);
    virtual void setFile(const DFMIO::DFileInfo &file);
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
DFMBASE_END_NAMESPACE

typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractFileInfo> AbstractFileInfoPointer;
Q_DECLARE_METATYPE(AbstractFileInfoPointer)


#endif // ABSTRACTFILEINFO_H
