/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

    m_url = url;

    QByteArray text;

    {
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

        if (!info)
            return false;

        QScopedPointer<QIODevice> device(info->createIODevice());

        if (!device) {
            if (url.isLocalFile()) {
               device.reset(new QFile(url.toLocalFile()));
            }
        }

        if (!device)
            return false;

        if (!device->open(QIODevice::ReadOnly)) {
            return false;
        }

        text = device->readAll();
    }

    if (!m_textBrowser) {
        m_textBrowser = new QPlainTextEdit();

        m_textBrowser->setReadOnly(true);
        m_textBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        m_textBrowser->setWordWrapMode(QTextOption::NoWrap);
        m_textBrowser->setFixedSize(800, 500);
        m_textBrowser->setFocusPolicy(Qt::NoFocus);
    }


    QString convertedStr{ DFMGlobal::toUnicode(text, url.toLocalFile()) };

    m_textBrowser->setPlainText(convertedStr);
    m_title = QFileInfo(url.toLocalFile()).fileName();

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

DFM_END_NAMESPACE
