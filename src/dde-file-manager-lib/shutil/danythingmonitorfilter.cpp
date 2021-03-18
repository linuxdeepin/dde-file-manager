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

#include <map>
#include <memory>
#include <utility>
#include <functional>

#include "dfmsettings.h"
#include "dfmapplication.h"
#include "danythingmonitorfilter.h"

#include <QDir>
#include <QList>
#include <QDebug>
#include <QString>
#include <QFileInfo>
#include <QJsonObject>
#include <QTextStream>


namespace detail {

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


static QString restoreEscapedChar(const QString &value)
{
    QString tempValue{ value };

    if (!tempValue.isEmpty() && !tempValue.isNull()) {
        std::map<QString, QString>::const_iterator table_beg{ StrTableOfEscapeChar.cbegin() };
        std::map<QString, QString>::const_iterator table_end{ StrTableOfEscapeChar.cend() };

        for (; table_beg != table_end; ++table_beg) {
            int pos = tempValue.indexOf(table_beg->first);

            if (pos != -1) {

                while (pos != -1) {
                    tempValue = tempValue.replace(table_beg->first, table_beg->second);
                    pos = tempValue.indexOf(table_beg->first);
                }
            }
        }
    }

    return tempValue;
}

}


class DAnythingMonitorFilterPrivate
{
public:

    explicit DAnythingMonitorFilterPrivate(DAnythingMonitorFilter *const q_q);
    ~DAnythingMonitorFilterPrivate() = default;
    DAnythingMonitorFilterPrivate(const DAnythingMonitorFilterPrivate &) = delete;
    DAnythingMonitorFilterPrivate &operator=(const DAnythingMonitorFilterPrivate &) = delete;

    QList<DUrl> whiteList();
    QList<DUrl> blackList();

    ///###: jundge whether the url should be monitored.
    bool whetherFilterThePath(const QByteArray &local_path);

    void read_setting();
    void get_home_path_of_all_users();

    DAnythingMonitorFilter *q_ptr{ nullptr };

    std::map<QString, QString> m_user_name_and_home_path{};
    std::unique_ptr<QList<QString>> m_black_list{ nullptr };
    std::unique_ptr<QList<QString>> m_white_list{ nullptr };
    std::unique_ptr<dde_file_manager::DFMSettings> m_fm_setting{ nullptr };
};

DAnythingMonitorFilterPrivate::DAnythingMonitorFilterPrivate(DAnythingMonitorFilter *const q_q)
    : q_ptr{ q_q },
      m_black_list{ new QList<QString> },
m_white_list{ new QList<QString> },
m_fm_setting{ dde_file_manager::DFMApplication::genericSetting() }
{
    this->read_setting();
}

bool DAnythingMonitorFilterPrivate::whetherFilterThePath(const QByteArray &local_path)
{
    QString local_file{ QString::fromLocal8Bit(local_path) };

    //从数据盘进入主目录的路径在判断之前需要先处理成/home路径
    if (local_file.startsWith("/data/home/"))
        local_file.remove(0, sizeof("/data") - 1);

    bool result{ false };

    for (const QString &path : *m_white_list) {

        if (local_file == path || local_file.startsWith(path)) {
            result = true;
        }
    }

    for (const QString &path : *m_black_list) {

        if (local_file == path || local_file.startsWith(path)) {
            result = false;
        }
    }

    return result;
}

void DAnythingMonitorFilterPrivate::get_home_path_of_all_users()
{
    if (QFileInfo::exists("/etc/passwd")) {
        QFile i_file{"/etc/passwd"};

        if (!i_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qFatal("Can not open /etc/passwd!");
            return;
        }

        QTextStream in(&i_file);

        while (!in.atEnd()) {
            QString line_content{ in.readLine() };
            if (line_content.isEmpty()) continue;
            QList<QString> contents{ line_content.split(':') };
            if (contents.size() < 2) {
                continue;
            }
            QString path_remove_ESC{ detail::restoreEscapedChar(contents[contents.size() - 2]) };
            m_user_name_and_home_path[contents[0]] = path_remove_ESC;
        }
    }
}


