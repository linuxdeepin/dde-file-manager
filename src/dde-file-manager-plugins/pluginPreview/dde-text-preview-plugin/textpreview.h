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

#ifndef TEXTPREVIEW_H
#define TEXTPREVIEW_H

#include <QObject>
#include <QWidget>
#include <QPointer>

#include "dfmfilepreview.h"
#include "durl.h"

#include <fstream>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
QT_END_NAMESPACE

class TextPreview : public DFM_NAMESPACE::DFMFilePreview
{
    Q_OBJECT

public:
    explicit TextPreview(QObject *parent = nullptr);
    ~TextPreview() override;

    bool setFileUrl(const DUrl &url) override;
    DUrl fileUrl() const override;

    QWidget *contentWidget() const override;

    QString title() const override;
    bool showStatusBarSeparator() const override;

    QWidget *previewWidget();

public slots:
    void appendText();

private:
    DUrl m_url;
    QString m_title;

    QPointer<QPlainTextEdit> m_textBrowser;

    //! 操作文件的对象
    std::ifstream m_device;

    QTimer *m_timer {nullptr};

    int m_textSize = 0;

    int m_readSize = 0;

    QString m_textData {};
};

#endif // TEXTPREVIEW_H
