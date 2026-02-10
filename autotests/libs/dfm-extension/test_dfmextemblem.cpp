// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include <dfm-extension/emblemicon/dfmextemblem.h>
#include <dfm-extension/emblemicon/dfmextemblemiconlayout.h>
#include "dfm_test_main.h"
#include "stubext.h"

using namespace DFMEXT;
using namespace testing;

/**
 * @brief DFMExtEmblem类单元测试
 *
 * 测试范围：
 * 1. 构造函数和析构函数
 * 2. 拷贝构造函数和赋值操作符
 * 3. 设置和获取标志图标功能
 * 4. 内存管理和资源清理
 * 5. 边界条件和异常处理
 */
class DFMExtEmblemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

/**
 * @brief 测试DFMExtEmblem默认构造函数
 * 验证对象能够正确创建并初始化内部数据
 */
TEST_F(DFMExtEmblemTest, DefaultConstructor)
{
    // 创建DFMExtEmblem对象
    DFMExtEmblem *emblem = new DFMExtEmblem();
    
    // 验证对象创建成功
    EXPECT_NE(emblem, nullptr);
    
    // 验证初始状态：标志图标列表应该为空
    auto emblems = emblem->emblems();
    EXPECT_TRUE(emblems.empty());
    
    // 清理资源
    delete emblem;
}

/**
 * @brief 测试DFMExtEmblem析构函数
 * 验证对象能够正确析构，不会出现内存泄漏
 */
TEST_F(DFMExtEmblemTest, Destructor)
{
    // 创建对象并在析构时验证不会崩溃
    {
        DFMExtEmblem emblem;
        EXPECT_TRUE(true); // 如果到达这里说明构造成功
    }
    // 对象自动析构，如果析构有问题会导致测试失败
    
    // 测试动态创建的对象
    DFMExtEmblem *dynamicEmblem = new DFMExtEmblem();
    EXPECT_NE(dynamicEmblem, nullptr);
    delete dynamicEmblem; // 应该不会崩溃
}

/**
 * @brief 测试DFMExtEmblem拷贝构造函数
 * 验证对象能够正确拷贝，创建独立的副本
 */
TEST_F(DFMExtEmblemTest, CopyConstructor)
{
    // 创建原始对象并设置一些标志图标
    DFMExtEmblem original;
    std::vector<DFMExtEmblemIconLayout> icons;
    icons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::TopLeft, "test_icon1.png", 10, 10));
    icons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::BottomRight, "test_icon2.png", 20, 20));
    original.setEmblem(icons);
    
    // 使用拷贝构造函数创建新对象
    DFMExtEmblem copy(original);
    
    // 验证拷贝对象的数据与原始对象一致
    auto copiedIcons = copy.emblems();
    EXPECT_EQ(copiedIcons.size(), 2);
    EXPECT_EQ(copiedIcons[0].iconPath(), "test_icon1.png");
    EXPECT_EQ(copiedIcons[0].locationType(), DFMExtEmblemIconLayout::LocationType::TopLeft);
    // EXPECT_DEPRECATION, 10);
    // EXPECT_DEPRECATION, 10);
    
    EXPECT_EQ(copiedIcons[1].iconPath(), "test_icon2.png");
    EXPECT_EQ(copiedIcons[1].locationType(), DFMExtEmblemIconLayout::LocationType::BottomRight);
    // EXPECT_DEPRECATION, 20);
    // EXPECT_DEPRECATION, 20);
    
    // 修改原始对象不应该影响拷贝对象
    std::vector<DFMExtEmblemIconLayout> newIcons;
    newIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::Custom, "new_icon.png", 50, 50));
    original.setEmblem(newIcons);
    
    // 验证拷贝对象没有受到影响
    auto unchangedIcons = copy.emblems();
    EXPECT_EQ(unchangedIcons.size(), 2);
    EXPECT_EQ(unchangedIcons[0].iconPath(), "test_icon1.png");
}

/**
 * @brief 测试DFMExtEmblem赋值操作符
 * 验证对象能够正确赋值，处理自赋值情况
 */
