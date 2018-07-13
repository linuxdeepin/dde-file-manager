/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     shihua <tangtong@deepin.com>
 *
 * Maintainer: shihua <tangtong@deepin.com>
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

#include <regex>
#include <thread>
#include <string>
#include <fstream>

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

#include <regex.h>

#ifdef __cplusplus
}
#endif //__cplusplus

#include "dfileinfo.h"
#include "dquicksearch.h"
#include "shutil/dquicksearchfilter.h"


#include <QDebug>



#define MAX_RESULTS 100

#define ACT_NEW_FILE    0
#define ACT_NEW_LINK    1
#define ACT_NEW_SYMLINK 2
#define ACT_NEW_FOLDER  3
#define ACT_DEL_FILE    4
#define ACT_DEL_FOLDER  5
#define ACT_RENAME_FILE     6
#define ACT_RENAME_FOLDER   7




#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

int match_regex(const char *name, void *query)
{
    regmatch_t subs[1024];
    memset(subs, 0, sizeof(subs));
    regex_t *compiled = (regex_t *)query;
    return (regexec(compiled, name, 1024, subs, 0) == REG_NOERROR) ? 1 : 0;
}


#ifdef __cplusplus
}
#endif //__cplusplus





static constexpr const std::size_t buffer_size{ (1 << 24) };
static constexpr const std::size_t MAXPARTIONSIZE{ 99 };

namespace detail
{

static const std::map<QString, QString> StrTableOfEscapeChar{
    {"\\007", "\a"},
    {"\\010", "\b"},
    {"\\014", "\f"},
    {"\\012", "\n"},
    {"\\015", "\r"},
    {"\\011", "\t"},
    {"\\013", "\v"},
    {"\\134", "\\"},
    {"\\047", "\'"},
    {"\\042", "\""},
    {"\\040", " "}
};



static QString restore_escaped_char(const QString &value)
{
    QString tempValue{ value };

    if (!tempValue.isEmpty() && !tempValue.isNull()) {

        int pos{ -1 };
        std::map<QString, QString>::const_iterator tableCBeg{ StrTableOfEscapeChar.cbegin() };
        std::map<QString, QString>::const_iterator tableCEnd{ StrTableOfEscapeChar.cend() };

        for (; tableCBeg != tableCEnd; ++tableCBeg) {
            pos = tempValue.indexOf(tableCBeg->first);

            if (pos != -1) {

                while (pos != -1) {
                    tempValue = tempValue.replace(tableCBeg->first, tableCBeg->second);

                    pos = tempValue.indexOf(tableCBeg->first);
                }
            }
        }
    }

    return tempValue;
}


static std::deque<partition> get_all_partition()
{
    std::deque<partition> partitions_deque{};
    int partion_count{ 0 };
    partition partitions[MAXPARTIONSIZE] {};

    if (get_partitions(&partion_count, partitions) != 0) {
        qFatal("can not get the partitions!");
        return partitions_deque;
    }

    for (int index = 0; index != partion_count; ++index) {
        partitions_deque.push_back(std::move(partitions[index]));
    }

    return partitions_deque;
}


static std::multimap<QString, QString>  query_partions_of_devices()
{
    std::multimap<QString, QString> partionsAndMountpoints{};
    std::deque<partition> partitions_deque{ get_all_partition() };

    for (const partition &partion : partitions_deque) {
        QString dev_str{ partion.dev };
        QString mount_point{ partion.mount_point };
        mount_point = restore_escaped_char(mount_point);

        if (!dev_str.isEmpty() && !mount_point.isEmpty()) {
            partionsAndMountpoints.emplace(dev_str, mount_point);
        }
    }

    return partionsAndMountpoints;
}


///###: get the specify url's partion and device.
static QPair<QString, QString> get_mount_point_of_file(const QString &local_path, std::multimap<QString, QString> &partionsAndMountPoints)
{
    QPair<QString, QString> partionAndMountPoint{};

    if (QFileInfo::exists(local_path) && !partionsAndMountPoints.empty()) {
        QString parent_path{};

        if (local_path == QString{"/"} || local_path == QString{"//"}) {
            parent_path = QString{"/"};

        } else {
            int index{ local_path.lastIndexOf("/") };
            parent_path = local_path.left(index);
        }

#ifdef QT_DEBUG
        qDebug() << parent_path;
#endif //QT_DEBUG

        std::multimap<QString, QString>::const_iterator partionAndMounpointItr{};
        std::multimap<QString, QString>::const_iterator rootPathPartionAndMountpointItr{};

        std::multimap<QString, QString>::const_iterator itrOfPartionAndMountpoint{ partionsAndMountPoints.cbegin() };
        std::multimap<QString, QString>::const_iterator itrOfPartionAndMountpointEnd{ partionsAndMountPoints.cend() };

        for (; itrOfPartionAndMountpoint != itrOfPartionAndMountpointEnd; ++itrOfPartionAndMountpoint) {

            if (itrOfPartionAndMountpoint->second == QString{ "/" }) {
                rootPathPartionAndMountpointItr = itrOfPartionAndMountpoint;
            }
        }

        itrOfPartionAndMountpoint = partionsAndMountPoints.cbegin();
        itrOfPartionAndMountpointEnd = partionsAndMountPoints.cend();

        for (; itrOfPartionAndMountpoint != itrOfPartionAndMountpointEnd; ++itrOfPartionAndMountpoint) {

            if (itrOfPartionAndMountpoint->second != QString{ "/" } && parent_path.startsWith(itrOfPartionAndMountpoint->second)) {
                partionAndMounpointItr = itrOfPartionAndMountpoint;
                break;
            }
        }

        if (partionAndMounpointItr != std::multimap<QString, QString>::const_iterator{}) {
            partionAndMountPoint.first = partionAndMounpointItr->first;
            partionAndMountPoint.second = partionAndMounpointItr->second;
        }


        if (partionAndMounpointItr == std::multimap<QString, QString>::const_iterator{} && parent_path.startsWith("/")) {
            partionAndMountPoint.first = rootPathPartionAndMountpointItr->first;
            partionAndMountPoint.second = rootPathPartionAndMountpointItr->second;
        }
    }

    return partionAndMountPoint;
}




template<std::size_t size>
static std::shared_ptr<std::pair<std::queue<partition>, std::queue<partition>>> removable_inner_partion(const partition(& partions)[size])
{
    std::queue<partition> usb_partions{};
    std::queue<partition> inner_partions{};

    for (std::size_t index = 0; index < size; ++index) {
        QString dev_path{ partions[index].dev };

        if (!dev_path.isEmpty()) {

            if (DQuickSearch::isUsbDevice(partions[index].dev)) {
                usb_partions.push(partions[index]);
                continue;
            }

            inner_partions.push(partions[index]);
        }
    }

    std::shared_ptr<std::pair<std::queue<partition>, std::queue<partition>>> the_partions{
        new std::pair<std::queue<partition>, std::queue<partition>>{
            std::move(usb_partions), std::move(inner_partions)
        }
    };

    return the_partions;
}

}

