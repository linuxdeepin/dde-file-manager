// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PATHEXCLUDEMATCHER_H
#define PATHEXCLUDEMATCHER_H

#include "service_textindex_global.h"

#include <QList>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

SERVICETEXTINDEX_BEGIN_NAMESPACE

/**
 * @brief Exclude pattern types for path matching
 *
 * Pattern syntax rules:
 * - ExactName: Plain directory name like "tmp", ".git"
 *   Matches any directory with this exact name in the path
 *
 * - PathSegment: Contains "/" but doesn't start with "/"
 *   Like ".local/share/Trash", matches this exact path segment
 *
 * - AbsolutePrefix: Starts with "/" like "/home/user/exclude"
 *   Matches paths that start with this absolute path
 *
 * - GlobPattern: Contains "*" or "?" wildcards
 *   Like "build-*", "*.cache", matches directory names using glob
 */
enum class ExcludePatternType {
    ExactName,   ///< Exact directory name match (e.g., "tmp", ".git")
    PathSegment,   ///< Path segment match (e.g., ".local/share/Trash")
    AbsolutePrefix,   ///< Absolute path prefix match (e.g., "/home/user/exclude")
    GlobPattern   ///< Glob wildcard pattern (e.g., "build-*", "*.cache")
};

/**
 * @brief Path exclusion pattern matcher
 *
 * A utility class for matching file system paths against exclusion patterns.
 * Supports four types of matching syntax:
 *
 * 1. Exact Name Match (ExactName)
 *    Syntax: "tmp", ".git", "node_modules"
 *    Rule: If any directory component in the path exactly equals this value,
 *          the path and all its children are excluded.
 *    Example: "tmp" matches /home/user/tmp and /home/user/tmp/subdir,
 *             but NOT /home/user/template
 *
 * 2. Path Segment Match (PathSegment)
 *    Syntax: ".local/share/Trash" (contains "/" but doesn't start with "/")
 *    Rule: The path contains this exact contiguous sub-path
 *    Example: ".local/share/Trash" matches /home/user/.local/share/Trash/files
 *
 * 3. Absolute Prefix Match (AbsolutePrefix)
 *    Syntax: "/home/user/exclude" (starts with "/")
 *    Rule: The path starts with this absolute path
 *    Example: "/home/user/exclude" matches /home/user/exclude/any/subpath
 *
 * 4. Glob Pattern Match (GlobPattern)
 *    Syntax: "build-*", "*.cache", "test?" (contains * or ?)
 *    Rule: Any directory component in the path matches the glob pattern
 *    Example: "build-*" matches /project/build-debug/file
 *
 * @note This class is thread-safe for read operations after construction.
 *       Write operations (add/remove/clear) are not thread-safe.
 */
class PathExcludeMatcher
{
public:
    /**
     * @brief Construct an empty matcher
     */
    PathExcludeMatcher();

    /**
     * @brief Construct a matcher with initial patterns
     * @param patterns List of pattern strings to parse and add
     */
    explicit PathExcludeMatcher(const QStringList &patterns);

    /**
     * @brief Add a single exclusion pattern
     * @param pattern Pattern string to parse and add
     */
    void addPattern(const QString &pattern);

    /**
     * @brief Add multiple exclusion patterns
     * @param patterns List of pattern strings to parse and add
     */
    void addPatterns(const QStringList &patterns);

    /**
     * @brief Remove a pattern by its original string
     * @param pattern Original pattern string to remove
     */
    void removePattern(const QString &pattern);

    /**
     * @brief Clear all patterns
     */
    void clear();

    /**
     * @brief Check if a path should be excluded
     * @param absolutePath Absolute path to check
     * @return true if path matches any exclusion pattern, false otherwise
     */
    bool shouldExclude(const QString &absolutePath) const;

    /**
     * @brief Get the list of original pattern strings
     * @return List of pattern strings that were added
     */
    QStringList patterns() const;

    /**
     * @brief Check if the matcher has any patterns
     * @return true if at least one pattern exists
     */
    bool hasPatterns() const;

    /**
     * @brief Get the number of patterns
     * @return Number of patterns in the matcher
     */
    int patternCount() const;

    /**
     * @brief Create a PathExcludeMatcher instance configured with index blacklist
     *
     * This factory method encapsulates the blacklist configuration logic for the text index service,
     * automatically loading blacklist patterns from the following sources:
     * - TextIndexConfig::folderExcludeFilters() - Text index configuration exclude directories
     * - IndexUtility::AnythingConfigWatcher::defaultBlacklistPaths() - Anything service blacklist
     *
     * @return Configured PathExcludeMatcher instance
     *
     * @note This method ensures blacklist configuration consistency, avoiding duplicate initialization logic
     */
    static PathExcludeMatcher createForIndex();

private:
    /**
     * @brief Internal representation of a parsed exclude pattern
     */
    struct ExcludePattern
    {
        QString original;   ///< Original pattern string
        ExcludePatternType type;   ///< Parsed pattern type
        QRegularExpression regex;   ///< Compiled regex (only for GlobPattern)
    };

    /**
     * @brief Parse a pattern string into ExcludePattern
     * @param pattern Raw pattern string
     * @return Parsed ExcludePattern structure
     */
    static ExcludePattern parsePattern(const QString &pattern);

    /**
     * @brief Check if a path matches a specific pattern
     * @param absolutePath Path to check
     * @param pattern Parsed pattern to match against
     * @return true if path matches the pattern
     */
    static bool matchPattern(const QString &absolutePath, const ExcludePattern &pattern);

    /**
     * @brief Convert a glob pattern to QRegularExpression
     * @param glob Glob pattern string (e.g., "build-*", "*.cache")
     * @return Compiled regular expression
     */
    static QRegularExpression globToRegex(const QString &glob);

    QList<ExcludePattern> m_patterns;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // PATHEXCLUDEMATCHER_H