TEST_F(DFMExtEmblemTest, AssignmentOperator)
{
    // 创建两个对象
    DFMExtEmblem source;
    DFMExtEmblem target;
    
    // 为源对象设置标志图标
    std::vector<DFMExtEmblemIconLayout> sourceIcons;
    sourceIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::TopLeft, "source_icon.png", 15, 15));
    source.setEmblem(sourceIcons);
    
    // 执行赋值操作
    target = source;
    
    // 验证赋值结果
    auto targetIcons = target.emblems();
    EXPECT_EQ(targetIcons.size(), 1);
    EXPECT_EQ(targetIcons[0].iconPath(), "source_icon.png");
    EXPECT_EQ(targetIcons[0].locationType(), DFMExtEmblemIconLayout::LocationType::TopLeft);
    // EXPECT_DEPRECATION, 15);
    // EXPECT_DEPRECATION, 15);
    
    // 测试自赋值
    target = target;
    auto selfAssignIcons = target.emblems();
    EXPECT_EQ(selfAssignIcons.size(), 1);
    EXPECT_EQ(selfAssignIcons[0].iconPath(), "source_icon.png");
    
    // 修改源对象不应该影响目标对象
    std::vector<DFMExtEmblemIconLayout> newSourceIcons;
    newSourceIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::Custom, "modified_icon.png", 100, 100));
    source.setEmblem(newSourceIcons);
    
    auto unchangedTargetIcons = target.emblems();
    EXPECT_EQ(unchangedTargetIcons.size(), 1);
    EXPECT_EQ(unchangedTargetIcons[0].iconPath(), "source_icon.png");
}

/**
 * @brief 测试setEmblem方法
 * 验证设置标志图标功能的各种情况
 */
TEST_F(DFMExtEmblemTest, SetEmblem)
{
    DFMExtEmblem emblem;
    
    // 测试设置空图标列表
    std::vector<DFMExtEmblemIconLayout> emptyIcons;
    emblem.setEmblem(emptyIcons);
    auto result1 = emblem.emblems();
    EXPECT_TRUE(result1.empty());
    
    // 测试设置单个图标
    std::vector<DFMExtEmblemIconLayout> singleIcon;
    singleIcon.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::TopRight, "single.png", 5, 5));
    emblem.setEmblem(singleIcon);
    auto result2 = emblem.emblems();
    EXPECT_EQ(result2.size(), 1);
    EXPECT_EQ(result2[0].iconPath(), "single.png");
    
    // 测试设置多个图标
    std::vector<DFMExtEmblemIconLayout> multipleIcons;
    multipleIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::TopLeft, "icon1.png", 0, 0));
    multipleIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::TopRight, "icon2.png", 50, 0));
    multipleIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::BottomLeft, "icon3.png", 0, 50));
    multipleIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::BottomRight, "icon4.png", 50, 50));
    multipleIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::Custom, "icon5.png", 25, 25));
    emblem.setEmblem(multipleIcons);
    auto result3 = emblem.emblems();
    EXPECT_EQ(result3.size(), 5);
    
    // 验证每个图标的位置和路径
    EXPECT_EQ(result3[0].iconPath(), "icon1.png");
    EXPECT_EQ(result3[0].locationType(), DFMExtEmblemIconLayout::LocationType::TopLeft);
    // EXPECT_DEPRECATION, 0);
    // EXPECT_DEPRECATION, 0);
    
    EXPECT_EQ(result3[1].iconPath(), "icon2.png");
    EXPECT_EQ(result3[1].locationType(), DFMExtEmblemIconLayout::LocationType::TopRight);
    // EXPECT_DEPRECATION, 50);
    // EXPECT_DEPRECATION, 0);
    
    EXPECT_EQ(result3[2].iconPath(), "icon3.png");
    EXPECT_EQ(result3[2].locationType(), DFMExtEmblemIconLayout::LocationType::BottomLeft);
    // EXPECT_DEPRECATION, 0);
    // EXPECT_DEPRECATION, 50);
    
    EXPECT_EQ(result3[3].iconPath(), "icon4.png");
    EXPECT_EQ(result3[3].locationType(), DFMExtEmblemIconLayout::LocationType::BottomRight);
    // EXPECT_DEPRECATION, 50);
    // EXPECT_DEPRECATION, 50);
    
    EXPECT_EQ(result3[4].iconPath(), "icon5.png");
    EXPECT_EQ(result3[4].locationType(), DFMExtEmblemIconLayout::LocationType::Custom);
    // EXPECT_DEPRECATION, 25);
    // EXPECT_DEPRECATION, 25);
    
    // 测试重新设置（替换）
    std::vector<DFMExtEmblemIconLayout> replacementIcons;
    replacementIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::Custom, "replacement.png", 10, 10));
    emblem.setEmblem(replacementIcons);
    auto result4 = emblem.emblems();
    EXPECT_EQ(result4.size(), 1);
    EXPECT_EQ(result4[0].iconPath(), "replacement.png");
}

