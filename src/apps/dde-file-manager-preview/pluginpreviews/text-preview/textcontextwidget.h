// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTCONTEXTWIDGET_H
#define TEXTCONTEXTWIDGET_H

#include <QWidget>

namespace plugin_filepreview {
class TextBrowserEdit;
class TextContextWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextContextWidget(QWidget *parent = nullptr);
    plugin_filepreview::TextBrowserEdit *textBrowserEdit() const;

private:
    plugin_filepreview::TextBrowserEdit *editWidget { nullptr };
};
}
#endif // TEXTCONTEXTWIDGET_H
