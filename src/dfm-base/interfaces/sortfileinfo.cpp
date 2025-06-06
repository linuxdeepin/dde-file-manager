// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/private/sortfileinfo_p.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <QtConcurrent>
#include <QMutexLocker>
#include <sys/stat.h>

namespace dfmbase {

SortFileInfo::SortFileInfo()
    : d(new SortFileInfoPrivate(this))
{
}

SortFileInfo::~SortFileInfo()
{
}

void SortFileInfo::setUrl(const QUrl &url)
{
    d->url = url;
}

void SortFileInfo::setSize(const qint64 size)
{
    d->filesize = size;
}

void SortFileInfo::setFile(const bool isfile)
{
    d->file = isfile;
}

void SortFileInfo::setDir(const bool isdir)
{
    d->dir = isdir;
}

void SortFileInfo::setSymlink(const bool isSymlink)
{
    d->symLink = isSymlink;
}

void SortFileInfo::setHide(const bool ishide)
{
    d->hide = ishide;
}

void SortFileInfo::setReadable(const bool readable)
{
    d->readable = readable;
}

void SortFileInfo::setWriteable(const bool writeable)
{
    d->writeable = writeable;
}

void SortFileInfo::setExecutable(const bool executable)
{
    d->executable = executable;
}

void SortFileInfo::setLastReadTime(const qint64 time)
{
    d->lastRead = time;
}

void SortFileInfo::setLastModifiedTime(const qint64 time)
{
    d->lastModifed = time;
}

void SortFileInfo::setCreateTime(const qint64 time)
{
    d->create = time;
}

void SortFileInfo::setDisplayType(const QString &displayType)
{
    d->displayType = displayType;
}

void SortFileInfo::setHighlightContent(const QString &content)
{
    d->highlightContent = content;
}

QString SortFileInfo::highlightContent() const
{
    return d->highlightContent;
}

QUrl SortFileInfo::fileUrl() const
{
    return d->url;
}

qint64 SortFileInfo::fileSize() const
{
    return d->filesize;
}

bool SortFileInfo::isFile() const
{
    return d->file;
}

bool SortFileInfo::isDir() const
{
    return d->dir;
}

bool SortFileInfo::isSymLink() const
{
    return d->symLink;
}

bool SortFileInfo::isHide() const
{
    return d->hide;
}

bool SortFileInfo::isReadable() const
{
    return d->readable;
}

bool SortFileInfo::isWriteable() const
{
    return d->writeable;
}

bool SortFileInfo::isExecutable() const
{
    return d->executable;
}

qint64 SortFileInfo::lastReadTime() const
{
    return d->lastRead;
}

qint64 SortFileInfo::lastModifiedTime() const
{
    return d->lastModifed;
}

qint64 SortFileInfo::createTime() const
{
    return d->create;
}

QString SortFileInfo::displayType() const
{
    return d->displayType;
}

// 信息完整性相关方法
void SortFileInfo::setInfoCompleted(const bool completed)
{
    QMutexLocker locker(&d->mutex);
    d->infoCompleted = completed;
}

void SortFileInfo::markAsCompleted()
{
    setInfoCompleted(true);
}

bool SortFileInfo::isInfoCompleted() const
{
    QMutexLocker locker(&d->mutex);
    return d->infoCompleted;
}

bool SortFileInfo::needsCompletion() const
{
    return !isInfoCompleted();
}

// 新增：文件信息补全接口
bool SortFileInfo::completeFileInfo()
{
    if (isInfoCompleted()) {
        return true;  // 已经完成，无需重复获取
    }
    return d->doCompleteFileInfo();
}

QFuture<bool> SortFileInfo::completeFileInfoAsync()
{
    if (isInfoCompleted()) {
        // 如果已经完成，返回一个立即完成的 Future
        QPromise<bool> promise;
        promise.start();
        promise.addResult(true);
        promise.finish();
        return promise.future();
    }
    
    return QtConcurrent::run([this]() {
        return d->doCompleteFileInfo();
    });
}

QFuture<bool> SortFileInfo::completeFileInfoAsync(CompletionCallback callback)
{
    if (isInfoCompleted()) {
        if (callback) {
            callback(true);
        }
        QPromise<bool> promise;
        promise.start();
        promise.addResult(true);
        promise.finish();
        return promise.future();
    }
    
    return QtConcurrent::run([this, callback]() {
        bool success = d->doCompleteFileInfo();
        if (callback) {
            callback(success);
        }
        return success;
    });
}

SortFileInfoPrivate::SortFileInfoPrivate(SortFileInfo *qq)
    : q(qq)
{
}

SortFileInfoPrivate::~SortFileInfoPrivate()
{
}

bool SortFileInfoPrivate::doCompleteFileInfo()
{
    if (!url.isLocalFile()) {
        return false;
    }
    
    struct stat64 statBuffer;
    const QString filePath = url.path();
    
    if (::stat64(filePath.toUtf8().constData(), &statBuffer) != 0) {
        return false;
    }
    
    QMutexLocker locker(&mutex);
    
    // 一次性设置所有从 stat64 获取的信息
    
    // 基础信息
    filesize = statBuffer.st_size;
    file = S_ISREG(statBuffer.st_mode);
    dir = S_ISDIR(statBuffer.st_mode);
    symLink = S_ISLNK(statBuffer.st_mode);
    
    // 权限信息
    readable = statBuffer.st_mode & S_IRUSR;
    writeable = statBuffer.st_mode & S_IWUSR;
    executable = statBuffer.st_mode & S_IXUSR;
    
    // 时间信息
    lastRead = statBuffer.st_atime;
    lastModifed = statBuffer.st_mtime;
    create = statBuffer.st_ctime;
    
    // 隐藏文件检查
    QString fileName = url.fileName();
    hide = fileName.startsWith('.');
    
    // 设置 MIME 类型显示名称（这个不需要额外的文件系统调用）
    displayType = MimeTypeDisplayManager::instance()->displayTypeFromPath(url.path());
    
    // 标记所有信息已完成
    infoCompleted = true;
    
    return true;
}

}
