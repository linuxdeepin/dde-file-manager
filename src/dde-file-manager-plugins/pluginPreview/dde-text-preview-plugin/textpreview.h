// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
