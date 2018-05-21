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

#include <ios>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "fmstatemanager.h"

#include "qobjecthelper.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "shutil/fileutils.h"
#include "shutil/viewstatesmanager.h"
#include "dfileinfo.h"
#include "dfilesystemmodel.h"
#include "interfaces/dfmstandardpaths.h"

#include <QJsonParseError>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QDir>

QMap<DUrl, QPair<int, int>> FMStateManager::SortStates;


///###: this configure record how to show files under specify dir(by last modified time of desending or asending).
static QString config_path{ QString("%1/%2/%3").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::ApplicationSharePath),
                               "config",
                               "default-view-states.json") };


FMStateManager::FMStateManager(QObject *parent)
    : QObject(parent)
    , BaseManager()
{
    m_fmState = new FMState(this);
    initConnect();
}

FMStateManager::~FMStateManager()
{

}

void FMStateManager::initConnect()
{

}

QString FMStateManager::cacheFilePath()
{
//    return QString("%1/%2").arg(StandardPath::getCachePath(), "FMState.json");
    return getConfigPath("fmstate");
}

QString FMStateManager::sortCacheFilePath()
{
//    return QString("%1/%2").arg(StandardPath::getCachePath(), "sort.json");
    return getConfigPath("sort");
}

QPair<int, int> FMStateManager::getRoleAndSortOrderByUrl(const DUrl& url)
{
    QMap<DUrl, QPair<int, int>> url_role_sort_order{ FMStateManager::getUrlsForFilttering() };

    if (url_role_sort_order.contains(url)) {
        QPair<int, int> role_and_sort{ url_role_sort_order[url] };
        return FMStateManager::SortStates.value(url, role_and_sort);
    }

    return FMStateManager::SortStates.value(url, QPair<int, int>{DFileSystemModel::FileDisplayNameRole, Qt::AscendingOrder});
}

void FMStateManager::loadCache()
{
    //Migration for old config files, and rmove that codes for further
    FileUtils::migrateConfigFileFromCache("FMState");

    QString cache = readCacheFromFile(cacheFilePath());
    if (!cache.isEmpty()){
        QObjectHelper::json2qobject(cache, m_fmState);
    }
    loadSortCache();
}


void FMStateManager::saveCache()
{
    QString content = QObjectHelper::qobject2json(m_fmState);
    writeCacheToFile(cacheFilePath(), content);
}

void FMStateManager::loadSortCache()
{
    //Migration for old config files, and rmove that codes for further
    FileUtils::migrateConfigFileFromCache("sort");

    QString cache = readCacheFromFile(sortCacheFilePath());
    if (!cache.isEmpty()){
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(cache.toLocal8Bit(),&error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject obj = doc.object();
            foreach (QString key, obj.keys()) {
                const QStringList &list = obj.value(key).toString().split(",");

                if (list.count() == 2)
                    FMStateManager::SortStates.insert(DUrl(key), qMakePair(list.first().toInt(), list.last().toInt()));
            }
        }else{
            qDebug() << "load cache file: " << sortCacheFilePath() << error.errorString();
        }
    }

    qDebug() << FMStateManager::SortStates;
}

void FMStateManager::saveSortCache()
{
    QVariantMap sortCache;
    foreach (const DUrl& url, FMStateManager::SortStates.keys()) {
        const QPair<int, int> &sort = FMStateManager::SortStates.value(url);

        sortCache.insert(url.toString(), QString("%1,%2").arg(sort.first).arg(sort.second));
    }

    QJsonDocument doc(QJsonObject::fromVariantMap(sortCache));
    writeCacheToFile(sortCacheFilePath(), doc.toJson());

}

void FMStateManager::cacheSortState(const DUrl &url, int role, Qt::SortOrder order)
{
    FMStateManager::SortStates.insert(url, QPair<int, int>(role, order));
    FMStateManager::saveSortCache();
}

QMap<DUrl, QPair<int, int>> FMStateManager::getUrlsForFilttering() noexcept
{
    QMap<DUrl, QPair<int, int>> url_role_sort_order{};

    if (QFileInfo::exists(config_path)) {
        std::basic_ifstream<char> read_json{ config_path.toStdString() };

        if (read_json.is_open() && (read_json.rdstate() == std::ios_base::goodbit
                                    || read_json.rdstate() == std::ios_base::eofbit)) {
            std::istream_iterator<char> input_itr{ read_json };
            std::basic_ostringstream<char> store_content{};
            std::copy(input_itr, std::istream_iterator<char>{}, std::ostream_iterator<char>{store_content});
            std::basic_string<char> content{store_content.str()};
            QJsonParseError error{};
            QJsonDocument json_document{ QJsonDocument::fromJson(QByteArray::fromStdString(content), &error) };

            if (error.error == QJsonParseError::NoError && !json_document.isNull()) {

                if (json_document.isObject()) {
                    QJsonObject json_object{ json_document.object() };
                    QList<QString> keys{ json_object.keys() };

                    for (const QString key : keys) {
                        QJsonValue value{ json_object.take(key) };

                        if (value.isObject()) {
                            QJsonObject object{ value.toObject() };
                            QPair<int, int> role_sort_order{};

                            if (object.contains("sortRole")) {
                                QJsonValue sort_role{ object.take("sortRole") };
                                int int_value{ sort_role.toInt() };
                                role_sort_order.first = int_value;
                            }

                            if (object.contains("sortOrder")) {
                                QJsonValue sort_order{ object.take("sortOrder") };
                                QString sort_order_str{ sort_order.toString() };

                                if (sort_order_str == "DescendingOrder") {
                                    role_sort_order.second = Qt::DescendingOrder;

                                } else if (sort_order_str == "AscendingOrder") {
                                    role_sort_order.second = Qt::AscendingOrder;
                                }
                            }

                            url_role_sort_order[DUrl::fromUserInput(key)] = role_sort_order;
                        }
                    }
                }

            } else {
                qErrnoWarning("Some errors occured, when is opening default-view-states.json.");
            }

        } else {
            qErrnoWarning("Some errors occured, when is opening default-view-states.json.");
        }
    }

    return url_role_sort_order;
}


FMState *FMStateManager::fmState() const
{
    return m_fmState;
}

void FMStateManager::setFmState(FMState *fmState)
{
    m_fmState = fmState;
}

