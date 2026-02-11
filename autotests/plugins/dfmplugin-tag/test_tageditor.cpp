// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/widgets/tageditor.h"
#include "plugins/common/dfmplugin-tag/utils/tagmanager.h"
#include "plugins/common/dfmplugin-tag/utils/taghelper.h"

#include <dfm-base/utils/windowutils.h>

#include <gtest/gtest.h>

#include <QUrl>
#include <QColor>
#include <DCrumbEdit>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_tag;

class UT_TagEditor : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&dfmbase::WindowUtils::isWayLand, []() -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        stub.set_lamda(&TagManager::getTagsColor, [](TagManager *, const QStringList &) -> QMap<QString, QColor> {
            __DBG_STUB_INVOKE__
            QMap<QString, QColor> map;
            map["Tag1"] = QColor("#ffa503");
            return map;
        });

        stub.set_lamda(&TagManager::assignColorToTags, [](TagManager *, const QStringList &) -> QMap<QString, QColor> {
            __DBG_STUB_INVOKE__
            QMap<QString, QColor> map;
            map["NewTag"] = QColor("#ffa503");
            return map;
        });

        stub.set_lamda(&TagManager::setTagsForFiles, [](TagManager *, const QStringList &, const QList<QUrl> &) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        editor = new TagEditor();
    }

    virtual void TearDown() override
    {
        if (editor) {
            editor->deleteLater();
            editor = nullptr;
        }
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TagEditor *editor = nullptr;
};

TEST_F(UT_TagEditor, constructor)
{
    // Test constructor
    EXPECT_NE(editor, nullptr);
}

TEST_F(UT_TagEditor, setFocusOutSelfClosing)
{
    // Test setting focus out self closing
    EXPECT_NO_THROW(editor->setFocusOutSelfClosing(true));
    EXPECT_NO_THROW(editor->setFocusOutSelfClosing(false));
}

TEST_F(UT_TagEditor, setFilesForTagging)
{
    // Test setting files for tagging
    QList<QUrl> files;
    files << QUrl::fromLocalFile("/home/user/test1.txt");
    files << QUrl::fromLocalFile("/home/user/test2.txt");

    EXPECT_NO_THROW(editor->setFilesForTagging(files));
}

TEST_F(UT_TagEditor, setDefaultCrumbs)
{
    // Test setting default crumbs
    QStringList list;
    list << "Tag1"
         << "Tag2";

    stub.set_lamda(&DCrumbEdit::clear, [] {
        __DBG_STUB_INVOKE__
    });

    typedef bool (DCrumbEdit::*InsertCrumb)(const DCrumbTextFormat &, int);
    stub.set_lamda(static_cast<InsertCrumb>(&DCrumbEdit::insertCrumb), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_THROW(editor->setDefaultCrumbs(list));
}

TEST_F(UT_TagEditor, onFocusOut)
{
    // Test focus out slot
    editor->setFocusOutSelfClosing(true);

    stub.set_lamda(&DCrumbEdit::toPlainText, [] {
        __DBG_STUB_INVOKE__
        return "NewTag";
    });

    typedef bool (DCrumbEdit::*AppendCrumb)(const QString &);
    stub.set_lamda(static_cast<AppendCrumb>(&DCrumbEdit::appendCrumb), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DCrumbEdit::crumbList, [](const DCrumbEdit *) -> QStringList {
        __DBG_STUB_INVOKE__
        QStringList list;
        list << "NewTag";
        return list;
    });

    stub.set_lamda(&DCrumbEdit::clear, [] {
        __DBG_STUB_INVOKE__
    });

    typedef bool (DCrumbEdit::*InsertCrumb)(const DCrumbTextFormat &, int);
    stub.set_lamda(static_cast<InsertCrumb>(&DCrumbEdit::insertCrumb), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool closed = false;
    stub.set_lamda(&TagEditor::close, [&closed] {
        __DBG_STUB_INVOKE__
        closed = true;
        return true;
    });

    EXPECT_NO_THROW(editor->onFocusOut());
    EXPECT_TRUE(closed);
}

TEST_F(UT_TagEditor, filterInput)
{
    // Test filter input
    stub.set_lamda(&TagHelper::crumbEditInputFilter, [](TagHelper *, DCrumbEdit *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(editor->filterInput());
}

TEST_F(UT_TagEditor, keyPressEvent)
{
    // Test key press event - ESC key
    stub.set_lamda(&DCrumbEdit::crumbList, [](const DCrumbEdit *) -> QStringList {
        __DBG_STUB_INVOKE__
        return QStringList();
    });

    stub.set_lamda(&DCrumbEdit::clear, [] {
        __DBG_STUB_INVOKE__
    });

    bool closed = false;
    stub.set_lamda(&TagEditor::close, [&closed] {
        __DBG_STUB_INVOKE__
        closed = true;
        return true;
    });

    QKeyEvent escEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_NO_THROW(editor->keyPressEvent(&escEvent));
    EXPECT_TRUE(closed);
}

TEST_F(UT_TagEditor, constructorWithInTagDir)
{
    // Test constructor with inTagDir parameter
    stub.set_lamda(&dfmbase::WindowUtils::isWayLand, []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    TagEditor *tagDirEditor = nullptr;
    EXPECT_NO_THROW(tagDirEditor = new TagEditor(nullptr, true));
    EXPECT_NE(tagDirEditor, nullptr);

    if (tagDirEditor) {
        tagDirEditor->deleteLater();
    }
}
