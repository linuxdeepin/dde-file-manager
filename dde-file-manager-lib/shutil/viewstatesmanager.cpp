#include <QTextStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include "dfmstandardpaths.h"
#include "viewstatesmanager.h"
#include "fileutils.h"

ViewStatesManager::ViewStatesManager(QObject *parent) : QObject(parent)
{
    initData();
}

void ViewStatesManager::initData()
{
    QString viewStateFilePath = getViewStateFilePath();
    m_viewStatesJsonObject = FileUtils::getJsonObjectFromFile(viewStateFilePath);
    loadViewStates(m_viewStatesJsonObject);
}

void ViewStatesManager::loadViewStates(const QJsonObject& viewStateObje)
{
    foreach (const QString& url, viewStateObje.keys()) {
        QJsonObject obj = viewStateObje[url].toObject();

        //check if is data valid
        if(!isValidViewStateObj(obj))
            continue;

        m_viewStatesMap.insert(url, objectToViewState(obj));
    }
}

void ViewStatesManager::saveViewState(const QString &url, const ViewState &viewState)
{
    if(m_viewStatesMap.contains(url)){
        m_viewStatesMap.take(url);
        m_viewStatesMap.insert(url, viewState);
    } else{
        m_viewStatesMap.insert(url, viewState);
    }

    //store viewState to obj
    QJsonObject stateObj = viewStateToObject(viewState);

    if(m_viewStatesJsonObject.contains(url)){
        m_viewStatesJsonObject[url] = stateObj;
    } else{
        m_viewStatesJsonObject.insert(url, stateObj);
    }

    //write datas
    bool ret = FileUtils::writeJsonObjectFile(getViewStateFilePath(), m_viewStatesJsonObject);
    if(!ret){
        qDebug () << "failed to save viewState file";
    }
}

ViewState ViewStatesManager::viewstate(const QString &url)
{
    if(m_viewStatesMap.contains(url))
        return m_viewStatesMap.value(url);
    return ViewState();
}

QString ViewStatesManager::getViewStateFilePath()
{
    return QString("%1/%2").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::ApplicationConfigPath), "viewstates.json");
}

ViewState ViewStatesManager::objectToViewState(const QJsonObject &obj)
{
    ViewState viewState;
    QMetaEnum vieModeEnum = QMetaEnum::fromType<DFileView::ViewMode>();
    QMetaEnum sortEnum = QMetaEnum::fromType<Qt::SortOrder>();

    viewState.iconSize = obj["iconSize"].toInt();
    viewState.sortRole = obj["sortRole"].toInt();
    viewState.sortOrder = (Qt::SortOrder)sortEnum.keyToValue(obj["sortOrder"].toString().toLocal8Bit().constData());
    viewState.viewMode = (DFileView::ViewMode)vieModeEnum.keysToValue(obj["viewMode"].toString().toLocal8Bit().constData());
    viewState.dataValid = true;
    return viewState;
}

QJsonObject ViewStatesManager::viewStateToObject(const ViewState &viewState)
{
    QMetaEnum vieModeEnum = QMetaEnum::fromType<DFileView::ViewMode>();
    QMetaEnum sortEnum = QMetaEnum::fromType<Qt::SortOrder>();
    QJsonObject obj;

    obj.insert("iconSize", viewState.iconSize);
    obj.insert("sortRole", viewState.sortRole);
    obj.insert("viewMode", vieModeEnum.valueToKey(viewState.viewMode));
    obj.insert("sortOrder", sortEnum.valueToKey(viewState.sortOrder));

    return obj;
}

bool ViewStatesManager::isValidViewStateObj(const QJsonObject &obj)
{
    return (obj.contains("iconSize") && obj.contains("viewMode") &&
            obj.contains("sortRole") && obj.contains("sortOrder"));
}

bool ViewState::isValid() const
{
    return dataValid;
}
