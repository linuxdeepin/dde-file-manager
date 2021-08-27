/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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
#include "vfs/dfmvfsdevice.h"
#include "vfs/private/dfmvfsdevice_p.h"
#include "stubext.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QJsonObject>

using namespace stub_ext;
using namespace dde_file_manager;

namespace  {
class TestDFMVfsDevice: public testing::Test {
public:
    DFMVfsDevice *m_service {nullptr};
    DFMVfsDevicePrivate *d {nullptr};

    static void SetUpTestCase()
    {
        // 反复调用 gio mount 可能导致 org.gtk.vfs.UDisks2VolumeMonitor 无发正常提供服务，所以尽可能少调用
        system("gio mount -a ftp://ftp.freebsd.org 1>/dev/null 2>&1");
    }

    static void TearDownTestCase()
    {
    }

    void SetUp() override
    {
        DUrl url;
        url.setScheme(FTP_SCHEME);
        url.setUrl("ftp://ftp.freebsd.org/");
        m_service = dde_file_manager::DFMVfsDevice::createUnsafe(url);

        d = new DFMVfsDevicePrivate(url, m_service);
    }

    void TearDown() override
    {
        delete m_service;
        delete d;
    }
};
}

TEST_F(TestDFMVfsDevice, create)
{
    DUrl url;
    url.setScheme(FTP_SCHEME);
    url.setUrl("ftp://ftp.freebsd.org/");
    auto p = m_service->create(url, nullptr);

    EXPECT_NE(p, nullptr);

    if (p)
        p->deleteLater();
}

TEST_F(TestDFMVfsDevice, attach)
{
    EXPECT_FALSE(m_service->attach());
}

TEST_F(TestDFMVfsDevice, detachAsync)
{
    EXPECT_TRUE(m_service->detachAsync());
    system("gio mount -a ftp://ftp.freebsd.org 1>/dev/null 2>&1");
}

TEST_F(TestDFMVfsDevice, eventHandler)
{
    m_service->setEventHandler(nullptr);
    EXPECT_FALSE(m_service->eventHandler());
}

TEST_F(TestDFMVfsDevice, isReadOnly)
{
    EXPECT_FALSE(m_service->isReadOnly());
}

TEST_F(TestDFMVfsDevice, canDetach)
{
    EXPECT_TRUE(m_service->canDetach());
}

TEST_F(TestDFMVfsDevice, totalBytes)
{
    EXPECT_EQ(m_service->totalBytes(), 0);
}

TEST_F(TestDFMVfsDevice, usedBytes)
{
    EXPECT_EQ(m_service->usedBytes(), 0);
}

TEST_F(TestDFMVfsDevice, freeBytes)
{
    EXPECT_EQ(m_service->freeBytes(), 0);
}

TEST_F(TestDFMVfsDevice, iconList)
{
    EXPECT_TRUE(m_service->iconList().size() >= 0);
}


TEST_F(TestDFMVfsDevice, symbolicIconList)
{
    EXPECT_TRUE(m_service->symbolicIconList().size() >= 0);
}

TEST_F(TestDFMVfsDevice, defaultUri)
{
    const QUrl &url = m_service->defaultUri();
    EXPECT_TRUE(url.isValid());
}

TEST_F(TestDFMVfsDevice, rootPath)
{
    const QString &path = m_service->rootPath();
    EXPECT_NO_FATAL_FAILURE(path.isEmpty());
}

TEST_F(TestDFMVfsDevice, defaultPath)
{
    const QString &path = m_service->defaultPath();
    EXPECT_NO_FATAL_FAILURE(path.isEmpty());
}

TEST_F(TestDFMVfsDevice, name)
{
    const QString &name = m_service->name();
    EXPECT_FALSE(name.isEmpty());
}


// private
TEST_F(TestDFMVfsDevice, p_getThemedIconName)
{
    try {
        d->getThemedIconName(nullptr);
    } catch(...) {

    }
    try {
        DFMGIcon icon(g_themed_icon_new("test"));
        d->getThemedIconName(G_THEMED_ICON(icon.data())).size();
    } catch(...) {

    }
}

