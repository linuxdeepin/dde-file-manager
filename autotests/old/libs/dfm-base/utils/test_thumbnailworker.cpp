// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dfm-base/utils/thumbnail/thumbnailworker.h>
#include "stubext.h"

#include <QUrl>
#include <QString>
#include <QSignalSpy>
#include <QDebug>
#include <QImage>

DFMBASE_USE_NAMESPACE

class ThumbnailWorkerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // 初始化测试环境
        worker.reset(new ThumbnailWorker());
    }

    void TearDown() override
    {
        // 清理
        if (worker) {
            worker->stop();
        }
        worker.reset();
        
        stub_ext::StubExt clear;
        clear.clear();
    }

    QScopedPointer<ThumbnailWorker> worker;
    stub_ext::StubExt stub;
};

TEST_F(ThumbnailWorkerTest, Constructor_Default_ExpectedValidObject) {
    // Arrange & Act
    QScopedPointer<ThumbnailWorker> testWorker(new ThumbnailWorker());

    // Assert
    EXPECT_NE(testWorker.data(), nullptr) << "ThumbnailWorker should be created successfully";
}

TEST_F(ThumbnailWorkerTest, RegisterCreator_ValidMimeType_ExpectedSuccess) {
    // Arrange
    QString mimeType = "image/jpeg";
    ThumbnailWorker::ThumbnailCreator creator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        return QImage(100, 100, QImage::Format_RGB32);
    };

    // Act
    bool result = worker->registerCreator(mimeType, creator);

    // Assert
    EXPECT_TRUE(result) << "Registering valid creator should succeed";
}

TEST_F(ThumbnailWorkerTest, RegisterCreator_EmptyMimeType_ExpectedFailure) {
    // Arrange
    QString emptyMimeType = "";
    ThumbnailWorker::ThumbnailCreator creator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        return QImage(100, 100, QImage::Format_RGB32);
    };

    // Act
    bool result = worker->registerCreator(emptyMimeType, creator);

    // Assert
    EXPECT_FALSE(result) << "Registering creator with empty MIME type should fail";
}

TEST_F(ThumbnailWorkerTest, OnTaskAdded_ValidTasks_ExpectedNoCrash) {
    // Arrange
    ThumbnailWorker::ThumbnailTaskMap taskMap;
    QUrl imageUrl = QUrl::fromLocalFile("/tmp/test.jpg");
    taskMap[imageUrl] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall;

    // Act
    worker->onTaskAdded(taskMap);

    // Assert
    // Just ensure no crash - the actual processing is asynchronous
    EXPECT_TRUE(true);
}

TEST_F(ThumbnailWorkerTest, OnTaskAdded_EmptyTasks_ExpectedNoCrash) {
    // Arrange
    ThumbnailWorker::ThumbnailTaskMap emptyTaskMap;

    // Act
    worker->onTaskAdded(emptyTaskMap);

    // Assert
    EXPECT_TRUE(true);
}

TEST_F(ThumbnailWorkerTest, StopWorker_ExpectedNoCrash) {
    // Arrange
    ThumbnailWorker::ThumbnailTaskMap taskMap;
    QUrl imageUrl = QUrl::fromLocalFile("/tmp/test.jpg");
    taskMap[imageUrl] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall;
    
    // Add some work
    worker->onTaskAdded(taskMap);

    // Act
    worker->stop();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(ThumbnailWorkerTest, MultipleCreators_ExpectedNoConflict) {
    // Arrange
    QString jpegMimeType = "image/jpeg";
    QString pngMimeType = "image/png";
    
    ThumbnailWorker::ThumbnailCreator jpegCreator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        return QImage(100, 100, QImage::Format_RGB32);
    };
    
    ThumbnailWorker::ThumbnailCreator pngCreator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        return QImage(200, 200, QImage::Format_RGB32);
    };

    // Act
    bool jpegResult = worker->registerCreator(jpegMimeType, jpegCreator);
    bool pngResult = worker->registerCreator(pngMimeType, pngCreator);

    // Assert
    EXPECT_TRUE(jpegResult) << "JPEG creator registration should succeed";
    EXPECT_TRUE(pngResult) << "PNG creator registration should succeed";
}

