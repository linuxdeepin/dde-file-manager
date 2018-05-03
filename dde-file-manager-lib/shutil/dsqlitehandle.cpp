

#include <string>
#include <fstream>
#include <algorithm>
#include <functional>
#include <unordered_set>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "singleton.h"
#include "tag/tagutil.h"
#include "dsqlitehandle.h"

#include <QDir>
#include <QList>
#include <QColor>
#include <QProcess>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

#include "pwd.h"
#include "unistd.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/



namespace std
{

template<>
struct less<QString>
{
    typedef bool result_type;
    typedef QString first_argument_type;
    typedef QString second_argument_type;
    ///###: delete the codes above. When in c++17.


    bool operator()(const QString& lhs, const QString& rhs) const
    {
        std::string lhStr{ lhs.toStdString() };
        std::string rhStr{ rhs.toStdString() };

        return (std::less<std::string>{}(lhStr, rhStr));
    }
};

}


static std::once_flag flag{};//###: flag for instance.
static std::atomic<int> counter{ 0 };

static constexpr const char* const MOUNTTABLE{"/proc/mounts"};
static constexpr const char* const ROOTPATH{"/"};
static constexpr const std::size_t MAXTHREAD{ 3 };
static constexpr const char* const CONNECTIONNAME{ "deep" };
static constexpr const char* const USERNAME{"username"};
static constexpr const char* const PASSWORD{"password"};


static const std::map<QString, QString> StrTableOfEscapeChar{
                                                                {"\\007","\a"},
                                                                {"\\010","\b"},
                                                                {"\\014","\f"},
                                                                {"\\012","\n"},
                                                                {"\\015","\r"},
                                                                {"\\011","\t"},
                                                                {"\\013","\v"},
                                                                {"\\134","\\"},
                                                                {"\\047","\'"},
                                                                {"\\042","\""},
                                                                {"\\040"," "}
                                                                };


static const std::multimap<DSqliteHandle::SqlType, QString> SqlTypeWithStrs {
                                                          {DSqliteHandle::SqlType::BeforeTagFiles, "SELECT COUNT(tag_property.tag_name) AS counter "
                                                                                                   "FROM tag_property WHERE tag_property.tag_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::BeforeTagFiles, "INSERT INTO tag_property (tag_name, tag_color) "
                                                                                                                    "VALUES(\'%1\', \'%2\')"},

                                                          {DSqliteHandle::SqlType::TagFiles, "SELECT COUNT (tag_with_file.file_name) AS counter "
                                                                                                                        "FROM tag_with_file WHERE tag_with_file.file_name = \'%1\' "
                                                                                                                        "AND tag_with_file.tag_name = \'%2\'"},
                                                          {DSqliteHandle::SqlType::TagFiles, "INSERT INTO tag_with_file (file_name, tag_name) VALUES (\'%1\', \'%2\')"},
                                                          {DSqliteHandle::SqlType::TagFiles, "DELETE FROM tag_with_file WHERE tag_with_file.tag_name = \'%1\' "
                                                                                                                              "AND tag_with_file.file_name = \'%2\'"},
                                                          {DSqliteHandle::SqlType::TagFiles, "DELETE FROM file_property WHERE file_property.file_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::TagFiles, "SELECT tag_with_file.tag_name FROM tag_with_file "
                                                                                                                                "WHERE tag_with_file.file_name = \'%1\'" },
                                                          {DSqliteHandle::SqlType::TagFiles, "UPDATE file_property SET tag_1 = \'%1\', tag_2 = \'%2\', tag_3 = \'%3\' "
                                                                                                                                "WHERE file_property.file_name = \'%4\'" },
                                                          {DSqliteHandle::SqlType::TagFiles, "SELECT COUNT (file_property.file_name) AS counter "
                                                                                                                            "FROM file_property WHERE file_property.file_name = \'%1\'" },
                                                          {DSqliteHandle::SqlType::TagFiles, "INSERT INTO file_property (file_name, tag_1, tag_2, tag_3) "
                                                                                                                            "VALUES(\'%1\', \'%2\', \'%3\', \'%4\')"},

                                                          {DSqliteHandle::SqlType::ChangeFilesName, "UPDATE file_property SET file_name = \'%1\' "
                                                                                                                                            "WHERE file_property.file_name = \'%2\'"},
                                                          {DSqliteHandle::SqlType::ChangeFilesName, "UPDATE tag_with_file SET file_name = \'%1\' "
                                                                                                                                        "WHERE tag_with_file.file_name = \'%2\'"},

                                                          {DSqliteHandle::SqlType::ChangeTagsName, "UPDATE file_property SET tag_1 = \'%1\' "
                                                                                                                                        "WHERE file_property.tag_1 = \'%2\'"},
                                                          {DSqliteHandle::SqlType::ChangeTagsName, "UPDATE file_property SET tag_2 = \'%1\' "
                                                                                                                                        "WHERE file_property.tag_2 = \'%2\'"},
                                                          {DSqliteHandle::SqlType::ChangeTagsName, "UPDATE file_property SET tag_3 = \'%1\' "
                                                                                                                                         "WHERE file_property.tag_3 = \'%2\'"},
                                                          {DSqliteHandle::SqlType::ChangeTagsName, "UPDATE tag_with_file SET tag_name = \'%1\' "
                                                                                                                                           "WHERE tag_with_file.tag_name = \'%2\'"},
                                                          {DSqliteHandle::SqlType::ChangeTagsName2, "UPDATE tag_property SET tag_name = \'%1\' "
                                                                                                                                           "WHERE tag_property.tag_name = \'%2\'"},

                                                          {DSqliteHandle::SqlType::DeleteFiles, "DELETE FROM tag_with_file WHERE tag_with_file.tag_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::DeleteFiles, "DELETE FROM file_property WHERE file_property.file_name = \'%1\'"},

                                                          {DSqliteHandle::SqlType::DeleteTags, "DELETE FROM tag_with_file WHERE tag_with_file.tag_name = \'%1\'"},

                                                          {DSqliteHandle::SqlType::DeleteTags2, "SELECT file_property.file_name FROM file_property WHERE file_property.tag_1 = \'%1\' "
                                                                                                                                                    "OR file_property.tag_2 = \'%1\' "
                                                                                                                                                    "OR file_property.tag_3 = \'%1\'"},
                                                          {DSqliteHandle::SqlType::DeleteTags2, "SELECT tag_with_file.tag_name FROM tag_with_file WHERE tag_with_file.file_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::DeleteTags2, "UPDATE file_property SET tag_1 = \'%1\', tag_2 =\'%2\', tag_3=\'%3\' "
                                                                                                                                        "WHERE file_name = \'%4\'"},
                                                          {DSqliteHandle::SqlType::DeleteTags3, "DELETE FROM tag_property WHERE tag_name = \'%1\'"},

                                                          {DSqliteHandle::SqlType::UntagSamePartionFiles, "DELETE FROM tag_with_file WHERE tag_with_file.file_name = \'%1\' "
                                                                                                          "AND tag_with_file.tag_name = \'%2\'"},

                                                          {DSqliteHandle::SqlType::UntagSamePartionFiles2, "SELECT COUNT(tag_with_file.tag_name) AS counter "
                                                                                                          "FROM tag_with_file WHERE tag_with_file.file_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::UntagSamePartionFiles2, "SELECT tag_with_file.tag_name FROM tag_with_file "
                                                                                                          "WHERE tag_with_file.file_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::UntagSamePartionFiles2, "UPDATE file_property SET tag_1 = \'%1\', tag_2 = \'%2\', tag_3 = \'%3' "
                                                                                                          "WHERE file_property.file_name = \'%4\'"},
                                                          {DSqliteHandle::SqlType::UntagSamePartionFiles2, "DELETE FROM file_property WHERE file_property.file_name = \'%1\'"},

                                                          {DSqliteHandle::SqlType::UntagDiffPartionFiles, "DELETE FROM tag_with_file WHERE tag_with_file.file_name = \'%1\' "
                                                                                                          "AND tag_with_file.tag_name = \'%2\'"},
                                                          {DSqliteHandle::SqlType::UntagDiffPartionFiles, "SELECT COUNT(tag_with_file.file_name) AS counter "
                                                                                                                  "FROM tag_with_file WHERE tag_with_file.file_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::UntagDiffPartionFiles, "DELETE FROM file_property WHERE file_property.file_name = \'%1\'"},

                                                          {DSqliteHandle::SqlType::GetTagsThroughFile, "SELECT tag_with_file.tag_name FROM tag_with_file "
                                                                                                                                     "WHERE tag_with_file.file_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::GetFilesThroughTag, "SELECT tag_with_file.file_name FROM tag_with_file "
                                                                                                                                       "WHERE tag_with_file.tag_name = \'%1\'"},

                                                          {DSqliteHandle::SqlType::TagFilesThroughColor, "SELECT COUNT(tag_with_file.tag_name) AS counter FROM tag_with_file "
                                                                                                          "WHERE tag_with_file.tag_name = \'%1\' AND tag_with_file.file_name = \'%2\'"},
                                                          {DSqliteHandle::SqlType::TagFilesThroughColor, "INSERT INTO tag_with_file (tag_name, file_name) VALUES(\'%1\', \'%2\')"},
                                                          {DSqliteHandle::SqlType::TagFilesThroughColor, "SELECT tag_with_file.tag_name FROM tag_with_file "
                                                                                                                                        "WHERE tag_with_file.file_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::TagFilesThroughColor, "INSERT INTO file_property (file_name, tag_1, tag_2, tag_3)  "
                                                                                                         "VALUES (\'%1\', \'%2\', \'%3\', \'%4\')"},

                                                          {DSqliteHandle::SqlType::TagFilesThroughColor2, "SELECT COUNT(file_property.file_name) AS counter FROM file_property "
                                                                                                          "WHERE file_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::TagFilesThroughColor2, "UPDATE file_property SET tag_1 = \'%1\', tag_2 = \'%2\', tag_3 = \'%3' "
                                                                                                          "WHERE file_property.file_name = \'%4\'"},
                                                          {DSqliteHandle::SqlType::TagFilesThroughColor3, "SELECT COUNT(tag_property.tag_name) AS counter FROM tag_property "
                                                                                                          "WHERE tag_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::TagFilesThroughColor3, "INSERT INTO tag_property(tag_name, tag_color) VALUES(\'%1\', \'%2\')"},


                                                          {DSqliteHandle::SqlType::GetAllTags, "SELECT * FROM tag_property"},

                                                          {DSqliteHandle::SqlType::GetTagColor, "SELECT * FROM tag_property WHERE tag_property.tag_name = \'%1\'" },

                                                          {DSqliteHandle::SqlType::ChangeTagColor, "UPDATE tag_property SET tag_color = \'%1\' "
                                                                                                   "WHERE tag_property.tag_name = \'%2\' AND tag_property.tag_color = \'%3\'" }
                                                      };


DSqliteHandle::DSqliteHandle(QObject * const parent)
              :QObject{ parent },
               m_sqlDatabasePtr{ new QSqlDatabase }
{
    std::lock_guard<std::mutex> raiiLock{ m_mutex };
    std::map<QString, std::multimap<QString, QString>> partionsAndMounPoints{ DSqliteHandle::queryPartionsInfoOfDevices() };

    if(!partionsAndMounPoints.empty()){
        m_partionsOfDevices.reset(new std::map<QString, std::multimap<QString, QString>>{ partionsAndMounPoints } );
    }

    this->initializeConnect();
}


///###: this is a auxiliary function. so do nont need a mutex.
QPair<QString, QString> DSqliteHandle::getMountPointOfFile(const DUrl& url,
                                                           std::unique_ptr<std::map<QString, std::multimap<QString, QString>>>& partionsAndMountPoints)
{
    QPair<QString, QString> partionAndMountPoint{};

    if(DFileInfo::exists(url) && partionsAndMountPoints
                              && !partionsAndMountPoints->empty()){
        QString parentPath{url.parentUrl().path()};
        std::multimap<QString, QString>::const_iterator partionAndMounpointItr{};
        std::multimap<QString, QString>::const_iterator rootPathPartionAndMountpointItr{};
        std::map<QString, std::multimap<QString, QString>>::const_iterator cbeg{ partionsAndMountPoints->cbegin() };
        std::map<QString, std::multimap<QString, QString>>::const_iterator cend{ partionsAndMountPoints->cend() };

        for(; cbeg != cend; ++cbeg){
            std::multimap<QString, QString>::const_iterator itrOfPartionAndMountpoint{ cbeg->second.cbegin() };
            std::multimap<QString, QString>::const_iterator itrOfPartionAndMountpointEnd{ cbeg->second.cend() };
            bool flag{ false };

            for(; itrOfPartionAndMountpoint != itrOfPartionAndMountpointEnd; ++itrOfPartionAndMountpoint){

                if(itrOfPartionAndMountpoint->second == ROOTPATH){
                    rootPathPartionAndMountpointItr = itrOfPartionAndMountpoint;
                }

                if(itrOfPartionAndMountpoint->second != ROOTPATH && parentPath.startsWith(itrOfPartionAndMountpoint->second)){
                    partionAndMounpointItr = itrOfPartionAndMountpoint;
                    flag = true;
                    break;
                }
            }

            if(flag){
                break;
            }
        }

        if(partionAndMounpointItr != std::multimap<QString, QString>::const_iterator{}){
            partionAndMountPoint.first = partionAndMounpointItr->first;
            partionAndMountPoint.second = partionAndMounpointItr->second;
        }


        if(partionAndMounpointItr == std::multimap<QString, QString>::const_iterator{} && parentPath.startsWith(ROOTPATH)){
            partionAndMountPoint.first = rootPathPartionAndMountpointItr->first;
            partionAndMountPoint.second = rootPathPartionAndMountpointItr->second;
        }
    }

    return partionAndMountPoint;
}




///###: this is a auxiliary function. so do not need a mutex.
std::map<QString, std::multimap<QString, QString>>  DSqliteHandle::queryPartionsInfoOfDevices()
{
    std::map<QString, std::multimap<QString, QString>> partionsAndMountpoints{};

    if(DFileInfo::exists(DUrl::fromLocalFile(MOUNTTABLE))){
        std::basic_ifstream<char> iFstream{MOUNTTABLE};
        std::list<std::basic_string<char>> partionInfoAndMountpoint{};
        std::basic_regex<char> matchNumber{ "[0-9]+" };

        if(iFstream.is_open()){
            std::basic_string<char> str{};


            while(std::getline(iFstream, str)){

                if(!str.empty()){
                    std::basic_string<char>::size_type pos{ str.find("/dev/") };

                    if(pos != std::string::npos){
                        partionInfoAndMountpoint.push_back(str);
                    }
                }
            }
        }

        for(const std::basic_string<char>& theStr : partionInfoAndMountpoint){
            QString qsTr{ QString::fromStdString(theStr) };
            QString deviceName{};
            QString partionName{};
            QString mountpoint{};

            QList<QString> subsTrs{ qsTr.split(' ') };
            partionName = subsTrs[0];
            mountpoint = subsTrs[1];

            std::match_results<std::basic_string<char>::const_iterator> matchedResult{};
            std::basic_string<char> partionNameStd{ partionName.toStdString() };

            if(std::regex_search(partionNameStd, matchedResult, matchNumber)){
                deviceName = QString::fromStdString( matchedResult.prefix() );

                if(!deviceName.isEmpty() && !deviceName.isNull()){
                    mountpoint = DSqliteHandle::restoreEscapedChar(mountpoint);

                    ///###: do not delete this.
//                    qDebug()<< deviceName << partionName << mountpoint;

                    partionsAndMountpoints[deviceName].emplace(std::move(partionName), mountpoint);
                }
            }
        }
    }

    ///###: do not delete this. It is convenient for testing.
//    for(const auto& item : partionsAndMountpoints){
//        qDebug()<< item.first;

//        for(const auto& itm : item.second){
//            qDebug()<< itm.first << "=====" <<itm.second;
//        }

//        qDebug()<< "\n";
//    }

    return partionsAndMountpoints;
}

DSqliteHandle* DSqliteHandle::instance()
{
    static DSqliteHandle* theInstance{ new DSqliteHandle };
    return theInstance;
}

void DSqliteHandle::onMountAdded(UDiskDeviceInfoPointer infoPointer)
{
    (void)infoPointer;
    m_flag.store(true, std::memory_order_release);

    std::lock_guard<std::mutex> raiiLock{ m_mutex };
    std::map<QString, std::multimap<QString, QString>> partionsAndMountPoints{ DSqliteHandle::queryPartionsInfoOfDevices() };
    m_partionsOfDevices.reset(nullptr);

    if(!partionsAndMountPoints.empty()){
        m_partionsOfDevices = std::unique_ptr<std::map<QString, std::multimap<QString, QString>>>{
                                          new std::map<QString, std::multimap<QString, QString>>{ partionsAndMountPoints }
                                                                                                 };
    }
    m_flag.store(false, std::memory_order_release);
}

void DSqliteHandle::onMountRemoved(UDiskDeviceInfoPointer infoPointer)
{
    (void)infoPointer;

    m_flag.store(true, std::memory_order_release);
    std::lock_guard<std::mutex> raiiLock{ m_mutex };
    std::map<QString, std::multimap<QString, QString>> partionsAndMountPoints{ DSqliteHandle::queryPartionsInfoOfDevices() };
    m_partionsOfDevices.reset(nullptr);

    if(!partionsAndMountPoints.empty()){
        m_partionsOfDevices = std::unique_ptr<std::map<QString, std::multimap<QString, QString>>>{
                                          new std::map<QString, std::multimap<QString, QString>>{ partionsAndMountPoints }
                                                                                                 };
    }
    m_flag.store(false, std::memory_order_release);
}


QString DSqliteHandle::restoreEscapedChar(const QString& value)
{
    QString tempValue{ value };

    if(!tempValue.isEmpty() && !tempValue.isNull()){

        int pos{ -1 };
        std::map<QString, QString>::const_iterator tableCBeg{ StrTableOfEscapeChar.cbegin() };
        std::map<QString, QString>::const_iterator tableCEnd{ StrTableOfEscapeChar.cend() };

        for(; tableCBeg != tableCEnd; ++tableCBeg){
            pos = tempValue.indexOf(tableCBeg->first);

            if(pos != -1){

                while(pos != -1){
                    tempValue = tempValue.replace(tableCBeg->first, tableCBeg->second);

                    pos = tempValue.indexOf(tableCBeg->first);
                }
            }
        }
    }

    return tempValue;
}


///#:-----------------------------------------> <tagName, <tagColor, <files>>>.
QVariant DSqliteHandle::disposeClientData(const QMap<QString, QList<QString>>& filesAndTags, const unsigned long long& type)
{
    QVariant var{};

    if(counter.load(std::memory_order_consume) != 0){
        return var;
    }

    std::size_t numberOfType{ static_cast<std::size_t>(type) };

    if(!filesAndTags.isEmpty() && type != 0){

        switch(numberOfType)
        {
            case 1: ///###: tag files!!!!
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::TagFiles, bool>(filesAndTags) };
                var.setValue(value);

                break;
            }
            case 2:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                QStringList files{ this->execSqlstr<DSqliteHandle::SqlType::GetFilesThroughTag, QList<QString>>(filesAndTags) };
                var.setValue(files);

                break;
            }
            case 3:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                QStringList tags{ this->execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(filesAndTags) };
                var.setValue(tags);

                break;
            }
            case 4: ///###: untag files(support different partion).
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::UntagDiffPartionFiles, bool>(filesAndTags) };
                var.setValue(value);

                break;
            }
            case 5:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::DeleteTags, bool>(filesAndTags) };//###: do not be confused by the name of variant.
                var.setValue(value);

                break;
            }
            case 6:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::ChangeTagsName, bool>(filesAndTags) };
                var.setValue(value);

                break;
            }
            case 7:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::DeleteFiles, bool>(filesAndTags) };
                var.setValue(value);

                break;
            }
            case 8:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::TagFilesThroughColor, bool>(filesAndTags) };
                var.setValue(value);

                break;
            }
            case 9:
            {
                std::lock_guard<std::mutex> raillLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::ChangeFilesName, bool>(filesAndTags) };
                var.setValue(value);

                break;
            }
            case 10:
            {
                std::lock_guard<std::mutex> raillLock{ m_mutex };
                QMap<QString, QVariant> tag_and_color{ this->execSqlstr<DSqliteHandle::SqlType::GetAllTags, QMap<QString, QVariant>>(filesAndTags) };
                var.setValue(tag_and_color);

                break;
            }
            case 11:
            {
                std::lock_guard<std::mutex> raii_lock{ m_mutex };
                bool result{ this->execSqlstr<DSqliteHandle::SqlType::BeforeTagFiles, bool>(filesAndTags) };
                var.setValue(result);

                break;
            }
            case 12:
            {
                std::lock_guard<std::mutex> raii_lock{ m_mutex };
                QMap<QString, QVariant> tag_and_color{ this->execSqlstr<DSqliteHandle::SqlType::GetTagColor, QMap<QString, QVariant>>(filesAndTags) };
                var.setValue(tag_and_color);

                break;
            }
            case 13:
            {
                std::lock_guard<std::mutex> raii_lock{ m_mutex };
                bool result{ this->execSqlstr<DSqliteHandle::SqlType::ChangeTagColor, bool>(filesAndTags) };
                var.setValue(result);

                break;
            }
            default:
                break;
        }
    }

    return var;
}


