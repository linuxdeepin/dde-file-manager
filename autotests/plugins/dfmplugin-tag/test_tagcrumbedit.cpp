// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/widgets/tagcrumbedit.h"

#include <gtest/gtest.h>

#include <QMouseEvent>
#include <QTextDocument>
#include <qabstracttextdocumentlayout.h>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_tag;

class UT_TagCrumbEdit : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        edit = new TagCrumbEdit();
    }

    virtual void TearDown() override
    {
        delete edit;
        edit = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TagCrumbEdit *edit = nullptr;
};

TEST_F(UT_TagCrumbEdit, constructor)
{
    // Test constructor
    EXPECT_NE(edit, nullptr);
}

TEST_F(UT_TagCrumbEdit, isEditing)
{
    // Test isEditing - should be false by default
    bool editing = edit->isEditing();

    EXPECT_FALSE(editing);
}

TEST_F(UT_TagCrumbEdit, mouseDoubleClickEvent)
{
    // Test mouse double click event
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Stub the base class method to prevent actual editing
    stub.set_lamda(VADDR(DCrumbEdit, mouseDoubleClickEvent), [](DCrumbEdit*, QMouseEvent*) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(edit->mouseDoubleClickEvent(&event));

    // After double click event completes, isEditing should be false again
    EXPECT_FALSE(edit->isEditing());
}

TEST_F(UT_TagCrumbEdit, documentMargins)
{
    // Test that document margins are set correctly in constructor
    QTextDocument *doc = edit->document();

    EXPECT_NE(doc, nullptr);
    // Document margin should be increased by 5
    EXPECT_GT(doc->documentMargin(), 0);
}

TEST_F(UT_TagCrumbEdit, viewportMargins)
{
    // Test viewport margins
    QMargins margins = edit->viewportMargins();

    EXPECT_EQ(margins.left(), 0);
    EXPECT_EQ(margins.right(), 0);
    EXPECT_EQ(margins.top(), 0);
    EXPECT_EQ(margins.bottom(), 0);
}

TEST_F(UT_TagCrumbEdit, scrollBarPolicy)
{
    // Test scroll bar policy
    Qt::ScrollBarPolicy vPolicy = edit->verticalScrollBarPolicy();

    EXPECT_EQ(vPolicy, Qt::ScrollBarAlwaysOff);
}

TEST_F(UT_TagCrumbEdit, updateHeight)
{
    // Test height update when document size changes
    stub.set_lamda(&QTextDocument::size, [](const QTextDocument*) -> QSizeF {
        __DBG_STUB_INVOKE__
        return QSizeF(100, 50);
    });

    // Trigger document size change
    QTextDocument *doc = edit->document();
    if (doc) {
        QAbstractTextDocumentLayout *layout = doc->documentLayout();
        if (layout) {
            emit layout->documentSizeChanged(QSizeF(100, 50));
        }
    }

    // Height should be updated based on document size
    EXPECT_GT(edit->height(), 0);
}
