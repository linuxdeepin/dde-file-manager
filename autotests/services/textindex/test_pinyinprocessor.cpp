// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pinyinprocessor.cpp
 * @brief Unit tests for PinyinProcessor (pinyinprocessor.cpp)
 *        Covers dictionary loading (anything-format U+xxxx: pinyin[,pinyin] # hanzi),
 *         tone removal (ā→a, ǖ→v), and pinyin/acronym conversion (non-dict chars
 *         passed through, output lowercased — matching deepin-anything behavior).
 *
 * Uses a self-contained temp dictionary to avoid dependency on the installed
 * /usr/share/dde-file-manager/pinyin.txt (which may not be present in the
 * build/test environment). loadDictionary() clears the map before loading, so
 * re-loading the global singleton with a test dict gives a deterministic state.
 */

#include <gtest/gtest.h>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QFile>
#include <QString>
#include <QTextStream>

#include "services/textindex/utils/pinyinprocessor.h"

#include "stubext.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

namespace {

// Build a small but representative dictionary in the anything format.
// Returns the path to the temp dict file (caller owns f's lifetime).
void writeTempDict(QTemporaryFile &f)
{
    ASSERT_TRUE(f.open());
    QTextStream s(&f);
    s << "# test pinyin dict\n";
    s << "U+4E2D: zhong, zhong  # 中\n";   // 中 → zhōng/zhòng → first "zhong"
    s << "U+6587: wen  # 文\n";
    s << "U+4EF6: jian  # 件\n";
    s << "U+540D: ming  # 名\n";
    s << "U+7D22: suo  # 索\n";
    s << "U+5F15: yin  # 引\n";
    s << "U+641C: sou  # 搜\n";
    s << "U+7247: pian  # 片\n";
    s << "U+56FE: tu  # 图\n";
    s << "U+97F3: yin  # 音 (duplicate pinyin yin, different char)\n";
    // Polyphonic char: 行 → xíng/háng → first "xing"
    s << "U+884C: xing, hang  # 行\n";
    // Toned pinyin: lǜ → lü → remove_tone → "lv"
    s << "U+7EFF: lü  # 绿\n";
    s << "# trailing comment\n";
    s.flush();
    f.close();
}

// Helper: reload the global singleton with a temp dict and return it loaded.
// Each test gets a fresh map (loadDictionary clears before loading).
bool reloadWithTempDict(QTemporaryFile &f)
{
    writeTempDict(f);
    return PinyinProcessor::instance().loadDictionary(f.fileName());
}

}   // namespace

TEST(PinyinProcessorTest, LoadDictionary_ValidFormat_LoadsEntries)
{
    QTemporaryFile f;
    ASSERT_TRUE(reloadWithTempDict(f));
    EXPECT_TRUE(PinyinProcessor::instance().isLoaded());
}

TEST(PinyinProcessorTest, LoadDictionary_NonExistent_ReturnsFalse)
{
    EXPECT_FALSE(PinyinProcessor::instance().loadDictionary("/nonexistent/pinyin/dict.txt"));
}

TEST(PinyinProcessorTest, DictionaryLookupPaths_UserDirFirstThenSystem)
{
    // XDG user directory must come first (user/OEM override), then the CMake-injected
    // system install path. Stub QStandardPaths to avoid depending on the test env.
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(QStandardPaths, writableLocation),
                   [](QStandardPaths::StandardLocation) -> QString {
                       __DBG_STUB_INVOKE__
                       return "/tmp/fake-xdg-data";
                   });

    const QStringList paths = PinyinProcessor::dictionaryLookupPaths();
    ASSERT_EQ(paths.size(), 2);
    EXPECT_EQ(paths.first(), QString("/tmp/fake-xdg-data/deepin/dde-file-manager/pinyin.txt"));
    EXPECT_TRUE(paths.last().endsWith("deepin/dde-file-manager/pinyin.txt"))
        << "system fallback path: " << paths.last().toStdString();
}

TEST(PinyinProcessorTest, LoadDictionary_EmptyFile_NotLoaded)
{
    QTemporaryFile f;
    ASSERT_TRUE(f.open());
    f.write("# only comments\n");
    f.close();
    EXPECT_FALSE(PinyinProcessor::instance().loadDictionary(f.fileName()));
    EXPECT_FALSE(PinyinProcessor::instance().isLoaded());
}

