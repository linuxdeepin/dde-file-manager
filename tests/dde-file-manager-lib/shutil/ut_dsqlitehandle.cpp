#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "shutil/fileutils.h"
#include "testhelper.h"
#include "tag/tagutil.h"
#define private public
#include "shutil/dsqlitehandle.h"

#include "stubext.h"
#include "addr_pri.h"

using namespace stub_ext;

namespace  {
    class TestDSqliteHandle : public testing::Test {
    public:
        void SetUp() override
        {
            m_pHandle = new DSqliteHandle();
            m_filePath = TestHelper::createTmpFile(".txt");
            m_dirPath = TestHelper::createTmpDir();
            m_tagNameAlpha = "ut_sqlit_alpha";
            m_tagNameBeta = "ut_sqlit_beta";
        }

        void TearDown() override
        {
            if (m_pHandle) {
                delete m_pHandle;
                m_pHandle = nullptr;
            }

            TestHelper::deleteTmpFiles(QStringList() << m_filePath << m_dirPath);
        }

        DSqliteHandle *m_pHandle;
        QString m_filePath;
        QString m_dirPath;
        QString m_tagNameAlpha;
        QString m_tagNameBeta;

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


static int nextCount = 0;

TEST_F(TestDSqliteHandle, can_dispose_clientDatas)
{
    ASSERT_NE(m_pHandle, nullptr);
    QMap<QString, QList<QString>> filesAndTags;

    StubExt st;
    st.set_lamda((bool(QSqlQuery::*)(const QString&))ADDR(QSqlQuery, exec), []{return true;});
    st.set_lamda(&QSqlQuery::next, []{return ++nextCount > 3 ? false : true;});
    st.set_lamda((QVariant(QSqlQuery::*)(const QString&) const)ADDR(QSqlQuery, value), []{return QVariant("ut_qurey_value" + nextCount);});
    st.set_lamda((QVariant(QSqlQuery::*)(int) const)ADDR(QSqlQuery, value), []{return QVariant("ut_qurey_value" + nextCount);});
    st.set_lamda(&DSqliteHandle::checkDBFileExist, []{return DSqliteHandle::ReturnCode::Exist;});

    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 0).toList().isEmpty() );
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 1).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 2).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 3).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 4).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 5).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 6).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 7).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 8).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 9).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 10).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 11).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 12).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 13).toBool());

    QString fileName = "sql1.txt";
    QStringList pictureList = FileUtils::filesList(QString("%1/Pictures/Wallpapers").arg(QDir::homePath()));
    EXPECT_TRUE( pictureList.count() > 3 );
    ASSERT_TRUE( pictureList.count() > 3 );

    QList<QString> tags;
    tags << "tag1" << "tag2";
    filesAndTags[pictureList.first()] = tags;
    filesAndTags[pictureList.takeAt(1)] = tags;
    filesAndTags[pictureList.takeAt(2)] = tags;

    st.reset(&DSqliteHandle::checkDBFileExist);
    st.set_lamda(&DSqliteHandle::checkDBFileExist, []{return DSqliteHandle::ReturnCode::NoExist;});
    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 1).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 2).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 3).toBool());
    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 4).toBool());
    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 5).toBool());
    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 6).toBool());
    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 7).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 8).toBool());
    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 9).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 10).toBool());
    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 11).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 12).toBool());
    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 13).toBool());
}


TEST_F(TestDSqliteHandle, can_get_sda_partions)
{
    ASSERT_NE(m_pHandle, nullptr);

    StubExt st;
    nextCount = 0;
    st.set_lamda((bool(QSqlQuery::*)(const QString&))ADDR(QSqlQuery, exec), []{return true;});
    st.set_lamda(&QSqlQuery::next, []{return ++nextCount > 3 ? false : true;});

    std::map<QString, std::multimap<QString, QString>> partionResult = m_pHandle->queryPartionsInfoOfDevices();
    EXPECT_FALSE( partionResult.empty() );
//    EXPECT_FALSE( partionResult.at("/dev/sda").empty() );

    std::map<QString, std::multimap<QString, QString>>* newpartionResult = new std::map<QString, std::multimap<QString, QString>>();
    *newpartionResult = partionResult;
    std::unique_ptr<std::map<QString, std::multimap<QString, QString>>> partionsAndMountPoints( newpartionResult);
    QPair<QString, QString> resultPt = m_pHandle->getMountPointOfFile(DUrl::fromLocalFile("/proc/mounts"), partionsAndMountPoints);

    EXPECT_FALSE( resultPt.first.isEmpty() );
}

TEST_F(TestDSqliteHandle, can_monitor_the_mounted_points)
{
    ASSERT_NE(m_pHandle, nullptr);

    StubExt st;
    nextCount = 0;
    st.set_lamda((bool(QSqlQuery::*)(const QString&))ADDR(QSqlQuery, exec), []{return true;});
    st.set_lamda(&QSqlQuery::next, []{return ++nextCount > 3 ? false : true;});
    st.set_lamda((QVariant(QSqlQuery::*)(const QString&) const)ADDR(QSqlQuery, value), []{return QVariant("ut_qurey_value" + nextCount);});
    st.set_lamda((QVariant(QSqlQuery::*)(int) const)ADDR(QSqlQuery, value), []{return QVariant("ut_qurey_value" + nextCount);});
    st.set_lamda(&DSqliteHandle::checkDBFileExist, []{return DSqliteHandle::ReturnCode::NoThisDir;});

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

    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 1).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 2).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 3).toBool());
    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 4).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 5).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 6).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 7).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 8).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 9).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 10).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 11).toBool());
    nextCount = 0;
    EXPECT_FALSE( m_pHandle->disposeClientData(filesAndTags, 12).toBool());
    nextCount = 0;
    EXPECT_TRUE( m_pHandle->disposeClientData(filesAndTags, 13).toBool());
}

TEST_F(TestDSqliteHandle, can_monitor_the_unmounted_points)
{
    UDiskDeviceInfoPointer infoPointer(new UDiskDeviceInfo(DUrl::fromLocalFile("/dev/sr0"))) ;
    emit deviceListener->mountRemoved(infoPointer);
}

TEST_F(TestDSqliteHandle, test_before_tag_files)
{
    ASSERT_NE(m_pHandle, nullptr);
    QMap<QString, QList<QString>> filesAndTags;

    filesAndTags[m_tagNameAlpha] =QList<QString>{ Tag::ColorName.at(0) };
    filesAndTags[m_tagNameBeta] =QList<QString>{ Tag::ColorName.at(1) };

    EXPECT_FALSE(m_pHandle->disposeClientData(filesAndTags, 11).toBool());

    StubExt st;
    st.set_lamda((bool(QSqlQuery::*)(const QString&))ADDR(QSqlQuery, exec), []{return true;});
    st.set_lamda(&QSqlQuery::next, []{return true;});
    EXPECT_TRUE(m_pHandle->disposeClientData(filesAndTags, 11).toBool());
}