void DAnythingMonitorFilterPrivate::read_setting()
{

    QVariant white_list{ m_fm_setting->value("AnythingMonitorFilterPath", "WhiteList") };
    QVariant black_list{ m_fm_setting->value("AnythingMonitorFilterPath", "BlackList") };
    *m_white_list = white_list.toStringList();
    *m_black_list = black_list.toStringList();
    QList<QString> invalid_path{};
    QList<QString> removed_tilde{};


#ifdef QT_DEBUG
    qDebug() << "white-list:" << *m_white_list;
    qDebug() << "black-list: " << *m_black_list;
#endif //QT_DEBUG

    this->get_home_path_of_all_users();

    for (QString &path : *m_black_list) {

        if (path.startsWith("~/")) {
            removed_tilde.push_back(path);
            continue;
        }
    }

    for (const QString &path : removed_tilde) {
        m_black_list->removeAll(path);
    }

    for (QString &path : removed_tilde) {
        path.remove(0, 1);

        for (const std::pair<QString, QString> &user_name_and_home_path : m_user_name_and_home_path) {
            m_black_list->push_back(user_name_and_home_path.second + path);
        }
    }

    for (const QString &path : *m_black_list) {

        if (!QFileInfo::exists(path)) {
            invalid_path.push_back(path);
        }
    }

    for (const QString &path : invalid_path) {
        m_black_list->removeAll(path);
    }

    invalid_path.clear();
    ///###: above this.
    ///###: remove '~' in configure. And replace '~' by the home path of every user.
    ///###: remove invalid path in the black-list.



    ///###: remove invalid path in white-list.
    ///###: make sure every in black-list is subdirectory of white-list.
    for (const QString &path : *m_white_list) {

        if (!QFileInfo::exists(path)) {
            invalid_path.push_back(path);
        }
    }

    for (const QString &path : invalid_path) {
        m_white_list->removeAll(path);
    }

    QList<QString>::const_iterator itr_beg{};
    QList<QString>::const_iterator itr_end{};
    invalid_path.clear();

    ///###: here, start to jundge whether directories in black-list are subdirectories in white-list.
    for (const QString &dir_in_black_list : *m_black_list) {
        itr_beg = m_white_list->cbegin();
        itr_end = m_white_list->cend();

        for (; itr_beg != itr_end; ++itr_beg) {

            if (dir_in_black_list.startsWith(*itr_beg)) {
                break;
            }
        }

        if (itr_beg == itr_end) {
            invalid_path.push_back(dir_in_black_list);
        }
    }

    for (const QString &path : invalid_path) {
        m_black_list->removeAll(path);
    }


    ///###:at last, maybe there are files in configure. So delete these.
    invalid_path.clear();

    std::function<void(std::unique_ptr<QList<QString>>& list)> reserve_dir{
        [](std::unique_ptr<QList<QString>> &list)
        {
            std::list<QString> path_invalid{};

            for (const QString &path : *list) {
                QFileInfo file_info{ path };

                if (!file_info.isDir()) {
                    path_invalid.push_back(path);
                }
            }

            for (const QString &path : path_invalid) {
                list->removeAll(path);
            }
        }
    };

    reserve_dir(std::ref(m_white_list));
    reserve_dir(std::ref(m_black_list));


#ifdef QT_DEBUG
    qDebug() << "white-list: " << *m_white_list;
    qDebug() << "black-list: " << *m_black_list;
#endif //QT_DEBUG
}

DAnythingMonitorFilter::DAnythingMonitorFilter(QObject *const parent)
    : QObject{ parent },
      d_ptr{ new DAnythingMonitorFilterPrivate{this} }
{
    //###:constructor.
}

DAnythingMonitorFilter::~DAnythingMonitorFilter()
{
    //###: destructor. must be defined in .cpp.
    //###: if not, QScopedPointer can not use a incomplete type as it's type.
}

bool DAnythingMonitorFilter::whetherFilterCurrentPath(const QByteArray &local_path)
{
    DAnythingMonitorFilterPrivate *const d{ d_func() };
    return d->whetherFilterThePath(local_path);
}

