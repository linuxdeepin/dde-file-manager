// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textpreview.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "textbrowseredit.h"

#include "durl.h"

#include <QProcess>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>

using namespace std;

TextPreview::TextPreview(QObject *parent)
    : DFMFilePreview(parent)
{
}

TextPreview::~TextPreview()
{
}

bool TextPreview::setFileUrl(const DUrl &url)
{
    if (selectUrl == url)
        return true;

    selectUrl = url;

    device.open(url.path().toLocal8Bit().data(), ios::binary);

    if (!device.is_open()) {
        qInfo() << "File open failed";
        return false;
    }

    if (!textBrowser) {
        textBrowser = new TextBrowserEdit;
        textBrowser->setReadOnly(true);
        textBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        textBrowser->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        textBrowser->setFixedSize(800, 500);
        textBrowser->setFocusPolicy(Qt::NoFocus);
        textBrowser->setContextMenuPolicy(Qt::NoContextMenu);
    }

    titleStr = QFileInfo(url.toLocalFile()).fileName();

    long len = device.seekg(0, ios::end).tellg();
    if (len <= 0)
        return false;

    vector<char> buf(static_cast<unsigned long>(len));
    device.seekg(0, ios::beg).read(&buf[0], static_cast<streamsize>(buf.size()));
    device.close();

    textBrowser->setFileData(buf);

    Q_EMIT titleChanged();

    return true;
}

DUrl TextPreview::fileUrl() const
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
    return true;
}
