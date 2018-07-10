/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef DFMDISKDEVICE_H
#define DFMDISKDEVICE_H

#include <dfmglobal.h>

#include <QObject>
#include <QVariantMap>

DFM_BEGIN_NAMESPACE

class DFMDiskDevicePrivate;
class DFMDiskDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMDiskDevice)

    Q_PROPERTY(QString path READ path CONSTANT FINAL)
    Q_PROPERTY(bool canPowerOff READ canPowerOff CONSTANT FINAL)
    Q_PROPERTY(QVariantMap configuration READ configuration CONSTANT FINAL)
    Q_PROPERTY(QString connectionBus READ connectionBus CONSTANT FINAL)
    Q_PROPERTY(bool ejectable READ ejectable CONSTANT FINAL)
    Q_PROPERTY(QString id READ id CONSTANT FINAL)
    Q_PROPERTY(QString media READ media CONSTANT FINAL)
    Q_PROPERTY(bool mediaAvailable READ mediaAvailable CONSTANT FINAL)
    Q_PROPERTY(bool mediaChangeDetected READ mediaChangeDetected CONSTANT FINAL)
    Q_PROPERTY(QStringList mediaCompatibility READ mediaCompatibility CONSTANT FINAL)
    Q_PROPERTY(bool mediaRemovable READ mediaRemovable CONSTANT FINAL)
    Q_PROPERTY(QString model READ model CONSTANT FINAL)
    Q_PROPERTY(bool optical READ optical CONSTANT FINAL)
    Q_PROPERTY(bool opticalBlank READ opticalBlank CONSTANT FINAL)
    Q_PROPERTY(uint opticalNumAudioTracks READ opticalNumAudioTracks CONSTANT FINAL)
    Q_PROPERTY(uint opticalNumDataTracks READ opticalNumDataTracks CONSTANT FINAL)
    Q_PROPERTY(uint opticalNumSessions READ opticalNumSessions CONSTANT FINAL)
    Q_PROPERTY(uint opticalNumTracks READ opticalNumTracks CONSTANT FINAL)
    Q_PROPERTY(bool removable READ removable CONSTANT FINAL)
    Q_PROPERTY(QString revision READ revision CONSTANT FINAL)
    Q_PROPERTY(int rotationRate READ rotationRate CONSTANT FINAL)
    Q_PROPERTY(QString seat READ seat CONSTANT FINAL)
    Q_PROPERTY(QString serial READ serial CONSTANT FINAL)
    Q_PROPERTY(QString siblingId READ siblingId CONSTANT FINAL)
    Q_PROPERTY(qulonglong size READ size CONSTANT FINAL)
    Q_PROPERTY(QString sortKey READ sortKey CONSTANT FINAL)
    Q_PROPERTY(qulonglong timeDetected READ timeDetected CONSTANT FINAL)
    Q_PROPERTY(qulonglong timeMediaDetected READ timeMediaDetected CONSTANT FINAL)
    Q_PROPERTY(QString vendor READ vendor CONSTANT FINAL)
    Q_PROPERTY(QString WWN READ WWN CONSTANT FINAL)

public:
    ~DFMDiskDevice();
    QString path() const;
    bool canPowerOff() const;
    QVariantMap configuration() const;
    QString connectionBus() const;
    bool ejectable() const;
    QString id() const;
    QString media() const;
    bool mediaAvailable() const;
    bool mediaChangeDetected() const;
    QStringList mediaCompatibility() const;
    bool mediaRemovable() const;
    QString model() const;
    bool optical() const;
    bool opticalBlank() const;
    uint opticalNumAudioTracks() const;
    uint opticalNumDataTracks() const;
    uint opticalNumSessions() const;
    uint opticalNumTracks() const;
    bool removable() const;
    QString revision() const;
    int rotationRate() const;
    QString seat() const;
    QString serial() const;
    QString siblingId() const;
    qulonglong size() const;
    QString sortKey() const;
    qulonglong timeDetected() const;
    qulonglong timeMediaDetected() const;
    QString vendor() const;
    QString WWN() const;

public Q_SLOTS: // METHODS
    void eject(const QVariantMap &options);
    void powerOff(const QVariantMap &options);
    void setConfiguration(const QVariantMap &value, const QVariantMap &options);

private:
    explicit DFMDiskDevice(const QString &path, QObject *parent = nullptr);

    QScopedPointer<DFMDiskDevicePrivate> d_ptr;

    friend class DFMDiskManager;
};

DFM_END_NAMESPACE

#endif // DFMDISKDEVICE_H
