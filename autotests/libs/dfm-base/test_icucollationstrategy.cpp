// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_icucollationstrategy.cpp
 * @brief IcuCollationStrategy 单元测试 —— 新排序核心策略的功能覆盖。
 *
 * 覆盖点：
 *   - 不可拷贝契约（编译期 static_assert）
 *   - sortKey 确定性 / 非空 / 排序一致性
 *   - compare 符号契约（<0 / 0 / >0）与 sortKey 序一致
 *   - 数值排序（UCOL_NUMERIC_COLLATION=ON：file2 < file10）
 *   - 大小写敏感（UCOL_DEFAULT_STRENGTH）
 *   - Unicode（中文）排序键
 *   - clearReorder 拉丁在前 vs 默认拼音重排（zh_CN 下行为差异）
 *   - 析构安全（多次构造/析构不泄漏/不崩溃）
 */

#include <gtest/gtest.h>

#include <dfm-base/utils/collation/icucollationstrategy.h>

#include <QByteArray>
#include <QLocale>
#include <QString>
#include <QStringList>
#include <algorithm>
#include <type_traits>

using namespace dfmbase;

namespace {
/// 当前系统 locale 是否为简体中文（决定 clearReorder 是否有可观测行为差异）
bool systemIsZhCn()
{
    const QLocale loc = QLocale::system();
    return loc.language() == QLocale::Chinese && loc.country() == QLocale::China;
}
}   // namespace

class IcuCollationStrategyTest : public testing::Test
{
protected:
    /// 默认策略（不清空重排），等价于原 QCollator 行为
    IcuCollationStrategy defaultStrategy;
    /// 拉丁在前策略（清空脚本重排）
    IcuCollationStrategy latinFirstStrategy { true };
};

// 不可拷贝：IcuCollationStrategy 持有 ICU 句柄，禁止拷贝（编译期校验）
TEST_F(IcuCollationStrategyTest, NonCopyableContract)
{
    static_assert(!std::is_copy_constructible<IcuCollationStrategy>::value,
                  "IcuCollationStrategy 应不可拷贝构造");
    static_assert(!std::is_copy_assignable<IcuCollationStrategy>::value,
                  "IcuCollationStrategy 应不可拷贝赋值");
    SUCCEED();
}

// sortKey 确定性：相同输入必须产出相同键
TEST_F(IcuCollationStrategyTest, SortKeyDeterministic)
{
    EXPECT_EQ(defaultStrategy.sortKey("hello"), defaultStrategy.sortKey("hello"));
    EXPECT_EQ(defaultStrategy.sortKey(""), defaultStrategy.sortKey(""));
    EXPECT_EQ(defaultStrategy.sortKey("文件.txt"), defaultStrategy.sortKey("文件.txt"));
}

// sortKey 非空：非空输入产出非空键
TEST_F(IcuCollationStrategyTest, SortKeyNonEmptyForNonEmptyInput)
{
    EXPECT_FALSE(defaultStrategy.sortKey("abc").isEmpty());
    EXPECT_FALSE(defaultStrategy.sortKey("测试文件").isEmpty());
}

// 不同输入产出不同键（区分明显不同的字符串）
TEST_F(IcuCollationStrategyTest, SortKeyDistinctForDistinctInput)
{
    EXPECT_NE(defaultStrategy.sortKey("apple"), defaultStrategy.sortKey("banana"));
    EXPECT_NE(defaultStrategy.sortKey("file1"), defaultStrategy.sortKey("file2"));
}

// sortKey 字母序一致：apple < banana < cherry
TEST_F(IcuCollationStrategyTest, SortKeyAlphabeticalOrdering)
{
    EXPECT_TRUE(defaultStrategy.sortKey("apple") < defaultStrategy.sortKey("banana"));
    EXPECT_TRUE(defaultStrategy.sortKey("banana") < defaultStrategy.sortKey("cherry"));
    // 逆序不成立
    EXPECT_FALSE(defaultStrategy.sortKey("cherry") < defaultStrategy.sortKey("apple"));
}

