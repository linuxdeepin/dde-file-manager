// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdowntheme.h"
#include "stubext.h"

#include <DGuiApplicationHelper>
#include <DPalette>

#include <gtest/gtest.h>

#include <QApplication>
#include <QColor>
#include <QFont>

DGUI_USE_NAMESPACE

using namespace plugin_filepreview;

namespace {

// Kept in sync with the private mixColor() helper: the exact colour values
// are part of the visible contract, not just an implementation detail.
QColor mixed(const QColor &from, const QColor &to, qreal ratio)
{
    const qreal weight = qBound(0.0, ratio, 1.0);
    return QColor::fromRgbF(from.redF() * (1.0 - weight) + to.redF() * weight,
                            from.greenF() * (1.0 - weight) + to.greenF() * weight,
                            from.blueF() * (1.0 - weight) + to.blueF() * weight);
}

DPalette paletteWith(const QColor &base, const QColor &text, const QColor &title,
                     const QColor &tips, const QColor &link, const QColor &highlight)
{
    DPalette palette;
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::Text, text);
    palette.setColor(DPalette::TextTitle, title);
    palette.setColor(DPalette::TextTips, tips);
    palette.setColor(QPalette::Link, link);
    palette.setColor(QPalette::Highlight, highlight);
    return palette;
}

void stubHelper(stub_ext::StubExt &stub, DGuiApplicationHelper::ColorType type, const DPalette &palette)
{
    stub.set_lamda(&DGuiApplicationHelper::themeType, [type] {
        __DBG_STUB_INVOKE__
        return type;
    });

    using PaletteGetter = DPalette (DGuiApplicationHelper::*)() const;
    stub.set_lamda(static_cast<PaletteGetter>(&DGuiApplicationHelper::applicationPalette),
                   [palette] {
                       __DBG_STUB_INVOKE__
                       return palette;
                   });
}

}   // namespace

TEST(UT_MarkdownTheme, DefaultConstructed_ExposesDocumentMetrics)
{
    const MarkdownTheme theme;

    EXPECT_FALSE(theme.dark);
    EXPECT_DOUBLE_EQ(theme.baseFontPointSize, 10.0);
    EXPECT_EQ(theme.documentMargin, 24);
    EXPECT_EQ(theme.contentWidth, 700);
    EXPECT_FALSE(theme.textColor.isValid());
    EXPECT_TRUE(theme.textFontFamily.isEmpty());
    EXPECT_TRUE(theme.monoFontFamily.isEmpty());
}

TEST(UT_MarkdownTheme, HeadingPointSize_AllLevels_FollowScaleTable)
{
    MarkdownTheme theme;
    theme.baseFontPointSize = 10.0;

    EXPECT_DOUBLE_EQ(theme.headingPointSize(1), 19.0);
    EXPECT_DOUBLE_EQ(theme.headingPointSize(2), 15.0);
    EXPECT_DOUBLE_EQ(theme.headingPointSize(3), 12.8);
    EXPECT_DOUBLE_EQ(theme.headingPointSize(4), 11.2);
    EXPECT_DOUBLE_EQ(theme.headingPointSize(5), 10.0);
    EXPECT_DOUBLE_EQ(theme.headingPointSize(6), 9.4);
}

TEST(UT_MarkdownTheme, HeadingPointSize_OutOfRangeLevels_ClampedToFirstAndLast)
{
    MarkdownTheme theme;
    theme.baseFontPointSize = 20.0;

    EXPECT_DOUBLE_EQ(theme.headingPointSize(0), 38.0);
    EXPECT_DOUBLE_EQ(theme.headingPointSize(-5), 38.0);
    EXPECT_DOUBLE_EQ(theme.headingPointSize(7), 18.8);
    EXPECT_DOUBLE_EQ(theme.headingPointSize(99), 18.8);
}

TEST(UT_MarkdownTheme, HeadingTopMargin_PerLevel_MatchesLayoutTable)
{
    const MarkdownTheme theme;

    EXPECT_EQ(theme.headingTopMargin(1), 6);
    EXPECT_EQ(theme.headingTopMargin(2), 18);
    EXPECT_EQ(theme.headingTopMargin(3), 16);
    EXPECT_EQ(theme.headingTopMargin(4), 12);
    EXPECT_EQ(theme.headingTopMargin(5), 12);
    EXPECT_EQ(theme.headingTopMargin(6), 12);
    EXPECT_EQ(theme.headingTopMargin(0), 6);
    EXPECT_EQ(theme.headingTopMargin(7), 12);
}

