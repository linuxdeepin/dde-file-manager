// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textbrowseredit.h"

#include <QScrollBar>
#include <QDebug>
#include <QTextCodec>
#include <algorithm>

constexpr int kReadTextSize { 1024 * 1024 * 5 };
TextBrowserEdit::TextBrowserEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setViewportMargins(0, 0, 40, 0);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TextBrowserEdit::scrollbarValueChange);
    connect(verticalScrollBar(), &QScrollBar::sliderMoved, this, &TextBrowserEdit::sliderPositionValueChange);
}

TextBrowserEdit::~TextBrowserEdit()
{
    fileData.clear();
}

void TextBrowserEdit::setFileData(std::vector<char> &data)
{
    clear();
    fileData = data;
    std::vector<char>::iterator temp = fileData.begin();
    appendText(temp);
    this->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    lastPosition = verticalScrollBar()->sliderPosition();
}

void TextBrowserEdit::wheelEvent(QWheelEvent *e)
{
    QPoint numDegrees = e->angleDelta();
    if (numDegrees.y() < 0) {
        int sbValue = verticalScrollBar()->value();
        if(verticalScrollBar()->maximum() <= sbValue) {
            if(fileData.begin() != fileData.end()) {
                std::vector<char>::iterator data = fileData.begin();
                appendText(data);
            }
        }
    }
    QPlainTextEdit::wheelEvent(e);
}

void TextBrowserEdit::scrollbarValueChange(int value)
{
    if(verticalScrollBar()->maximum() <= value){
        verticalScrollBar()->setValue(value);
    }
}

void TextBrowserEdit::sliderPositionValueChange(int position)
{
    if(position > lastPosition){
        if(verticalScrollBar()->maximum() <= position) {
            if(fileData.begin() != fileData.end()) {
                std::vector<char>::iterator data = fileData.begin();
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

void TextBrowserEdit::appendText(std::vector<char>::iterator &data)
{
    QString textData;
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if (fileData.size() >= kReadTextSize) {
        char temp[kReadTextSize] { 0 };

        std::copy(data, data + kReadTextSize, temp);

        char ch = temp[kReadTextSize - 1];
        int l = verifyEndOfStrIntegrity(temp, kReadTextSize);
        if (!(0 <= ch && 127 >= static_cast<int>(ch))) {
            temp[l] = '\0';
        }
        fileData.erase(std::begin(fileData), std::begin(fileData) + l);

        codec->toUnicode(QByteArray(temp).constData(), l, &state);
        if (state.invalidChars > 0){
            QTextCodec *codec = QTextCodec::codecForName("GBK");
            QTextStream in(temp);
            in.setCodec(codec);
            textData = in.readAll();
        } else {
            textData = QString::fromLocal8Bit(temp, l);
        }

        insertPlainText(textData);
    } else if (fileData.size() > 0) {
        unsigned long len = static_cast<unsigned long>(fileData.size());
        char *buf = new char[len];
        std::copy(data, fileData.end(), buf);
        fileData.erase(std::begin(fileData), std::begin(fileData) + static_cast<int>(len));

        codec->toUnicode(QByteArray(buf,len).constData(), int(len) , &state);
        if (state.invalidChars > 0){
            QTextCodec *codec = QTextCodec::codecForName("GBK");
            QTextStream in(buf);
            in.setCodec(codec);
            textData = in.readAll();
        } else {
            textData = QString::fromLocal8Bit(buf, static_cast<int>(len));
        }

        delete[] buf;
        buf = nullptr;
        insertPlainText(textData);
    }
}


