// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
