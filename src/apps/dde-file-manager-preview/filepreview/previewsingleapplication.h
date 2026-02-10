// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWSINGLEAPPLICATION_H
#define PREVIEWSINGLEAPPLICATION_H

#include "previewlibrary.h"

#include <DApplication>

QT_BEGIN_NAMESPACE
class QLocalServer;
class QLocalSocket;
QT_END_NAMESPACE

class PreviewLibrary;
class PreviewSingleApplication : public DTK_WIDGET_NAMESPACE::DApplication
{
    Q_OBJECT
public:
    explicit PreviewSingleApplication(int &argc, char **argv, int = ApplicationFlags);
    ~PreviewSingleApplication() override;
    void handleNewClient(const QString &uniqueKey);

    static void processArgs(const QStringList &list);

public Q_SLOTS:
    bool setSingleInstance(const QString &key);
    void handleConnection();
    void closeServer();
    void readData();

private:
    static QLocalSocket *getNewClientConnect(const QString &key, const QByteArray &message);
    static QString userServerName(const QString &key);

    void initConnect();

private:
    QLocalServer *localServer;
};

#endif // PREVIEWSINGLEAPPLICATION_H
