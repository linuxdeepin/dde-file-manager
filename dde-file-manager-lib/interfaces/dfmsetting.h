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
    enum OpenFileAction{
        Click,
        DoubleClick
    };
    explicit DFMSetting(QObject *parent = 0);
    void loadConfigDataFromJsonFile(const QString& filePath);
    void reCreateConfigTemplate(const QString& filePath);
    bool isAllwayOpenOnNewWindow();
    int iconSizeIndex();
    OpenFileAction openFileAction();
    QString newWindowPath();
    QString newTabPath();
    DFileView::ViewMode viewMode();

signals:

public slots:

private:
    QPointer<Settings> settings;
    QStringList paths;
};

#endif // DFMSETTING_H
