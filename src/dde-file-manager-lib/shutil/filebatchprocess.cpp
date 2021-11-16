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

#include "dfmglobal.h"
#include "filebatchprocess.h"
#include "dfmeventdispatcher.h"

#include <QDebug>
#include <QByteArray>

std::once_flag FileBatchProcess::flag;
const static QString MIMETYPE_APP_DESKTOP = "application/x-desktop";

QSharedMap<DUrl, DUrl> FileBatchProcess::replaceText(const QList<DUrl>& originUrls, const QPair<QString, QString> &pair) const
{
    if(originUrls.isEmpty() == true) { //###: here, judge whether there are fileUrls in originUrls.
        return QSharedMap<DUrl, DUrl>{ nullptr };
    }

    QSharedMap<DUrl, DUrl> result{ new QMap<DUrl, DUrl>{}};

    for(auto url : originUrls){
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(nullptr, url);

        if (!info)
            continue;


        // debug case 25414: failure to rename desktop app name
        bool isDeskTopApp = info->mimeTypeName().contains(MIMETYPE_APP_DESKTOP);

        ///###: symlink is also processed here.
        QString fileBaseName = isDeskTopApp? info->fileDisplayName(): info->baseName();
        const QString &suffix = info->suffix().isEmpty() ? QString() : QString(".") + info->suffix();
        fileBaseName.replace(pair.first, pair.second);

        if (fileBaseName.trimmed().isEmpty()) {
            qWarning() << "replace fileBaseName(not include suffix) trimmed is empty string";
            continue;
        }

        int max_length = MAX_FILE_NAME_CHAR_COUNT - suffix.toLocal8Bit().size();

        if (fileBaseName.toLocal8Bit().size() > max_length) {
            fileBaseName = DFMGlobal::cutString(fileBaseName, max_length, QTextCodec::codecForLocale());
        }

        if(!isDeskTopApp){
            fileBaseName += suffix;
        }
        DUrl changedUrl{ info->getUrlByNewFileName(fileBaseName) };

        if (changedUrl != url)
            result->insert(url, changedUrl);
    }

    return result;
}

QSharedMap<DUrl, DUrl> FileBatchProcess::addText(const QList<DUrl> &originUrls, const QPair<QString, DFileService::AddTextFlags>& pair) const
{
    if(originUrls.isEmpty()){  //###: here, judge whether there are fileUrls in originUrls.
        return QSharedMap<DUrl, DUrl>{ nullptr };
    }

    QSharedMap<DUrl, DUrl> result{ new QMap<DUrl, DUrl>{} };

    for (auto url : originUrls) {
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(nullptr, url);

        if (!info)
            continue;

        // debug case 25414: failure to rename desktop app name
        bool isDeskTopApp = info->mimeTypeName().contains(MIMETYPE_APP_DESKTOP);

        QString fileBaseName = isDeskTopApp? info->fileDisplayName(): info->baseName();//{ info->baseName() };
        QString oldFileName = fileBaseName;

        QString add_text = pair.first;
        const QString &suffix = info->suffix().isEmpty() ? QString() : QString(".") + info->suffix();
        int max_length = MAX_FILE_NAME_CHAR_COUNT - info->fileName().toLocal8Bit().size();

        if (add_text.toLocal8Bit().size() > max_length) {
            add_text = DFMGlobal::cutString(add_text, max_length, QTextCodec::codecForLocale());
        }

        if (pair.second == DFileService::AddTextFlags::Before) {
            fileBaseName.insert(0, add_text);
        } else {
            fileBaseName.append(add_text);
        }

        if(!isDeskTopApp){
            fileBaseName += suffix;
        }
        DUrl changedUrl = { info->getUrlByNewFileName(fileBaseName) };

        if(isDeskTopApp) {
            qDebug()<<"this is desktop app case,file name will be changed { "<< oldFileName << " } to { " << fileBaseName << " } for path:" << info->fileUrl();
        }

        if (changedUrl != url)
            result->insert(url, changedUrl);
    }

    return result;
}

