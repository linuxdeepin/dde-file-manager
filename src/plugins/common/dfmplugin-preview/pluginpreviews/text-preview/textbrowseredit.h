// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H
#include "preview_plugin_global.h"

#include <QPlainTextEdit>

#include <vector>

namespace plugin_filepreview {
class TextBrowserEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit TextBrowserEdit(QWidget *parent = nullptr);

    virtual ~TextBrowserEdit() override;

    void setFileData(std::string &data);

protected:
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void scrollbarValueChange(int value);

    void sliderPositionValueChange(int position);

private:
    int verifyEndOfStrIntegrity(const char *s, int l);

    void appendText(std::string::iterator &data);

    std::string filestr;

    int lastPosition { 0 };
};
}
#endif   // TEXTBROWSER_H