// compare 符号契约：<0 / 0 / >0
TEST_F(IcuCollationStrategyTest, CompareSignContract)
{
    EXPECT_LT(defaultStrategy.compare("a", "b"), 0);   // a 排在 b 前
    EXPECT_EQ(defaultStrategy.compare("same", "same"), 0);   // 相等
    EXPECT_GT(defaultStrategy.compare("b", "a"), 0);   // b 排在 a 后
}

// compare 与 sortKey 序一致：compare<0 当且仅当 sortKey 偏小
TEST_F(IcuCollationStrategyTest, CompareAgreesWithSortKey)
{
    const QString a = "apple";
    const QString b = "banana";
    EXPECT_LT(defaultStrategy.compare(a, b), 0);
    EXPECT_TRUE(defaultStrategy.sortKey(a) < defaultStrategy.sortKey(b));
    EXPECT_GT(defaultStrategy.compare(b, a), 0);
    EXPECT_FALSE(defaultStrategy.sortKey(b) < defaultStrategy.sortKey(a));
}

// 数值排序：UCOL_NUMERIC_COLLATION=ON 使 file2 < file10
TEST_F(IcuCollationStrategyTest, NumericCollationOrdering)
{
    EXPECT_LT(defaultStrategy.compare("file2", "file10"), 0);
    EXPECT_TRUE(defaultStrategy.sortKey("file2") < defaultStrategy.sortKey("file10"));

    // 完整列表排序验证
    QStringList names = { "file10", "file2", "file1", "file20" };
    std::sort(names.begin(), names.end(),
              [this](const QString &l, const QString &r) {
                  return defaultStrategy.sortKey(l) < defaultStrategy.sortKey(r);
              });
    EXPECT_EQ(names, QStringList({ "file1", "file2", "file10", "file20" }));
}

// 大小写敏感：UCOL_DEFAULT_STRENGTH 下 "A" 与 "a" 不相等
TEST_F(IcuCollationStrategyTest, CaseSensitiveDistinction)
{
    EXPECT_NE(defaultStrategy.compare("A", "a"), 0);
    EXPECT_NE(defaultStrategy.sortKey("A"), defaultStrategy.sortKey("a"));
}

// Unicode（中文）排序键有效
TEST_F(IcuCollationStrategyTest, ChineseStringSortKey)
{
    EXPECT_FALSE(defaultStrategy.sortKey("测试文件").isEmpty());
    EXPECT_EQ(defaultStrategy.sortKey("测试文件"), defaultStrategy.sortKey("测试文件"));
    EXPECT_NE(defaultStrategy.sortKey("文件A"), defaultStrategy.sortKey("文件B"));
}

// 两种策略均产出有效键（构造不崩溃、sortKey 可用）
TEST_F(IcuCollationStrategyTest, BothStrategiesProduceValidKeys)
{
    EXPECT_FALSE(defaultStrategy.sortKey("test").isEmpty());
    EXPECT_FALSE(latinFirstStrategy.sortKey("test").isEmpty());
    EXPECT_FALSE(defaultStrategy.sortKey("中文").isEmpty());
    EXPECT_FALSE(latinFirstStrategy.sortKey("中文").isEmpty());
}

// clearReorder 行为差异：zh_CN 下拉丁在前 vs 默认拼音重排
TEST_F(IcuCollationStrategyTest, ClearReorderBehaviorZhCn)
{
    // 非 zh_CN 环境下 clearReorder 为 no-op，仅验证不崩溃
    if (!systemIsZhCn()) {
        SUCCEED() << "非 zh_CN 环境，clearReorder 为 no-op，跳过排序差异断言";
        return;
    }

    // zh_CN 默认含 [reorder Hani]：中文字符按拼音与拉丁交错。
    //   "啊" 拼音 "a" → 在拼音序中排在 "zzz"（z）之前。
    // clearReorder=true 清空重排：拉丁脚本块整体排在中文之前 → "zzz" < "啊"。
    const QByteArray keyLatinDefault = defaultStrategy.sortKey("zzz");
    const QByteArray keyChineseDefault = defaultStrategy.sortKey("啊");
    const QByteArray keyLatinFirst = latinFirstStrategy.sortKey("zzz");
    const QByteArray keyChineseFirst = latinFirstStrategy.sortKey("啊");

    // 默认（拼音重排）："啊"(a) < "zzz"(z)
    EXPECT_TRUE(keyChineseDefault < keyLatinDefault)
        << "zh_CN 默认应按拼音交错，'啊'(a) 排在 'zzz'(z) 之前";

    // 拉丁在前：所有拉丁排在中文之前 → "zzz" < "啊"
    EXPECT_TRUE(keyLatinFirst < keyChineseFirst)
        << "clearReorder=true 应使拉丁排在中文之前，'zzz' < '啊'";

    // 两种策略对同一字符串可能产出不同键（重排规则不同）
    // 至少存在某些字符串使键不同（已由上面的交错 vs 分块差异隐含覆盖）
}

