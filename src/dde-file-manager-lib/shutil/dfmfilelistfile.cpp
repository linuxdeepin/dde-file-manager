/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
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

#include "dfmfilelistfile.h"
#include "dfmapplication.h"

#include "gvfs/gvfsmountmanager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QSet>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QFileSystemWatcher>

class DFMFileListFilePrivate
{
public:
    DFMFileListFilePrivate(const QString &dirPath, DFMFileListFile *qq);
    ~DFMFileListFilePrivate();

    bool isWritable() const;
    bool write(QIODevice &device) const;

    QString filePath() const;

    bool loadFile();
    bool loadFileWithoutCreateHidden(const QString &path);
    bool loadFileWithCreateHidden(const QString &path);
    bool parseData(const QByteArray &data);
    void setStatus(const DFMFileListFile::Status &newStatus) const;

protected:
    QString dirPath;
    QSet<QString> fileListSet;
    mutable DFMFileListFile::Status status = DFMFileListFile::NoError;

private:
    char __padding[4] = {0};
    DFMFileListFile *q_ptr = nullptr;

    Q_DECLARE_PUBLIC(DFMFileListFile)
};

DFMFileListFilePrivate::DFMFileListFilePrivate(const QString &dirPath, DFMFileListFile *qq)
    : dirPath(dirPath), q_ptr(qq)
{
    loadFile();
}

DFMFileListFilePrivate::~DFMFileListFilePrivate()
{

}

bool DFMFileListFilePrivate::isWritable() const
{
    Q_Q(const DFMFileListFile);

    QFileInfo fileInfo(filePath());

#ifndef QT_NO_TEMPORARYFILE
    if (fileInfo.exists()) {
#endif
        QFile file(q->filePath());
        /*fix bug45741 smb的时候勾选文件夹里面的隐藏文件夹不能生效*/
        return file.open(QFile::WriteOnly);
#ifndef QT_NO_TEMPORARYFILE
    } else {
        // Create the directories to the file.
        QDir dir(fileInfo.absolutePath());
        if (!dir.exists()) {
            if (!dir.mkpath(dir.absolutePath()))
                return false;
        }

        // we use a temporary file to avoid race conditions
        QTemporaryFile file(filePath());
        return file.open();
    }
#endif
}

bool DFMFileListFilePrivate::write(QIODevice &device) const
{
    QStringList lines(fileListSet.toList());
    QString dataStr = lines.join('\n');
    QByteArray data;
    data.append(dataStr);
    qint64 ret = device.write(data);

    if (ret == -1) return false;

    return true;
}

QString DFMFileListFilePrivate::filePath() const
{
    QString strPath = QDir(dirPath).absoluteFilePath(".hidden");
    return strPath;
}

bool DFMFileListFilePrivate::loadFile()
{
    bool autoCreate = false;
    const QString &path = filePath();

    // mtp 文件夹是否显示底部统计信息
    const auto showInfo = dde_file_manager::DFMApplication::genericAttribute(dde_file_manager::DFMApplication::GA_MTPShowBottomInfo).toBool();
    if (!showInfo) {
        static const QString &mtpType = "gvfs/mtp:host";
        autoCreate = path.contains(mtpType);
    }

    // 采用直接访问是否存在方式
    if (!autoCreate) {
        return loadFileWithoutCreateHidden(path);
    } else {
        // mtp 采用自动创建.hidden文件
        return loadFileWithCreateHidden(path);
    }
}

bool DFMFileListFilePrivate::loadFileWithoutCreateHidden(const QString &path)
{
    bool ret = false;

    GFile *gfile = g_file_new_for_path(path.toLocal8Bit().data());
    const bool exist = g_file_query_exists(gfile, nullptr);
    if (exist) {
        // exist, read content
        char *contents = nullptr; //need g_free release memory
        GError *error = nullptr;
        gsize len = 0;
        const bool succ = g_file_load_contents(gfile, nullptr, &contents, &len, nullptr, &error);
        if (succ) {
            if (contents && len > 0) {
                parseData(contents);
            }
            ret = true;
        } else {
            if (error)
                g_error_free(error);
            setStatus(DFMFileListFile::AccessError);
        }
        // Loads the content of the file into memory.
        // The data is always zero-terminated, but this is not included in the resultant length.
        // The returned contents should be freed with g_free() when no longer needed.
        // link : https://docs.gtk.org/gio/method.File.load_contents.html
        g_free(contents);
    } else {
        setStatus(DFMFileListFile::NotExisted);
    }
    g_object_unref(gfile);

    return ret;
}

