// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docutils.h"

#include <lucene++/NumericField.h>
#include <boost/shared_ptr.hpp>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace DocUtils {

Lucene::DocumentPtr copyFieldsExcept(const Lucene::DocumentPtr &sourceDoc,
                                     std::initializer_list<Lucene::String> excludeFieldNames)
{
    using namespace Lucene;

    if (!sourceDoc) {
        return nullptr;
    }

    DocumentPtr newDoc = newLucene<Document>();
    Collection<FieldablePtr> fields = sourceDoc->getFields();

    for (Collection<FieldablePtr>::iterator fieldIt = fields.begin(); fieldIt != fields.end(); ++fieldIt) {
        FieldablePtr fieldable = *fieldIt;
        String fieldName = fieldable->name();

        // Check if field name is in the exclude list
        bool shouldExclude = false;
        for (const auto &excludeName : excludeFieldNames) {
            if (fieldName == excludeName) {
                shouldExclude = true;
                break;
            }
        }

        if (!shouldExclude) {
            // Check if this is a NumericField
            NumericFieldPtr numericField = boost::dynamic_pointer_cast<NumericField>(fieldable);
            if (numericField) {
                // Copy NumericField with its numeric value
                NumericFieldPtr newNumericField = newLucene<NumericField>(
                        fieldName,
                        fieldable->isStored() ? Field::STORE_YES : Field::STORE_NO,
                        fieldable->isIndexed());
                newNumericField->setLongValue(numericField->getNumericValue());
                newDoc->add(newNumericField);
            } else {
                // Copy regular field with original properties
                newDoc->add(newLucene<Field>(fieldName, fieldable->stringValue(),
                                             fieldable->isStored() ? Field::STORE_YES : Field::STORE_NO,
                                             fieldable->isIndexed() ? (fieldable->isTokenized() ? Field::INDEX_ANALYZED : Field::INDEX_NOT_ANALYZED) : Field::INDEX_NO));
            }
        }
    }

    return newDoc;
}

}   // namespace DocUtils

SERVICETEXTINDEX_END_NAMESPACE
