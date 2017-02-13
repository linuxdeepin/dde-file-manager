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
    QVariant getValueByKey(const QString& key);
    bool isAllwayOpenOnNewWindow();
    int iconSizeIndex();
    int openFileAction();
    QString newWindowPath();
    QString newTabPath();
    QString getConfigFilePath();
    void setSettings(Settings* settings);
    bool isQuickSearch();
    bool isCompressFilePreview();
    bool isTextFilePreview();
    bool isDocumentFilePreview();
    bool isImageFilePreview();
    bool isVideoFilePreview();
    bool isAutoMount();
    bool isAutoMountAndOpen();
    bool isDefaultChooserDialog();

signals:

public slots:

private:
    QPointer<Settings> m_settings;
    QStringList paths;
};

#endif // DFMSETTING_H
