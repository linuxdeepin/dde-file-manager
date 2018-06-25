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


#include <QList>
#include <QDebug>

#include "dfmapplication.h"
#include "dquicksearchfilter.h"



struct DQuickSearchFilterPrivate {
    DQuickSearchFilterPrivate(DQuickSearchFilter *const q);
    ~DQuickSearchFilterPrivate() = default;
    DQuickSearchFilterPrivate(const DQuickSearchFilterPrivate &) = delete;
    DQuickSearchFilterPrivate &operator=(const DQuickSearchFilterPrivate &) = delete;

    bool whetherFilterCurrentFile(const QByteArray &file)const;

    DQuickSearchFilter *q_ptr{ nullptr };
    QList<std::basic_regex<char>> m_black_list{};
    std::unique_ptr<dde_file_manager::DFMSettings> m_fm_setting{ nullptr };
};




DQuickSearchFilterPrivate::DQuickSearchFilterPrivate(DQuickSearchFilter *const q)
    : q_ptr{ q },
      m_fm_setting{ dde_file_manager::DFMApplication::genericSetting() }
{
    QVariant black_var{ m_fm_setting->value("QuickSearchFilter", "BlackList") };
    QList<QString> black_list{ black_var.toStringList() };

    if (!black_list.isEmpty()) {

        for (const QString &str : black_list) {
#ifdef QT_DEBUG
            qDebug() << str;
#endif //QT_DEBUG
            std::basic_regex<char> regex { str.toLocal8Bit().constData(), std::basic_regex<char>::grep };
            m_black_list.push_back(std::move(regex));
        }
    }
}

bool DQuickSearchFilterPrivate::whetherFilterCurrentFile(const QByteArray &file) const
{
    if (file.isEmpty()) {

        for (const std::basic_regex<char> &regex : m_black_list) {
            std::match_results<QByteArray::const_iterator> matched_result{};

            if (std::regex_search(file.constData(), matched_result, regex)) {
                return false;
            }
        }
    }

    return true;
}



DQuickSearchFilter::DQuickSearchFilter()
    : d_ptr{ new DQuickSearchFilterPrivate{ this } }
{
}

bool DQuickSearchFilter::whetherFilterCurrentFile(const QByteArray &file) const
{
    const DQuickSearchFilterPrivate *d{ d_func() };
    return d->whetherFilterCurrentFile(file);
}

DQuickSearchFilter::~DQuickSearchFilter()
{
}
