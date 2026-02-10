// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTPREVIEW_H
#define TEXTPREVIEW_H

#include "preview_plugin_global.h"

#include <dfm-base/interfaces/abstractbasepreview.h>

#include <QWidget>
#include <QPointer>
#include <QTimer>
#include <QString>

#include <fstream>

namespace plugin_filepreview {
class TextContextWidget;
class TextPreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT

public:
    explicit TextPreview(QObject *parent = nullptr);
    ~TextPreview() override;

    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;

    QWidget *contentWidget() const override;

    QString title() const override;
    bool showStatusBarSeparator() const override;

private:
    QUrl selectUrl;
    QString titleStr;

    TextContextWidget *textBrowser { nullptr };

    //! 操作文件的对象
    std::ifstream device;

    int textSize = 0;

    int readSize = 0;
};
}
#endif   // TEXTPREVIEW_H
