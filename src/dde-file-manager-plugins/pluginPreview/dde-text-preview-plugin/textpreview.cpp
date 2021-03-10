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

DFM_USE_NAMESPACE

TextPreview::TextPreview(QObject *parent):
    DFMFilePreview(parent)
{

}

TextPreview::~TextPreview()
{
    if (m_textBrowser)
        m_textBrowser->deleteLater();
    if (m_device)
        m_device->deleteLater();
}

bool TextPreview::setFileUrl(const DUrl &url)
{
    if (m_url == url)
        return true;

    m_url = url;

    m_flg = true;   //! 预览新文件时对其设置初始值

    //! 关闭并回收上个文件使用的m_device文件操作对象
    if (m_device) {
        m_device->close();
        delete m_device;
        m_device = nullptr;
    }

    QByteArray text;

    {
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

        if (!info)
            return false;

        m_device = info->createIODevice();

        if (!m_device) {
            if (url.isLocalFile()) {
                m_device = new QFile(url.toLocalFile());
            }
        }

        if (!m_device)
            return false;

        if (!m_device->open(QIODevice::ReadOnly)) {
            return false;
        }

        //fix 如果文件不可读行，之后读行会导致文管假死或中断
        //文本如果不可读行就不显示预览
        if (!m_device->canReadLine()) {
            return false;
        }
    }

    if (!m_textBrowser) {
        m_textBrowser = new QPlainTextEdit();

        m_textBrowser->setReadOnly(true);
        m_textBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        m_textBrowser->setWordWrapMode(QTextOption::NoWrap);
        m_textBrowser->setFixedSize(800, 500);
        m_textBrowser->setFocusPolicy(Qt::NoFocus);
        m_textBrowser->setContextMenuPolicy(Qt::NoContextMenu);

        //! 滚动条值改变的信号与曹的连接，用以处理分段加载数据的问题
        connect(m_textBrowser->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
    }

    //! 每次预览文本时，预先加载49行数据提供显示
    for (int i = 0; i < 49; ++i) {
        text.append(m_device->readLine());
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

void TextPreview::valueChanged(int index)
{
    if (index >= m_textBrowser->verticalScrollBar()->maximum() && m_flg) {
        if (!m_device)
            return ;

        m_flg = false; //! 等待这次文本数据加载完成后重新设置为true，以免在设置数据时重复执行以下代码

        //! 每次分段加载数据36行
        QByteArray text;
        for (int i = 0; i < 36; ++i) {
            text.append(m_device->readLine()); //! 读取文本一行数据，并追加到text中
        }
        QString convertedStr{ DFMGlobal::toUnicode(text, m_url.toLocalFile()) }; //! 字符编码转换

        m_textBrowser->appendPlainText(convertedStr);
        m_textBrowser->verticalScrollBar()->setValue(index);    //! 设置垂直滚动条停留的位置

        m_flg = true; //! 设置为true使得下次需要分段加载数据能够被执行
    }
}
