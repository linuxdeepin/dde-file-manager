#include "shutil/fileutils.h"
#include "shutil/desktopfile.h"
#include "interfaces/dfmstandardpaths.h"
#include "app/define.h"
#include "dfileservices.h"

#include <QJsonArray>
#include <QJsonObject>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
    class TestFileUtils : public testing::Test {
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
            return QString("%1/test_shutil/fileutils").arg(QDir::currentPath());
        }

        void makeFolder(const QString& folderPath)
        {
            QDir dir;
            if (!dir.exists(folderPath))
            {
                dir.mkpath(folderPath);
            }
            DUrl url = DUrl::fromLocalFile(folderPath);
            FileUtils::mkpath(url);
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

        // return file path
        QString getDesktopFile(const QString& filename)
        {
            return QString("/usr/share/applications/%1").arg(filename);
        }

        bool copyFileToPath(QString sourcefile ,QString toDir, QString destfile)
        {
            if (!QFile::exists(sourcefile)){
                return false;
            }

            if (QFile::exists(destfile)){
                return true;
            }

            makeFolder(toDir);

            if(!QFile::copy(sourcefile, toDir))
            {
                return false;
            }
            return true;
        }

        QJsonArray getJsonArry()
        {
            QJsonArray array = { 1, 2.2, "string" };
            return array;
        }

        QJsonObject getJsonObjects()
        {
            QJsonObject object
              {
                  {"p1", 1},
                  {"p2", 2.2},
                  {"p3", "string"}
              };
            return object;
        }
    };
}

TEST_F(TestFileUtils, can_remove_file)
{
    QString filename = "fileutil_file1.txt";
    QString folderPath = getTestFolder();

    QString filePath = createOneFile(filename,folderPath );

    EXPECT_TRUE( FileUtils::isFileExists(filePath));

    FileUtils::removeRecurse(folderPath, filename);

    EXPECT_FALSE( FileUtils::isFileExists(filePath));
}

TEST_F(TestFileUtils, can_get_txt_files_info)
{
    QString filename = "fileutil_file_new1.txt";
    QString defaultfolderPath = getTestFolder();
    QString fileutil_t1 = createDefaultFile("fileutil_t1.txt" );
    QString fileutil_t2 = createDefaultFile("fileutil_t2.txt" );
    QString subfolderPath = QString("%1/sub").arg(getTestFolder());
    QStringList filelist;
    filelist.push_back(filename);

    EXPECT_FALSE( FileUtils::openFile(filename));
    EXPECT_FALSE( FileUtils::openFiles(filelist));

    QString filePath = createOneFile(filename,subfolderPath );
    filelist.clear();
    filelist.push_back(filePath);

    EXPECT_EQ( "txt", FileUtils::getRealSuffix(filePath));
    EXPECT_TRUE( FileUtils::isFileExists(filePath));
    EXPECT_TRUE( FileUtils::openFile(filePath));
    EXPECT_TRUE( FileUtils::openFiles(filelist));
    EXPECT_TRUE( FileUtils::getFileContent(filePath).contains("shutil/fileutils"));

    EXPECT_FALSE( FileUtils::getKernelParameters().isEmpty());

    EXPECT_FALSE( FileUtils::searchGenericIcon(filePath).isNull());
    EXPECT_FALSE( FileUtils::isArchive(filePath));
    EXPECT_FALSE( FileUtils::isDesktopFile(filePath));
    EXPECT_FALSE( FileUtils::isFileRunnable(filePath));
    EXPECT_FALSE( FileUtils::isGvfsMountFile(filePath));
    EXPECT_FALSE( FileUtils::isFileExecutable(fileutil_t1));
    EXPECT_FALSE( FileUtils::isFileManagerSelf(filePath));
    EXPECT_FALSE( FileUtils::isExecutableScript(fileutil_t2));
    EXPECT_FALSE( FileUtils::isFileWindowsUrlShortcut(filePath));
    EXPECT_FALSE( FileUtils::isDesktopFile(QFileInfo(filePath)));

    FileUtils::getInternetShortcutUrl(fileutil_t2);

    filelist.clear();
    FileUtils::recurseFolder(defaultfolderPath, defaultfolderPath, &filelist);

    EXPECT_TRUE(FileUtils::filesCount(defaultfolderPath));

    QStringList listDefaultFiles = FileUtils::filesList(defaultfolderPath);
    EXPECT_FALSE(listDefaultFiles.empty());

    DUrlList fileurllist;
    EXPECT_EQ( DFMGlobal::MenuExtension::EmptyArea, FileUtils::getMenuExtension(fileurllist));

    foreach (QString fileItem, listDefaultFiles) {
        fileurllist.push_back(DUrl::fromLocalFile(fileItem));
    }
    bool limited = false;
    FileUtils::getMenuExtension(fileurllist);
    qint64 total1 = FileUtils::totalSize(defaultfolderPath);
    qint64 total2 = FileUtils::totalSize(fileurllist);
    qint64 total3 = FileUtils::totalSize(fileurllist,1000, limited);

    EXPECT_TRUE(total1);
    EXPECT_EQ(total2, total3);
}

