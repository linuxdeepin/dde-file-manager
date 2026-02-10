// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALENTRYDATA_H
#define VIRTUALENTRYDATA_H

#include <QObject>

inline constexpr char kTableName[] { "virtual_entry_data" };

namespace dfm_upgrade {

class VirtualEntryData : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "virtual_entry_data")
    Q_PROPERTY(QString key READ getKey WRITE setKey NOTIFY keyChanged)
    Q_PROPERTY(QString protocol READ getProtocol WRITE setProtocol NOTIFY protocolChanged)
    Q_PROPERTY(QString host READ getHost WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ getPort WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString displayName READ getDisplayName WRITE setDisplayName NOTIFY displayNameChanged)

public:
    explicit VirtualEntryData(QObject *parent = nullptr);
    VirtualEntryData(const QString &standardSmbPath);
    VirtualEntryData(const VirtualEntryData &other);
    VirtualEntryData &operator=(const VirtualEntryData &other);

    const QString &getKey() const;
    void setKey(const QString &newKey);
    const QString &getProtocol() const;
    void setProtocol(const QString &newProtocol);
    const QString &getHost() const;
    void setHost(const QString &newHost);
    int getPort() const;
    void setPort(int newPort);
    const QString &getDisplayName() const;
    void setDisplayName(const QString &newDisplayName);

signals:
    void keyChanged();
    void protocolChanged();
    void hostChanged();
    void portChanged();
    void displayNameChanged();

private:
    QString key {};
    QString protocol {};
    QString host {};
    int port { -1 };
    QString displayName {};
};
}

#endif   // VIRTUALENTRYDATA_H