TEST(UT_MarkdownTheme, HeadingBottomMargin_PerLevel_MatchesLayoutTable)
{
    const MarkdownTheme theme;

    EXPECT_EQ(theme.headingBottomMargin(1), 10);
    EXPECT_EQ(theme.headingBottomMargin(2), 8);
    EXPECT_EQ(theme.headingBottomMargin(3), 6);
    EXPECT_EQ(theme.headingBottomMargin(6), 6);
    EXPECT_EQ(theme.headingBottomMargin(-1), 10);
    EXPECT_EQ(theme.headingBottomMargin(42), 6);
}

TEST(UT_MarkdownTheme, FromApplication_RealPalette_ProducesUsableTokens)
{
    const MarkdownTheme theme = MarkdownTheme::fromApplication();
    const DGuiApplicationHelper::ColorType type = DGuiApplicationHelper::instance()->themeType();

    EXPECT_EQ(theme.dark, type == DGuiApplicationHelper::DarkType);
    EXPECT_TRUE(theme.textColor.isValid());
    EXPECT_TRUE(theme.headingColor.isValid());
    EXPECT_TRUE(theme.mutedColor.isValid());
    EXPECT_TRUE(theme.linkColor.isValid());
    EXPECT_TRUE(theme.codeTextColor.isValid());
    EXPECT_TRUE(theme.codeBackground.isValid());
    EXPECT_TRUE(theme.inlineCodeBackground.isValid());
    EXPECT_TRUE(theme.quoteTextColor.isValid());
    EXPECT_TRUE(theme.quoteBarColor.isValid());
    EXPECT_TRUE(theme.tableHeaderBackground.isValid());

    EXPECT_FALSE(theme.textFontFamily.isEmpty());
    EXPECT_FALSE(theme.monoFontFamily.isEmpty());
    EXPECT_GT(theme.baseFontPointSize, 0.0);

    EXPECT_EQ(theme.documentMargin, 24);
    EXPECT_EQ(theme.contentWidth, 800 - 2 * theme.documentMargin - 16);
}

TEST(UT_MarkdownTheme, FromApplication_LightPalette_MapsRolesAndDarkensSurfaces)
{
    stub_ext::StubExt stub;
    const QColor base("#fafafa");
    const QColor text("#202020");
    const QColor title("#101010");
    const QColor tips("#808080");
    const QColor link("#0078d7");
    const QColor highlight("#ff00ff");
    stubHelper(stub, DGuiApplicationHelper::LightType,
               paletteWith(base, text, title, tips, link, highlight));

    const MarkdownTheme theme = MarkdownTheme::fromApplication();

    EXPECT_FALSE(theme.dark);
    EXPECT_EQ(theme.textColor, text);
    EXPECT_EQ(theme.headingColor, title);
    EXPECT_EQ(theme.mutedColor, tips);
    // QPalette::Link wins over QPalette::Highlight when it is valid.
    EXPECT_EQ(theme.linkColor, link);

    const QColor codeBackground = base.darker(104);
    EXPECT_EQ(theme.codeBackground, codeBackground);
    EXPECT_EQ(theme.inlineCodeBackground, base.darker(105));
    EXPECT_EQ(theme.tableHeaderBackground, base.darker(109));
    EXPECT_EQ(theme.codeTextColor, mixed(text, codeBackground, 0.15));
    EXPECT_EQ(theme.quoteTextColor, mixed(text, base, 0.25));
    EXPECT_EQ(theme.quoteBarColor, mixed(text, base, 0.78));
}

