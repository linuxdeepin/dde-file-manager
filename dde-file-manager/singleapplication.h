/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include <QtGlobal>

#include <DApplication>
#include <durl.h>

QT_BEGIN_NAMESPACE
class QLocalServer;
class QLocalSocket;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class SingleApplication : public DApplication
{
    Q_OBJECT
public:
    explicit SingleApplication(int &argc, char **argv, int = ApplicationFlags);
    ~SingleApplication();
    void initConnect();

    static void initSources();
    static QLocalSocket *newClientProcess(const QString& key, const QByteArray &message);
    static QString userServerName(const QString& key);
    static QString userId();

    bool loadTranslator(QList<QLocale> localeFallback = QList<QLocale>() << QLocale::system());

signals:
    void screenDevicePixelRatioChanged(QScreen *s);

public slots:
    bool setSingleInstance(const QString& key);
    void handleConnection();
    void readData();
    void closeServer();

private:
    void handleQuitAction() Q_DECL_OVERRIDE;

    static QString getUserID();

    static QString UserID;
    QLocalServer* m_localServer;
};

#endif // SINGLEAPPLICATION_H
