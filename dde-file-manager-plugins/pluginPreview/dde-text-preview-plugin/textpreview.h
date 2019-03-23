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

#ifndef TEXTPREVIEWPLUGIN_H
#define TEXTPREVIEWPLUGIN_H

#include <QObject>
#include <QWidget>
#include <QPointer>

#include "dfmfilepreview.h"
#include "durl.h"

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
QT_END_NAMESPACE

DFM_BEGIN_NAMESPACE

class TextPreview : public DFMFilePreview
{
    Q_OBJECT

public:
    explicit TextPreview(QObject* parent = 0);
    ~TextPreview();

    bool setFileUrl(const DUrl &url) Q_DECL_OVERRIDE;
    DUrl fileUrl() const override;

    QWidget *contentWidget() const Q_DECL_OVERRIDE;

    QString title() const Q_DECL_OVERRIDE;
    bool showStatusBarSeparator() const Q_DECL_OVERRIDE;

    QWidget* previewWidget();

private:
    DUrl m_url;
    QString m_title;

    QPointer<QPlainTextEdit> m_textBrowser;
};

DFM_END_NAMESPACE

#endif // TEXTPREVIEWPLUGIN_H
