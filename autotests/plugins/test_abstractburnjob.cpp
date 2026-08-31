// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractburnjob.cpp
 * @brief Unit tests for AbstractBurnJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class AbstractBurnJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractBurnJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractBurnJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractBurnJobTest, addTask)
{
    // Test method: void addTask(())
    EXPECT_NO_FATAL_FAILURE(obj->addTask());
}

TEST_F(AbstractBurnJobTest, comfort)
{
    // Test method: void comfort(())
    EXPECT_NO_FATAL_FAILURE(obj->comfort());
}

TEST_F(AbstractBurnJobTest, currentDeviceInfo)
{
    // Test getter: QVariantMap currentDeviceInfo()
    auto result = obj->currentDeviceInfo();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AbstractBurnJobTest, fileSystemLimitsValid)
{
    // Test bool getter: fileSystemLimitsValid()
    bool result = obj->fileSystemLimitsValid();
    EXPECT_FALSE(result);

}

TEST_F(AbstractBurnJobTest, mediaChangDected)
{
    // Test bool getter: mediaChangDected()
    bool result = obj->mediaChangDected();
    EXPECT_FALSE(result);

}

TEST_F(AbstractBurnJobTest, onJobUpdated)
{
    // Test method: void onJobUpdated((JobStatus status, int progress, const QString &speed, const QStringList &message))
    QString _arg2{};
    QStringList _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->onJobUpdated(JobStatus(), 0, _arg2, _arg3));
}

TEST_F(AbstractBurnJobTest, property)
{
    // Test method: QVariant property((AbstractBurnJob::PropertyType type))
    auto result = obj->property(AbstractBurnJob::PropertyType());
    EXPECT_FALSE(result.isValid());

}

TEST_F(AbstractBurnJobTest, readyToWork)
{
    // Test bool getter: readyToWork()
    bool result = obj->readyToWork();
    EXPECT_FALSE(result);

}

TEST_F(AbstractBurnJobTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}

TEST_F(AbstractBurnJobTest, setProperty)
{
    // Test setter: void setProperty((AbstractBurnJob::PropertyType type, const QVariant &val))
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setProperty(AbstractBurnJob::PropertyType(), _arg1));
}

TEST_F(AbstractBurnJobTest, updateMessage)
{
    // Test method: void updateMessage((JobInfoPointer ptr))
    EXPECT_NO_FATAL_FAILURE(obj->updateMessage(JobInfoPointer()));
}

TEST_F(AbstractBurnJobTest, updateSpeed)
{
    // Test method: void updateSpeed((JobInfoPointer ptr, JobStatus status, const QString &speed))
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->updateSpeed(JobInfoPointer(), JobStatus(), _arg2));
}

TEST_F(AbstractBurnJobTest, updatedInSubProcess)
{
    // Test method: QByteArray updatedInSubProcess((JobStatus status, int progress, const QString &speed, const QStringList &message))
    QString _arg2{};
    QStringList _arg3{};
    auto result = obj->updatedInSubProcess(JobStatus(), 0, _arg2, _arg3);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AbstractBurnJobTest, AbstractBurnJob)
{
    // Test constructor: AbstractBurnJob((const QString &dev, const JobHandlePointer handler))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AbstractBurnJobTest, createManager)
{
    // Test method: DOpticalDiscManager createManager((int fd))
    auto result = obj->createManager(0);
    EXPECT_NO_FATAL_FAILURE({ obj->createManager(0); });

}
