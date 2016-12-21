#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QObject>
#include <QWidget>
#include <QMap>
#include <QSet>

class ViewManager : public QObject
{
    Q_OBJECT
public:
    explicit ViewManager(QObject *parent = 0);

    bool isSchemeRegistered(const QString& scheme);
    bool isViewIdRegistered(const QString& viewId);
    QWidget* getViewById(const QString& viewId);

    QMap<QString, QWidget*> views();
    QSet<QString> supportSchemes();

signals:

public slots:
    void registerView(const QString& viewId, QWidget* view);
    bool unRegisterView(const QString& viewId, bool isDeleteLater=true);
    void registerScheme(const QString& scheme);

private:
    QMap<QString, QWidget*> m_views = {};
    QSet<QString> m_schemes;
};

#endif // VIEWMANAGER_H
