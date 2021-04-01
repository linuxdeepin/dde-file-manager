#include <string>
#include <fstream>
#include <algorithm>
#include <functional>
#include <unordered_set>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "singleton.h"
#include "tag/tagutil.h"
#include "dsqlitehandle.h"

#include "ddiskmanager.h"
#include "dblockdevice.h"

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
#include <QStandardPaths>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

#include "pwd.h"
#include "unistd.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/



namespace std {

template<>
struct less<QString> {
    typedef bool result_type;
    typedef QString first_argument_type;
    typedef QString second_argument_type;
    ///###: delete the codes above. When in c++17.


    bool operator()(const QString &lhs, const QString &rhs) const
    {
        std::string lhStr{ lhs.toStdString() };
        std::string rhStr{ rhs.toStdString() };

        return (std::less<std::string> {}(lhStr, rhStr));
    }
};

}


namespace impl {

#define has_member_function(member)\
    template<typename __Ty>\
    struct has_member_##member\
    {\
    private:\
        template<typename __TR>\
        static auto check(int)->decltype((std::declval<__TR>().member(), std::true_type{}));\
        template<typename __TR>\
        static std::false_type check(...);\
    public:\
        enum { value = std::is_same<decltype(check<__Ty>(0)), std::true_type>::value };\
    };


has_member_function(insert)
has_member_function(cbegin)
has_member_function(cend)
has_member_function(push_back)




template < typename __Ty, typename __TR, typename = typename std::enable_if <
               std::is_same<__Ty, __Ty &>::value &&
               std::is_same<typename std::remove_reference<typename std::remove_cv<__Ty>::type>::type, QMap<QString, QList<QString>>>::value
                                                                                                            >::type >
               __Ty & operator+=(__Ty && lh, const __TR &rh)noexcept
{
    if (rh.isEmpty()) {
        return lh;
    }

    auto itr_beg{ rh.cbegin() };
    auto itr_end{ rh.cend() };

    for (; itr_beg != itr_end; ++itr_beg) {
        lh.insert(itr_beg.key(), itr_beg.value());
    }

    return lh;
}


template < typename __Ty, typename __TR, typename std::enable_if <
               std::is_same<__Ty, __Ty &>::value &&
               has_member_cbegin<typename std::remove_reference<typename std::remove_cv<__Ty>::type>::type>::value &&
               has_member_cend<typename std::remove_reference<typename std::remove_cv<__Ty>::type>::type>::value &&
               has_member_push_back<typename std::remove_reference<typename std::remove_cv<__Ty>::type>::type>::value
               >::type >
__Ty &operator+=(__Ty &&lh, const __TR &rh)noexcept
{
    auto itr_beg{ rh.cbegin() };
    auto itr_end{ rh.cend() };

    for (; itr_beg != itr_end; ++itr_beg) {
        lh.push_back(*itr_beg);
    }

    return lh;
}

}


static std::once_flag flag{};//###: flag for instance.
static std::atomic<int> counter{ 0 };

static constexpr const char *const MOUNTTABLE{"/proc/mounts"};
static constexpr const char *const ROOTPATH{"/"};
static constexpr const std::size_t MAXTHREAD{ 3 };
static constexpr const char *const CONNECTIONNAME{ "deep" };
static constexpr const char *const USERNAME{"username"};
static constexpr const char *const PASSWORD{"password"};


static const std::map<QString, QString> StrTableOfEscapeChar{
    {"\\007", "\a"},
    {"\\010", "\b"},
    {"\\014", "\f"},
    {"\\012", "\n"},
    {"\\015", "\r"},
    {"\\011", "\t"},
    {"\\013", "\v"},
    {"\\134", "\\"},
    {"\\047", "\'"},
    {"\\042", "\""},
    {"\\040", " "}
};


static const std::multimap<DSqliteHandle::SqlType, QString> SqlTypeWithStrs {
    {
        DSqliteHandle::SqlType::BeforeTagFiles, "SELECT COUNT(tag_property.tag_name) AS counter "
        "FROM tag_property WHERE tag_property.tag_name = \'%1\'"
    },
    {
        DSqliteHandle::SqlType::BeforeTagFiles, "INSERT INTO tag_property (tag_name, tag_color) "
        "VALUES(\'%1\', \'%2\')"
    },

    {
        DSqliteHandle::SqlType::TagFiles, "SELECT COUNT (tag_with_file.file_name) AS counter "
        "FROM tag_with_file WHERE tag_with_file.file_name = \'%1\' "
        "AND tag_with_file.tag_name = \'%2\'"
    },
    {DSqliteHandle::SqlType::TagFiles, "INSERT INTO tag_with_file (file_name, tag_name) VALUES (\'%1\', \'%2\')"},
    {
        DSqliteHandle::SqlType::TagFiles, "DELETE FROM tag_with_file WHERE tag_with_file.tag_name = \'%1\' "
        "AND tag_with_file.file_name = \'%2\'"
    },
    {DSqliteHandle::SqlType::TagFiles, "DELETE FROM file_property WHERE file_property.file_name = \'%1\'"},
    {
        DSqliteHandle::SqlType::TagFiles, "SELECT tag_with_file.tag_name FROM tag_with_file "
        "WHERE tag_with_file.file_name = \'%1\'"
    },
    {
        DSqliteHandle::SqlType::TagFiles, "UPDATE file_property SET tag_1 = \'%1\', tag_2 = \'%2\', tag_3 = \'%3\' "
        "WHERE file_property.file_name = \'%4\'"
    },
    {
        DSqliteHandle::SqlType::TagFiles, "SELECT COUNT (file_property.file_name) AS counter "
        "FROM file_property WHERE file_property.file_name = \'%1\'"
    },
    {
        DSqliteHandle::SqlType::TagFiles, "INSERT INTO file_property (file_name, tag_1, tag_2, tag_3) "
        "VALUES(\'%1\', \'%2\', \'%3\', \'%4\')"
    },

    {
        DSqliteHandle::SqlType::ChangeFilesName, "UPDATE file_property SET file_name = \'%1\' "
        "WHERE file_property.file_name = \'%2\'"
    },
    {
        DSqliteHandle::SqlType::ChangeFilesName, "UPDATE tag_with_file SET file_name = \'%1\' "
        "WHERE tag_with_file.file_name = \'%2\'"
    },

    {
        DSqliteHandle::SqlType::ChangeFilesName2, "SELECT tag_with_file.tag_name FROM tag_with_file "
        "WHERE tag_with_file.file_name = \'%1\'"
    },

    {
        DSqliteHandle::SqlType::ChangeTagsName, "UPDATE file_property SET tag_1 = \'%1\' "
        "WHERE file_property.tag_1 = \'%2\'"
    },
    {
        DSqliteHandle::SqlType::ChangeTagsName, "UPDATE file_property SET tag_2 = \'%1\' "
        "WHERE file_property.tag_2 = \'%2\'"
    },
    {
        DSqliteHandle::SqlType::ChangeTagsName, "UPDATE file_property SET tag_3 = \'%1\' "
        "WHERE file_property.tag_3 = \'%2\'"
    },
    {
        DSqliteHandle::SqlType::ChangeTagsName, "UPDATE tag_with_file SET tag_name = \'%1\' "
        "WHERE tag_with_file.tag_name = \'%2\'"
    },
    {
        DSqliteHandle::SqlType::ChangeTagsName2, "UPDATE tag_property SET tag_name = \'%1\' "
        "WHERE tag_property.tag_name = \'%2\'"
    },

    {DSqliteHandle::SqlType::DeleteFiles, "DELETE FROM tag_with_file WHERE tag_with_file.file_name = \'%1\'"},
    {DSqliteHandle::SqlType::DeleteFiles, "DELETE FROM file_property WHERE file_property.file_name = \'%1\'"},

    {
        DSqliteHandle::SqlType::DeleteFiles2, "SELECT tag_with_file.tag_name FROM tag_with_file "
        "WHERE tag_with_file.file_name = \'%1\'"
    },

    {DSqliteHandle::SqlType::DeleteTags, "DELETE FROM tag_with_file WHERE tag_with_file.tag_name = \'%1\'"},

    {
        DSqliteHandle::SqlType::DeleteTags2, "SELECT file_property.file_name FROM file_property WHERE file_property.tag_1 = \'%1\' "
        "OR file_property.tag_2 = \'%1\' "
        "OR file_property.tag_3 = \'%1\'"
    },
    {DSqliteHandle::SqlType::DeleteTags2, "SELECT tag_with_file.tag_name FROM tag_with_file WHERE tag_with_file.file_name = \'%1\'"},
    {
        DSqliteHandle::SqlType::DeleteTags2, "UPDATE file_property SET tag_1 = \'%1\', tag_2 =\'%2\', tag_3=\'%3\' "
        "WHERE file_name = \'%4\'"
    },
    {DSqliteHandle::SqlType::DeleteTags3, "DELETE FROM tag_property WHERE tag_name = \'%1\'"},

    {
        DSqliteHandle::SqlType::DeleteTags4, "SELECT tag_with_file.file_name FROM tag_with_file "
        "WHERE tag_with_file.tag_name = \'%1\'"
    },

    {
        DSqliteHandle::SqlType::UntagSamePartionFiles, "DELETE FROM tag_with_file WHERE tag_with_file.file_name = \'%1\' "
        "AND tag_with_file.tag_name = \'%2\'"
    },

    {
        DSqliteHandle::SqlType::UntagSamePartionFiles2, "SELECT COUNT(tag_with_file.tag_name) AS counter "
        "FROM tag_with_file WHERE tag_with_file.file_name = \'%1\'"
    },
    {
        DSqliteHandle::SqlType::UntagSamePartionFiles2, "SELECT tag_with_file.tag_name FROM tag_with_file "
        "WHERE tag_with_file.file_name = \'%1\'"
    },
    {
        DSqliteHandle::SqlType::UntagSamePartionFiles2, "UPDATE file_property SET tag_1 = \'%1\', tag_2 = \'%2\', tag_3 = \'%3' "
        "WHERE file_property.file_name = \'%4\'"
    },
    {DSqliteHandle::SqlType::UntagSamePartionFiles2, "DELETE FROM file_property WHERE file_property.file_name = \'%1\'"},

    {
        DSqliteHandle::SqlType::UntagDiffPartionFiles, "DELETE FROM tag_with_file WHERE tag_with_file.file_name = \'%1\' "
        "AND tag_with_file.tag_name = \'%2\'"
    },
    {
        DSqliteHandle::SqlType::UntagDiffPartionFiles, "SELECT COUNT(tag_with_file.file_name) AS counter "
        "FROM tag_with_file WHERE tag_with_file.file_name = \'%1\'"
    },
    {DSqliteHandle::SqlType::UntagDiffPartionFiles, "DELETE FROM file_property WHERE file_property.file_name = \'%1\'"},

    {
        DSqliteHandle::SqlType::GetTagsThroughFile, "SELECT tag_with_file.tag_name FROM tag_with_file "
        "WHERE tag_with_file.file_name = \'%1\'"
    },
    {
        DSqliteHandle::SqlType::GetFilesThroughTag, "SELECT tag_with_file.file_name FROM tag_with_file "
        "WHERE tag_with_file.tag_name = \'%1\'"
    },

    {
        DSqliteHandle::SqlType::TagFilesThroughColor, "SELECT COUNT(tag_with_file.tag_name) AS counter FROM tag_with_file "
        "WHERE tag_with_file.tag_name = \'%1\' AND tag_with_file.file_name = \'%2\'"
    },
    {DSqliteHandle::SqlType::TagFilesThroughColor, "INSERT INTO tag_with_file (tag_name, file_name) VALUES(\'%1\', \'%2\')"},
    {
        DSqliteHandle::SqlType::TagFilesThroughColor, "SELECT tag_with_file.tag_name FROM tag_with_file "
        "WHERE tag_with_file.file_name = \'%1\'"
    },
    {
        DSqliteHandle::SqlType::TagFilesThroughColor, "INSERT INTO file_property (file_name, tag_1, tag_2, tag_3)  "
        "VALUES (\'%1\', \'%2\', \'%3\', \'%4\')"
    },

    {
        DSqliteHandle::SqlType::TagFilesThroughColor2, "SELECT COUNT(file_property.file_name) AS counter FROM file_property "
        "WHERE file_name = \'%1\'"
    },
    {
        DSqliteHandle::SqlType::TagFilesThroughColor2, "UPDATE file_property SET tag_1 = \'%1\', tag_2 = \'%2\', tag_3 = \'%3' "
        "WHERE file_property.file_name = \'%4\'"
    },
    {
        DSqliteHandle::SqlType::TagFilesThroughColor3, "SELECT COUNT(tag_property.tag_name) AS counter FROM tag_property "
        "WHERE tag_name = \'%1\'"
    },
    {DSqliteHandle::SqlType::TagFilesThroughColor3, "INSERT INTO tag_property(tag_name, tag_color) VALUES(\'%1\', \'%2\')"},


    {DSqliteHandle::SqlType::GetAllTags, "SELECT * FROM tag_property"},

    {DSqliteHandle::SqlType::GetTagColor, "SELECT * FROM tag_property WHERE tag_property.tag_name = \'%1\'" },

    {
        DSqliteHandle::SqlType::ChangeTagColor, "UPDATE tag_property SET tag_color = \'%1\' "
        "WHERE tag_property.tag_name = \'%2\'"
    }
};


DSqliteHandle::DSqliteHandle(QObject *const parent)
    : QObject{ parent },
      m_sqlDatabasePtr{ new QSqlDatabase }
{
    std::lock_guard<std::mutex> raiiLock{ m_mutex };
    std::map<QString, std::multimap<QString, QString>> partionsAndMounPoints{ DSqliteHandle::queryPartionsInfoOfDevices() };

    if (!partionsAndMounPoints.empty()) {
        m_partionsOfDevices.reset(new std::map<QString, std::multimap<QString, QString>> { partionsAndMounPoints });
    }

    this->initializeConnect();
}


