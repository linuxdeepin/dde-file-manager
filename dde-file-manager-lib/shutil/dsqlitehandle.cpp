

#include <mutex>
#include <string>
#include <fstream>
#include <algorithm>
#include <functional>
#include <unordered_set>

#include "singleton.h"
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



#include <iostream>


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
                                                          {DSqliteHandle::SqlType::TagFiles, "SELECT COUNT (tag_with_file.file_name) AS counter "
                                                                                                                        "FROM tag_with_file WHERE tag_with_file.file_name = \'%1\' "
                                                                                                                        "AND tag_with_file.tag_name = \'%2\'"},
                                                          {DSqliteHandle::SqlType::TagFiles, "INSERT INTO tag_with_file (file_name, tag_name) VALUES (\'%1\', \'%2\')"},



                                                          {DSqliteHandle::SqlType::TagFiles2, "SELECT COUNT (file_property.file_name) AS counter "
                                                                                                            "FROM file_property WHERE file_property.file_name = \'%1\'" },
                                                          {DSqliteHandle::SqlType::TagFiles2, "INSERT INTO file_property (file_name, tag_1, tag_2, tag_3)  "
                                                                                                                          "VALUES (\'%1\', \'%2\', \'%3\', \'%4\')" },
                                                          {DSqliteHandle::SqlType::TagFiles2, "SELECT * FROM file_property WHERE file_property.file_name = \'%1\'" },
                                                          {DSqliteHandle::SqlType::TagFiles2, "UPDATE file_property SET tag_1 = \'%1\', tag_2 = \'%2\', tag_3 = \'%3\' "
                                                                                                                    "WHERE file_property.file_name = \'%4\'" },



                                                          {DSqliteHandle::SqlType::TagFiles3, "DELETE FROM tag_with_file WHERE tag_with_file.tag_name = \'%1\' "
                                                                                                                         "AND tag_with_file.file_name = \'%2\'"},
                                                          {DSqliteHandle::SqlType::TagFiles3, "SELECT tag_with_file.tag_name FROM tag_with_file "
                                                                                                                             "WHERE tag_with_file.file_name = \'%1\'" },
                                                          {DSqliteHandle::SqlType::TagFiles3, "UPDATE file_property SET tag_1 = \'%1\', tag_2 = \'%2\', tag_3 = \'%3\' "
                                                                                                                                "WHERE file_property.file_name = \'%4\'" },
                                                          {DSqliteHandle::SqlType::TagFiles3, "DELETE FROM file_property WHERE file_property.file_name = \'%1\'"},


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

                                                          {DSqliteHandle::SqlType::DeleteFiles, "DELETE FROM tag_with_file WHERE tag_with_file.tag_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::DeleteFiles, "DELETE FROM file_property WHERE file_property.file_name = \'%1\'"},

                                                          {DSqliteHandle::SqlType::DeleteTags, "DELETE FROM tag_with_file WHERE tag_with_file.tag_name = \'%1\'"},

                                                          {DSqliteHandle::SqlType::DeleteTags2, "SELECT file_property.file_name FROM file_property WHERE file_property.tag_1 = \'%1\' "
                                                                                                                                                    "OR file_property.tag_2 = \'%1\' "
                                                                                                                                                    "OR file_property.tag_3 = \'%1\'"},
                                                          {DSqliteHandle::SqlType::DeleteTags2, "SELECT tag_with_file.tag_name FROM tag_with_file WHERE tag_with_file.file_name = \'%1\'"},
                                                          {DSqliteHandle::SqlType::DeleteTags2, "UPDATE file_property SET tag_1 = \'%1\', tag_2 =\'%2\', tag_3=\'%3\' "
                                                                                                                                        "WHERE file_name = \'%4\'"},

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


        if(partionAndMounpointItr == std::multimap<QString, QString>::const_iterator{} &&
           parentPath.startsWith(ROOTPATH)){
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

QSharedPointer<DSqliteHandle> DSqliteHandle::instance()
{
    static QSharedPointer<DSqliteHandle> theInstance{ nullptr };
    std::call_once(flag, [&]{ theInstance = QSharedPointer<DSqliteHandle>{ new DSqliteHandle }; });
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



QString DSqliteHandle::getConnectionNameFromPartion(const QString& partion) noexcept
{
    if(partion == ROOTPATH){
        return QString{"root"};

    }else{

        if(!partion.isNull() && !partion.isEmpty()){
            std::size_t hashValue{ (std::hash<std::string>{}(partion.toStdString())) };
            QString connection{ QString::fromStdString( std::to_string(hashValue) ) };
            return connection;
        }
    }

    return QString{};
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
QVariant DSqliteHandle::disposeClientData(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName, const std::size_t& type)
{
    QVariant var{};

    if(counter.load(std::memory_order_consume) != 0){
        return var;
    }

    std::size_t numberOfType{ static_cast<std::size_t>(type) };

    if(!filesAndTags.isEmpty() && !userName.isEmpty() && type != 0){

        switch(numberOfType)
        {
            case 1: ///###: tag files!!!!
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::TagFiles, bool>(filesAndTags, userName) };
                var.setValue(value);

                break;
            }
            case 2:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                QStringList filesName{ this->execSqlstr<DSqliteHandle::SqlType::GetFilesThroughTag, QList<QString>>(filesAndTags, userName) };
                var.setValue(filesName);

                break;
            }
            case 3:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                QStringList tags{ this->execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(filesAndTags, userName) };
                var.setValue(tags);

                break;
            }
            case 4: ///###: untag files(diff partion).
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::UntagDiffPartionFiles, bool>(filesAndTags, userName) };
                var.setValue(value);

                break;
            }
            case 5:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::DeleteTags, bool>(filesAndTags, userName) };//###: do not be confused by the name of variant.
                var.setValue(value);

                break;
            }
            case 6:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::ChangeTagsName, bool>(filesAndTags, userName) };

                var.setValue(value);
                break;
            }
            case 7:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::DeleteFiles, bool>(filesAndTags, userName) };

                var.setValue(value);
                break;
            }
            case 8:
            {
                std::lock_guard<std::mutex> raiiLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::TagFilesThroughColor, bool>(filesAndTags, userName) };

                var.setValue(value);
                break;
            }
            case 9:
            {
                std::lock_guard<std::mutex> raillLock{ m_mutex };
                bool value{ this->execSqlstr<DSqliteHandle::SqlType::ChangeFilesName, bool>(filesAndTags, userName) };

                var.setValue(value);
                break;
            }
            default:
                break;
        }
    }

    return var;
}

