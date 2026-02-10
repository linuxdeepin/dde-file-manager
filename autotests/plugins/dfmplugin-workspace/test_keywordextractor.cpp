// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/keywordextractor.h"
#include <dfm-base/base/application/application.h>

#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

using namespace dfmplugin_workspace;

class KeywordExtractorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// ============== SimpleKeywordStrategy Tests ==============

class SimpleKeywordStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        strategy = new SimpleKeywordStrategy();
    }

    void TearDown() override
    {
        delete strategy;
        stub.clear();
    }

    SimpleKeywordStrategy *strategy = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SimpleKeywordStrategyTest, ExtractKeywords_ValidKeyword_ReturnsKeyword)
{
    // Test extracting keywords from a simple keyword
    QString testKeyword = "test";
    QStringList result = strategy->extractKeywords(testKeyword);
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first(), testKeyword);
}

TEST_F(SimpleKeywordStrategyTest, ExtractKeywords_EmptyKeyword_ReturnsEmpty)
{
    // Test extracting keywords from empty string
    QString emptyKeyword = "";
    QStringList result = strategy->extractKeywords(emptyKeyword);
    
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(SimpleKeywordStrategyTest, ExtractKeywords_ComplexKeyword_ReturnsOriginal)
{
    // Test that complex keywords are returned as-is
    QString complexKeyword = "complex_keyword_123";
    QStringList result = strategy->extractKeywords(complexKeyword);
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first(), complexKeyword);
}

TEST_F(SimpleKeywordStrategyTest, CanHandle_AnyKeyword_ReturnsTrue)
{
    // Test that SimpleKeywordStrategy can handle any keyword (fallback)
    EXPECT_TRUE(strategy->canHandle("simple"));
    EXPECT_TRUE(strategy->canHandle("complex keyword"));
    EXPECT_TRUE(strategy->canHandle("*.txt"));
    EXPECT_TRUE(strategy->canHandle(""));
}

TEST_F(SimpleKeywordStrategyTest, Priority_ReturnsLowestPriority)
{
    // Test that SimpleKeywordStrategy has the lowest priority
    EXPECT_EQ(strategy->priority(), 100);
}

// ============== BooleanKeywordStrategy Tests ==============

class BooleanKeywordStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        strategy = new BooleanKeywordStrategy();
    }

    void TearDown() override
    {
        delete strategy;
        stub.clear();
    }

    BooleanKeywordStrategy *strategy = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BooleanKeywordStrategyTest, ExtractKeywords_MultipleWords_ReturnsSeparateKeywords)
{
    // Test extracting keywords from multiple words separated by spaces
    QString multiKeyword = "hello world test";
    QStringList result = strategy->extractKeywords(multiKeyword);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_TRUE(result.contains("hello"));
    EXPECT_TRUE(result.contains("world"));
    EXPECT_TRUE(result.contains("test"));
}

TEST_F(BooleanKeywordStrategyTest, ExtractKeywords_TabSeparated_ReturnsSeparateKeywords)
{
    // Test extracting keywords from tab-separated words
    QString tabKeyword = "hello\tworld\ttest";
    QStringList result = strategy->extractKeywords(tabKeyword);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_TRUE(result.contains("hello"));
    EXPECT_TRUE(result.contains("world"));
    EXPECT_TRUE(result.contains("test"));
}

TEST_F(BooleanKeywordStrategyTest, ExtractKeywords_MixedWhitespace_ReturnsSeparateKeywords)
{
    // Test extracting keywords from mixed whitespace
    QString mixedKeyword = "hello \t world\n\r test";
    QStringList result = strategy->extractKeywords(mixedKeyword);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_TRUE(result.contains("hello"));
    EXPECT_TRUE(result.contains("world"));
    EXPECT_TRUE(result.contains("test"));
}

