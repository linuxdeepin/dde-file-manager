#ifndef IMAGEPREVIEWPLUGIN_H
#define IMAGEPREVIEWPLUGIN_H

#include <QWidget>
#include <QImage>

#include "../../plugininterfaces/preview/previewinterface.h"

class ImagePreviewPlugin : public QObject, public PreviewInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PreviewInterface_iid FILE "dde-image-preview-plugin.json")
    Q_INTERFACES(PreviewInterface)
public:
    explicit ImagePreviewPlugin(QObject *parent = 0);
    ~ImagePreviewPlugin();

    void init(const QString &uri);

    QWidget* previewWidget();

    QSize previewWidgetMinSize() const;

    bool canPreview() const;

    QWidget* toolBarItem();

    QString pluginName() const;

    QIcon pluginLogo() const;

    QString pluginDescription() const;

signals:

public slots:
private:
    QString m_uri;
};

#endif // IMAGEPREVIEWPLUGIN_H
