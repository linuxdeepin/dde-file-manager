// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QPlainTextEdit>

#include <vector>

class TextBrowserEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit TextBrowserEdit(QWidget *parent = nullptr);

    virtual ~TextBrowserEdit() override;

    void setFileData(std::vector<char> &data);

protected:
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void scrollbarValueChange(int value);

    void sliderPositionValueChange(int position);

private:
    int verifyEndOfStrIntegrity(const char *s, int l);

    void appendText(std::vector<char>::iterator &data);

    std::vector<char> fileData;

    int lastPosition {0};
};
#endif   // TEXTBROWSER_H