void DSqliteHandle::lockBackend()noexcept
{
    if(counter.load(std::memory_order_consume) == 0){
        ++counter;
        std::unique_lock<std::mutex> uniqueLock{ m_mutex, std::defer_lock};

        while(!uniqueLock.try_lock()){
            std::this_thread::yield();
        }
        emit backendIsBlocked();
    }
}

void DSqliteHandle::unlockBackend()noexcept
{
    counter.store(0, std::memory_order_release);
}


DSqliteHandle::ReturnCode DSqliteHandle::checkWhetherHasSqliteInPartion(const QString& mountPoint, const QString& userName)
{
    QDir dir{ mountPoint };
    QString DBName{ QString{".__"} + userName + QString{".db"} };

    if(dir.exists()){
        QList<QString> files{ dir.entryList() };

        if(!files.isEmpty()){
            QList<QString>::const_iterator pos{ std::find(files.cbegin(),
                                                          files.cend(), DBName) };
            if(pos != files.cend()){
                return DSqliteHandle::ReturnCode::Exist;
            }
        }
        return DSqliteHandle::ReturnCode::NoExist;
    }
    return DSqliteHandle::ReturnCode::NoThisDir;
}

void DSqliteHandle::initializeConnect()
{
    QObject::connect(deviceListener, &UDiskListener::mountAdded, this, &DSqliteHandle::onMountAdded);
    QObject::connect(deviceListener, &UDiskListener::mountRemoved, this, &DSqliteHandle::onMountRemoved);
}

void DSqliteHandle::connectToSqlite(const QString& mountPoint, const QString& userName)
{
    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };
    std::function<void()> initDatabasePtr{ [&]{
            QString connectionName{ DSqliteHandle::getConnectionNameFromPartion(mountPoint) };

            if(!connectionName.isNull() && !connectionName.isEmpty()){

                if(m_sqlDatabasePtr && m_sqlDatabasePtr->isOpen()){
                    m_sqlDatabasePtr->close();
                }

                if(!QSqlDatabase::contains(connectionName)){
                    m_sqlDatabasePtr = std::unique_ptr<QSqlDatabase>{new QSqlDatabase{ QSqlDatabase::addDatabase(R"foo(QSQLITE)foo", connectionName)} };
                }

                QString DBName{mountPoint + QString{"/.__"} + userName + QString{".db"} };
                qDebug() << DBName;

                m_sqlDatabasePtr->setDatabaseName(DBName);
                m_sqlDatabasePtr->setUserName(USERNAME);
                m_sqlDatabasePtr->setPassword(PASSWORD);

            }else{
                m_sqlDatabasePtr.reset(nullptr);
            }
                                           } };

    if(!m_sqlDatabasePtr){
        m_sqlDatabasePtr = std::unique_ptr<QSqlDatabase>{ new QSqlDatabase };
    }

    if(code == DSqliteHandle::ReturnCode::NoExist){
        initDatabasePtr();

        if(m_sqlDatabasePtr->open()){

            if(m_sqlDatabasePtr->transaction()){
                QSqlQuery sqlQuery{ *m_sqlDatabasePtr };
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

                    if(!m_sqlDatabasePtr->commit()){
                        m_sqlDatabasePtr->rollback();
                    }
                    m_sqlDatabasePtr->close();

                }else{
                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

                    if(code == DSqliteHandle::ReturnCode::NoThisDir){
                        return;
                    }
                }

            }else{
                m_sqlDatabasePtr->close();
                qWarning("failed to open transaction!!!!!!");
            }

        }else{
            qWarning("errors occured when creating a DB in a partion.");
        }

    }else if(code == DSqliteHandle::ReturnCode::Exist){
        initDatabasePtr();
        m_sqlDatabasePtr->close();
    }
}


