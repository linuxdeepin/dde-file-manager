#include "fmstatemanager.h"
#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../shutil/standardpath.h"
#include "utils/qobjecthelper.h"

QMap<DUrl, int> FMStateManager::SortStates;


FMStateManager::FMStateManager(QObject *parent)
    : QObject(parent)
    , BaseManager()
{
    m_fmState = new FMState(this);
    initConnect();
}

FMStateManager::~FMStateManager()
{

}

void FMStateManager::initConnect()
{

}

QString FMStateManager::cacheFilePath()
{
    return QString("%1/%2").arg(StandardPath::getCachePath(), "FMState.json");
}

QString FMStateManager::sortCacheFilePath()
{
    return QString("%1/%2").arg(StandardPath::getCachePath(), "sort.json");
}

void FMStateManager::loadCache()
{
    QString cache = readCacheFromFile(cacheFilePath());
    if (!cache.isEmpty()){
        QObjectHelper::json2qobject(cache, m_fmState);
    }
    loadSortCache();
}


void FMStateManager::saveCache()
{
    QString content = QObjectHelper::qobject2json(m_fmState);
    writeCacheToFile(cacheFilePath(), content);
}

void FMStateManager::loadSortCache()
{
    QString cache = readCacheFromFile(sortCacheFilePath());
    if (!cache.isEmpty()){
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(cache.toLocal8Bit(),&error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject obj = doc.object();
            foreach (QString key, obj.keys()) {
                FMStateManager::SortStates.insert(DUrl(key), obj.value(key).toInt());
            }
        }else{
            qDebug() << "load cache file: " << sortCacheFilePath() << error.errorString();
        }
    }

    qDebug() << FMStateManager::SortStates;
}

void FMStateManager::saveSortCache()
{
    QVariantMap sortCache;
    foreach (const DUrl& url, FMStateManager::SortStates.keys()) {
        sortCache.insert(url.toString(), FMStateManager::SortStates.value(url));
    }

    QJsonDocument doc(QJsonObject::fromVariantMap(sortCache));
    writeCacheToFile(sortCacheFilePath(), doc.toJson());

}

void FMStateManager::cacheSortState(const DUrl &url, int role)
{
    FMStateManager::SortStates.insert(url, role);
    FMStateManager::saveSortCache();
}


FMState *FMStateManager::fmState() const
{
    return m_fmState;
}

void FMStateManager::setFmState(FMState *fmState)
{
    m_fmState = fmState;
}

