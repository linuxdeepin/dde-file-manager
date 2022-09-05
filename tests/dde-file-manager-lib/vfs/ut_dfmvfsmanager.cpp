// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vfs/dfmvfsmanager.h"
#include "vfs/private/dfmvfsmanager_p.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace dde_file_manager;

namespace  {

class TestDFMVfsManager: public testing::Test {
public:
    DFM_NAMESPACE::DFMVfsManager *m_manager {nullptr};
    DFMVfsManagerPrivate *d {nullptr};

    void SetUp() override
    {
        m_manager = new DFM_NAMESPACE::DFMVfsManager;
        d = new DFMVfsManagerPrivate(m_manager);
    }

    void TearDown() override
    {
        delete m_manager;
        delete d;
    }
};
}

TEST_F(TestDFMVfsManager, getVfsList)
{
    EXPECT_NO_FATAL_FAILURE(m_manager->getVfsList().size());
}

TEST_F(TestDFMVfsManager, attach)
{
    EXPECT_FALSE(m_manager->attach(DUrl::fromLocalFile("/")));
}

TEST_F(TestDFMVfsManager, handler)
{
    m_manager->setEventHandler(nullptr);
    EXPECT_EQ(m_manager->eventHandler(), nullptr);
}

// private
TEST_F(TestDFMVfsManager, p_GVolumeMonitorMountAddedCb)
{
    EXPECT_NO_FATAL_FAILURE(d->GVolumeMonitorMountAddedCb(nullptr, nullptr, m_manager));
}

TEST_F(TestDFMVfsManager, p_GVolumeMonitorMountRemovedCb)
{
    EXPECT_NO_FATAL_FAILURE(d->GVolumeMonitorMountRemovedCb(nullptr, nullptr, m_manager));
}

TEST_F(TestDFMVfsManager, p_GVolumeMonitorMountChangedCb)
{
    EXPECT_NO_FATAL_FAILURE(d->GVolumeMonitorMountChangedCb(nullptr, nullptr, m_manager));
}
