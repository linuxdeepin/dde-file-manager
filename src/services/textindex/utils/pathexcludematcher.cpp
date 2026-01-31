// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pathexcludematcher.h"
#include "textindexconfig.h"
#include "indexutility.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

PathExcludeMatcher::PathExcludeMatcher() = default;

PathExcludeMatcher::PathExcludeMatcher(const QStringList &patterns)
{
    addPatterns(patterns);
}

void PathExcludeMatcher::addPattern(const QString &pattern)
{
    if (pattern.isEmpty()) {
        return;
    }

    // Check if pattern already exists
    for (const auto &existing : m_patterns) {
        if (existing.original == pattern) {
            return;
        }
    }

    m_patterns.append(parsePattern(pattern));
}

void PathExcludeMatcher::addPatterns(const QStringList &patterns)
{
    for (const QString &pattern : patterns) {
        addPattern(pattern);
    }
}

void PathExcludeMatcher::removePattern(const QString &pattern)
{
    m_patterns.removeIf([&pattern](const ExcludePattern &p) {
        return p.original == pattern;
    });
}

void PathExcludeMatcher::clear()
{
    m_patterns.clear();
}

bool PathExcludeMatcher::shouldExclude(const QString &absolutePath) const
{
    if (absolutePath.isEmpty() || m_patterns.isEmpty()) {
        return false;
    }

    for (const auto &pattern : m_patterns) {
        if (matchPattern(absolutePath, pattern)) {
            return true;
        }
    }

    return false;
}

QStringList PathExcludeMatcher::patterns() const
{
    QStringList result;
    result.reserve(m_patterns.size());
    for (const auto &pattern : m_patterns) {
        result.append(pattern.original);
    }
    return result;
}

bool PathExcludeMatcher::hasPatterns() const
{
    return !m_patterns.isEmpty();
}

int PathExcludeMatcher::patternCount() const
{
    return m_patterns.size();
}

PathExcludeMatcher::ExcludePattern PathExcludeMatcher::parsePattern(const QString &pattern)
{
    ExcludePattern result;
    result.original = pattern;

    // Determine pattern type based on syntax
    if (pattern.startsWith('/')) {
        // Absolute path prefix: starts with "/"
        result.type = ExcludePatternType::AbsolutePrefix;
    } else if (pattern.contains('*') || pattern.contains('?')) {
        // Glob pattern: contains wildcards
        result.type = ExcludePatternType::GlobPattern;
        result.regex = globToRegex(pattern);
    } else if (pattern.contains('/')) {
        // Path segment: contains "/" but doesn't start with "/"
        result.type = ExcludePatternType::PathSegment;
    } else {
        // Exact name: plain directory name
        result.type = ExcludePatternType::ExactName;
    }

    return result;
}

bool PathExcludeMatcher::matchPattern(const QString &absolutePath, const ExcludePattern &pattern)
{
    switch (pattern.type) {
    case ExcludePatternType::ExactName: {
        // Split path into components and check if any matches exactly
        // This ensures "tmp" matches "/home/user/tmp" and "/home/user/tmp/subdir"
        // but NOT "/home/user/template"
        const QStringList parts = absolutePath.split('/', Qt::SkipEmptyParts);
        return parts.contains(pattern.original);
    }

    case ExcludePatternType::PathSegment: {
        // Match path segment with proper boundaries
        // Ensure we match complete path segments, not substrings
        // ".local/share/Trash" should match "/.local/share/Trash/" or "/.local/share/Trash"
        // but NOT "/.local/share/Trashcan"
        const QString pathWithTrailingSlash = absolutePath.endsWith('/')
                ? absolutePath
                : absolutePath + '/';
        const QString segmentPattern = '/' + pattern.original + '/';
        return pathWithTrailingSlash.contains(segmentPattern);
    }

    case ExcludePatternType::AbsolutePrefix: {
        // Match absolute path prefix
        // "/home/user/exclude" matches "/home/user/exclude" and "/home/user/exclude/subdir"
        if (absolutePath == pattern.original) {
            return true;
        }
        // Ensure we match complete path components
        const QString prefixWithSlash = pattern.original.endsWith('/')
                ? pattern.original
                : pattern.original + '/';
        return absolutePath.startsWith(prefixWithSlash);
    }

    case ExcludePatternType::GlobPattern: {
        // Split path into components and check if any matches the glob pattern
        const QStringList parts = absolutePath.split('/', Qt::SkipEmptyParts);
        for (const QString &part : parts) {
            if (pattern.regex.match(part).hasMatch()) {
                return true;
            }
        }
        return false;
    }
    }

    return false;
}

QRegularExpression PathExcludeMatcher::globToRegex(const QString &glob)
{
    QString regexPattern;
    regexPattern.reserve(glob.size() * 2);

    // Anchor at start
    regexPattern += '^';

    for (const QChar &ch : glob) {
        if (ch == '*') {
            // * matches any characters (including none)
            regexPattern += ".*";
        } else if (ch == '?') {
            // ? matches exactly one character
            regexPattern += '.';
        } else if (ch == '.' || ch == '+' || ch == '^' || ch == '$'
                   || ch == '(' || ch == ')' || ch == '[' || ch == ']'
                   || ch == '{' || ch == '}' || ch == '|' || ch == '\\') {
            // Escape regex special characters
            regexPattern += '\\';
            regexPattern += ch;
        } else {
            regexPattern += ch;
        }
    }

    // Anchor at end
    regexPattern += '$';

    return QRegularExpression(regexPattern);
}

PathExcludeMatcher PathExcludeMatcher::createForIndex()
{
    PathExcludeMatcher matcher;

    // Load default blacklist directories from TextIndexConfig
    const auto &textIndexBlacklist = TextIndexConfig::instance().folderExcludeFilters();
    matcher.addPatterns(textIndexBlacklist);

    // Load blacklist from AnythingConfigWatcher
    const auto &anythingBlacklist =
        IndexUtility::AnythingConfigWatcher::instance()->defaultBlacklistPaths();
    matcher.addPatterns(anythingBlacklist);

    return matcher;
}

SERVICETEXTINDEX_END_NAMESPACE
