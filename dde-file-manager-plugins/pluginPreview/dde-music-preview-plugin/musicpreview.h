#ifndef MUSICPREVIEWPLUGIN_H
#define MUSICPREVIEWPLUGIN_H

#include "dfmfilepreview.h"
#include "durl.h"

#include <QPointer>

class MusicMessageView;
class ToolBarFrame;

DFM_BEGIN_NAMESPACE

class MusicPreview : public DFMFilePreview
{
    Q_OBJECT

public:
    MusicPreview(QObject *parent = 0);
    ~MusicPreview();

    bool setFileUrl(const DUrl &url) Q_DECL_OVERRIDE;

    QWidget *contentWidget() const Q_DECL_OVERRIDE;
    QWidget *statusBarWidget() const Q_DECL_OVERRIDE;
    Qt::Alignment statusBarWidgetAlignment() const Q_DECL_OVERRIDE;

    bool canPreview(const DUrl &url) const;

private:
    DUrl m_url;

    QPointer<MusicMessageView> m_musicView;
    QPointer<ToolBarFrame> m_statusBarFrame;
};

DFM_END_NAMESPACE

#endif // MUSICPREVIEWPLUGIN_H
