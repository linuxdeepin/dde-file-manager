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
#include "private/abstractfiledevice_p.h"
#include "base/abstractfiledevice.h"
#include "base/urlroute.h"

DFMBASE_BEGIN_NAMESPACE
/*!
 * \class AbstractFileDevice 抽象文件device类
 *
 * \brief 此类是拷贝时，对文件的操作，对应不同的url实现不一样的拷贝使用的接口类
 */

AbstractFileDevicePrivate::AbstractFileDevicePrivate(AbstractFileDevice *qq)
    : QObject (qq)
    , q(qq)
{

}

AbstractFileDevicePrivate::~AbstractFileDevicePrivate()
{

}

AbstractFileDevice::AbstractFileDevice(const QUrl &url)
{
    d->url = QUrl::fromLocalFile(UrlRoute::urlToPath(url));
}

AbstractFileDevice::~AbstractFileDevice()
{

}
/*!
 * \brief url 获取当前文件的url
 *
 * \return QUrl 当前文件的url
 */
QUrl AbstractFileDevice::url() const
{
    return d->url;
}
/*!
 * \brief setFileUrl 设置文件的url
 *
 * \param const QUrl &url 文件的url
 *
 * \return bool 设置是否成功
 */
bool AbstractFileDevice::setFileUrl(const QUrl &url)
{
    d->url = url;
    return true;
}

AbstractFileDevice::AbstractFileDevice(AbstractFileDevicePrivate &dd)
    : d(&dd)
{

}
/*!
 * \brief handle 获取文件的描述符
 *
 * \return int 文件的描述符
 */
int AbstractFileDevice::handle() const
{
    return -1;
}
/*!
 * \brief resize 修改文件的大小
 *
 * \param qint64 size 修改文件的大小
 *
 * \return bool 修改文件大小是否成功
 */
bool AbstractFileDevice::resize(qint64 size)
{
    Q_UNUSED(size)

    return false;
}
/*!
 * \brief flush 将当次的数据刷出缓存
 *
 * \return bool 数据刷出缓存是否成功
 */
bool AbstractFileDevice::flush()
{
    return false;
}
/*!
 * \brief syncToDisk 同步数据到磁盘
 *
 * \param const AbstractFileDevice::SyncOperate &op 同步的操作方式
 *
 * \return bool 是否同步成功
 */
bool AbstractFileDevice::syncToDisk(const AbstractFileDevice::SyncOperate &op)
{
    Q_UNUSED(op);
    return false;
}

DFMBASE_END_NAMESPACE