DSqliteHandle::ReturnCode DSqliteHandle::checkWhetherHasSqliteInPartion(const QString& mountPoint, const QString& db_name)
{
    QDir dir{ mountPoint };

    if (!dir.exists())
        return ReturnCode::NoThisDir;

    if (dir.exists(db_name))
        return ReturnCode::Exist;

    return ReturnCode::NoExist;
}

void DSqliteHandle::initializeConnect()
{
    QObject::connect(deviceListener, &UDiskListener::mountAdded, this, &DSqliteHandle::onMountAdded);
    QObject::connect(deviceListener, &UDiskListener::mountRemoved, this, &DSqliteHandle::onMountRemoved);
}

void DSqliteHandle::connectToSqlite(const QString& mountPoint, const QString& db_name)
{
    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, db_name) };
    std::function<void()> initDatabasePtr{ [&]{

            if(m_sqlDatabasePtr->isOpen()){
                m_sqlDatabasePtr->close();
            }

            if(QSqlDatabase::contains(CONNECTIONNAME)){
                QSqlDatabase::removeDatabase(CONNECTIONNAME);
            }

            m_sqlDatabasePtr = std::unique_ptr<QSqlDatabase>{new QSqlDatabase{ QSqlDatabase::addDatabase(R"foo(QSQLITE)foo", CONNECTIONNAME)} };
            QString DBName{mountPoint + QString{"/"} + db_name};
            qDebug() << DBName;

            m_sqlDatabasePtr->setDatabaseName(DBName);
            m_sqlDatabasePtr->setUserName(USERNAME);
            m_sqlDatabasePtr->setPassword(PASSWORD);
                                           } };

    if(code == DSqliteHandle::ReturnCode::NoExist){
        initDatabasePtr();

        if(m_sqlDatabasePtr->open()){

            if(m_sqlDatabasePtr->transaction()){
                QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

                if(db_name != QString{".__main.db"}){
                    QString createFileProperty{
                        "CREATE TABLE IF NOT EXISTS \"file_property\" "
                        "  ("
                        " `file_name` TEXT NOT NULL UNIQUE,"
                        " `tag_1` TEXT NOT NULL,"
                        " `tag_2` TEXT,"
                        " `tag_3` TEXT"
                        " )"
                    };

                    QString createTagWithFile{
                        "CREATE TABLE IF NOT EXISTS \"tag_with_file\" "
                        " ("
                        " `tag_name` TEXT NOT NULL,"
                        " `file_name` TEXT NOT NULL "
                        " )"
                    };

                    if(!m_flag.load(std::memory_order_consume)){

                        if(!sqlQuery.exec(createFileProperty)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        }

                        if(!sqlQuery.exec(createTagWithFile)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        }

                    }else{
                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                        if(code != DSqliteHandle::ReturnCode::NoThisDir){

                            if(!sqlQuery.exec(createFileProperty)){
                                qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            }

                            if(!sqlQuery.exec(createTagWithFile)){
                                qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            }
                        }
                    }

                }else{

                    QString createTagProperty{
                        "CREATE TABLE IF NOT EXISTS \"tag_property\" "
                        " ("
                        "`tag_index` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                        "`tag_name`  TEXT NOT NULL UNIQUE,"
                        "`tag_color` TEXT NOT NULL"
                        ")"
                    };

                    if(!m_flag.load(std::memory_order_consume)){

                        if(!sqlQuery.exec(createTagProperty)){
                            qWarning() << sqlQuery.lastError().text();
                        }

                    }else{
                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                        if(code != DSqliteHandle::ReturnCode::NoThisDir){

                            if(!sqlQuery.exec(createTagProperty)){
                                qWarning() << sqlQuery.lastError().text();
                            }
                        }
                    }
                }

                if(!m_sqlDatabasePtr->commit()){
                    m_sqlDatabasePtr->rollback();
                }

            }else{
                qWarning("failed to open transaction!!!!!!");
                m_sqlDatabasePtr->close();
            }

        }else{
            qWarning("errors occured when creating a DB in a partion.");
        }

    }else if(code == DSqliteHandle::ReturnCode::Exist){
        initDatabasePtr();
    }

    this->closeSqlDatabase();
}


