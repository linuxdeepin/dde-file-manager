// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCUTILS_H
#define DOCUTILS_H

#include "service_textindex_global.h"

#include <lucene++/LuceneHeaders.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace DocUtils {

/**
 * @brief Copy all fields from source document except the specified excluded fields
 * @param sourceDoc Source document to copy from
 * @param excludeFieldNames Field names to exclude from copying
 * @return New document with copied fields
 */
Lucene::DocumentPtr copyFieldsExcept(const Lucene::DocumentPtr &sourceDoc,
                                     std::initializer_list<Lucene::String> excludeFieldNames);

}   // namespace DocUtils

SERVICETEXTINDEX_END_NAMESPACE

#endif   // DOCUTILS_H
