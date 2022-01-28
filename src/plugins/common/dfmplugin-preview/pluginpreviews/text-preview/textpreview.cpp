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

#include <QProcess>
#include <QMimeType>
#include <QMimeDatabase>
#include <QUrl>
#include <QFileInfo>
#include <QPlainTextEdit>
#include <QDebug>
#include <QScrollBar>
#include <QTimer>

using namespace std;
DFMBASE_USE_NAMESPACE
PREVIEW_USE_NAMESPACE
#define READTEXTSIZE 100000

TextPreview::TextPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TextPreview::appendText);
}

TextPreview::~TextPreview()
{
    if (textBrowser)
        textBrowser->deleteLater();

    if (timer) {
        timer->stop();
    }
}

bool TextPreview::setFileUrl(const QUrl &url)
{
    if (selectUrl == url)
        return true;

    timer->stop();

    selectUrl = url;

    device.open(url.path().toLocal8Bit().data(), ios::binary);

    if (!device.is_open()) {
        qInfo() << "File open failed";
        return false;
    }

    if (!textBrowser) {
        textBrowser = new QPlainTextEdit();

        textBrowser->setReadOnly(true);
        textBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        textBrowser->setWordWrapMode(QTextOption::NoWrap);
        textBrowser->setFixedSize(800, 500);
        textBrowser->setFocusPolicy(Qt::NoFocus);
        textBrowser->setContextMenuPolicy(Qt::NoContextMenu);
    }

    titleStr = QFileInfo(url.toLocalFile()).fileName();

    vector<char> buf(device.seekg(0, ios::end).tellg());
    device.seekg(0, ios::beg).read(&buf[0], static_cast<streamsize>(buf.size()));
    device.close();

    char *txt = new char[buf.size()];
    copy(buf.begin(), buf.end(), txt);
    m_textData = QString::fromLocal8Bit(txt, static_cast<int>(buf.size()));
    delete[] txt;
    txt = nullptr;
    textSize = m_textData.count();
    readSize = textSize > READTEXTSIZE ? READTEXTSIZE : textSize;
    if (textSize > readSize) {
        textSize = textSize - readSize;
        textBrowser->setPlainText(m_textData.mid(0, readSize));
        timer->start(500);
    } else {
        textBrowser->setPlainText(m_textData);
    }

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

void TextPreview::appendText()
{
    if (textSize > 0) {
        if (textSize < READTEXTSIZE) {
            textSize = 0;
            textBrowser->appendPlainText(m_textData.mid(readSize));
            timer->stop();
        } else {
            textSize = textSize - READTEXTSIZE;
            textBrowser->appendPlainText(m_textData.mid(readSize, READTEXTSIZE));
            readSize += READTEXTSIZE;
        }
    }
}
