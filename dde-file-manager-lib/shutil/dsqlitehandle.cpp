

#include <mutex>
#include <string>
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


static std::once_flag flag{};//###: flag for instance.
static std::atomic<int> counter{ 0 };

static constexpr const char* const ROOTPATH{"/"};
static constexpr const std::size_t MAXTHREAD{ 3 };
static constexpr const char* DEFAULTMOUNTPOINT{"/media"};

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
                                                                                                                                       "WHERE tag_with_file.tag_name = \'%1\'"}
                                                      };


DSqliteHandle::DSqliteHandle(QObject * const parent)
              :QObject{ parent }
{
    std::lock_guard<std::mutex> raiiLock{ m_mutex };
    m_partionsOfDevices.reset(new QList<QPair<QString, QList<QPair<QString, QString>>>>{ DSqliteHandle::queryPartionsInfoOfDevices() });
    this->initializeConnect();
}


///###: this is a auxiliary function. so do nont need a mutex.
QPair<QString, QString> DSqliteHandle::getMountPointOfFile(const DUrl& url,
                                                           QScopedPointer<QList<QPair<QString, QList<QPair<QString, QString>>>>> &partionsInfoPtr)
{
    QList<QPair<QString, QList<QPair<QString, QString>>>>::const_iterator cDevicePos{};
    QList<QPair<QString, QString>>::const_iterator cPartionPos{};
    QPair<QString, QString> partionAndMountPoint{};
    QString parentPath{ url.parentUrl().path() };

    if(partionsInfoPtr && !partionsInfoPtr->isEmpty()){
        QList<QPair<QString, QList<QPair<QString, QString>>>>::const_iterator cDeviceBeg{ partionsInfoPtr->cbegin() };
        QList<QPair<QString, QList<QPair<QString, QString>>>>::const_iterator cDeviceEnd{ partionsInfoPtr->cend() };

        for(; cDeviceBeg != cDeviceEnd; ++cDeviceBeg){
            QList<QPair<QString, QString>>::const_iterator cPartionBeg{ cDeviceBeg->second.cbegin() };
            QList<QPair<QString, QString>>::const_iterator cPartionEnd{ cDeviceBeg->second.cend() };
            bool flag{ false };


            for(; cPartionBeg != cPartionEnd; ++cPartionBeg){

                if(cPartionBeg->second != ROOTPATH && parentPath.startsWith(cPartionBeg->second)){
                    cPartionPos = cPartionBeg;
                    flag = true;
                    break;
                }
            }

            if(flag){
                cDevicePos = cDeviceBeg;
                break;
            }
        }

        if(cDevicePos != cDeviceEnd &&
                (cDevicePos != QList<QPair<QString, QList<QPair<QString, QString>>>>::const_iterator{})){
            partionAndMountPoint = *cPartionPos;

        }else{
            cDeviceBeg = partionsInfoPtr->cbegin();
            partionAndMountPoint = *(cDeviceBeg->second.cbegin());
        }

    }

    return partionAndMountPoint;
}


