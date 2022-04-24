/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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
#include "dabstractfileinfo.h"
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
DFM_USE_NAMESPACE

#define READTEXTSIZE 100000

TextPreview::TextPreview(QObject *parent):
    DFMFilePreview(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TextPreview::appendText);
}

TextPreview::~TextPreview()
{
    if (m_textBrowser)
        m_textBrowser->deleteLater();

    if(m_timer){
        m_timer->stop();
    }
}

bool TextPreview::setFileUrl(const DUrl &url)
{
    if (m_url == url)
        return true;

    m_timer->stop();

    m_url = url;

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

    if (!info)
        return false;

    m_device.open(info->toLocalFile().toLocal8Bit().data(), ios::binary);

    if(!m_device.is_open()){
        qWarning() << "open file failed :" << m_url;
        return false;
    }

    if (!m_textBrowser) {
        m_textBrowser = new QPlainTextEdit();

        m_textBrowser->setReadOnly(true);
        m_textBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        m_textBrowser->setWordWrapMode(QTextOption::NoWrap);
        m_textBrowser->setFixedSize(800, 500);
        m_textBrowser->setFocusPolicy(Qt::NoFocus);
        m_textBrowser->setContextMenuPolicy(Qt::NoContextMenu);
    }

    m_title = info->fileName();

    long len = m_device.seekg(0, ios::end).tellg();
    if (len <= 0)
        return false;

    vector<char> buf(static_cast<unsigned long>(len));
    m_device.seekg(0, ios::beg).read(&buf[0], static_cast<streamsize>(buf.size()));
    m_device.close();

    char *txt = new char[buf.size() + 1];
    copy(buf.begin(), buf.end(), txt);
    m_textData = QString::fromLocal8Bit(txt, static_cast<int>(buf.size()));
    delete [] txt;
    txt = nullptr;
    m_textSize = m_textData.count();
    m_readSize = m_textSize > READTEXTSIZE ? READTEXTSIZE : m_textSize;
    if(m_textSize > m_readSize) {
        m_textSize = m_textSize - m_readSize;
        m_textBrowser->setPlainText(m_textData.mid(0, m_readSize));
        m_timer->start(500);
    } else {
        m_textBrowser->setPlainText(m_textData);
    }

    Q_EMIT titleChanged();

    return true;
}

DUrl TextPreview::fileUrl() const
{
    return m_url;
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

void TextPreview::appendText()
{
    if(m_textSize > 0) {
        if(m_textSize < READTEXTSIZE) {
            m_textSize = 0;
            m_textBrowser->appendPlainText(m_textData.mid(m_readSize));
            m_timer->stop();
        } else {
            m_textSize = m_textSize - READTEXTSIZE;
            m_textBrowser->appendPlainText(m_textData.mid(m_readSize, READTEXTSIZE));
            m_readSize += READTEXTSIZE;
        }
    }
}
