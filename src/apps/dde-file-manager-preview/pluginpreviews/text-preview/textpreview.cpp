// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textpreview.h"
#include "textbrowseredit.h"
#include "textcontextwidget.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <DTextEncoding>

#include <QProcess>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>

using namespace std;
DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;
static constexpr int kReadTextSize { 1024 * 1024 * 5 };

TextPreview::TextPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    fmInfo() << "Text preview: TextPreview instance created";
}

TextPreview::~TextPreview()
{
    fmInfo() << "Text preview: TextPreview instance destroyed";
    if (textBrowser) {
        // Set parent to nullptr first to prevent recursive destruction issues
        textBrowser->setParent(nullptr);
        textBrowser->deleteLater();
        textBrowser = nullptr;
    }
}

bool TextPreview::setFileUrl(const QUrl &url)
{
    fmInfo() << "Text preview: setting file URL:" << url;

    if (selectUrl == url) {
        fmDebug() << "Text preview: URL unchanged, skipping:" << url;
        return true;
    }

    if (!url.isLocalFile()) {
        fmWarning() << "Text preview: URL is not a local file:" << url;
        return false;
    }

    const QString filePath = url.toLocalFile();
    if (!QFileInfo::exists(filePath)) {
        fmWarning() << "Text preview: file does not exist:" << filePath;
        return false;
    }

    selectUrl = url;

    device.open(filePath.toLocal8Bit().data(), ios::binary);

    if (!device.is_open()) {
        fmWarning() << "Text preview: failed to open file:" << filePath;
        return false;
    }

    if (!textBrowser) {
        fmDebug() << "Text preview: creating new TextContextWidget";
        textBrowser = new TextContextWidget;
    }

    titleStr = QFileInfo(filePath).fileName();

    long len = device.seekg(0, ios::end).tellg();
    if (len <= 0) {
        fmWarning() << "Text preview: file is empty or cannot determine size:" << filePath;
        device.close();
        return false;
    }

    fmDebug() << "Text preview: file size:" << len << "bytes, will read up to:" << kReadTextSize << "bytes";

    if (len > kReadTextSize) {
        fmDebug() << "Text preview: file size exceeds limit, truncating to:" << kReadTextSize << "bytes";
        len = kReadTextSize;
    }

    char *buf = new char[static_cast<unsigned long>(len)];
    device.seekg(0, ios::beg).read(buf, static_cast<streamsize>(len));
    device.close();

    bool ok { false };
    QString fileEncoding = DTK_NAMESPACE::DCORE_NAMESPACE::DTextEncoding::detectFileEncoding(filePath, &ok);
    fmDebug() << "Text preview: detected file encoding:" << fileEncoding << "detection success:" << ok;

    std::string strBuf(buf, static_cast<unsigned long>(len));
    if (ok && fileEncoding.toLower() != "utf-8") {
        fmDebug() << "Text preview: converting from" << fileEncoding << "to UTF-8";
        QByteArray out;
        QByteArray in(buf, static_cast<int>(len));
        if (DTK_NAMESPACE::DCORE_NAMESPACE::DTextEncoding::convertTextEncoding(in, out, "utf-8")) {
            strBuf = out.toStdString();
            fmDebug() << "Text preview: encoding conversion successful";
        } else {
            fmWarning() << "Text preview: encoding conversion failed, using original data";
        }
    }

    textBrowser->textBrowserEdit()->setFileData(strBuf);
    // Set syntax highlighting after setting file data
    textBrowser->textBrowserEdit()->setSyntaxDefinition(filePath);

    delete[] buf;
    buf = nullptr;

    fmInfo() << "Text preview: file loaded successfully:" << filePath << "title:" << titleStr;
    Q_EMIT titleChanged();

    return true;
}

QUrl TextPreview::fileUrl() const
{
    return selectUrl;
}

QWidget *TextPreview::contentWidget() const
{
    return textBrowser;
}

QString TextPreview::title() const
{
    return titleStr;
}

bool TextPreview::showStatusBarSeparator() const
{
    return false;
}