DQuickSearch::DQuickSearch(QObject *const parent)
    : QObject{ parent }
{
}


QList<QString> DQuickSearch::search(const QString &local_path, const QString &key_words)
{
    QList<QString> searched_list{};

#ifdef QT_DEBUG
    qDebug() << local_path << key_words;
#endif //QT_DEBUG

    if (QFileInfo::exists(local_path) && !key_words.isEmpty()) {
        std::multimap<QString, QString> devices_and_mount_points{ detail::query_partions_of_devices() };
        QPair<QString, QString> device_and_mount_point{ detail::get_mount_point_of_file(local_path, devices_and_mount_points) };
        std::map<QString, QString>::const_iterator pos{ m_mount_point_and_lft_buf.find(device_and_mount_point.second) };

        if (pos != m_mount_point_and_lft_buf.cend()) {
            fs_buf *buf{ nullptr };
            load_fs_buf(&buf, pos->second.toLocal8Bit().constData());

            if (buf) {
                QByteArray query_str{ key_words.toLocal8Bit() };
                QByteArray local_path_8bit{ local_path.toLocal8Bit() };
                std::uint32_t path_off{ 0 };
                std::uint32_t end_off{ 0 };
                std::uint32_t start_off{ 0 };
                regex_t *compiled;
                int err = regcomp(compiled, query_str.constData(), REG_ICASE | REG_EXTENDED);

#ifdef QT_DEBUG
                qDebug() << local_path_8bit;
#endif //QT_DEBUG

                get_path_range(buf, local_path_8bit.data(), &path_off,  &start_off, &end_off);

                end_off = end_off == 0 ? get_tail(buf) : end_off;
                start_off = start_off == 0 ? first_name(buf) : start_off;
                std::uint32_t name_offs[MAX_RESULTS] {};
                std::uint32_t count{ MAX_RESULTS };

#ifdef QT_DEBUG
                qDebug() << start_off << end_off << path_off;
#endif //QT_DEBUG

                if (!err) {

                    search_files(buf, &start_off, end_off, compiled, match_regex, name_offs, &count);

                    char path[PATH_MAX];
                    for (std::uint32_t i = 0; i < count; i++) {
                        char *file_or_dir_name{ get_path_by_name_off(buf, name_offs[i], path, sizeof(path)) };

                        if (!DQuickSearchFilter::instance()->whetherFilterCurrentFile(QByteArray{ file_or_dir_name })) {
                            searched_list.push_back(QString{ file_or_dir_name });
                        }
                    }

                    std::vector<std::uint32_t> vec_names_off{};
                    std::uint32_t total = count;

                    while (count == 100) {
                        std::uint32_t names_off[100] {};
                        search_files(buf, &start_off, end_off, compiled, match_regex, name_offs, &count);

                        for (std::size_t index = 0; index < 100; ++index) {
                            vec_names_off.push_back(names_off[index]);
                        }

                        total += count;
                    }

                    std::vector<std::uint32_t>::const_iterator off_beg{ vec_names_off.cbegin() };

                    for (std::uint32_t index = count; index < total; ++index) {
                        char *file_or_dir_name{ get_path_by_name_off(buf, *off_beg, path, sizeof(path)) };

                        if (!DQuickSearchFilter::instance()->whetherFilterCurrentFile(QByteArray{ file_or_dir_name })) {
                            searched_list.push_back(QString{ file_or_dir_name });
                        }

                        ++off_beg;

#ifdef QT_DEBUG
                        qDebug() << file_or_dir_name;
#endif //QT_DEBUG
                    }

                    return searched_list;
                }

                free_fs_buf(buf);
            }
        }
    }

    m_readyFlag.store(false, std::memory_order_release);
    return searched_list;
}

