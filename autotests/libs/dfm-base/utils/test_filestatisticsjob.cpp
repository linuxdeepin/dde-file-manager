// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QList>

#include <dfm-base/utils/filestatisticsjob.h>
#include "stubext.h"

using namespace dfmbase;

class FileStatisticsJobTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(FileStatisticsJobTest, Constructor_CreateJob_ExpectedJobCreated) {
    // Arrange & Act
    FileStatisticsJob job;

    // Assert
    EXPECT_EQ(job.state(), FileStatisticsJob::kStoppedState);
}

TEST_F(FileStatisticsJobTest, State_GetState_ExpectedInitialState) {
    // Arrange
    FileStatisticsJob job;

    // Act
    auto state = job.state();

    // Assert
    EXPECT_EQ(state, FileStatisticsJob::kStoppedState);
}

TEST_F(FileStatisticsJobTest, TotalSize_GetSize_ExpectedInitialSize) {
    // Arrange
    FileStatisticsJob job;

    // Act
    qint64 size = job.totalSize();

    // Assert
    EXPECT_EQ(size, 0);
}

TEST_F(FileStatisticsJobTest, TotalProgressSize_GetSize_ExpectedInitialSize) {
    // Arrange
    FileStatisticsJob job;

    // Act
    qint64 size = job.totalProgressSize();

    // Assert
    EXPECT_EQ(size, 0);
}

TEST_F(FileStatisticsJobTest, FilesCount_GetCount_ExpectedInitialCount) {
    // Arrange
    FileStatisticsJob job;

    // Act
    int count = job.filesCount();

    // Assert
    EXPECT_EQ(count, 0);
}

TEST_F(FileStatisticsJobTest, DirectorysCount_GetCount_ExpectedInitialCount) {
    // Arrange
    FileStatisticsJob job;

    // Act
    int count = job.directorysCount();

    // Assert
    EXPECT_EQ(count, 0);
}

TEST_F(FileStatisticsJobTest, FileHints_GetHints_ExpectedDefaultHints) {
    // Arrange
    FileStatisticsJob job;

    // Act
    auto hints = job.fileHints();

    // Assert
    EXPECT_EQ(hints, FileStatisticsJob::FileHints(FileStatisticsJob::kNoHint));
}

TEST_F(FileStatisticsJobTest, SetFileHints_SetAndVerify_ExpectedHintsSet) {
    // Arrange
    FileStatisticsJob job;
    FileStatisticsJob::FileHints hints = FileStatisticsJob::kNoFollowSymlink;

    // Act
    job.setFileHints(hints);

    // Assert
    EXPECT_EQ(job.fileHints(), hints);
}

TEST_F(FileStatisticsJobTest, StartWithEmptyList_ExpectedNoCrash) {
    // Arrange
    FileStatisticsJob job;
    QList<QUrl> emptyList;

    // Act
    job.start(emptyList);

    // Assert
    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(FileStatisticsJobTest, Stop_WhenStopped_ExpectedNoCrash) {
    // Arrange
    FileStatisticsJob job;

    // Act
    job.stop();

    // Assert
    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(FileStatisticsJobTest, TogglePause_WhenStopped_ExpectedNoCrash) {
    // Arrange
    FileStatisticsJob job;

    // Act
    job.togglePause();

    // Assert
    // Should not crash
    EXPECT_TRUE(true);
}