/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#ifndef ABSTRACTDIRITERATOR_H
#define ABSTRACTDIRITERATOR_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QDir>
#include <QDirIterator>
/*!
 * \class AbstractDirIterator 文件迭代器接口类
 *
 * \brief 定义了文件迭代器的通用接口
 *
 */
DFMBASE_BEGIN_NAMESPACE
class AbstractDirIterator : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDirIterator() = delete;

    explicit AbstractDirIterator(const QUrl &url,
                                 const QStringList &nameFilters = QStringList(),
                                 QDir::Filters filters = QDir::NoFilter,
                                 QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
    {
        Q_UNUSED(url)
        Q_UNUSED(nameFilters)
        Q_UNUSED(filters)
        Q_UNUSED(flags)
    }

    /*!
     * \brief next 下一个文件，迭代器指向下一个文件
     *
     * \param
     *
     * \return QUrl 返回第一个文件的QUrl
     */
    virtual QUrl next() = 0;
    /*!
     * \brief hasNext 获取迭代器是否还有下一个文件
     *
     * \param
     *
     * \return bool 返回是否还有下一个文件
     */
    virtual bool hasNext() const = 0;
    /*!
     * \brief close 关闭当前迭代器
     *
     * \param
     *
     * \return
     */
    virtual void close() {}
    /*!
     * \brief fileName 获取文件迭代器当前文件的文件名称
     *
     * \param
     *
     * \return QString 返回文件迭代器当前文件的文件名称
     */
    virtual QString fileName() const = 0;
    /*!
     * \brief fileUrl 获取文件迭代器当前文件全路径url
     *
     * \param
     *
     * \return QString 返回文件迭代器当前文件全路径url,Returns the full file url for the current directory entry.
     */
    virtual QUrl fileUrl() const = 0;
    /*!
     * \brief fileUrl 获取文件迭代器当前文件的文件信息
     *
     * \param
     *
     * \return DAbstractFileInfoPointer 返回文件迭代器当前文件的文件信息的智能指针
     */
    virtual const AbstractFileInfoPointer fileInfo() const = 0;
    /*!
     * \brief url 获取文件迭代器的基本文件路径的url
     *
     * \param
     *
     * \return QUrl 返回文件迭代器的基本文件路径的url Returns the base url of the iterator.
     **/
    virtual QUrl url() const = 0;
    /*!
     * \brief enableIteratorByKeyword
     *
     * \param const QString &keyword 哪一个键
     *
     * \return bool 是否设置成功
     */
    virtual bool enableIteratorByKeyword(const QString &keyword)
    {
        Q_UNUSED(keyword);
        return false;
    }
};

DFMBASE_END_NAMESPACE
typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> AbstractDirIteratorPointer;

Q_DECLARE_METATYPE(AbstractDirIteratorPointer)

#endif   // ABSTRACTDIRITERATOR_H
