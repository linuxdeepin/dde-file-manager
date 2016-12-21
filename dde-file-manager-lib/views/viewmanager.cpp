#include "viewmanager.h"
#include <QDebug>

ViewManager::ViewManager(QObject *parent) : QObject(parent)
{

}

bool ViewManager::isSchemeRegistered(const QString &scheme)
{
    return m_schemes.contains(scheme);
}

bool ViewManager::isViewIdRegistered(const QString &viewId)
{
    if (m_views.contains(viewId)){
        return true;
    }
    return false;
}

QWidget *ViewManager::getViewById(const QString &viewId)
{
    if (m_views.contains(viewId)){
        return m_views.value(viewId);
    }
    return NULL;
}

QMap<QString, QWidget *> ViewManager::views()
{
    return m_views;
}

QSet<QString> ViewManager::supportSchemes()
{
    return m_schemes;
}

void ViewManager::registerView(const QString &viewId, QWidget *view)
{
    m_views.insert(viewId, view);
}

bool ViewManager::unRegisterView(const QString &viewId, bool isDeleteLater)
{
    int ret = 0;
    if (m_views.contains(viewId)){
        if (isDeleteLater){
            QWidget* w = m_views.value(viewId);
            w->deleteLater();
        }
        ret = m_views.remove(viewId);
    }
    if (ret >= 1){
        return true;
    }
    return false;
}

void ViewManager::registerScheme(const QString &scheme)
{
    m_schemes.insert(scheme);
}
