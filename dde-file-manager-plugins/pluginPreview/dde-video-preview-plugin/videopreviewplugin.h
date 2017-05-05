#ifndef VIDEOPREVIEWPLUGIN_H
#define VIDEOPREVIEWPLUGIN_H

#include <QWidget>
#include <QApplication>
#include <QAbstractNativeEventFilter>
#include <QWindow>
#include <QWindowList>
#include <QDebug>

#include <xcb/xproto.h>
#include <xcb/xcb_aux.h>
#include <QX11Info>
#include <xcb/xproto.h>
#undef Bool

#include "../../plugininterfaces/preview/previewinterface.h"

class QLabel;
class DVideoWidget;
class QSlider;
class QFrame;
class MpvProxyWidget;
class VideoPreviewPlugin : public PreviewInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PreviewInterface_iid FILE "dde-video-preview-plugin.json")
    Q_INTERFACES(PreviewInterface)

public:
    VideoPreviewPlugin(QObject *parent = 0);

    void init(const QString &uri);

    QWidget* previewWidget();

    QSize previewWidgetMinSize() const;

    bool canPreview() const;

    QWidget* toolBarItem();

    QString pluginName() const;

    QIcon pluginLogo() const;

    QString pluginDescription() const;

private:

    QString formatNumberString(const int& number) const;

    QStringList m_supportSuffixes;
    QString m_uri;
    QSlider* m_progressSlider;
    QLabel* m_durationLabel;
    DVideoWidget* m_videoWidget;
    QFrame* m_toolBarFrame;
    QStringList m_supportyMimeTypes;
//    MpvProxyWidget* m_mpvProxyWidget;

};

#endif // VIDEOPREVIEWPLUGIN_H
