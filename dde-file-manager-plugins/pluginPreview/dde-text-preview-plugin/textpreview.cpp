#include "textpreview.h"

#include <QProcess>
#include <QMimeType>
#include <QMimeDatabase>
#include <QUrl>
#include <QFileInfo>
#include <QPlainTextEdit>
#include <QDebug>

DFM_BEGIN_NAMESPACE

TextPreview::TextPreview(QObject *parent):
    DFMFilePreview(parent)
{

}

TextPreview::~TextPreview()
{
    if (m_textBrowser)
        m_textBrowser->deleteLater();
}

bool TextPreview::setFileUrl(const DUrl &url)
{
    if (m_url == url)
        return true;

    if (!url.isLocalFile())
        return false;

    m_url = url;

    if (!m_textBrowser) {
        m_textBrowser = new QPlainTextEdit();

        m_textBrowser->setReadOnly(true);
        m_textBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        m_textBrowser->setWordWrapMode(QTextOption::NoWrap);
        m_textBrowser->setFixedSize(800, 500);
        m_textBrowser->setFocusPolicy(Qt::NoFocus);
    }

    QFile file(url.toLocalFile());

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray &text = file.readAll();

    m_textBrowser->setPlainText(DFMGlobal::toUnicode(text));

    file.close();

    m_title = QFileInfo(url.toLocalFile()).fileName();

    Q_EMIT titleChanged();

    return true;
}

QWidget *TextPreview::contentWidget() const
{
    return m_textBrowser;
}

QString TextPreview::title() const
{
    return m_title;
}

bool TextPreview::showStatusBarSeparator() const
{
    return true;
}

DFM_END_NAMESPACE