TEST(UT_MarkdownTheme, FromApplication_DarkPalette_LightensSurfaces)
{
    stub_ext::StubExt stub;
    const QColor base("#202020");
    const QColor text("#f0f0f0");
    const QColor title("#ffffff");
    const QColor tips("#a0a0a0");
    const QColor link("#00aaff");
    const QColor highlight("#336699");
    stubHelper(stub, DGuiApplicationHelper::DarkType,
               paletteWith(base, text, title, tips, link, highlight));

    const MarkdownTheme theme = MarkdownTheme::fromApplication();

    EXPECT_TRUE(theme.dark);
    EXPECT_EQ(theme.textColor, text);
    EXPECT_EQ(theme.headingColor, title);
    EXPECT_EQ(theme.mutedColor, tips);
    EXPECT_EQ(theme.linkColor, link);

    const QColor codeBackground = base.lighter(118);
    EXPECT_EQ(theme.codeBackground, codeBackground);
    EXPECT_EQ(theme.inlineCodeBackground, base.lighter(130));
    EXPECT_EQ(theme.tableHeaderBackground, base.lighter(138));
    EXPECT_EQ(theme.codeTextColor, mixed(text, codeBackground, 0.15));
    EXPECT_EQ(theme.quoteTextColor, mixed(text, base, 0.25));
    EXPECT_EQ(theme.quoteBarColor, mixed(text, base, 0.78));
}

TEST(UT_MarkdownTheme, FromApplication_InvalidLightPalette_UsesLightFallbacks)
{
    stub_ext::StubExt stub;
    DPalette empty;
    empty.setColor(QPalette::Base, QColor());
    empty.setColor(QPalette::Text, QColor());
    empty.setColor(DPalette::TextTitle, QColor());
    empty.setColor(DPalette::TextTips, QColor());
    empty.setColor(QPalette::Link, QColor());
    empty.setColor(QPalette::Highlight, QColor());
    stubHelper(stub, DGuiApplicationHelper::LightType, empty);

    const MarkdownTheme theme = MarkdownTheme::fromApplication();

    EXPECT_FALSE(theme.dark);
    EXPECT_EQ(theme.textColor, QColor(0x1f, 0x23, 0x28));
    EXPECT_EQ(theme.headingColor, theme.textColor);
    EXPECT_TRUE(theme.mutedColor.isValid());
    EXPECT_EQ(theme.linkColor, QColor(0x00, 0x63, 0xd6));
    EXPECT_TRUE(theme.codeBackground.isValid());
    EXPECT_TRUE(theme.quoteBarColor.isValid());
}

TEST(UT_MarkdownTheme, FromApplication_InvalidDarkPalette_UsesDarkFallbacks)
{
    stub_ext::StubExt stub;
    DPalette empty;
    empty.setColor(QPalette::Base, QColor());
    empty.setColor(QPalette::Text, QColor());
    empty.setColor(DPalette::TextTitle, QColor());
    empty.setColor(DPalette::TextTips, QColor());
    empty.setColor(QPalette::Link, QColor());
    empty.setColor(QPalette::Highlight, QColor());
    stubHelper(stub, DGuiApplicationHelper::DarkType, empty);

    const MarkdownTheme theme = MarkdownTheme::fromApplication();

    EXPECT_TRUE(theme.dark);
    EXPECT_EQ(theme.textColor, QColor(0xe6, 0xed, 0xf3));
    EXPECT_EQ(theme.headingColor, theme.textColor);
    EXPECT_TRUE(theme.mutedColor.isValid());
    EXPECT_EQ(theme.linkColor, QColor(0x00, 0x63, 0xd6));
    EXPECT_TRUE(theme.codeBackground.isValid());
}

TEST(UT_MarkdownTheme, FromApplication_InvalidLinkFallsBackToHighlight)
{
    stub_ext::StubExt stub;
    const QColor highlight("#123456");
    stubHelper(stub, DGuiApplicationHelper::LightType,
               paletteWith(QColor("#ffffff"), QColor("#000000"), QColor("#000000"),
                           QColor("#777777"), QColor(), highlight));

    const MarkdownTheme theme = MarkdownTheme::fromApplication();

    EXPECT_EQ(theme.linkColor, highlight);
}

TEST(UT_MarkdownTheme, FromApplication_Fonts_ComeFromApplicationAndMonoCandidates)
{
    stub_ext::StubExt stub;
    stubHelper(stub, DGuiApplicationHelper::LightType,
               paletteWith(QColor("#ffffff"), QColor("#000000"), QColor("#000000"),
                           QColor("#777777"), QColor("#0000ff"), QColor("#00ff00")));

    const MarkdownTheme theme = MarkdownTheme::fromApplication();

    EXPECT_EQ(theme.textFontFamily, QApplication::font().family());
    EXPECT_GT(theme.baseFontPointSize, 0.0);
    EXPECT_FALSE(theme.monoFontFamily.isEmpty());
}