TEST(PinyinProcessorTest, LoadDictionary_CommentAndBlankLinesSkipped)
{
    QTemporaryFile f;
    ASSERT_TRUE(f.open());
    QTextStream s(&f);
    s << "\n# comment\n\nU+4E2D: zhong  # 中\n\n";
    s.flush();
    f.close();
    ASSERT_TRUE(PinyinProcessor::instance().loadDictionary(f.fileName()));

    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("中"), full, acro);
    EXPECT_EQ(full, QString("zhong"));
}

TEST(PinyinProcessorTest, ConvertToPinyin_PureChinese_FullAndAcronym)
{
    QTemporaryFile f;
    ASSERT_TRUE(reloadWithTempDict(f));

    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("文件名"), full, acro);
    // 文件名 → wenjianming / wjm
    EXPECT_EQ(full, QString("wenjianming"));
    EXPECT_EQ(acro, QString("wjm"));
}

TEST(PinyinProcessorTest, ConvertToPinyin_Polyphonic_UsesFirstPinyin)
{
    QTemporaryFile f;
    ASSERT_TRUE(reloadWithTempDict(f));

    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("中"), full, acro);
    // 中 is polyphonic (zhong, zhong); first entry wins → "zhong"
    EXPECT_EQ(full, QString("zhong"));
    EXPECT_EQ(acro, QString("z"));

    PinyinProcessor::instance().convertToPinyin(QStringLiteral("行"), full, acro);
    // 行 is polyphonic (xing, hang); first entry wins → "xing"
    EXPECT_EQ(full, QString("xing"));
    EXPECT_EQ(acro, QString("x"));
}

TEST(PinyinProcessorTest, ConvertToPinyin_TonedPinyin_ToneRemoved)
{
    QTemporaryFile f;
    ASSERT_TRUE(reloadWithTempDict(f));

    // 绿 → lü → remove_tone → "lv" (anything maps ü→v)
    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("绿"), full, acro);
    EXPECT_EQ(full, QString("lv"));
    EXPECT_EQ(acro, QString("l"));
}

TEST(PinyinProcessorTest, ConvertToPinyin_NonDictChars_PassedThrough)
{
    QTemporaryFile f;
    ASSERT_TRUE(reloadWithTempDict(f));

    // English/digits/symbols not in dict should be passed through unchanged
    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("abc123-文件"), full, acro);
    // 中文部分→ wenjian, 英文原样拼入 → "abc123-wenjian" (lowercased)
    EXPECT_EQ(full, QString("abc123-wenjian"));
    EXPECT_EQ(acro, QString("abc123-wj"));
}

TEST(PinyinProcessorTest, ConvertToPinyin_EmptyInput_ReturnsEmpty)
{
    QTemporaryFile f;
    ASSERT_TRUE(reloadWithTempDict(f));

    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QString(), full, acro);
    EXPECT_TRUE(full.isEmpty());
    EXPECT_TRUE(acro.isEmpty());
}

TEST(PinyinProcessorTest, ConvertToPinyin_NotLoaded_ReturnsEmpty)
{
    // Force unloaded state by loading an empty dict
    QTemporaryFile f;
    ASSERT_TRUE(f.open());
    f.write("# empty\n");
    f.close();
    ASSERT_FALSE(PinyinProcessor::instance().loadDictionary(f.fileName()));
    ASSERT_FALSE(PinyinProcessor::instance().isLoaded());

    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("文件"), full, acro);
    EXPECT_TRUE(full.isEmpty());
    EXPECT_TRUE(acro.isEmpty());
}

TEST(PinyinProcessorTest, ConvertToPinyin_OutputIsLowercased)
{
    QTemporaryFile f;
    ASSERT_TRUE(reloadWithTempDict(f));

    // Dict already lowercase, but verify the output path lowercases (anything toLower)
    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("ABC文件"), full, acro);
    EXPECT_EQ(full, QString("abcwenjian"));
    EXPECT_EQ(acro, QString("abcwj"));
}

