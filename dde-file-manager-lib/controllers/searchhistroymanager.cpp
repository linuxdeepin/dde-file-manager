#include "searchhistroymanager.h"

#include "models/searchhistory.h"
#include "shutil/fileutils.h"

#include <QList>
#include <QJsonObject>
#include <QFile>
#include <stdlib.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QDateTime>
#include <QDir>

SearchHistroyManager::SearchHistroyManager(QObject *parent)
    : QObject(parent)
    , BaseManager()
{
    m_stringList.clear();
    load();
}

SearchHistroyManager::~SearchHistroyManager()
{

}

void SearchHistroyManager::load()
{
    //Migration for old config files, and rmove that codes for further
    FileUtils::migrateConfigFileFromCache("searchhistory");

    //TODO: check permission and existence of the path
    QString filePath = getSearchHistroyCachePath();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Couldn't open search data file!";
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
    loadJson(jsonDoc.object());
    QStringList list = toStringList();
}

void SearchHistroyManager::save()
{
    //TODO: check permission and existence of the path
    QString filePath = getSearchHistroyCachePath();
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Couldn't open search data file!";
        return;
    }
    QJsonObject object;
    writeJson(object);
    QJsonDocument jsonDoc(object);
    file.write(jsonDoc.toJson());
}

QStringList SearchHistroyManager::toStringList()
{
    if(!m_stringList.isEmpty())
        return m_stringList;
    m_stringList.clear();
    for(int i = 0; i < m_historyList.size(); i++)
    {
        m_stringList.append(m_historyList.at(i)->getKeyword());
    }
    return m_stringList;
}

QString SearchHistroyManager::getSearchHistroyCachePath()
{
    return getConfigPath("searchhistory");
}

void SearchHistroyManager::loadJson(const QJsonObject &json)
{
    QJsonArray jsonArray = json["searchData"].toArray();
    for(int i = 0; i < jsonArray.size(); i++)
    {
        QJsonObject object = jsonArray[i].toObject();
        QString time = object["t"].toString();
        QString keyword = object["k"].toString();
        m_historyList.append(new SearchHistory(QDateTime::fromString(time), keyword));
    }
}

void SearchHistroyManager::writeJson(QJsonObject &json)
{
    QJsonArray localArray;
    for(int i = 0; i < m_historyList.size(); i++)
    {
        QJsonObject object;
        object["k"] = m_historyList.at(i)->getKeyword();
        object["t"] = m_historyList.at(i)->getDateTime().toString();
        localArray.append(object);
    }
    json["searchData"] = localArray;
}

void SearchHistroyManager::writeIntoSearchHistory(QString keyword)
{
    SearchHistory * history = new SearchHistory(QDateTime::currentDateTime(), keyword, this);
    m_historyList.append(history);
    save();
}

