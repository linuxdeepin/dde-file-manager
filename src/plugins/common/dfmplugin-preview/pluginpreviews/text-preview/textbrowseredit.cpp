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
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TextBrowserEdit::scrollbarValueChange);
    connect(verticalScrollBar(), &QScrollBar::sliderMoved, this, &TextBrowserEdit::sliderPositionValueChange);
}

TextBrowserEdit::~TextBrowserEdit()
{
    filestr.clear();
}

void TextBrowserEdit::setFileData(std::string &data)
{
    clear();
    filestr = data;
    std::string::iterator temp = filestr.begin();
    appendText(temp);
    this->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    lastPosition = verticalScrollBar()->sliderPosition();
}

void TextBrowserEdit::wheelEvent(QWheelEvent *e)
{
    QPoint numDegrees = e->angleDelta();
    if (numDegrees.y() < 0) {
        int sbValue = verticalScrollBar()->value();
        if (verticalScrollBar()->maximum() <= sbValue) {
            if (!filestr.empty()) {
                std::string::iterator data = filestr.begin();
                appendText(data);
            }
        }
    }
    QPlainTextEdit::wheelEvent(e);
}

void TextBrowserEdit::scrollbarValueChange(int value)
{
    int maxValue = verticalScrollBar()->maximum();
    if (maxValue <= value) {
        verticalScrollBar()->setValue(value);
    }
}

void TextBrowserEdit::sliderPositionValueChange(int position)
{
    if (position > lastPosition) {
        if (verticalScrollBar()->maximum() <= position) {
            if (!filestr.empty()) {
                std::string::iterator data = filestr.begin();
                appendText(data);
            }
        }
    }
    lastPosition = position;
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

void TextBrowserEdit::appendText(std::string::iterator &data)
{
    if (filestr.size() >= kReadTextSize) {
        char temp[kReadTextSize] { 0 };

        std::copy(data, data + kReadTextSize, temp);

        char ch = temp[kReadTextSize - 1];
        int l = verifyEndOfStrIntegrity(temp, kReadTextSize);
        if (!(0 <= ch && 127 >= static_cast<int>(ch))) {
            temp[l] = '\0';
        }

        filestr.erase(std::begin(filestr), std::begin(filestr) + l);
        QString textData = QString::fromLocal8Bit(temp, l);
        appendPlainText(textData);
    } else if (filestr.size() > 0) {
        unsigned long len = static_cast<unsigned long>(filestr.size());
        QString textData = QString::fromLocal8Bit(filestr.c_str(), static_cast<int>(len));
        insertPlainText(textData);
        filestr.clear();
    }
}