TEST_F(TestDFMVfsDevice, p_GMountOperationAskPasswordCb)
{
    Stub st;
    bool (*isEmpty_r)(void *) = [] (void *) {return false;};
    st.set(ADDR(QJsonObject, isEmpty), isEmpty_r);

    GMountOperation *op = DFMVfsDevicePrivate::GMountOperationNewMountOp(m_service);
    GAskPasswordFlags f = G_ASK_PASSWORD_NEED_USERNAME ;
    EXPECT_NO_FATAL_FAILURE (
        DFMVfsDevicePrivate::GMountOperationAskPasswordCb(op, "test", "root", "localhost", f, m_service);
    );

    f = G_ASK_PASSWORD_NEED_DOMAIN ;
    EXPECT_NO_FATAL_FAILURE (
        DFMVfsDevicePrivate::GMountOperationAskPasswordCb(op, "test", "root", "localhost", f, m_service);
    );

    f = G_ASK_PASSWORD_NEED_PASSWORD ;
    EXPECT_NO_FATAL_FAILURE (
        DFMVfsDevicePrivate::GMountOperationAskPasswordCb(op, "test", "root", "localhost", f, m_service);
    );

    f = G_ASK_PASSWORD_SAVING_SUPPORTED ;
    EXPECT_NO_FATAL_FAILURE (
        DFMVfsDevicePrivate::GMountOperationAskPasswordCb(op, "test", "root", "localhost", f, m_service);
    );
}

TEST_F(TestDFMVfsDevice, p_GMountOperationAskQuestionCb)
{
    GMountOperation *op = DFMVfsDevicePrivate::GMountOperationNewMountOp(m_service);

    EXPECT_NO_FATAL_FAILURE (
        DFMVfsDevicePrivate::GMountOperationAskQuestionCb(op, "test", NULL, m_service);
    );
}

TEST_F(TestDFMVfsDevice, p_GFileMountDoneCb)
{

    StubExt stub;
    stub.set_lamda(&g_file_mount_enclosing_volume_finish,
                   [](GFile                      *,
                   GAsyncResult               *,
                   GError                    **error) {

        GError *e = new GError;
        char *m = (char *)malloc(256);
        strcpy(m, "test");
        e->message = m;
        e->domain = G_IO_ERROR;
        *error = e;

        return false;
    });
    EXPECT_NO_FATAL_FAILURE (
        DFMVfsDevicePrivate::GFileMountDoneCb(nullptr, nullptr, m_service);
    );
}

TEST_F(TestDFMVfsDevice, p_GFileUnmountDoneCb)
{
    gboolean (*f)(GMount *, GAsyncResult *, GError **) = [](GMount *, GAsyncResult *, GError **error) {
        gboolean ret = false;
        GError *e = new GError;
        char *m = (char *)malloc(256);
        strcpy(m, "test");
        e->message = m;
        e->domain = G_IO_ERROR;
        *error = e;
        return ret;
    };
    Stub st;
    st.set(&g_mount_unmount_with_operation_finish, f);
    EXPECT_NO_FATAL_FAILURE (
        DFMVfsDevicePrivate::GFileUnmountDoneCb(nullptr, nullptr, m_service);
    );
}

TEST_F(TestDFMVfsDevice, p_createRootFileInfo)
{
    StubExt st;
    st.set_lamda(&g_file_query_filesystem_info, [](GFile                      *,
                                                  const char                 *,
                                                  GCancellable               *,
                                                  GError                    **error) {
        GError *e = g_slice_new(GError);
        e->message = g_strdup("test");;
        *error = e;
        return nullptr;
    });
    try {
        d->createRootFileInfo();
    } catch(...) {

    }
    //EXPECT_NO_FATAL_FAILURE(d->createRootFileInfo());
}
