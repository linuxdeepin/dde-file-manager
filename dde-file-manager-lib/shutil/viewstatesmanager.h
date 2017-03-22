#ifndef VIEWSTATESMANAGER_H
#define VIEWSTATESMANAGER_H

#include <QObject>
#include "../views/dfileview.h"
#include <QJsonObject>

struct ViewState{
    int iconSize = -1;
    int sortRole = -1;
    Qt::SortOrder sortOrder;
    DFileView::ViewMode viewMode;
    bool dataValid = false;
    bool isValid() const;
};
Q_DECLARE_METATYPE(ViewState)

class ViewStatesManager : public QObject
{
    Q_OBJECT
public:
    explicit ViewStatesManager(QObject *parent = 0);
    void initData();

    void loadViewStates(const QJsonObject &viewStateObje);
    void saveViewState(const QString& url, const ViewState& viewState);
    ViewState viewstate(const QString& url);

    static QString getViewStateFilePath();
    static ViewState objectToViewState(const QJsonObject& obj);
    static QJsonObject viewStateToObject(const ViewState& viewState);

signals:

public slots:

private:
    QMap<QString, ViewState> m_viewStatesMap;
    QJsonObject m_viewStatesJsonObject;

    bool isValidViewStateObj(const QJsonObject& obj);
};

#endif // VIEWSTATESMANAGER_H
