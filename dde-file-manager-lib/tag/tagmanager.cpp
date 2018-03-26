

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



std::once_flag TagManager::onceFlag{};


std::multimap<TagManager::SqlType, QString> SqlStr{
                                                        { TagManager::SqlType::GetAllTags, "SELECT * FROM tag_property" },
                                                        { TagManager::SqlType::GetFilesThroughTag, "SELECT COUNT(tag_property.tag_name) AS counting FROM tag_property "
                                                                                                   "WHERE tag_property.tag_name = \'%1\'" },
                                                        { TagManager::SqlType::MakeFilesTags, "SELECT COUNT(tag_property.tag_name) AS counting FROM tag_property "
                                                                                                                                               "WHERE tag_property.tag_name = \'%1\'"},
                                                        { TagManager::SqlType::MakeFilesTags, "INSERT INTO tag_property (tag_name, tag_color)  "
                                                                                                                "VALUES (\'%1\', \'%2\')" },
                                                        { TagManager::SqlType::MakeFilesTags, "SELECT tag_property.tag_name FROM tag_property" },

                                                        { TagManager::SqlType::MakeFilesTagThroughColor, "SELECT COUNT(tag_property.tag_name) AS counting FROM tag_property "
                                                                                                                                    "WHERE tag_property.tag_name = \'%1\'" },
                                                        { TagManager::SqlType::MakeFilesTagThroughColor, "INSERT INTO tag_property (tag_name, tag_color) VALUES (\'%1\', \'%2\')" }
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


static const QMap<QString, QString> ColorsWithNames{
                                                        { "#ffa503", "Orange"},
                                                        { "#ff1c49", "Red"},
                                                        { "#9023fc", "Purple"},
                                                        { "#3468ff", "Navy-blue"},
                                                        { "#00b5ff", "Azure"},
                                                        { "#58df0a", "Grass-green"},
                                                        { "#fef144", "Yellow"} ,
                                                        { "#cccccc", "Gray" }
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
    impl::shared_mutex<QReadWriteLock> sharedLck{ mutex, impl::shared_mutex<QReadWriteLock>::Options::Read };
    QMap<QString, QString> tagNameAndColor{};

    if(sqlDataBase.open()){
        QSqlQuery sqlQuery{ sqlDataBase };
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
    impl::shared_mutex<QReadWriteLock> sharedLck{ mutex, impl::shared_mutex<QReadWriteLock>::Options::Read};
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
    impl::shared_mutex<QReadWriteLock> sharedLck{ mutex, impl::shared_mutex<QReadWriteLock>::Options::Read};
    QMap<QString, QString> tagNameAndColor{};

    if(!tags.isEmpty()){
      QString sqlStr{ "SELECT * FROM tag_property WHERE tag_property.tag_name = \'%1\'" };

      if(sqlDataBase.open()){
          QSqlQuery sqlQuery{ sqlDataBase };

          for(const QString& tagName : tags){
              QString sql{ sqlStr };
              sql = sql.arg(tagName);

              if(!sqlQuery.exec(sql)){
                  qWarning(sqlQuery.lastError().text().toStdString().c_str());

              }else{

                  if(sqlQuery.next()){
                      QString tagColor{ sqlQuery.value("tag_color").toString() };

                      if(!tagColor.isEmpty()){
                          tagNameAndColor[tagName] = tagColor;

                      }else{
                          qWarning()<< "Can not get the color of specify tag: "<< tagName;
                      }
                  }
              }
          }
      }
    }

    return tagNameAndColor;
}

QList<QString> TagManager::getFilesThroughTag(const QString& tagName)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ mutex, impl::shared_mutex<QReadWriteLock>::Options::Read};
    QList<QString> filesPath{};

    if(sqlDataBase.open()){
        QSqlQuery sqlQuery{ sqlDataBase };
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

    impl::shared_mutex<QReadWriteLock> sharedLck{ mutex,impl::shared_mutex<QReadWriteLock>::Options::Write};
    bool resultValue{ false };

    if(sqlDataBase.open() && sqlDataBase.transaction()){
        QSqlQuery sqlQuery{ sqlDataBase };
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

                        std::vector<QString>::const_iterator cbeg{ ColorName.cbegin() };
                        std::vector<QString>::const_iterator cend{ ColorName.cend() };
                        std::vector<QString>::const_iterator pos{ std::find(cbeg, cend, tagName) };
                        QString sqlForInsertingTag{ itrForInsertTag->second.arg(tagName) };

                        if(pos == cend){
                            sqlForInsertingTag = sqlForInsertingTag.arg(randomColor());

                        }else{
                            sqlForInsertingTag = sqlForInsertingTag.arg(*pos);
                        }

                        if(!sqlQuery.exec(sqlForInsertingTag)){
                            flag = true;
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());
                        }
                    }
                }
            }

            if(!flag && sqlDataBase.commit()){
                QMap<QString, QVariant> filesAndTags{};

                for(const DUrl& url : files){
                    filesAndTags[url.toString()] = QVariant{tags};
                }

                QVariant var{ TagManagerDaemonController::instance()->disposeClientData(filesAndTags,
                                                                                TagManager::getCurrentUserName(), Tag::ActionType::MakeFilesTags) };
                resultValue = var.toBool();
            }else{
                sqlDataBase.rollback();
            }
        }
    }

    return resultValue;
}