///###:this is also a auxiliary function. do not need a mutex.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles, QMap<QString, QString>>(const QMap<QString, QString>& sqlStrs, const QString& mountPoint, const QString& userName)
{
    if(!m_flag.load(std::memory_order_acquire) && !sqlStrs.isEmpty()){
        QMap<QString, QString>::const_iterator cbeg{ sqlStrs.cbegin() };
        QMap<QString, QString>::const_iterator cend{ sqlStrs.cend() };
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for(; cbeg != cend; ++cbeg){

            if(m_flag.load(std::memory_order_acquire)){
                DSqliteHandle::ReturnCode code{
                    this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(!sqlQuery.exec(cbeg.key())){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        continue;
                    }

                    if(sqlQuery.next()){
                        int counter{ sqlQuery.value(0).toInt() };

                        if(counter == 0){

                            if(!sqlQuery.exec(cbeg.value())){
                                qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            }
                        }
                    }

                }else if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::NoThisDir){
                    return false;
                }

            }else{

                if(!sqlQuery.exec(cbeg.key())){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    continue;
                }

                if(sqlQuery.next()){
                    int counter{ sqlQuery.value(0).toInt() };

                    if(counter == 0){

                        if(!sqlQuery.exec(cbeg.value())){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        }
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
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles2, QMap<QString, QList<QString>>>(
                                                      const QMap<QString, QList<QString>>& fileNamesAndTagNames, const QString& mountPoint, const QString& userName)
{
    if(!fileNamesAndTagNames.empty() && !mountPoint.isEmpty() && !userName.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ fileNamesAndTagNames.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ fileNamesAndTagNames.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFiles2) };

        std::list<QString> sqlForCounting{};
        for(; cbeg != cend; ++cbeg){
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
            QString sqlStrForCounting{ rangeBeg->second.arg(cbeg.key()) };
            sqlForCounting.push_back(sqlStrForCounting);
        }

        cbeg = fileNamesAndTagNames.cbegin();
        cend = fileNamesAndTagNames.cend();
        std::list<QString> sqlForFileProperty{};
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        std::list<QString>::const_iterator countingSqlBeg{ sqlForCounting.cbegin() };
        std::list<QString>::const_iterator countingSqlEnd{ sqlForCounting.cend() };

        for(; countingSqlBeg != countingSqlEnd && cbeg != cend; ++countingSqlBeg, ++cbeg){

            if(!m_flag.load(std::memory_order_consume)){

                if(!sqlQuery.exec(*countingSqlBeg)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    continue;
                }

                if(sqlQuery.next()){
                    int counting{ sqlQuery.value("counter").toInt() };

                    if(counting > 0){


                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForGetingRow{ range.second };
                        --itrForGetingRow;
                        QString getRowOfFileProperty{ (--itrForGetingRow)->second.arg(cbeg.key()) };
                        std::list<QString> tagNames{};
                        int size{ cbeg.value().size() };

                        if(size >= 3){

                            for(int index = size-3;  index < size; ++index){
                                tagNames.push_back(cbeg.value()[index]);
                            }

                        }else{

                            if(!m_flag.load(std::memory_order_consume)){
                                sqlQuery = QSqlQuery{ *m_sqlDatabasePtr };

                                if(!sqlQuery.exec(getRowOfFileProperty)){
                                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                    continue;
                                }

                                if(sqlQuery.next()){
                                    QString tag_1{ sqlQuery.value("tag_1").toString() };
                                    QString tag_2{ sqlQuery.value("tag_2").toString() };
                                    QString tag_3{ sqlQuery.value("tag_3").toString() };
                                    tagNames = {tag_1, tag_2, tag_3 };

                                    tagNames.erase(std::remove_if(tagNames.begin(), tagNames.end(), [](const QString& tagName){

                                        if(tagName.isEmpty() || tagName.isNull()){
                                            return true;
                                        }
                                        return false;
                                    }), tagNames.end());

                                    for(const QString& tagName : cbeg.value()){
                                        tagNames.emplace_back(tagName);
                                    }

                                    tagNames.sort();
                                    std::list<QString>::const_iterator pos{ std::unique(tagNames.begin(), tagNames.end()) };
                                    tagNames.erase(pos, tagNames.cend());

                                    ///###: if < 3;
                                    if(tagNames.size() < 3){
                                        std::size_t result{ 3u - tagNames.size() };

                                        for(std::size_t index = 0; index < result; ++index){
                                            tagNames.push_back(QString{});
                                        }
                                    }

                                    ///###: if > 3;
                                    if(tagNames.size() > 3){
                                        std::size_t result{ tagNames.size() - 3 };

                                        if(result > 1){
                                            --result;
                                        }

                                        std::list<QString>::const_iterator cbeg{ tagNames.cbegin() };
                                        std::list<QString>::const_iterator last = cbeg;

                                        for(std::size_t index = 0; index < result; ++index){
                                            ++last;
                                        }
                                        tagNames.erase(cbeg, last);
                                    }
                                }

                            }else{

                                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

                                if(code == DSqliteHandle::ReturnCode::Exist){
                                    sqlQuery = QSqlQuery{ *m_sqlDatabasePtr };

                                    if(!sqlQuery.exec(getRowOfFileProperty)){
                                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                        continue;
                                    }

                                    if(sqlQuery.next()){
                                        QString tag_1{ sqlQuery.value("tag_1").toString() };
                                        QString tag_2{ sqlQuery.value("tag_2").toString() };
                                        QString tag_3{ sqlQuery.value("tag_3").toString() };
                                        tagNames = {tag_1, tag_2, tag_3 };


                                        tagNames.erase(std::remove_if(tagNames.begin(), tagNames.end(), [](const QString& tagName){

                                            if(tagName.isEmpty() || tagName.isNull()){
                                                return true;
                                            }
                                            return false;
                                        }), tagNames.end());

                                        for(const QString& tagName : cbeg.value()){
                                            tagNames.emplace_back(tagName);
                                        }

                                        tagNames.sort();
                                        std::list<QString>::const_iterator pos{ std::unique(tagNames.begin(), tagNames.end()) };
                                        tagNames.erase(pos, tagNames.cend());

                                        ///###: if < 3;
                                        if(tagNames.size() < 3){
                                            std::size_t result{ 3u - tagNames.size() };

                                            for(std::size_t index = 0; index < result; ++index){
                                                tagNames.push_back(QString{});
                                            }
                                        }


                                        if(tagNames.size() > 3){
                                            std::size_t result{ tagNames.size() - 3 };
                                            --result;

                                            std::list<QString>::const_iterator cbeg{ tagNames.cbegin() };
                                            std::list<QString>::const_iterator last = cbeg;

                                            for(std::size_t index = 0; index != result; ++index){
                                                ++last;
                                            }
                                            tagNames.erase(cbeg, last);
                                        }
                                    }

                                }else{
                                    return false;
                                }
                            }
                        }

                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeTail{ range.second };
                        std::list<QString>::const_iterator tagItr{ tagNames.cbegin() };
                        --rangeTail;
                        QString updateFileProperty{ rangeTail->second.arg(*tagItr) };
                        updateFileProperty = updateFileProperty.arg(*(++tagItr));
                        updateFileProperty = updateFileProperty.arg(*(++tagItr));
                        updateFileProperty = updateFileProperty.arg(cbeg.key());

                        sqlForFileProperty.push_back(updateFileProperty);

                    }else{
                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForInserting{ range.first };
                        ++itrForInserting;
                        std::list<QString> tagNames{};
                        int size{ cbeg.value().size() };

                        if(size >= 3){

                            for(int index = size - 3; index < size; ++index){
                                tagNames.push_back(cbeg.value()[index]);
                            }

                        }else{

                            for(int index = 0; index <= size - 1; ++index){
                                tagNames.push_back( cbeg.value()[index] );
                            }

                            std::size_t last{ 3u - tagNames.size() };

                            for(std::size_t index = 0; index < last; ++index){
                                tagNames.push_back(QString{});
                            }
                        }

                        std::list<QString>::const_iterator tagItr{ tagNames.cbegin() };
                        QString insertFileProperty{ itrForInserting->second.arg(cbeg.key()) };
                        insertFileProperty = insertFileProperty.arg(*tagItr);
                        insertFileProperty = insertFileProperty.arg(*(++tagItr));
                        insertFileProperty = insertFileProperty.arg(*(++tagItr));

                        sqlForFileProperty.push_back(insertFileProperty);
                    }
                }


            }else{

                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(!sqlQuery.exec(*countingSqlBeg)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        continue;
                    }

                    if(sqlQuery.next()){
                        int counting{ sqlQuery.value("counter").toInt() };

                        if(counting > 0){
                            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForGetingRow{ range.second };
                            --itrForGetingRow;
                            QString getRowOfFileProperty{ (--itrForGetingRow)->second.arg(cbeg.key()) };
                            std::list<QString> tagNames{};
                            int size{ cbeg.value().size() };

                            if(size >= 3){

                                for(int index = size-3;  index < size; ++index){
                                    tagNames.push_back(cbeg.value()[index]);
                                }

                            }else{

                                if(!m_flag.load(std::memory_order_consume)){
                                    sqlQuery = QSqlQuery{ *m_sqlDatabasePtr };

                                    if(!sqlQuery.exec(getRowOfFileProperty)){
                                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                        continue;
                                    }

                                    if(sqlQuery.next()){
                                        QString tag_1{ sqlQuery.value("tag_1").toString() };
                                        QString tag_2{ sqlQuery.value("tag_2").toString() };
                                        QString tag_3{ sqlQuery.value("tag_3").toString() };
                                        tagNames = {tag_1, tag_2, tag_3 };


                                        tagNames.erase(std::remove_if(tagNames.begin(), tagNames.end(), [](const QString& tagName){

                                            if(tagName.isEmpty() || tagName.isNull()){
                                                return true;
                                            }
                                            return false;
                                        }), tagNames.end());

                                        for(const QString& tagName : cbeg.value()){
                                            tagNames.emplace_back(tagName);
                                        }

                                        tagNames.sort();
                                        std::list<QString>::const_iterator pos{ std::unique(tagNames.begin(), tagNames.end()) };
                                        tagNames.erase(pos, tagNames.cend());

                                        ///###: if < 3;
                                        if(tagNames.size() < 3){
                                            std::size_t result{ 3u - tagNames.size() };

                                            for(std::size_t index = 0; index < result; ++index){
                                                tagNames.push_back(QString{});
                                            }
                                        }


                                        if(tagNames.size() > 3){
                                            std::size_t result{ tagNames.size() - 3 };

                                            if(result > 1){
                                                --result;
                                            }

                                            std::list<QString>::const_iterator cbeg{ tagNames.cbegin() };
                                            std::list<QString>::const_iterator last = cbeg;

                                            for(std::size_t index = 0; index != result; ++index){
                                                ++last;
                                            }
                                            tagNames.erase(cbeg, last);
                                        }
                                    }

                                }else{

                                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

                                    if(code == DSqliteHandle::ReturnCode::Exist){
                                        sqlQuery = QSqlQuery{ *m_sqlDatabasePtr };

                                        if(!sqlQuery.exec(getRowOfFileProperty)){
                                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                            continue;
                                        }

                                        if(sqlQuery.next()){
                                            QString tag_1{ sqlQuery.value("tag_1").toString() };
                                            QString tag_2{ sqlQuery.value("tag_2").toString() };
                                            QString tag_3{ sqlQuery.value("tag_3").toString() };
                                            tagNames = {tag_1, tag_2, tag_3 };


                                            tagNames.erase(std::remove_if(tagNames.begin(), tagNames.end(), [](const QString& tagName){

                                                if(tagName.isEmpty() || tagName.isNull()){
                                                    return true;
                                                }
                                                return false;
                                            }), tagNames.end());

                                            for(const QString& tagName : cbeg.value()){
                                                tagNames.emplace_back(tagName);
                                            }

                                            tagNames.sort();
                                            std::list<QString>::const_iterator pos{ std::unique(tagNames.begin(), tagNames.end()) };
                                            tagNames.erase(pos, tagNames.cend());

                                            ///###: if < 3;
                                            if(tagNames.size() < 3){
                                                std::size_t result{ 3u - tagNames.size() };

                                                for(std::size_t index = 0; index < result; ++index){
                                                    tagNames.push_back(QString{});
                                                }
                                            }


                                            if(tagNames.size() > 3){
                                                std::size_t result{ tagNames.size() - 3 };
                                                --result;

                                                std::list<QString>::const_iterator cbeg{ tagNames.cbegin() };
                                                std::list<QString>::const_iterator last = cbeg;

                                                for(std::size_t index = 0; index != result; ++index){
                                                    ++last;
                                                }
                                                tagNames.erase(cbeg, last);
                                            }
                                        }

                                    }else{
                                        return false;
                                    }

                                }
                            }

                            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeTail{ range.second };
                            std::list<QString>::const_iterator tagItr{ tagNames.cbegin() };
                            --rangeTail;
                            QString updateFileProperty{ rangeTail->second.arg(*tagItr) };
                            updateFileProperty = updateFileProperty.arg(*(++tagItr));
                            updateFileProperty = updateFileProperty.arg(*(++tagItr));
                            updateFileProperty = updateFileProperty.arg(cbeg.key());

                            sqlForFileProperty.push_back(updateFileProperty);

                        }else{
                            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForInserting{ ++range.first };
                            std::list<QString> tagNames{};
                            int size{ cbeg.value().size() };

                            if(size >= 3){

                                for(int index = size - 3; index < size; ++index){
                                    tagNames.push_back(cbeg.value()[index]);
                                }

                            }else{

                                for(int index = 0; index <= size - 1; ++index){
                                    tagNames.push_back( cbeg.value()[index] );
                                }

                                std::size_t last{ 3u - tagNames.size() };

                                for(std::size_t index = 0; index < last; ++index){
                                    tagNames.push_back(QString{});
                                }
                            }

                            std::list<QString>::const_iterator tagItr{ tagNames.cbegin() };
                            QString insertFileProperty{ itrForInserting->second.arg(cbeg.key()) };
                            insertFileProperty = insertFileProperty.arg(*tagItr);
                            insertFileProperty = insertFileProperty.arg(*(++tagItr));
                            insertFileProperty = insertFileProperty.arg(*(++tagItr));

                            sqlForFileProperty.push_back(insertFileProperty);
                        }
                    }
                }
            }
        }

        if(!sqlForFileProperty.empty()){

            for(const QString& sql : sqlForFileProperty){

                if(!m_flag.load(std::memory_order_consume)){

                    if(!sqlQuery.exec(sql)){
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        continue;
                    }

                }else{
                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

                    if(code == DSqliteHandle::ReturnCode::Exist){

                        if(!sqlQuery.exec(sql)){
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
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles3, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>>& fileNamesAndTagNames,
                                                                                                        const QString& mountPoint, const QString& userName)
{
    if(!fileNamesAndTagNames.empty() && !mountPoint.isEmpty() && !userName.isEmpty()){
        int size{ fileNamesAndTagNames.size() };
        QList<QString> tagsOfFilesOrFile{};
        QList<QString> newTags{}; //###: this list stores tags which are appended through DTagEdit.
        QList<QString> currentTags{ fileNamesAndTagNames.begin().value() };//###: these tags were got from DTagEdit.

        if(size == 1){
            tagsOfFilesOrFile = this->execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(fileNamesAndTagNames, userName);
        }

        if(size > 1){
            tagsOfFilesOrFile = this->execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(fileNamesAndTagNames, userName);
        }

        if(tagsOfFilesOrFile.isEmpty()){
            return true;
        }

        for(const QString& currentTag : currentTags){
            QList<QString>::const_iterator resultItr{ std::find_if(tagsOfFilesOrFile.cbegin(), tagsOfFilesOrFile.cend(), [&currentTag](const QString& tag){

                if(currentTag == tag){
                    return true;
                }
                return false;
            })};

            if(resultItr == currentTags.cend()){
                newTags.push_back(currentTag); //###: the tag in newTags will be insert to tag_with_file.
            }
        }

        for(const QString& newTag : newTags){
            currentTags.removeAll(newTag);
        }


        ///###: the left Tag(s) in tagsOfFilesOrFile for deleting.
        for(const QString& currentTag : currentTags){
            tagsOfFilesOrFile.removeAll(currentTag);
        }

        if(tagsOfFilesOrFile.isEmpty()){
            return true;
        }


        QMap<QString, QList<QString>>::const_iterator cbeg{ fileNamesAndTagNames.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ fileNamesAndTagNames.cend() };
        QSqlQuery sqlQuery{*m_sqlDatabasePtr};
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFiles3) };
        std::list<QString> sqlStrListForDeleting{};

        for(const QString& tagForDeleting : tagsOfFilesOrFile){

            for(; cbeg != cend; ++cbeg){

                if(!cbeg.key().isEmpty()){
                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItr{ range.first };
                    QString sqlStr{ sqlItr->second.arg(tagForDeleting) };
                    sqlStr = sqlStr.arg(cbeg.key());
                    sqlStrListForDeleting.push_back(sqlStr);
                }
            }
        }


        bool flagForDeleting{ true };

        for(const QString& sqlStr : sqlStrListForDeleting){

            if(!m_flag.load(std::memory_order_consume)){

                if(!sqlQuery.exec(sqlStr)){
                    flagForDeleting = false;
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

            }else{

                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

                if(code == DSqliteHandle::ReturnCode::Exist){

                    if(!sqlQuery.exec(sqlStr)){
                        flagForDeleting = false;
                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                    }

                }else{
                    return false;
                }

            }
        }


        std::list<QString> sqlStrListForSelecting{};
        bool flagForSelecting{ true };

        if(flagForDeleting){
            cbeg = fileNamesAndTagNames.cbegin();

            for(; cbeg != cend; ++cbeg){
                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItr{ range.first };
                ++sqlItr;
                QString sqlStr{ sqlItr->second.arg(cbeg.key()) };
                sqlStrListForSelecting.push_back(sqlStr);
            }

            cbeg = fileNamesAndTagNames.cbegin();

            for(const QString& sqlStr : sqlStrListForSelecting){

                if(sqlQuery.exec(sqlStr)){
                    std::vector<QString> leftTags{};

                    while(sqlQuery.next()){
                        QString tagName{ sqlQuery.value("tag_name").toString() };
                        leftTags.push_back(tagName);
                    }

                    ///###: if leftTags is empty shows that there are not file_name(s) and tag_name(s) as a row in tag_with_file.
                    if(leftTags.empty()){
                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItr{ range.second };
                        --sqlItr;
                        QString updateRowInFileProperty{ sqlItr->second.arg(cbeg.key()) };


                        if(!m_flag.load(std::memory_order_consume)){

                            if(!sqlQuery.exec(updateRowInFileProperty)){
                                flagForSelecting = false;
                                qWarning(sqlQuery.lastError().text().toStdString().c_str());
                            }

                        }else{

                            DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

                            if(code == DSqliteHandle::ReturnCode::Exist){

                                if(!sqlQuery.exec(updateRowInFileProperty)){
                                    flagForSelecting = false;
                                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                }

                            }else{
                                return false;
                            }
                        }

                    }else{

                        std::size_t size{ leftTags.size() };
                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItr{ range.second };
                        --sqlItr;
                        --sqlItr;


                        if(size < 3){
                            std::size_t result{ 3u - size };

                            for(std::size_t index = 0; index < result; ++index){
                                leftTags.push_back(QString{});
                            }
                        }


                        if(leftTags.size() == 3){
                            size = leftTags.size();
                            QString updateRowInFileProperty{ sqlItr->second.arg(leftTags[size-1]) };
                            updateRowInFileProperty = updateRowInFileProperty.arg(leftTags[size-2]);
                            updateRowInFileProperty = updateRowInFileProperty.arg(leftTags[size-3]);
                            updateRowInFileProperty = updateRowInFileProperty.arg(cbeg.key());


                            if(!m_flag.load(std::memory_order_consume)){

                                if(!sqlQuery.exec(updateRowInFileProperty)){
                                    flagForSelecting = false;
                                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                }

                            }else{

                                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

                                if(code == DSqliteHandle::ReturnCode::Exist){

                                    if(!sqlQuery.exec(updateRowInFileProperty)){
                                        flagForSelecting = false;
                                        qWarning(sqlQuery.lastError().text().toStdString().c_str());
                                    }

                                }else{
                                    return false;
                                }
                            }
                        }
                    }
                }

                ++cbeg;
            }
        }

        if(flagForDeleting && flagForSelecting){
            return true;
        }
    }

    return false;
}




template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor,
                    std::list<std::tuple<QString, QString, QString, QString, QString, QString>>,
                    bool>(const std::list<std::tuple<QString, QString, QString, QString, QString, QString>>& sqlStrs,
                                                                                     const QString& mountPoint, const QString& userName)
{
    if(!m_flag.load(std::memory_order_consume) && !sqlStrs.empty() &&
            !mountPoint.isEmpty() && !userName.isEmpty()){
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

                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

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
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles,
                                                        std::list<QString>>( const std::list<QString>& sqlStrs, const QString& mountPoint, const QString& userName)
{
    if(!m_flag.load(std::memory_order_consume) && !sqlStrs.empty() && !mountPoint.isEmpty() && !userName.isEmpty()){
        std::list<QString>::const_iterator cbeg{ sqlStrs.cbegin() };
        std::list<QString>::const_iterator cend{ sqlStrs.cend() };
        QSqlQuery sqlQuery{*m_sqlDatabasePtr};

        for(; cbeg != cend; ++cbeg){

            if(!m_flag.load(std::memory_order_consume)){

                if(!sqlQuery.exec(*cbeg)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

            }else{

                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

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
                                                                                                              const QString& mountPoint, const QString& userName)
{
    if(!fileNameAndTagNames.isEmpty() && !mountPoint.isEmpty() &&
       !userName.isEmpty() && static_cast<bool>(m_sqlDatabasePtr)){
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

                            DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

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

                                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

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

                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

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

                                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

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

                                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

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
                                                                                       const QString& mountPoint, const QString& userName)
{
    if(!sqlStrs.empty() && !mountPoint.isEmpty() && !userName.isEmpty()){
        std::map<QString, std::pair<QString, QString>>::const_iterator cbeg{ sqlStrs.cbegin() };
        std::map<QString, std::pair<QString, QString>>::const_iterator cend{ sqlStrs.cend() };
        QSqlQuery sqlQuery{*m_sqlDatabasePtr};


        for(; cbeg != cend; ++cbeg){

            if(!m_flag.load(std::memory_order_acquire)){

                if(!sqlQuery.exec(cbeg->second.first)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

            }else{
                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

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
                DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPoint, userName) };

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
                                                                                  const QString& mountPoint, const QString& userName)
{
    if(!sqlStrs.empty() && !mountPoint.isEmpty() && !userName.isEmpty()){
        std::list<QString>::const_iterator sqlCBeg{ sqlStrs.cbegin() };
        std::list<QString>::const_iterator sqlCEnd{ sqlStrs.cend() };
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for(; sqlCBeg != sqlCEnd; ++sqlCBeg){

            if(!m_flag.load(std::memory_order_consume)){

                if(!sqlQuery.exec(*sqlCBeg)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

            }else{
                DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint, userName)};

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
                                QMap<QString, QList<QString>>>(const QMap<QString, QList<QString>>& fileNameAndTagNames, const QString& mountPoint, const QString& userName)
{

    if(!fileNameAndTagNames.isEmpty() && !mountPoint.isEmpty() && !userName.isEmpty()){
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

                    DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint, userName)};

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
                    DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint, userName)};

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
                    DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint, userName)};

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
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeFilesName, std::map<QString, QString>>(const std::map<QString, QString>& sqlStrs, const QString& mountPoint, const QString& userName)
{
    if(!sqlStrs.empty() && !mountPoint.isEmpty() && !userName.isEmpty()){
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
                DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint, userName)};

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
                                                                                           const QString& mountPoint, const QString& userName)
{
    if(!sqlStrs.empty() && !mountPoint.isEmpty() && !userName.isEmpty()){
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
                DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint, userName)};

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
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetTagsThroughFile, QString,
                                          QList<QString>>(const QString& sqlStr, const QString& mountPoint, const QString& userName)
{
    QList<QString> tagsNames{};

    if(!sqlStr.isEmpty() && !mountPoint.isEmpty() && !userName.isEmpty()){
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        if(!m_flag.load(std::memory_order_consume)){

            if(!sqlQuery.exec(sqlStr)){
                qWarning(sqlQuery.lastError().text().toStdString().c_str());
            }

            while(sqlQuery.next()){
                QString tagName{ sqlQuery.value("tag_name").toString() };
                tagsNames.push_back(tagName);
            }

        }else{

            DSqliteHandle::ReturnCode code{this->checkWhetherHasSqliteInPartion(mountPoint, userName)};

            if(code == DSqliteHandle::ReturnCode::Exist){
                if(!sqlQuery.exec(sqlStr)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                while(sqlQuery.next()){
                    QString tagName{ sqlQuery.value("tag_name").toString() };
                    tagsNames.push_back(tagName);
                }
            }
        }
    }

    return tagsNames;
}


template<>
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetFilesThroughTag,
                                          QString, QList<QString>>(const QString& sqlStr, const QString& mountPoint, const QString& userName)
{
    QList<QString> files{};

    if(!sqlStr.isEmpty() && !mountPoint.isEmpty() && !userName.isEmpty()){
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        if(!m_flag.load(std::memory_order_consume)){

            if(!sqlQuery.exec(sqlStr)){
                qWarning(sqlQuery.lastError().text().toStdString().c_str());
            }

            while(sqlQuery.next()){
                QString fileName{ sqlQuery.value("file_name").toString() };
                files.push_back(fileName);
            }
        }
    }

    return files;
}




template<>                                           ///###:<file, [tagsName]>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };
        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(unixDeviceAndMountPoint.second, userName) };

        qDebug()<< unixDeviceAndMountPoint.second;

        if(unixDeviceAndMountPoint.second.isEmpty() || unixDeviceAndMountPoint.second.isNull()){
            return false;
        }

        qDebug()<< unixDeviceAndMountPoint.first;

        if(code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist){
            this->connectToSqlite(unixDeviceAndMountPoint.second, userName);

            if(static_cast<bool>(m_sqlDatabasePtr)){
                std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                          std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(SqlType::TagFiles) };

                ///###:<sql, sql>
                QMap<QString, QString> insertRowToTagWithFile{};

                for(; cbeg != cend; ++cbeg){
                    QList<QString>::const_iterator tagCBeg{ cbeg.value().cbegin() };
                    QList<QString>::const_iterator tagCEnd{ cbeg.value().cend() };

                    for(; tagCBeg != tagCEnd; ++tagCBeg){
                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
                        QString sqlForCounting{ rangeBeg->second.arg(cbeg.key()) };
                        sqlForCounting = sqlForCounting.arg(*tagCBeg);

                        ++rangeBeg;

                        QString sqlForInserting{ rangeBeg->second.arg(cbeg.key()) };
                        sqlForInserting = sqlForInserting.arg(*tagCBeg);
                        insertRowToTagWithFile[sqlForCounting] = sqlForInserting;
                    }
                }


                bool resultOfDeletingRedundancy{ false };
                bool resultOfUpdating{ false };
                bool resultOfInserting{ false };

                if(m_sqlDatabasePtr->open()){
                    resultOfDeletingRedundancy = this->helpExecSql<DSqliteHandle::SqlType::TagFiles3,
                                                              QMap<QString, QList<QString>>, bool>(filesAndTags, unixDeviceAndMountPoint.second, userName);

                    if(m_sqlDatabasePtr->isOpen() && m_sqlDatabasePtr->transaction()){
                        resultOfUpdating = this->helpExecSql<DSqliteHandle::SqlType::TagFiles2,
                                                                 QMap<QString, QList<QString>>, bool>(filesAndTags, unixDeviceAndMountPoint.second, userName);

                        if(resultOfUpdating && !insertRowToTagWithFile.isEmpty()){
                            resultOfInserting = this->helpExecSql<DSqliteHandle::SqlType::TagFiles, QMap<QString, QString>, bool>(insertRowToTagWithFile,
                                                                                                                            unixDeviceAndMountPoint.second, userName);
                        }
                    }


                    if(m_sqlDatabasePtr->isOpen()){

                        if(!(resultOfDeletingRedundancy && resultOfInserting &&
                             resultOfUpdating && m_sqlDatabasePtr->commit())){
                            m_sqlDatabasePtr->rollback();
                            qWarning(m_sqlDatabasePtr->lastError().text().toStdString().c_str());

                            return false;
                        }

                        return true;
                    }
                }
            }

        }else{
            qWarning("A partion was unmounted just now!");
            return false;
        }
    }
    return false;
}



template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFilesThroughColor, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(SqlType::TagFilesThroughColor) };
        std::list<std::tuple<QString, QString, QString, QString, QString, QString>> sqlStrs{};

        QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };

        qDebug()<< unixDeviceAndMountPoint;

        DSqliteHandle::ReturnCode code{
                        this->checkWhetherHasSqliteInPartion(unixDeviceAndMountPoint.second, userName) };

        if(code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist){

            qDebug()<< ((bool)m_sqlDatabasePtr);

            if(static_cast<bool>(m_sqlDatabasePtr)){
                qDebug()<< unixDeviceAndMountPoint.second;
                this->connectToSqlite(unixDeviceAndMountPoint.second, userName);

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
                                                  bool>(sqlStrs, unixDeviceAndMountPoint.second, userName);

                        if(value){

                            if(!m_sqlDatabasePtr->commit()){
                                m_sqlDatabasePtr->rollback();

                                return false;
                            }
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}





template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagSamePartionFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };
        DSqliteHandle::ReturnCode code{
                        this->checkWhetherHasSqliteInPartion(unixDeviceAndMountPoint.second, userName) };

        if(code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist){
            this->connectToSqlite(unixDeviceAndMountPoint.second, userName);

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
                                                                               std::list<QString>, bool>(sqlForDeletingRowOfTagWithFile, unixDeviceAndMountPoint.second, userName) };
                        bool resultOfUpdateFileProperty{ false };

                        if(resultOfDeleteRowInTagWithFile){
                            resultOfUpdateFileProperty = this->helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles2,
                                                                               QMap<QString, QList<QString>>, bool>(filesAndTags, unixDeviceAndMountPoint.second, userName);
                        }

                        if(!(resultOfDeleteRowInTagWithFile && resultOfUpdateFileProperty
                                                            && m_sqlDatabasePtr->commit())){
                            m_sqlDatabasePtr->rollback();

                            return false;
                        }

                        return true;
                    }
                }
            }

        }else{
            qWarning("A partion was unmounted just now!");

            return false;
        }
    }

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagDiffPartionFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
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
                bool val{ this->execSqlstr<DSqliteHandle::SqlType::UntagSamePartionFiles, bool>(partionItrBeg->second, userName) };
                result = (val && result);
            }

            return true;
        }
    }

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteFiles, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
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
                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(partion.first, userName) };

                    if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                        this->connectToSqlite(partion.first, userName);

                        if(static_cast<bool>(m_sqlDatabasePtr) && m_sqlDatabasePtr->open()){

                            bool result{ this->helpExecSql<DSqliteHandle::SqlType::DeleteFiles,
                                               std::map<QString, std::pair<QString, QString>>, bool>(partion.second, partion.first, userName) };
                            value = (value && result);
                        }
                    }
                }
                return value;
            }
        }
    }
    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteTags, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
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
                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointItr->second, userName) };

                        if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                            this->connectToSqlite(mountPointItr->second, userName);
                            bool flagForDeleteInTagWithFile{ false };
                            bool flagForUpdatingFileProperty{ false };

                            if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){
                                flagForDeleteInTagWithFile = this->helpExecSql<DSqliteHandle::SqlType::DeleteTags,
                                                                               std::list<QString>, bool>(sqlStrs, mountPointItr->second, userName);

                                if(flagForDeleteInTagWithFile){
                                    flagForUpdatingFileProperty = this->helpExecSql<DSqliteHandle::SqlType::DeleteTags2,
                                                                                    QMap<QString, QList<QString>>, bool>(filesAndTags, mountPointItr->second, userName);
                                }
                            }

                            if(!(flagForDeleteInTagWithFile && flagForUpdatingFileProperty &&
                                                               m_sqlDatabasePtr && m_sqlDatabasePtr->commit())){
                                m_sqlDatabasePtr->rollback();
                                result = false && result;
                            }

                            result = true && result;
                        }
                    }
                }
            }

            return result;
        }
    }

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeFilesName, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
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
                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointAndSqls.first, userName) };

                    if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                        this->connectToSqlite(mountPointAndSqls.first, userName);

                        if(m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()){
                            bool resultOfExecSql{ this->helpExecSql<DSqliteHandle::SqlType::ChangeFilesName,
                                                                    std::map<QString, QString>, bool>(mountPointAndSqls.second, mountPointAndSqls.first, userName) };

                            if(!(resultOfExecSql && m_sqlDatabasePtr->commit())){
                                m_sqlDatabasePtr->rollback();
                                result = false;
                            }
                        }
                    }
                }
                return result;
            }
        }
    }
    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeTagsName, bool>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
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
                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointItr->second, userName) };

                        if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                            this->connectToSqlite(mountPointItr->second, userName);
                            bool resultOfChangeNameOfTag{ true };
                            bool flagOfTransaction{ true };

                            if(m_sqlDatabasePtr && m_sqlDatabasePtr->open()){
                                flagOfTransaction = m_sqlDatabasePtr->transaction();

                                if(flagOfTransaction){
                                    resultOfChangeNameOfTag = this->helpExecSql<DSqliteHandle::SqlType::ChangeTagsName,
                                                                                std::list<std::tuple<QString, QString,
                                                                                QString, QString>>, bool>(sqlStrs, mountPointItr->second, userName);
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
            return result;
        }
    }
    return false;
}