void DQuickSearch::filesWereCreated(const QList<QByteArray> &files_path)
{
    if (m_readyFlag.load(std::memory_order_consume)) {
        return;
    }

    if (files_path.isEmpty()) {
        fs_change changes[10] {}; //###:temporarily useless. Maybe forever.
        std::multimap<QString, QString> devices_and_mount_points{ detail::query_partions_of_devices() };

        std::lock_guard<std::mutex> raii_lock{ m_mutex };

        for (const QByteArray &path : files_path) {
            QString path_str{ QString::fromLocal8Bit(path) };
            QFileInfo file_info{ path_str };

            if (m_flag.load(std::memory_order_consume)) {
                devices_and_mount_points = detail::query_partions_of_devices();
                m_flag.store(false, std::memory_order_release);
            }

            QPair<QString, QString> device_and_mount_point{ detail::get_mount_point_of_file(path, devices_and_mount_points) };
            std::map<QString, QString>::const_iterator pos{ m_mount_point_and_lft_buf.find(device_and_mount_point.second) };

            if (pos == m_mount_point_and_lft_buf.cend()) {
                continue;
            }

            fs_buf *buf{ nullptr };
            std::basic_string<char> local_8bit{ path.toStdString() };
            load_fs_buf(&buf, pos->second.toLocal8Bit().constData());

            if (buf) {

                if (file_info.isSymLink()) {
                    insert_path(buf, const_cast<char *>(local_8bit.data()), ACT_NEW_SYMLINK, changes);
                    continue;
                }

                if (file_info.isFile()) {
                    insert_path(buf, const_cast<char *>(local_8bit.data()), ACT_NEW_FILE, changes);
                    continue;
                }

                if (file_info.isDir()) {
                    insert_path(buf, const_cast<char *>(local_8bit.data()), ACT_NEW_FOLDER, changes);
                    continue;
                }

                free_fs_buf(buf);
            }
        }
    }
}