bool TagManager::changeTagColor(const QString& oldColorName, const QString& newColorName)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ mutex, impl::shared_mutex<QReadWriteLock>::Options::Write};


    if(!oldColorName.isEmpty() && !newColorName.isEmpty()){

        if(sqlDataBase.open() && sqlDataBase.transaction()){
            QSqlQuery sqlQuery{ sqlDataBase };
            QString sqlStr{ "UPDATE tag_property SET tag_color = \'%1\' WHERE tag_property.tag_color = \'%2\'" };
            sqlStr = sqlStr.arg(newColorName);
            sqlStr = sqlStr.arg(oldColorName);

            if(!sqlQuery.exec(sqlStr)){
                qWarning(sqlQuery.lastError().text().toStdString().c_str());
            }

            if(!sqlDataBase.commit()){
                sqlDataBase.rollback();

                return false;
            }
            return true;
        }
    }
    return false;
}

bool TagManager::remveTagsOfFiles(const QList<QString>& tags, const QList<DUrl>& files)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ mutex, impl::shared_mutex<QReadWriteLock>::Options::Write };
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
    impl::shared_mutex<QReadWriteLock> sharedLck{ mutex, impl::shared_mutex<QReadWriteLock>::Options::Write };

    if(!tags.isEmpty()){

        if(sqlDataBase.open() && sqlDataBase.transaction()){
            bool value{ false };
            QString sqlStr{ "DELETE FROM tag_property WHERE tag_property.tag_name = \'%1\'" };

            for(const QString& tag : tags){
                QSqlQuery sqlQuery{ sqlDataBase };
                sqlStr = sqlStr.arg(tag);

                if(!sqlQuery.exec(sqlStr)){
                    value = true;
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                sqlQuery.clear();
            }

            if(!(!value && sqlDataBase.commit())){
                sqlDataBase.rollback();
                return false;
            }
        }

        QMap<QString, QVariant> tagsAsKeys{};

        for(const QString& tag : tags){
            tagsAsKeys[tag] = QVariant{ QList<QString>{} };
        }

        QVariant var{ TagManagerDaemonController::instance()->disposeClientData(tagsAsKeys,
                                                  TagManager::getCurrentUserName(), Tag::ActionType::DeleteTags) };

        return var.toBool();
    }

    return false;
}

bool TagManager::deleteFiles(const QList<DUrl>& fileList)
{
    if(!fileList.isEmpty()){
        QList<QString> urlList{};

        for(const DUrl& localFile : fileList){
            QString localFileStr{ localFile.toLocalFile() };
            urlList.push_back(localFileStr);
        }

        if(!urlList.isEmpty()){
            bool value{ TagManager::instance()->deleteFiles(urlList) };
            return value;
        }
    }

    return false;
}

bool TagManager::deleteFiles(const QList<QString>& fileList)
{
    if(!fileList.isEmpty()){
        QMap<QString, QVariant> filesForDeleting{};

        for(const QString& file : fileList){
            filesForDeleting[file] = QVariant{ QList<QString>{} };
        }

        if(!filesForDeleting.isEmpty()){
            QVariant var{ TagManagerDaemonController::instance()->disposeClientData(filesForDeleting,
                                                      TagManager::getCurrentUserName(), Tag::ActionType::DeleteFiles) };
            return var.toBool();
        }
    }

    return false;
}