template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    QList<QString> tags{};

    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QPair<QString, QString> partionAndMountPoint{ DSqliteHandle::getMountPointOfFile( DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices ) };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::GetTagsThroughFile) };
        QString sqlForGetTagsThroughFile{range.first->second.arg(cbeg.key())};


        if(partionAndMountPoint.second.isEmpty() || partionAndMountPoint.second.isNull()){
            return tags;
        }

        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(partionAndMountPoint.second, userName) };

        if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
            this->connectToSqlite(partionAndMountPoint.second, userName);

            ///###: no transaction.
            if(m_sqlDatabasePtr->open()){
                tags = this->helpExecSql<DSqliteHandle::SqlType::GetTagsThroughFile,
                                             QString, QList<QString>>(sqlForGetTagsThroughFile, partionAndMountPoint.second, userName);
            }
        }
    }

    return tags;
}

template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetFilesThroughTag, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    QList<QString> files{};

    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
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
                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointItr->second, userName) };

                        if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                            this->connectToSqlite(mountPointItr->second, userName);

                            if(m_sqlDatabasePtr->open()){

                                QList<QString> filesOfPartion{ this->helpExecSql<DSqliteHandle::SqlType::GetFilesThroughTag,
                                                                     QString, QList<QString>>(sqlForGetFilesThroughTag, mountPointItr->second, userName) };

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

    return files;
}


template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    QList<QString> totalTagsNames{};

    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };

        for(; cbeg != cend; ++cbeg){
            QMap<QString, QList<QString>> file{};
            file.insert(cbeg.key(), cbeg.value());

            QList<QString> tagsNames{ this->execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>( file, userName) };
            totalTagsNames.append(tagsNames);
        }
    }

    if(!totalTagsNames.isEmpty()){
        std::list<QString> stdList{ totalTagsNames.toStdList() };
        stdList.sort();
        std::list<QString>::iterator pos{ std::unique(stdList.begin(), stdList.end()) };

        if(pos != stdList.end()){
            stdList.erase(pos, stdList.end());
        }

        totalTagsNames = QList<QString>::fromStdList(stdList);
    }

   return totalTagsNames;
}

