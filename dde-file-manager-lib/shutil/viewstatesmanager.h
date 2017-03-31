#ifndef VIEWSTATESMANAGER_H
#define VIEWSTATESMANAGER_H

#include <QObject>
#include "../views/dfileview.h"
#include "dfilesystemmodel.h"
#include <QJsonObject>
#include "durl.h"

struct ViewState{
    int iconSize = -1;
    int sortRole = -1;
    Qt::SortOrder sortOrder;
    DFileView::ViewMode viewMode;
    bool isValid() const;
};

Q_DECLARE_FLAGS(SortOrders, Qt::SortOrder)

Q_DECLARE_METATYPE(ViewState)

class ViewStatesManager : public QObject
{
    Q_OBJECT
    Q_ENUM(Qt::SortOrder)
public:
    explicit ViewStatesManager(QObject *parent = 0);
    void initData();

    void loadViewStates(const QJsonObject &viewStateObj);
    void loadDefaultViewStates(const QJsonObject& viewStateObj);

    void saveViewState(const DUrl& url, const ViewState& viewState);
    ViewState viewstate(const DUrl& url);

    static QString getViewStateFilePath();
    static ViewState objectToViewState(const QJsonObject& obj);
    static QJsonObject viewStateToObject(const ViewState& viewState);
    static QString getDefaultViewStateConfigFile();
    static bool isValidViewStateObj(const QJsonObject& obj);
    static bool isValidViewState(const ViewState& state);

signals:

public slots:

private:
    QMap<DUrl, ViewState> m_viewStatesMap;
    QMap<DUrl, ViewState> m_defaultViewStateMap;
    QJsonObject m_viewStatesJsonObject;
    QJsonObject m_defautlViewStateJsonObject;

};

#endif // VIEWSTATESMANAGER_H
