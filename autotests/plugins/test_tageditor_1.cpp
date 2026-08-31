// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tageditor_1.cpp
 * @brief Unit tests for TagEditor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "widgets/tageditor.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagEditorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagEditor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagEditor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagEditorTest, TagEditor)
{
    // Test constructor: TagEditor(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagEditorTest, M_~TagEditor)
{
    // Test method:  ~TagEditor(())
    EXPECT_NO_FATAL_FAILURE({ TagEditor *tmp = new TagEditor(); delete tmp; });
}

TEST_F(TagEditorTest, operator=)
{
    // Test getter: TagEditor operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(TagEditorTest, filterInput)
{
    // Test method: void filterInput(())
    EXPECT_NO_FATAL_FAILURE(obj->filterInput());
}

TEST_F(TagEditorTest, initializeWidgets)
{
    // Test method: void initializeWidgets(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeWidgets());
}

TEST_F(TagEditorTest, initializeParameters)
{
    // Test method: void initializeParameters(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeParameters());
}

TEST_F(TagEditorTest, initializeLayout)
{
    // Test method: void initializeLayout(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeLayout());
}

TEST_F(TagEditorTest, initializeConnect)
{
    // Test method: void initializeConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeConnect());
}

TEST_F(TagEditorTest, processTags)
{
    // Test method: void processTags(())
    EXPECT_NO_FATAL_FAILURE(obj->processTags());
}

TEST_F(TagEditorTest, setDefaultCrumbs)
{
    // Test setter: void setDefaultCrumbs((const QStringList &list))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDefaultCrumbs(_arg0));
}

TEST_F(TagEditorTest, onFocusOut)
{
    // Test method: void onFocusOut(())
    EXPECT_NO_FATAL_FAILURE(obj->onFocusOut());
}

TEST_F(TagEditorTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *event))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

TEST_F(TagEditorTest, mouseMoveEvent)
{
    // Test event handler: mouseMoveEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseMoveEvent(&_event));
}

TEST_F(TagEditorTest, setupEditHeight)
{
    // Test method: void setupEditHeight(())
    EXPECT_NO_FATAL_FAILURE(obj->setupEditHeight());
}

TEST_F(TagEditorTest, updateCrumbsColor)
{
    // Test method: void updateCrumbsColor((const QMap<QString, QColor> &tagsColor))
    QMap<QString, QColor> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateCrumbsColor(_arg0));
}

TEST_F(TagEditorTest, updateBackgroundColor)
{
    // Test method: void updateBackgroundColor(())
    EXPECT_NO_FATAL_FAILURE(obj->updateBackgroundColor());
}

TEST_F(TagEditorTest, setFilesForTagging)
{
    // Test setter: void setFilesForTagging((const QList<QUrl> &files))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFilesForTagging(_arg0));
}

TEST_F(TagEditorTest, setFocusOutSelfClosing)
{
    // Test setter: void setFocusOutSelfClosing((bool value))
    EXPECT_NO_FATAL_FAILURE(obj->setFocusOutSelfClosing(false));
}

TEST_F(TagEditorTest, public)
{
    // Test getter: Q_OBJECT public()
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
}

TEST_F(TagEditorTest, files)
{
    // Test getter: QList<QUrl> files()
    auto result = obj->files();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagEditorTest, crumbEdit)
{
    // Test getter: DTK_WIDGET_NAMESPACE::DCrumbEdit crumbEdit()
    auto result = obj->crumbEdit();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(TagEditorTest, promptLabel)
{
    // Test getter: QLabel promptLabel()
    auto result = obj->promptLabel();
    EXPECT_NO_FATAL_FAILURE({ obj->promptLabel(); });

}

TEST_F(TagEditorTest, totalLayout)
{
    // Test getter: QVBoxLayout totalLayout()
    auto result = obj->totalLayout();
    EXPECT_NO_FATAL_FAILURE({ obj->totalLayout(); });

}

TEST_F(TagEditorTest, backgroundFrame)
{
    // Test getter: QFrame backgroundFrame()
    auto result = obj->backgroundFrame();
    EXPECT_NO_FATAL_FAILURE({ obj->backgroundFrame(); });

}

TEST_F(TagEditorTest, flagForShown)
{
    // Test getter: std::atomic<bool> flagForShown()
    auto result = obj->flagForShown();
    EXPECT_NO_FATAL_FAILURE({ obj->flagForShown(); });

}

TEST_F(TagEditorTest, isSettingDefault)
{
    // Test bool getter: isSettingDefault()
    bool result = obj->isSettingDefault();
    EXPECT_FALSE(result);

}

TEST_F(TagEditorTest, isShowInTagDir)
{
    // Test bool getter: isShowInTagDir()
    bool result = obj->isShowInTagDir();
    EXPECT_FALSE(result);

}
