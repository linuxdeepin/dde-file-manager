// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file field_names_compat.h
 * @brief Test-only compatibility header that supplements the system-installed
 *        dfm-search/field_names.h with kFileExt constants for the Content and
 *        OcrText namespaces.
 *
 * The system header (dfm6-search) only defines kFileExt under the FileName
 * namespace. The textindex source files (contentdocumentbuilder.cpp,
 * ocrdocumentbuilder.cpp) reference Content::kFileExt and OcrText::kFileExt,
 * which causes a compilation failure. This header is force-included into the
 * library build via the test CMakeLists.txt so that those symbols resolve,
 * without modifying any production source files.
 */

#pragma once

#include <dfm-search/field_names.h>

DFM_SEARCH_BEGIN_NS
namespace LuceneFieldNames {
namespace Content {
constexpr const wchar_t kFileExt[] = L"file_ext";
}   // namespace Content
namespace OcrText {
constexpr const wchar_t kFileExt[] = L"file_ext";
}   // namespace OcrText
}   // namespace LuceneFieldNames
DFM_SEARCH_END_NS