/**
 * @brief 测试emblems方法
 * 验证获取标志图标列表功能
 */
TEST_F(DFMExtEmblemTest, Emblems)
{
    DFMExtEmblem emblem;
    
    // 测试初始状态（空列表）
    auto initialEmblems = emblem.emblems();
    EXPECT_TRUE(initialEmblems.empty());
    
    // 设置图标后获取
    std::vector<DFMExtEmblemIconLayout> testIcons;
    testIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::TopLeft, "test1.png", 1, 1));
    testIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::TopRight, "test2.png", 2, 2));
    emblem.setEmblem(testIcons);
    
    auto retrievedEmblems = emblem.emblems();
    EXPECT_EQ(retrievedEmblems.size(), 2);
    
    // 验证返回的是数据的副本（修改返回值不应该影响原对象）
    retrievedEmblems.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::BottomRight, "extra.png", 99, 99));
    
    auto unchangedEmblems = emblem.emblems();
    EXPECT_EQ(unchangedEmblems.size(), 2); // 应该还是原来的数量
}

/**
 * @brief 测试边界条件
 * 验证在各种边界条件下的行为
 */
TEST_F(DFMExtEmblemTest, BoundaryConditions)
{
    DFMExtEmblem emblem;
    
    // 测试空路径图标
    std::vector<DFMExtEmblemIconLayout> emptyPathIcons;
    emptyPathIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::Custom, "", 0, 0));
    emblem.setEmblem(emptyPathIcons);
    auto result1 = emblem.emblems();
    EXPECT_EQ(result1.size(), 1);
    EXPECT_EQ(result1[0].iconPath(), "");
    
    // 测试长路径图标
    std::string longPath(1000, 'a');
    std::vector<DFMExtEmblemIconLayout> longPathIcons;
    longPathIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::TopLeft, longPath, 100, 100));
    emblem.setEmblem(longPathIcons);
    auto result2 = emblem.emblems();
    EXPECT_EQ(result2.size(), 1);
    EXPECT_EQ(result2[0].iconPath(), longPath);
    
    // 测试负坐标
    std::vector<DFMExtEmblemIconLayout> negativeCoordsIcons;
    negativeCoordsIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::BottomRight, "negative.png", -10, -20));
    emblem.setEmblem(negativeCoordsIcons);
    auto result3 = emblem.emblems();
    EXPECT_EQ(result3.size(), 1);
    // EXPECT_DEPRECATION, -10);
    // EXPECT_DEPRECATION, -20);
    
    // 测试大坐标值
    std::vector<DFMExtEmblemIconLayout> largeCoordsIcons;
    largeCoordsIcons.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::Custom, "large_coords.png", INT_MAX, INT_MAX));
    emblem.setEmblem(largeCoordsIcons);
    auto result4 = emblem.emblems();
    EXPECT_EQ(result4.size(), 1);
    // EXPECT_DEPRECATION, INT_MAX);
    // EXPECT_DEPRECATION, INT_MAX);
    
    // 测试所有位置类型
    std::vector<DFMExtEmblemIconLayout> allLocationTypes;
    allLocationTypes.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::TopLeft, "tl.png", 0, 0));
    allLocationTypes.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::TopRight, "tr.png", 0, 0));
    allLocationTypes.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::BottomLeft, "bl.png", 0, 0));
    allLocationTypes.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::BottomRight, "br.png", 0, 0));
    allLocationTypes.push_back(DFMExtEmblemIconLayout(
        DFMExtEmblemIconLayout::LocationType::Custom, "c.png", 0, 0));
    emblem.setEmblem(allLocationTypes);
    auto result5 = emblem.emblems();
    EXPECT_EQ(result5.size(), 5);
    EXPECT_EQ(result5[0].locationType(), DFMExtEmblemIconLayout::LocationType::TopLeft);
    EXPECT_EQ(result5[1].locationType(), DFMExtEmblemIconLayout::LocationType::TopRight);
    EXPECT_EQ(result5[2].locationType(), DFMExtEmblemIconLayout::LocationType::BottomLeft);
    EXPECT_EQ(result5[3].locationType(), DFMExtEmblemIconLayout::LocationType::BottomRight);
    EXPECT_EQ(result5[4].locationType(), DFMExtEmblemIconLayout::LocationType::Custom);
}