static QString DESK_TOP_FILE_PATH = "/usr/share/applications";
TEST_F(TestFileUtils, can_get_app_file_details)
{
    QStringList applist = FileUtils::getApplicationNames();
    EXPECT_FALSE( applist.isEmpty());

    DesktopFile desktopEmptyfile("nofile");
    EXPECT_FALSE( FileUtils::searchAppIcon(desktopEmptyfile).isNull()); // get default icon

    QString desktopfilePath = getDesktopFile("dde-computer.desktop");
    QStringList filelist;
    filelist.push_back(desktopfilePath);

    FileUtils::openFile(desktopfilePath);
    FileUtils::openFiles(filelist);
    EXPECT_TRUE( FileUtils::isDesktopFile(desktopfilePath));

    DesktopFile desktopfile(desktopfilePath);
    filelist.clear();
    EXPECT_FALSE( FileUtils::openFilesByApp(desktopfilePath,filelist));

    filelist.push_back("/home/max/Desktop");
    filelist.push_back("/home/max/Videos");
    filelist.push_back("/home/max/Pictures");
    EXPECT_TRUE( FileUtils::openFilesByApp(desktopfilePath,filelist));
    EXPECT_FALSE( FileUtils::searchAppIcon(desktopfile).isNull());

    EXPECT_TRUE( FileUtils::launchApp(desktopfilePath, filelist));
    EXPECT_TRUE( FileUtils::launchAppByGio(desktopfilePath,filelist));
    EXPECT_TRUE( FileUtils::launchAppByDBus(desktopfilePath,filelist));

    EXPECT_FALSE( FileUtils::shouldAskUserToAddExecutableFlag(desktopfilePath));
}


TEST_F(TestFileUtils, can_format_Udisk_Size)
{
    quint64 usedSize = 0,totalSize = 0;

    QString srcfile = "/home/max/.config/deepin/dde-file-manager/dde-file-manager.json";
    QString destdir = DFMStandardPaths::location(DFMStandardPaths::ApplicationConfigPath);
    QString destfile = QString("%1/dde-file-manager.json").arg(destdir);

    copyFileToPath(srcfile, destdir, destfile);

    EXPECT_EQ( "0 B", FileUtils::formatSize(0));
    EXPECT_EQ( "0 B", FileUtils::formatSize(-1));
    EXPECT_EQ( "0M", FileUtils::diskUsageString(usedSize, totalSize, "dev/sr0"));
    EXPECT_EQ( "0M", FileUtils::defaultOpticalSize("dev/sr0",usedSize, totalSize));

    usedSize = 1024;
    totalSize = 2048;
    EXPECT_EQ( "1 KB", FileUtils::formatSize(usedSize));
    EXPECT_EQ( "0M/0M", FileUtils::diskUsageString(usedSize, totalSize, "dev/sr0"));
    EXPECT_EQ( "0M", FileUtils::defaultOpticalSize("dev/sr0",usedSize, totalSize));
}

TEST_F(TestFileUtils, can_handle_disk_string)
{
    EXPECT_FALSE(FileUtils::deviceShouldBeIgnore("/dev"));
    EXPECT_FALSE(FileUtils::deviceShouldBeIgnore("/dev/sdbfff0"));

    FileUtils::mountAVFS();
    FileUtils::umountAVFS();
}


TEST_F(TestFileUtils, can_new_name_docment)
{
    QString filename = "fileutil_file_new1.txt";
    QString defaultfolderPath = getTestFolder();
    QString subfolderPath = QString("%1/sub").arg(getTestFolder());
    QStringList filelist;
    filelist.push_back(filename);

    FileUtils::removeRecurse(subfolderPath, "");
    EXPECT_TRUE( FileUtils::newDocmentName(subfolderPath,"fileutil_file_new1","txt").isSimpleText());

    DUrl url = DUrl::fromLocalFile(subfolderPath);
    DAbstractFileInfoPointer info = fileService->createFileInfo(nullptr, url);
    EXPECT_TRUE( FileUtils::newDocumentUrl(info,"fileutil_file_new1","txt").isValid());

    QString filePath = createOneFile(filename,subfolderPath );

    filelist.clear();
    filelist.push_back(filePath);

    EXPECT_TRUE( FileUtils::newDocmentName(subfolderPath,filename,"").isSimpleText());
    EXPECT_TRUE( FileUtils::newDocumentUrl(info,filename,"").isValid());

    WId wid;
    EXPECT_FALSE(FileUtils::cpTemplateFileToTargetDir(subfolderPath,"fileutil_file_new2","txt",wid));
    EXPECT_EQ( "txt", FileUtils::getRealSuffix(filePath));
}