///###:this is also a auxiliary function. do not need a mutex.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles, QMap<QString,
                               QList<QString>>, bool>(const QMap<QString, QList<QString>>& forDecreasing, const QString& mountPoint)
{
    if(!forDecreasing.isEmpty() && !mountPoint.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ forDecreasing.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ forDecreasing.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> itrOfSqlForDeleting{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFiles) };
        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itr{ itrOfSqlForDeleting.first };
        ++itr; ++itr;
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };


        for(; cbeg != cend; ++cbeg){

            for(const QString& tagName : cbeg.value()){
                QString sql{ itr->second.arg(tagName) };
                sql = sql.arg(cbeg.key());

                if(!m_flag.load(std::memory_order_acquire)){

                    ///###: delete redundant item in tag_with_file.
                    if(!sqlQuery.exec(sql)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        continue;
                    }

                }else{

                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                    if(code == DSqliteHandle::ReturnCode::Exist){

                        if(!sqlQuery.exec(sql)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            continue;
                        }

                    }else{

                        return false;
                    }
                }
            }
        }

        return true;
    }

    return false;
}

///###:this is also a auxiliary function. do not need a mutex.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles2, QMap<QString, QList<QString>>, bool>(
                                                      const QMap<QString, QList<QString>>& forIncreasing, const QString& mountPoint)
{
    if(!forIncreasing.isEmpty() && !mountPoint.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ forIncreasing.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ forIncreasing.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> itrOfSqlForDeleting{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFiles) };
        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itr{ itrOfSqlForDeleting.first };
        ++itr;
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for(; cbeg != cend; ++cbeg){

            for(const QString& tagName : cbeg.value()){
                QString sql{ itr->second.arg(cbeg.key()) };
                sql = sql.arg(tagName);

                if(!m_flag.load(std::memory_order_acquire)){

                    ///###: tag files
                    if(!sqlQuery.exec(sql)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        continue;
                    }

                }else{

                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                    if(code == DSqliteHandle::ReturnCode::Exist){

                        if(!sqlQuery.exec(sql)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            continue;
                        }

                    }else{

                        return false;
                    }
                }
            }
        }

        return true;
    }

    return false;
}



