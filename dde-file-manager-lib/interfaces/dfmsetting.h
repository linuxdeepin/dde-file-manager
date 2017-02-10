#ifndef DFMSETTING_H
#define DFMSETTING_H

#include <QObject>
#include "views/dfileview.h"
#include <QJsonObject>
#include <settings.h>
DTK_USE_NAMESPACE
class DFMSetting : public QObject
{
    Q_OBJECT

public:
    explicit DFMSetting(QObject *parent = 0);
    bool isAllwayOpenOnNewWindow();
    int iconSizeIndex();
    int openFileAction();
    QString newWindowPath();
    QString newTabPath();
    QString getConfigFilePath();
    void setSettings(Settings* settings);

signals:

public slots:

private:
    QPointer<Settings> m_settings;
    QStringList paths;
};

#endif // DFMSETTING_H
