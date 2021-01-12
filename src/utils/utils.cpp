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

#include <QStandardPaths>
#include <QSvgRenderer>
#include <QPainter>
#include <QProcess>
#include <QGuiApplication>
#include "utils.h"
#include "utils.h"
#include "dmimedatabase.h"
#include "interfaces/dfmglobal.h"


DFM_USE_NAMESPACE

QString getThumbnailsPath(){
    QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
    QString thumbnailPath = joinPath(cachePath, "thumbnails");
    if (!QDir(thumbnailPath).exists()){
        QDir(thumbnailPath).mkpath(thumbnailPath);
    }
    return thumbnailPath;
}

QString decodeUrl(QString url){
    if (!url.startsWith(FilePrefix)){
        url = FilePrefix + url;
        url = QUrl(url).toLocalFile();
    }else{
        url = QUrl(url).toLocalFile();
    }
    return url;
}

QString deleteFilePrefix(QString path){
    QString ret = path.replace(FilePrefix, "");
    return decodeUrl(ret);
}

bool isDesktop(QString url){
    if (url.startsWith(FilePrefix)){
        url.replace(FilePrefix, "");
    }
    if (url == desktopLocation){
        return true;
    }
    return false;
}

bool isAppGroup(QString url){
    if (url.startsWith(FilePrefix)){
        url.replace(FilePrefix, "");
    }
    QFileInfo f(url);
    if (decodeUrl(f.path()) == desktopLocation){
        if (f.fileName().startsWith(RichDirPrefix)){
            return true;
        }
    }
    return false;
}


bool isApp(QString url){
    if (url.endsWith(AppSuffix)){
        return true;
    }
    return false;
}

bool isAllApp(QStringList urls){
    if (urls.length() > 0){
        bool ret = true;
        foreach (QString url, urls) {
            ret = ret && isApp(url);
            if (!ret){
                return false;
            }
        }
        return ret;
    }else{
        return false;
    }
}

bool isComputer(QString url){
    if (url == ComputerUrl){
        return true;
    }
    return false;
}

bool isTrash(QString url){
    if (url == TrashUrl){
        return true;
    }
    return false;
}


bool isFolder(QString url){
    url.replace(FilePrefix, "");
    if (QDir(url).exists()){
        if (decodeUrl(url).contains(desktopLocation)){
            if (QDir(url).dirName().startsWith(RichDirPrefix)){
                return false;
            }
        }
        return true;
    }
    return false;
}

bool isInDesktop(QString url){
    if (decodeUrl(QFileInfo(url).path()) == desktopLocation){
        return true;
    }else if (QFileInfo(url).path() == desktopLocation){
        return true;
    }else if (url.contains(desktopLocation)){
        return true;
    }
    return false;
}

bool isInDesktopFolder(QString url){
    if (decodeUrl(QFileInfo(url).path()) == desktopLocation){
        return true;
    }else if (QFileInfo(url).path() == desktopLocation){
        return true;
    }
    return false;
}


bool isDesktopAppFile(QString url){
    if (QFileInfo(url).fileName().endsWith(AppSuffix)){
        return true;
    }
    return false;
}

bool isRequestThumbnail(QString url){
    QString mimetypeName = getMimeTypeName(url);
    if (SupportMimeTypes.contains(mimetypeName)){
        return true;
    }else{
        return false;
    }
}

QString getMimeTypeGenericIconName(QString url){
    DMimeDatabase mimeDataBae;
    QMimeType mimeType = mimeDataBae.mimeTypeForFile(deleteFilePrefix(url));
    return mimeType.genericIconName();
}

QString getMimeTypeIconName(QString url){
    DMimeDatabase mimeDataBae;
    QMimeType mimeType = mimeDataBae.mimeTypeForFile(deleteFilePrefix(url));
    return mimeType.iconName();
}

QString getMimeTypeName(QString url){
    DMimeDatabase mimeDataBae;
    QMimeType mimeType = mimeDataBae.mimeTypeForFile(deleteFilePrefix(url));
    return mimeType.name();
}


QString getQssFromFile(QString filename)
{
    QFile f(filename);
    QString qss = "";
    if (f.open(QFile::ReadOnly))
    {
        qss = QLatin1String(f.readAll());
        f.close();
    }
    return qss;
}

QByteArray joinPath(const QByteArray& path, const QByteArray &fileName){
    return path + QDir::separator().toLatin1() + fileName;
}

QPixmap svgToPixmap(const QString &path, int w, int h)
{
    QPixmap pixmap(w, h);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);

    renderer.render(&painter);

    painter.end();

    return pixmap;
}

QPixmap svgToHDPIPixmap(const QString &path, int w, int h)
{
    qreal ratio = qApp->devicePixelRatio();
    return svgToPixmap(path, w * ratio, h * ratio);
}

QPixmap svgToHDPIPixmap(const QString &path)
{
    QPixmap p(path);
    qreal ratio = qApp->devicePixelRatio();
    return svgToPixmap(path, p.width() * ratio, p.height() * ratio);
}

QString joinPath(const QString &path, const QString &fileName)
{
    return path + QDir::separator() + fileName;
}

bool isAvfsMounted()
{
    QProcess p;
    QString cmd = "/bin/bash";
    QStringList args;
    args << "-c" << "ps -ax -o 'cmd'|grep '.avfs$'";
    p.start(cmd, args);
    p.waitForFinished();
    QString result = p.readAll().trimmed();
    if(!result.isEmpty()){
        QStringList datas = result.split(" ");

        if(datas.count() == 2){
            if(datas.at(0) == "avfsd" && QFile::exists(datas.at(1)))
                return true;
        }

    }
    return false;
}
