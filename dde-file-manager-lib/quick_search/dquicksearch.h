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
#include <mutex>
#include <queue>
#include <memory>
#include <atomic>


#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <limits.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "deepin-anything/fs_buf.h"
#include "deepin-anything/walkdir.h"
#include "deepin-anything/index/index.h"
#include "deepin-anything/index/index_allmem.h"

#ifdef __cplusplus
}
#endif //__cplusplus

#include <QObject>


#include "durl.h"
#include "deviceinfo/udisklistener.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "interfaces/udisks2/dfmdiskmanager.h"



class DQuickSearch final : public QObject
{
    Q_OBJECT

public:
    explicit DQuickSearch(QObject *const parent = nullptr);
    DQuickSearch(const DQuickSearch &) = delete;
    DQuickSearch &operator=(const DQuickSearch &) = delete;

    QList<QString> search(const QString &local_path, const QString &key_words);

    void filesWereCreated(const QList<QByteArray> &files_path);
    void filesWereDeleted(const QList<QByteArray> &files_path);
    void filesWereRenamed(const QList<QPair<QByteArray, QByteArray>> &files_path);

    bool createCache();
    inline bool whetherCacheCompletely()const noexcept
    {
        return m_readyFlag.load(std::memory_order_consume);
    }

    static DQuickSearch *instance()noexcept
    {
        static DQuickSearch *quick_search{ new DQuickSearch };
        return quick_search;
    }

    static QPair<QString, QString> getDevAndMountPoint(const QString &local_path);
    static bool isUsbDevice(const QString &dev_path);
    static bool isFiltered(const DUrl &path);


    ///###: These APIs are temporarily useless, under.
    static bool is_auto_indexes_inner()
    {
        return true;
    }

    static bool is_auto_indexes_removable()
    {
        return true;
    }
    ///###

    static bool store_adler32_value(const QString &mount_point, const std::size_t &value)noexcept;
    static std::size_t read_adler32_value(const QString &mount_point)noexcept;
    static std::size_t count_adler32(const QString &mount_point)noexcept;

public slots:
    ///###: These APIs are temporarily useless, under.
    void onMountAdded(const QString &blockDevicePath, const QByteArray &mountPoint);
    void onMountRemoved(const QString &blockDevicePath, const QByteArray &mountPoint);
    ///###


    ///###: These APIs are temporarily useless, under.
    void onAutoInnerIndexesOpened();
    void onAutoInnerIndexesClosed();
    void onAutoRemovableIndexesOpened();
    void onAutoRemovableIndexesClosed();
    ///###

private:
    void cache_every_partion();
    void initialize_connection()noexcept;
    bool create_lft(const QString &mount_point);

    ///###: it is standby.
    static QList<QString> filter_result(const QList<QString> &searched_result, const QByteArray &regex);



    std::atomic<bool> m_readyFlag{ false };
    std::mutex m_mutex{};
    std::atomic<bool> m_flag{ false };
    std::deque<QString> m_backup{};
    std::map<QString, QString> m_mount_point_and_lft_buf{};

    std::basic_regex<char> m_wildcard_char{};

    std::unique_ptr<dde_file_manager::DFMDiskManager> m_disk_manager{ nullptr };
};
