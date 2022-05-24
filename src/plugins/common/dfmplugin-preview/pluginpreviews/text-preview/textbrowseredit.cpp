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
#include "textbrowseredit.h"

#include <QScrollBar>
#include <QDebug>

#include <algorithm>

PREVIEW_USE_NAMESPACE
constexpr int kReadTextSize { 1024 * 1024 * 5 };
TextBrowserEdit::TextBrowserEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TextBrowserEdit::scrollbarVauleChange);
}

void TextBrowserEdit::setFileData(std::vector<char> &data)
{
    fileData = data;
    std::vector<char>::iterator temp = fileData.begin();
    appendText(temp);
    verticalScrollBar()->setValue(0);
}

void TextBrowserEdit::scrollbarVauleChange(int value)
{
    if (verticalScrollBar()->maximum() <= value) {
        std::vector<char>::iterator data = fileData.begin();
        appendText(data);
        verticalScrollBar()->setValue(value);
    }
}

int TextBrowserEdit::verifyEndOfStrIntegrity(const char *s, int l)
{
    int len = 0, i = 0;
    if (s == nullptr || *s == 0 || l <= 0) {
        return 0;
    }
    while (*s) {
        if ((*s & 0x80) == 0) {
            i++;
            s++;
            len++;
        } else {
            if (*(s + 1) == 0)
                break;
            i += 3;
            s += 3;
            len += 3;
        }

        if (i == l)
            break;
        else if (i > l) {
            len -= 3;
            break;
        }
    }
    return len;
}

void TextBrowserEdit::appendText(std::vector<char>::iterator &data)
{
    if (fileData.size() >= kReadTextSize) {
        char temp[kReadTextSize] { 0 };

        std::copy(data, data + kReadTextSize, temp);

        char ch = temp[kReadTextSize - 1];
        int l = verifyEndOfStrIntegrity(temp, kReadTextSize);
        if (!(0 <= ch && 127 >= static_cast<int>(ch))) {
            temp[l] = '\0';
        }

        fileData.erase(std::begin(fileData), std::begin(fileData) + l);
        QString textData = QString::fromLocal8Bit(temp, l);
        appendPlainText(textData);
    } else if (fileData.size() > 0) {
        unsigned long len = static_cast<unsigned long>(fileData.size());
        char *buf = new char[len];
        std::copy(data, fileData.end(), buf);
        fileData.erase(std::begin(fileData), std::begin(fileData) + static_cast<int>(len));
        QString textData = QString::fromLocal8Bit(buf, static_cast<int>(len));
        delete[] buf;
        buf = nullptr;
        appendPlainText(textData);
    }
}