QSharedMap<DUrl, DUrl> FileBatchProcess::customText(const QList<DUrl> &originUrls, const QPair<QString, QString> &pair) const
{
    if(originUrls.isEmpty() == true || pair.first.isEmpty() == true || pair.second.isEmpty() == true){  //###: here, jundge whether there are fileUrls in originUrls.
        return QSharedMap<DUrl, DUrl>{ nullptr };
    }

    unsigned long long SNNumber{ pair.second.toULongLong() };
    unsigned long long index{ 0 };

    if (SNNumber == ULONG_LONG_MAX) {  //##: Maybe, this value will be equal to the max value of the type of unsigned long long
        index = SNNumber - originUrls.size();
    } else {
        index = SNNumber;
    }

    QSharedMap<DUrl, DUrl> result{new QMap<DUrl, DUrl>{}};

    QList<DUrl> modifyUrls;

    bool needRecombination = false;
    for (auto url : originUrls) {
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(nullptr, url);

        if (!info)
            continue;

        // debug case 25414: failure to rename desktop app name
        bool isDeskTopApp = info->mimeTypeName().contains(MIMETYPE_APP_DESKTOP);

        QString fileBaseName{ pair.first };
        const QString &index_string = QString::number(index);
        const QString &suffix = info->suffix().isEmpty() ? QString() : QString(".") + info->suffix();
        int max_length = MAX_FILE_NAME_CHAR_COUNT - index_string.toLocal8Bit().size() - suffix.toLocal8Bit().size();

        if (fileBaseName.toLocal8Bit().size() > max_length) {
            fileBaseName = DFMGlobal::cutString(fileBaseName, max_length, QTextCodec::codecForLocale());
        }

        fileBaseName = isDeskTopApp ? (fileBaseName + index_string) : (fileBaseName + index_string + suffix);
        DUrl beModifieddUrl = { info->getUrlByNewFileName(fileBaseName) };
        result->insert(url, beModifieddUrl);

        modifyUrls << beModifieddUrl;

        // 如果源url包含了待修改的url 就需要重组结果
        if (originUrls.contains(beModifieddUrl))
            needRecombination = true;

        if(isDeskTopApp) {
            qDebug()<<"this is desktop app case,file name will be changed as { "<< fileBaseName << " } for path:" << info->fileUrl();
        }

        ++index;
    }

    // 重组map
    if (needRecombination) {
        QList<DUrl> originUrlsTemp = originUrls;

        auto it = modifyUrls.begin();
        while (it != modifyUrls.end()) {
            DUrl url = *it;
            if (originUrlsTemp.contains(url)) {
                originUrlsTemp.removeOne(url);
                it = modifyUrls.erase(it);
                continue;
            }
            ++it;
        }

        if (originUrlsTemp.size() == modifyUrls.size()) {
            result.reset(new QMap<DUrl, DUrl>{});
            for (int i = 0, end = originUrlsTemp.size(); i < end; ++i) {
                result->insert(originUrlsTemp[i], modifyUrls[i]);
            }
        }
    }

    return result;
}



////###: use the value of map to rename the file who name is the key of map.
QMap<DUrl, DUrl> FileBatchProcess::batchProcessFile(const QSharedMap<DUrl, DUrl> &map)
{
    QMap<DUrl, DUrl> cache;

    if (static_cast<bool>(map) == false) {  //###: here, jundge whether there are fileUrls in map.
        return cache;
    }

    QMap<DUrl, DUrl>::const_iterator beg = map->constBegin();
    QMap<DUrl, DUrl>::const_iterator end = map->constEnd();

    // 实现批量回退
    DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>();

    for (; beg != end; ++beg) {
        DUrl currentName{ beg.key() };
        DUrl hopedName{ beg.value() };

        if (currentName == hopedName) {
            continue;
        }

       ///###: just cache files that rename successfully.
       if (DFileService::instance()->renameFile(nullptr, currentName, hopedName) == true ) {
           cache[currentName] = hopedName;
       }
    }

    // 实现批量回退
    DFMEventDispatcher::instance()->processEvent<DFMSaveOperatorEvent>();

    return cache;
}
