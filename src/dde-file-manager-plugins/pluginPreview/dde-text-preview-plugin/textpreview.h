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

#ifndef TEXTPREVIEW_H
#define TEXTPREVIEW_H

#include "dfmfilepreview.h"

#include <QWidget>
#include <QPointer>
#include <QTimer>
#include <QString>

#include <fstream>

class TextBrowserEdit;
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

private:
    DUrl selectUrl;
    QString titleStr;

    TextBrowserEdit *textBrowser { nullptr };

    //! 操作文件的对象
    std::ifstream device;

    int textSize = 0;

    int readSize = 0;
};
#endif   // TEXTPREVIEW_H