void DQuickSearch::filesWereDeleted(const QList<QByteArray> &files_path)
{
    if (m_readyFlag.load(std::memory_order_consume)) {
        return;
    }

    if (files_path.isEmpty()) {
        fs_change changes[10] {};
        std::uint32_t change_count{  sizeof(changes) / sizeof(fs_change) };
        std::multimap<QString, QString> devices_and_mount_points{ detail::query_partions_of_devices() };

        std::lock_guard<std::mutex> raii_lock{ m_mutex };

        for (const QByteArray &path : files_path) {
            QString path_str{ QString::fromLocal8Bit(path) };

            if (m_flag.load(std::memory_order_consume)) {
                devices_and_mount_points = detail::query_partions_of_devices();
                m_flag.store(false, std::memory_order_release);
            }

            QPair<QString, QString> device_and_mount_point{ detail::get_mount_point_of_file(path, devices_and_mount_points) };
            std::map<QString, QString>::const_iterator pos{ m_mount_point_and_lft_buf.find(device_and_mount_point.second) };


            if (pos == m_mount_point_and_lft_buf.cend()) {
                continue;
            }

            fs_buf *buf{ nullptr };
            std::basic_string<char> local_8bit{ path.toStdString() };
            load_fs_buf(&buf, pos->second.toLocal8Bit().constData());

            if (buf) {
                remove_path(buf, const_cast<char *>(local_8bit.data()), changes, &change_count);
                free_fs_buf(buf);
            }
        }
    }
}

void DQuickSearch::filesWereRenamed(const QList<QPair<QByteArray, QByteArray> > &files_path)
{
    if (m_readyFlag.load(std::memory_order_consume)) {
        return;
    }

    if (files_path.isEmpty()) {
        fs_change changes[10] {};
        std::uint32_t change_count{  sizeof(changes) / sizeof(fs_change) };
        std::multimap<QString, QString> devices_and_mount_points{ detail::query_partions_of_devices() };

        std::lock_guard<std::mutex> raii_lock{ m_mutex };

        for (const QPair<QByteArray, QByteArray> &old_and_new_name : files_path) {
            QString path_str{ QString::fromLocal8Bit(old_and_new_name.second) };

            if (m_flag.load(std::memory_order_consume)) {
                devices_and_mount_points = detail::query_partions_of_devices();
                m_flag.store(false, std::memory_order_release);
            }

            QPair<QString, QString> device_and_mount_point{ detail::get_mount_point_of_file(path_str, devices_and_mount_points) };
            std::map<QString, QString>::const_iterator pos{ m_mount_point_and_lft_buf.find(device_and_mount_point.second) };

            if (pos == m_mount_point_and_lft_buf.cend()) {
                continue;
            }

            std::basic_string<char> old_name{ old_and_new_name.first.toStdString() };
            std::basic_string<char> new_name{ old_and_new_name.second.toStdString() };

            fs_buf *buf{ nullptr };
            load_fs_buf(&buf, pos->second.toLocal8Bit().constData());

            if (buf) {
                rename_path(buf, const_cast<char *>(old_name.data()), const_cast<char *>(new_name.data()), changes, &change_count);
                free_fs_buf(buf);
            }
        }
    }
}

bool DQuickSearch::createCache()
{
    if (!m_readyFlag.load(std::memory_order_consume)) {
        std::function<void(DQuickSearch *)> func_for_creating_cache{ &DQuickSearch::cache_every_partion };
        std::thread thread_for_creating_cache{ func_for_creating_cache, DQuickSearch::instance() };
        thread_for_creating_cache.detach();

        return false;
    }

    return true;
}

QPair<QString, QString> DQuickSearch::getDevAndMountPoint(const QString &local_path)
{
    std::multimap<QString, QString> devices_and_mount_points{ detail::query_partions_of_devices() };
    QPair<QString, QString> device_and_mount_point{ detail::get_mount_point_of_file(local_path, devices_and_mount_points) };

    return device_and_mount_point;
}


