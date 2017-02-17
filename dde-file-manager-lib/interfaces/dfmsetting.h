#ifndef DFMSETTING_H
#define DFMSETTING_H

#include <QObject>
#include "views/dfileview.h"
#include "dtk_global.h"
#include <QJsonObject>
#include <QDir>

DTK_BEGIN_NAMESPACE

class Settings;

DTK_END_NAMESPACE

DTK_USE_NAMESPACE
class DFMSetting : public QObject
{
    Q_OBJECT

public:
    explicit DFMSetting(QObject *parent = 0);
    void initConnections();
    QVariant getValueByKey(const QString& key);
    bool isAllwayOpenOnNewWindow();
    int iconSizeIndex();
    int openFileAction();
    QString newWindowPath();
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
    QDir::Filters viewFilters();

    QPointer<Settings> settings();

signals:

public slots:
    void onValueChanged(const QString& key, const QVariant& value);

private:
    Settings* m_settings;
    QStringList paths;
};

#endif // DFMSETTING_H
