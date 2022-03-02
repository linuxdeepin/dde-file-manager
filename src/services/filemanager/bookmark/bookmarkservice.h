/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#ifndef BOOKMARKSERVICE_H
#define BOOKMARKSERVICE_H

#include "dfm_filemanager_service_global.h"

#include <dfm-framework/framework.h>

DSB_FM_BEGIN_NAMESPACE

class BookMarkServicePrivate;
class BookMarkService : public DPF_NAMESPACE::PluginService,
                        DPF_NAMESPACE::AutoServiceRegister<BookMarkService>
{
    Q_OBJECT
    Q_DISABLE_COPY(BookMarkService)
    friend class DPF_NAMESPACE::QtClassFactory<DPF_NAMESPACE::PluginService>;

public:
    static QString name() { return "org.deepin.service.BookMarkService"; }

private:
    explicit BookMarkService(QObject *parent = nullptr);
    ~BookMarkService() override;
    QScopedPointer<BookMarkServicePrivate> d;
};

DSB_FM_END_NAMESPACE

#endif // BOOKMARKSERVICE_H
