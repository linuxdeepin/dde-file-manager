#ifndef FILEMENUMANAGER_H
#define FILEMENUMANAGER_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QMap>


class FileMenuManager : public QObject
{
    Q_OBJECT
public:
    explicit FileMenuManager(QObject *parent = 0);
    ~FileMenuManager();

    void initData();
    void initActions();
    QMenu* genereteMenuByFileType(QString type);
    QMenu* genereteMenuByKeys(const QStringList keys);
signals:

public slots:
    void handleAction(QAction* action);

private:
    QMap<QString, QString> m_actionKeys;
    QMap<QString, QAction*> m_actions;
};

#endif // FILEMENUMANAGER_H