void DQuickSearch::onMountAdded(const QString &blockDevicePath, const QByteArray &mountPoint)
{
    m_flag.store(true, std::memory_order_release);
    QString mount_point{ detail::restore_escaped_char(mountPoint) };

    DUrl mount_url{ DUrl::fromLocalFile(mount_point) };

    std::lock_guard<std::mutex> raii_lock{ m_mutex };

    if (isFiltered(mount_url)) {
        return;
    }

    if (!blockDevicePath.isEmpty()) {

        if (DQuickSearch::is_auto_indexes_inner() && DQuickSearch::is_auto_indexes_removable()) {

            if (!create_lft(mount_point)) {
                qWarning() << "A error occured, when creating lft in: " << mount_point;
            }

            return;
        }

        if (DQuickSearch::isUsbDevice(blockDevicePath)) {

            if (DQuickSearch::is_auto_indexes_removable()) {

                if (!create_lft(mount_point)) {
                    qWarning() << "A error occured, when creating lft in: " << mount_point;
                }
            }

            return;
        }

        if (!DQuickSearch::isUsbDevice(blockDevicePath)) {

            if (DQuickSearch::is_auto_indexes_removable()) {

                if (!create_lft(mountPoint)) {
                    qWarning() << "A error occured, when creating lft in: " << mount_point;
                }
            }
        }
    }
}

void DQuickSearch::onMountRemoved(const QString &blockDevicePath, const QByteArray &mountPoint)
{
    (void)blockDevicePath;

    m_flag.store(true, std::memory_order_release);
    QString mount_point{ detail::restore_escaped_char(mountPoint) };

    std::lock_guard<std::mutex> raii_lock{ m_mutex };

    m_mount_point_and_lft_buf.erase(mount_point);
}

void DQuickSearch::onAutoInnerIndexesOpened()
{
    std::lock_guard<std::mutex> raii_lock{ m_mutex };
    int partion_count{ 0 };
    partition partitions[MAXPARTIONSIZE] {};

    if (get_partitions(&partion_count, partitions) != 0) {
        qFatal("can not get the partitions!");
        return;
    }

    std::shared_ptr<std::pair<std::queue<partition>, std::queue<partition>>> removable_and_inner_partions{
        detail::removable_inner_partion(partitions)
    };

    while (!removable_and_inner_partions->second.empty()) {
        const partition &top_element = removable_and_inner_partions->second.front();
        QString mount_point{ top_element.mount_point };

        if (QFileInfo::exists(mount_point)) {
            std::deque<QString>::const_iterator mount_point_pos{
                std::find(m_backup.cbegin(), m_backup.cend(), mount_point)
            };

            if (mount_point_pos != m_backup.cend()) {
                fs_buf *buf{ nullptr };
                QByteArray lft_file{ mount_point.toLocal8Bit() };

                if (lft_file == QByteArray{"/"}) {
                    lft_file += QByteArray{ ".__deepin.lft" };

                } else {
                    lft_file += QByteArray{ "/.__deepin.lft" };
                }

                int code{ load_fs_buf(&buf, lft_file.constData()) };

                if (code == 0 && buf != nullptr) {
                    m_mount_point_and_lft_buf.emplace(mount_point, QString::fromLocal8Bit(lft_file));
                    free_fs_buf(buf);
                }

            } else {

                if (!create_lft(mount_point)) {
                    qWarning() << "A error occured, when creating lft in: " << mount_point;
                }
            }
        }

        removable_and_inner_partions->second.pop();
    }
}

void DQuickSearch::onAutoInnerIndexesClosed()
{
    std::lock_guard<std::mutex> raii_lock{ m_mutex };
    int partion_count{ 0 };
    partition partitions[MAXPARTIONSIZE] {};

    if (get_partitions(&partion_count, partitions) != 0) {
        qFatal("can not get the partitions!");
        return;
    }

    std::shared_ptr<std::pair<std::queue<partition>, std::queue<partition>>> removable_and_inner_partions{
        detail::removable_inner_partion(partitions)
    };

    while (!removable_and_inner_partions->second.empty()) {
        const partition &top_element = removable_and_inner_partions->second.front();
        QString mount_point{ top_element.mount_point };

        if (QFileInfo::exists(mount_point)) {
            std::map<QString, QString>::iterator pos{ m_mount_point_and_lft_buf.find(mount_point) };

            if (pos != m_mount_point_and_lft_buf.cend()) {
                m_backup.push_back(pos->first);
                m_mount_point_and_lft_buf.erase(pos->first);
                m_backup.push_back(pos->first);
            }
        }

        removable_and_inner_partions->second.pop();
    }
}

