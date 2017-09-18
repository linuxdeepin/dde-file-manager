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

#include <QDebug>
#include <QFileInfo>


#include "filebatchprocess.h"


std::once_flag FileBatchProcess::flag;


QSharedMap<DUrl, DUrl> FileBatchProcess::replaceText(const QList<DUrl>& originUrls, const QPair<QString, QString> &pair) const
{
    if(originUrls.isEmpty()){ //###: here, jundge whether there are fileUrls in originUrls.
        return QSharedMap<DUrl, DUrl>{ nullptr };
    }

    QSharedMap<DUrl, DUrl> result{ new QMap<DUrl, DUrl>{}};

    for(auto url : originUrls){

        QFileInfo info{ url.toLocalFile() };

        ///###: symlink is also processed here.
        if(info.isFile()){
            QString fileNameStr{ info.baseName() + QString{"."} + info.completeSuffix() }; //###: baseName + completeSuffix
            fileNameStr.replace(pair.first, pair.second);

            DUrl changedUrl{DUrl::fromLocalFile(info.path() + QString{"/"} + fileNameStr)};
            result->insert(url, changedUrl);
            continue;
        }

        if(info.isDir()){
            QString dirNameStr{ info.baseName() };
            dirNameStr.replace(pair.first, pair.second);
            DUrl changedUrl{DUrl::fromLocalFile(info.path() + QString{"/"} + dirNameStr)};
            result->insert(url, changedUrl);
            continue;
        }
    }

    return result;
}

QSharedMap<DUrl, DUrl> FileBatchProcess::addText(const QList<DUrl> &originUrls, const QPair<QString, DFileService::AddTextFlags>& pair) const
{
    if(originUrls.isEmpty()){  //###: here, jundge whether there are fileUrls in originUrls.
        return QSharedMap<DUrl, DUrl>{ nullptr };
    }

    QSharedMap<DUrl, DUrl> result{new QMap<DUrl, DUrl>{}};

    for(auto url : originUrls){
        QFileInfo info(url.toLocalFile());
        QString baseNameStr{  info.baseName() };

        if(pair.second == DFileService::AddTextFlags::Before){

            baseNameStr.push_front(pair.first);

            if(info.isFile()){
                DUrl changedUrl{ DUrl::fromLocalFile(info.path() + QString{"/"} + baseNameStr + QString{"."} + info.completeSuffix()) };
                result->insert(url, changedUrl);
                continue;
            }

            if(info.isDir()){
                DUrl changedUrl{ DUrl::fromLocalFile(info.path() + QString{"/"} + baseNameStr) };
                result->insert(url, changedUrl);
                continue;
            }


        }else if(pair.second == DFileService::AddTextFlags::After){

            baseNameStr.push_back(pair.first);

            if(info.isFile()){
                DUrl changedUrl{DUrl::fromLocalFile(info.path() + QString{"/"} + baseNameStr + QString{"."} + info.completeSuffix())};
                result->insert(url, changedUrl);
                continue;
            }

            if(info.isDir()){
                DUrl changedUrl{ DUrl::fromLocalFile(info.path() + QString{"/"} + baseNameStr) };
                result->insert(url, changedUrl);
                continue;
            }

        }
    }


    return result;
}

QSharedMap<DUrl, DUrl> FileBatchProcess::customText(const QList<DUrl> &originUrls, const QPair<QString, std::size_t> &pair) const
{
    if(originUrls.isEmpty()){  //###: here, jundge whether there are fileUrls in originUrls.
        return QSharedMap<DUrl, DUrl>{ nullptr };
    }

    std::size_t index{ pair.second };
    QSharedMap<DUrl, DUrl> result{new QMap<DUrl, DUrl>{}};
    for(auto url : originUrls){
        QFileInfo info{url.toLocalFile()};

        if(info.isFile()){
            QString fileNameStr{ info.path() + QString{"/"} + pair.first + QString::fromStdString(std::to_string(index)) +
                                 QString{"." } + info.completeSuffix() };
            DUrl changedUrl{ DUrl::fromLocalFile(fileNameStr) };
            result->insert(url, changedUrl);
            ++index;
            continue;
        }

        if(info.isDir()){
            QString fileNameStr{ info.path() + QString{"/"} + pair.first + QString::fromStdString(std::to_string(index)) };
            DUrl changedUrl{ DUrl::fromLocalFile(fileNameStr) };
            result->insert(url, changedUrl);
            ++index;
            continue;
        }

    }

    return result;
}



////###: use the value of map to rename the file who name is the key of map.
QSharedPointer<QList<DUrl>> FileBatchProcess::batchProcessFile(const QSharedMap<DUrl, DUrl> &map)
{
    if(static_cast<bool>(map) == false){  //###: here, jundge whether there are fileUrls in map.
        return QSharedPointer<QList<DUrl>>{ nullptr };
    }


    QMap<DUrl, DUrl>::const_iterator beg = map->constBegin();
    QMap<DUrl, DUrl>::const_iterator end = map->constEnd();

    QSharedPointer<QList<DUrl>> cache{ new QList<DUrl>{} };

    for(; beg != end; ++beg){

        QString originName{ beg.key().toLocalFile() };
        QString changedName{ beg.value().toLocalFile() };

        if(originName == changedName){
            continue;
        }

       QString currentName{ beg.key().toLocalFile() };
       QString hopedName{ beg.value().toLocalFile() };

       ///###: just cache files that rename successfully.
       if( QFile::rename(currentName, hopedName) == true ){
           cache->push_back( DUrl::fromLocalFile(hopedName) );
       }
    }

    return cache;
}




