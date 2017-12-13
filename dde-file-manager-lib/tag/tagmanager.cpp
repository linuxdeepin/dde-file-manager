

#include <thread>
#include <chrono>
#include <random>


#include "tagmanager.h"
#include "tag/tagutil.h"
#include "shutil/dsqlitehandle.h"
#include "controllers/appcontroller.h"
#include "controllers/tagmanagerdaemoncontroller.h"

#include <QMap>
#include <QDebug>
#include <QVariant>


QReadWriteLock TagManager::mutex{};
QSqlDatabase TagManager::sqlDataBase{};
std::once_flag TagManager::onceFlag{};
std::atomic<int> TagManager::counter{ 0 };

std::multimap<TagManager::SqlType, QString> SqlStr{
                                                        { TagManager::SqlType::GetAllTags, "SELECT * FROM tag_property" },
                                                        { TagManager::SqlType::GetFilesThroughTag, "SELECT COUNT(tag_property.tag_name) AS counting FROM tag_property "
                                                                                                   "WHERE tag_property.tag_name = \'%1\'" },
                                                        { TagManager::SqlType::MakeFilesTags, "SELECT COUNT(tag_property.tag_name) AS counting FROM tag_property "
                                                                                                                                               "WHERE tag_property.tag_name = \'%1\'"},
                                                        { TagManager::SqlType::MakeFilesTags, "INSERT INTO tag_property (tag_name, tag_color)  "
                                                                                                                "VALUES (\'%1\', \'%2\')" },
                                                        { TagManager::SqlType::MakeFilesTags, "SELECT tag_property.tag_name FROM tag_property" }
                                                  };


std::vector<QString> ColorName{
                                "Orange",
                                "Red",
                                "Purple",
                                "Navy-blue",
                                "Azure",
                                "Grass-green",
                                "Yellow",
                                "Gray"
                              };


static QString randomColor() noexcept
{
    std::random_device device{};

    // Choose a random mean between 0 and 6
    std::default_random_engine engine(device());
    std::uniform_int_distribution<int> uniform_dist(0, 6);
    return  ColorName[uniform_dist(engine)];
}


QMap<QString, QString> TagManager::getAllTags()
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ TagManager::mutex,
                                                  impl::shared_mutex<QReadWriteLock>::Options::Read };
    QMap<QString, QString> tagNameAndColor{};

    if(TagManager::sqlDataBase.open()){
        QSqlQuery sqlQuery{ TagManager::sqlDataBase };
        std::pair<std::multimap<TagManager::SqlType, QString>::const_iterator,
                  std::multimap<TagManager::SqlType, QString>::const_iterator> range{ SqlStr.equal_range(TagManager::SqlType::GetAllTags) };

        if(range.first != SqlStr.cend()){

            if(sqlQuery.exec(range.first->second)){
                qWarning(sqlQuery.lastError().text().toStdString().c_str());
            }

            while(sqlQuery.next()){
                QString tagName{ sqlQuery.value("tag_name").toString() };
                QString tagColor{ sqlQuery.value("tag_color").toString() };
                tagNameAndColor[tagName] = tagColor;
            }
        }
    }

    return tagNameAndColor;
}

QList<QString> TagManager::getSameTagsOfDiffFiles(const QList<DUrl>& files)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ TagManager::mutex, impl::shared_mutex<QReadWriteLock>::Options::Read};
    QMap<QString, QVariant> varMap{};

    for(const DUrl& url : files){
        varMap[url.toString()] = QVariant{ QList<QString>{} };
    }

    QVariant var{ TagManagerDaemonController::instance()->disposeClientData(varMap,
                                              TagManager::getCurrentUserName(), Tag::ActionType::GetTagsThroughFile) };
    return var.toStringList();
}

QMap<QString, QString> TagManager::getTagColor(const QList<QString>& tags)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ TagManager::mutex, impl::shared_mutex<QReadWriteLock>::Options::Read};
    QMap<QString, QString> tagNameAndColor{};

    if(!tags.isEmpty()){
      QString sqlStr{ "SELECT * FROM tag_property WHERE tag_property.tag_name = \'%1\'" };

      if(TagManager::sqlDataBase.open()){
          QSqlQuery sqlQuery{ TagManager::sqlDataBase };

          for(const QString& tagName : tags){
              QString sql{ sqlStr };
              sql = sql.arg(tagName);

              if(!sqlQuery.exec(sql)){
                  qWarning(sqlQuery.lastError().text().toStdString().c_str());

              }else{

                  if(sqlQuery.next()){
                      QString tagColor{ sqlQuery.value("tag_color").toString() };
                      tagNameAndColor[tagName] = tagColor;
                  }
              }
          }
      }
    }

    return tagNameAndColor;
}

