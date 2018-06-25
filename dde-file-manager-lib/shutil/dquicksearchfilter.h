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
#ifndef DQUICKSEARCHFILTER_H
#define DQUICKSEARCHFILTER_H

#include <regex>
#include <memory>



#include <QObject>
#include <QByteArray>
#include <QScopedPointer>



#include "dfmsettings.h"



class DQuickSearchFilterPrivate;
class DQuickSearchFilter final : public QObject
{
    Q_OBJECT
public:
    DQuickSearchFilter();
    ~DQuickSearchFilter();

    DQuickSearchFilter(const DQuickSearchFilter &) = delete;
    DQuickSearchFilter &operator=(const DQuickSearchFilter &) = delete;

    bool whetherFilterCurrentFile(const QByteArray &file) const;

    inline static DQuickSearchFilter *instance()
    {
        static DQuickSearchFilter *filter{ nullptr };

        return filter;
    }

private:

    QScopedPointer<DQuickSearchFilterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DQuickSearchFilter)
};

#endif // DQUICKSEARCHFILTER_H