template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles3, QList<QString>, bool>(const QList<QString>& forUpdating,
                                                                                                        const QString& mountPoint)
{
    if(!forUpdating.isEmpty() && !mountPoint.isEmpty()){
        QList<QString>::const_iterator cbeg{ forUpdating.cbegin() };
        QList<QString>::const_iterator cend{ forUpdating.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> itrOfSqlForDeleting{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFiles) };
        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itr{ itrOfSqlForDeleting.first };
        ++itr; ++itr; ++itr; ++itr;
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for(; cbeg != cend; ++cbeg){
            QString sqlForGettingTag{ itr->second.arg(*cbeg) };
            std::vector<QString> leftTags{};

            if(!m_flag.load(std::memory_order_acquire)){

                if(sqlQuery.exec(sqlForGettingTag)){

                    while(sqlQuery.next()){
                        QString tagName{ sqlQuery.value("tag_name").toString() };
                        leftTags.push_back(tagName);
                    }
                }

            }else{

                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(sqlQuery.exec(sqlForGettingTag)){

                        while(sqlQuery.next()){
                            QString tagName{ sqlQuery.value("tag_name").toString() };
                            leftTags.push_back(tagName);
                        }
                    }

                }else{
                    return false;
                }
            }

            if(leftTags.empty()){
                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForDelRowInFileProperty{itr};
                --itrForDelRowInFileProperty;
                QString sqlForDelRowInFileProperty{ itrForDelRowInFileProperty->second.arg(*cbeg) };


                if(!m_flag.load(std::memory_order_acquire)){

                    if(!sqlQuery.exec(sqlForDelRowInFileProperty)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        continue;
                    }

                }else{

                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                    if(code == DSqliteHandle::ReturnCode::Exist){

                        if(!sqlQuery.exec(sqlForDelRowInFileProperty)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            continue;
                        }

                    }else{
                        return false;
                    }
                }

            }else{

                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForCounterFileInFP{ itr };
                ++itrForCounterFileInFP; ++itrForCounterFileInFP;
                QString sqlOfCountingFileInFP{ itrForCounterFileInFP->second.arg(*cbeg) };

                int counter{ 0 };

                if(!m_flag.load(std::memory_order_acquire)){

                    if(sqlQuery.exec(sqlOfCountingFileInFP)){

                        if(sqlQuery.next()){
                            counter =  sqlQuery.value("counter").toInt();
                        }
                    }

                }else{
                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                    if(code == DSqliteHandle::ReturnCode::Exist){

                        if(sqlQuery.exec(sqlOfCountingFileInFP)){
                            counter = sqlQuery.value("counter").toInt();
                        }

                    }else{
                        return false;
                    }
                }

                std::size_t size{ leftTags.size() };

                if(size < 3){
                    std::size_t differenceValue{ 3u - size };

                    for(std::size_t index = 0; index < differenceValue; ++index){
                        leftTags.push_back(QString{});
                    }
                }


                if(counter > 0){
                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForUpdating{itr};
                    ++itrForUpdating;

                    std::size_t sizeOfTags{ leftTags.size() };

                    QString sqlForUpdatingFileProperty{itrForUpdating->second.arg(leftTags[sizeOfTags-3])};
                    sqlForUpdatingFileProperty = sqlForUpdatingFileProperty.arg(leftTags[sizeOfTags-2]);
                    sqlForUpdatingFileProperty = sqlForUpdatingFileProperty.arg(leftTags[sizeOfTags-1]);
                    sqlForUpdatingFileProperty = sqlForUpdatingFileProperty.arg(*cbeg);

                    if(!m_flag.load(std::memory_order_acquire)){

                        if(!sqlQuery.exec(sqlForUpdatingFileProperty)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            continue;
                        }

                    }else{

                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                        if(code == DSqliteHandle::ReturnCode::Exist){

                            if(!sqlQuery.exec(sqlForUpdatingFileProperty)){
                                qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                continue;
                            }

                        }else{
                            return false;
                        }

                    }

                }else{

                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForInsertRowInFP{ itr };
                    ++itrForInsertRowInFP;
                    ++itrForInsertRowInFP;
                    ++itrForInsertRowInFP;
                    std::size_t sizeOfTags{ leftTags.size() };
                    QString sqlForInsertRowInFP{ itrForInsertRowInFP->second.arg(*cbeg) };
                    sqlForInsertRowInFP = sqlForInsertRowInFP.arg(leftTags[sizeOfTags-3]);
                    sqlForInsertRowInFP = sqlForInsertRowInFP.arg(leftTags[sizeOfTags-2]);
                    sqlForInsertRowInFP = sqlForInsertRowInFP.arg(leftTags[sizeOfTags-1]);


                    if(!m_flag.load(std::memory_order_acquire)){

                        if(!sqlQuery.exec(sqlForInsertRowInFP)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            continue;
                        }

                    }else{

                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                        if(code == DSqliteHandle::ReturnCode::Exist){

                            if(!sqlQuery.exec(sqlForInsertRowInFP)){
                                qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                continue;
                            }

                        }else{
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    return false;
}


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor,
                    std::list<std::tuple<QString, QString, QString, QString, QString, QString>>,
                    bool>(const std::list<std::tuple<QString, QString, QString, QString, QString, QString>>& sqlStrs,
                                                                                     const QString& mountPoint)
{
    if(!m_flag.load(std::memory_order_consume) && !sqlStrs.empty() && !mountPoint.isEmpty()){
        std::list<std::tuple<QString, QString, QString, QString, QString, QString>>::const_iterator cbeg{ sqlStrs.cbegin() };
        std::list<std::tuple<QString, QString, QString, QString, QString, QString>>::const_iterator cend{ sqlStrs.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFilesThroughColor2) };


        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };
        bool result{ true };

        for(; cbeg != cend; ++cbeg){

            if(!m_flag.load(std::memory_order_consume)){

                if(!sqlQuery.exec(std::get<0>(*cbeg))){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                int counter{ 0 };

                if(sqlQuery.next()){
                    counter = sqlQuery.value("counter").toInt();
                }


                if(counter == 0){
                    bool flag{ true };

                    if(!sqlQuery.exec(std::get<1>(*cbeg))){
                        flag = false;
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                    if(flag){

                        if(!sqlQuery.exec(std::get<2>(*cbeg))){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        }

                        std::list<QString> tagNames{};

                        while(sqlQuery.next()){
                            QString tagName{ sqlQuery.value("tag_name").toString() };
                            tagNames.emplace_back(std::move(tagName));
                        }

                        if(!tagNames.empty()){

                            std::size_t size{ tagNames.size() };

                            if(size < 3){
                                std::size_t redundant{ 3 - size };

                                for(std::size_t index = 0; index < redundant; ++index){
                                    tagNames.emplace_back(QString{""});
                                }
                            }

                            QString sqlForCounting{ range.first->second };
                            sqlForCounting = sqlForCounting.arg(std::get<4>(*cbeg));
                            std::list<QString>::const_iterator tagNameItr{ tagNames.cbegin() };

                            if(!sqlQuery.exec(sqlForCounting)){
                                qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                result = false;
                                break;
                            }

                            if(sqlQuery.next()){
                                int counter{ sqlQuery.value("counter").toInt() };

                                if(counter == 0){
                                    QString sqlForInsertingNewRow{ std::get<3>(*cbeg) };
                                    sqlForInsertingNewRow = sqlForInsertingNewRow.arg(std::get<4>(*cbeg));
                                    sqlForInsertingNewRow = sqlForInsertingNewRow.arg(*tagNameItr);
                                    sqlForInsertingNewRow = sqlForInsertingNewRow.arg(*(++tagNameItr));
                                    sqlForInsertingNewRow = sqlForInsertingNewRow.arg(*(++tagNameItr));

                                    if(!sqlQuery.exec(sqlForInsertingNewRow)){
                                         qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                         result = false;
                                         break;
                                    }
                                    continue;

                                }else{
                                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItrForUpdating{ range.first };
                                    ++sqlItrForUpdating;
                                    QString sqlForUpdating{ sqlItrForUpdating->second };
                                    sqlForUpdating = sqlForUpdating.arg(*tagNameItr);
                                    sqlForUpdating = sqlForUpdating.arg(*(++tagNameItr));
                                    sqlForUpdating = sqlForUpdating.arg(*(++tagNameItr));
                                    sqlForUpdating = sqlForUpdating.arg(std::get<4>(*cbeg));

                                    if(!sqlQuery.exec(sqlForUpdating)){
                                         qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                         result = false;
                                         break;
                                    }
                                    continue;
                                }
                            }
                        }
                    }
                    result = false;
                    break;
                }

            }else{

                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(!sqlQuery.exec(std::get<0>(*cbeg))){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                    int counter{ 0 };

                    if(sqlQuery.next()){
                        counter = sqlQuery.value("counter").toInt();
                    }


                    if(counter == 0){
                        bool flag{ true };

                        if(!sqlQuery.exec(std::get<1>(*cbeg))){
                            flag = false;
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        }

                        if(flag){

                            if(!sqlQuery.exec(std::get<2>(*cbeg))){
                                qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            }

                            std::list<QString> tagNames{};

                            while(sqlQuery.next()){
                                QString tagName{ sqlQuery.value("tag_name").toString() };
                                tagNames.emplace_back(std::move(tagName));
                            }

                            if(!tagNames.empty()){
                                QString sqlForInsertingNewRow{ std::get<3>(*cbeg) };
                                std::size_t size{ tagNames.size() };

                                if(size < 3){
                                    std::size_t redundant{ 3 - size };

                                    for(std::size_t index = 0; index < redundant; ++index){
                                        tagNames.emplace_back(QString{""});
                                    }
                                }

                                if(sqlQuery.next()){
                                    int counter{ sqlQuery.value("counter").toInt() };
                                    std::list<QString>::const_iterator tagNameItr{ tagNames.cbegin() };

                                    if(counter == 0){
                                        QString sqlForInsertingNewRow{ std::get<3>(*cbeg) };
                                        sqlForInsertingNewRow = sqlForInsertingNewRow.arg(*tagNameItr);
                                        sqlForInsertingNewRow = sqlForInsertingNewRow.arg(*(++tagNameItr));
                                        sqlForInsertingNewRow = sqlForInsertingNewRow.arg(*(++tagNameItr));
                                        sqlForInsertingNewRow = sqlForInsertingNewRow.arg(std::get<4>(*cbeg));

                                        if(!sqlQuery.exec(sqlForInsertingNewRow)){
                                             qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                             result = false;
                                             break;
                                        }
                                        continue;

                                    }else{
                                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItrForUpdating{ range.first };
                                        ++sqlItrForUpdating;
                                        QString sqlForUpdating{ sqlItrForUpdating->second };
                                        sqlForUpdating = sqlForUpdating.arg(*tagNameItr);
                                        sqlForUpdating = sqlForUpdating.arg(*(++tagNameItr));
                                        sqlForUpdating = sqlForUpdating.arg(*(++tagNameItr));
                                        sqlForUpdating = sqlForUpdating.arg(std::get<4>(*cbeg));

                                        if(!sqlQuery.exec(sqlForInsertingNewRow)){
                                             qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                             result = false;
                                             break;
                                        }
                                        continue;
                                    }
                                }
                            }
                        }
                        result = false;
                        break;
                    }
                    continue;
                }
                result = false;
                break;
            }
        }
        return result;
    }
    return false;
}


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor3, QString, bool>(const QString& tag_name, const QString& mountPoint)
{
    if(!tag_name.isEmpty() && mountPoint == QString{"/home"}){
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFilesThroughColor3) };
        QSqlQuery sql_query{ *m_sqlDatabasePtr };
        QString sql_counting{ range.first->second.arg(tag_name) };
        QString sql_inserting{ (++range.first)->second.arg(tag_name) };

        if(sql_query.exec(sql_counting)){

            if(sql_query.next()){
                int number{ sql_query.value("counter").toInt() };

                if(number == 0){
                    sql_query.clear();

                    if(!sql_query.exec(sql_inserting)){
                        qWarning()<< sql_query.lastError().text();

                        return false;
                    }

                    emit addNewTags(QVariant{QList<QString>{tag_name}});
                }

                return true;
            }
        }
    }

    return false;
}



template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles,
                                                        std::list<QString>>( const std::list<QString>& sqlStrs, const QString& mountPoint)
{
    if(!m_flag.load(std::memory_order_consume) && !sqlStrs.empty() && !mountPoint.isEmpty()){
        std::list<QString>::const_iterator cbeg{ sqlStrs.cbegin() };
        std::list<QString>::const_iterator cend{ sqlStrs.cend() };
        QSqlQuery sqlQuery{*m_sqlDatabasePtr};

        for(; cbeg != cend; ++cbeg){

            if(!m_flag.load(std::memory_order_consume)){

                if(!sqlQuery.exec(*cbeg)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

            }else{

                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(!sqlQuery.exec(*cbeg)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                }else{
                    return false;
                }
            }
        }

        return true;
    }

    return false;
}


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles2, QMap<QString, QList<QString>>>(const QMap<QString, QList<QString>>& fileNameAndTagNames,
                                                                                                              const QString& mountPoint)
{
    if(!fileNameAndTagNames.isEmpty() && !mountPoint.isEmpty() && static_cast<bool>(m_sqlDatabasePtr)){
        QMap<QString, QList<QString>>::const_iterator cbeg{ fileNameAndTagNames.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ fileNameAndTagNames.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::UntagSamePartionFiles2) };
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for(; cbeg != cend; ++cbeg){
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
            QString countTagNamesInTagWithFile{ rangeBeg->second.arg(cbeg.key()) };


            if(!m_flag.load(std::memory_order_consume)){

                if(!sqlQuery.exec(countTagNamesInTagWithFile)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    continue;
                }

                if(sqlQuery.next()){
                    int size{ sqlQuery.value(0).toInt() };

                    if(size == 0){
                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator  rangeEnd{ range.second };
                        QString deleteRowInFileProperty{ (--rangeEnd)->second.arg(cbeg.key()) };

                        if(!m_flag.load(std::memory_order_consume)){

                            if(!sqlQuery.exec(deleteRowInFileProperty)){
                                qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                continue;
                            }

                        }else{

                            DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                            if(code == DSqliteHandle::ReturnCode::Exist){

                                if(!sqlQuery.exec(deleteRowInFileProperty)){
                                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                    continue;
                                }

                            }else{
                                return false;
                            }
                        }


                    }else{

                        QString sqlForGetingLeftTag{ (++rangeBeg)->second.arg(cbeg.key()) };

                        if(!m_flag.load(std::memory_order_consume)){

                            if(!sqlQuery.exec(sqlForGetingLeftTag)){
                                qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                continue;
                            }

                            std::deque<QString> leftTags{};

                            while(sqlQuery.next()){
                                leftTags.push_back(sqlQuery.value("tag_name").toString());
                            }

                            std::size_t sizeOfLeftTags{ leftTags.size() };
                            std::deque<QString>::const_iterator leftTagsCbeg{ leftTags.cbegin() };

                            if(sizeOfLeftTags >= 3u){
                                leftTags.erase(leftTagsCbeg, leftTagsCbeg + (sizeOfLeftTags - 3u));

                            }else{

                                for(; size < 3; ++size){
                                    leftTags.push_back(QString{});
                                }
                            }

                            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForUpdateRow{ (--range.second) };
                            leftTagsCbeg = leftTags.cbegin();
                            QString updateRowInFileProperty{ (--itrForUpdateRow)->second.arg(*leftTagsCbeg) };
                            updateRowInFileProperty = updateRowInFileProperty.arg(*(++leftTagsCbeg));
                            updateRowInFileProperty = updateRowInFileProperty.arg(*(++leftTagsCbeg));
                            updateRowInFileProperty = updateRowInFileProperty.arg(cbeg.key());

                            if(!m_flag.load(std::memory_order_consume)){

                                if(!sqlQuery.exec(updateRowInFileProperty)){
                                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                    continue;
                                }

                            }else{

                                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                                if(code == DSqliteHandle::ReturnCode::Exist){

                                    if(!sqlQuery.exec(updateRowInFileProperty)){
                                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                        continue;
                                    }

                                }else{
                                    return false;
                                }

                            }
                        }
                    }
                }

            }else{

                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(sqlQuery.exec(countTagNamesInTagWithFile)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        continue;
                    }

                    if(sqlQuery.next()){

                        int size{ sqlQuery.value(0).toInt() };

                        if(size == 0){
                            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator  rangeEnd{ range.second };
                            QString deleteRowInFileProperty{ (--rangeEnd)->second.arg(cbeg.key()) };

                            if(!m_flag.load(std::memory_order_consume)){

                                if(!sqlQuery.exec(deleteRowInFileProperty)){
                                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                    continue;
                                }

                            }else{

                                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                                if(code == DSqliteHandle::ReturnCode::Exist){

                                    if(!sqlQuery.exec(deleteRowInFileProperty)){
                                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                        continue;
                                    }

                                }else{
                                    return false;
                                }
                            }


                        }else{

                            QString sqlForGetingLeftTag{ (++rangeBeg)->second };

                            if(!m_flag.load(std::memory_order_consume)){

                                if(!sqlQuery.exec(sqlForGetingLeftTag)){
                                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                    continue;
                                }

                                std::deque<QString> leftTags{};
                                int sizeOfTags{ sqlQuery.size() };

                                for(long int index = 0; index < sizeOfTags; ++index){
                                    leftTags.push_back(sqlQuery.value(index).toString());
                                }

                                std::size_t sizeOfLeftTags{ leftTags.size() };
                                std::deque<QString>::const_iterator leftTagsCbeg{ leftTags.cbegin() };

                                if(sizeOfLeftTags >= 3u){
                                    leftTags.erase(leftTagsCbeg, leftTagsCbeg + (sizeOfLeftTags - 3u));

                                }else{

                                    for(; size < 3; ++size){
                                        leftTags.push_back(QString{});
                                    }
                                }

                                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForUpdateRow{ (--range.second) };
                                leftTagsCbeg = leftTags.cbegin();
                                QString updateRowInFileProperty{ (--itrForUpdateRow)->second.arg(*leftTagsCbeg) };
                                updateRowInFileProperty = updateRowInFileProperty.arg(*(++leftTagsCbeg));
                                updateRowInFileProperty = updateRowInFileProperty.arg(*(++leftTagsCbeg));
                                updateRowInFileProperty = updateRowInFileProperty.arg(cbeg.key());

                                if(!m_flag.load(std::memory_order_consume)){

                                    if(!sqlQuery.exec(updateRowInFileProperty)){
                                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                        continue;
                                    }

                                }else{

                                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                                    if(code == DSqliteHandle::ReturnCode::Exist){

                                        if(!sqlQuery.exec(updateRowInFileProperty)){
                                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                            continue;
                                        }

                                    }else{
                                        return false;
                                    }

                                }

                            }
                        }
                    }
                }
            }
        }

        return true;
    }

    return false;
}

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteFiles,
                                std::map<QString, std::pair<QString, QString>>>(const std::map<QString, std::pair<QString, QString>>& sqlStrs,
                                                                                       const QString& mountPoint)
{
    if(!sqlStrs.empty() && !mountPoint.isEmpty()){
        std::map<QString, std::pair<QString, QString>>::const_iterator cbeg{ sqlStrs.cbegin() };
        std::map<QString, std::pair<QString, QString>>::const_iterator cend{ sqlStrs.cend() };
        QSqlQuery sqlQuery{*m_sqlDatabasePtr};


        for(; cbeg != cend; ++cbeg){

            if(!m_flag.load(std::memory_order_acquire)){

                if(!sqlQuery.exec(cbeg->second.first)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

            }else{
                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(!sqlQuery.exec(cbeg->second.first)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                }else{
                    return false;
                }

            }
        }


        cbeg = sqlStrs.cbegin();
        cend = sqlStrs.cend();

        for(; cbeg != cend; ++cbeg){

            if(!m_flag.load(std::memory_order_acquire)){

                if(!sqlQuery.exec(cbeg->second.second)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

            }else{
                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint) };

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(!sqlQuery.exec(cbeg->second.second)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                }else{
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags,
                                std::list<QString>>(const std::list<QString>& sqlStrs,
                                                                                  const QString& mountPoint)
{
    if(!sqlStrs.empty() && !mountPoint.isEmpty()){
        std::list<QString>::const_iterator sqlCBeg{ sqlStrs.cbegin() };
        std::list<QString>::const_iterator sqlCEnd{ sqlStrs.cend() };
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for(; sqlCBeg != sqlCEnd; ++sqlCBeg){

            if(!m_flag.load(std::memory_order_consume)){

                if(!sqlQuery.exec(*sqlCBeg)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

            }else{
                DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint)};

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(!sqlQuery.exec(*sqlCBeg)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                }else{

                    return false;
                }
            }
        }



        return true;
    }

    return false;
}

template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags2,
                                QMap<QString, QList<QString>>>(const QMap<QString, QList<QString>>& fileNameAndTagNames, const QString& mountPoint)
{

    if(!fileNameAndTagNames.isEmpty() && !mountPoint.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ fileNameAndTagNames.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ fileNameAndTagNames.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteTags2) };
        std::list<QString> gettingFileNamesThroughTagName{};

        for(; cbeg != cend; ++cbeg){
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
            QString sqlForGettingFileNames{ rangeBeg->second.arg(cbeg.key()) };
            gettingFileNamesThroughTagName.emplace_back(std::move(sqlForGettingFileNames));
        }

        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };
        std::map<QString, std::deque<QString>> filesNameAndTagsName{};

        if(!gettingFileNamesThroughTagName.empty()){
            std::list<QString>::const_iterator sqlCBeg{ gettingFileNamesThroughTagName.cbegin() };
            std::list<QString>::const_iterator sqlCEnd{ gettingFileNamesThroughTagName.cend() };

            for(; sqlCBeg != sqlCEnd; ++sqlCBeg){

                if(!m_flag.load(std::memory_order_consume)){

                    if(!sqlQuery.exec(*sqlCBeg)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                    while(sqlQuery.next()){
                        QString fileName{ sqlQuery.value("file_name").toString() };
                        filesNameAndTagsName[fileName] = std::deque<QString>{};
                    }


                }else{

                    DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint)};

                    if(code == DSqliteHandle::ReturnCode::Exist){

                        if(!sqlQuery.exec(*sqlCBeg)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        }

                        while(sqlQuery.next()){
                            QString fileName{ sqlQuery.value("file_name").toString() };
                            filesNameAndTagsName[fileName] = std::deque<QString>{};
                        }

                    }else{
                        return false;
                    }
                }
            }
        }

        if(!filesNameAndTagsName.empty()){
            std::map<QString, std::deque<QString>> transcript{};
            std::map<QString, std::deque<QString>>::const_iterator filesAndTagsItrBeg{ filesNameAndTagsName.cbegin() };
            std::map<QString, std::deque<QString>>::const_iterator filesAndTagsItrEnd{ filesNameAndTagsName.cend() };

            for(; filesAndTagsItrBeg != filesAndTagsItrEnd; ++filesAndTagsItrBeg){

                if(!m_flag.load(std::memory_order_consume)){
                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
                    ++rangeBeg;
                    QString sqlForGettingTagsOfFile{ rangeBeg->second.arg(filesAndTagsItrBeg->first) };

                    if(!sqlQuery.exec(sqlForGettingTagsOfFile)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                    while(sqlQuery.next()){
                        QString tagName{ sqlQuery.value("tag_name").toString() };
                        transcript[filesAndTagsItrBeg->first].emplace_back(std::move(tagName));
                    }

                }else{
                    DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint)};

                    if(code == DSqliteHandle::ReturnCode::Exist){
                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
                        ++rangeBeg;
                        QString sqlForGettingTagsOfFile{ rangeBeg->second.arg(filesAndTagsItrBeg->first) };

                        if(!sqlQuery.exec(sqlForGettingTagsOfFile)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            continue;
                        }

                        while(sqlQuery.next()){
                            QString tagName{ sqlQuery.value("tag_name").toString() };
                            transcript[filesAndTagsItrBeg->first].emplace_back(std::move(tagName));
                        }
                    }
                }
            }

            if(!transcript.empty()){
                filesNameAndTagsName = std::move(transcript);
            }
        }


        if(!filesNameAndTagsName.empty()){
            std::map<QString, std::deque<QString>>::const_iterator itrForUpdatingOrDeleting{ filesNameAndTagsName.cbegin() };
            std::map<QString, std::deque<QString>>::const_iterator itrForUpdatingOrDeletingEnd{ filesNameAndTagsName.cend() };
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeLast{ --(range.second) };

            for(; itrForUpdatingOrDeleting != itrForUpdatingOrDeletingEnd; ++itrForUpdatingOrDeleting){
                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItr{ rangeLast };
                QString sqlForUpdatingOrDeleting{};
                std::size_t tagAmount{ itrForUpdatingOrDeleting->second.size() };

                if(tagAmount >= 3){
                    sqlForUpdatingOrDeleting = sqlItr->second.arg(itrForUpdatingOrDeleting->second[tagAmount-3]);
                    sqlForUpdatingOrDeleting = sqlItr->second.arg(itrForUpdatingOrDeleting->second[tagAmount-2]);
                    sqlForUpdatingOrDeleting = sqlItr->second.arg(itrForUpdatingOrDeleting->second[tagAmount-1]);
                    sqlForUpdatingOrDeleting = sqlItr->second.arg(itrForUpdatingOrDeleting->first);

                }else if(tagAmount == 0){
                    QString sqlForDeletingRowInFileProperty{ "DELETE FROM file_property WHERE file_property.file_name = \'%1\'" };
                    sqlForUpdatingOrDeleting = sqlForDeletingRowInFileProperty.arg(itrForUpdatingOrDeleting->first);

                }else{
                    std::size_t difference{ 3u - tagAmount };
                    std::deque<QString>::const_iterator tagNameBeg{ itrForUpdatingOrDeleting->second.cbegin() };
                    std::deque<QString>::const_iterator tagNameEnd{ itrForUpdatingOrDeleting->second.cend() };

                    sqlForUpdatingOrDeleting = sqlItr->second.arg(*tagNameBeg);
                    ++tagNameBeg;

                    for(; tagNameBeg != tagNameEnd; ++tagNameBeg){
                        sqlForUpdatingOrDeleting = sqlForUpdatingOrDeleting.arg(*tagNameBeg);
                    }

                    for(std::size_t index = 0; index < difference; ++index){
                        sqlForUpdatingOrDeleting = sqlForUpdatingOrDeleting.arg(QString{});
                    }
                    sqlForUpdatingOrDeleting = sqlForUpdatingOrDeleting.arg(itrForUpdatingOrDeleting->first);
                }


                if(!m_flag.load(std::memory_order_consume) && !sqlForUpdatingOrDeleting.isEmpty()){

                    if(!sqlQuery.exec(sqlForUpdatingOrDeleting)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                }else if(m_flag.load(std::memory_order_consume) && !sqlForUpdatingOrDeleting.isEmpty()){
                    DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint)};

                    if(code == DSqliteHandle::ReturnCode::Exist){

                        if(!sqlQuery.exec(sqlForUpdatingOrDeleting)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        }

                    }else{
                        return false;
                    }

                }
            }
        }
        return true;
    }
    return false;
}


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags3, QList<QString>, bool>(const QList<QString>& tag_name, const QString& mountPoint)
{
    if(!tag_name.isEmpty() && mountPoint == QString{"/home"} && QFileInfo::exists("/home")){
        QList<QString>::const_iterator c_beg{ tag_name.cbegin() };
        QList<QString>::const_iterator c_end{ tag_name.cend() };
        QSqlQuery sql_query{ *m_sqlDatabasePtr };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteTags3) };

        for(; c_beg != c_end; ++c_beg){
            QString sql_str{ range.first->second };
            sql_str = sql_str.arg(*c_beg);

            if (!sql_query.exec(sql_str)) {
                qWarning()<< sql_query.lastError().text();
                return false;
            }
        }

        return true;
    }

    return false;
}



