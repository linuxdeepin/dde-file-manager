// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docutils.cpp
 * @brief Unit tests for DocUtils (utils/docutils.cpp).
 *        Tests copyFieldsExcept with various document configurations.
 */

#include <gtest/gtest.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/utils/docutils.h"

#include <lucene++/LuceneHeaders.h>
#include <lucene++/NumericField.h>
#include <lucene++/Document.h>
#include <lucene++/Field.h>

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace Lucene;

class DocUtilsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        doc = newLucene<Document>();
    }

    DocumentPtr doc;
};

TEST_F(DocUtilsTest, CopyFieldsExcept_NullDoc_ReturnsNull)
{
    auto result = DocUtils::copyFieldsExcept(DocumentPtr(), { L"field1" });
    EXPECT_EQ(result, nullptr);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_EmptyDocument)
{
    auto result = DocUtils::copyFieldsExcept(doc, { L"nonexistent" });
    ASSERT_NE(result, nullptr);
    // Empty doc copied, should have no fields
    EXPECT_EQ(result->getFields().size(), 0);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_SingleField_NoExclusion)
{
    doc->add(newLucene<Field>(L"name", L"value", Field::STORE_YES, Field::INDEX_ANALYZED));

    auto result = DocUtils::copyFieldsExcept(doc, {});
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_SingleField_ExactExclusion)
{
    doc->add(newLucene<Field>(L"name", L"value", Field::STORE_YES, Field::INDEX_ANALYZED));

    auto result = DocUtils::copyFieldsExcept(doc, { L"name" });
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 0);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_SingleField_DifferentExclusion)
{
    doc->add(newLucene<Field>(L"name", L"value", Field::STORE_YES, Field::INDEX_ANALYZED));

    auto result = DocUtils::copyFieldsExcept(doc, { L"other" });
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_MultipleFields_PartialExclusion)
{
    doc->add(newLucene<Field>(L"path", L"/home/user/file.txt", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"content", L"hello world", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"filename", L"file.txt", Field::STORE_YES, Field::INDEX_ANALYZED));

    auto result = DocUtils::copyFieldsExcept(doc, { L"path" });
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    // Should have content and filename, but not path
    EXPECT_EQ(fields.size(), 2);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_MultipleExcludedFields)
{
    doc->add(newLucene<Field>(L"path", L"/home/user/file.txt", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"ancestor_paths", L"/home/user", Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"content", L"hello world", Field::STORE_YES, Field::INDEX_ANALYZED));

    auto result = DocUtils::copyFieldsExcept(doc, { L"path", L"ancestor_paths" });
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_WithNumericField_NotExcluded)
{
    doc->add(newLucene<Field>(L"path", L"/file.txt", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    NumericFieldPtr numField = newLucene<NumericField>(L"modify_time", Field::STORE_YES, true);
    numField->setLongValue(1700000000);
    doc->add(numField);

    auto result = DocUtils::copyFieldsExcept(doc, { L"path" });
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_WithNumericField_Excluded)
{
    NumericFieldPtr numField = newLucene<NumericField>(L"modify_time", Field::STORE_YES, true);
    numField->setLongValue(1700000000);
    doc->add(numField);

    auto result = DocUtils::copyFieldsExcept(doc, { L"modify_time" });
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 0);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_MixedFieldTypes)
{
    doc->add(newLucene<Field>(L"path", L"/file.txt", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"ancestor_paths", L"/home", Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"content", L"text content", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"filename", L"file.txt", Field::STORE_YES, Field::INDEX_ANALYZED));

    NumericFieldPtr numField1 = newLucene<NumericField>(L"modify_time", Field::STORE_YES, true);
    numField1->setLongValue(1700000000);
    doc->add(numField1);

    NumericFieldPtr numField2 = newLucene<NumericField>(L"birth_time", Field::STORE_YES, true);
    numField2->setLongValue(1600000000);
    doc->add(numField2);

    NumericFieldPtr numField3 = newLucene<NumericField>(L"file_size", Field::STORE_YES, true);
    numField3->setLongValue(1024);
    doc->add(numField3);

    // Exclude path and ancestor_paths
    auto result = DocUtils::copyFieldsExcept(doc, { L"path", L"ancestor_paths" });
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    // Should have content, filename, modify_time, birth_time, file_size = 5
    EXPECT_EQ(fields.size(), 5);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_NumericFieldLargeValue)
{
    NumericFieldPtr numField = newLucene<NumericField>(L"big_num", Field::STORE_YES, true);
    numField->setLongValue(LLONG_MAX);
    doc->add(numField);

    auto result = DocUtils::copyFieldsExcept(doc, {});
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_NumericFieldZeroValue)
{
    NumericFieldPtr numField = newLucene<NumericField>(L"zero_num", Field::STORE_YES, true);
    numField->setLongValue(0);
    doc->add(numField);

    auto result = DocUtils::copyFieldsExcept(doc, {});
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_FieldNotStored)
{
    doc->add(newLucene<Field>(L"content", L"text", Field::STORE_NO, Field::INDEX_ANALYZED));

    auto result = DocUtils::copyFieldsExcept(doc, {});
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_FieldNotIndexed)
{
    doc->add(newLucene<Field>(L"data", L"raw data", Field::STORE_YES, Field::INDEX_NO));

    auto result = DocUtils::copyFieldsExcept(doc, {});
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_NumericFieldNotStored)
{
    NumericFieldPtr numField = newLucene<NumericField>(L"num_no_store", Field::STORE_NO, true);
    numField->setLongValue(42);
    doc->add(numField);

    auto result = DocUtils::copyFieldsExcept(doc, {});
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_NumericFieldNotIndexed)
{
    NumericFieldPtr numField = newLucene<NumericField>(L"num_no_idx", Field::STORE_YES, false);
    numField->setLongValue(42);
    doc->add(numField);

    auto result = DocUtils::copyFieldsExcept(doc, {});
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_ManyFieldsManyExclusions)
{
    for (int i = 0; i < 10; i++) {
        QString fieldName = QString("field_%1").arg(i);
        doc->add(newLucene<Field>(fieldName.toStdWString(), L"value", Field::STORE_YES, Field::INDEX_ANALYZED));
    }

    // Exclude odd-numbered fields
    auto result = DocUtils::copyFieldsExcept(doc, { L"field_1", L"field_3", L"field_5", L"field_7", L"field_9" });
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 5);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_ExcludeNonExistentField)
{
    doc->add(newLucene<Field>(L"real_field", L"value", Field::STORE_YES, Field::INDEX_ANALYZED));

    auto result = DocUtils::copyFieldsExcept(doc, { L"fake_field" });
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_AllFieldsExcluded)
{
    doc->add(newLucene<Field>(L"field1", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"field2", L"v2", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    auto result = DocUtils::copyFieldsExcept(doc, { L"field1", L"field2" });
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 0);
}

TEST_F(DocUtilsTest, CopyFieldsExcept_NotAnalyzedFieldCopiedCorrectly)
{
    doc->add(newLucene<Field>(L"exact", L"exact_value", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    auto result = DocUtils::copyFieldsExcept(doc, {});
    ASSERT_NE(result, nullptr);
    auto fields = result->getFields();
    EXPECT_EQ(fields.size(), 1);
}
