/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "gvfs/secretmanager.h"
#include "stubext.h"

#include <gtest/gtest.h>

#include <QFile>

using namespace stub_ext;

namespace  {
class TestSecretManager: public testing::Test
{
public:
    SecretManager *m_secreat {nullptr};
    void SetUp() override
    {
        m_secreat = new SecretManager;
    }

    void TearDown() override
    {
        delete m_secreat;
    }
};
}

TEST_F(TestSecretManager, SMBSecretSchema)
{
    const SecretSchema *schema = m_secreat->SMBSecretSchema();
    EXPECT_TRUE(schema);
}

TEST_F(TestSecretManager, FTPSecretSchema)
{
    const SecretSchema *schema = m_secreat->FTPSecretSchema();
    EXPECT_TRUE(schema);
}

TEST_F(TestSecretManager, VaultSecretSchema)
{
    const SecretSchema *schema = m_secreat->VaultSecretSchema();
    EXPECT_TRUE(schema);
}


TEST_F(TestSecretManager, on_password_cleared)
{
    StubExt st;
    st.set_lamda(&secret_password_clear_finish, [](GAsyncResult *result, GError **error) {
        return  true;
    });
    EXPECT_NO_FATAL_FAILURE(m_secreat->on_password_cleared(nullptr, nullptr, nullptr));
}

TEST_F(TestSecretManager, storeVaultPassword)
{
    EXPECT_NO_FATAL_FAILURE(m_secreat->storeVaultPassword(""));
}

TEST_F(TestSecretManager, lookupVaultPassword)
{
    EXPECT_NO_FATAL_FAILURE(m_secreat->lookupVaultPassword());
}

TEST_F(TestSecretManager, clearVaultPassword)
{
    EXPECT_NO_FATAL_FAILURE(m_secreat->clearVaultPassword());
}

TEST_F(TestSecretManager, clearPasswordByLoginObj)
{
    QJsonObject obj1;
    obj1["protocol"] = "smb";
    EXPECT_NO_FATAL_FAILURE(m_secreat->clearPasswordByLoginObj(obj1));

    QJsonObject obj2;
    obj2["protocol"] = "ftp";
    EXPECT_NO_FATAL_FAILURE(m_secreat->clearPasswordByLoginObj(obj2));

}

TEST_F(TestSecretManager, getLoginData)
{
    EXPECT_NO_FATAL_FAILURE(m_secreat->getLoginData(""));
}

TEST_F(TestSecretManager, getLoginDatas)
{
    EXPECT_NO_FATAL_FAILURE(m_secreat->getLoginDatas());
}

TEST_F(TestSecretManager, cachePath)
{
    QString path = m_secreat->cachePath();
    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestSecretManager, cacheSambaLoginData)
{
    QString cachePath = m_secreat->cachePath();
    QJsonObject obj;
    obj["id"] = "smb://";
    EXPECT_NO_FATAL_FAILURE(m_secreat->cacheSambaLoginData(obj));
    QFile::remove(cachePath);
}

TEST_F(TestSecretManager, loadCache)
{
    QString cachePath = m_secreat->cachePath();
    EXPECT_NO_FATAL_FAILURE(m_secreat->loadCache());
    QFile::remove(cachePath);
}

TEST_F(TestSecretManager, saveCache)
{
    QString cachePath = m_secreat->cachePath();
    EXPECT_NO_FATAL_FAILURE(m_secreat->saveCache());

    QFile::remove(cachePath);
}
