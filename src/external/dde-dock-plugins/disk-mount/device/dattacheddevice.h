// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    virtual void detach() = 0;

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
     * \brief query device info from server
     */
    virtual void query() = 0;

protected:
    QString deviceId;
};

#endif   // DATTACHEDDEVICE_H
