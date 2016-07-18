#ifndef SEARCHHISTROYMANAGER_H
#define SEARCHHISTROYMANAGER_H

#include <QObject>

#include "basemanager.h"

class SearchHistory;

class SearchHistroyManager : public QObject, public BaseManager
{
    Q_OBJECT
public:
    explicit SearchHistroyManager(QObject *parent = 0);
    ~SearchHistroyManager();
    void load();
    void save();
    QStringList toStringList();
    static QString getSearchHistroyCachePath();
private:
    void loadJson(const QJsonObject &json);
    void writeJson(QJsonObject &json);
    QList<SearchHistory *> m_historyList;
    QStringList m_stringList;

public slots:
    void writeIntoSearchHistory(QString keyword);
};

#endif // SEARCHHISTROYMANAGER_H
