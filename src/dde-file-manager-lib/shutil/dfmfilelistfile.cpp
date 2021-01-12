/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
    bool parseData(const QByteArray &data);
    void setStatus(const DFMFileListFile::Status &newStatus) const;

protected:
    QString dirPath;
    QSet<QString> fileListSet;
    mutable DFMFileListFile::Status status;

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
    QFile file(filePath());

    if (!file.exists()) {
        setStatus(DFMFileListFile::NotExisted);
        return false;
    } else {
        //
    }

    if (!file.open(QFile::ReadOnly)) {
        setStatus(DFMFileListFile::AccessError);
        return false;
    }

    if (file.isReadable() && file.size() != 0) {
        bool ok = false;
        QByteArray data = file.readAll();

        ok = parseData(data);

        if (!ok) {
            setStatus(DFMFileListFile::FormatError);
            return false;
        }
    }

    return true;
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

