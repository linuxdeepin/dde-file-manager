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
#include "gvfs/secretmanager.h"

#include <gtest/gtest.h>

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
