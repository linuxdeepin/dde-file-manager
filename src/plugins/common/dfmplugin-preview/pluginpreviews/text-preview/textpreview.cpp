// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textpreview.h"
#include "dfileservices.h"
#include "textbrowseredit.h"
#include "textcontextwidget.h"

#include <dfm-base/interfaces/fileinfo.h>

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
}

TextPreview::~TextPreview()
{
    if (textBrowser)
        textBrowser->deleteLater();
}

bool TextPreview::setFileUrl(const QUrl &url)
{
    if (selectUrl == url)
        return true;

    selectUrl = url;

    device.open(url.path().toLocal8Bit().data(), ios::binary);

    if (!device.is_open()) {
        qWarning() << "Text Preview: File open failed!";
        return false;
    }

    if (!textBrowser) {
        textBrowser = new TextContextWidget;
    }

    titleStr = QFileInfo(url.toLocalFile()).fileName();

    long len = device.seekg(0, ios::end).tellg();
    if (len <= 0)
        return false;

    if (len > kReadTextSize)
        len = kReadTextSize;

    char *buf = new char[static_cast<unsigned long>(len)];
    device.seekg(0, ios::beg).read(buf, static_cast<streamsize>(len));
    device.close();
    std::string strBuf(buf, len);
    textBrowser->textBrowserEdit()->setFileData(strBuf);
    delete[] buf;
    buf = nullptr;

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
