#include "shutil/dsqlitehandle.h"
#include "shutil/fileutils.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
    class TestDSqliteHandle : public testing::Test {
    public:
        void SetUp() override
        {
        }
        void TearDown() override
        {
        }

    public:
        // return file folder
        QString getTestFolder()
        {
            return QString("%1/test_shutil/sqlite").arg(QDir::currentPath());
        }

        void makeFolder(const QString& folderPath)
        {
            QDir dir;
            if (!dir.exists(folderPath))
            {
                dir.mkpath(folderPath);
            }
        }

        // return file path
        QString createOneFile(const QString& filename, const QString& folderPath)
        {
            makeFolder(folderPath);

            QString filePath = QString("%1/%2").arg(folderPath).arg(filename);

            QString content= "test for shutil/fileutils.h";
            if(!FileUtils::isFileExists(filePath)){
                FileUtils::writeTextFile(filePath,content);
            }
            return filePath;
        }

        // return file path
        QString createDefaultFile(const QString& filename)
        {
            return createOneFile(filename, getTestFolder());
        }
    };
}

TEST_F(TestDSqliteHandle, can_dispose_clientDatas)
{
    QMap<QString, QList<QString>> filesAndTags;

    EXPECT_TRUE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 0).toList().isEmpty() );
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 1).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 2).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 3).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 4).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 5).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 6).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 7).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 8).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 9).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 10).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 11).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 12).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 13).toBool());

    QString fileName = "sql1.txt";
    QStringList pictureList = FileUtils::filesList("/home/max/Pictures/Wallpapers");
    EXPECT_TRUE( pictureList.count() > 3 );

    QList<QString> tags;
    tags << "tag1" << "tag2";
    filesAndTags[pictureList.first()] = tags;
    filesAndTags[pictureList.takeAt(1)] = tags;
    filesAndTags[pictureList.takeAt(2)] = tags;

    EXPECT_TRUE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 1).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 2).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 3).toBool());
    EXPECT_TRUE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 4).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 5).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 6).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 7).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 8).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 9).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 10).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 11).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 12).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 13).toBool());
}


TEST_F(TestDSqliteHandle, can_get_sda_partions)
{
    std::map<QString, std::multimap<QString, QString>> partionResult = DSqliteHandle::instance()->queryPartionsInfoOfDevices();
    EXPECT_FALSE( partionResult.empty() );
    EXPECT_FALSE( partionResult.at("/dev/sda").empty() );

    std::map<QString, std::multimap<QString, QString>>* newpartionResult = new std::map<QString, std::multimap<QString, QString>>();
    *newpartionResult = partionResult;
    std::unique_ptr<std::map<QString, std::multimap<QString, QString>>> partionsAndMountPoints( newpartionResult);
    QPair<QString, QString> resultPt = DSqliteHandle::instance()->getMountPointOfFile(DUrl::fromLocalFile("/proc/mounts"), partionsAndMountPoints);

    EXPECT_FALSE( resultPt.first.isEmpty() );
}

TEST_F(TestDSqliteHandle, can_monitor_the_mounted_points)
{
    QString dev1 = "/dev/sr0";
    UDiskDeviceInfoPointer infoPointer(new UDiskDeviceInfo(DUrl::fromLocalFile("/dev/sr0"))) ;
    emit deviceListener->mountAdded(infoPointer);

    QString dev2 = "/dev/sr1";
    UDiskDeviceInfoPointer infoPointer2(new UDiskDeviceInfo(DUrl::fromLocalFile("/dev/sr1"))) ;
    emit deviceListener->mountAdded(infoPointer2);

    QMap<QString, QList<QString>> filesAndTags;
    QList<QString> tags;
    tags << "tag1" << "tag2";
    filesAndTags[dev1] = tags;
    filesAndTags[dev2] = tags;

    EXPECT_TRUE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 1).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 2).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 3).toBool());
    EXPECT_TRUE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 4).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 5).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 6).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 7).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 8).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 9).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 10).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 11).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 12).toBool());
    EXPECT_FALSE( DSqliteHandle::instance()->disposeClientData(filesAndTags, 13).toBool());
}

TEST_F(TestDSqliteHandle, can_monitor_the_unmounted_points)
{
    UDiskDeviceInfoPointer infoPointer(new UDiskDeviceInfo(DUrl::fromLocalFile("/dev/sr0"))) ;
    emit deviceListener->mountRemoved(infoPointer);
}