bool TagManager::changeTagName(const QPair<QString, QString>& oldAndNewName)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ mutex, impl::shared_mutex<QReadWriteLock>::Options::Write };

    if(!oldAndNewName.first.isEmpty() && !oldAndNewName.second.isEmpty()){
        bool flagForUpdatingMainDB{ false };
        QString sqlStr{ "UPDATE tag_property SET tag_name = \'%1\' WHERE tag_property.tag_name = \'%2\'" };

        if(sqlDataBase.open() && TagManager::sqlDataBase.transaction()){

            QSqlQuery sqlQuery{ sqlDataBase };
            sqlStr = sqlStr.arg(oldAndNewName.second);
            sqlStr = sqlStr.arg(oldAndNewName.first);

            if(!sqlQuery.exec(sqlStr)){
                flagForUpdatingMainDB = true;
                qWarning(sqlQuery.lastError().text().toStdString().c_str());
            }

            if(!(!flagForUpdatingMainDB && sqlDataBase.commit())){
                flagForUpdatingMainDB = true;
                sqlDataBase.rollback();
            }

            QVariant var{};
            QMap<QString, QVariant> oldAndNew{ {oldAndNewName.first, QVariant{ QList<QString>{ oldAndNewName.second } }} };

            if(!flagForUpdatingMainDB){
                var = TagManagerDaemonController::instance()->disposeClientData(oldAndNew,
                                                              TagManager::getCurrentUserName(), Tag::ActionType::ChangeTagName);
            }

            if(!flagForUpdatingMainDB && var.toBool()){
                return true;
            }
        }
    }
    return false;
}

bool TagManager::makeFilesTagThroughColor(const QString& color, const QList<DUrl>& files)
{
    impl::shared_mutex<QReadWriteLock> sharedLck{ mutex, impl::shared_mutex<QReadWriteLock>::Options::Write };

    if(!color.isEmpty() && !files.isEmpty()){
        QString colorName{ ColorsWithNames[color] };

        if(!colorName.isEmpty()){
            std::pair<std::multimap<TagManager::SqlType, QString>::const_iterator,
                      std::multimap<TagManager::SqlType, QString>::const_iterator> range{ SqlStr.equal_range(TagManager::SqlType::MakeFilesTagThroughColor) };
            QString sqlForCounting{ range.first->second.arg(colorName) };

            if(sqlDataBase.open() && sqlDataBase.transaction()){
                QSqlQuery sqlQuery{ sqlDataBase };

                if(!sqlQuery.exec(sqlForCounting)){
                    qWarning(sqlQuery.lastError().text().toStdString().c_str());
                }

                if(sqlQuery.next()){
                    int counting{ sqlQuery.value("counting").toInt() };

                    if(counting == 0){
                        std::multimap<TagManager::SqlType, QString>::const_iterator sqlItrBeg{ range.first };
                        ++sqlItrBeg;

                        QString sqlForInserting{ sqlItrBeg->second.arg(colorName) };
                        sqlForInserting = sqlForInserting.arg(colorName);

                        sqlQuery.clear();

                        if(!sqlQuery.exec(sqlForInserting)){
                            qWarning(sqlQuery.lastError().text().toStdString().c_str());

                            sqlDataBase.rollback();
                            return false;
                        }
                    }

                    QMap<QString, QVariant> filesAndColorName{};
                    QVariant dbusResult{};

                    for(const DUrl& file : files){
                        filesAndColorName[file.toString()] = QVariant{ QList<QString>{colorName} };
                    }


                    dbusResult =  TagManagerDaemonController::instance()->disposeClientData(filesAndColorName,
                                                                                            TagManager::getCurrentUserName(),
                                                                                            Tag::ActionType::MakeFilesTagThroughColor);
                    if(dbusResult.toBool() && sqlDataBase.commit()){
                        return true;
                    }
                    if(!dbusResult.toBool()){
                        sqlDataBase.rollback();
                        return false;
                    }
                }
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