bool DFMFileListFilePrivate::loadFileWithCreateHidden(const QString &path)
{
    bool ret = false;

    GError *error = nullptr;
    GFile *gfile = g_file_new_for_path(path.toLocal8Bit().data());
    GFileOutputStream *outputStream = g_file_create(gfile, G_FILE_CREATE_NONE, nullptr, &error);
    if (outputStream) {
        // 创建成功了 就表示原来没有 直接析构stream就行
        g_object_unref(outputStream);
    } else {
        // 否则表示创建失败 代表原来文件夹下有hidden文件 准备进行读取
        if (error)
            g_error_free(error);
        // exist, read content
        char *contents = nullptr;
        GError *error = nullptr;
        gsize len = 0;
        bool succ = g_file_load_contents(gfile, nullptr, &contents, &len, nullptr, &error);
        if (succ) {
            if (contents && len > 0) {
                parseData(contents);
            }
            ret = true;
        } else {
            g_error_free(error);
            setStatus(DFMFileListFile::AccessError);
        }
        g_free(contents);
    }
    g_object_unref(gfile);

    return ret;
}

bool DFMFileListFilePrivate::parseData(const QByteArray &data)
{
    QString dataStr(data);
    fileListSet = QSet<QString>::fromList(dataStr.split('\n', QString::SkipEmptyParts));

    return true;
}

void DFMFileListFilePrivate::setStatus(const DFMFileListFile::Status &newStatus) const
{
    if (newStatus == DFMFileListFile::NoError || this->status == DFMFileListFile::NoError) {
        this->status = newStatus;
    }
}


DFMFileListFile::DFMFileListFile(const QString &dirPath, QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMFileListFilePrivate(dirPath, this))

{

}

DFMFileListFile::~DFMFileListFile()
{
    // save on ~ ?
}

QString DFMFileListFile::filePath() const
{
    Q_D(const DFMFileListFile);

    return d->filePath();
}

QString DFMFileListFile::dirPath() const
{
    Q_D(const DFMFileListFile);

    return d->dirPath;
}

bool DFMFileListFile::save() const
{
    Q_D(const DFMFileListFile);

    // write to file.
    if (d->isWritable()) {
        bool ok = false;
        QFileInfo fileInfo(d->dirPath);
        //!使用QFile实现，QSaveFile会导致filewatcher无法监视到.hidden文件改变
        //!导致界面无法实时响应文件显示隐藏
        QFile sf(filePath());
        if (!sf.open(QIODevice::WriteOnly)) {
            d->setStatus(DFMFileListFile::AccessError);
            return false;
        }
        ok = d->write(sf);
        sf.close();
        if (ok) {
            return true;
        } else {
            d->setStatus(DFMFileListFile::AccessError);
            return false;
        }
    }

    return false;
}

bool DFMFileListFile::contains(const QString &fileName) const
{
    Q_D(const DFMFileListFile);

    return d->fileListSet.contains(fileName);
}

void DFMFileListFile::insert(const QString &fileName)
{
    Q_D(DFMFileListFile);

    d->fileListSet.insert(fileName);
}

bool DFMFileListFile::remove(const QString &fileName)
{
    Q_D(DFMFileListFile);

    return d->fileListSet.remove(fileName);
}

QSet<QString> DFMFileListFile::getHiddenFiles()
{
    Q_D(DFMFileListFile);

    return d->fileListSet;
}

// Should we show the "Hide this file" checkbox?
bool DFMFileListFile::supportHideByFile(const QString &fileFullPath)
{
    QFileInfo fileInfo(fileFullPath);
    if (!fileInfo.exists()) return false;
    if (fileInfo.fileName().startsWith('.')) return false;

    return true;
}

// Can user check or uncheck the "Hide this file" checkbox?
bool DFMFileListFile::canHideByFile(const QString &fileFullPath)
{
    QFileInfo fileInfo(fileFullPath);
    QFileInfo dirInfo(fileInfo.absolutePath());

    return dirInfo.isWritable();
}

bool DFMFileListFile::reload()
{
    Q_D(DFMFileListFile);

    d->fileListSet.clear();

    return d->loadFile();
}