template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeFilesName, std::map<QString, QString>>(const std::map<QString, QString>& sqlStrs, const QString& mountPoint)
{
    if(!sqlStrs.empty() && !mountPoint.isEmpty()){
        std::map<QString, QString>::const_iterator sqlCBeg{ sqlStrs.cbegin() };
        std::map<QString, QString>::const_iterator sqlCEnd{ sqlStrs.cend() };
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for(; sqlCBeg != sqlCEnd; ++sqlCBeg){

            if(!m_flag.load(std::memory_order_consume)){

                if(!sqlQuery.exec(sqlCBeg->first)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                if(!sqlQuery.exec(sqlCBeg->second)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

            }else{
                DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint)};

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(!sqlQuery.exec(sqlCBeg->first)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                    if(!sqlQuery.exec(sqlCBeg->second)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                }else{
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeTagsName,
                                std::list<std::tuple<QString, QString, QString, QString>>>(const std::list<std::tuple<QString, QString, QString, QString>>& sqlStrs,
                                                                                           const QString& mountPoint)
{
    if(!sqlStrs.empty() && !mountPoint.isEmpty()){
        std::list<std::tuple<QString, QString, QString, QString>>::const_iterator sqlCBeg{ sqlStrs.cbegin() };
        std::list<std::tuple<QString, QString, QString, QString>>::const_iterator sqlCEnd{ sqlStrs.cend() };
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };


        for(; sqlCBeg != sqlCEnd; ++sqlCBeg){

            if(!m_flag.load(std::memory_order_consume)){

                QSqlQuery sqlQuery(*m_sqlDatabasePtr);

                if(!sqlQuery.exec(std::get<0>(*sqlCBeg))){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                if(!sqlQuery.exec(std::get<1>(*sqlCBeg))){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                if(!sqlQuery.exec(std::get<2>(*sqlCBeg))){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                if(!sqlQuery.exec(std::get<3>(*sqlCBeg))){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

            }else{
                DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint)};

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(!sqlQuery.exec(std::get<0>(*sqlCBeg))){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                    if(!sqlQuery.exec(std::get<1>(*sqlCBeg))){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                    if(!sqlQuery.exec(std::get<2>(*sqlCBeg))){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                    if(!sqlQuery.exec(std::get<3>(*sqlCBeg))){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                }else{
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeTagsName2, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>>& old_and_new,
                                                                                                                     const QString& mountPoint)
{
    if(!old_and_new.empty() && mountPoint == QString{"/home"} && QFileInfo::exists("/home")){
        QMap<QString, QList<QString>>::const_iterator c_beg{ old_and_new.cbegin() };
        QMap<QString, QList<QString>>::const_iterator c_end{ old_and_new.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(SqlType::ChangeTagsName2) };
        QSqlQuery sql_query{ *m_sqlDatabasePtr };

        for(; c_beg != c_end; ++c_beg){
            QString sql_str{ range.first->second.arg(c_beg.value().first()) };
            sql_str = sql_str.arg(c_beg.key());

            if(!sql_query.exec(sql_str)){
                qWarning()<< sql_query.lastError().text();
                return false;
            }
        }

        return true;
    }

    return false;
}





template<>
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetTagsThroughFile, QString,
                                          QList<QString>>(const QString& sqlStr, const QString& mountPoint)
{
    QList<QString> tagNames{};

    if(!sqlStr.isEmpty() && !mountPoint.isEmpty()){
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        if(!m_flag.load(std::memory_order_consume)){

            if(!sqlQuery.exec(sqlStr)){
                qWarning(sqlQuery.lastError().text().toStdString().c_str());
            }

            while(sqlQuery.next()){
                QString tagName{ sqlQuery.value("tag_name").toString() };
                tagNames.push_back(tagName);
            }

        }else{

            DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint)};

            if(code == DSqliteHandle::ReturnCode::Exist){

                if(!sqlQuery.exec(sqlStr)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                while(sqlQuery.next()){
                    QString tagName{ sqlQuery.value("tag_name").toString() };
                    tagNames.push_back(tagName);
                }
            }
        }
    }

    return tagNames;
}


template<>
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetFilesThroughTag,
                                          QString, QList<QString>>(const QString& sqlStr, const QString& mountPoint)
{
    QList<QString> files{};

    if(!sqlStr.isEmpty() && !mountPoint.isEmpty()){
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        if(!m_flag.load(std::memory_order_consume)){

            if(sqlQuery.exec(sqlStr)){

                while(sqlQuery.next()){
                    QString fileName{ sqlQuery.value("file_name").toString() };
                    files.push_back(fileName);
                }
            }
        }
    }
    return files;
}




template<>                                           ///###:<file, [tagsName]>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags)
{
    if(!filesAndTags.isEmpty()){
       int size{ filesAndTags.size() };
       QList<QString> mutualTags{};
       QList<QString> currentTags{ filesAndTags.cbegin().value() };

       if(size == 1){
           mutualTags = this->execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(filesAndTags);
       }

       if(size > 1){
           mutualTags = this->execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(filesAndTags);
       } 

       QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(
                                                        DUrl::fromLocalFile(filesAndTags.cbegin().key()), m_partionsOfDevices) };

       if(unixDeviceAndMountPoint.second.isEmpty() || unixDeviceAndMountPoint.second.isNull()){
           return false;
       }

       DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(unixDeviceAndMountPoint.second) };

       ///###: for testing.
//       qDebug()<< "mountpoint: " << unixDeviceAndMountPoint.second;
//       qDebug()<< "mutual tags: " << mutualTags;
//       qDebug()<< "code: " << static_cast<std::size_t>(code);

       ///###: when tag files through many tags.
       ///###: if these files have mutual do this.
       if(!mutualTags.isEmpty()){

           if(code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist){
               this->connectToSqlite(unixDeviceAndMountPoint.second);
               QList<QString> newTags{};
               QList<QString> existingTags{};
               QList<QString> decreased{};

               for (const QString& tagName : currentTags) {
                   if (mutualTags.contains(tagName)) {
                       existingTags.push_back(tagName);
                   } else {
                       newTags.push_back(tagName);
                   }
               }

               if (existingTags.size() < mutualTags.size()) {
                   for (const QString& tagName : mutualTags) {
                       if (!existingTags.contains(tagName))
                           decreased << tagName;
                   }
               }

               if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){
                   bool valueOfDelRedundant{ true };

                   if(!decreased.isEmpty()){
                       QMap<QString, QList<QString>> mapForDelRedundant{};
                       QMap<QString, QList<QString>>::const_iterator cbegOfFiles{ filesAndTags.cbegin() };
                       QMap<QString, QList<QString>>::const_iterator cendOfFiles{ filesAndTags.cend() };

                       for(; cbegOfFiles != cendOfFiles; ++cbegOfFiles){
                           mapForDelRedundant[cbegOfFiles.key()] = decreased;
                       }

                       valueOfDelRedundant = this->helpExecSql<DSqliteHandle::SqlType::TagFiles, QMap<QString, QList<QString>>,
                                                               bool>(mapForDelRedundant, unixDeviceAndMountPoint.second);
                   }

                   bool valueOfInsertNew{ true };

                   if(!newTags.isEmpty()){
                       QMap<QString, QList<QString>>::const_iterator cbegOfFiles{ filesAndTags.cbegin() };
                       QMap<QString, QList<QString>>::const_iterator cendOfFiles{ filesAndTags.cend() };
                       QMap<QString, QList<QString>> mapForInsertingNew{};


                       for(; cbegOfFiles != cendOfFiles; ++cbegOfFiles){
                           mapForInsertingNew[cbegOfFiles.key()] = newTags;
                       }

                       valueOfInsertNew = this->helpExecSql<DSqliteHandle::SqlType::TagFiles2, QMap<QString, QList<QString>>,
                                                                                               bool>(mapForInsertingNew, unixDeviceAndMountPoint.second);
                   }


                   bool valueOfUpdating{ true };
                   QList<QString> files{ filesAndTags.keys() };
                   valueOfUpdating = this->helpExecSql<DSqliteHandle::SqlType::TagFiles3, QList<QString>,
                                                                                          bool>(files, unixDeviceAndMountPoint.second);

                   if(!(valueOfDelRedundant && valueOfInsertNew && valueOfUpdating && m_sqlDatabasePtr->commit())){
                       m_sqlDatabasePtr->rollback();
                       this->closeSqlDatabase();

                       return false;
                   }

                   this->closeSqlDatabase();
                   QMap<QString, QVariant> var_map{};
                   QMap<QString, QList<QString>>::const_iterator the_beg{ filesAndTags.cbegin() };
                   QMap<QString, QList<QString>>::const_iterator the_end{ filesAndTags.cend() };

                   for(; the_beg != the_end; ++the_beg){
                       var_map[the_beg.key()] = QVariant{ the_beg.value() };
                   }

                   emit filesWereTagged(var_map);

                   return true;
               }
           }

           ///###: if these files dont have mutual tag(s).
       }else{

           if(code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist){
               this->connectToSqlite(unixDeviceAndMountPoint.second);

               if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){

                   bool valueOfInsertNew{ true };
                   valueOfInsertNew = this->helpExecSql<DSqliteHandle::SqlType::TagFiles2, QMap<QString, QList<QString>>,
                                                           bool>(filesAndTags, unixDeviceAndMountPoint.second);

                   bool valueOfUpdating{ true };
                   QList<QString> files{ filesAndTags.keys() };
                   valueOfUpdating = this->helpExecSql<DSqliteHandle::SqlType::TagFiles3, QList<QString>,
                                                                                          bool>(files, unixDeviceAndMountPoint.second);

                   if(!(valueOfInsertNew && valueOfUpdating && m_sqlDatabasePtr->commit())){
                       m_sqlDatabasePtr->rollback();
                       this->closeSqlDatabase();

                       return false;
                   }

                   this->closeSqlDatabase();
                   QMap<QString, QVariant> var_map{};
                   QMap<QString, QList<QString>>::const_iterator the_beg{ filesAndTags.cbegin() };
                   QMap<QString, QList<QString>>::const_iterator the_end{ filesAndTags.cend() };

                   for(; the_beg != the_end; ++the_beg){
                       var_map[the_beg.key()] = QVariant{ the_beg.value() };
                   }

                   emit filesWereTagged(var_map);

                   return true;
               }
           }
       }

    }

    this->closeSqlDatabase();

    return false;
}



template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFilesThroughColor, bool>(const QMap<QString, QList<QString>>& filesAndTags)
{
    ///##: log! but you need specify the path of file.
//    std::basic_ofstream<char> outStream{ "", std::ios_base::out | std::ios_base::app };

    if(!filesAndTags.isEmpty()){

        DSqliteHandle::ReturnCode return_code{ this->checkWhetherHasSqliteInPartion("/home", ".__main.db") };

        if(return_code != DSqliteHandle::ReturnCode::Exist){
            return false;
        }

        this->connectToSqlite("/home", ".__main.db");
        bool the_result{ true };

        if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){
            the_result = this->helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor3, QString, bool>(filesAndTags.cbegin().key(), "/home");
        }

        if(!(the_result && m_sqlDatabasePtr->commit())){
            m_sqlDatabasePtr->rollback();
            this->closeSqlDatabase();

            return false;
        }

        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(SqlType::TagFilesThroughColor) };
        std::list<std::tuple<QString, QString, QString, QString, QString, QString>> sqlStrs{};
        QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };

        ///###: log!
//        outStream << "LANG: " << qgetenv("LANG").toStdString()
//                 << ", LANGUAGE: " << qgetenv("LANGUAGE").toStdString() << std::endl;

        if(unixDeviceAndMountPoint.second.isEmpty() || unixDeviceAndMountPoint.second.isNull()){
            return false;
        }

        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(unixDeviceAndMountPoint.second) };

        if(code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist){
            this->connectToSqlite(unixDeviceAndMountPoint.second);

            if(static_cast<bool>(m_sqlDatabasePtr)){

                for(; cbeg != cend; ++cbeg){
                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItrBeg{ range.first };
                    QString sqlForCounting{sqlItrBeg->second.arg(cbeg.value().first())};
                    sqlForCounting = sqlForCounting.arg(cbeg.key());

                    ++sqlItrBeg;

                    QString sqlForInserting{sqlItrBeg->second.arg(cbeg.value().first())};
                    sqlForInserting = sqlForInserting.arg(cbeg.key());

                    ++sqlItrBeg;

                    QString sqlForSelectingTagName{ sqlItrBeg->second.arg(cbeg.key()) };

                    ++sqlItrBeg;

                    QString sqlForInsertingNewRow{ sqlItrBeg->second };

                    std::tuple<QString, QString, QString, QString, QString, QString> sqlTuple{ sqlForCounting, sqlForInserting, sqlForSelectingTagName,
                                                                             sqlForInsertingNewRow, cbeg.key(), cbeg.value().first() };
                    sqlStrs.push_back(std::move(sqlTuple));
                }

                if(!sqlStrs.empty()){
                    bool value{ false };

                    if(m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){
                        value = this->helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor,
                                                  std::list<std::tuple<QString, QString, QString, QString, QString, QString>>,
                                                                                                                    bool>(sqlStrs, unixDeviceAndMountPoint.second);

                        if(value){

                            if(!m_sqlDatabasePtr->commit()){
                                m_sqlDatabasePtr->rollback();
                                this->closeSqlDatabase();

                                return false;
                            }

                            this->closeSqlDatabase();
                            QMap<QString, QVariant> var_map{};
                            QMap<QString, QList<QString>>::const_iterator the_beg{ filesAndTags.cbegin() };
                            QMap<QString, QList<QString>>::const_iterator the_end{ filesAndTags.cend() };

                            for(; the_beg != the_end; ++the_beg){
                                var_map[the_beg.key()] = QVariant{ the_beg.value() };
                            }

                            emit filesWereTagged(var_map);

                            return true;
                        }
                    }
                }
            }
        }
    }

    this->closeSqlDatabase();

    return false;
}





template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagSamePartionFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags)
{
    if(!filesAndTags.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };
        DSqliteHandle::ReturnCode code{
                        this->checkWhetherHasSqliteInPartion(unixDeviceAndMountPoint.second) };

        if(code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist){
            this->connectToSqlite(unixDeviceAndMountPoint.second);

            if(static_cast<bool>(m_sqlDatabasePtr)){
                std::pair<std::multimap<SqlType, QString>::const_iterator,
                          std::multimap<SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(SqlType::UntagSamePartionFiles) };

                if(range.first != range.second){
                    ///###: [sql].
                    std::list<QString> sqlForDeletingRowOfTagWithFile{};

                    for(; cbeg != cend; ++cbeg){
                        std::multimap<SqlType, QString>::const_iterator rangeCBeg{ range.first };
                        QString fileLocalName{ cbeg.key() };
                        QList<QString>::const_iterator cTagBeg{ cbeg.value().cbegin() };
                        QList<QString>::const_iterator cTagEnd{ cbeg.value().cend() };

                        for(; cTagBeg != cTagEnd; ++cTagBeg){
                            QString deletingRowOfTagWithFile{ rangeCBeg->second.arg(fileLocalName) };
                            deletingRowOfTagWithFile = deletingRowOfTagWithFile.arg(*cTagBeg);
                            sqlForDeletingRowOfTagWithFile.push_back( deletingRowOfTagWithFile );
                        }
                    }

                    if(!sqlForDeletingRowOfTagWithFile.empty() && m_sqlDatabasePtr->open()
                                                               && m_sqlDatabasePtr->transaction()){
                        bool resultOfDeleteRowInTagWithFile{ this->helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles,
                                                                               std::list<QString>, bool>(sqlForDeletingRowOfTagWithFile, unixDeviceAndMountPoint.second) };
                        bool resultOfUpdateFileProperty{ false };

                        if(resultOfDeleteRowInTagWithFile){
                            resultOfUpdateFileProperty = this->helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles2,
                                                                               QMap<QString, QList<QString>>, bool>(filesAndTags, unixDeviceAndMountPoint.second);
                        }

                        if(!(resultOfDeleteRowInTagWithFile && resultOfUpdateFileProperty
                                                            && m_sqlDatabasePtr->commit())){
                            m_sqlDatabasePtr->rollback();
                            this->closeSqlDatabase();

                            return false;
                        }

                        this->closeSqlDatabase();

                        return true;
                    }
                }
            }

        }else{

            qWarning("A partion was unmounted just now!");
            this->closeSqlDatabase();

            return false;
        }
    }

    this->closeSqlDatabase();

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagDiffPartionFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags)
{

    if(!filesAndTags.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };

        ///###: <mount-point path, [<files,[tag-name(s)]>]>
        std::unordered_map<QString, QMap<QString, QList<QString>>> untagFilesInSpecifyPartion{};

        ///###: classify the files through mount-point.
        for(; cbeg != cend; ++cbeg){
            QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };

            for(const QString& str : cbeg.value()){
                untagFilesInSpecifyPartion[unixDeviceAndMountPoint.second][cbeg.key()].push_back(str);
            }
        }

        if(!untagFilesInSpecifyPartion.empty()){
            std::unordered_map<QString, QMap<QString, QList<QString>>>::const_iterator partionItrBeg{ untagFilesInSpecifyPartion.cbegin() };
            std::unordered_map<QString, QMap<QString, QList<QString>>>::const_iterator partionItrEnd{ untagFilesInSpecifyPartion.cend() };
            bool result{ true };

            for(; partionItrBeg != partionItrEnd; ++partionItrBeg){
                bool val{ this->execSqlstr<DSqliteHandle::SqlType::UntagSamePartionFiles, bool>(partionItrBeg->second) };
                result = (val && result);
            }

            this->closeSqlDatabase();
            QMap<QString, QVariant> var_map{};
            QMap<QString, QList<QString>>::const_iterator the_beg{ filesAndTags.cbegin() };
            QMap<QString, QList<QString>>::const_iterator the_end{ filesAndTags.cend() };

            for(; the_beg != the_end; ++the_beg){
                var_map[the_beg.key()] = QVariant{ the_beg.value() };
            }

            emit untagFiles(var_map);

            return true;
        }
    }

    this->closeSqlDatabase();

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags)
{

    if(!filesAndTags.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        std::pair<std::map<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::map<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteFiles) };

        ///###: <mount-point, [files]>
        std::map<QString, std::list<QString>> filesOfPartions{};

        ///###: classify files through the mount-point of every file.
        for(; cbeg != cend; ++cbeg){
            QPair<QString, QString> unixDeviceAndMountPoint{
                                    DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };

            if(!unixDeviceAndMountPoint.second.isEmpty() && !unixDeviceAndMountPoint.second.isNull()){
                filesOfPartions[unixDeviceAndMountPoint.second].push_back(cbeg.key());
            }
        }



        ///###: splice sql.
        ///###: <mount-point, [<file, <sql, sql>>]>
        std::map<QString, std::map<QString, std::pair<QString, QString>>> sqlForDeletingFiles{};

        if(!filesOfPartions.empty()){
            std::map<QString, std::list<QString>>::const_iterator partionBeg{ filesOfPartions.cbegin() };
            std::map<QString, std::list<QString>>::const_iterator partionEnd{ filesOfPartions.cend() };

            for(; partionBeg != partionEnd; ++partionBeg){

                for(const QString& file : partionBeg->second){
                    std::map<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
                    std::pair<QString, QString> sqls{ rangeBeg->second.arg(file), (++rangeBeg)->second.arg(file) };
                    sqlForDeletingFiles[partionBeg->first][file] = sqls;
                }
            }

            if(!sqlForDeletingFiles.empty()){
                bool value{ true };

                for(const std::pair<QString, std::map<QString, std::pair<QString, QString>>>& partion : sqlForDeletingFiles){
                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(partion.first) };

                    if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                        this->connectToSqlite(partion.first);

                        if(static_cast<bool>(m_sqlDatabasePtr) && m_sqlDatabasePtr->open()){

                            bool result{ this->helpExecSql<DSqliteHandle::SqlType::DeleteFiles,
                                               std::map<QString, std::pair<QString, QString>>, bool>(partion.second, partion.first) };
                            value = (value && result);
                        }
                    }
                }

                this->closeSqlDatabase();

                return value;
            }
        }
    }

    this->closeSqlDatabase();

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteTags, bool>(const QMap<QString, QList<QString>>& filesAndTags)
{
    if(!filesAndTags.isEmpty()){

        DSqliteHandle::ReturnCode return_code{ this->checkWhetherHasSqliteInPartion("/home", ".__main.db") };

        if(return_code != DSqliteHandle::ReturnCode::Exist){
            return false;
        }

        this->connectToSqlite("/home", ".__main.db");
        bool the_result{ true };
        QList<QString> the_tags_for_deleting{ filesAndTags.keys() };

        if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){
            the_result = this->helpExecSql<DSqliteHandle::SqlType::DeleteTags3, QList<QString>, bool>(the_tags_for_deleting, "/home");
        }

        if(!(the_result && m_sqlDatabasePtr->commit())){
            m_sqlDatabasePtr->rollback();
            this->closeSqlDatabase();

            return false;
        }

        std::list<QString> sqlStrs{};
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteTags) };

        for(; cbeg != cend; ++cbeg){
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
            QString deleteRowOfTagWithFile{ rangeBeg->second.arg(cbeg.key()) };
            sqlStrs.emplace_back(std::move(deleteRowOfTagWithFile));
        }


        if(m_partionsOfDevices && !m_partionsOfDevices->empty()){
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItr{ m_partionsOfDevices->cbegin() };
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItrEnd{ m_partionsOfDevices->cend() };

            bool result{ true };

            for(; deviceItr != deviceItrEnd; ++deviceItr){
                std::multimap<QString, QString>::const_iterator mountPointItr{ deviceItr->second.cbegin() };
                std::multimap<QString, QString>::const_iterator mountPointItrEnd{ deviceItr->second.cend() };

                for(; mountPointItr != mountPointItrEnd; ++mountPointItr){

                    if(!mountPointItr->second.isEmpty() && !mountPointItr->second.isNull()){
                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointItr->second) };

                        if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                            this->connectToSqlite(mountPointItr->second);
                            bool flagForDeleteInTagWithFile{ false };
                            bool flagForUpdatingFileProperty{ false };

                            if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){
                                flagForDeleteInTagWithFile = this->helpExecSql<DSqliteHandle::SqlType::DeleteTags,
                                                                               std::list<QString>, bool>(sqlStrs, mountPointItr->second);

                                if(flagForDeleteInTagWithFile){
                                    flagForUpdatingFileProperty = this->helpExecSql<DSqliteHandle::SqlType::DeleteTags2,
                                                                                    QMap<QString, QList<QString>>, bool>(filesAndTags, mountPointItr->second);
                                }
                            }

                            if(!(flagForDeleteInTagWithFile && flagForUpdatingFileProperty && m_sqlDatabasePtr && m_sqlDatabasePtr->commit())){
                                m_sqlDatabasePtr->rollback();
                                result = false;
                            }
                        }
                    }
                }
            }

            this->closeSqlDatabase();

            if(result){
                emit deleteTags(QVariant{the_tags_for_deleting});
            }

            return result;
        }
    }

    this->closeSqlDatabase();

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeFilesName, bool>(const QMap<QString, QList<QString>>& filesAndTags)
{
    if(!filesAndTags.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::ChangeFilesName) };

        std::map<QString, std::map<QString, QString>> partionsAndFileNames{};

        for(; cbeg != cend; ++cbeg){
            QPair<QString, QString> unixDeviceAndMountPoint{
                DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };

            if(!unixDeviceAndMountPoint.second.isEmpty() && !unixDeviceAndMountPoint.second.isNull()){
                partionsAndFileNames[unixDeviceAndMountPoint.second][cbeg.key()] = cbeg.value().first();
            }
        }

        std::map<QString, std::map<QString, QString>> sqlForChangingFilesName{};

        if(!partionsAndFileNames.empty()){
            std::map<QString, std::map<QString, QString>>::const_iterator partionBeg{ partionsAndFileNames.cbegin() };
            std::map<QString, std::map<QString, QString>>::const_iterator partionEnd{ partionsAndFileNames.cend() };

            for(; partionBeg != partionEnd; ++partionBeg){

                for(const std::pair<QString, QString>& oldAndNewName : partionBeg->second){
                    std::map<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
                    QString updateFileProperty{ rangeBeg->second.arg(oldAndNewName.second)};
                    updateFileProperty = updateFileProperty.arg(oldAndNewName.first);

                    QString updateTagWithFile{ (++rangeBeg)->second.arg(oldAndNewName.second) };
                    updateTagWithFile = updateTagWithFile.arg(oldAndNewName.first);

                    sqlForChangingFilesName[partionBeg->first].emplace(updateFileProperty, updateTagWithFile);
                }
            }


            if(!sqlForChangingFilesName.empty()){
                bool result{ true };

                for(const std::pair<QString, std::map<QString, QString>>& mountPointAndSqls : sqlForChangingFilesName){
                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointAndSqls.first) };

                    if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                        this->connectToSqlite(mountPointAndSqls.first);

                        if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){
                            bool resultOfExecSql{ this->helpExecSql<DSqliteHandle::SqlType::ChangeFilesName,
                                                                    std::map<QString, QString>, bool>(mountPointAndSqls.second, mountPointAndSqls.first) };

                            if(!(resultOfExecSql && m_sqlDatabasePtr->commit())){
                                m_sqlDatabasePtr->rollback();
                                result = false;
                            }
                        }
                    }
                }

                this->closeSqlDatabase();

                return result;
            }
        }
    }

    this->closeSqlDatabase();

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeTagsName, bool>(const QMap<QString, QList<QString>>& filesAndTags)
{
    if(!filesAndTags.isEmpty()){
        DSqliteHandle::ReturnCode return_code{ this->checkWhetherHasSqliteInPartion("/home", ".__main.db") };

        if(return_code != DSqliteHandle::ReturnCode::Exist){
            return false;
        }

        this->connectToSqlite("/home", ".__main.db");
        bool the_result{ true };

        if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){
            the_result = this->helpExecSql<DSqliteHandle::SqlType::ChangeTagsName2, QMap<QString, QList<QString>>, bool>(filesAndTags, "/home");
        }

        if(!(the_result && m_sqlDatabasePtr->commit())){
            m_sqlDatabasePtr->rollback();
            return false;
        }

        QMap<QString, QList<QString>>::const_iterator tagNameBeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator tagNameCend{ filesAndTags.cend() };
        std::list<std::tuple<QString, QString, QString, QString>> sqlStrs{};
        std::pair<std::map<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::map<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::ChangeTagsName) };

        for(; tagNameBeg != tagNameCend; ++tagNameBeg){
            std::map<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
            QString updateTag1{ rangeBeg->second.arg(tagNameBeg.value().first()) };
            updateTag1 = updateTag1.arg(tagNameBeg.key());

            QString updateTag2{ (++rangeBeg)->second.arg(tagNameBeg.value().first()) };
            updateTag2 = updateTag2.arg(tagNameBeg.key());

            QString updateTag3{ (++rangeBeg)->second.arg(tagNameBeg.value().first()) };
            updateTag3 = updateTag3.arg(tagNameBeg.key());

            QString updateTagWithFile{ (++rangeBeg)->second.arg(tagNameBeg.value().first()) };
            updateTagWithFile = updateTagWithFile.arg(tagNameBeg.key());

            sqlStrs.emplace_back(std::move(updateTag1), std::move(updateTag2), std::move(updateTag3), std::move(updateTagWithFile));
        }

        if(m_partionsOfDevices && !m_partionsOfDevices->empty()){
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItr{ m_partionsOfDevices->cbegin() };
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItrEnd{ m_partionsOfDevices->cend() };
            bool result{ true };

            for(; deviceItr != deviceItrEnd; ++deviceItr){
                std::multimap<QString, QString>::const_iterator mountPointItr{ deviceItr->second.cbegin() };
                std::multimap<QString, QString>::const_iterator mountPointItrEnd{ deviceItr->second.cend() };

                for(; mountPointItr != mountPointItrEnd; ++mountPointItr){

                    if(!mountPointItr->second.isEmpty() && !mountPointItr->second.isNull()){
                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointItr->second) };

                        if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                            this->connectToSqlite(mountPointItr->second);
                            bool resultOfChangeNameOfTag{ true };
                            bool flagOfTransaction{ true };

                            if(m_sqlDatabasePtr && m_sqlDatabasePtr->open()){
                                flagOfTransaction = m_sqlDatabasePtr->transaction();

                                if(flagOfTransaction){
                                    resultOfChangeNameOfTag = this->helpExecSql<DSqliteHandle::SqlType::ChangeTagsName,
                                                                                std::list<std::tuple<QString, QString,
                                                                                QString, QString>>, bool>(sqlStrs, mountPointItr->second);
                                }
                            }

                            if(!(resultOfChangeNameOfTag && flagOfTransaction && m_sqlDatabasePtr && m_sqlDatabasePtr->commit())){
                                m_sqlDatabasePtr->rollback();
                                result = false;
                            }
                        }
                    }
                }
            }

            this->closeSqlDatabase();

            if(result){
                QMap<QString, QList<QString>>::const_iterator c_beg{ filesAndTags.cbegin() };
                QMap<QString, QList<QString>>::const_iterator c_end{ filesAndTags.cend() };
                QMap<QString, QVariant> old_and_new_name{};

                for(; c_beg != c_end; ++c_beg){
                    old_and_new_name[c_beg.key()] = QVariant{c_beg.value().first()};
                }

                emit changeTagName(old_and_new_name);
            }

            return result;
        }
    }

    this->closeSqlDatabase();

    return false;
}




