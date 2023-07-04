// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "widgets/tageditor.h"
#include "utils/tagmanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagEditorTest : public testing::Test
{

protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&TagManager::getAllTags, []() {
            __DBG_STUB_INVOKE__
            return QMap<QString, QColor>();
        });
        stub.set_lamda(&TagManager::getTagsByUrls, []() {
            __DBG_STUB_INVOKE__
            return QStringList();
        });
        ins = new TagEditor();
        ins->setFocusOutSelfClosing(true);
        ins->setFilesForTagging(QList<QUrl>() << QUrl("file://hello/world"));
        ins->setDefaultCrumbs(QStringList());
    }
    virtual void TearDown() override
    {
        stub.clear();
        if (ins) {
            delete ins;
            ins = nullptr;
        }
    }

private:
    stub_ext::StubExt stub;
    TagEditor *ins;
};

TEST_F(TagEditorTest, onFocusOut)
{
    ins->onFocusOut();
    EXPECT_TRUE(ins->close());
}

TEST_F(TagEditorTest, processTags)
{
    stub.set_lamda(&TagManager::setTagsForFiles, [](TagManager *, const QStringList &tags, const QList<QUrl> &files) {
        __DBG_STUB_INVOKE__
        EXPECT_FALSE(files.isEmpty());
        return true;
    });
    ins->processTags();
}

TEST_F(TagEditorTest, updateCrumbsColor)
{
    auto func = static_cast<bool (DCrumbEdit::*)(const DCrumbTextFormat &, int)>(&DCrumbEdit::insertCrumb);
    stub.set_lamda(func, [](DCrumbEdit *, const DCrumbTextFormat format, int) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(format.text() == "red");
        return true;
    });
    ins->updateCrumbsColor(QMap<QString, QColor>());
    QMap<QString, QColor> map;
    map["red"] = QColor("red");
    ins->updateCrumbsColor(map);
}
