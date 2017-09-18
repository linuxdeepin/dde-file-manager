/*
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

#include "filessizeworker.h"
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDebug>


FilesSizeWorker::FilesSizeWorker(const DUrlList &urls, QObject *parent) :

    QObject(parent)
{
    m_urls = urls;
}

FilesSizeWorker::~FilesSizeWorker()
{

}

void FilesSizeWorker::coumpueteSize()
{
    foreach (DUrl url, m_urls) {
        if (stopped()){
            return;
        }
        QString filePath = url.toLocalFile();
        QFileInfo targetInfo(filePath);
        if (targetInfo.exists()){
            if (targetInfo.isDir()){
                QDir d(filePath);
                QFileInfoList entryInfoList = d.entryInfoList(QDir::AllEntries | QDir::System
                            | QDir::NoDotAndDotDot | QDir::NoSymLinks
                            | QDir::Hidden);
                foreach (QFileInfo file, entryInfoList) {
                    if (file.isFile()){
                        if (stopped()){
                            return;
                        }
                        m_size += file.size();
                        updateSize();
                    }
                    else {
                        if (stopped()){
                            return;
                        }
                        if (file.fileName() == ".avfs"){
                            continue;
                        }
                        QDirIterator it(file.absoluteFilePath(), QDir::AllEntries | QDir::System
                                      | QDir::NoDotAndDotDot | QDir::NoSymLinks
                                      | QDir::Hidden, QDirIterator::Subdirectories);
                        while (it.hasNext()) {
                            if (stopped()){
                                return;
                            }
                            it.next();
                            m_size += it.fileInfo().size();
                            updateSize();
                        }
                    }
                }
            }else{
                if (stopped()){
                    return;
                }
                m_size += targetInfo.size();
            }
        }
        emit sizeUpdated(m_size);
    }
}

void FilesSizeWorker::updateSize()
{
    emit sizeUpdated(m_size);
}

void FilesSizeWorker::stop()
{
    setStopped(true);
    setSize(0);
}
qint64 FilesSizeWorker::size() const
{
    return m_size;
}

void FilesSizeWorker::setSize(const qint64 &size)
{
    m_size = size;
}

bool FilesSizeWorker::stopped() const
{
    return m_stopped;
}

void FilesSizeWorker::setStopped(bool stopped)
{
    m_stopped = stopped;
}

DUrlList FilesSizeWorker::urls() const
{
    return m_urls;
}

void FilesSizeWorker::setUrls(const DUrlList &urls)
{
    m_urls = urls;
}