///###: this is a auxiliary function. so do not need a mutex.
QList<QPair<QString, QList<QPair<QString, QString>>>>  DSqliteHandle::queryPartionsInfoOfDevices()
{
    QList<QPair<QString, QList<QPair<QString, QString>>>> partionsOfDevices{};
    QProcess lsblk{};
    QList<QString> arguments{ {"-J"} };
    lsblk.setProgram("lsblk");
    lsblk.setArguments(arguments);
    lsblk.start();

    if(!lsblk.waitForFinished(-1)){
        qWarning(lsblk.readAllStandardError().constData()); //log!
        return partionsOfDevices;
    }

    QJsonParseError error{};
    QJsonDocument document{ QJsonDocument::fromJson(lsblk.readAllStandardOutput(), &error) };

    if(error.error == QJsonParseError::NoError){
        QJsonObject totalObject{ document.object() };

        if(totalObject.contains("blockdevices")){
            QJsonValue blockdevicesValue{ totalObject.take("blockdevices") };

            if(blockdevicesValue.isArray()){
                QJsonArray blockdevicesArray(blockdevicesValue.toArray()); //###: do not use barace to initialize local variable.

                for(const QJsonValue& value : blockdevicesArray){
                    QPair<QString, QList<QPair<QString, QString>>> devicePartions{};

                    if(value.isObject()){
                        QJsonObject objOfValue{ value.toObject() };

                        if(objOfValue.contains("name")){
                            QJsonValue nameValue{ objOfValue.take("name") };
                            devicePartions.first = nameValue.toString();
                        }

                        if(objOfValue.contains("children")){
                            QJsonValue childrenValue{ objOfValue.take("children") };

                            if(childrenValue.isArray()){
                                QJsonArray childrenArray(childrenValue.toArray()); //###: do not use brace!.

                                for(const QJsonValue& value : childrenArray){
                                    QJsonObject theChildren{ value.toObject() };

                                    if(theChildren.contains("name") && theChildren.contains("mountpoint")){
                                        QPair<QString, QString> nameAndMountPoint{};
                                        nameAndMountPoint.first = theChildren.take("name").toString();
                                        QString mountPointOfDevice{ theChildren.take("mountpoint").toString() };

                                        if(!mountPointOfDevice.isNull() && !mountPointOfDevice.isEmpty()){
                                            nameAndMountPoint.second = mountPointOfDevice;
                                            devicePartions.second.push_back(nameAndMountPoint);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    partionsOfDevices.push_back(devicePartions);
                }
            }
        }
    }

    return partionsOfDevices;
}

QSharedPointer<DSqliteHandle> DSqliteHandle::instance()
{
    static QSharedPointer<DSqliteHandle> theInstance{ nullptr };
    std::call_once(flag, [&]{ theInstance = QSharedPointer<DSqliteHandle>{ new DSqliteHandle }; });
    return theInstance;
}

void DSqliteHandle::onMountAdded(UDiskDeviceInfoPointer infoPointer)
{
    m_flag.store(true, std::memory_order_release);
    UDiskDeviceInfo::MediaType mediaType{ infoPointer->getMediaType() };
    QString mountPoint{ infoPointer->getMountPointUrl().toLocalFile() };
    QPair<QString, QString> partion{ infoPointer->getDiskInfo().unix_device(),
                                     mountPoint };

    if((mediaType == UDiskDeviceInfo::MediaType::native || mediaType == UDiskDeviceInfo::MediaType::removable) &&
                                                                        mountPoint.startsWith(DEFAULTMOUNTPOINT)){
        std::lock_guard<std::mutex> raiiLock{ m_mutex };
        QList<QPair<QString, QList<QPair<QString, QString>>>>::iterator beg{ m_partionsOfDevices->begin() };
        QList<QPair<QString, QList<QPair<QString, QString>>>>::iterator end{ m_partionsOfDevices->end() };

        QList<QPair<QString, QList<QPair<QString, QString>>>>::iterator pos{

            std::find_if(beg, end, [&](const QPair<QString, QList<QPair<QString, QString>>>& device)->bool
            {
                if(partion.first.startsWith(device.first) == true){
                    return true;
                }
                return false;
            }
            )
        };

        if(pos != end){
            pos->second.push_back(partion);
        }
    }
    m_flag.store(false, std::memory_order_release);
}

void DSqliteHandle::onMountRemoved(UDiskDeviceInfoPointer infoPointer)
{
    m_flag.store(true, std::memory_order_release);
    UDiskDeviceInfo::MediaType mediaType{ infoPointer->getMediaType() };
    QString unixDevice{ infoPointer->getDiskInfo().unix_device() };
    QString mountPointPath{ infoPointer->getMountPointUrl().path() };

    if((mediaType == UDiskDeviceInfo::MediaType::removable || mediaType == UDiskDeviceInfo::MediaType::native) &&
                                                                       mountPointPath.startsWith(DEFAULTMOUNTPOINT)){
        std::lock_guard<std::mutex> raiiLock{ m_mutex };
        QList<QPair<QString, QList<QPair<QString, QString>>>>::iterator beg{ m_partionsOfDevices->begin() };
        QList<QPair<QString, QList<QPair<QString, QString>>>>::iterator end{ m_partionsOfDevices->end() };
        QList<QPair<QString, QList<QPair<QString, QString>>>>::iterator devicePos{};
        QList<QPair<QString, QString>>::iterator pos{};

        for(; beg != end; ++beg){
            QList<QPair<QString, QString>>::iterator partionBeg{ beg->second.begin() };
            QList<QPair<QString, QString>>::iterator partionEnd{ beg->second.end() };

            pos = std::find_if(partionBeg, partionEnd, [&](const QPair<QString, QString>& partion)
            {
                if(partion.first == unixDevice){
                    return true;
                }
                return false;
            }
                               );

            if(pos != QList<QPair<QString, QString>>::iterator{} &&  pos != beg->second.end()){
                devicePos = beg;
                break;
            }
        }

        if(devicePos != QList<QPair<QString, QList<QPair<QString, QString>>>>::iterator{} &&
                pos != QList<QPair<QString, QString>>::iterator{}){
            devicePos->second.erase(pos);
        }
    }
    m_flag.store(false, std::memory_order_release);
}



QString DSqliteHandle::getConnectionNameFromPartion(const QString& partion) noexcept
{
    if(partion == ROOTPATH){
        return "root";

    }else{

        if(!partion.isNull() && !partion.isEmpty()){
            std::size_t hashValue{ (std::hash<std::string>{}(partion.toStdString())) };
            QString connection{ QString::fromStdString( std::to_string(hashValue) ) };
            return connection;
        }
    }

    return QString{};
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
//            case 2: ///###: untag files(same partion).
//            {
//                std::lock_guard<std::mutex> raiiLock{ m_mutex };
//                m_sqlDatabasePtr=std::unique_ptr<QSqlDatabase>{ nullptr };
//                this->execSqlstr<DSqliteHandle::SqlType::UntagSamePartionFiles>(filesAndTags, userName);

//                break;
//            }
//           case 3: ///###: untag files(diff partion).
//           {
//                std::lock_guard<std::mutex> raiiLock{ m_mutex };
//                this->execSqlstr<DSqliteHandle::SqlType::UntagDiffPartionFiles>(filesAndTags, userName);

//                break;
//           }
//           case 4:
//           {
//                std::lock_guard<std::mutex> raillLock{ m_mutex };
//                this->execSqlstr<DSqliteHandle::SqlType::ChangeTagsName>(filesAndTags, userName);

//                break;
//           }
//           case 5:
//           {
//                std::lock_guard<std::mutex> raillLock{ m_mutex };
//                this->execSqlstr<DSqliteHandle::SqlType::DeleteTags>(filesAndTags, userName);

//                break;
//           }
//           case 6:
//           {
//                std::lock_guard<std::mutex> raillLock{ m_mutex };
//                this->execSqlstr<DSqliteHandle::SqlType::ChangeFilesName>(filesAndTags, userName);

//                break;
//           }
          default:
          {
                break;
          }
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
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles,
                                                        std::list<QString>>( const std::list<QString>& sqlStrs, const QString& mountPoint, const QString& userName)
{
    if(!m_flag.load(std::memory_order_acquire) && !sqlStrs.empty() && !mountPoint.isEmpty() && !userName.isEmpty()){
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
            filesOfPartions[unixDeviceAndMountPoint.second].push_back(cbeg.key());
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
                    std::pair<QString, QString> sqls{
                        rangeBeg->second.arg(file),
                        (++rangeBeg)->second.arg(file)
                    };
                    sqlForDeletingFiles[partionBeg->first][file] = sqls;
                }
            }

            if(!sqlForDeletingFiles.empty()){
                bool value{ true };

                for(const std::pair<QString, std::map<QString, std::pair<QString, QString>>>& partion : sqlForDeletingFiles){
                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(partion.first, userName) };

                    if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                        this->connectToSqlite(partion.first, userName);

                        if(static_cast<bool>(m_sqlDatabasePtr)){
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
            QList<QPair<QString, QList<QPair<QString, QString>>>>::const_iterator deviceItr{ m_partionsOfDevices->cbegin() };
            QList<QPair<QString, QList<QPair<QString, QString>>>>::const_iterator deviceItrEnd{ m_partionsOfDevices->cend() };

            bool result{ true };

            for(; deviceItr != deviceItrEnd; ++deviceItr){
                QList<QPair<QString, QString>>::const_iterator mountPointItr{ deviceItr->second.cbegin() };
                QList<QPair<QString, QString>>::const_iterator mountPointItrEnd{ deviceItr->second.cend() };

                for(; mountPointItr != mountPointItrEnd; ++mountPointItr){

                    if(!mountPointItr->second.isEmpty() && !mountPointItr->second.isNull()){
                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointItr->second, userName) };

                        if(code == DSqliteHandle::ReturnCode::NoExist ||
                           code == DSqliteHandle::ReturnCode::Exist){
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
void DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeFilesName>(const QMap<QString, QList<QString>>& filesAndTags, const QString& userName)
{
    if(!filesAndTags.isEmpty() && !userName.isEmpty()){

        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::ChangeFilesName) };

        ///###: <mount-point, [<OldFileName, NewFileName>]>
        std::map<QString, std::map<QString, QString>> partionsAndFileNames{};

        ///###: classify files through the mount-point of every file.
        for(; cbeg != cend; ++cbeg){
            QPair<QString, QString> unixDeviceAndMountPoint{
                DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };
            partionsAndFileNames[unixDeviceAndMountPoint.second][cbeg.key()] = cbeg.value().first();
        }

        ///###: splice sql.
        ///###: <mount-point, [<sql, sql>]>
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

                for(const std::pair<QString,
                    std::map<QString, QString>>& mountPointAndSqls : sqlForChangingFilesName){
                    DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointAndSqls.first, userName) };

                    if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                        this->connectToSqlite(mountPointAndSqls.first, userName);

                        if(static_cast<bool>(m_sqlDatabasePtr) && m_sqlDatabasePtr->open()
                                && m_sqlDatabasePtr->transaction()){
                            bool resultOfExecSql{ this->helpExecSql<DSqliteHandle::SqlType::ChangeFilesName,
                                        std::map<QString, QString>, bool>(mountPointAndSqls.second, mountPointAndSqls.first, userName) };

                            if(!(resultOfExecSql && m_sqlDatabasePtr->commit())){
                                m_sqlDatabasePtr->rollback();
                            }
                        }
                    }
                }
            }
        }
    }
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
            QList<QPair<QString, QList<QPair<QString, QString>>>>::const_iterator deviceItr{ m_partionsOfDevices->cbegin() };
            QList<QPair<QString, QList<QPair<QString, QString>>>>::const_iterator deviceItrEnd{ m_partionsOfDevices->cend() };

            for(; deviceItr != deviceItrEnd; ++deviceItr){
                QList<QPair<QString, QString>>::const_iterator mountPointItr{ deviceItr->second.cbegin() };
                QList<QPair<QString, QString>>::const_iterator mountPointItrEnd{ deviceItr->second.cend() };

                for(; mountPointItr != mountPointItrEnd; ++mountPointItr){

                    if(!mountPointItr->second.isEmpty() && !mountPointItr->second.isNull()){
                        DSqliteHandle::ReturnCode code{ this->checkWhetherHasSqliteInPartion(mountPointItr->second, userName) };

                        if(code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist){
                            this->connectToSqlite(mountPointItr->second, userName);
                            bool resultOfChangeNameOfTag{ false };
                            bool flagOfTransaction{ false };

                            if(m_sqlDatabasePtr && m_sqlDatabasePtr->open()){
                                flagOfTransaction = m_sqlDatabasePtr->transaction();

                                if(flagOfTransaction){
                                    resultOfChangeNameOfTag = this->helpExecSql<DSqliteHandle::SqlType::ChangeTagsName,
                                                                                std::list<std::tuple<QString, QString,
                                                                                QString, QString>>, bool>(sqlStrs, mountPointItr->second, userName);
                                }
                            }

                            if(!(resultOfChangeNameOfTag && flagOfTransaction &&
                               m_sqlDatabasePtr && m_sqlDatabasePtr->commit())){
                                m_sqlDatabasePtr->rollback();

                                return false;
                            }
                        }
                    }
                }
            }

            return true;
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
            QList<QPair<QString, QList<QPair<QString, QString>>>>::const_iterator deviceItr{ m_partionsOfDevices->cbegin() };
            QList<QPair<QString, QList<QPair<QString, QString>>>>::const_iterator deviceItrEnd{ m_partionsOfDevices->cend() };

            for(; deviceItr != deviceItrEnd; ++deviceItr){
                QList<QPair<QString, QString>>::const_iterator mountPointItr{ deviceItr->second.cbegin() };
                QList<QPair<QString, QString>>::const_iterator mountPointItrEnd{ deviceItr->second.cend() };

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

