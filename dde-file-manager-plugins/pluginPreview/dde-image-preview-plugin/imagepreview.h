#ifndef IMAGEPREVIEWPLUGIN_H
#define IMAGEPREVIEWPLUGIN_H

#include <QWidget>
#include <QImage>
#include <QPointer>

#include "dfmfilepreview.h"
#include "durl.h"

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class ImageView;

DFM_BEGIN_NAMESPACE

class ImagePreview : public DFMFilePreview
{
    Q_PLUGIN_METADATA(IID PreviewInterface_iid FILE "dde-image-preview-plugin.json")
    Q_INTERFACES(PreviewInterface)
public:
    explicit ImagePreview(QObject *parent = 0);
    ~ImagePreview();

    bool canPreview(const QUrl &url) const;

    void initialize(QWidget *window, QWidget *statusBar) Q_DECL_OVERRIDE;

    bool setFileUrl(const DUrl &url) Q_DECL_OVERRIDE;

    QWidget *contentWidget() const Q_DECL_OVERRIDE;

    QString title() const Q_DECL_OVERRIDE;

private:
    DUrl m_url;
    QPointer<QLabel> m_messageStatusBar;
    QPointer<ImageView> m_imageView;
    QString m_title;
};

DFM_END_NAMESPACE

#endif // IMAGEPREVIEWPLUGIN_H