///###: this is a auxiliary function. so do nont need a mutex.
QPair<QString, QString> DSqliteHandle::getMountPointOfFile(DUrl url,
                                                           std::unique_ptr<std::map<QString, std::multimap<QString, QString>>> &partionsAndMountPoints)
{
    while (!DFileInfo::exists(url)) {
        const DUrl &parent_url = url.parentUrl();

        if (!parent_url.isValid() || parent_url == url) {
            break;
        }

        url = parent_url;
    }

    QPair<QString, QString> partionAndMountPoint{};

    if (DFileInfo::exists(url) && partionsAndMountPoints
            && !partionsAndMountPoints->empty()) {
        QString path = url.path();
        std::pair<QString, QString> rootPathPartionAndMountpoint{};
        std::map<QString, std::multimap<QString, QString>>::const_iterator cbeg{ partionsAndMountPoints->cbegin() };
        std::map<QString, std::multimap<QString, QString>>::const_iterator cend{ partionsAndMountPoints->cend() };
        bool flg{ false };

        for (; cbeg != cend; ++cbeg) {
            std::multimap<QString, QString>::const_iterator itrOfPartionAndMountpoint{ cbeg->second.cbegin() };
            std::multimap<QString, QString>::const_iterator itrOfPartionAndMountpointEnd{ cbeg->second.cend() };

            for (; itrOfPartionAndMountpoint != itrOfPartionAndMountpointEnd; ++itrOfPartionAndMountpoint) {

                if (itrOfPartionAndMountpoint->second == ROOTPATH) {
                    rootPathPartionAndMountpoint = *itrOfPartionAndMountpoint;
                }

                if (itrOfPartionAndMountpoint->second != ROOTPATH && path.startsWith(itrOfPartionAndMountpoint->second)) {
                    partionAndMountPoint.first = itrOfPartionAndMountpoint->first;
                    partionAndMountPoint.second = itrOfPartionAndMountpoint->second;
                    flg = true;
                    break;
                }
            }

            if (flg) {
                break;
            }
        }

        if (!flg && path.startsWith(ROOTPATH)) {
            partionAndMountPoint.first = rootPathPartionAndMountpoint.first;
            partionAndMountPoint.second = rootPathPartionAndMountpoint.second;
        }
    }

    return partionAndMountPoint;
}




