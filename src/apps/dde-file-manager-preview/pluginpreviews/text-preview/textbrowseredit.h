// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H
#include "preview_plugin_global.h"

#include <QPlainTextEdit>
#include <QPointer>

#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/SyntaxHighlighter>

#include <vector>

namespace plugin_filepreview {
class TextBrowserEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit TextBrowserEdit(QWidget *parent = nullptr);

    virtual ~TextBrowserEdit() override;

    void setFileData(std::string &data);

    /**
     * @brief Set syntax highlighting based on file path
     * @param filePath File path used to detect syntax definition
     *
     * This method will automatically detect if the file is a code file
     * and enable syntax highlighting if applicable. Falls back to plain
     * text if no syntax definition is found.
     */
    void setSyntaxDefinition(const QString &filePath);

protected:
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void scrollbarValueChange(int value);

    void sliderPositionValueChange(int position);

    void onThemeTypeChanged();

private:
    int verifyEndOfStrIntegrity(const char *s, int l);

    void appendText(std::string::iterator &data);

    /**
     * @brief Update syntax highlighter theme based on current system theme
     *
     * This method is called when system theme changes (light/dark mode).
     * It's also reused by setSyntaxDefinition() for initial theme setup.
     */
    void updateHighlighterTheme();

    std::string filestr;

    int lastPosition { 0 };

    // Syntax highlighting support
    KSyntaxHighlighting::Repository m_repository;
    QPointer<KSyntaxHighlighting::SyntaxHighlighter> m_highlighter;
};
}
#endif   // TEXTBROWSER_H