template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags)
{
    QList<QString> tags{};

    if(!filesAndTags.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QPair<QString, QString> partionAndMountPoint{ DSqliteHandle::getMountPointOfFile( DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices ) };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::GetTagsThroughFile) };
        QString sqlForGetTagsThroughFile{range.first->second.arg(cbeg.key())};


        if(partionAndMountPoint.second.isEmpty() || partionAndMountPoint.second.isNull()){
            return tags;
        }

        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(partionAndMountPoint.second) };

        if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
            this->connectToSqlite(partionAndMountPoint.second);

            ///###: no transaction.
            if(m_sqlDatabasePtr->open()){
                tags = this->helpExecSql<DSqliteHandle::SqlType::GetTagsThroughFile,
                                             QString, QList<QString>>(sqlForGetTagsThroughFile, partionAndMountPoint.second);
            }
        }
    }

    this->closeSqlDatabase();

    return tags;
}

template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetFilesThroughTag, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags)
{
    QList<QString> files{};

    if(!filesAndTags.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::GetFilesThroughTag) };
        QString sqlForGetFilesThroughTag{ range.first->second.arg(cbeg.key()) };

        if(m_partionsOfDevices && !m_partionsOfDevices->empty()){
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItr{ m_partionsOfDevices->cbegin() };
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItrEnd{ m_partionsOfDevices->cend() };

            for(; deviceItr != deviceItrEnd; ++deviceItr){
                std::multimap<QString, QString>::const_iterator mountPointItr{ deviceItr->second.cbegin() };
                std::multimap<QString, QString>::const_iterator mountPointItrEnd{ deviceItr->second.cend() };

                for(; mountPointItr != mountPointItrEnd; ++mountPointItr){

                    if(!mountPointItr->second.isEmpty() && !mountPointItr->second.isNull()){

                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointItr->second) };

                        if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                            this->connectToSqlite(mountPointItr->second);

                            if(m_sqlDatabasePtr && m_sqlDatabasePtr->open()){

                                QList<QString> filesOfPartion{ this->helpExecSql<DSqliteHandle::SqlType::GetFilesThroughTag,
                                                                     QString, QList<QString>>(sqlForGetFilesThroughTag, mountPointItr->second) };

                                if(!filesOfPartion.isEmpty()){
                                    files += filesOfPartion;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    this->closeSqlDatabase();
    return files;
}


template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags)
{
    QList<QString> totalTagsNames{};
    std::map<QString, std::size_t> countForTags{};

    if(!filesAndTags.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };

        for(; cbeg != cend; ++cbeg){
            QMap<QString, QList<QString>> file{};
            file.insert(cbeg.key(), cbeg.value());

            QList<QString> tagsNames{ this->execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(file) };
            for(const QString& tagName : tagsNames){
                ++countForTags[tagName];
            }
        }
    }


    int size{ filesAndTags.size() };

    for(const std::pair<QString, std::size_t>& pair : countForTags){

        if(pair.second == static_cast<std::size_t>(size)){
            totalTagsNames.push_back(pair.first);
        }
    }

//    qDebug()<< totalTagsNames;

    return totalTagsNames;
}


template<>
QMap<QString, QVariant> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetAllTags, QMap<QString, QVariant>>(const QMap<QString, QList<QString>>& filesAndTags)
{
    (void)filesAndTags;
    QMap<QString, QVariant> tag_and_color{};

    if(QFileInfo::exists("/home")){
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::GetAllTags) };
        this->connectToSqlite("/home", ".__main.db");

        if(m_sqlDatabasePtr && m_sqlDatabasePtr->open()){
            QSqlQuery sql_query{ *m_sqlDatabasePtr };

            if(sql_query.exec(range.first->second)){

                while(sql_query.next()){
                    QString tag_name{ sql_query.value("tag_name").toString() };
                    QString tag_color{ sql_query.value("tag_color").toString() };
                    tag_and_color[tag_name] = QVariant{ tag_color };
                }
            }
        }
    }

    return tag_and_color;
}