///###: this is a auxiliary function. so do not need a mutex.
std::map<QString, std::multimap<QString, QString>>  DSqliteHandle::queryPartionsInfoOfDevices()
{
    std::map<QString, std::multimap<QString, QString>> partionsAndMountpoints{};

    if (DFileInfo::exists(DUrl::fromLocalFile(MOUNTTABLE))) {
        std::basic_ifstream<char> iFstream{MOUNTTABLE};
        std::list<std::basic_string<char>> partionInfoAndMountpoint{};
        std::basic_regex<char> matchNumber{ "[0-9]+" };

        if (iFstream.is_open()) {
            std::basic_string<char> str{};


            while (std::getline(iFstream, str)) {

                if (!str.empty()) {
                    std::basic_string<char>::size_type pos{ str.find("/dev/") };

                    if (pos != std::string::npos) {
                        partionInfoAndMountpoint.push_back(str);
                    }
                }
            }
        }

        for (const std::basic_string<char> &theStr : partionInfoAndMountpoint) {
            QString qsTr{ QString::fromStdString(theStr) };
            QString deviceName{};
            QString partionName{};
            QString mountpoint{};

            QList<QString> subsTrs{ qsTr.split(' ') };
            partionName = subsTrs[0];
            mountpoint = subsTrs[1];

            std::match_results<std::basic_string<char>::const_iterator> matchedResult{};
            std::basic_string<char> partionNameStd{ partionName.toStdString() };

            if (std::regex_search(partionNameStd, matchedResult, matchNumber)) {
                deviceName = QString::fromStdString(matchedResult.prefix());

                if (!deviceName.isEmpty() && !deviceName.isNull()) {
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

DSqliteHandle *DSqliteHandle::instance()
{
    static DSqliteHandle *theInstance{ new DSqliteHandle };
    return theInstance;
}

void DSqliteHandle::onMountAdded(UDiskDeviceInfoPointer infoPointer)
{
    (void)infoPointer;
    m_flag.store(true, std::memory_order_release);

    std::lock_guard<std::mutex> raiiLock{ m_mutex };
    std::map<QString, std::multimap<QString, QString>> partionsAndMountPoints{ DSqliteHandle::queryPartionsInfoOfDevices() };
    m_partionsOfDevices.reset(nullptr);

    if (!partionsAndMountPoints.empty()) {
        m_partionsOfDevices = std::unique_ptr<std::map<QString, std::multimap<QString, QString>>> {
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

    if (!partionsAndMountPoints.empty()) {
        m_partionsOfDevices = std::unique_ptr<std::map<QString, std::multimap<QString, QString>>> {
            new std::map<QString, std::multimap<QString, QString>>{ partionsAndMountPoints }
        };
    }
    m_flag.store(false, std::memory_order_release);
}


QString DSqliteHandle::restoreEscapedChar(const QString &value)
{
    QString tempValue{ value };

    if (!tempValue.isEmpty() && !tempValue.isNull()) {

        std::map<QString, QString>::const_iterator tableCBeg{ StrTableOfEscapeChar.cbegin() };
        std::map<QString, QString>::const_iterator tableCEnd{ StrTableOfEscapeChar.cend() };

        for (; tableCBeg != tableCEnd; ++tableCBeg) {
            int pos = tempValue.indexOf(tableCBeg->first);

            if (pos != -1) {

                while (pos != -1) {
                    tempValue = tempValue.replace(tableCBeg->first, tableCBeg->second);

                    pos = tempValue.indexOf(tableCBeg->first);
                }
            }
        }
    }

    return tempValue;
}


///#:-----------------------------------------> <tagName, <tagColor, <files>>>.
QVariant DSqliteHandle::disposeClientData(const QMap<QString, QList<QString>> &filesAndTags, const unsigned long long &type)
{
    QVariant var{};

    if (counter.load(std::memory_order_consume) != 0) {
        return var;
    }

    if (!filesAndTags.isEmpty() && type != 0) {
        std::size_t numberOfType{ static_cast<std::size_t>(type) };
        switch (numberOfType) {
        case 1: { ///###: tag files!!!!
            std::lock_guard<std::mutex> raiiLock{ m_mutex };
            bool value{ this->execSqlstr<DSqliteHandle::SqlType::TagFiles, bool>(filesAndTags) };
            var.setValue(value);

            break;
        }
        case 2: {
            std::lock_guard<std::mutex> raiiLock{ m_mutex };
            QStringList files{ this->execSqlstr<DSqliteHandle::SqlType::GetFilesThroughTag, QList<QString>>(filesAndTags) };
            var.setValue(files);

            break;
        }
        case 3: {
            std::lock_guard<std::mutex> raiiLock{ m_mutex };
            QStringList tags{ this->execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(filesAndTags) };
            var.setValue(tags);

            break;
        }
        case 4: { ///###: untag files(support different partion).
            std::lock_guard<std::mutex> raiiLock{ m_mutex };
            bool value{ this->execSqlstr<DSqliteHandle::SqlType::UntagDiffPartionFiles, bool>(filesAndTags) };
            var.setValue(value);

            break;
        }
        case 5: {
            std::lock_guard<std::mutex> raiiLock{ m_mutex };
            bool value{ this->execSqlstr<DSqliteHandle::SqlType::DeleteTags, bool>(filesAndTags) };//###: do not be confused by the name of variant.
            var.setValue(value);

            break;
        }
        case 6: {
            std::lock_guard<std::mutex> raiiLock{ m_mutex };
            bool value{ this->execSqlstr<DSqliteHandle::SqlType::ChangeTagsName, bool>(filesAndTags) };
            var.setValue(value);

            break;
        }
        case 7: {
            std::lock_guard<std::mutex> raiiLock{ m_mutex };
            bool value{ this->execSqlstr<DSqliteHandle::SqlType::DeleteFiles, bool>(filesAndTags) };
            var.setValue(value);

            break;
        }
        case 8: {
            std::lock_guard<std::mutex> raiiLock{ m_mutex };
            bool value{ this->execSqlstr<DSqliteHandle::SqlType::TagFilesThroughColor, bool>(filesAndTags) };
            var.setValue(value);

            break;
        }
        case 9: {
            std::lock_guard<std::mutex> raillLock{ m_mutex };
            bool value{ this->execSqlstr<DSqliteHandle::SqlType::ChangeFilesName, bool>(filesAndTags) };
            var.setValue(value);

            break;
        }
        case 10: {
            std::lock_guard<std::mutex> raillLock{ m_mutex };
            QMap<QString, QVariant> tag_and_color{ this->execSqlstr<DSqliteHandle::SqlType::GetAllTags, QMap<QString, QVariant>>(filesAndTags) };
            var.setValue(tag_and_color);

            break;
        }
        case 11: {
            std::lock_guard<std::mutex> raii_lock{ m_mutex };
            bool result{ this->execSqlstr<DSqliteHandle::SqlType::BeforeTagFiles, bool>(filesAndTags) };
            var.setValue(result);

            break;
        }
        case 12: {
            std::lock_guard<std::mutex> raii_lock{ m_mutex };
            QMap<QString, QVariant> tag_and_color{ this->execSqlstr<DSqliteHandle::SqlType::GetTagColor, QMap<QString, QVariant>>(filesAndTags) };
            var.setValue(tag_and_color);

            break;
        }
        case 13: {
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

DSqliteHandle::ReturnCode DSqliteHandle::checkDBFileExist(const QString &path, const QString &db_name)
{
    QDir dir(path);

    if (!dir.exists()) {
        return ReturnCode::NoThisDir;
    }

    if (dir.exists(db_name)) {
        return ReturnCode::Exist;
    }

    return ReturnCode::NoExist;
}

void DSqliteHandle::initializeConnect()
{
    QObject::connect(deviceListener, &UDiskListener::mountAdded, this, &DSqliteHandle::onMountAdded);
    QObject::connect(deviceListener, &UDiskListener::mountRemoved, this, &DSqliteHandle::onMountRemoved);
}

void DSqliteHandle::connectToSqlite(const QString &path, const QString &db_name)
{
    DSqliteHandle::ReturnCode code = this->checkDBFileExist(path, db_name);
    std::function<void()> initDatabasePtr{ [&]{
            if (m_sqlDatabasePtr->isOpen())
            {
                m_sqlDatabasePtr->close();
            }

            if (QSqlDatabase::contains(CONNECTIONNAME))
            {
                m_sqlDatabasePtr.reset(nullptr);
                QSqlDatabase::removeDatabase(CONNECTIONNAME);
            }

            m_sqlDatabasePtr = std::unique_ptr<QSqlDatabase>{new QSqlDatabase{ QSqlDatabase::addDatabase(R"foo(QSQLITE)foo", CONNECTIONNAME)} };
            QString DBName{path + QString{"/"} + db_name};

            ///###: for debugging.
//            qDebug() << DBName;

            m_sqlDatabasePtr->setDatabaseName(DBName);
            m_sqlDatabasePtr->setUserName(USERNAME);
            m_sqlDatabasePtr->setPassword(PASSWORD);
        } };

    if (code == DSqliteHandle::ReturnCode::NoExist) {
        initDatabasePtr();

        if (m_sqlDatabasePtr->open()) {
            if (m_sqlDatabasePtr->transaction()) {
                QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

                if (db_name != QString{".__main.db"}) {
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

                    if (!m_flag.load(std::memory_order_consume)) {

                        if (!sqlQuery.exec(createFileProperty)) {
                            qWarning() << sqlQuery.lastError().text();
                        }

                        if (!sqlQuery.exec(createTagWithFile)) {
                            qWarning() << sqlQuery.lastError().text();
                        }

                    } else {
                        DSqliteHandle::ReturnCode return_code{ this->checkDBFileExist(path) };

                        if (return_code != DSqliteHandle::ReturnCode::NoThisDir) {

                            if (!sqlQuery.exec(createFileProperty)) {
                                qWarning() << sqlQuery.lastError().text();
                            }

                            if (!sqlQuery.exec(createTagWithFile)) {
                                qWarning() << sqlQuery.lastError().text();
                            }
                        }
                    }

                } else {

                    QString createTagProperty{
                        "CREATE TABLE IF NOT EXISTS \"tag_property\" "
                        " ("
                        "`tag_index` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                        "`tag_name`  TEXT NOT NULL UNIQUE,"
                        "`tag_color` TEXT NOT NULL"
                        ")"
                    };

                    if (!m_flag.load(std::memory_order_consume)) {

                        if (!sqlQuery.exec(createTagProperty)) {
                            qWarning() << sqlQuery.lastError().text();
                        }

                    } else {
                        DSqliteHandle::ReturnCode return_code{ this->checkDBFileExist(path) };

                        if (return_code != DSqliteHandle::ReturnCode::NoThisDir) {

                            if (!sqlQuery.exec(createTagProperty)) {
                                qWarning() << sqlQuery.lastError().text();
                            }
                        }
                    }
                }

                if (!m_sqlDatabasePtr->commit()) {
                    m_sqlDatabasePtr->rollback();
                }

            } else {
                qWarning() << "failed to open transaction!!!!!!";
                m_sqlDatabasePtr->close();
            }

            QString udiskspath = "";
            for (auto &dev : DDiskManager::blockDevices({})) {
                QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(dev));
                bool match = false;
                for (auto &mp : blk->mountPoints()) {
                    if (path == QString(mp)) {
                        match = true;
                        break;
                    }
                }
                if (match) {
                    udiskspath = dev;
                    break;
                }
            }
            QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(udiskspath));
            if (blk->idType() == "ntfs") {
                QString db_path(path + QString("/") + db_name);
                QByteArray pathBytes(db_path.toUtf8());
                const char *db_path_cs(pathBytes.data());
                Q_UNUSED(db_path_cs)
                quint32 attr;
                pathBytes = db_path.toUtf8();
                getxattr(pathBytes.data(), "system.ntfs_attrib_be", static_cast<void *>(&attr), 4);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
                attr = __builtin_bswap32(attr);
#endif
                attr |= 0x2; // ATTR_HIDDEN
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
                attr = __builtin_bswap32(attr);
#endif
                pathBytes = db_path.toUtf8();
                setxattr(pathBytes.data(), "system.ntfs_attrib_be", static_cast<void *>(&attr), 4, 0);
            }

        } else {
            qWarning() << "errors occured when creating a DB in a partion.";
        }

    } else if (code == DSqliteHandle::ReturnCode::Exist) {
        initDatabasePtr();
    }

    this->closeSqlDatabase();
}

void DSqliteHandle::connectToShareSqlite(const QString &path, const QString &db_name)
{
    //检查share目录中是否已有数据库存在
    DSqliteHandle::ReturnCode code = this->checkDBFileExist(DATABASE_PATH, db_name);

    //如果目录不存在则创建该目录
    if (code == DSqliteHandle::ReturnCode::NoThisDir) {
        int mkdRet = QProcess::execute(QString("mkdir %1").arg(DATABASE_PATH));
        if (mkdRet) {
            code = this->checkDBFileExist(DATABASE_PATH, db_name);
        } else {
            qDebug() << "errors occured when make user local folder";
        }
    }

    if (code == DSqliteHandle::ReturnCode::NoExist) {
        //先判断/home目录下是否有对应数据库文件
        DSqliteHandle::ReturnCode homeCode = this->checkDBFileExist(path, db_name);
        QString mountDBPath = path + "/" + db_name;
        QString shareDBPath = DATABASE_PATH;
        shareDBPath.append("/" + db_name);

        if (homeCode == DSqliteHandle::ReturnCode::Exist) {
            //将旧的db文件迁移到文管共享目录下
            int moveRet = QProcess::execute(QString("mv %1 %2").arg(mountDBPath, shareDBPath));
            if (moveRet) {
                //连接移动后的db
                connectToSqlite(DATABASE_PATH, db_name);
            } else {
                qDebug() << "errors occured when move old db to user path";
            }
        } else if (homeCode == DSqliteHandle::ReturnCode::NoExist) {
            connectToSqlite(DATABASE_PATH, db_name);
        }
    } else if (code == DSqliteHandle::ReturnCode::Exist) {
        connectToSqlite(DATABASE_PATH, db_name);
    }
}

///###:this is also a auxiliary function. do not need a mutex.
template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles, QMap<QString,
                                                                       QList<QString>>, bool>(const QMap<QString, QList<QString>> &forDecreasing, const QString &mountPoint)
{
    if (!forDecreasing.isEmpty() && !mountPoint.isEmpty()) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ forDecreasing.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ forDecreasing.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> itrOfSqlForDeleting{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFiles) };
        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itr{ itrOfSqlForDeleting.first };
        ++itr; ++itr;
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };


        for (; cbeg != cend; ++cbeg) {

            for (const QString &tagName : cbeg.value()) {
                QString sql{ itr->second.arg(tagName) };
                sql = sql.arg(cbeg.key());

                if (!m_flag.load(std::memory_order_acquire)) {

                    ///###: delete redundant item in tag_with_file.
                    if (!sqlQuery.exec(sql)) {
                        qWarning() << sqlQuery.lastError().text();
                        continue;
                    }

                } else {

                    DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                    if (code == DSqliteHandle::ReturnCode::Exist) {

                        if (!sqlQuery.exec(sql)) {
                            qWarning() << sqlQuery.lastError().text();
                            continue;
                        }

                    } else {

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
    const QMap<QString, QList<QString>> &forIncreasing, const QString &mountPoint)
{
    if (!forIncreasing.isEmpty() && !mountPoint.isEmpty()) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ forIncreasing.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ forIncreasing.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> itrOfSqlForDeleting{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFiles) };
        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itr{ itrOfSqlForDeleting.first };
        ++itr;
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for (; cbeg != cend; ++cbeg) {

            for (const QString &tagName : cbeg.value()) {
                QString sql{ itr->second.arg(cbeg.key()) };
                sql = sql.arg(tagName);

                if (!m_flag.load(std::memory_order_acquire)) {

                    ///###: tag files
                    if (!sqlQuery.exec(sql)) {
                        qWarning() << sqlQuery.lastError().text();
                        continue;
                    }

                } else {

                    DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                    if (code == DSqliteHandle::ReturnCode::Exist) {

                        if (!sqlQuery.exec(sql)) {
                            qWarning() << sqlQuery.lastError().text();
                            continue;
                        }

                    } else {

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
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFiles3, QList<QString>, bool>(const QList<QString> &forUpdating,
                                                                                         const QString &mountPoint)
{
    if (!forUpdating.isEmpty() && !mountPoint.isEmpty()) {
        QList<QString>::const_iterator cbeg{ forUpdating.cbegin() };
        QList<QString>::const_iterator cend{ forUpdating.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> itrOfSqlForDeleting{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFiles) };
        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itr{ itrOfSqlForDeleting.first };
        ++itr; ++itr; ++itr; ++itr;
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for (; cbeg != cend; ++cbeg) {
            QString sqlForGettingTag{ itr->second.arg(*cbeg) };
            std::vector<QString> leftTags{};

            if (!m_flag.load(std::memory_order_acquire)) {

                if (sqlQuery.exec(sqlForGettingTag)) {

                    while (sqlQuery.next()) {
                        QString tagName{ sqlQuery.value("tag_name").toString() };
                        leftTags.push_back(tagName);
                    }
                }

            } else {

                DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                if (code == DSqliteHandle::ReturnCode::Exist) {

                    if (sqlQuery.exec(sqlForGettingTag)) {

                        while (sqlQuery.next()) {
                            QString tagName{ sqlQuery.value("tag_name").toString() };
                            leftTags.push_back(tagName);
                        }
                    }

                } else {
                    return false;
                }
            }

            if (leftTags.empty()) {
                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForDelRowInFileProperty{itr};
                --itrForDelRowInFileProperty;
                QString sqlForDelRowInFileProperty{ itrForDelRowInFileProperty->second.arg(*cbeg) };


                if (!m_flag.load(std::memory_order_acquire)) {

                    if (!sqlQuery.exec(sqlForDelRowInFileProperty)) {
                        qWarning() << sqlQuery.lastError().text();
                        continue;
                    }

                } else {

                    DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                    if (code == DSqliteHandle::ReturnCode::Exist) {

                        if (!sqlQuery.exec(sqlForDelRowInFileProperty)) {
                            qWarning() << sqlQuery.lastError().text();
                            continue;
                        }

                    } else {
                        return false;
                    }
                }

            } else {

                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForCounterFileInFP{ itr };
                ++itrForCounterFileInFP; ++itrForCounterFileInFP;
                QString sqlOfCountingFileInFP{ itrForCounterFileInFP->second.arg(*cbeg) };

                int cnter{ 0 };

                if (!m_flag.load(std::memory_order_acquire)) {

                    if (sqlQuery.exec(sqlOfCountingFileInFP)) {

                        if (sqlQuery.next()) {
                            cnter =  sqlQuery.value("counter").toInt();
                        }
                    }

                } else {
                    DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                    if (code == DSqliteHandle::ReturnCode::Exist) {

                        if (sqlQuery.exec(sqlOfCountingFileInFP)) {
                            cnter = sqlQuery.value("counter").toInt();
                        }

                    } else {
                        return false;
                    }
                }

                std::size_t size{ leftTags.size() };

                if (size < 3) {
                    std::size_t differenceValue{ 3u - size };

                    for (std::size_t index = 0; index < differenceValue; ++index) {
                        leftTags.push_back(QString{});
                    }
                }


                if (cnter > 0) {
                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForUpdating{itr};
                    ++itrForUpdating;

                    std::size_t sizeOfTags{ leftTags.size() };

                    QString sqlForUpdatingFileProperty{itrForUpdating->second.arg(leftTags[sizeOfTags - 3])};
                    sqlForUpdatingFileProperty = sqlForUpdatingFileProperty.arg(leftTags[sizeOfTags - 2]);
                    sqlForUpdatingFileProperty = sqlForUpdatingFileProperty.arg(leftTags[sizeOfTags - 1]);
                    sqlForUpdatingFileProperty = sqlForUpdatingFileProperty.arg(*cbeg);

                    if (!m_flag.load(std::memory_order_acquire)) {

                        if (!sqlQuery.exec(sqlForUpdatingFileProperty)) {
                            qWarning() << sqlQuery.lastError().text();
                            continue;
                        }

                    } else {

                        DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                        if (code == DSqliteHandle::ReturnCode::Exist) {

                            if (!sqlQuery.exec(sqlForUpdatingFileProperty)) {
                                qWarning() << sqlQuery.lastError().text();
                                continue;
                            }

                        } else {
                            return false;
                        }

                    }

                } else {

                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForInsertRowInFP{ itr };
                    ++itrForInsertRowInFP;
                    ++itrForInsertRowInFP;
                    ++itrForInsertRowInFP;
                    std::size_t sizeOfTags{ leftTags.size() };
                    QString sqlForInsertRowInFP{ itrForInsertRowInFP->second.arg(*cbeg) };
                    sqlForInsertRowInFP = sqlForInsertRowInFP.arg(leftTags[sizeOfTags - 3]);
                    sqlForInsertRowInFP = sqlForInsertRowInFP.arg(leftTags[sizeOfTags - 2]);
                    sqlForInsertRowInFP = sqlForInsertRowInFP.arg(leftTags[sizeOfTags - 1]);


                    if (!m_flag.load(std::memory_order_acquire)) {

                        if (!sqlQuery.exec(sqlForInsertRowInFP)) {
                            qWarning() << sqlQuery.lastError().text();
                            continue;
                        }

                    } else {

                        DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                        if (code == DSqliteHandle::ReturnCode::Exist) {

                            if (!sqlQuery.exec(sqlForInsertRowInFP)) {
                                qWarning() << sqlQuery.lastError().text();
                                continue;
                            }

                        } else {
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
                                bool>(const std::list<std::tuple<QString, QString, QString, QString, QString, QString>> &sqlStrs,
                                      const QString &mountPoint)
{
    if (!m_flag.load(std::memory_order_consume) && !sqlStrs.empty() && !mountPoint.isEmpty()) {
        std::list<std::tuple<QString, QString, QString, QString, QString, QString>>::const_iterator cbeg{ sqlStrs.cbegin() };
        std::list<std::tuple<QString, QString, QString, QString, QString, QString>>::const_iterator cend{ sqlStrs.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFilesThroughColor2) };


        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };
        bool result{ true };

        for (; cbeg != cend; ++cbeg) {

            if (!m_flag.load(std::memory_order_consume)) {

                if (!sqlQuery.exec(std::get<0>(*cbeg))) {
                    qWarning() << sqlQuery.lastError().text();
                }

                int cnter{ 0 };

                if (sqlQuery.next()) {
                    cnter = sqlQuery.value("counter").toInt();
                }


                if (cnter == 0) {
                    bool flg{ true };

                    if (!sqlQuery.exec(std::get<1>(*cbeg))) {
                        flg = false;
                        qWarning() << sqlQuery.lastError().text();
                    }

                    if (flg) {

                        if (!sqlQuery.exec(std::get<2>(*cbeg))) {
                            qWarning() << sqlQuery.lastError().text();
                        }

                        std::list<QString> tagNames{};

                        while (sqlQuery.next()) {
                            QString tagName{ sqlQuery.value("tag_name").toString() };
                            tagNames.emplace_back(std::move(tagName));
                        }

                        if (!tagNames.empty()) {

                            std::size_t size{ tagNames.size() };

                            if (size < 3) {
                                std::size_t redundant{ 3 - size };

                                for (std::size_t index = 0; index < redundant; ++index) {
                                    tagNames.emplace_back(QString{""});
                                }
                            }

                            QString sqlForCounting{ range.first->second };
                            sqlForCounting = sqlForCounting.arg(std::get<4>(*cbeg));
                            std::list<QString>::const_iterator tagNameItr{ tagNames.cbegin() };

                            if (!sqlQuery.exec(sqlForCounting)) {
                                qWarning() << sqlQuery.lastError().text();
                                result = false;
                                break;
                            }

                            if (sqlQuery.next()) {
                                int cter{ sqlQuery.value("counter").toInt() };

                                if (cter == 0) {
                                    QString sqlForInsertingNewRow{ std::get<3>(*cbeg) };
                                    sqlForInsertingNewRow = sqlForInsertingNewRow.arg(std::get<4>(*cbeg));
                                    sqlForInsertingNewRow = sqlForInsertingNewRow.arg(*tagNameItr);
                                    sqlForInsertingNewRow = sqlForInsertingNewRow.arg(*(++tagNameItr));
                                    sqlForInsertingNewRow = sqlForInsertingNewRow.arg(*(++tagNameItr));

                                    if (!sqlQuery.exec(sqlForInsertingNewRow)) {
                                        qWarning() << sqlQuery.lastError().text();
                                        result = false;
                                        break;
                                    }
                                    continue;

                                } else {
                                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItrForUpdating{ range.first };
                                    ++sqlItrForUpdating;
                                    QString sqlForUpdating{ sqlItrForUpdating->second };
                                    sqlForUpdating = sqlForUpdating.arg(*tagNameItr);
                                    sqlForUpdating = sqlForUpdating.arg(*(++tagNameItr));
                                    sqlForUpdating = sqlForUpdating.arg(*(++tagNameItr));
                                    sqlForUpdating = sqlForUpdating.arg(std::get<4>(*cbeg));

                                    if (!sqlQuery.exec(sqlForUpdating)) {
                                        qWarning() << sqlQuery.lastError().text();
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

            } else {

                DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                if (code == DSqliteHandle::ReturnCode::Exist) {

                    if (!sqlQuery.exec(std::get<0>(*cbeg))) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                    int cnter{ 0 };

                    if (sqlQuery.next()) {
                        cnter = sqlQuery.value("counter").toInt();
                    }


                    if (cnter == 0) {
                        bool flg{ true };

                        if (!sqlQuery.exec(std::get<1>(*cbeg))) {
                            flg = false;
                            qWarning() << sqlQuery.lastError().text();
                        }

                        if (flg) {

                            if (!sqlQuery.exec(std::get<2>(*cbeg))) {
                                qWarning() << sqlQuery.lastError().text();
                            }

                            std::list<QString> tagNames{};

                            while (sqlQuery.next()) {
                                QString tagName{ sqlQuery.value("tag_name").toString() };
                                tagNames.emplace_back(std::move(tagName));
                            }

                            if (!tagNames.empty()) {
                                QString sqlForInsertingNewRow{ std::get<3>(*cbeg) };
                                std::size_t size{ tagNames.size() };

                                if (size < 3) {
                                    std::size_t redundant{ 3 - size };

                                    for (std::size_t index = 0; index < redundant; ++index) {
                                        tagNames.emplace_back(QString{""});
                                    }
                                }

                                if (sqlQuery.next()) {
                                    int cter{ sqlQuery.value("counter").toInt() };
                                    std::list<QString>::const_iterator tagNameItr{ tagNames.cbegin() };

                                    if (cter == 0) {
                                        QString sql_for_inserting_new_row{ std::get<3>(*cbeg) };
                                        sql_for_inserting_new_row = sql_for_inserting_new_row.arg(*tagNameItr);
                                        sql_for_inserting_new_row = sql_for_inserting_new_row.arg(*(++tagNameItr));
                                        sql_for_inserting_new_row = sql_for_inserting_new_row.arg(*(++tagNameItr));
                                        sql_for_inserting_new_row = sql_for_inserting_new_row.arg(std::get<4>(*cbeg));

                                        if (!sqlQuery.exec(sql_for_inserting_new_row)) {
                                            qWarning() << sqlQuery.lastError().text();
                                            result = false;
                                            break;
                                        }
                                        continue;

                                    } else {
                                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItrForUpdating{ range.first };
                                        ++sqlItrForUpdating;
                                        QString sqlForUpdating{ sqlItrForUpdating->second };
                                        sqlForUpdating = sqlForUpdating.arg(*tagNameItr);
                                        sqlForUpdating = sqlForUpdating.arg(*(++tagNameItr));
                                        sqlForUpdating = sqlForUpdating.arg(*(++tagNameItr));
                                        sqlForUpdating = sqlForUpdating.arg(std::get<4>(*cbeg));

                                        if (!sqlQuery.exec(sqlForInsertingNewRow)) {
                                            qWarning() << sqlQuery.lastError().text();
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
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor3, QString, bool>(const QString &tag_name, const QString &mountPoint)
{
    if (!tag_name.isEmpty() && mountPoint == QString{"/home"}) {
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::TagFilesThroughColor3) };
        QSqlQuery sql_query{ *m_sqlDatabasePtr };
        QString sql_counting{ range.first->second.arg(tag_name) };
        QString sql_inserting{ (++range.first)->second.arg(tag_name) };

        if (sql_query.exec(sql_counting)) {

            if (sql_query.next()) {
                int number{ sql_query.value("counter").toInt() };

                if (number == 0) {
                    sql_query.clear();

                    if (!sql_query.exec(sql_inserting)) {
                        qWarning() << sql_query.lastError().text();

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
                                std::list<QString>>(const std::list<QString> &sqlStrs, const QString &mountPoint)
{
    if (!m_flag.load(std::memory_order_consume) && !sqlStrs.empty() && !mountPoint.isEmpty()) {
        std::list<QString>::const_iterator cbeg{ sqlStrs.cbegin() };
        std::list<QString>::const_iterator cend{ sqlStrs.cend() };
        QSqlQuery sqlQuery{*m_sqlDatabasePtr};

        for (; cbeg != cend; ++cbeg) {

            if (!m_flag.load(std::memory_order_consume)) {

                if (!sqlQuery.exec(*cbeg)) {
                    qWarning() << sqlQuery.lastError().text();
                }

            } else {

                DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                if (code == DSqliteHandle::ReturnCode::Exist) {

                    if (!sqlQuery.exec(*cbeg)) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                } else {
                    return false;
                }
            }
        }

        return true;
    }

    return false;
}


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles2, QMap<QString, QList<QString>>>(const QMap<QString, QList<QString>> &fileNameAndTagNames,
                                                                                                               const QString &mountPoint)
{
    if (!fileNameAndTagNames.isEmpty() && !mountPoint.isEmpty() && static_cast<bool>(m_sqlDatabasePtr)) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ fileNameAndTagNames.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ fileNameAndTagNames.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::UntagSamePartionFiles2) };
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for (; cbeg != cend; ++cbeg) {
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
            QString countTagNamesInTagWithFile{ rangeBeg->second.arg(cbeg.key()) };


            if (!m_flag.load(std::memory_order_consume)) {

                if (!sqlQuery.exec(countTagNamesInTagWithFile)) {
                    qWarning() << sqlQuery.lastError().text();
                    continue;
                }

                if (sqlQuery.next()) {
                    int size{ sqlQuery.value(0).toInt() };

                    if (size == 0) {
                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator  rangeEnd{ range.second };
                        QString deleteRowInFileProperty{ (--rangeEnd)->second.arg(cbeg.key()) };

                        if (!m_flag.load(std::memory_order_consume)) {

                            if (!sqlQuery.exec(deleteRowInFileProperty)) {
                                qWarning() << sqlQuery.lastError().text();
                                continue;
                            }

                        } else {

                            DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                            if (code == DSqliteHandle::ReturnCode::Exist) {

                                if (!sqlQuery.exec(deleteRowInFileProperty)) {
                                    qWarning() << sqlQuery.lastError().text();
                                    continue;
                                }

                            } else {
                                return false;
                            }
                        }


                    } else {

                        QString sqlForGetingLeftTag{ (++rangeBeg)->second.arg(cbeg.key()) };

                        if (!m_flag.load(std::memory_order_consume)) {

                            if (!sqlQuery.exec(sqlForGetingLeftTag)) {
                                qWarning() << sqlQuery.lastError().text();
                                continue;
                            }

                            std::deque<QString> leftTags{};

                            while (sqlQuery.next()) {
                                leftTags.push_back(sqlQuery.value("tag_name").toString());
                            }

                            std::size_t sizeOfLeftTags{ leftTags.size() };
                            std::deque<QString>::const_iterator leftTagsCbeg{ leftTags.cbegin() };

                            if (sizeOfLeftTags >= 3u) {
                                leftTags.erase(leftTagsCbeg, leftTagsCbeg + (sizeOfLeftTags - 3u));

                            } else {

                                for (; size < 3; ++size) {
                                    leftTags.push_back(QString{});
                                }
                            }

                            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForUpdateRow{ (--range.second) };
                            leftTagsCbeg = leftTags.cbegin();
                            QString updateRowInFileProperty{ (--itrForUpdateRow)->second.arg(*leftTagsCbeg) };
                            updateRowInFileProperty = updateRowInFileProperty.arg(*(++leftTagsCbeg));
                            updateRowInFileProperty = updateRowInFileProperty.arg(*(++leftTagsCbeg));
                            updateRowInFileProperty = updateRowInFileProperty.arg(cbeg.key());

                            if (!m_flag.load(std::memory_order_consume)) {

                                if (!sqlQuery.exec(updateRowInFileProperty)) {
                                    qWarning() << sqlQuery.lastError().text();
                                    continue;
                                }

                            } else {

                                DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                                if (code == DSqliteHandle::ReturnCode::Exist) {

                                    if (!sqlQuery.exec(updateRowInFileProperty)) {
                                        qWarning() << sqlQuery.lastError().text();
                                        continue;
                                    }

                                } else {
                                    return false;
                                }

                            }
                        }
                    }
                }

            } else {

                DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPoint) };

                if (code == DSqliteHandle::ReturnCode::Exist) {

                    if (sqlQuery.exec(countTagNamesInTagWithFile)) {
                        qWarning() << sqlQuery.lastError().text();
                        continue;
                    }

                    if (sqlQuery.next()) {

                        int size{ sqlQuery.value(0).toInt() };

                        if (size == 0) {
                            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator  rangeEnd{ range.second };
                            QString deleteRowInFileProperty{ (--rangeEnd)->second.arg(cbeg.key()) };

                            if (!m_flag.load(std::memory_order_consume)) {

                                if (!sqlQuery.exec(deleteRowInFileProperty)) {
                                    qWarning() << sqlQuery.lastError().text();
                                    continue;
                                }

                            } else {

                                DSqliteHandle::ReturnCode return_code{ this->checkDBFileExist(mountPoint) };

                                if (return_code == DSqliteHandle::ReturnCode::Exist) {

                                    if (!sqlQuery.exec(deleteRowInFileProperty)) {
                                        qWarning() << sqlQuery.lastError().text();
                                        continue;
                                    }

                                } else {
                                    return false;
                                }
                            }


                        } else {

                            QString sqlForGetingLeftTag{ (++rangeBeg)->second };

                            if (!m_flag.load(std::memory_order_consume)) {

                                if (!sqlQuery.exec(sqlForGetingLeftTag)) {
                                    qWarning() << sqlQuery.lastError().text();
                                    continue;
                                }

                                std::deque<QString> leftTags{};
                                int sizeOfTags{ sqlQuery.size() };

                                for (long int index = 0; index < sizeOfTags; ++index) {
                                    leftTags.push_back(sqlQuery.value(index).toString());
                                }

                                std::size_t sizeOfLeftTags{ leftTags.size() };
                                std::deque<QString>::const_iterator leftTagsCbeg{ leftTags.cbegin() };

                                if (sizeOfLeftTags >= 3u) {
                                    leftTags.erase(leftTagsCbeg, leftTagsCbeg + (sizeOfLeftTags - 3u));

                                } else {

                                    for (; size < 3; ++size) {
                                        leftTags.push_back(QString{});
                                    }
                                }

                                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator itrForUpdateRow{ (--range.second) };
                                leftTagsCbeg = leftTags.cbegin();
                                QString updateRowInFileProperty{ (--itrForUpdateRow)->second.arg(*leftTagsCbeg) };
                                updateRowInFileProperty = updateRowInFileProperty.arg(*(++leftTagsCbeg));
                                updateRowInFileProperty = updateRowInFileProperty.arg(*(++leftTagsCbeg));
                                updateRowInFileProperty = updateRowInFileProperty.arg(cbeg.key());

                                if (!m_flag.load(std::memory_order_consume)) {

                                    if (!sqlQuery.exec(updateRowInFileProperty)) {
                                        qWarning() << sqlQuery.lastError().text();
                                        continue;
                                    }

                                } else {

                                    DSqliteHandle::ReturnCode return_code{ this->checkDBFileExist(mountPoint) };

                                    if (return_code == DSqliteHandle::ReturnCode::Exist) {

                                        if (!sqlQuery.exec(updateRowInFileProperty)) {
                                            qWarning() << sqlQuery.lastError().text();
                                            continue;
                                        }

                                    } else {
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
                                std::list<QString>, bool>(const std::list<QString> &files, const QString &mount_point)
{

    if (!files.empty() && !mount_point.isEmpty()) {
        std::list<QString>::const_iterator file_itr_beg{ files.cbegin() };
        std::list<QString>::const_iterator file_itr_end{ files.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteFiles) };
        QSqlQuery sql_query{ *m_sqlDatabasePtr };

        if (!m_flag.load(std::memory_order_acquire)) {

            for (; file_itr_beg != file_itr_end; ++file_itr_beg) {
                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator range_beg{ range.first };
                QString clear_tag_with_file_table{ range_beg->second.arg(*file_itr_beg) };
                QString clear_file_property_table{ (++range_beg)->second.arg(*file_itr_beg) };

                if (!sql_query.exec(clear_file_property_table)) {
                    qWarning() << sql_query.lastError().text();

                    return false;
                }

                if (!sql_query.exec(clear_tag_with_file_table)) {
                    qWarning() << sql_query.lastError().text();

                    return false;
                }
            }

            return true;

        } else {

            DSqliteHandle::ReturnCode code{this->checkDBFileExist(mount_point)};

            if (code == DSqliteHandle::ReturnCode::Exist) {

                for (; file_itr_beg != file_itr_end; ++file_itr_beg) {
                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator range_beg{ range.first };
                    QString clear_tag_with_file_table{ range_beg->second.arg(*file_itr_beg) };
                    QString clear_file_property_table{ (++range_beg)->second.arg(*file_itr_beg) };

                    if (sql_query.exec(clear_file_property_table)) {
                        qWarning() << sql_query.lastError().text();

                        return false;
                    }

                    if (sql_query.exec(clear_tag_with_file_table)) {
                        qWarning() << sql_query.lastError().text();

                        return false;
                    }
                }

                return true;
            }
        }
    }

    return false;
}

template<>
QMap<QString, QList<QString>> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteFiles2,
                                                         std::list<QString>, QMap<QString, QList<QString>>>(const std::list<QString> &files, const QString &mount_point)
{
    QMap<QString, QList<QString>> file_and_tags{};

    if (!files.empty() && !mount_point.isEmpty()) {
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteFiles2) };
        QSqlQuery sql_query{ *m_sqlDatabasePtr };


        if (!m_flag.load(std::memory_order_acquire)) {

            for (const QString &file : files) {
                QString sql_for_getting_tags{ range.first->second.arg(file) };

                if (sql_query.exec(sql_for_getting_tags)) {

                    while (sql_query.next()) {
                        QString tag_name{ sql_query.value("tag_name").toString() };
                        file_and_tags[file].push_back(tag_name);
                    }
                }
            }

        } else {

            DSqliteHandle::ReturnCode code{this->checkDBFileExist(mount_point)};

            if (code == DSqliteHandle::ReturnCode::Exist) {

                for (const QString &file : files) {
                    QString sql_for_getting_tags{ range.first->second.arg(file) };

                    if (sql_query.exec(sql_for_getting_tags)) {

                        while (sql_query.next()) {
                            QString tag_name{ sql_query.value("tag_name").toString() };
                            file_and_tags[file].push_back(tag_name);
                        }
                    }
                }

            }
        }
    }

    return file_and_tags;
}




template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags,
                                std::list<QString>>(const std::list<QString> &sqlStrs,
                                                    const QString &mountPoint)
{
    if (!sqlStrs.empty() && !mountPoint.isEmpty()) {
        std::list<QString>::const_iterator sqlCBeg{ sqlStrs.cbegin() };
        std::list<QString>::const_iterator sqlCEnd{ sqlStrs.cend() };
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for (; sqlCBeg != sqlCEnd; ++sqlCBeg) {

            if (!m_flag.load(std::memory_order_consume)) {

                if (!sqlQuery.exec(*sqlCBeg)) {
                    qWarning() << sqlQuery.lastError().text();
                }

            } else {
                DSqliteHandle::ReturnCode code{this->checkDBFileExist(mountPoint)};

                if (code == DSqliteHandle::ReturnCode::Exist) {

                    if (!sqlQuery.exec(*sqlCBeg)) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                } else {

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
                                QMap<QString, QList<QString>>>(const QMap<QString, QList<QString>> &fileNameAndTagNames, const QString &mountPoint)
{

    if (!fileNameAndTagNames.isEmpty() && !mountPoint.isEmpty()) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ fileNameAndTagNames.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ fileNameAndTagNames.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteTags2) };
        std::list<QString> gettingFileNamesThroughTagName{};

        for (; cbeg != cend; ++cbeg) {
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
            QString sqlForGettingFileNames{ rangeBeg->second.arg(cbeg.key()) };
            gettingFileNamesThroughTagName.emplace_back(std::move(sqlForGettingFileNames));
        }

        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };
        std::map<QString, std::deque<QString>> filesNameAndTagsName{};

        if (!gettingFileNamesThroughTagName.empty()) {
            std::list<QString>::const_iterator sqlCBeg{ gettingFileNamesThroughTagName.cbegin() };
            std::list<QString>::const_iterator sqlCEnd{ gettingFileNamesThroughTagName.cend() };

            for (; sqlCBeg != sqlCEnd; ++sqlCBeg) {

                if (!m_flag.load(std::memory_order_consume)) {

                    if (!sqlQuery.exec(*sqlCBeg)) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                    while (sqlQuery.next()) {
                        QString fileName{ sqlQuery.value("file_name").toString() };
                        filesNameAndTagsName[fileName] = std::deque<QString> {};
                    }


                } else {

                    DSqliteHandle::ReturnCode code{this->checkDBFileExist(mountPoint)};

                    if (code == DSqliteHandle::ReturnCode::Exist) {

                        if (!sqlQuery.exec(*sqlCBeg)) {
                            qWarning() << sqlQuery.lastError().text();
                        }

                        while (sqlQuery.next()) {
                            QString fileName{ sqlQuery.value("file_name").toString() };
                            filesNameAndTagsName[fileName] = std::deque<QString> {};
                        }

                    } else {
                        return false;
                    }
                }
            }
        }

        if (!filesNameAndTagsName.empty()) {
            std::map<QString, std::deque<QString>> transcript{};
            std::map<QString, std::deque<QString>>::const_iterator filesAndTagsItrBeg{ filesNameAndTagsName.cbegin() };
            std::map<QString, std::deque<QString>>::const_iterator filesAndTagsItrEnd{ filesNameAndTagsName.cend() };

            for (; filesAndTagsItrBeg != filesAndTagsItrEnd; ++filesAndTagsItrBeg) {

                if (!m_flag.load(std::memory_order_consume)) {
                    std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
                    ++rangeBeg;
                    QString sqlForGettingTagsOfFile{ rangeBeg->second.arg(filesAndTagsItrBeg->first) };

                    if (!sqlQuery.exec(sqlForGettingTagsOfFile)) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                    while (sqlQuery.next()) {
                        QString tagName{ sqlQuery.value("tag_name").toString() };
                        transcript[filesAndTagsItrBeg->first].emplace_back(std::move(tagName));
                    }

                } else {
                    DSqliteHandle::ReturnCode code{this->checkDBFileExist(mountPoint)};

                    if (code == DSqliteHandle::ReturnCode::Exist) {
                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
                        ++rangeBeg;
                        QString sqlForGettingTagsOfFile{ rangeBeg->second.arg(filesAndTagsItrBeg->first) };

                        if (!sqlQuery.exec(sqlForGettingTagsOfFile)) {
                            qWarning() << sqlQuery.lastError().text();
                            continue;
                        }

                        while (sqlQuery.next()) {
                            QString tagName{ sqlQuery.value("tag_name").toString() };
                            transcript[filesAndTagsItrBeg->first].emplace_back(std::move(tagName));
                        }
                    }
                }
            }

            if (!transcript.empty()) {
                filesNameAndTagsName = std::move(transcript);
            }
        }


        if (!filesNameAndTagsName.empty()) {
            std::map<QString, std::deque<QString>>::const_iterator itrForUpdatingOrDeleting{ filesNameAndTagsName.cbegin() };
            std::map<QString, std::deque<QString>>::const_iterator itrForUpdatingOrDeletingEnd{ filesNameAndTagsName.cend() };
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeLast{ --(range.second) };

            for (; itrForUpdatingOrDeleting != itrForUpdatingOrDeletingEnd; ++itrForUpdatingOrDeleting) {
                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItr{ rangeLast };
                QString sqlForUpdatingOrDeleting{};
                std::size_t tagAmount{ itrForUpdatingOrDeleting->second.size() };

                if (tagAmount >= 3) {
                    sqlForUpdatingOrDeleting = sqlItr->second.arg(itrForUpdatingOrDeleting->first);
                } else if (tagAmount == 0) {
                    QString sqlForDeletingRowInFileProperty{ "DELETE FROM file_property WHERE file_property.file_name = \'%1\'" };
                    sqlForUpdatingOrDeleting = sqlForDeletingRowInFileProperty.arg(itrForUpdatingOrDeleting->first);

                } else {
                    std::size_t difference{ 3u - tagAmount };
                    std::deque<QString>::const_iterator tagNameBeg{ itrForUpdatingOrDeleting->second.cbegin() };
                    std::deque<QString>::const_iterator tagNameEnd{ itrForUpdatingOrDeleting->second.cend() };

                    sqlForUpdatingOrDeleting = sqlItr->second.arg(*tagNameBeg);
                    ++tagNameBeg;

                    for (; tagNameBeg != tagNameEnd; ++tagNameBeg) {
                        sqlForUpdatingOrDeleting = sqlForUpdatingOrDeleting.arg(*tagNameBeg);
                    }

                    for (std::size_t index = 0; index < difference; ++index) {
                        sqlForUpdatingOrDeleting = sqlForUpdatingOrDeleting.arg(QString{});
                    }
                    sqlForUpdatingOrDeleting = sqlForUpdatingOrDeleting.arg(itrForUpdatingOrDeleting->first);
                }


                if (!m_flag.load(std::memory_order_consume) && !sqlForUpdatingOrDeleting.isEmpty()) {

                    if (!sqlQuery.exec(sqlForUpdatingOrDeleting)) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                } else if (m_flag.load(std::memory_order_consume) && !sqlForUpdatingOrDeleting.isEmpty()) {
                    DSqliteHandle::ReturnCode code{this->checkDBFileExist(mountPoint)};

                    if (code == DSqliteHandle::ReturnCode::Exist) {

                        if (!sqlQuery.exec(sqlForUpdatingOrDeleting)) {
                            qWarning() << sqlQuery.lastError().text();
                        }

                    } else {
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
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags3, QList<QString>, bool>(const QList<QString> &tag_name, const QString &mountPoint)
{
    if (!tag_name.isEmpty() && mountPoint == QString{"/home"} && QFileInfo::exists("/home")) {
        QList<QString>::const_iterator c_beg{ tag_name.cbegin() };
        QList<QString>::const_iterator c_end{ tag_name.cend() };
        QSqlQuery sql_query{ *m_sqlDatabasePtr };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteTags3) };

        for (; c_beg != c_end; ++c_beg) {
            QString sql_str{ range.first->second };
            sql_str = sql_str.arg(*c_beg);

            if (!sql_query.exec(sql_str)) {
                qWarning() << sql_query.lastError().text();
                return false;
            }
        }

        return true;
    }

    return false;
}



template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeFilesName, std::map<QString, QString>>(const std::map<QString, QString> &sqlStrs, const QString &mountPoint)
{
    if (!sqlStrs.empty() && !mountPoint.isEmpty()) {
        std::map<QString, QString>::const_iterator sqlCBeg{ sqlStrs.cbegin() };
        std::map<QString, QString>::const_iterator sqlCEnd{ sqlStrs.cend() };
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        for (; sqlCBeg != sqlCEnd; ++sqlCBeg) {

            if (!m_flag.load(std::memory_order_consume)) {

                if (!sqlQuery.exec(sqlCBeg->first)) {
                    qWarning() << sqlQuery.lastError().text();
                }

                if (!sqlQuery.exec(sqlCBeg->second)) {
                    qWarning() << sqlQuery.lastError().text();
                }

            } else {
                DSqliteHandle::ReturnCode code{this->checkDBFileExist(mountPoint)};

                if (code == DSqliteHandle::ReturnCode::Exist) {

                    if (!sqlQuery.exec(sqlCBeg->first)) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                    if (!sqlQuery.exec(sqlCBeg->second)) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                } else {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

template<>
QMap<QString, QList<QString>> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeFilesName2, std::map<QString, QString>,
                                                         QMap<QString, QList<QString>>>(const std::map<QString, QString> &files, const QString &mount_point)
{
    QMap<QString, QList<QString>> file_with_tags{};

    if (!files.empty()) {
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(SqlType::ChangeFilesName2) };
        std::map<QString, QString>::const_iterator file_beg{ files.cbegin() };
        std::map<QString, QString>::const_iterator file_end{ files.cend() };
        QSqlQuery sql_query{ *m_sqlDatabasePtr };


        if (!m_flag.load(std::memory_order_consume)) {

            for (; file_beg != file_end; ++file_beg) {

                if (sql_query.exec(range.first->second.arg(file_beg->first))) {

                    while (sql_query.next()) {
                        QString tag_name{ sql_query.value("tag_name").toString() };
                        file_with_tags[file_beg->first].push_back(tag_name);
                    }
                }
            }

        } else {

            DSqliteHandle::ReturnCode code{this->checkDBFileExist(mount_point)};

            if (code == DSqliteHandle::ReturnCode::Exist) {

                for (; file_beg != file_end; ++file_beg) {

                    if (sql_query.exec(range.first->second.arg(file_beg->first))) {

                        while (sql_query.next()) {
                            QString tag_name{ sql_query.value("tag_name").toString() };
                            file_with_tags[file_beg->first].push_back(tag_name);
                        }
                    }
                }
            }
        }
    }

    return file_with_tags;
}


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeTagsName,
                                std::list<std::tuple<QString, QString, QString, QString>>>(const std::list<std::tuple<QString, QString, QString, QString>> &sqlStrs,
                                                                                           const QString &mountPoint)
{
    if (!sqlStrs.empty() && !mountPoint.isEmpty()) {
        std::list<std::tuple<QString, QString, QString, QString>>::const_iterator sqlCBeg{ sqlStrs.cbegin() };
        std::list<std::tuple<QString, QString, QString, QString>>::const_iterator sqlCEnd{ sqlStrs.cend() };
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };


        for (; sqlCBeg != sqlCEnd; ++sqlCBeg) {

            if (!m_flag.load(std::memory_order_consume)) {

                QSqlQuery sql_query(*m_sqlDatabasePtr);

                if (!sql_query.exec(std::get<0>(*sqlCBeg))) {
                    qWarning() << sql_query.lastError().text();
                }

                if (!sql_query.exec(std::get<1>(*sqlCBeg))) {
                    qWarning() << sql_query.lastError().text();
                }

                if (!sql_query.exec(std::get<2>(*sqlCBeg))) {
                    qWarning() << sql_query.lastError().text();
                }

                if (!sql_query.exec(std::get<3>(*sqlCBeg))) {
                    qWarning() << sql_query.lastError().text();
                }

            } else {
                DSqliteHandle::ReturnCode code{this->checkDBFileExist(mountPoint)};

                if (code == DSqliteHandle::ReturnCode::Exist) {

                    if (!sqlQuery.exec(std::get<0>(*sqlCBeg))) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                    if (!sqlQuery.exec(std::get<1>(*sqlCBeg))) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                    if (!sqlQuery.exec(std::get<2>(*sqlCBeg))) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                    if (!sqlQuery.exec(std::get<3>(*sqlCBeg))) {
                        qWarning() << sqlQuery.lastError().text();
                    }

                } else {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}


template<>
bool DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::ChangeTagsName2, QMap<QString, QList<QString>>, bool>(const QMap<QString, QList<QString>> &old_and_new,
                                                                                                              const QString &mountPoint)
{
    if (!old_and_new.empty() && mountPoint == QString{"/home"} && QFileInfo::exists("/home")) {
        QMap<QString, QList<QString>>::const_iterator c_beg{ old_and_new.cbegin() };
        QMap<QString, QList<QString>>::const_iterator c_end{ old_and_new.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(SqlType::ChangeTagsName2) };
        QSqlQuery sql_query{ *m_sqlDatabasePtr };

        for (; c_beg != c_end; ++c_beg) {
            QString sql_str{ range.first->second.arg(c_beg.value().first()) };
            sql_str = sql_str.arg(c_beg.key());

            if (!sql_query.exec(sql_str)) {
                qWarning() << sql_query.lastError().text();
                return false;
            }
        }

        return true;
    }

    return false;
}


template<>
QList<QString> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::GetTagsThroughFile, QString,
                                          QList<QString>>(const QString &sqlStr, const QString &mountPoint)
{
    QList<QString> tagNames{};

    if (!sqlStr.isEmpty() && !mountPoint.isEmpty()) {
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        if (!m_flag.load(std::memory_order_consume)) {

            if (!sqlQuery.exec(sqlStr)) {
                qWarning() << sqlQuery.lastError().text();
            }

            while (sqlQuery.next()) {
                QString tagName{ sqlQuery.value("tag_name").toString() };
                tagNames.push_back(tagName);
            }

        } else {

            DSqliteHandle::ReturnCode code{this->checkDBFileExist(mountPoint)};

            if (code == DSqliteHandle::ReturnCode::Exist) {

                if (!sqlQuery.exec(sqlStr)) {
                    qWarning() << sqlQuery.lastError().text();
                }

                while (sqlQuery.next()) {
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
                                          QString, QList<QString>>(const QString &sqlStr, const QString &mountPoint)
{
    QList<QString> files{};

    if (!sqlStr.isEmpty() && !mountPoint.isEmpty()) {
        QSqlQuery sqlQuery{ *m_sqlDatabasePtr };

        if (!m_flag.load(std::memory_order_consume)) {

            if (sqlQuery.exec(sqlStr)) {

                while (sqlQuery.next()) {
                    QString fileName{ sqlQuery.value("file_name").toString() };
                    files.push_back(mountPoint + fileName);
                }
            }

        } else {

            DSqliteHandle::ReturnCode code{this->checkDBFileExist(mountPoint)};

            if (code == DSqliteHandle::ReturnCode::Exist) {

                if (sqlQuery.exec(sqlStr)) {

                    while (sqlQuery.next()) {
                        QString fileName{ sqlQuery.value("file_name").toString() };
                        files.push_back(mountPoint + fileName);
                    }
                }

            }
        }
    }
    return files;
}


template<>
QMap<QString, QList<QString>> DSqliteHandle::helpExecSql<DSqliteHandle::SqlType::DeleteTags4,
                                                         QList<QString>, QMap<QString, QList<QString>>>(const QList<QString> &tag_names, const QString &mount_point)
{
    (void)mount_point;
    QMap<QString, QList<QString>> file_and_tags{};

    if (!tag_names.isEmpty()) {

        if (m_partionsOfDevices && !m_partionsOfDevices->empty()) {
            std::map<QString, std::multimap<QString, QString>>::const_iterator device_itr_beg{ m_partionsOfDevices->cbegin() };
            std::map<QString, std::multimap<QString, QString>>::const_iterator device_itr_end{ m_partionsOfDevices->cend() };
            std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteTags4) };

            for (; device_itr_beg != device_itr_end; ++device_itr_beg) {
                std::multimap<QString, QString>::const_iterator partion_itr_beg{ device_itr_beg->second.cbegin() };
                std::multimap<QString, QString>::const_iterator partion_itr_end{ device_itr_beg->second.cend() };

                for (; partion_itr_beg != partion_itr_end; ++partion_itr_beg) {
                    DSqliteHandle::ReturnCode code{ this->checkDBFileExist(partion_itr_beg->second) };

                    if (code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist) {
                        this->connectToShareSqlite(partion_itr_beg->second);

                        if (m_sqlDatabasePtr && m_sqlDatabasePtr->open()) {
                            QSqlQuery sql_query{ *m_sqlDatabasePtr };

                            for (const QString &tag_name : tag_names) {
                                QString sqlForGetFilesThroughTag{ range.first->second.arg(tag_name) };


                                if (!m_flag.load(std::memory_order_consume)) {

                                    if (sql_query.exec(sqlForGetFilesThroughTag)) {

                                        while (sql_query.next()) {
                                            QString fileName{ sql_query.value("file_name").toString() };

                                            if (!fileName.isEmpty()) {
                                                QString complete_file_name{ partion_itr_beg->second + fileName };
                                                file_and_tags[complete_file_name].push_back(tag_name);
                                            }
                                        }
                                    }

                                } else {

                                    DSqliteHandle::ReturnCode return_code{this->checkDBFileExist(partion_itr_beg->second)};

                                    if (return_code == DSqliteHandle::ReturnCode::Exist) {

                                        if (sql_query.exec(sqlForGetFilesThroughTag)) {

                                            while (sql_query.next()) {
                                                QString fileName{ sql_query.value("file_name").toString() };

                                                if (!fileName.isEmpty()) {
                                                    QString complete_file_name{ partion_itr_beg->second + fileName };
                                                    file_and_tags[complete_file_name].push_back(tag_name);
                                                }
                                            }
                                        }

                                    } else {
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    this->closeSqlDatabase();

    return file_and_tags;
}




template<>                                           ///###:<file, [tagsName]>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFiles, bool>(const QMap<QString, QList<QString>> &filesAndTags)
{
    if (!filesAndTags.isEmpty()) {
        int size{ filesAndTags.size() };
        QList<QString> mutualTags{};
        QList<QString> currentTags{ filesAndTags.cbegin().value() };

        if (size == 1) {
            mutualTags = this->execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(filesAndTags);
        }

        if (size > 1) {
            mutualTags = this->execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(filesAndTags);
        }

        QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(
                                                             DUrl::fromLocalFile(filesAndTags.cbegin().key()), m_partionsOfDevices) };

        if (unixDeviceAndMountPoint.second.isEmpty() || unixDeviceAndMountPoint.second.isNull()) {
            return false;
        }

        ///###: remove the info of mount-point.
        QMap<QString, QList<QString>> file_with_tags{};
        QMap<QString, QList<QString>>::const_iterator itr_cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator itr_cend{ filesAndTags.cend() };

        for (; itr_cbeg != itr_cend; ++itr_cbeg) {
            QString file{ itr_cbeg.key() };
            QString file_backup{ this->remove_mount_point(file, unixDeviceAndMountPoint.second) };

            file_with_tags[file_backup] = itr_cbeg.value();
        }

//#ifdef QT_DEBUG
//       qDebug()<< "mount point: " << unixDeviceAndMountPoint.second;
//       qDebug()<< "file_with_tags: " << file_with_tags;
//#endif

        DSqliteHandle::ReturnCode code{ this->checkDBFileExist(unixDeviceAndMountPoint.second) };

        ///###: when tag files through many tags.
        ///###: if these files have mutual do this.
        if (!mutualTags.isEmpty()) {

            if (code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist) {
                this->connectToShareSqlite(unixDeviceAndMountPoint.second);
                QList<QString> newTags{};
                QList<QString> existingTags{};
                QList<QString> decreased{};

                for (const QString &tagName : currentTags) {

                    if (mutualTags.contains(tagName)) {
                        existingTags.push_back(tagName);
                    } else {
                        newTags.push_back(tagName);
                    }
                }

                if (existingTags.size() < mutualTags.size()) {

                    for (const QString &tagName : mutualTags) {

                        if (!existingTags.contains(tagName)) {
                            decreased << tagName;
                        }
                    }
                }

                if (m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {
                    bool valueOfDelRedundant{ true };

                    if (!decreased.isEmpty()) {
                        QMap<QString, QList<QString>> mapForDelRedundant{};
                        QMap<QString, QList<QString>>::const_iterator cbegOfFiles{ file_with_tags.cbegin() };
                        QMap<QString, QList<QString>>::const_iterator cendOfFiles{ file_with_tags.cend() };

                        for (; cbegOfFiles != cendOfFiles; ++cbegOfFiles) {
                            mapForDelRedundant[cbegOfFiles.key()] = decreased;
                        }

                        valueOfDelRedundant = this->helpExecSql<DSqliteHandle::SqlType::TagFiles, QMap<QString, QList<QString>>,
                        bool>(mapForDelRedundant, unixDeviceAndMountPoint.second);
                    }

                    bool valueOfInsertNew{ true };

                    if (!newTags.isEmpty()) {
                        QMap<QString, QList<QString>>::const_iterator cbegOfFiles{ file_with_tags.cbegin() };
                        QMap<QString, QList<QString>>::const_iterator cendOfFiles{ file_with_tags.cend() };
                        QMap<QString, QList<QString>> mapForInsertingNew{};


                        for (; cbegOfFiles != cendOfFiles; ++cbegOfFiles) {
                            mapForInsertingNew[cbegOfFiles.key()] = newTags;
                        }

                        valueOfInsertNew = this->helpExecSql<DSqliteHandle::SqlType::TagFiles2, QMap<QString, QList<QString>>,
                        bool>(mapForInsertingNew, unixDeviceAndMountPoint.second);
                    }


                    bool valueOfUpdating{ true };
                    QList<QString> files{ file_with_tags.keys() };
                    valueOfUpdating = this->helpExecSql<DSqliteHandle::SqlType::TagFiles3, QList<QString>,
                    bool>(files, unixDeviceAndMountPoint.second);

                    if (!(valueOfDelRedundant && valueOfInsertNew && valueOfUpdating && m_sqlDatabasePtr->commit())) {
                        m_sqlDatabasePtr->rollback();
                        this->closeSqlDatabase();

                        return false;
                    }

                    this->closeSqlDatabase();

                    ///###: emit signal. which files were tagged and which files were untagged.
                    QMap<QString, QList<QString>> file_and_tags_backup{ filesAndTags };

                    if (!decreased.isEmpty()) {
                        QMap<QString, QList<QString>>::iterator itr_beg{ file_and_tags_backup.begin() };
                        QMap<QString, QList<QString>>::iterator itr_end{ file_and_tags_backup.end() };

                        for (; itr_beg != itr_end; ++itr_beg) {

                            for (const QString &tag_name : decreased) {
                                file_and_tags_backup[itr_beg.key()].removeAll(tag_name);
                            }
                        }
                    }

                    QMap<QString, QVariant> var_map{};
                    QMap<QString, QList<QString>>::const_iterator the_beg{ file_and_tags_backup.cbegin() };
                    QMap<QString, QList<QString>>::const_iterator the_end{ file_and_tags_backup.cend() };

                    for (; the_beg != the_end; ++the_beg) {
                        var_map[the_beg.key()] = QVariant{ the_beg.value() };
                    }

                    emit filesWereTagged(var_map);

                    var_map.clear();
                    the_beg = file_and_tags_backup.cbegin();
                    the_end = file_and_tags_backup.cend();

                    for (; the_beg != the_end; ++the_beg) {
                        var_map[the_beg.key()] = QVariant{ decreased };
                    }

                    emit untagFiles(var_map);
                    ///###: end of emmitting signal.

                    return true;
                }
            }

            ///###: if these files dont have mutual tag(s).
        } else {

            if (code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist) {
                this->connectToShareSqlite(unixDeviceAndMountPoint.second);

                if (m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {

                    bool valueOfInsertNew{ true };
                    valueOfInsertNew = this->helpExecSql<DSqliteHandle::SqlType::TagFiles2, QMap<QString, QList<QString>>,
                    bool>(file_with_tags, unixDeviceAndMountPoint.second);

                    bool valueOfUpdating{ true };
                    QList<QString> files{ file_with_tags.keys() };
                    valueOfUpdating = this->helpExecSql<DSqliteHandle::SqlType::TagFiles3, QList<QString>,
                    bool>(files, unixDeviceAndMountPoint.second);

                    if (!(valueOfInsertNew && valueOfUpdating && m_sqlDatabasePtr->commit())) {
                        m_sqlDatabasePtr->rollback();
                        this->closeSqlDatabase();

                        return false;
                    }

                    this->closeSqlDatabase();
                    QMap<QString, QVariant> var_map{};
                    QMap<QString, QList<QString>>::const_iterator the_beg{ filesAndTags.cbegin() };
                    QMap<QString, QList<QString>>::const_iterator the_end{ filesAndTags.cend() };

                    for (; the_beg != the_end; ++the_beg) {
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
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::TagFilesThroughColor, bool>(const QMap<QString, QList<QString>> &filesAndTags)
{
    ///##: log! but you need specify the path of file.
//    std::basic_ofstream<char> outStream{ "", std::ios_base::out | std::ios_base::app };

    if (!filesAndTags.isEmpty()) {

//        DSqliteHandle::ReturnCode return_code{ this->checkDBFileExist("/home", ".__main.db") };

//        if (return_code != DSqliteHandle::ReturnCode::Exist) {
//            return false;
//        }

        this->connectToShareSqlite("/home", ".__main.db");
        bool the_result{ true };

        if (m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {
            the_result = this->helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor3, QString, bool>(filesAndTags.cbegin().key(), "/home");
        }

        if (!(the_result && m_sqlDatabasePtr->commit())) {
            m_sqlDatabasePtr->rollback();
            this->closeSqlDatabase();

            return false;
        }

        QMap<QString, QList<QString>> mount_point_and_files{};
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        QString tag_name{ cbeg.value().first() };

        if (!(m_partionsOfDevices && m_partionsOfDevices->empty())) {
            return false;
        }

        for (; cbeg != cend; ++cbeg) {
            QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };

            if (!unixDeviceAndMountPoint.second.isEmpty()) {
                QString file_were_removed_mount_point{ this->remove_mount_point(cbeg.key(), unixDeviceAndMountPoint.second) };
                mount_point_and_files[unixDeviceAndMountPoint.second].push_back(file_were_removed_mount_point);
            }
        }

        ///###: log!
//        outStream << "LANG: " << qgetenv("LANG").toStdString()
//                 << ", LANGUAGE: " << qgetenv("LANGUAGE").toStdString() << std::endl;



        cbeg = mount_point_and_files.cbegin();
        cend = mount_point_and_files.cend();
        bool result{ true };

        for (; cbeg != cend; ++cbeg) {
            std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(SqlType::TagFilesThroughColor) };
            std::list<std::tuple<QString, QString, QString, QString, QString, QString>> sqlStrs{};
            DSqliteHandle::ReturnCode code{ this->checkDBFileExist(cbeg.key()) };

            if (code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist) {
                this->connectToShareSqlite(cbeg.key());

                if (static_cast<bool>(m_sqlDatabasePtr)) {
                    QList<QString>::const_iterator files_cbeg{ cbeg.value().cbegin() };
                    QList<QString>::const_iterator files_cend{ cbeg.value().cend() };

                    for (; files_cbeg != files_cend; ++files_cbeg) {
                        std::multimap<DSqliteHandle::SqlType, QString>::const_iterator sqlItrBeg{ range.first };
                        QString sqlForCounting{sqlItrBeg->second.arg(tag_name)};
                        sqlForCounting = sqlForCounting.arg(*files_cbeg);

                        ++sqlItrBeg;

                        QString sqlForInserting{sqlItrBeg->second.arg(tag_name)};
                        sqlForInserting = sqlForInserting.arg(*files_cbeg);

                        ++sqlItrBeg;

                        QString sqlForSelectingTagName{ sqlItrBeg->second.arg(*files_cbeg) };

                        ++sqlItrBeg;

                        QString sqlForInsertingNewRow{ sqlItrBeg->second };

                        std::tuple<QString, QString, QString, QString, QString, QString> sqlTuple{ sqlForCounting, sqlForInserting, sqlForSelectingTagName,
                                                                                                   sqlForInsertingNewRow, *files_cbeg, tag_name };
                        sqlStrs.push_back(std::move(sqlTuple));

                    }

                    if (!sqlStrs.empty()) {
                        if (m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {
                            bool value = this->helpExecSql<DSqliteHandle::SqlType::TagFilesThroughColor,
                                 std::list<std::tuple<QString, QString, QString, QString, QString, QString>>, bool>(sqlStrs, cbeg.key());

                            if (value) {

                                if (!m_sqlDatabasePtr->commit()) {
                                    m_sqlDatabasePtr->rollback();
                                    this->closeSqlDatabase();
                                    result = false;

                                    continue;
                                }

                                this->closeSqlDatabase();
                                QMap<QString, QVariant> var_map{};

                                for (const QString &file_name : cbeg.value()) {
                                    var_map[file_name] = QVariant{ QList<QString>{ tag_name } };
                                }

                                emit filesWereTagged(var_map);
                            }
                        }
                    }
                }
            }
        }

        this->closeSqlDatabase();

        return result;
    }

    this->closeSqlDatabase();

    return false;
}





template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagSamePartionFiles, bool>(const QMap<QString, QList<QString>> &filesAndTags)
{
    if (!filesAndTags.isEmpty()) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };
        DSqliteHandle::ReturnCode code{ this->checkDBFileExist(unixDeviceAndMountPoint.second) };

        if (code == DSqliteHandle::ReturnCode::Exist || code == DSqliteHandle::ReturnCode::NoExist) {

            ///###: remove mount-point of files.
            QMap<QString, QList<QString>> file_with_tags{};
            QMap<QString, QList<QString>>::const_iterator itr_cbeg{ filesAndTags.cbegin() };
            QMap<QString, QList<QString>>::const_iterator itr_cend{ filesAndTags.cend() };

            for (; itr_cbeg != itr_cend; ++itr_cbeg) {
                QString file_were_removed_mount_point{ this->remove_mount_point(itr_cbeg.key(), unixDeviceAndMountPoint.second) };
                file_with_tags[file_were_removed_mount_point] = itr_cbeg.value();
            }
            ///###: end to remove.

            this->connectToShareSqlite(unixDeviceAndMountPoint.second);

            if (static_cast<bool>(m_sqlDatabasePtr)) {
                std::pair<std::multimap<SqlType, QString>::const_iterator,
                    std::multimap<SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(SqlType::UntagSamePartionFiles) };

                if (range.first != range.second) {

                    ///###: [sql].
                    std::list<QString> sqlForDeletingRowOfTagWithFile{};
                    cbeg = file_with_tags.cbegin(); //###:!!!!!!!!
                    cend = file_with_tags.cend();//###:!!!!!!!!!

                    for (; cbeg != cend; ++cbeg) {
                        std::multimap<SqlType, QString>::const_iterator rangeCBeg{ range.first };
                        QString fileLocalName{ cbeg.key() };
                        QList<QString>::const_iterator cTagBeg{ cbeg.value().cbegin() };
                        QList<QString>::const_iterator cTagEnd{ cbeg.value().cend() };

                        for (; cTagBeg != cTagEnd; ++cTagBeg) {
                            QString deletingRowOfTagWithFile{ rangeCBeg->second.arg(fileLocalName) };
                            deletingRowOfTagWithFile = deletingRowOfTagWithFile.arg(*cTagBeg);
                            sqlForDeletingRowOfTagWithFile.push_back(deletingRowOfTagWithFile);
                        }
                    }

                    if (!sqlForDeletingRowOfTagWithFile.empty() && m_sqlDatabasePtr->open()
                            && m_sqlDatabasePtr->transaction()) {
                        bool resultOfDeleteRowInTagWithFile{ this->helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles,
                                                             std::list<QString>, bool>(sqlForDeletingRowOfTagWithFile, unixDeviceAndMountPoint.second) };
                        bool resultOfUpdateFileProperty{ false };

                        if (resultOfDeleteRowInTagWithFile) {
                            resultOfUpdateFileProperty = this->helpExecSql<DSqliteHandle::SqlType::UntagSamePartionFiles2,
                            QMap<QString, QList<QString>>, bool>(file_with_tags, unixDeviceAndMountPoint.second);
                        }

                        if (!(resultOfDeleteRowInTagWithFile && resultOfUpdateFileProperty
                                && m_sqlDatabasePtr->commit())) {
                            m_sqlDatabasePtr->rollback();
                            this->closeSqlDatabase();

                            return false;
                        }

                        this->closeSqlDatabase();

                        return true;
                    }
                }
            }

        } else {

            qWarning("A partion was unmounted just now!");
            this->closeSqlDatabase();

            return false;
        }
    }

    this->closeSqlDatabase();

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::UntagDiffPartionFiles, bool>(const QMap<QString, QList<QString>> &filesAndTags)
{

    if (!filesAndTags.isEmpty()) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };

        ///###: <mount-point path, [<files,[tag-name(s)]>]>
        std::unordered_map<QString, QMap<QString, QList<QString>>> untagFilesInSpecifyPartion{};

        ///###: classify the files through mount-point.
        for (; cbeg != cend; ++cbeg) {
            QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };

            for (const QString &tag_name : cbeg.value()) {
                untagFilesInSpecifyPartion[unixDeviceAndMountPoint.second][cbeg.key()].push_back(tag_name);
            }
        }

        if (!untagFilesInSpecifyPartion.empty()) {
            std::unordered_map<QString, QMap<QString, QList<QString>>>::const_iterator partionItrBeg{ untagFilesInSpecifyPartion.cbegin() };
            std::unordered_map<QString, QMap<QString, QList<QString>>>::const_iterator partionItrEnd{ untagFilesInSpecifyPartion.cend() };
            bool result{ true };

            for (; partionItrBeg != partionItrEnd; ++partionItrBeg) {
                bool val{ this->execSqlstr<DSqliteHandle::SqlType::UntagSamePartionFiles, bool>(partionItrBeg->second) };
                result = (val && result);
            }

            this->closeSqlDatabase();
            QMap<QString, QVariant> var_map{};
            QMap<QString, QList<QString>>::const_iterator the_beg{ filesAndTags.cbegin() };
            QMap<QString, QList<QString>>::const_iterator the_end{ filesAndTags.cend() };

            for (; the_beg != the_end; ++the_beg) {
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
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteFiles, bool>(const QMap<QString, QList<QString>> &filesAndTags)
{
    if (!filesAndTags.isEmpty()) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        std::pair<std::map<DSqliteHandle::SqlType, QString>::const_iterator,
            std::map<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteFiles) };

        ///###: <mount-point, [files]>
        std::map<QString, std::list<QString>> filesOfPartions{};

        ///###: classify files through the mount-point of every file.
        for (; cbeg != cend; ++cbeg) {
            QPair<QString, QString> unixDeviceAndMountPoint{
                DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };

            if (!unixDeviceAndMountPoint.second.isEmpty() && !unixDeviceAndMountPoint.second.isNull()) {
                QString file_were_removed_mount_point{ this->remove_mount_point(cbeg.key(), unixDeviceAndMountPoint.second) };
                filesOfPartions[unixDeviceAndMountPoint.second].push_back(file_were_removed_mount_point);
            }
        }

//#ifdef QT_DEBUG

//        for(const std::pair<QString, std::list<QString>>& item : filesOfPartions){
//            qDebug()<< item.first;

//            for(const QString& file_name : item.second){
//                qDebug()<< file_name;
//            }

//            qDebug()<< "\n";
//        }

//#endif //QT_DEBUG

        std::map<QString, std::list<QString>>::const_iterator itr_partion_and_files{ filesOfPartions.cbegin() };
        std::map<QString, std::list<QString>>::const_iterator itr_partion_and_files_end{ filesOfPartions.cend() };
        QMap<QString, QList<QString>> file_and_tags{};

        for (; itr_partion_and_files != itr_partion_and_files_end; ++itr_partion_and_files) {
            DSqliteHandle::ReturnCode code{ this->checkDBFileExist(itr_partion_and_files->first) };

            if (code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist) {
                this->connectToShareSqlite(itr_partion_and_files->first);

                if (m_sqlDatabasePtr && m_sqlDatabasePtr->open()) {
                    QMap<QString, QList<QString>> file_and_tags_partion{
                        this->helpExecSql<DSqliteHandle::SqlType::DeleteFiles2,
                        std::list<QString>, QMap<QString, QList<QString>>>(itr_partion_and_files->second, itr_partion_and_files->first)
                    };

                    if (!file_and_tags_partion.isEmpty()) {
                        QMap<QString, QList<QString>>::const_iterator itr_beg{ file_and_tags_partion.cbegin() };
                        QMap<QString, QList<QString>>::const_iterator itr_end{ file_and_tags_partion.cend() };

                        for (; itr_beg != itr_end; ++itr_beg) {
                            file_and_tags[itr_beg.key()] = itr_beg.value();
                        }
                    }
                }
            }
        }

        this->closeSqlDatabase();

        if (file_and_tags.isEmpty()) {
            return false;
        }

        QMap<QString, QVariant> result_of_emit{};
        itr_partion_and_files = filesOfPartions.cbegin();
        itr_partion_and_files_end = filesOfPartions.cend();
        bool result_of_return{ true };

        for (; itr_partion_and_files != itr_partion_and_files_end; ++itr_partion_and_files) {
            DSqliteHandle::ReturnCode code{ this->checkDBFileExist(itr_partion_and_files->first) };

            if (code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist) {
                this->connectToShareSqlite(itr_partion_and_files->first);

                if (m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {

                    bool result{ this->helpExecSql<DSqliteHandle::SqlType::DeleteFiles,
                                 std::list<QString>, bool>(itr_partion_and_files->second, itr_partion_and_files->first) };

                    if (!(result && m_sqlDatabasePtr->commit())) {
                        m_sqlDatabasePtr->rollback();
                        result_of_return = false;

                        qWarning() << "Failed to delete files. So there some dirty data in sqlite.";
                    }

                    if (result) {

                        for (const QString &file : itr_partion_and_files->second) {
                            result_of_emit[itr_partion_and_files->first + file] = QVariant{ file_and_tags[file] };
                        }

                        emit untagFiles(result_of_emit);
                    }
                }
            }
        }

        this->closeSqlDatabase();

        return result_of_return;
    }

    this->closeSqlDatabase();

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::DeleteTags, bool>(const QMap<QString, QList<QString>> &filesAndTags)
{
    if (!filesAndTags.isEmpty()) {
        this->connectToShareSqlite("/home", ".__main.db");
        bool the_result{ true };
        QList<QString> the_tags_for_deleting{ filesAndTags.keys() };

        if (m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {
            the_result = this->helpExecSql<DSqliteHandle::SqlType::DeleteTags3, QList<QString>, bool>(the_tags_for_deleting, "/home");
        }

        if (!(the_result && m_sqlDatabasePtr->commit())) {
            m_sqlDatabasePtr->rollback();
            this->closeSqlDatabase();

            return false;
        }

        ///###: the second parameter is a placeholder.
        ///###: you can transmit any string.
        ///###: do not affect function.
        QMap<QString, QList<QString>> file_and_tags{ this->helpExecSql<DSqliteHandle::SqlType::DeleteTags4,
                                                     QList<QString>, QMap<QString, QList<QString>>>(the_tags_for_deleting, QString{"placeholder_str"}) };

        std::list<QString> sqlStrs{};
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::DeleteTags) };

        for (; cbeg != cend; ++cbeg) {
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
            QString deleteRowOfTagWithFile{ rangeBeg->second.arg(cbeg.key()) };
            sqlStrs.emplace_back(std::move(deleteRowOfTagWithFile));
        }


        if (m_partionsOfDevices && !m_partionsOfDevices->empty()) {
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItr{ m_partionsOfDevices->cbegin() };
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItrEnd{ m_partionsOfDevices->cend() };

            bool result{ true };

            for (; deviceItr != deviceItrEnd; ++deviceItr) {
                std::multimap<QString, QString>::const_iterator mountPointItr{ deviceItr->second.cbegin() };
                std::multimap<QString, QString>::const_iterator mountPointItrEnd{ deviceItr->second.cend() };

                for (; mountPointItr != mountPointItrEnd; ++mountPointItr) {

                    if (!mountPointItr->second.isEmpty() && !mountPointItr->second.isNull()) {
                        DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPointItr->second) };

                        if (code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist) {
                            this->connectToShareSqlite(mountPointItr->second);
                            bool flagForDeleteInTagWithFile{ false };
                            bool flagForUpdatingFileProperty{ false };

                            if (m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {
                                flagForDeleteInTagWithFile = this->helpExecSql<DSqliteHandle::SqlType::DeleteTags,
                                std::list<QString>, bool>(sqlStrs, mountPointItr->second);

                                if (flagForDeleteInTagWithFile) {
                                    flagForUpdatingFileProperty = this->helpExecSql<DSqliteHandle::SqlType::DeleteTags2,
                                    QMap<QString, QList<QString>>, bool>(filesAndTags, mountPointItr->second);
                                }
                            }

                            if (!(flagForDeleteInTagWithFile && flagForUpdatingFileProperty && m_sqlDatabasePtr && m_sqlDatabasePtr->commit())) {
                                m_sqlDatabasePtr->rollback();
                                result = false;
                            }
                        }
                    }
                }
            }

            this->closeSqlDatabase();

            if (result) {
                emit deleteTags(QVariant{the_tags_for_deleting});

                if (!file_and_tags.isEmpty()) {
                    QMap<QString, QList<QString>>::const_iterator the_beg{ file_and_tags.cbegin() };
                    QMap<QString, QList<QString>>::const_iterator the_end{ file_and_tags.cend() };
                    QMap<QString, QVariant> tag_with_files{};

                    for (; the_beg != the_end; ++the_beg) {
                        tag_with_files[the_beg.key()] = QVariant{ the_beg.value() };
                    }

                    emit untagFiles(tag_with_files);
                }
            }

            return result;
        }
    }

    this->closeSqlDatabase();

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeFilesName, bool>(const QMap<QString, QList<QString>> &filesAndTags)
{
    if (!filesAndTags.isEmpty()) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::ChangeFilesName) };

        std::map<QString, std::map<QString, QString>> partionsAndFileNames{};

        for (; cbeg != cend; ++cbeg) {
            QPair<QString, QString> unixDeviceAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };

            if (!unixDeviceAndMountPoint.second.isEmpty() && !unixDeviceAndMountPoint.second.isNull()) {
                QString old_name_removed_mount_point{ this->remove_mount_point(cbeg.key(), unixDeviceAndMountPoint.second) };
                QString new_name_removed_mount_point{ this->remove_mount_point(cbeg.value().first(), unixDeviceAndMountPoint.second) };

                partionsAndFileNames[unixDeviceAndMountPoint.second][old_name_removed_mount_point] = new_name_removed_mount_point;
            }
        }

        QMap<QString, QMap<QString, QList<QString>>> file_with_tags_in_partion{};

        for (const std::pair<QString, std::map<QString, QString>> &partion_and_file_names : partionsAndFileNames) {
            DSqliteHandle::ReturnCode code{ this->checkDBFileExist(partion_and_file_names.first) };

            if (code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist) {
                this->connectToShareSqlite(partion_and_file_names.first);

                if (m_sqlDatabasePtr && m_sqlDatabasePtr->open()) {
                    QMap<QString, QList<QString>> file_with_tags{
                        this->helpExecSql<DSqliteHandle::SqlType::ChangeFilesName2, std::map<QString, QString>,
                        QMap<QString, QList<QString>>>(partion_and_file_names.second, partion_and_file_names.first)
                    };

                    if (!file_with_tags.isEmpty()) {
                        file_with_tags_in_partion[partion_and_file_names.first] = file_with_tags;
                    }
                }
            }
        }

        this->closeSqlDatabase();

        if (file_with_tags_in_partion.isEmpty()) {
            return false;
        }


        std::map<QString, std::map<QString, QString>> sqlForChangingFilesName{};
        std::map<QString, std::map<QString, QString>> partionsAndFileNames_backup{ partionsAndFileNames };

        if (!partionsAndFileNames.empty()) {
            std::map<QString, std::map<QString, QString>>::const_iterator partionBeg{ partionsAndFileNames.cbegin() };
            std::map<QString, std::map<QString, QString>>::const_iterator partionEnd{ partionsAndFileNames.cend() };

            for (; partionBeg != partionEnd; ++partionBeg) {

                for (const std::pair<QString, QString> &oldAndNewName : partionBeg->second) {
                    std::map<DSqliteHandle::SqlType, QString>::const_iterator rangeBeg{ range.first };
                    QString updateFileProperty{ rangeBeg->second.arg(oldAndNewName.second)};
                    updateFileProperty = updateFileProperty.arg(oldAndNewName.first);

                    QString updateTagWithFile{ (++rangeBeg)->second.arg(oldAndNewName.second) };
                    updateTagWithFile = updateTagWithFile.arg(oldAndNewName.first);
                    sqlForChangingFilesName[partionBeg->first].emplace(updateFileProperty, updateTagWithFile);
                }
            }

            if (!sqlForChangingFilesName.empty()) {
                bool result{ true };

                for (const std::pair<QString, std::map<QString, QString>> &mountPointAndSqls : sqlForChangingFilesName) {
                    DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPointAndSqls.first) };

                    if (code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist) {
                        this->connectToShareSqlite(mountPointAndSqls.first);

                        if (m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {
                            bool resultOfExecSql{ this->helpExecSql<DSqliteHandle::SqlType::ChangeFilesName,
                                                  std::map<QString, QString>, bool>(mountPointAndSqls.second, mountPointAndSqls.first) };

                            if (!(resultOfExecSql && m_sqlDatabasePtr->commit())) {
                                m_sqlDatabasePtr->rollback();
                                result = false;

                                partionsAndFileNames_backup.erase(mountPointAndSqls.first);
                                file_with_tags_in_partion.remove(mountPointAndSqls.first);
                            }
                        }
                    }
                }

                this->closeSqlDatabase();

                QMap<QString, QList<QString>> file_with_tags_new{};

                for (const std::pair<QString, std::map<QString, QString>> &mount_point_and_file_names : partionsAndFileNames_backup) {
                    std::map<QString, QString> new_and_old_names{};

                    for (const std::pair<QString, QString> &old_and_new_name : mount_point_and_file_names.second) {
                        new_and_old_names[old_and_new_name.second] = old_and_new_name.first;
                    }

                    DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mount_point_and_file_names.first) };

                    if (code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist) {
                        this->connectToShareSqlite(mount_point_and_file_names.first);

                        if (m_sqlDatabasePtr && m_sqlDatabasePtr->open()) {
                            QMap<QString, QList<QString>> file_with_tags{
                                this->helpExecSql<DSqliteHandle::SqlType::ChangeFilesName2, std::map<QString, QString>,
                                QMap<QString, QList<QString>>>(new_and_old_names, mount_point_and_file_names.first)
                            };

                            using namespace impl;
                            file_with_tags_new += file_with_tags;
                        }
                    }
                }

                QMap<QString, QList<QString>> file_with_tags_old{};
                QMap<QString, QMap<QString, QList<QString>>>::const_iterator itr_beg{ file_with_tags_in_partion.cbegin() };
                QMap<QString, QMap<QString, QList<QString>>>::const_iterator itr_end{ file_with_tags_in_partion.cend() };

                for (; itr_beg != itr_end; ++itr_beg) {
                    using namespace impl;
                    file_with_tags_old += itr_beg.value();
                }

                QMap<QString, QVariant> file_with_tags_var{};
                QMap<QString, QList<QString>>::iterator file_with_tags_beg{ file_with_tags_old.begin() };
                QMap<QString, QList<QString>>::iterator file_with_tags_end{ file_with_tags_old.end() };

                for (; file_with_tags_beg != file_with_tags_end; ++file_with_tags_beg) {
                    file_with_tags_var[file_with_tags_beg.key()] = QVariant{ file_with_tags_beg.value() };
                }

                emit untagFiles(file_with_tags_var);

                file_with_tags_var.clear();
                file_with_tags_beg = file_with_tags_new.begin();
                file_with_tags_end = file_with_tags_new.end();

                for (; file_with_tags_beg != file_with_tags_end; ++file_with_tags_beg) {
                    file_with_tags_var[file_with_tags_beg.key()] = QVariant{ file_with_tags_beg.value() };
                }

                emit filesWereTagged(file_with_tags_var);
                this->closeSqlDatabase();

                return result;
            }
        }
    }

    this->closeSqlDatabase();

    return false;
}


template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeTagsName, bool>(const QMap<QString, QList<QString>> &filesAndTags)
{
    if (!filesAndTags.isEmpty()) {
//        DSqliteHandle::ReturnCode return_code{ this->checkDBFileExist("/home", ".__main.db") };

//        if (return_code != DSqliteHandle::ReturnCode::Exist) {
//            return false;
//        }

        this->connectToShareSqlite("/home", ".__main.db");
        bool the_result{ true };

        if (m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {
            the_result = this->helpExecSql<DSqliteHandle::SqlType::ChangeTagsName2, QMap<QString, QList<QString>>, bool>(filesAndTags, "/home");
        }

        if (!(the_result && m_sqlDatabasePtr->commit())) {
            m_sqlDatabasePtr->rollback();
            return false;
        }

        QMap<QString, QList<QString>>::const_iterator tagNameBeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator tagNameCend{ filesAndTags.cend() };
        std::list<std::tuple<QString, QString, QString, QString>> sqlStrs{};
        std::pair<std::map<DSqliteHandle::SqlType, QString>::const_iterator,
            std::map<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::ChangeTagsName) };

        for (; tagNameBeg != tagNameCend; ++tagNameBeg) {
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

        if (m_partionsOfDevices && !m_partionsOfDevices->empty()) {
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItr{ m_partionsOfDevices->cbegin() };
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItrEnd{ m_partionsOfDevices->cend() };
            bool result{ true };

            for (; deviceItr != deviceItrEnd; ++deviceItr) {
                std::multimap<QString, QString>::const_iterator mountPointItr{ deviceItr->second.cbegin() };
                std::multimap<QString, QString>::const_iterator mountPointItrEnd{ deviceItr->second.cend() };

                for (; mountPointItr != mountPointItrEnd; ++mountPointItr) {

                    if (!mountPointItr->second.isEmpty() && !mountPointItr->second.isNull()) {
                        DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPointItr->second) };

                        if (code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist) {
                            this->connectToShareSqlite(mountPointItr->second);
                            bool resultOfChangeNameOfTag{ true };
                            bool flagOfTransaction{ true };

                            if (m_sqlDatabasePtr && m_sqlDatabasePtr->open()) {
                                flagOfTransaction = m_sqlDatabasePtr->transaction();

                                if (flagOfTransaction) {
                                    resultOfChangeNameOfTag = this->helpExecSql<DSqliteHandle::SqlType::ChangeTagsName,
                                    std::list<std::tuple<QString, QString,
                                    QString, QString>>, bool>(sqlStrs, mountPointItr->second);
                                }
                            }

                            if (!(resultOfChangeNameOfTag && flagOfTransaction && m_sqlDatabasePtr && m_sqlDatabasePtr->commit())) {
                                m_sqlDatabasePtr->rollback();
                                result = false;
                            }
                        }
                    }
                }
            }

            this->closeSqlDatabase();

            if (result) {
                QMap<QString, QList<QString>>::const_iterator c_beg{ filesAndTags.cbegin() };
                QMap<QString, QList<QString>>::const_iterator c_end{ filesAndTags.cend() };
                QMap<QString, QVariant> old_and_new_name{};

                for (; c_beg != c_end; ++c_beg) {
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
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(const QMap<QString, QList<QString>> &filesAndTags)
{
    QList<QString> tags{};

    if (!filesAndTags.isEmpty()) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QPair<QString, QString> partionAndMountPoint{ DSqliteHandle::getMountPointOfFile(DUrl::fromLocalFile(cbeg.key()), m_partionsOfDevices) };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::GetTagsThroughFile) };

        if (partionAndMountPoint.second.isEmpty() || partionAndMountPoint.second.isNull()) {
            return tags;
        }

        DSqliteHandle::ReturnCode code{ this->checkDBFileExist(partionAndMountPoint.second) };

        if (code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist) {
            QString file{ cbeg.key() };
            file = this->remove_mount_point(file, partionAndMountPoint.second);
            QString sqlForGetTagsThroughFile{range.first->second.arg(file)};
            this->connectToShareSqlite(partionAndMountPoint.second);

            ///###: no transaction.
            if (m_sqlDatabasePtr->open()) {
                tags = this->helpExecSql<DSqliteHandle::SqlType::GetTagsThroughFile,
                QString, QList<QString>>(sqlForGetTagsThroughFile, partionAndMountPoint.second);
            }
        }
    }

    this->closeSqlDatabase();
    QList<QString> tags_backup{};

    std::transform(tags.begin(), tags.end(), std::back_inserter(tags_backup),
    [](const QString & tag) {
        return Tag::restore_escaped_en_skim(tag);
    });

    return tags_backup;
}

template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetFilesThroughTag, QList<QString>>(const QMap<QString, QList<QString>> &filesAndTags)
{
    QList<QString> files{};

    if (!filesAndTags.isEmpty()) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::GetFilesThroughTag) };
        QString sqlForGetFilesThroughTag{ range.first->second.arg(cbeg.key()) };

        if (m_partionsOfDevices && !m_partionsOfDevices->empty()) {
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItr{ m_partionsOfDevices->cbegin() };
            std::map<QString, std::multimap<QString, QString>>::const_iterator deviceItrEnd{ m_partionsOfDevices->cend() };

            for (; deviceItr != deviceItrEnd; ++deviceItr) {
                std::multimap<QString, QString>::const_iterator mountPointItr{ deviceItr->second.cbegin() };
                std::multimap<QString, QString>::const_iterator mountPointItrEnd{ deviceItr->second.cend() };

                for (; mountPointItr != mountPointItrEnd; ++mountPointItr) {

                    if (!mountPointItr->second.isEmpty() && !mountPointItr->second.isNull()) {

                        DSqliteHandle::ReturnCode code{ this->checkDBFileExist(mountPointItr->second) };

                        if (code == DSqliteHandle::ReturnCode::NoExist || code == DSqliteHandle::ReturnCode::Exist) {
                            this->connectToShareSqlite(mountPointItr->second);

                            if (m_sqlDatabasePtr && m_sqlDatabasePtr->open()) {

                                QList<QString> filesOfPartion{ this->helpExecSql<DSqliteHandle::SqlType::GetFilesThroughTag,
                                                               QString, QList<QString>>(sqlForGetFilesThroughTag, mountPointItr->second) };

                                if (!filesOfPartion.isEmpty()) {
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
    QList<QString> files_backup{};

    std::transform(files.begin(), files.end(), std::back_inserter(files_backup),
    [](const QString & file) {
        return Tag::restore_escaped_en_skim(file);
    });

    return files_backup;
}


template<>
QList<QString> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetSameTagsOfDiffFiles, QList<QString>>(const QMap<QString, QList<QString>> &filesAndTags)
{
    QList<QString> totalTagsNames{};
    std::map<QString, std::size_t> countForTags{};

    if (!filesAndTags.isEmpty()) {
        QMap<QString, QList<QString>>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QList<QString>>::const_iterator cend{ filesAndTags.cend() };

        for (; cbeg != cend; ++cbeg) {
            QMap<QString, QList<QString>> file{};
            file.insert(cbeg.key(), cbeg.value());

            QList<QString> tagsNames{ this->execSqlstr<DSqliteHandle::SqlType::GetTagsThroughFile, QList<QString>>(file) };
            for (const QString &tagName : tagsNames) {
                ++countForTags[tagName];
            }
        }
    }

    int size{ filesAndTags.size() };

    for (const std::pair<QString, std::size_t> &pair : countForTags) {

        if (pair.second == static_cast<std::size_t>(size)) {
            totalTagsNames.push_back(pair.first);
        }
    }

//#ifdef QT_DEBUG
//    qDebug()<< totalTagsNames;
//#endif //QT_DEBUG

    QList<QString> total_tags_names_backup{};
    std::transform(totalTagsNames.begin(), totalTagsNames.end(), std::back_inserter(total_tags_names_backup),
    [](const QString & tag_name) {
        return Tag::restore_escaped_en_skim(tag_name);
    });

    return total_tags_names_backup;
}


template<>
QMap<QString, QVariant> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetAllTags, QMap<QString, QVariant>>(const QMap<QString, QList<QString>> &filesAndTags)
{
    (void)filesAndTags;
    QMap<QString, QVariant> tag_and_color{};

    if (QFileInfo::exists("/home")) {
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::GetAllTags) };
        this->connectToShareSqlite("/home", ".__main.db");

        if (m_sqlDatabasePtr && m_sqlDatabasePtr->open()) {
            QSqlQuery sql_query{ *m_sqlDatabasePtr };

            if (sql_query.exec(range.first->second)) {

                while (sql_query.next()) {
                    QString tag_name{ sql_query.value("tag_name").toString() };
                    QString tag_color{ sql_query.value("tag_color").toString() };

                    tag_name = Tag::restore_escaped_en_skim(tag_name);
//                    tag_color = Tag::restore_escaped_en_skim(tag_color); //###: color can not contain skim.

                    tag_and_color[tag_name] = QVariant{ tag_color };
                }
            }
        }
    }

    return tag_and_color;
}


template<>
QMap<QString, QVariant> DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::GetTagColor, QMap<QString, QVariant>>(const QMap<QString, QList<QString>> &fileAndTags)
{
    QMap<QString, QVariant> tag_and_color{};

    if (QFileInfo::exists("/home") && !fileAndTags.isEmpty()) {
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::GetTagColor) };
        this->connectToShareSqlite("/home", ".__main.db");

        if (m_sqlDatabasePtr && m_sqlDatabasePtr->open()) {
            QMap<QString, QList<QString>>::const_iterator c_beg{ fileAndTags.cbegin() };
            QMap<QString, QList<QString>>::const_iterator c_end{ fileAndTags.cend() };
            QString sql_str{ range.first->second };
            QSqlQuery sql_query{ *m_sqlDatabasePtr };

            for (; c_beg != c_end; ++c_beg) {

                if (sql_query.exec(sql_str.arg(c_beg.key()))) {

                    if (sql_query.next()) {
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
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::ChangeTagColor, bool>(const QMap<QString, QList<QString>> &filesAndTags)
{
    bool result{ true };
    QMap<QString, QVariant> tag_and_new_color{};

    if (QFileInfo::exists("/home") && !filesAndTags.isEmpty()) {
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
            std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::ChangeTagColor) };
        this->connectToShareSqlite("/home", ".__main.db");

        if (m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {
            QMap<QString, QList<QString>>::const_iterator c_beg{ filesAndTags.cbegin() };
            QMap<QString, QList<QString>>::const_iterator c_end{ filesAndTags.cend() };
            QSqlQuery sql_query{ *m_sqlDatabasePtr };

            for (; c_beg != c_end; ++c_beg) {
                QString sql_str{ range.first->second };
                sql_str = sql_str.arg(c_beg.value().first());
                sql_str = sql_str.arg(c_beg.key());

                if (!sql_query.exec(sql_str)) {
                    qWarning() << sql_query.lastError().text();
                    result = false;

                    break;
                }

                tag_and_new_color[c_beg.key()] = QVariant{c_beg.value().first()};
            }

            if (!(result && m_sqlDatabasePtr->commit())) {
                m_sqlDatabasePtr->rollback();
            }
        }
    }

    if (result) {
        emit changeTagColor(tag_and_new_color);
    }

    return result;
}

template<>
bool DSqliteHandle::execSqlstr<DSqliteHandle::SqlType::BeforeTagFiles, bool>(const QMap<QString, QList<QString>> &filesAndTags)
{
    if (QFileInfo::exists("/home") && !filesAndTags.isEmpty()) {
        std::pair<std::multimap<DSqliteHandle::SqlType, QString>::const_iterator,
                  std::multimap<DSqliteHandle::SqlType, QString>::const_iterator> range{ SqlTypeWithStrs.equal_range(DSqliteHandle::SqlType::BeforeTagFiles) };
        this->connectToShareSqlite("/home", ".__main.db");

        if (m_sqlDatabasePtr && m_sqlDatabasePtr->open() && m_sqlDatabasePtr->transaction()) {

            QMap<QString, QList<QString>>::const_iterator c_beg{ filesAndTags.cbegin() };
            QMap<QString, QList<QString>>::const_iterator c_end{ filesAndTags.cend() };
            QSqlQuery sql_query{ *m_sqlDatabasePtr };
            QString sql_counting{ range.first->second };
            QString sql_inserting{ (++(range.first))->second };

            for (; c_beg != c_end; ++c_beg) {
                QString counting{ sql_counting.arg(c_beg.key()) };

                if (sql_query.exec(counting)) {

                    if (sql_query.next()) {
                        int number{ sql_query.value("counter").toInt() };

                        if (number == 0) {
                            sql_query.clear();
                            QString inserting{ sql_inserting.arg(c_beg.key()) };
                            inserting = inserting.arg(c_beg.value().first());

                            if (!sql_query.exec(inserting)) {
                                qWarning() << sql_query.lastError().text();
                                m_sqlDatabasePtr->rollback();

                                this->closeSqlDatabase();

                                return false;
                            }

                            m_newAddedTags.push_back(c_beg.key());
                        }
                    }
                }
            }

            if (!m_sqlDatabasePtr->commit()) {
                m_sqlDatabasePtr->rollback();
            }

            this->closeSqlDatabase();

            ///###: emit signal when new tag was added.
            if (!m_newAddedTags.isEmpty()) {
                emit addNewTags(QVariant{m_newAddedTags});
                m_newAddedTags.clear();
            }

            return true;
        }
    }

    this->closeSqlDatabase();

    return false;
}
