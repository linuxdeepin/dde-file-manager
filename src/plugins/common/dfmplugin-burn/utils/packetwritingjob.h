// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKETWRITINGJOB_H
#define PACKETWRITINGJOB_H

#include "dfmplugin_burn_global.h"

#include <dfm-burn/dpacketwritingcontroller.h>

#include <QThread>
#include <QUrl>
#include <QQueue>
#include <QTimer>

DPBURN_BEGIN_NAMESPACE

class AbstractPacketWritingJob;
class PacketWritingScheduler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PacketWritingScheduler)

public:
    static PacketWritingScheduler &instance();
    void addJob(AbstractPacketWritingJob *job);

private Q_SLOTS:
    void onTimeout();

private:
    explicit PacketWritingScheduler(QObject *parent = nullptr);

private:
    QQueue<AbstractPacketWritingJob *> jobs;
    QTimer rotationTimer;
};

class AbstractPacketWritingJob : public QThread
{
    Q_OBJECT

public:
    explicit AbstractPacketWritingJob(const QString &device, QObject *parent = nullptr);
    virtual ~AbstractPacketWritingJob() override;
    QString device() const;

protected:
    void run() override;
    virtual bool work() = 0;
    QStringList urls2Names(const QList<QUrl> &urls) const;

protected:
    QString curDevice;
    QScopedPointer<DFMBURN::DPacketWritingController> pwController;
};

class PutPacketWritingJob : public AbstractPacketWritingJob
{
    Q_OBJECT
    Q_PROPERTY(QList<QUrl> pendingUrls READ getPendingUrls WRITE setPendingUrls)

public:
    explicit PutPacketWritingJob(const QString &device, QObject *parent = nullptr);

    QList<QUrl> getPendingUrls() const;
    void setPendingUrls(const QList<QUrl> &value);

protected:
    bool work() override;

private:
    QList<QUrl> pendingUrls;
};

class RemovePacketWritingJob : public AbstractPacketWritingJob
{
    Q_OBJECT
    Q_PROPERTY(QList<QUrl> pendingUrls READ getPendingUrls WRITE setPendingUrls)

public:
    explicit RemovePacketWritingJob(const QString &device, QObject *parent = nullptr);

    QList<QUrl> getPendingUrls() const;
    void setPendingUrls(const QList<QUrl> &value);

protected:
    bool work() override;

private:
    QList<QUrl> pendingUrls;
};

class RenamePacketWritingJob : public AbstractPacketWritingJob
{
    Q_OBJECT
    Q_PROPERTY(QUrl srcUrl READ getSrcUrl WRITE setSrcUrl)
    Q_PROPERTY(QUrl destUrl READ getDestUrl WRITE setDestUrl)

public:
    explicit RenamePacketWritingJob(const QString &device, QObject *parent = nullptr);

    QUrl getSrcUrl() const;
    void setSrcUrl(const QUrl &value);

    QUrl getDestUrl() const;
    void setDestUrl(const QUrl &value);

protected:
    bool work() override;

private:
    QUrl srcUrl;
    QUrl destUrl;
};

DPBURN_END_NAMESPACE

#endif   // PACKETWRITINGJOB_H
