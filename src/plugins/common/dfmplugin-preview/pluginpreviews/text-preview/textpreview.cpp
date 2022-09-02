/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "textpreview.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfileservices.h"
#include "textbrowseredit.h"

#include <QProcess>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>

using namespace std;
DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;

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

    char *buf = new char[static_cast<unsigned long>(len)];
    device.seekg(0, ios::beg).read(buf, static_cast<streamsize>(len));
    device.close();
    std::string strBuf(buf, len);
    textBrowser->setFileData(strBuf);
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
    return true;
}