TEST_F(TestFileUtils, can_encode_string)
{
    QString filename = "fileutil_file_new1.txt";
    QString defaultfolderPath = getTestFolder();
    QString subfolderPath = QString("%1/sub").arg(getTestFolder());
    QString filePath = createOneFile(filename,subfolderPath );

    EXPECT_EQ(32, FileUtils::md5(filePath).length());

    QFile file(filePath);
    EXPECT_EQ(32, FileUtils::md5(&file,filePath).length());

    QFile emptyfile("fileutil_file_new111.txt");
    EXPECT_EQ(32, FileUtils::md5(&file,filePath).length());
}

TEST_F(TestFileUtils, can_not_run_normalfile_as_ex)
{
    QString filename = "fileutil_file_new1.txt";
    QString defaultfolderPath = getTestFolder();
    QString subfolderPath = QString("%1/sub").arg(getTestFolder());
    QString filePath = createOneFile(filename,subfolderPath );

    EXPECT_FALSE(FileUtils::isFileExecutable(filePath));
    EXPECT_FALSE(FileUtils::openExcutableScriptFile(filePath,0));
    EXPECT_TRUE(FileUtils::openExcutableScriptFile(filePath,1));

    EXPECT_FALSE(FileUtils::addExecutableFlagAndExecuse(filePath,0));
    EXPECT_TRUE(FileUtils::addExecutableFlagAndExecuse(filePath,1));

    EXPECT_FALSE(FileUtils::openExcutableFile(filePath,0));
    EXPECT_TRUE(FileUtils::openExcutableFile(filePath,1));

    EXPECT_FALSE(FileUtils::openExcutableFile(filePath,0));
    EXPECT_TRUE(FileUtils::openExcutableFile(filePath,1));
}

TEST_F(TestFileUtils, can_set_the_background)
{
    QString sysPicturePath = "/home/max/Pictures/Wallpapers";
    EXPECT_TRUE(FileUtils::displayPath(sysPicturePath).contains("~"));

    EXPECT_EQ("png", FileUtils::imageFormatName(QImage::Format_ARGB32) );
    EXPECT_EQ("jpeg", FileUtils::imageFormatName(QImage::Format_RGB32));

    QStringList listPicutures = FileUtils::filesList(sysPicturePath);
    EXPECT_FALSE(listPicutures.empty());

    foreach (QString onefile, listPicutures) {
        if(FileUtils::getRealSuffix(onefile).contains("jpg")){
            EXPECT_TRUE(FileUtils::setBackground(onefile) );
            break;
        }
    }
}

TEST_F(TestFileUtils, can_read_write_json_file)
{
    QString folderPath = getTestFolder();

    QString objName = "jsonObj.json";
    QString arryName = "jsonArry.json";

    FileUtils::removeRecurse(folderPath,objName);
    FileUtils::removeRecurse(folderPath,arryName);

    QString filePathObj = createOneFile(objName,folderPath );
    QString filePathArry = createOneFile(arryName,folderPath );

    EXPECT_TRUE(FileUtils::writeJsonObjectFile(filePathObj, getJsonObjects()));
    EXPECT_TRUE(FileUtils::writeJsonnArrayFile(filePathArry, getJsonArry()));

    QJsonObject jsonObject = FileUtils::getJsonObjectFromFile(filePathObj);
    EXPECT_FALSE(jsonObject.isEmpty());
    EXPECT_EQ(1,jsonObject.value("p1").toInt());
    EXPECT_EQ(2.2,jsonObject.value("p2").toDouble());
    EXPECT_EQ("string",jsonObject.value("p3").toString());

    QJsonArray jsonArry = FileUtils::getJsonArrayFromFile(filePathArry);
    EXPECT_EQ(3,jsonArry.count());
    EXPECT_EQ(1,jsonArry.at(0).toInt());
    EXPECT_EQ(2.2,jsonArry.at(1).toDouble());
    EXPECT_EQ("string",jsonArry.at(2).toString());
}
