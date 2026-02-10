// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textbrowseredit.h"

#include <DGuiApplicationHelper>
#include <KSyntaxHighlighting/Theme>

#include <QScrollBar>
#include <QDebug>

#include <algorithm>

DGUI_USE_NAMESPACE
using namespace plugin_filepreview;
constexpr int kReadTextSize { 1024 * 1024 * 5 };
TextBrowserEdit::TextBrowserEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setFixedSize(800, 500);
    setFrameStyle(QFrame::NoFrame);

    // Set fixed-pitch (monospace) font, reference: deepin-editor dtextedit.cpp:1909
    QFont font = document()->defaultFont();
    font.setFamily("Noto Mono");
    font.setFixedPitch(true);
    font.setPointSizeF(10);
    setFont(font);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TextBrowserEdit::scrollbarValueChange);
    connect(verticalScrollBar(), &QScrollBar::sliderMoved, this, &TextBrowserEdit::sliderPositionValueChange);

    // Connect to system theme change signal for dynamic theme switching
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &TextBrowserEdit::onThemeTypeChanged);
}

TextBrowserEdit::~TextBrowserEdit()
{
    filestr.clear();
    // Explicitly delete highlighter to ensure clean destruction
    if (m_highlighter) {
        delete m_highlighter;
        m_highlighter = nullptr;
    }
}

void TextBrowserEdit::setFileData(std::string &data)
{
    // Clear old content first
    clear();
    filestr = data;
    std::string::iterator temp = filestr.begin();
    appendText(temp);
    this->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    lastPosition = verticalScrollBar()->sliderPosition();

    fmDebug() << "Text preview: file data loaded";
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

void TextBrowserEdit::setSyntaxDefinition(const QString &filePath)
{
    // IMPORTANT: Delete old highlighter to ensure clean state
    // This prevents crashes from pending highlight events on old content
    if (m_highlighter) {
        delete m_highlighter;
        m_highlighter = nullptr;
        fmDebug() << "Text preview: deleted old syntax highlighter";
    }

    // Detect syntax definition based on file name
    auto definition = m_repository.definitionForFileName(filePath);

    if (definition.isValid()) {
        // Code file detected: create NEW highlighter for this file
        m_highlighter = new KSyntaxHighlighting::SyntaxHighlighter(document());

        // Set theme based on current system theme
        bool isDarkTheme = DGuiApplicationHelper::instance()->themeType()
                == DGuiApplicationHelper::DarkType;
        auto theme = m_repository.defaultTheme(isDarkTheme
                                                       ? KSyntaxHighlighting::Repository::DarkTheme
                                                       : KSyntaxHighlighting::Repository::LightTheme);
        m_highlighter->setTheme(theme);

        // Set syntax definition
        m_highlighter->setDefinition(definition);

        fmInfo() << "Text preview: created new syntax highlighter for" << definition.name()
                 << "with theme" << theme.name();
    } else {
        // Plain text file: no highlighter needed
        fmDebug() << "Text preview: no syntax definition found for" << filePath
                  << ", using plain text mode";
    }
}

void TextBrowserEdit::onThemeTypeChanged()
{
    updateHighlighterTheme();
}

void TextBrowserEdit::updateHighlighterTheme()
{
    if (!m_highlighter) {
        fmDebug() << "Text preview: no highlighter to update theme";
        return;
    }

    // Determine current system theme type
    bool isDarkTheme = DGuiApplicationHelper::instance()->themeType()
            == DGuiApplicationHelper::DarkType;

    // Get corresponding syntax highlighting theme
    auto theme = m_repository.defaultTheme(isDarkTheme
                                                   ? KSyntaxHighlighting::Repository::DarkTheme
                                                   : KSyntaxHighlighting::Repository::LightTheme);

    // Apply theme to highlighter
    m_highlighter->setTheme(theme);

    // Re-apply highlighting with new theme
    m_highlighter->rehighlight();

    fmInfo() << "Text preview: syntax highlighter theme updated to" << theme.name();
}
