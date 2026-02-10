// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <DApplication>

QT_BEGIN_NAMESPACE
class QLocalServer;
class QLocalSocket;
QT_END_NAMESPACE

class SingleApplication : public DTK_WIDGET_NAMESPACE::DApplication
{
    Q_OBJECT
public:
    explicit SingleApplication(int &argc, char **argv, int = ApplicationFlags);
    ~SingleApplication() override;
    void openAsAdmin();
    void handleNewClient(const QString &uniqueKey);

public slots:
    bool setSingleInstance(const QString &key);
    void handleConnection();
    void readData();
    void closeServer();

protected:
    virtual void handleQuitAction() override;

private:
    static QLocalSocket *getNewClientConnect(const QString &key, const QByteArray &message);
    static QString userServerName(const QString &key);

    void initConnect();

private:
    QLocalServer *localServer;
};

#endif   // SINGLEAPPLICATION_H
