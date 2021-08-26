/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "private/dabstractfileinfo_p.h"

#include "dabstractfileinfo.h"

#include <QMetaType>

Q_GLOBAL_STATIC_WITH_ARGS(int,type_id,{qRegisterMetaType<DAbstractFileInfoPointer>("DAbstractFileInfo")});

DAbstractFileInfo::DAbstractFileInfo()
    :d_ptr(new DAbstractFileInfoPrivate(this))
{
    Q_UNUSED(type_id)
}

DAbstractFileInfo::DAbstractFileInfo(const QUrl &url)
    :d_ptr(new DAbstractFileInfoPrivate(this))
{
    Q_D(DAbstractFileInfo);
    d->fileInfo = QFileInfo(DFMUrlRoute::urlToPath(url));
    d->url = url;
}

DAbstractFileInfo::DAbstractFileInfo(const QString &file)
    :d_ptr(new DAbstractFileInfoPrivate(this))
{
    Q_D(DAbstractFileInfo);
    d->fileInfo.setFile(file);
    d->url = DFMUrlRoute::pathToUrl(d->fileInfo.filePath());
}

DAbstractFileInfo::DAbstractFileInfo(const QFile &file)
    :d_ptr(new DAbstractFileInfoPrivate(this))
{
    Q_D(DAbstractFileInfo);
    d->fileInfo.setFile(file);
    d->url = DFMUrlRoute::pathToUrl(d->fileInfo.filePath());
}

DAbstractFileInfo::DAbstractFileInfo(const QDir &dir, const QString &file)
    :d_ptr(new DAbstractFileInfoPrivate(this))
{
    Q_D(DAbstractFileInfo);
    d->fileInfo = QFileInfo(dir,file);
    d->url = DFMUrlRoute::pathToUrl(d->fileInfo.filePath());
}

DAbstractFileInfo::DAbstractFileInfo(const QFileInfo &fileinfo)
    :d_ptr(new DAbstractFileInfoPrivate(this))
{
    Q_D(DAbstractFileInfo);
    d->fileInfo = fileinfo;
    d->url = DFMUrlRoute::pathToUrl(d->fileInfo.filePath());
}

DAbstractFileInfo::~DAbstractFileInfo()
{

}

DAbstractFileInfo &DAbstractFileInfo::operator=(const DAbstractFileInfo &fileinfo)
{
    Q_D(DAbstractFileInfo);
    d->fileInfo = fileinfo.d_ptr->fileInfo;
    return *this;
}

bool DAbstractFileInfo::operator==(const DAbstractFileInfo &fileinfo) const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo == fileinfo.d_ptr->fileInfo;
}

bool DAbstractFileInfo::operator!=(const DAbstractFileInfo &fileinfo) const
{
    return !(operator==(fileinfo));
}

void DAbstractFileInfo::setFile(const QString &file)
{
    Q_D(DAbstractFileInfo);
    d->fileInfo.setFile(file);
    d->url = DFMUrlRoute::pathToUrl(d->fileInfo.filePath());
}

void DAbstractFileInfo::setFile(const QFile &file)
{
    Q_D(DAbstractFileInfo);
    d->fileInfo.setFile(file);
    d->url = DFMUrlRoute::pathToUrl(d->fileInfo.filePath());
}

void DAbstractFileInfo::setFile(const QDir &dir, const QString &file)
{
    Q_D(DAbstractFileInfo);
    d->fileInfo.setFile(dir,file);
    d->url = DFMUrlRoute::pathToUrl(d->fileInfo.filePath());
}

void DAbstractFileInfo::setFile(const QUrl &url)
{
    Q_D(DAbstractFileInfo);
    d->fileInfo = QFileInfo(DFMUrlRoute::urlToPath(url));
    d->url = url;
    this->refresh();
}

bool DAbstractFileInfo::exists() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.exists();
}

void DAbstractFileInfo::refresh()
{
    Q_D(DAbstractFileInfo);
    return d->fileInfo.refresh();
}

QString DAbstractFileInfo::filePath() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.filePath();
}

QString DAbstractFileInfo::absoluteFilePath() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.absoluteFilePath();
}