// clearReorder 完整列表排序：拉丁在前时拉丁整体先于中文
TEST_F(IcuCollationStrategyTest, ClearReorderFullListOrderingZhCn)
{
    if (!systemIsZhCn()) {
        SUCCEED() << "非 zh_CN 环境，跳过完整列表排序差异断言";
        return;
    }

    // 混合拉丁与中文名
    QStringList names = { "work", "apple", "文件", "备份" };

    // 拉丁在前排序：所有拉丁排在所有中文之前
    QStringList latinFirst = names;
    std::sort(latinFirst.begin(), latinFirst.end(),
              [this](const QString &l, const QString &r) {
                  return latinFirstStrategy.sortKey(l) < latinFirstStrategy.sortKey(r);
              });
    // 前两个应是拉丁（apple, work），后两个是中文
    EXPECT_TRUE(latinFirst.at(0) == "apple" || latinFirst.at(0) == "work");
    EXPECT_TRUE(latinFirst.at(1) == "apple" || latinFirst.at(1) == "work");
    EXPECT_TRUE(latinFirst.at(2) == "文件" || latinFirst.at(2) == "备份");
    EXPECT_TRUE(latinFirst.at(3) == "文件" || latinFirst.at(3) == "备份");

    // 默认（拼音重排）：中文与拉丁交错，至少有一个中文排在某个拉丁之前
    QStringList defaultOrder = names;
    std::sort(defaultOrder.begin(), defaultOrder.end(),
              [this](const QString &l, const QString &r) {
                  return defaultStrategy.sortKey(l) < defaultStrategy.sortKey(r);
              });
    bool chineseBeforeLatin = false;
    for (int i = 0; i < defaultOrder.size(); ++i) {
        bool iIsChinese = (defaultOrder[i] == "文件" || defaultOrder[i] == "备份");
        for (int j = i + 1; j < defaultOrder.size(); ++j) {
            bool jIsLatin = (defaultOrder[j] == "apple" || defaultOrder[j] == "work");
            if (iIsChinese && jIsLatin)
                chineseBeforeLatin = true;
        }
    }
    EXPECT_TRUE(chineseBeforeLatin)
        << "默认拼音重排应使中文（按拼音）与拉丁交错，存在中文排在拉丁之前的情况";
}

// 析构安全：多次构造/析构不崩溃（验证 ucol_close 正确释放）
TEST_F(IcuCollationStrategyTest, RepeatedConstructionDestructionSafe)
{
    for (int i = 0; i < 100; ++i) {
        IcuCollationStrategy s(false);
        EXPECT_FALSE(s.sortKey("test").isEmpty());
        IcuCollationStrategy s2(true);
        EXPECT_FALSE(s2.sortKey("test").isEmpty());
    }
    SUCCEED();
}

// 长字符串与特殊字符不崩溃
TEST_F(IcuCollationStrategyTest, LongAndSpecialStringsSafe)
{
    const QString longName(10000, 'a');
    EXPECT_NO_FATAL_FAILURE({ (void)defaultStrategy.sortKey(longName); });
    EXPECT_NO_FATAL_FAILURE({ (void)defaultStrategy.sortKey("!@#$%^&*()_+-="); });
    EXPECT_NO_FATAL_FAILURE({ (void)defaultStrategy.compare(longName, "a"); });
    EXPECT_FALSE(defaultStrategy.sortKey(longName).isEmpty());
}