TEST(PinyinProcessorTest, ConvertToPinyin_LongChinesePhrase)
{
    QTemporaryFile f;
    ASSERT_TRUE(reloadWithTempDict(f));

    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("文件名搜索"), full, acro);
    // 文件名搜索 → wenjianmingsousuo / wjms + ss = wjmss
    EXPECT_EQ(full, QString("wenjianmingsousuo"));
    EXPECT_EQ(acro, QString("wjmss"));
}

TEST(PinyinProcessorTest, InstanceSingleton_CallableAndDoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE({ (void)PinyinProcessor::instance().isLoaded(); });
    QString full, acro;
    EXPECT_NO_FATAL_FAILURE({ PinyinProcessor::instance().convertToPinyin(QStringLiteral("test"), full, acro); });
}

TEST(PinyinProcessorTest, LoadDictionary_ReloadClearsPreviousEntries)
{
    // Load full test dict, then reload with a single-entry dict: previous chars gone.
    QTemporaryFile f1;
    ASSERT_TRUE(reloadWithTempDict(f1));
    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("文件"), full, acro);
    EXPECT_EQ(full, QString("wenjian"));

    QTemporaryFile f2;
    ASSERT_TRUE(f2.open());
    QTextStream s(&f2);
    s << "U+4E2D: zhong  # 中\n";   // only 中
    s.flush();
    f2.close();
    ASSERT_TRUE(PinyinProcessor::instance().loadDictionary(f2.fileName()));

    // 文 is no longer in the dict → passed through unchanged (original chars kept)
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("文件"), full, acro);
    // Neither 文 nor 件 in the single-entry dict → both passed through as-is
    EXPECT_EQ(full, QString("文件"));
    EXPECT_EQ(acro, QString("文件"));
}

TEST(PinyinProcessorTest, SupplementaryPlaneEntries_LoadAndConvert)
{
    // CJK Extension B chars (U+20000+) are surrogate pairs in UTF-16 — the dict key
    // must handle them without tripping QChar's <=0xFFFF assert (regression: the
    // XDG user dictionary contains 15k+ extension-B entries).
    QTemporaryFile f;
    ASSERT_TRUE(f.open());
    QTextStream s(&f);
    s << "U+20000: he  # \xF0\xA0\x80\x80\n";    // U+20000 𠀀
    s << "U+2A700: qi  # \xF0\xAA\x9C\x80\n";    // U+2A700 (needs 3-hex-digit... still 21-bit)
    s << "U+4E2D: zhong  # 中\n";
    s.flush();
    f.close();
    ASSERT_TRUE(PinyinProcessor::instance().loadDictionary(f.fileName()));

    // Extension-B char gets its pinyin; BMP neighbor unchanged
    const QString name = QString::fromUtf8("\xF0\xA0\x80\x80") + QStringLiteral("中");
    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(name, full, acro);
    EXPECT_EQ(full, QString("hezhong"));
    EXPECT_EQ(acro, QString("hz"));
}

TEST(PinyinProcessorTest, SupplementaryPlane_NotInDict_PassedThrough)
{
    QTemporaryFile f;
    ASSERT_TRUE(f.open());
    QTextStream s(&f);
    s << "U+4E2D: zhong  # 中\n";
    s.flush();
    f.close();
    ASSERT_TRUE(PinyinProcessor::instance().loadDictionary(f.fileName()));

    // Extension-B char not in dict → passed through as the original character
    const QString extB = QString::fromUtf8("\xF0\xA0\x80\x80");   // U+20000
    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(extB + QStringLiteral("中"), full, acro);
    EXPECT_EQ(full, extB + QString("zhong"));
    EXPECT_EQ(acro, extB + QString("z"));
}

TEST(PinyinProcessorTest, LoadDictionary_CodePointOutOfRange_Skipped)
{
    // Malformed/over-range codepoints (U+110000+) must be skipped, not crash
    QTemporaryFile f;
    ASSERT_TRUE(f.open());
    QTextStream s(&f);
    s << "U+110000: bad  # out of Unicode range\n";
    s << "U+4E2D: zhong  # 中\n";
    s.flush();
    f.close();
    ASSERT_TRUE(PinyinProcessor::instance().loadDictionary(f.fileName()));

    QString full, acro;
    PinyinProcessor::instance().convertToPinyin(QStringLiteral("中"), full, acro);
    EXPECT_EQ(full, QString("zhong"));
}