QString DAbstractFileInfo::canonicalFilePath() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.canonicalFilePath();
}

QString DAbstractFileInfo::fileName() const
{
    Q_D(const DAbstractFileInfo);

    if (d->fileInfo.isFile())
        return d->fileInfo.fileName();
    if (d->fileInfo.isDir()) {
        auto list = d->fileInfo.filePath().split("/");
        auto itera = list.end();
        while (itera != list.begin()) {
            -- itera;
            if (!itera->isEmpty())
                return *itera;
        }
    }
    return "";
}

QString DAbstractFileInfo::baseName() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.baseName();
}

QString DAbstractFileInfo::completeBaseName() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.completeBaseName();
}

QString DAbstractFileInfo::suffix() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.suffix();
}

QString DAbstractFileInfo::bundleName() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.bundleName();
}

QString DAbstractFileInfo::completeSuffix()
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.completeSuffix();
}

QString DAbstractFileInfo::path() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.path();
}

QString DAbstractFileInfo::absolutePath() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.absolutePath();
}

QString DAbstractFileInfo::canonicalPath() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.canonicalPath();
}

QDir DAbstractFileInfo::dir() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.dir();
}

QDir DAbstractFileInfo::absoluteDir() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.absoluteDir();
}

QUrl DAbstractFileInfo::url() const
{
    return DFMUrlRoute::pathToUrl(filePath());
}

bool DAbstractFileInfo::isReadable() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isReadable();
}

bool DAbstractFileInfo::isWritable() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isWritable();
}

bool DAbstractFileInfo::isExecutable() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isExecutable();
}

bool DAbstractFileInfo::isHidden() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isHidden();
}

bool DAbstractFileInfo::isNativePath() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isNativePath();
}

bool DAbstractFileInfo::isRelative() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isRelative();
}

bool DAbstractFileInfo::isAbsolute() const
{
    return !isRelative();
}

bool DAbstractFileInfo::makeAbsolute()
{
    Q_D(DAbstractFileInfo);
    return d->fileInfo.makeAbsolute();
}

bool DAbstractFileInfo::isFile() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isFile();
}

bool DAbstractFileInfo::isDir() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isDir();
}

bool DAbstractFileInfo::isSymLink() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isSymLink();
}

bool DAbstractFileInfo::isRoot() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isRoot();
}

bool DAbstractFileInfo::isBundle() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.isBundle();
}

QString DAbstractFileInfo::readLink() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.readLink();
}

QString DAbstractFileInfo::symLinkTarget() const
{
    return readLink();
}

QString DAbstractFileInfo::owner() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.owner();
}

uint DAbstractFileInfo::ownerId() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.ownerId();
}

QString DAbstractFileInfo::group() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.group();
}

uint DAbstractFileInfo::groupId() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.groupId();
}

bool DAbstractFileInfo::permission(QFileDevice::Permissions permissions) const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.permission(permissions);
}

QFileDevice::Permissions DAbstractFileInfo::permissions() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.permissions();
}

qint64 DAbstractFileInfo::size() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.size();
}

QDateTime DAbstractFileInfo::created() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.created();
}

QDateTime DAbstractFileInfo::birthTime() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.birthTime();
}

QDateTime DAbstractFileInfo::metadataChangeTime() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.metadataChangeTime();
}

QDateTime DAbstractFileInfo::lastModified() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.metadataChangeTime();
}

QDateTime DAbstractFileInfo::lastRead() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.lastRead();
}

QDateTime DAbstractFileInfo::fileTime(QFileDevice::FileTime time) const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.fileTime(time);
}

bool DAbstractFileInfo::caching() const
{
    Q_D(const DAbstractFileInfo);
    return d->fileInfo.caching();
}

void DAbstractFileInfo::setCaching(bool on)
{
    Q_D(DAbstractFileInfo);
    return d->fileInfo.setCaching(on);
}

DAbstractFileInfoPrivate::DAbstractFileInfoPrivate(DAbstractFileInfo *qq)
    : q_ptr(qq)
{

}

DAbstractFileInfoPrivate::~DAbstractFileInfoPrivate() {

}