TEST_F(BooleanKeywordStrategyTest, ExtractKeywords_EmptyKeyword_ReturnsEmpty)
{
    // Test extracting keywords from empty string
    QString emptyKeyword = "";
    QStringList result = strategy->extractKeywords(emptyKeyword);
    
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(BooleanKeywordStrategyTest, ExtractKeywords_OnlyWhitespace_ReturnsEmpty)
{
    // Test extracting keywords from whitespace only
    QString whitespaceKeyword = "   \t\n  ";
    QStringList result = strategy->extractKeywords(whitespaceKeyword);
    
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(BooleanKeywordStrategyTest, CanHandle_WithWhitespace_ReturnsTrue)
{
    // Test that BooleanKeywordStrategy can handle keywords with whitespace
    EXPECT_TRUE(strategy->canHandle("hello world"));
    EXPECT_TRUE(strategy->canHandle("test\ttab"));
    EXPECT_TRUE(strategy->canHandle("new\nline"));
}

TEST_F(BooleanKeywordStrategyTest, CanHandle_NoWhitespace_ReturnsFalse)
{
    // Test that BooleanKeywordStrategy cannot handle keywords without whitespace
    EXPECT_FALSE(strategy->canHandle("singleword"));
    EXPECT_FALSE(strategy->canHandle(""));
}

TEST_F(BooleanKeywordStrategyTest, Priority_ReturnsHighPriority)
{
    // Test that BooleanKeywordStrategy has high priority
    EXPECT_EQ(strategy->priority(), 10);
}

// ============== WildcardKeywordStrategy Tests ==============

class WildcardKeywordStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        strategy = new WildcardKeywordStrategy();
    }

    void TearDown() override
    {
        delete strategy;
        stub.clear();
    }

    WildcardKeywordStrategy *strategy = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WildcardKeywordStrategyTest, ExtractKeywords_AsteriskWildcard_ReturnsPatternAndSegments)
{
    // Test extracting keywords from asterisk wildcard pattern
    QString wildcardKeyword = "file*.txt";
    QStringList result = strategy->extractKeywords(wildcardKeyword);
    
    EXPECT_GE(result.size(), 1);
    EXPECT_EQ(result.first(), wildcardKeyword); // Original pattern should be first
    EXPECT_TRUE(result.contains("file"));
    EXPECT_TRUE(result.contains("txt"));
}

TEST_F(WildcardKeywordStrategyTest, ExtractKeywords_QuestionMarkWildcard_ReturnsPatternAndSegments)
{
    // Test extracting keywords from question mark wildcard pattern
    QString wildcardKeyword = "test?.doc";
    QStringList result = strategy->extractKeywords(wildcardKeyword);
    
    EXPECT_GE(result.size(), 1);
    EXPECT_EQ(result.first(), wildcardKeyword); // Original pattern should be first
    EXPECT_TRUE(result.contains("test"));
    EXPECT_TRUE(result.contains("doc"));
}

TEST_F(WildcardKeywordStrategyTest, ExtractKeywords_MixedWildcards_ReturnsPatternAndSegments)
{
    // Test extracting keywords from mixed wildcard pattern
    QString wildcardKeyword = "data*file?.backup";
    QStringList result = strategy->extractKeywords(wildcardKeyword);
    
    EXPECT_GE(result.size(), 1);
    EXPECT_EQ(result.first(), wildcardKeyword); // Original pattern should be first
    EXPECT_TRUE(result.contains("data"));
    EXPECT_TRUE(result.contains("file"));
    EXPECT_TRUE(result.contains("backup"));
}

TEST_F(WildcardKeywordStrategyTest, ExtractKeywords_OnlyWildcards_ReturnsOnlyPattern)
{
    // Test extracting keywords from only wildcards
    QString wildcardKeyword = "**??*";
    QStringList result = strategy->extractKeywords(wildcardKeyword);
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first(), wildcardKeyword); // Only original pattern
}