void DQuickSearch::onAutoRemovableIndexesOpened()
{
    std::lock_guard<std::mutex> raii_lock{ m_mutex };
    int partion_count{ 0 };
    partition partitions[MAXPARTIONSIZE] {};

    if (get_partitions(&partion_count, partitions) != 0) {
        qFatal("can not get the partitions!");
        return;
    }

    std::shared_ptr<std::pair<std::queue<partition>, std::queue<partition>>> removable_and_inner_partions{
        detail::removable_inner_partion(partitions)
    };

    while (!removable_and_inner_partions->first.empty()) {
        const partition &top_element = removable_and_inner_partions->first.front();
        QString mount_point{ top_element.mount_point };

        if (QFileInfo::exists(mount_point)) {
            std::deque<QString>::const_iterator mount_point_pos{
                std::find(m_backup.cbegin(), m_backup.cend(), mount_point)
            };

            if (mount_point_pos != m_backup.cend()) {
                fs_buf *buf{ nullptr };
                QByteArray lft_file{ mount_point.toLocal8Bit() };

                if (lft_file == QByteArray{"/"}) {
                    lft_file += QByteArray{ ".__deepin.lft" };

                } else {
                    lft_file += QByteArray{ "/.__deepin.lft" };
                }

                int code{ load_fs_buf(&buf, lft_file.constData()) };

                if (code == 0 && buf != nullptr) {
                    m_mount_point_and_lft_buf.emplace(mount_point, QString::fromLocal8Bit(lft_file));
                    free_fs_buf(buf);
                }

            } else {

                if (!create_lft(mount_point)) {
                    qWarning() << "A error occured, when creating lft in: " << mount_point;
                }
            }
        }

        removable_and_inner_partions->first.pop();
    }
}

void DQuickSearch::onAutoRemovableIndexesClosed()
{
    std::lock_guard<std::mutex> raii_lock{ m_mutex };
    int partion_count{ 0 };
    partition partitions[MAXPARTIONSIZE] {};

    if (get_partitions(&partion_count, partitions) != 0) {
        qFatal("can not get the partitions!");
        return;
    }

    std::shared_ptr<std::pair<std::queue<partition>, std::queue<partition>>> removable_and_inner_partions{
        detail::removable_inner_partion(partitions)
    };

    while (!removable_and_inner_partions->first.empty()) {
        const partition &top_element = removable_and_inner_partions->first.front();
        QString mount_point{ top_element.mount_point };

        if (QFileInfo::exists(mount_point)) {
            std::map<QString, QString>::iterator pos{ m_mount_point_and_lft_buf.find(mount_point) };

            if (pos != m_mount_point_and_lft_buf.cend()) {
                m_backup.push_back(pos->first);
                m_mount_point_and_lft_buf.erase(pos->first);
            }
        }

        removable_and_inner_partions->first.pop();
    }
}

void DQuickSearch::initialize_connection() noexcept
{
}


