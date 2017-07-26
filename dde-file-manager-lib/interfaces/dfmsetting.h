#ifndef DFMSETTING_H
#define DFMSETTING_H

#include <QObject>
#include <dtkcore_global.h>
#include "durl.h"
#include <QJsonObject>
#include <QDir>


class DAbstractFileWatcher;

DCORE_BEGIN_NAMESPACE

class DSettings;

DCORE_END_NAMESPACE

DTK_USE_NAMESPACE
class DFMSetting : public QObject
{
    Q_OBJECT

public:

    static DFMSetting* instance();

    explicit DFMSetting(QObject *parent = 0);
    void initConnections();
    QVariant getValueByKey(const QString& key);
    bool isAllwayOpenOnNewWindow();
    int iconSizeIndex();
    int openFileAction();
    QString defaultWindowPath();
    QString newTabPath();
    QString getConfigFilePath();
    bool isQuickSearch();
    bool isCompressFilePreview();
    bool isTextFilePreview();
    bool isDocumentFilePreview();
    bool isImageFilePreview();
    bool isVideoFilePreview();
    bool isAutoMount();
    bool isAutoMountAndOpen();
    bool isDefaultChooserDialog();
    bool isShowedHiddenOnSearch();
    bool isShowedHiddenOnView();

    QPointer<DTK_CORE_NAMESPACE::DSettings> settings();

signals:

public slots:
    void onValueChanged(const QString& key, const QVariant& value);
    void onConfigFileChanged(const DUrl &fromUrl, const DUrl &toUrl);

private:
    DTK_CORE_NAMESPACE::DSettings* m_settings;
    QStringList m_newTabOptionPaths;
    QStringList m_defaultWindowOptionPaths;
    DAbstractFileWatcher* m_fileSystemWathcer;
};

#endif // DFMSETTING_H
