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

#ifndef DFILEBATCHPROCESS_H
#define DFILEBATCHPROCESS_H

#include <type_traits>

#include <QMap>
#include <QList>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QFile>
#include <QDebug>

#include <memory>
#include <mutex>


#include "durl.h"
#include "controllers/appcontroller.h"
#include "interfaces/dfileservices.h"


class FileBatchProcessPrivate;
class QFileInfo;

template<typename KeyT, typename ValueT, typename = typename std::enable_if<
                        std::is_copy_assignable<ValueT>::value ||
                        std::is_copy_constructible<ValueT>::value,
                        void>::type>
using QSharedMap = QSharedPointer<QMap<KeyT, ValueT>>;

class FileBatchProcess
{
public:
    explicit FileBatchProcess()=default;
    ~FileBatchProcess() = default;



    ///###: in fact: you can not also use std::move!
    FileBatchProcess(const FileBatchProcess&) =delete;
    FileBatchProcess& operator=(const FileBatchProcess&)=delete;

    QSharedMap<DUrl, DUrl> replaceText(const QList<DUrl>& originUrls, const QPair<QString, QString> &pair)const;
    QSharedMap<DUrl, DUrl> addText(const QList<DUrl>& originUrls, const QPair<QString, DFileService::AddTextFlags>& pair)const;
    QSharedMap<DUrl, DUrl> customText(const QList<DUrl>& originUrls, const QPair<QString, QString> &pair)const;




    static QMap<DUrl, DUrl> batchProcessFile(const QSharedMap<DUrl, DUrl> &map);

    ////###: this is thread safe.
    inline static QSharedPointer<FileBatchProcess> instance()
    {
        static QSharedPointer<FileBatchProcess> batchProcess;

        std::call_once(FileBatchProcess::flag,
                       [&]{batchProcess = QSharedPointer<FileBatchProcess>{ new FileBatchProcess };});
        return batchProcess;
    }


private:
    ////###: there flag is very important.
    static std::once_flag flag;
};


#endif // DFILEBATCHPROCESS_H
