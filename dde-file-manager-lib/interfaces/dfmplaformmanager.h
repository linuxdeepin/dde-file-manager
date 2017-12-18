#ifndef DFMPLAFORMMANAGER_H
#define DFMPLAFORMMANAGER_H

#include <QObject>
#include <QSettings>

class DFMPlaformManager : public QObject
{
    Q_OBJECT
public:
    explicit DFMPlaformManager(QObject *parent = nullptr);
    QString platformConfigPath();

    bool isDisableUnMount();
    bool isRoot_hidden();

signals:

public slots:

private:
    QSettings* m_platformSettings = nullptr;
};

#endif // DFMPLAFORMMANAGER_H