QList<QString> TagManager::getFilesThroughTag(const QString& tagName)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ TagManager::mutex,
                                                  impl::shared_mutex<QReadWriteLock>::Options::Read};
    QList<QString> filesPath{};

    if(TagManager::sqlDataBase.open()){
        QSqlQuery sqlQuery{ TagManager::sqlDataBase };
        std::pair<std::multimap<TagManager::SqlType, QString>::const_iterator,
                  std::multimap<TagManager::SqlType, QString>::const_iterator> range{ SqlStr.equal_range(TagManager::SqlType::GetFilesThroughTag) };

        if(range.first != SqlStr.cend()){
            QString sqlForQueryTag{ range.first->second.arg(tagName) };

            if(sqlQuery.exec(sqlForQueryTag)){
                qWarning(sqlQuery.lastError().text().toStdString().c_str());
            }

            if(sqlQuery.next()){
                int amount{ sqlQuery.value(0).toInt() };

                if(amount > 0){
                    QMap<QString, QVariant> varMap{ {tagName, QVariant{QList<QString>{}}} };
                    QVariant var{ TagManagerDaemonController::instance()->disposeClientData(varMap,
                                                                                            TagManager::getCurrentUserName(), Tag::ActionType::GetFilesThroughTag) };
                    filesPath = var.toStringList();
                }
            }
        }
    }

    return filesPath;
}

bool TagManager::makeFilesTags(const QList<QString>& tags, const QList<DUrl>& files)
{

    impl::shared_mutex<QReadWriteLock> sharedLck{ TagManager::mutex,
                                                  impl::shared_mutex<QReadWriteLock>::Options::Write};
    bool resultValue{ false };

    if(TagManager::sqlDataBase.open() && TagManager::sqlDataBase.transaction()){
        QSqlQuery sqlQuery{ TagManager::sqlDataBase };
        std::pair<std::multimap<TagManager::SqlType, QString>::const_iterator,
                  std::multimap<TagManager::SqlType, QString>::const_iterator> range{ SqlStr.equal_range(TagManager::SqlType::MakeFilesTags) };

        if(range.first != SqlStr.cend()){
            bool flag{ false };

            for(const QString& tagName : tags){
                QString sqlForCountingTag{ range.first->second.arg(tagName) };

                if(!sqlQuery.exec(sqlForCountingTag)){
                    flag = true;
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                if(sqlQuery.next()){
                    int amount{ sqlQuery.value("counting").toInt() };

                    if(amount > 0){
                        continue;

                    }else{
                        std::multimap<TagManager::SqlType, QString>::const_iterator itrForInsertTag{ range.first };
                        ++itrForInsertTag;
                        QString sqlForInsertingTag{ itrForInsertTag->second.arg(tagName) };
                        sqlForInsertingTag = sqlForInsertingTag.arg(randomColor());

                        if(!sqlQuery.exec(sqlForInsertingTag)){
                            flag = true;
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        }
                    }
                }
            }

            if(!flag && TagManager::sqlDataBase.commit()){
                QMap<QString, QVariant> filesAndTags{};

                for(const DUrl& url : files){
                    filesAndTags[url.toString()] = QVariant{tags};
                }

                QVariant var{ TagManagerDaemonController::instance()->disposeClientData(filesAndTags,
                                                                                TagManager::getCurrentUserName(), Tag::ActionType::MakeFilesTags) };
                resultValue = var.toBool();

            }else{
                TagManager::sqlDataBase.rollback();
            }
        }
    }

    return resultValue;
}

bool TagManager::changeTagColor(const QString& oldColorName, const QString& newColorName)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ TagManager::mutex,
                                                  impl::shared_mutex<QReadWriteLock>::Options::Write};


    if(!oldColorName.isEmpty() && !newColorName.isEmpty()){

        if(TagManager::sqlDataBase.open() && TagManager::sqlDataBase.transaction()){
            QSqlQuery sqlQuery{ TagManager::sqlDataBase };
            QString sqlStr{ "UPDATE tag_property SET tag_color = \'%1\' WHERE tag_property.tag_color = \'%2\'" };
            sqlStr = sqlStr.arg(newColorName);
            sqlStr = sqlStr.arg(oldColorName);

            qDebug()<< sqlStr;

            if(!sqlQuery.exec(sqlStr)){
                qWarning(sqlQuery.lastError().text().toStdString().c_str());
            }

            if(!TagManager::sqlDataBase.commit()){
                TagManager::sqlDataBase.rollback();

                return false;
            }

            return true;
        }
    }

    return false;
}