TEST_F(WildcardKeywordStrategyTest, ExtractKeywords_EmptyKeyword_ReturnsEmpty)
{
    // Test extracting keywords from empty string
    QString emptyKeyword = "";
    QStringList result = strategy->extractKeywords(emptyKeyword);
    
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(WildcardKeywordStrategyTest, ExtractKeywords_DeduplicatesResults)
{
    // Test that duplicate segments are removed
    QString wildcardKeyword = "test*test?.test";
    QStringList result = strategy->extractKeywords(wildcardKeyword);
    
    EXPECT_EQ(result.first(), wildcardKeyword); // Original pattern first
    // Count occurrences of "test" - should only appear once after the original pattern
    int testCount = 0;
    for (int i = 1; i < result.size(); ++i) {
        if (result[i] == "test") {
            testCount++;
        }
    }
    EXPECT_EQ(testCount, 1);
}

TEST_F(WildcardKeywordStrategyTest, CanHandle_WithAsterisk_ReturnsTrue)
{
    // Test that WildcardKeywordStrategy can handle asterisk wildcards
    EXPECT_TRUE(strategy->canHandle("*.txt"));
    EXPECT_TRUE(strategy->canHandle("file*"));
    EXPECT_TRUE(strategy->canHandle("*"));
}

TEST_F(WildcardKeywordStrategyTest, CanHandle_WithQuestionMark_ReturnsTrue)
{
    // Test that WildcardKeywordStrategy can handle question mark wildcards
    EXPECT_TRUE(strategy->canHandle("file?.doc"));
    EXPECT_TRUE(strategy->canHandle("?test"));
    EXPECT_TRUE(strategy->canHandle("?"));
}

TEST_F(WildcardKeywordStrategyTest, CanHandle_NoWildcards_ReturnsFalse)
{
    // Test that WildcardKeywordStrategy cannot handle keywords without wildcards
    EXPECT_FALSE(strategy->canHandle("normalfile.txt"));
    EXPECT_FALSE(strategy->canHandle("test"));
    EXPECT_FALSE(strategy->canHandle(""));
}

TEST_F(WildcardKeywordStrategyTest, Priority_ReturnsMediumPriority)
{
    // Test that WildcardKeywordStrategy has medium priority
    EXPECT_EQ(strategy->priority(), 20);
}

// ============== KeywordExtractor Tests ==============

TEST_F(KeywordExtractorTest, Constructor_InitializesStrategies)
{
    // Test that constructor properly initializes strategies
    KeywordExtractor extractor;
    const auto &strategies = extractor.getStrategies();
    
    EXPECT_EQ(strategies.size(), 3);
    
    // Check that strategies are sorted by priority
    EXPECT_EQ(strategies[0]->priority(), 10); // BooleanKeywordStrategy
    EXPECT_EQ(strategies[1]->priority(), 20); // WildcardKeywordStrategy
    EXPECT_EQ(strategies[2]->priority(), 100); // SimpleKeywordStrategy
}

TEST_F(KeywordExtractorTest, ExtractFromUrl_ValidUrlWithKeyword_ReturnsExtractedKeywords)
{
    // Test extracting keywords from URL with keyword parameter
    KeywordExtractor extractor;
    QUrl testUrl("file:///search?keyword=hello%20world");
    
    QStringList result = extractor.extractFromUrl(testUrl);
    
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.contains("hello"));
    EXPECT_TRUE(result.contains("world"));
}

TEST_F(KeywordExtractorTest, ExtractFromUrl_UrlWithoutKeyword_ReturnsEmpty)
{
    // Test extracting keywords from URL without keyword parameter
    KeywordExtractor extractor;
    QUrl testUrl("file:///search?other=value");
    
    QStringList result = extractor.extractFromUrl(testUrl);
    
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(KeywordExtractorTest, ExtractFromUrl_UrlWithEncodedKeyword_ReturnsDecodedKeywords)
{
    // Test extracting keywords from URL with percent-encoded keyword
    KeywordExtractor extractor;
    QUrl testUrl("file:///search?keyword=test%2Dfile");
    
    QStringList result = extractor.extractFromUrl(testUrl);
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first(), "test-file");
}

TEST_F(KeywordExtractorTest, ExtractFromKeyword_SimpleKeyword_UsesSimpleStrategy)
{
    // Test extracting keywords using simple strategy
    KeywordExtractor extractor;
    QString simpleKeyword = "test";
    
    QStringList result = extractor.extractFromKeyword(simpleKeyword);
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first(), simpleKeyword);
}

TEST_F(KeywordExtractorTest, ExtractFromKeyword_BooleanKeyword_UsesBooleanStrategy)
{
    // Test extracting keywords using boolean strategy
    KeywordExtractor extractor;
    QString booleanKeyword = "hello world";
    
    QStringList result = extractor.extractFromKeyword(booleanKeyword);
    
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.contains("hello"));
    EXPECT_TRUE(result.contains("world"));
}

TEST_F(KeywordExtractorTest, ExtractFromKeyword_WildcardKeyword_UsesWildcardStrategy)
{
    // Test extracting keywords using wildcard strategy
    KeywordExtractor extractor;
    QString wildcardKeyword = "*.txt";
    
    QStringList result = extractor.extractFromKeyword(wildcardKeyword);
    
    EXPECT_GE(result.size(), 1);
    EXPECT_EQ(result.first(), wildcardKeyword);
    EXPECT_TRUE(result.contains("txt"));
}