/**
 * @brief 测试性能
 * 验证大量数据处理的性能
 */
TEST_F(DFMExtEmblemTest, Performance)
{
    DFMExtEmblem emblem;
    
    // 创建大量图标数据
    const int iconCount = 1000;
    std::vector<DFMExtEmblemIconLayout> largeIconList;
    for (int i = 0; i < iconCount; ++i) {
        largeIconList.push_back(DFMExtEmblemIconLayout(
            static_cast<DFMExtEmblemIconLayout::LocationType>(i % 5),
            "icon_" + std::to_string(i) + ".png",
            i % 100,
            i % 100
        ));
    }
    
    // 测试设置大量图标的性能
    auto start = std::chrono::high_resolution_clock::now();
    emblem.setEmblem(largeIconList);
    auto end = std::chrono::high_resolution_clock::now();
    auto setDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(setDuration.count(), 100); // 设置1000个图标应该在100ms内完成
    
    // 测试获取大量图标的性能
    start = std::chrono::high_resolution_clock::now();
    auto retrievedIcons = emblem.emblems();
    end = std::chrono::high_resolution_clock::now();
    auto getDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(getDuration.count(), 50); // 获取1000个图标应该在50ms内完成
    
    // 验证数据完整性
    EXPECT_EQ(retrievedIcons.size(), iconCount);
    for (int i = 0; i < 10; ++i) { // 抽样验证
        EXPECT_EQ(retrievedIcons[i].iconPath(), "icon_" + std::to_string(i) + ".png");
        // EXPECT_DEPRECATION, i % 100);
        // EXPECT_DEPRECATION, i % 100);
    }
}

/**
 * @brief 测试内存安全
 * 验证内存管理的正确性
 */
TEST_F(DFMExtEmblemTest, MemorySafety)
{
    // 测试重复设置和获取不会导致内存问题
    DFMExtEmblem emblem;
    for (int i = 0; i < 100; ++i) {
        std::vector<DFMExtEmblemIconLayout> icons;
        icons.push_back(DFMExtEmblemIconLayout(
            DFMExtEmblemIconLayout::LocationType::Custom,
            "memory_test_" + std::to_string(i) + ".png",
            i, i
        ));
        emblem.setEmblem(icons);
        
        auto retrieved = emblem.emblems();
        EXPECT_EQ(retrieved.size(), 1);
        EXPECT_EQ(retrieved[0].iconPath(), "memory_test_" + std::to_string(i) + ".png");
    }
    
    // 测试大量拷贝和赋值不会导致内存问题
    std::vector<DFMExtEmblem> emblems;
    for (int i = 0; i < 50; ++i) {
        DFMExtEmblem temp;
        std::vector<DFMExtEmblemIconLayout> icons;
        icons.push_back(DFMExtEmblemIconLayout(
            DFMExtEmblemIconLayout::LocationType::TopLeft,
            "copy_test_" + std::to_string(i) + ".png",
            i, i
        ));
        temp.setEmblem(icons);
        emblems.push_back(temp); // 拷贝构造
    }
    
    // 验证所有对象的数据都正确
    for (int i = 0; i < 50; ++i) {
        auto retrieved = emblems[i].emblems();
        EXPECT_EQ(retrieved.size(), 1);
        EXPECT_EQ(retrieved[0].iconPath(), "copy_test_" + std::to_string(i) + ".png");
        // EXPECT_DEPRECATION, i);
        // EXPECT_DEPRECATION, i);
    }
}

// Note: main function is provided by main.cpp
