#ifndef MUSICPREVIEWPLUGIN_H
#define MUSICPREVIEWPLUGIN_H

#include "../../plugininterfaces/preview/previewinterface.h"

class MusicPreviewPlugin : public PreviewInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PreviewInterface_iid FILE "dde-music-preview-plugin.json")
    Q_INTERFACES(PreviewInterface)
public:
    MusicPreviewPlugin(QObject *parent = 0);

    void init(const QString &uri);

    QWidget* previewWidget();

    QSize previewWidgetMinSize() const;

    bool canPreview() const;

    QWidget* toolBarItem();

    QString pluginName() const;

    QIcon pluginLogo() const;

    QString pluginDescription() const;

private:
    QStringList m_supportSuffixed;
    QString m_uri;

};

#endif // MUSICPREVIEWPLUGIN_H