TEST_F(KeywordExtractorTest, ExtractFromKeyword_EmptyKeyword_ReturnsEmpty)
{
    // Test extracting keywords from empty string
    KeywordExtractor extractor;
    QString emptyKeyword = "";
    
    QStringList result = extractor.extractFromKeyword(emptyKeyword);
    
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(KeywordExtractorTest, RegisterStrategy_ValidStrategy_AddsToList)
{
    // Test registering a new strategy
    KeywordExtractor extractor;
    auto initialCount = extractor.getStrategies().size();
    
    auto newStrategy = QSharedPointer<KeywordExtractionStrategy>(new SimpleKeywordStrategy());
    extractor.registerStrategy(newStrategy);
    
    EXPECT_EQ(extractor.getStrategies().size(), initialCount + 1);
}

TEST_F(KeywordExtractorTest, RegisterStrategy_NullStrategy_DoesNotAdd)
{
    // Test registering null strategy
    KeywordExtractor extractor;
    auto initialCount = extractor.getStrategies().size();
    
    extractor.registerStrategy(QSharedPointer<KeywordExtractionStrategy>());
    
    EXPECT_EQ(extractor.getStrategies().size(), initialCount);
}

TEST_F(KeywordExtractorTest, RegisterStrategy_SortsStrategiesByPriority)
{
    // Test that strategies are sorted by priority after registration
    KeywordExtractor extractor;
    
    // Create a custom strategy with very high priority
    class HighPriorityStrategy : public KeywordExtractionStrategy {
    public:
        QStringList extractKeywords(const QString &) const override { return {}; }
        bool canHandle(const QString &) const override { return false; }
        int priority() const override { return 1; } // Very high priority
    };
    
    auto highPriorityStrategy = QSharedPointer<KeywordExtractionStrategy>(new HighPriorityStrategy());
    extractor.registerStrategy(highPriorityStrategy);
    
    const auto &strategies = extractor.getStrategies();
    EXPECT_EQ(strategies.first()->priority(), 1); // Should be first
}

// ============== KeywordExtractorManager Tests ==============

class KeywordExtractorManagerTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(KeywordExtractorManagerTest, Instance_ReturnsSameInstance)
{
    // Test that instance() returns the same singleton instance
    auto &instance1 = KeywordExtractorManager::instance();
    auto &instance2 = KeywordExtractorManager::instance();
    
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(KeywordExtractorManagerTest, Extractor_ReturnsValidExtractor)
{
    // Test that extractor() returns a valid extractor
    auto &manager = KeywordExtractorManager::instance();
    auto &extractor = manager.extractor();
    
    // Test that the extractor works by checking its strategies
    const auto &strategies = extractor.getStrategies();
    EXPECT_GT(strategies.size(), 0);
}

TEST_F(KeywordExtractorManagerTest, Extractor_ReturnsSameExtractor)
{
    // Test that extractor() returns the same extractor instance
    auto &manager = KeywordExtractorManager::instance();
    auto &extractor1 = manager.extractor();
    auto &extractor2 = manager.extractor();
    
    EXPECT_EQ(&extractor1, &extractor2);
}

// ============== Integration Tests ==============

class KeywordExtractorIntegrationTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(KeywordExtractorIntegrationTest, EndToEnd_ComplexScenarios)
{
    // Test end-to-end scenarios with different keyword types
    auto &manager = KeywordExtractorManager::instance();
    auto &extractor = manager.extractor();
    
    // Test simple keyword
    auto simpleResult = extractor.extractFromKeyword("document");
    EXPECT_EQ(simpleResult.size(), 1);
    EXPECT_EQ(simpleResult.first(), "document");
    
    // Test boolean keyword
    auto booleanResult = extractor.extractFromKeyword("hello world test");
    EXPECT_EQ(booleanResult.size(), 3);
    
    // Test wildcard keyword
    auto wildcardResult = extractor.extractFromKeyword("*.pdf");
    EXPECT_GE(wildcardResult.size(), 1);
    EXPECT_EQ(wildcardResult.first(), "*.pdf");
    
    // Test URL extraction
    QUrl testUrl("file:///search?keyword=test%20file");
    auto urlResult = extractor.extractFromUrl(testUrl);
    EXPECT_EQ(urlResult.size(), 2);
    EXPECT_TRUE(urlResult.contains("test"));
    EXPECT_TRUE(urlResult.contains("file"));
}

TEST_F(KeywordExtractorIntegrationTest, PriorityHandling_SelectsCorrectStrategy)
{
    // Test that the correct strategy is selected based on priority
    auto &extractor = KeywordExtractorManager::instance().extractor();
    
    // Wildcard should take precedence over boolean for "test *.txt"
    // (assuming wildcard has higher priority than boolean for this specific case)
    QString mixedKeyword = "test*.txt file";
    auto result = extractor.extractFromKeyword(mixedKeyword);
    
    // Should use wildcard strategy because it contains '*'
    EXPECT_GE(result.size(), 1);
    EXPECT_EQ(result.first(), mixedKeyword); // Wildcard strategy puts original pattern first
} 