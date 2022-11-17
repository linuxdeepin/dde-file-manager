/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "file/local/private/localdiriterator_p.h"
#include "file/local/localfileinfo.h"
#include "file/local/localdiriterator.h"
#include "base/urlroute.h"
#include "base/schemefactory.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-io/local/dlocalenumerator.h>
#include <dfm-io/core/denumerator.h>
#include <dfm-io/dfmio_utils.h>

#include <functional>

USING_IO_NAMESPACE
using namespace dfmbase;

LocalDirIteratorPrivate::LocalDirIteratorPrivate(const QUrl &url, const QStringList &nameFilters,
                                                 QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                 LocalDirIterator *q)
    : q(q)
{
    QUrl urlReally = QUrl::fromLocalFile(UrlRoute::urlToPath(url));

    DecoratorFileEnumerator enumerator(urlReally, nameFilters,
                                       static_cast<DEnumerator::DirFilter>(static_cast<int32_t>(filters)),
                                       static_cast<DEnumerator::IteratorFlag>(static_cast<uint8_t>(flags)));

    dfmioDirIterator = enumerator.enumeratorPtr();
    if (!dfmioDirIterator) {
        qWarning("Failed dfm-io use factory create enumerator");
        abort();
    }
}

LocalDirIteratorPrivate::~LocalDirIteratorPrivate()
{
}

void LocalDirIteratorPrivate::initQuerierAsyncCallback(bool succ, void *data)
{
    if (!succ) {
        if (data) {
            InitQuerierAsyncOp *op = static_cast<InitQuerierAsyncOp *>(data);
            if (op) {
                op->me = nullptr;
                delete op;
            }
        }

        return;
    }

    if (!data)
        return;

    InitQuerierAsyncOp *op = static_cast<InitQuerierAsyncOp *>(data);
    if (op) {
        if (!op->me) {
            delete op;
            return;
        }

        const QUrl &url = op->url;
        auto fileinfo = op->me->dfmioDirIterator->fileInfo();
        QSharedPointer<LocalFileInfo> info = QSharedPointer<LocalFileInfo>(new LocalFileInfo(url, fileinfo));
        auto infoTrans = InfoFactory::transfromInfo<AbstractFileInfo>(url.scheme(), info);

        const QString &fileName = fileinfo->attribute(DFileInfo::AttributeID::kStandardName, nullptr).toString();
        bool isHidden = false;
        if (fileName.startsWith(".")) {
            isHidden = true;
        } else {
            isHidden = hideFileList.contains(fileName);
        }
        infoTrans->cacheAttribute(DFileInfo::AttributeID::kStandardIsHidden, isHidden);
        infoTrans->setIsLocalDevice(isLocalDevice);
        infoTrans->setIsCdRomDevice(isCdRomDevice);

        InfoCache::instance().cacheInfo(url, infoTrans);

        op->me = nullptr;
        delete op;
    }
}

void LocalDirIteratorPrivate::cacheAttribute(const QUrl &url)
{
    auto dfileInfo = dfmioDirIterator->fileInfo();

    InitQuerierAsyncOp *op = new InitQuerierAsyncOp;
    op->me = this;
    op->url = url;

    auto func = std::bind(&LocalDirIteratorPrivate::initQuerierAsyncCallback, this, std::placeholders::_1, std::placeholders::_2);
    dfileInfo->initQuerierAsync(0, func, op);
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
    : AbstractDirIterator(url, nameFilters, filters, flags), d(new LocalDirIteratorPrivate(url, nameFilters, filters, flags, this))
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
    if (d->dfmioDirIterator) {
        d->currentUrl = d->dfmioDirIterator->next();
    }

    return d->currentUrl;
}
/*!
 * \brief hasNext 获取迭代器是否还有下一个文件
 *
 * \return bool 返回是否还有下一个文件
 */
bool LocalDirIterator::hasNext() const
{
    if (d->dfmioDirIterator) {
        bool has = d->dfmioDirIterator->hasNext();
        if (has) {
            const QUrl &urlNext = d->dfmioDirIterator->next();
            const bool needCache = !InfoCache::instance().cacheDisable(urlNext.scheme());
            if (needCache) {
                AbstractFileInfoPointer infoCache = InfoCache::instance().getCacheInfo(urlNext);
                if (!infoCache)
                    // cache info
                    d->cacheAttribute(urlNext);
            }
        }

        return has;
    }

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

    path = path.replace(QRegExp("/*/"), "/");
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
    return UrlRoute::pathToReal(d->currentUrl.path());
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
    return InfoFactory::create<AbstractFileInfo>(fileUrl());
}
/*!
 * \brief url 获取文件迭代器的基本文件路径的url
 *
 * \return QUrl 返回文件迭代器的基本文件路径的url Returns the base url of the iterator.
 */
QUrl LocalDirIterator::url() const
{
    if (d->dfmioDirIterator)
        return UrlRoute::pathToReal(d->dfmioDirIterator->uri().path());

    return QUrl();
}

void LocalDirIterator::cacheBlockIOAttribute()
{
    const QUrl &rootUrl = this->url();
    const QUrl &url = DFMIO::DFMUtils::buildFilePath(rootUrl.toString().toStdString().c_str(), ".hidden", nullptr);
    d->hideFileList = DFMIO::DFMUtils::hideListFromUrl(url);

    d->isLocalDevice = FileUtils::isLocalDevice(rootUrl);
    d->isCdRomDevice = FileUtils::isCdRomDevice(rootUrl);
}
