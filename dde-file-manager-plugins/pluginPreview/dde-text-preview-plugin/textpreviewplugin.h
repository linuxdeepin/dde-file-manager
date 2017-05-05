#ifndef TEXTPREVIEWPLUGIN_H
#define TEXTPREVIEWPLUGIN_H

#include <QObject>
#include <QWidget>
#include "../../plugininterfaces/preview/previewinterface.h"

class TextPreviewPlugin : public PreviewInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PreviewInterface_iid FILE "dde-text-preview-plugin.json")
    Q_INTERFACES(PreviewInterface)
public:
    explicit TextPreviewPlugin(QObject* parent = 0);

    void init(const QString &uri);

    QWidget* previewWidget();

    QSize previewWidgetMinSize() const;

    bool canPreview() const;

    QWidget* toolBarItem();

    QString pluginName() const;

    QIcon pluginLogo() const;

    QString pluginDescription() const;

private:
    QStringList m_supportSuffixes;
    QString m_uri;
};

#endif // TEXTPREVIEWPLUGIN_H
