/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hujianzhong<hujianzhong@uniontech.com>
 *
 * Maintainer: hujianzhong<hujianzhong@uniontech.com>
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

#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"

#include "controllers/interface/tagmanagerdaemon_interface.h"

DFM_USE_NAMESPACE

static constexpr const  char *const service{ "com.deepin.filemanager.daemon" };
static constexpr const char *const path{ "/com/deepin/filemanager/daemon/TagManagerDaemon" };


namespace  {

class TestTagManagerDaemonInterface : public testing::Test
{
public:
    std::unique_ptr<TagManagerDaemonInterface> m_daemonInterface{ nullptr };

    virtual void SetUp() override
    {
        m_daemonInterface = std::unique_ptr<TagManagerDaemonInterface> { new TagManagerDaemonInterface {
                service,
                path,
                QDBusConnection::systemBus()
            }
        };

        // blumia: since it's associated with context menu so we need to set a shorter timeout time.
        m_daemonInterface->setTimeout(3000);
    }

    virtual void TearDown() override
    {
        m_daemonInterface->destroyed();
    }
};

}

TEST_F(TestTagManagerDaemonInterface, tst_test)
{

}