template<>
QMap<QString, QVariant> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetTagColor, QMap<QString, QVariant>>(const QMap<QString, QList<QString>>& fileAndTags)
{
    QMap<QString, QVariant> tag_and_color{};

    if(QFileInfo::exists("/home") && !fileAndTags.isEmpty()){
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::GetTagColor) };
        this->connectToSqlite("/home", ".__main.db");

        if(m_sqlDatabasePtr && m_sqlDatabasePtr->open()){
            QMap<QString, QList<QString>>::const_iterator c_beg{ fileAndTags.cbegin() };
            QMap<QString, QList<QString>>::const_iterator c_end{ fileAndTags.cend() };
            QString sql_str{ range.first->second };
            QSqlQuery sql_query{ *m_sqlDatabasePtr };

            for(; c_beg != c_end; ++c_beg){

                if(sql_query.exec(sql_str.arg(c_beg.key()))){

                    if(sql_query.next()){
                        QString tag_color{ sql_query.value("tag_color").toString() };
                        tag_and_color[c_beg.key()] = tag_color;
                    }
                }
            }
        }
    }

    return tag_and_color;
}

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeTagColor, bool>(const QMap<QString, QList<QString>>& filesAndTags)
{
    bool result{ true };
    QMap<QString, QVariant> tag_and_new_color{};

    if(QFileInfo::exists("/home") && !filesAndTags.isEmpty()){
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::ChangeTagColor) };
        this->connectToSqlite("/home", ".__main.db");

        if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){
            QMap<QString, QList<QString>>::const_iterator c_beg{ filesAndTags.cbegin() };
            QMap<QString, QList<QString>>::const_iterator c_end{ filesAndTags.cend() };
            QSqlQuery sql_query{ *m_sqlDatabasePtr };

            for(; c_beg != c_end; ++c_beg){
                QString sql_str{ range.first->second };
                sql_str = sql_str.arg(c_beg.value()[1]);
                sql_str = sql_str.arg(c_beg.key());
                sql_str = sql_str.arg(c_beg.value()[0]);
                tag_and_new_color[c_beg.key()] = QVariant{c_beg.value()[1]};

                if(!sql_query.exec(sql_str)){
                    qWarning()<< sql_query.lastError().text();
                    result = false;

                    break;
                }
            }

            if(!(result && m_sqlDatabasePtr->commit())){
                m_sqlDatabasePtr->rollback();
            }
        }
    }

    if(result){
        emit changeTagColor(tag_and_new_color);
    }

    return result;
}

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::BeforeTagFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags)
{
    if(QFileInfo::exists("/home") && !filesAndTags.isEmpty()){
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::BeforeTagFiles) };
        this->connectToSqlite("/home", ".__main.db");


        if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){

            QMap<QString, QList<QString>>::const_iterator c_beg{ filesAndTags.cbegin() };
            QMap<QString, QList<QString>>::const_iterator c_end{ filesAndTags.cend() };
            QSqlQuery sql_query{ *m_sqlDatabasePtr };
            QString sql_counting{ range.first->second };
            QString sql_inserting{ (++(range.first))->second };

            for(; c_beg != c_end; ++c_beg){
                QString counting{ sql_counting.arg(c_beg.key()) };

                if(sql_query.exec(counting)){

                    if(sql_query.next()){
                        int number{ sql_query.value("counter").toInt() };

                        if(number == 0){
                            sql_query.clear();
                            QString inserting{ sql_inserting.arg(c_beg.key()) };
                            inserting = inserting.arg(c_beg.value().first());

                            if(!sql_query.exec(inserting)){
                                qWarning()<< sql_query.lastError().text();
                                m_sqlDatabasePtr->rollback();

                                this->closeSqlDatabase();

                                return false;
                            }

                            m_newAddedTags.push_back(c_beg.key());
                        }
                    }
                }
            }

            if(!m_sqlDatabasePtr->commit()){
                m_sqlDatabasePtr->rollback();
            }

            this->closeSqlDatabase();

            ///###: emit signal when new tag was added.
            if(!m_newAddedTags.isEmpty()){
                emit addNewTags(QVariant{m_newAddedTags});
                m_newAddedTags.clear();
            }

            return true;
        }
    }

    this->closeSqlDatabase();

    return false;
}
