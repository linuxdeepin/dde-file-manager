/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#include "desktopdbusinterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QDBusInterface>
#include <QDBusPendingCall>

using namespace dde_desktop;

TEST(DesktopDBusInterface, Refresh)
{
    DesktopDBusInterface ifs;
    stub_ext::StubExt stub;
    QString srv;
    QString addr;
    QString sifs;
    QString me;
    bool silent = false;
    stub.set_lamda(&QDBusAbstractInterface::asyncCall, [&srv, &addr, &sifs, &silent, &me](QDBusAbstractInterface *self, const QString &method,
                                                   const QVariant &arg1,
                                                   const QVariant &arg2,
                                                   const QVariant &arg3,
                                                   const QVariant &arg4,
                                                   const QVariant &arg5,
                                                   const QVariant &arg6,
                                                   const QVariant &arg7,
                                                   const QVariant &arg8) {
        srv = self->service();
        addr = self->path();
        sifs = self->interface();
        me = method;
        silent = arg1.toBool();
        return QDBusPendingCall::fromError(QDBusError());
    });

    ifs.Refresh();
    EXPECT_EQ(srv, QString("com.deepin.dde.desktop"));
    EXPECT_EQ(addr, QString("/com/deepin/dde/desktop/canvas"));
    EXPECT_EQ(sifs, QString("com.deepin.dde.desktop.canvas"));

    EXPECT_EQ(me, QString("Refresh"));
    EXPECT_TRUE(silent);

    me.clear();
    silent = true;
    ifs.Refresh(false);
    EXPECT_EQ(me, QString("Refresh"));
    EXPECT_FALSE(silent);
}
