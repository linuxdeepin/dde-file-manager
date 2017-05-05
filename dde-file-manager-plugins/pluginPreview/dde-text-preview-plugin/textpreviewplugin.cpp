#include "textpreviewplugin.h"

#include <QProcess>
#include <QMimeType>
#include <QMimeDatabase>
#include <QUrl>
#include <QFileInfo>
#include <QTextEdit>
#include <QDebug>

TextPreviewPlugin::TextPreviewPlugin(QObject *parent):
    PreviewInterface(parent)
{
    m_supportSuffixes << "go" << "html" << "txt"
                      << "java" << "json" << "qml"
                      << "h" << "ini" << "md"
                      << "py" << "sh" << "xml"
                      << "cpp" << "css" << "php"
                      << "pl" << "rss" << "rtx"
                      << "sql" << "hpp" << "htm"
                      << "js" << "perl" << "pro"
                      << "pri";
}

void TextPreviewPlugin::init(const QString &uri)
{
    m_uri = uri;
}


QWidget *TextPreviewPlugin::previewWidget()
{
    QTextEdit* textBrowser = new QTextEdit;
    textBrowser->setReadOnly(true);
    textBrowser->setTextInteractionFlags(Qt::NoTextInteraction);

    QFile f(QUrl(m_uri).path());
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug () << "Failed to read file";
    }

    QString text = f.readAll();
    textBrowser->setText(text);

    return textBrowser;
}

QSize TextPreviewPlugin::previewWidgetMinSize() const
{
    return QSize(400, 500);
}

bool TextPreviewPlugin::canPreview() const
{
    QFileInfo info(QUrl(m_uri).path());
    if(m_supportSuffixes.contains(info.suffix())){
        return true;
    }

    QMimeDatabase md;
    QString mimeType = md.mimeTypeForFile(QUrl(m_uri).path()).name();
    if(mimeType.startsWith("text")){
        return true;
    }

    return false;
}

QWidget *TextPreviewPlugin::toolBarItem()
{
    return NULL;
}

QString TextPreviewPlugin::pluginName() const
{
    return "dde-text-preview-plugin";
}

QIcon TextPreviewPlugin::pluginLogo() const
{
    return QIcon();
}

QString TextPreviewPlugin::pluginDescription() const
{
    return "Deepin text preview plugin";
}
