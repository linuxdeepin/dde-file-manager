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

#include "dfm-base/base/dfmurlroute.h"

#include <QFileInfo>
#include <QDateTime>
#include <QFileSystemWatcher>

class DAbstractFileInfoPrivate;

/* @class DAbstractFileInfo 抽象文件信息类
 * @brief 内部实现Url到真实路径的信息关联，设置的真实本地路径总是指向虚拟路径Url
 *  在这之前你应该明确你的路径是否被DFMUrlRoute注册，如果没有注册那么Url会指向QUrl标准定义：
 *  QUrl(file:///root) 标识/root路径
 */

class DAbstractFileInfo: public QSharedData
{
    Q_DECLARE_PRIVATE(DAbstractFileInfo)
    DAbstractFileInfoPrivate * const d_ptr;

public:
    DAbstractFileInfo();

    //新增接口Url转换，传入订阅的scheme会转换成真实路径进行FileInfo构造
    DAbstractFileInfo(const QUrl &url);
    DAbstractFileInfo(const QString &file);
    DAbstractFileInfo(const QFile &file);
    DAbstractFileInfo(const QDir &dir, const QString &file);
    DAbstractFileInfo(const QFileInfo &fileinfo);

    virtual ~DAbstractFileInfo();
    virtual DAbstractFileInfo &operator=(const DAbstractFileInfo &fileinfo);
    virtual bool operator==(const DAbstractFileInfo &fileinfo) const;
    virtual bool operator!=(const DAbstractFileInfo &fileinfo) const;
    virtual void setFile(const QString &file);
    virtual void setFile(const QFile &file);
    virtual void setFile(const QDir &dir, const QString &file);
    //新增接口Url转换，传入订阅的scheme会转换成真实路径进行FileInfo构造
    virtual void setFile(const QUrl &url);
    virtual bool exists() const;
    virtual void refresh();
    virtual QString filePath() const;
    virtual QString absoluteFilePath() const;
    virtual QString canonicalFilePath() const;
    virtual QString fileName() const;
    virtual QString baseName() const;
    virtual QString completeBaseName() const;
    virtual QString suffix() const;
    virtual QString bundleName() const;
    virtual QString completeSuffix();
    virtual QString path() const;
    virtual QString absolutePath() const;
    virtual QString canonicalPath() const;
    virtual QDir dir() const;
    virtual QDir absoluteDir() const;
    //返回真实路径转换的url
    virtual QUrl url() const;
    virtual bool isReadable() const;
    virtual bool isWritable() const;
    virtual bool isExecutable() const;
    virtual bool isHidden() const;
    virtual bool isNativePath() const;
    virtual bool isRelative() const;
    virtual bool isAbsolute() const;
    virtual bool makeAbsolute();
    virtual bool isFile() const;
    virtual bool isDir() const;
    virtual bool isSymLink() const;
    virtual bool isRoot() const;
    virtual bool isBundle() const;
    virtual QString readLink() const;
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
    virtual bool caching() const;
    virtual void setCaching(bool on);
};

typedef QSharedPointer<DAbstractFileInfo> DAbstractFileInfoPointer;

Q_DECLARE_METATYPE(DAbstractFileInfoPointer)

#endif // ABSTRACTFILEINFO_H
