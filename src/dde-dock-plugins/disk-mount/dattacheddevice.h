/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DATTACHEDDEVICE_H
#define DATTACHEDDEVICE_H

#include <QString>
#include <QPair>
#include <QUrl>
#include <QPointer>

class DAttachedDevice
{
public:
    explicit DAttachedDevice(const QString &id);
    virtual ~DAttachedDevice();

    /*!
     * \brief device isvalid and useable
     * \return
     */
    virtual bool isValid() = 0;

    /*!
     * \brief can be unmounted / removable or not.
     * \return
     */
    virtual bool detachable() = 0;

    /*!
     * \brief do unmount, also do eject if possible.
     */
    virtual void detach();

    /*!
     * \brief device display name.
     * \return
     */
    virtual QString displayName() = 0;

    /*!
     * \brief storage valid / available or not.
     * \return
     */
    virtual bool deviceUsageValid() = 0;

    /*!
     * \brief used / total, in bytes.
     * \return
     */
    virtual QPair<quint64, quint64> deviceUsage() = 0;

    /*!
     * \brief device icon (theme) name.
     * \return
     */
    virtual QString iconName() = 0;

    /*!
     * \brief path to the device mount point.
     * \return
     */
    virtual QUrl mountpointUrl() = 0;

    /*!
     * \brief path to the open filemanger.
     * \return
     */
    virtual QUrl accessPointUrl() = 0;

    /*!
     * \brief parse json from server
     */
    virtual void parse() = 0;

protected:
    QString deviceId;
};

#endif // DATTACHEDDEVICE_H
