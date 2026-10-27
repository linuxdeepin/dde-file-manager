// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdowntheme.h"

#include <DGuiApplicationHelper>
#include <DPalette>

#include <QApplication>
#include <QFontDatabase>
#include <QStringList>

DGUI_USE_NAMESPACE

namespace plugin_filepreview {
namespace {

QColor mixColor(const QColor &from, const QColor &to, qreal ratio)
{
    const qreal weight = qBound(0.0, ratio, 1.0);
    return QColor::fromRgbF(from.redF() * (1.0 - weight) + to.redF() * weight,
                            from.greenF() * (1.0 - weight) + to.greenF() * weight,
                            from.blueF() * (1.0 - weight) + to.blueF() * weight);
}

QString preferredMonoFamily()
{
    static const QStringList candidates {
        QStringLiteral("Noto Sans Mono"),
        QStringLiteral("JetBrains Mono"),
        QStringLiteral("Source Code Pro"),
        QStringLiteral("DejaVu Sans Mono"),
        QStringLiteral("Noto Mono"),
        QStringLiteral("Liberation Mono"),
        QStringLiteral("Ubuntu Mono"),
        QStringLiteral("Courier New")
    };

    const QStringList families = QFontDatabase::families();
    for (const QString &candidate : candidates) {
        if (families.contains(candidate))
            return candidate;
    }

    return QStringLiteral("monospace");
}

}   // namespace

MarkdownTheme MarkdownTheme::fromApplication()
{
    MarkdownTheme theme;

    auto *helper = DGuiApplicationHelper::instance();
    theme.dark = helper && helper->themeType() == DGuiApplicationHelper::DarkType;

    // Keep the DTK palette type: it adds the extra ColorType roles on top of
    // the Qt ones.
    const DPalette palette = helper ? helper->applicationPalette() : DPalette();

    QColor baseColor = palette.color(QPalette::Base);
    if (!baseColor.isValid())
        baseColor = theme.dark ? QColor(0x1e, 0x1e, 0x1e) : QColor(0xff, 0xff, 0xff);

    theme.textColor = palette.color(QPalette::Text);
    if (!theme.textColor.isValid())
        theme.textColor = theme.dark ? QColor(0xe6, 0xed, 0xf3) : QColor(0x1f, 0x23, 0x28);

    const QColor titleColor = palette.color(DPalette::TextTitle);
    theme.headingColor = titleColor.isValid() ? titleColor : theme.textColor;

    const QColor tipsColor = palette.color(DPalette::TextTips);
    theme.mutedColor = tipsColor.isValid() ? tipsColor : mixColor(theme.textColor, baseColor, 0.45);

    const QColor linkColor = palette.color(QPalette::Link);
    theme.linkColor = linkColor.isValid() ? linkColor : palette.color(QPalette::Highlight);
    if (!theme.linkColor.isValid())
        theme.linkColor = QColor(0x00, 0x63, 0xd6);

    // Keep the code surface one step away from the page so blocks read as
    // separate cards without introducing a second palette.
    theme.codeBackground = theme.dark ? baseColor.lighter(118) : baseColor.darker(104);
    theme.inlineCodeBackground = theme.dark ? baseColor.lighter(130) : baseColor.darker(105);
    theme.codeTextColor = mixColor(theme.textColor, theme.codeBackground, 0.15);
    theme.quoteTextColor = mixColor(theme.textColor, baseColor, 0.25);
    theme.quoteBarColor = mixColor(theme.textColor, baseColor, 0.78);
    // A table header must not reuse the code surface colour: the renderer uses
    // that colour to locate fenced code blocks in the imported document.
    theme.tableHeaderBackground = theme.dark ? baseColor.lighter(138) : baseColor.darker(109);

    const QFont applicationFont = QApplication::font();
    theme.textFontFamily = applicationFont.family();
    if (theme.textFontFamily.isEmpty())
        theme.textFontFamily = QStringLiteral("Noto Sans");

    const qreal pointSize = applicationFont.pointSizeF();
    theme.baseFontPointSize = pointSize > 0 ? pointSize : 10.0;
    theme.monoFontFamily = preferredMonoFamily();

    // Leave room for the vertical scroll bar plus both document margins.
    theme.contentWidth = 800 - 2 * theme.documentMargin - 16;

    return theme;
}

qreal MarkdownTheme::headingPointSize(int level) const
{
    static const qreal scales[] = { 1.9, 1.5, 1.28, 1.12, 1.0, 0.94 };
    const int index = qBound(1, level, 6) - 1;
    return baseFontPointSize * scales[index];
}

int MarkdownTheme::headingTopMargin(int level) const
{
    // Headings need more air above than below, and consecutive sections
    // should not drift apart.
    switch (qBound(1, level, 6)) {
    case 1:
        return 6;
    case 2:
        return 18;
    case 3:
        return 16;
    default:
        return 12;
    }
}

int MarkdownTheme::headingBottomMargin(int level) const
{
    switch (qBound(1, level, 6)) {
    case 1:
        return 10;
    case 2:
        return 8;
    default:
        return 6;
    }
}

}   // namespace plugin_filepreview