void DQuickSearch::cache_every_partion()
{
    int partion_count{ 0 };
    partition partitions[MAXPARTIONSIZE] {};

    if (get_partitions(&partion_count, partitions) != 0) {
        qFatal("can not get the partitions!");
        return;
    }

    ///###: for debugging.
//    for (int index = 0; index != partion_count; ++index) {
//        qDebug() << partitions[index].dev << partitions[index].mount_point << partitions[index].fs_type
//                 << partitions[index].major << partitions[index].minor;
//    }

    std::function<void()> change_status_flag{
        [this]{
            bool flag{ false };
            m_readyFlag.compare_exchange_strong(flag, true, std::memory_order_release);
        }
    };

    if (partion_count > 0) {
        std::lock_guard<std::mutex> raii_lock{ m_mutex }; //###: locked!

        if (is_auto_indexes_inner() && is_auto_indexes_removable()) {

            for (int index = 0; index < partion_count; ++index) {
                QString mount_point{ detail::restore_escaped_char(partitions[index].mount_point) };

                if (QFileInfo::exists(mount_point)) {

                    if (!isFiltered(DUrl::fromLocalFile(mount_point))) {

                        if (!create_lft(mount_point)) {
                            qWarning() << "A error occured, when creating lft in: " << mount_point;
                        }
                    }
                }
            }

            change_status_flag();

            return;
        }

        std::shared_ptr<std::pair<std::queue<partition>, std::queue<partition>>> usb_and_inner_partion{
            detail::removable_inner_partion(partitions)
        };

        if (is_auto_indexes_inner() && !is_auto_indexes_removable()) {

            while (!usb_and_inner_partion->second.empty()) {
                partition &partion = usb_and_inner_partion->second.front();

                if (DFileInfo::exists(DUrl::fromLocalFile(partion.mount_point))) {

                    if (!isFiltered(DUrl::fromLocalFile(partion.mount_point))) {

                        if (!create_lft(partion.mount_point)) {
                            qWarning() << "A error occured, when creating lft in: " << partion.mount_point;
                        }
                    }

                    usb_and_inner_partion->second.pop();
                }
            }

            change_status_flag();

            return;
        }

        if (!is_auto_indexes_inner() && is_auto_indexes_removable()) {

            while (!usb_and_inner_partion->first.empty()) {
                partition &partion = usb_and_inner_partion->first.front();

                if (DFileInfo::exists(DUrl::fromLocalFile(partion.mount_point))) {

                    if (isFiltered(DUrl::fromLocalFile(partion.mount_point))) {

                        if (!create_lft(partion.mount_point)) {
                            qWarning() << "A error occured, when creating lft in: " << partion.mount_point;
                        }
                    }

                    usb_and_inner_partion->first.pop();
                }
            }

            change_status_flag();

            return;
        }
    }

}


///###: jundge whether the specify partition is a usb device.
bool DQuickSearch::isUsbDevice(const QString &dev_path)
{
    struct stat buf;
    char link[512] {};
    char link_path[512] {};

    if (lstat(dev_path.toStdString().c_str(), &buf) < 0) {
        return false;
    }

#ifdef QT_DEBUG
    qDebug() << "dev" << buf.st_dev;
#endif //QT_DEBUG

    if (S_ISBLK(buf.st_mode)) {
        sprintf(link_path, "/sys/dev/block/%d:%d", major(buf.st_rdev), minor(buf.st_rdev));

        if (access(link_path, F_OK) >= 0) {
            ssize_t num_of_reading{ readlink(link_path, link, 512) };

            if (num_of_reading == 0) {
                return false;
            }

            std::string strlink{ link };
            std::size_t pos{ strlink.find("usb", 0) };

            if (pos == std::string::npos) {
                return false;
            }

            return true;
        }
    }

    return false;
}

bool DQuickSearch::isFiltered(const DUrl &path)
{
    QByteArray local8bit_str{ path.toLocalFile().toLocal8Bit() };
    bool result{ DQuickSearchFilter::instance()->whetherFilterCurrentFile(local8bit_str) };

    return result;
}

bool DQuickSearch::create_lft(const QString &mount_point)
{
    if (!mount_point.isEmpty()) {
        std::basic_string<char> file_located{ mount_point.toStdString() };
        std::basic_string<char> full_path{ mount_point.toStdString() };

        if (file_located != std::basic_string<char> {"/"}) {
            file_located += std::basic_string<char> { "/.__deepin.lft" };
            full_path += std::basic_string<char> { "/" };
        } else {
            file_located = std::basic_string<char> { "/.__deepin.lft" };
        }

        fs_buf *buffer = new_fs_buf(buffer_size, full_path.c_str());

        if (buffer) {
            build_fstree(buffer, 0, NULL, NULL);

            if (save_fs_buf(buffer, file_located.c_str()) == 0) {
                m_mount_point_and_lft_buf[mount_point] = QString::fromStdString(file_located);

                return true;
            }
        }
    }

    return false;
}

QList<QString> DQuickSearch::filter_result(const QList<QString> &searched_result, const QByteArray &regex)
{
    QList<QString> result{};

    if (!searched_result.isEmpty() && !regex.isEmpty()) {
        std::basic_regex<char> regex_temp{ regex.constData(), std::basic_regex<char>::grep }; //###: support grep grammar.
        std::match_results<QByteArray::const_iterator> matched_result{};

        for (const QString &str : searched_result) {
            QByteArray u8_str{ str.toLocal8Bit() };

            if (std::regex_search(u8_str.cbegin(), u8_str.cend(), matched_result, regex_temp)) {
                result.push_back(str);
            }
        }
    }

    return result;
}