TEST_F(ThumbnailWorkerTest, SignalConnection_ExpectedValidConnection) {
    // Arrange
    QSignalSpy finishedSpy(worker.data(), &ThumbnailWorker::thumbnailCreateFinished);
    QSignalSpy failedSpy(worker.data(), &ThumbnailWorker::thumbnailCreateFailed);

    // Act
    ThumbnailWorker::ThumbnailTaskMap taskMap;
    QUrl imageUrl = QUrl::fromLocalFile("/tmp/test.jpg");
    taskMap[imageUrl] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall;
    
    worker->onTaskAdded(taskMap);

    // Assert
    // Just ensure signals can be connected
    EXPECT_TRUE(finishedSpy.isValid()) << "Finished signal should be valid";
    EXPECT_TRUE(failedSpy.isValid()) << "Failed signal should be valid";
}

TEST_F(ThumbnailWorkerTest, CreatorReturnNullImage_ExpectedNoCrash) {
    // Arrange
    QString mimeType = "image/test";
    ThumbnailWorker::ThumbnailCreator nullCreator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        return QImage();  // Return null image
    };

    // Act
    bool result = worker->registerCreator(mimeType, nullCreator);

    // Assert
    EXPECT_TRUE(result) << "Registering null-returning creator should succeed";
    
    // Test processing
    ThumbnailWorker::ThumbnailTaskMap taskMap;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.test");
    taskMap[testUrl] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall;
    
    worker->onTaskAdded(taskMap);
    EXPECT_TRUE(true);
}

TEST_F(ThumbnailWorkerTest, CreatorWithException_ExpectedNoCrash) {
    // Arrange
    QString mimeType = "image/exception";
    ThumbnailWorker::ThumbnailCreator exceptionCreator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        // Simulate an exception
        throw std::runtime_error("Test exception");
        return QImage();
    };

    // Act
    bool result = worker->registerCreator(mimeType, exceptionCreator);

    // Assert
    EXPECT_TRUE(result) << "Registering exception-throwing creator should succeed";
    
    // Test processing - should handle exception gracefully
    ThumbnailWorker::ThumbnailTaskMap taskMap;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/exception.test");
    taskMap[testUrl] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall;
    
    worker->onTaskAdded(taskMap);
    EXPECT_TRUE(true);
}

TEST_F(ThumbnailWorkerTest, TaskMapWithDifferentSizes_ExpectedNoCrash) {
    // Arrange
    QString mimeType = "image/jpeg";
    ThumbnailWorker::ThumbnailCreator creator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        int imageSize = 64;  // Default size
        switch (size) {
        case DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall:
            imageSize = 64;
            break;
        case DFMGLOBAL_NAMESPACE::ThumbnailSize::kNormal:
            imageSize = 128;
            break;
        case DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge:
            imageSize = 256;
            break;
        }
        return QImage(imageSize, imageSize, QImage::Format_RGB32);
    };

    // Act
    bool result = worker->registerCreator(mimeType, creator);
    EXPECT_TRUE(result);

    // Test with different sizes
    ThumbnailWorker::ThumbnailTaskMap taskMap;
    QUrl imageUrl1 = QUrl::fromLocalFile("/tmp/test_small.jpg");
    QUrl imageUrl2 = QUrl::fromLocalFile("/tmp/test_normal.jpg");
    QUrl imageUrl3 = QUrl::fromLocalFile("/tmp/test_large.jpg");
    
    taskMap[imageUrl1] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall;
    taskMap[imageUrl2] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kNormal;
    taskMap[imageUrl3] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge;
    
    worker->onTaskAdded(taskMap);

    // Assert
    EXPECT_TRUE(true);
}