bool TagManager::remveTagsOfFiles(const QList<QString>& tags, const QList<DUrl>& files)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ TagManager::mutex,
                                                  impl::shared_mutex<QReadWriteLock>::Options::Write };
    bool value{ false };

    if(!tags.isEmpty() && !files.isEmpty()){
        QMap<QString, QVariant> filesAndTags{};

        for(const DUrl& url : files){
            filesAndTags[url.toString()] = QVariant{tags};
        }

        QVariant var{ TagManagerDaemonController::instance()->disposeClientData(filesAndTags,
                                                                                TagManager::getCurrentUserName(), Tag::ActionType::RemoveTagsOfFiles) };

        value = var.toBool();
    }

    return value;
}

bool TagManager::deleteTags(const QList<QString>& tags)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ TagManager::mutex,
                                                  impl::shared_mutex<QReadWriteLock>::Options::Write };

    if(!tags.isEmpty()){

        if(TagManager::sqlDataBase.open() && TagManager::sqlDataBase.transaction()){
            bool value{ false };
            QString sqlStr{ "DELETE FROM tag_property WHERE tag_property.tag_name = \'%1\'" };

            for(const QString& tag : tags){
                QSqlQuery sqlQuery{ TagManager::sqlDataBase };
                sqlStr = sqlStr.arg(tag);

                if(!sqlQuery.exec(sqlStr)){
                    value = true;
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                sqlQuery.clear();
            }

            if(!(!value && TagManager::sqlDataBase.commit())){
                TagManager::sqlDataBase.rollback();
                return false;
            }
        }

        QMap<QString, QVariant> tagsAsKeys{};

        for(const QString& tag : tags){
            tagsAsKeys[tag] = QVariant{ QList<QString>{} };
        }

        QVariant var{ TagManagerDaemonController::instance()->disposeClientData(tagsAsKeys, TagManager::getCurrentUserName(), Tag::ActionType::DeleteTags) };

        return var.toBool();
    }

    return false;
}

bool TagManager::changeTagName(const QPair<QString, QString>& oldAndNewName)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ TagManager::mutex,
                                                  impl::shared_mutex<QReadWriteLock>::Options::Write };

    if(!oldAndNewName.first.isEmpty() && !oldAndNewName.second.isEmpty()){
        bool flagForUpdatingMainDB{ false };
        QString sqlStr{ "UPDATE tag_property SET tag_name = \'%1\' WHERE tag_property.tag_name = \'%2\'" };

        if(TagManager::sqlDataBase.open() /*&& TagManager::sqlDataBase.transaction()*/){
            qDebug()<< TagManager::sqlDataBase.transaction();

            QSqlQuery sqlQuery{ TagManager::sqlDataBase };
            sqlStr = sqlStr.arg(oldAndNewName.second);
            sqlStr = sqlStr.arg(oldAndNewName.first);

            if(!sqlQuery.exec(sqlStr)){
                flagForUpdatingMainDB = true;
                qWarning(sqlQuery.lastError().text().toStdString().c_str());
            }

            if(!(!flagForUpdatingMainDB && TagManager::sqlDataBase.commit())){
                flagForUpdatingMainDB = true;
                TagManager::sqlDataBase.rollback();
            }

            QVariant var{};
            QMap<QString, QVariant> oldAndNew{ {oldAndNewName.first, QVariant{oldAndNewName.second}} };

            if(!flagForUpdatingMainDB){
                qDebug()<< oldAndNew;
                var = TagManagerDaemonController::instance()->disposeClientData(oldAndNew,
                                                              TagManager::getCurrentUserName(), Tag::ActionType::ChangeTagName);
                qDebug()<< var;
            }

            if(!flagForUpdatingMainDB && var.toBool()){
                return true;
            }
        }
    }

    return false;
}

QString TagManager::getMainDBLocation()
{
    passwd* pwd = getpwuid(getuid());
    QString mainDBLocation{ QString{"/home/"} + QString::fromStdString(pwd->pw_name)+
                            QString{"/.config/deepin/dde-file-manager/"} + QString{"main.db" } };

    return mainDBLocation;
}
