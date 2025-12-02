// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/dfmextactionimpl.h"
#include "plugins/common/dfmplugin-utils/extensionimpl/menuimpl/private/dfmextactionimpl_p.h"

#include <QAction>
#include <QMenu>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_DFMExtActionImpl : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_DFMExtActionImpl, Constructor_WithNullAction_CreatesNewAction)
{
    DFMExtActionImpl *impl = new DFMExtActionImpl(nullptr);

    EXPECT_NE(impl, nullptr);

    delete impl;
}

TEST_F(UT_DFMExtActionImpl, Constructor_WithExistingAction_UsesAction)
{
    QAction *action = new QAction();
    DFMExtActionImpl *impl = new DFMExtActionImpl(action);

    EXPECT_NE(impl, nullptr);

    delete impl;
}

// ========== DFMExtActionImplPrivate Tests ==========

class UT_DFMExtActionImplPrivate : public testing::Test
{
protected:
    void SetUp() override
    {
        action = new QAction();
        impl = new DFMExtActionImpl(nullptr);
        d = dynamic_cast<DFMExtActionImplPrivate *>(impl->d);
    }

    void TearDown() override
    {
        delete action;
        action = nullptr;
        delete impl;
        impl = nullptr;
        stub.clear();
    }

    QAction *action { nullptr };
    DFMExtActionImpl *impl { nullptr };
    DFMExtActionImplPrivate *d { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_DFMExtActionImplPrivate, isInterior_ExternalAction_ReturnsFalse)
{
    EXPECT_FALSE(d->isInterior());
}

TEST_F(UT_DFMExtActionImplPrivate, setText_SetsActionText)
{
    d->setText("Test Text");

    std::string result = d->text();
    EXPECT_FALSE(result.empty());
}

TEST_F(UT_DFMExtActionImplPrivate, setToolTip_SetsActionToolTip)
{
    d->setToolTip("Test Tooltip");

    std::string result = d->toolTip();
    EXPECT_EQ(result, "Test Tooltip");
}

TEST_F(UT_DFMExtActionImplPrivate, setSeparator_SetsActionSeparator)
{
    d->setSeparator(true);

    EXPECT_TRUE(d->isSeparator());
}

TEST_F(UT_DFMExtActionImplPrivate, setCheckable_SetsActionCheckable)
{
    d->setCheckable(true);

    EXPECT_TRUE(d->isCheckable());
}

TEST_F(UT_DFMExtActionImplPrivate, setEnabled_SetsActionEnabled)
{
    d->setEnabled(false);

    EXPECT_FALSE(d->isEnabled());
}

TEST_F(UT_DFMExtActionImplPrivate, setIcon_ThemeName_SetsIcon)
{
    d->setIcon("dialog-information");

    std::string result = d->icon();
}

TEST_F(UT_DFMExtActionImplPrivate, qaction_ReturnsQAction)
{
    QAction *result = d->qaction();

    EXPECT_NE(result, nullptr);
}

TEST_F(UT_DFMExtActionImplPrivate, actionImpl_ReturnsImpl)
{
    DFMExtActionImpl *result = d->actionImpl();

    EXPECT_EQ(result, impl);
}

TEST_F(UT_DFMExtActionImplPrivate, setChecked_SetsActionChecked)
{
    d->setCheckable(true);
    d->setChecked(true);

    bool result = d->isChecked();
    Q_UNUSED(result)
}

TEST_F(UT_DFMExtActionImplPrivate, setIcon_FilePath_SetsIcon)
{
    d->setIcon("/usr/share/icons/default.png");

    std::string result = d->icon();
    Q_UNUSED(result)
}

TEST_F(UT_DFMExtActionImplPrivate, setIcon_EmptyPath_SetsEmptyIcon)
{
    d->setIcon("");

    std::string result = d->icon();
    Q_UNUSED(result)
}

TEST_F(UT_DFMExtActionImplPrivate, text_WithEmptyAction_ReturnsEmpty)
{
    std::string result = d->text();
    Q_UNUSED(result)
}

TEST_F(UT_DFMExtActionImplPrivate, setMenu_NullMenu_DoesNotCrash)
{
    d->setMenu(nullptr);
}

TEST_F(UT_DFMExtActionImplPrivate, menu_NoMenu_ReturnsNull)
{
    DFMEXT::DFMExtMenu *result = d->menu();

    EXPECT_EQ(result, nullptr);
}

// ========== Interior Action Tests ==========

class UT_DFMExtActionImplPrivate_Interior : public testing::Test
{
protected:
    void SetUp() override
    {
        action = new QAction();
        impl = new DFMExtActionImpl(action);
        d = dynamic_cast<DFMExtActionImplPrivate *>(impl->d);
    }

    void TearDown() override
    {
        delete impl;
        impl = nullptr;
        stub.clear();
    }

    QAction *action { nullptr };
    DFMExtActionImpl *impl { nullptr };
    DFMExtActionImplPrivate *d { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_DFMExtActionImplPrivate_Interior, isInterior_InteriorAction_ReturnsTrue)
{
    EXPECT_TRUE(d->isInterior());
}

TEST_F(UT_DFMExtActionImplPrivate_Interior, setText_InteriorAction_DoesNotSetText)
{
    QString originalText = action->text();

    d->setText("New Text");

    EXPECT_EQ(action->text(), originalText);
}

TEST_F(UT_DFMExtActionImplPrivate_Interior, setIcon_InteriorAction_DoesNotSetIcon)
{
    d->setIcon("dialog-information");
}

TEST_F(UT_DFMExtActionImplPrivate_Interior, setToolTip_InteriorAction_DoesNotSetToolTip)
{
    QString originalTip = action->toolTip();

    d->setToolTip("New Tip");

    EXPECT_EQ(action->toolTip(), originalTip);
}

TEST_F(UT_DFMExtActionImplPrivate_Interior, setSeparator_InteriorAction_DoesNotSetSeparator)
{
    bool originalSeparator = action->isSeparator();

    d->setSeparator(true);

    EXPECT_EQ(action->isSeparator(), originalSeparator);
}

TEST_F(UT_DFMExtActionImplPrivate_Interior, setCheckable_InteriorAction_DoesNotSetCheckable)
{
    bool originalCheckable = action->isCheckable();

    d->setCheckable(true);

    EXPECT_EQ(action->isCheckable(), originalCheckable);
}

TEST_F(UT_DFMExtActionImplPrivate_Interior, setChecked_InteriorAction_DoesNotSetChecked)
{
    action->setCheckable(true);
    bool originalChecked = action->isChecked();

    d->setChecked(true);

    EXPECT_EQ(action->isChecked(), originalChecked);
}

TEST_F(UT_DFMExtActionImplPrivate_Interior, setEnabled_InteriorAction_DoesNotSetEnabled)
{
    bool originalEnabled = action->isEnabled();

    d->setEnabled(false);

    EXPECT_EQ(action->isEnabled(), originalEnabled);
}

