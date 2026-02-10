// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QMetaObject>
#include <QMetaEnum>
#include "dfmplugin_propertydialog_global.h"

#define DPPROPERTYDIALOG_NAMESPACE dfmplugin_propertydialog
using namespace DPPROPERTYDIALOG_NAMESPACE;

class TestPropertyDialogGlobal : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};
// Test PropertyFilterType enum values
TEST_F(TestPropertyDialogGlobal, PropertyFilterTypeEnum)
{
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kNotFilter), 0);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kIconTitle), 1);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kBasisInfo), 2);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kPermission), 4);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kFileSizeFiled), 8);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kFileCountFiled), 16);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kFileTypeFiled), 32);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kFilePositionFiled), 64);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kFileCreateTimeFiled), 128);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kFileAccessedTimeFiled), 256);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kFileModifiedTimeFiled), 512);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kFileMediaResolutionFiled), 1024);
    EXPECT_EQ(static_cast<int>(DPPROPERTYDIALOG_NAMESPACE::PropertyFilterType::kFileMediaDurationFiled), 2048);
}
// Test BasicFieldExpandEnum values
TEST_F(TestPropertyDialogGlobal, BasicFieldExpandEnum)
{
    EXPECT_EQ(static_cast<int>(BasicFieldExpandEnum::kNotAll), 0);
    EXPECT_EQ(static_cast<int>(BasicFieldExpandEnum::kFileSize), 1);
    EXPECT_EQ(static_cast<int>(BasicFieldExpandEnum::kFileCount), 2);
    EXPECT_EQ(static_cast<int>(BasicFieldExpandEnum::kFileType), 3);
    EXPECT_EQ(static_cast<int>(BasicFieldExpandEnum::kFilePosition), 4);
    EXPECT_EQ(static_cast<int>(BasicFieldExpandEnum::kFileCreateTime), 5);
    EXPECT_EQ(static_cast<int>(BasicFieldExpandEnum::kFileAccessedTime), 6);
    EXPECT_EQ(static_cast<int>(BasicFieldExpandEnum::kFileModifiedTime), 7);
    EXPECT_EQ(static_cast<int>(BasicFieldExpandEnum::kFileMediaResolution), 8);
    EXPECT_EQ(static_cast<int>(BasicFieldExpandEnum::kFileMediaDuration), 9);
}

// Test BasicExpandType values
TEST_F(TestPropertyDialogGlobal, BasicExpandType)
{
    EXPECT_EQ(static_cast<int>(BasicExpandType::kFieldInsert), 0);
    EXPECT_EQ(static_cast<int>(BasicExpandType::kFieldReplace), 1);
}

// Test ComputerInfoItem values
TEST_F(TestPropertyDialogGlobal, ComputerInfoItem)
{
    EXPECT_EQ(static_cast<int>(ComputerInfoItem::kName), 0);
    EXPECT_EQ(static_cast<int>(ComputerInfoItem::kVersion), 1);
    EXPECT_EQ(static_cast<int>(ComputerInfoItem::kEdition), 2);
    EXPECT_EQ(static_cast<int>(ComputerInfoItem::kOSBuild), 3);
    EXPECT_EQ(static_cast<int>(ComputerInfoItem::kType), 4);
    EXPECT_EQ(static_cast<int>(ComputerInfoItem::kCpu), 5);
    EXPECT_EQ(static_cast<int>(ComputerInfoItem::kMemory), 6);
}

// Test Q_ENUM_NS registration
TEST_F(TestPropertyDialogGlobal, MetaObjectRegistration)
{
    const QMetaObject *metaObj = &staticMetaObject;
    ASSERT_NE(metaObj, nullptr);
    
    // Test PropertyFilterType enum meta-object
    int propertyFilterTypeIdx = metaObj->indexOfEnumerator("PropertyFilterType");
    EXPECT_GE(propertyFilterTypeIdx, 0);
    
    if (propertyFilterTypeIdx >= 0) {
        QMetaEnum propertyFilterTypeEnum = metaObj->enumerator(propertyFilterTypeIdx);
        EXPECT_STREQ(propertyFilterTypeEnum.name(), "PropertyFilterType");
        EXPECT_GT(propertyFilterTypeEnum.keyCount(), 0);
    }
    
    // Test BasicFieldExpandEnum meta-object
    int basicFieldExpandEnumIdx = metaObj->indexOfEnumerator("BasicFieldExpandEnum");
    EXPECT_GE(basicFieldExpandEnumIdx, 0);
    
    if (basicFieldExpandEnumIdx >= 0) {
        QMetaEnum basicFieldExpandEnum = metaObj->enumerator(basicFieldExpandEnumIdx);
        EXPECT_STREQ(basicFieldExpandEnum.name(), "BasicFieldExpandEnum");
        EXPECT_GT(basicFieldExpandEnum.keyCount(), 0);
    }
    
    // Test BasicExpandType meta-object
    int basicExpandTypeIdx = metaObj->indexOfEnumerator("BasicExpandType");
    EXPECT_GE(basicExpandTypeIdx, 0);
    
    if (basicExpandTypeIdx >= 0) {
        QMetaEnum basicExpandType = metaObj->enumerator(basicExpandTypeIdx);
        EXPECT_STREQ(basicExpandType.name(), "BasicExpandType");
        EXPECT_GT(basicExpandType.keyCount(), 0);
    }
}

// Test static constants
TEST_F(TestPropertyDialogGlobal, StaticConstants)
{
    EXPECT_STREQ(kOption_Key_Name, "Option_Key_Name");
    EXPECT_STREQ(kOption_Key_BasicInfoExpand, "Option_Key_BasicInfoExpand");
    EXPECT_STREQ(kOption_Key_ExtendViewExpand, "Option_Key_ExtendViewExpand");
    EXPECT_STREQ(kOption_Key_ViewIndex, "Option_Key_ViewIndex");
    EXPECT_STREQ(kOption_Key_ViewInitCalback, "Option_Key_ViewInitCalback");
    EXPECT_STREQ(kOption_Key_CreatorCalback, "Option_Key_CreatorCalback");
}

// Test Q_DECLARE_METATYPE declarations
TEST_F(TestPropertyDialogGlobal, MetaTypeDeclarations)
{
    int customViewExtensionViewTypeId = qMetaTypeId<CustomViewExtensionView>();
    EXPECT_GT(customViewExtensionViewTypeId, 0);
    
    int basicViewFieldFuncTypeId = qMetaTypeId<BasicViewFieldFunc>();
    EXPECT_GT(basicViewFieldFuncTypeId, 0);
    
    int viewIntiCallbackTypeId = qMetaTypeId<ViewIntiCallback>();
    EXPECT_GT(viewIntiCallbackTypeId, 0);
}