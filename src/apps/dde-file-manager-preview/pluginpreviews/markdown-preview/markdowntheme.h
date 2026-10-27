// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MARKDOWNTHEME_H
#define MARKDOWNTHEME_H

#include "preview_plugin_global.h"

#include <QColor>
#include <QFont>
#include <QString>

namespace plugin_filepreview {

/*!
 * \brief Visual tokens used by the markdown preview.
 *
 * Every colour is derived from the current system palette, so a light/dark
 * switch is handled by simply re-reading the theme instead of shipping two
 * hand written style sheets.
 */
struct MarkdownTheme
{
    bool dark { false };

    QColor textColor;
    QColor headingColor;
    QColor mutedColor;
    QColor linkColor;
    QColor codeTextColor;
    QColor codeBackground;
    QColor inlineCodeBackground;
    QColor quoteTextColor;
    QColor quoteBarColor;
    QColor tableHeaderBackground;

    QString textFontFamily;
    QString monoFontFamily;
    qreal baseFontPointSize { 10.0 };

    int documentMargin { 24 };
    int contentWidth { 700 };   // widest image that fits into the preview viewport

    static MarkdownTheme fromApplication();

    qreal headingPointSize(int level) const;
    int headingTopMargin(int level) const;
    int headingBottomMargin(int level) const;
};

}   // namespace plugin_filepreview

#endif   // MARKDOWNTHEME_H
