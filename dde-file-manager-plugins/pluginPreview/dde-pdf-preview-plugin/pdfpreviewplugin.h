#ifndef PDFPREVIEWPLUGIN_H
#define PDFPREVIEWPLUGIN_H

#include <QObject>
#include <QWidget>
#include "../../plugininterfaces/preview/previewinterface.h"

class PDFPreviewPlugin : public QObject, public PreviewInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PreviewInterface_iid FILE "dde-pdf-preview-plugin.json")
    Q_INTERFACES(PreviewInterface)

public:
    PDFPreviewPlugin(QObject *parent = 0);

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

#endif // PDFPREVIEWPLUGIN_H
