/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: yanghao<yanghao@uniontech.com>
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

#include <gtest/gtest.h>
#define private public
#include "singleapplication.h"
#undef private

#include "commandlinemanager.h"

#include "testhelper.h"
#include "dfilewatcher.h"

#include <QLocalSocket>
#include <QtConcurrent>
#include <QStandardPaths>

using namespace testing;

namespace  {
class SingleApplicationTest : public Test
{
public:
private:
    virtual void SetUp() override
    {
        app =  dynamic_cast<SingleApplication *>(qApp);
    }

    virtual void TearDown() override
    {
    }

public:
    SingleApplication *app;
};

}


TEST_F(SingleApplicationTest, test_loadTranslator)
{
    TestHelper::runInLoop([] {});
    ASSERT_NE(app, nullptr);
    QString appName = app->applicationName();
    ASSERT_FALSE(app->loadTranslator());
    app->setApplicationName("dde-file-manager");
    ASSERT_TRUE(app->loadTranslator());
    app->setApplicationName(appName);
}

TEST_F(SingleApplicationTest, test_userServerName)
{
    QString key = "testKey";
    QString expectValue = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + "/" + key;
    QString serverName = app->userServerName(key);
    ASSERT_EQ(expectValue, serverName);
}

TEST_F(SingleApplicationTest, test_userId)
{
    ASSERT_EQ(app->userId(), SingleApplication::UserID);
}

TEST_F(SingleApplicationTest, test_getUserID)
{
    ASSERT_EQ(app->getUserID(), SingleApplication::UserID);
}

TEST_F(SingleApplicationTest, test_newClientProcess_get_monitor_files)
{
    ASSERT_NE(app, nullptr);
    app->initConnect();
    QString userKey = "SingleApplicationTest1";
    QString tepPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();
    DFileWatcher *watcher = new DFileWatcher(tepPath);
    watcher->startWatcher();
    TestHelper::runInLoop([&] {
        app->setSingleInstance(userKey);
        QtConcurrent::run([&]()
        {
            QByteArray data;
            data.append(QString("dde-file-manager").toLocal8Bit().toBase64());
            data.append(' ');
            data.append(QString("--get-monitor-files").toLocal8Bit().toBase64());
            QLocalSocket *socket = SingleApplication::newClientProcess(userKey, data);
            ASSERT_EQ(socket->error(), QLocalSocket::UnknownSocketError);
            ASSERT_TRUE(socket->waitForReadyRead());
            QStringList result;
            for (const QByteArray &i : socket->readAll().split(' ')) {
                QString s = QString::fromLocal8Bit(QByteArray::fromBase64(i));
                result << QString::fromLocal8Bit(QByteArray::fromBase64(i));
            }
            EXPECT_NO_FATAL_FAILURE(result.contains(tepPath));
            FreePointer(socket);
        });
    }, 1000);
    watcher->stopWatcher();
    delete watcher;
}

