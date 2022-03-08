/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
    bool loadTranslator(QList<QLocale> localeFallback = QList<QLocale>() << QLocale::system());
    void openAsAdmin();
    void handleNewClient(const QString &uniqueKey);

public slots:
    bool setSingleInstance(const QString &key);
    void handleConnection();
    void readData();
    void closeServer();

private:
    static QLocalSocket *getNewClientConnect(const QString &key, const QByteArray &message);
    static QString userServerName(const QString &key);

    void initConnect();

private:
    QLocalServer *localServer;
};

#endif   // SINGLEAPPLICATION_H
