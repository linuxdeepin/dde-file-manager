#ifndef TEXTPREVIEWPLUGIN_H
#define TEXTPREVIEWPLUGIN_H

#include <QObject>
#include <QWidget>
#include <QPointer>

#include "dfmfilepreview.h"
#include "durl.h"

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
QT_END_NAMESPACE

DFM_BEGIN_NAMESPACE

class TextPreview : public DFMFilePreview
{
    Q_OBJECT

public:
    explicit TextPreview(QObject* parent = 0);
    ~TextPreview();

    bool setFileUrl(const DUrl &url) Q_DECL_OVERRIDE;

    QWidget *contentWidget() const Q_DECL_OVERRIDE;

    QString title() const Q_DECL_OVERRIDE;
    bool showStatusBarSeparator() const Q_DECL_OVERRIDE;

    QWidget* previewWidget();

private:
    DUrl m_url;
    QString m_title;

    QPointer<QPlainTextEdit> m_textBrowser;
};

DFM_END_NAMESPACE

#endif // TEXTPREVIEWPLUGIN_H
