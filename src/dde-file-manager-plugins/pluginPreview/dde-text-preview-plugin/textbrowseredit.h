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
