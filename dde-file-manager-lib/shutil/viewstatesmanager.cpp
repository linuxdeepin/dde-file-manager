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
    QString defaultViewStateFilePath = getDefaultViewStateConfigFile();

    m_viewStatesJsonObject = FileUtils::getJsonObjectFromFile(viewStateFilePath);
    m_defautlViewStateJsonObject = FileUtils::getJsonObjectFromFile(defaultViewStateFilePath);

    //load user saved view states and default view states config file
    loadViewStates(m_viewStatesJsonObject);
    loadDefaultViewStates(m_defautlViewStateJsonObject);
}

void ViewStatesManager::loadViewStates(const QJsonObject& viewStateObj)
{
    foreach (const QString& url, viewStateObj.keys()) {
        QJsonObject obj = viewStateObj[url].toObject();

        //check if is data valid
        if(!isValidViewStateObj(obj))
            continue;

        m_viewStatesMap.insert(DUrl::fromUserInput(url), objectToViewState(obj));
    }
}

void ViewStatesManager::loadDefaultViewStates(const QJsonObject& viewStateObj)
{
    foreach (const QString& url, viewStateObj.keys()) {
        QJsonObject obj = viewStateObj[url].toObject();

        //check if is data valid
        if(!isValidViewStateObj(obj))
            continue;

        m_defaultViewStateMap.insert(DUrl::fromUserInput(url), objectToViewState(obj));
    }
}

void ViewStatesManager::saveViewState(const DUrl &url, const ViewState &viewState)
{
    if(m_viewStatesMap.contains(url)){
        m_viewStatesMap.take(url);
        m_viewStatesMap.insert(url, viewState);
    } else{
        m_viewStatesMap.insert(url, viewState);
    }

    //store viewState to obj
    QJsonObject stateObj = viewStateToObject(viewState);

    if(m_viewStatesJsonObject.contains(url.toString())){
        m_viewStatesJsonObject[url.toString()] = stateObj;
    } else{
        m_viewStatesJsonObject.insert(url.toString(), stateObj);
    }

    //write datas
    bool ret = FileUtils::writeJsonObjectFile(getViewStateFilePath(), m_viewStatesJsonObject);
    if(!ret){
        qDebug () << "failed to save viewState file";
    }
}

ViewState ViewStatesManager::viewstate(const DUrl &url)
{
    if(m_viewStatesMap.contains(url)){
        return m_viewStatesMap.value(url);
    } else if(m_defaultViewStateMap.contains(url)){
        return m_defaultViewStateMap.value(url);
    }
    return ViewState();
}

QString ViewStatesManager::getViewStateFilePath()
{
    //viewstate config file choosing between dialog mode and window mode
    QString confFile;
    if(DFMGlobal::IsFileManagerDiloagProcess)
        confFile = "dialogviewstates.json";
    else
        confFile = "windowviewstates.json";

    return QString("%1/%2").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::ApplicationConfigPath), confFile);
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

QString ViewStatesManager::getDefaultViewStateConfigFile()
{
    return QString("%1/%2/%3").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::ApplicationSharePath),
                                "config",
                                "default-view-states.json");
}

bool ViewStatesManager::isValidViewStateObj(const QJsonObject &obj)
{
    return (obj.contains("iconSize") && obj.contains("viewMode") &&
            obj.contains("sortRole") && obj.contains("sortOrder"));
}

bool ViewStatesManager::isValidViewState(const ViewState &state)
{
    //iconSize
    if(state.iconSize < 0 || state.iconSize > 4)
        return false;

    //viewMode
    QMetaEnum viewModeMeta = QMetaEnum::fromType<DFileView::ViewMode>();
    if(viewModeMeta.valueToKey(state.viewMode) == "")
        return false;

    //sortOrder
    QMetaEnum sortOrderMeta = QMetaEnum::fromType<Qt::SortOrder>();
    if(sortOrderMeta.valueToKey(state.sortOrder) == "")
        return false;

    //sortRole
    QMetaEnum rolesMeta = QMetaEnum::fromType<DFileSystemModel::Roles>();
    if(rolesMeta.valueToKey((DFileSystemModel::Roles)state.sortRole) == "")
        return false;

    return true;
}

bool ViewState::isValid() const
{
    return ViewStatesManager::isValidViewState(*this);
}