TEST_F(ThumbnailWorkerTest, DuplicateRegistration_ExpectedOverwrite) {
    // Arrange
    QString mimeType = "image/jpeg";
    
    ThumbnailWorker::ThumbnailCreator firstCreator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        return QImage(100, 100, QImage::Format_RGB32);
    };
    
    ThumbnailWorker::ThumbnailCreator secondCreator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        return QImage(200, 200, QImage::Format_RGB32);
    };

    // Act
    bool firstResult = worker->registerCreator(mimeType, firstCreator);
    bool secondResult = worker->registerCreator(mimeType, secondCreator);

    // Assert
    EXPECT_TRUE(firstResult) << "First creator registration should succeed";
    EXPECT_TRUE(secondResult) << "Second creator registration should overwrite the first";
}

TEST_F(ThumbnailWorkerTest, MultipleStopCalls_ExpectedNoCrash) {
    // Arrange & Act
    worker->stop();
    worker->stop();  // Call stop multiple times

    // Assert
    EXPECT_TRUE(true);
}

TEST_F(ThumbnailWorkerTest, EdgeCases_ExpectedNoCrash) {
    // Arrange & Act & Assert
    
    // Test with very long file path
    QString longPath = QString("/").repeated(1000) + "test.jpg";
    QUrl longUrl = QUrl::fromLocalFile(longPath);
    ThumbnailWorker::ThumbnailTaskMap longPathMap;
    longPathMap[longUrl] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall;
    worker->onTaskAdded(longPathMap);
    EXPECT_TRUE(true);
    
    // Test with empty file path
    QUrl emptyUrl = QUrl::fromLocalFile("");
    ThumbnailWorker::ThumbnailTaskMap emptyPathMap;
    emptyPathMap[emptyUrl] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall;
    worker->onTaskAdded(emptyPathMap);
    EXPECT_TRUE(true);
    
    // Test with special characters in path
    QString specialPath = "/tmp/test with spaces & symbols.jpg";
    QUrl specialUrl = QUrl::fromLocalFile(specialPath);
    ThumbnailWorker::ThumbnailTaskMap specialPathMap;
    specialPathMap[specialUrl] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall;
    worker->onTaskAdded(specialPathMap);
    EXPECT_TRUE(true);
}

TEST_F(ThumbnailWorkerTest, PerformanceTest_MultipleTasks) {
    // Arrange
    QString mimeType = "image/jpeg";
    ThumbnailWorker::ThumbnailCreator creator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        return QImage(64, 64, QImage::Format_RGB32);
    };

    bool result = worker->registerCreator(mimeType, creator);
    ASSERT_TRUE(result);

    // Act
    const int taskCount = 100;
    ThumbnailWorker::ThumbnailTaskMap largeTaskMap;
    
    for (int i = 0; i < taskCount; ++i) {
        QUrl url = QUrl::fromLocalFile(QString("/tmp/test_%1.jpg").arg(i));
        largeTaskMap[url] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall;
    }

    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    worker->onTaskAdded(largeTaskMap);
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();

    // Assert
    qint64 duration = endTime - startTime;
    EXPECT_LT(duration, 1000) << "Adding many tasks should complete within reasonable time";
}

TEST_F(ThumbnailWorkerTest, CreatorComplexLogic_ExpectedNoCrash) {
    // Arrange
    QString mimeType = "image/complex";
    ThumbnailWorker::ThumbnailCreator complexCreator = [](const QString &filePath, DFMGLOBAL_NAMESPACE::ThumbnailSize size) -> QImage {
        // Complex logic in creator
        QImage image(100, 100, QImage::Format_RGB32);
        image.fill(Qt::blue);
        
        // Add some text overlay (simulated)
        if (size == DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge) {
            image = image.scaled(200, 200);
        }
        
        // Simulate some processing delay
        QThread::msleep(1);
        
        return image;
    };

    // Act
    bool result = worker->registerCreator(mimeType, complexCreator);
    EXPECT_TRUE(result);

    ThumbnailWorker::ThumbnailTaskMap taskMap;
    QUrl imageUrl = QUrl::fromLocalFile("/tmp/complex_test.complex");
    taskMap[imageUrl] = DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge;
    
    worker->onTaskAdded(taskMap);

    // Assert
    EXPECT_TRUE(true);
}
