/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "localfile/private/localdiriterator_p.h"
#include "localfile/localfileinfo.h"
#include "localfile/localdiriterator.h"
#include "base/urlroute.h"
#include "base/schemefactory.h"

DFMBASE_BEGIN_NAMESPACE

LocalDirIteratorPrivate::LocalDirIteratorPrivate(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags, LocalDirIterator *q)
    : q_ptr(q),
      url(url)
{
    Q_UNUSED(nameFilters);
    Q_UNUSED(filters);
    Q_UNUSED(flags);
    QUrl temp = QUrl::fromLocalFile(UrlRoute::urlToPath(url));

    QSharedPointer<DIOFactory> factory = produceQSharedIOFactory(temp.scheme(), temp);
    if (!factory) {
        qWarning("create factory failed.");      
        abort();
    }

    dfmioDirIterator = factory->createEnumerator();
    if (!dfmioDirIterator) {
        qWarning("create enumerator failed.");
        abort();
    }
}

/*!
 * @class LocalDirIterator 本地（即和系统盘在同一个磁盘的目录）文件迭代器类
 *
 * @brief 使用dfm-io实现了本文件的迭代器
 */
LocalDirIterator::LocalDirIterator(const QUrl &url,
                                       const QStringList &nameFilters,
                                       QDir::Filters filters,
                                       QDirIterator::IteratorFlags flags)
    : d_ptr(new LocalDirIteratorPrivate(url,nameFilters, filters,flags,this))
{

}

LocalDirIterator::~LocalDirIterator()
{

}
/*!
 * \brief next 下一个文件，迭代器指向下一个文件
 *
 * \return QUrl 返回第一个文件的QUrl
 **/

QUrl LocalDirIterator::next()
{
    Q_D(LocalDirIterator);
    if (d->dfmioDirIterator)
        return UrlRoute::pathToUrl(d->dfmioDirIterator->next());
    return QUrl();
}
/*!
 * \brief hasNext 获取迭代器是否还有下一个文件
 *
 * \return bool 返回是否还有下一个文件
 */

bool LocalDirIterator::hasNext() const
{
    Q_D(const LocalDirIterator);
    if (d->dfmioDirIterator)
        return d->dfmioDirIterator->hasNext();
    return false;
}
/*!
 * \brief fileName 获取文件迭代器当前文件的文件名称
 *
 * \return QString 返回文件迭代器当前文件的文件名称
 **/
QString LocalDirIterator::fileName() const
{
    QString path = fileUrl().path();
    if (path.isEmpty())
        return QString();
    path = path.replace(QRegExp("/*/"),"/");
    if (path == "/")
        return QString();
    if (path.endsWith("/"))
        path = path.left(path.size() - 1);
    QStringList pathList = path.split("/");
    return pathList.last();
}
/*!
 * \brief fileUrl 获取文件迭代器当前文件全路径url
 *
 * \return QString 返回文件迭代器当前文件全路径url
 */
QUrl LocalDirIterator::fileUrl() const
{
    Q_D(const LocalDirIterator);
    if (d->dfmioDirIterator)
        return d->dfmioDirIterator->uri();
    return QUrl();
}
/*!
 * \brief fileUrl 获取文件迭代器当前文件的文件信息
 *
 * 必须使用事件去获取文件信息是否有缓存
 *
 * \return DAbstractFileInfoPointer 返回文件迭代器当前文件的文件信息的智能指针
 **/
const AbstractFileInfoPointer LocalDirIterator::fileInfo() const
{
    return InfoFactory::instance().create<AbstractFileInfo>(fileUrl());
}
/*!
 * \brief url 获取文件迭代器的基本文件路径的url
 *
 * \return QUrl 返回文件迭代器的基本文件路径的url Returns the base url of the iterator.
 */
QUrl LocalDirIterator::url() const
{
    Q_D(const LocalDirIterator);
    return d->url;
}

DFMBASE_END_NAMESPACE
