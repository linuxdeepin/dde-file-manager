// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "dfiledialoghandle.h"
#include "views/dfiledialog.h"
#include "testhelper.h"

#include <QPointer>
#include <QWindow>
#include <QApplication>

namespace  {
    class DFileDialogHandleTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            parent = new QWidget;
            handler = new DFileDialogHandle(parent);
        }

        virtual void TearDown() override
        {
            delete handler;
            handler = nullptr;
            delete parent;
            parent = nullptr;
        }

        QWidget *parent;
        DFileDialogHandle *handler;
    };
}

//TEST_F(DFileDialogHandleTest, parent)
//{
//    TestHelper::runInLoop([](){});
//    QWidget *p = new QWidget;
//    handler->setParent(p);
//    EXPECT_TRUE(handler->parent() == p);
//}

//TEST_F(DFileDialogHandleTest, set_directory)
//{
//    QDir dir("/etc/apt/");
//    handler->setDirectory(dir);
//    EXPECT_TRUE(handler->directory() == dir);
//}

//TEST_F(DFileDialogHandleTest, set_dir_string)
//{
//    const QString dir("/etc/apt/");
//    handler->setDirectory(dir);
//    EXPECT_TRUE(handler->directory() == dir);
//}

//TEST_F(DFileDialogHandleTest, set_directory_url)
//{
//    QUrl url("trash://");
//    handler->setDirectoryUrl(url);
//    EXPECT_TRUE(handler->directoryUrl() == url);
//}

//TEST_F(DFileDialogHandleTest, select_file)
//{
//    handler->selectFile("/home");
//    EXPECT_TRUE(handler->selectedFiles().empty());
//}

//TEST_F(DFileDialogHandleTest, select_uri)
//{
//    handler->selectUrl(QUrl(""));
//    handler->selectUrl(QUrl("/home"));
//    EXPECT_TRUE(handler->selectedUrls().empty());
//}

//TEST_F(DFileDialogHandleTest, add_disable_url_scheme)
//{
//    const QString scheme = "file://";
//    handler->addDisableUrlScheme(scheme);
//}

//TEST_F(DFileDialogHandleTest, set_name_filters)
//{
//    const QStringList filters {"exclude"};
//    handler->setNameFilters(filters);
//    EXPECT_TRUE(handler->nameFilters() == filters);
//}

//TEST_F(DFileDialogHandleTest, select_name_filter)
//{
//    QString filter {"exclude"};
//    handler->selectNameFilter(filter);
//}

//TEST_F(DFileDialogHandleTest, select_name_filters_by_index)
//{
//    QList<QString> filters;
//    filters << "are" << "you" << "ok";
//    handler->setNameFilters(filters);
//    int index = 1;
//    handler->selectNameFilterByIndex(index);
//    EXPECT_TRUE(handler->selectedNameFilterIndex() == index);
//}

//TEST_F(DFileDialogHandleTest, set_filter)
//{
//    auto filters = QDir::Dirs | QDir::Files;
//    handler->setFilter(filters);
//    EXPECT_TRUE(handler->filter() == filters);
//}

//TEST_F(DFileDialogHandleTest, set_view_mode)
//{
//    QFileDialog::ViewMode mode = QFileDialog::ViewMode::List;
//    handler->setViewMode(mode);
//    EXPECT_TRUE(handler->viewMode() == mode);
//    mode = QFileDialog::ViewMode::Detail;
//    handler->setViewMode(mode);
//    EXPECT_TRUE(handler->viewMode() == mode);
//}

//TEST_F(DFileDialogHandleTest, set_file_mode)
//{
//    handler->setFileMode(QFileDialog::FileMode::AnyFile);
//}

//TEST_F(DFileDialogHandleTest, set_accept_mode)
//{
//    QFileDialog::AcceptMode mode = QFileDialog::AcceptMode::AcceptOpen;
//    auto mode2 = handler->acceptMode();
//    EXPECT_TRUE(mode2 == mode);
//}

//TEST_F(DFileDialogHandleTest, set_lable_text)
//{
//    QFileDialog::DialogLabel label = QFileDialog::Reject;
//    QString txt("test");
//    handler->setLabelText(label, txt);
//    QString result = handler->labelText(label);
//    EXPECT_EQ(txt, result);
//}

//TEST_F(DFileDialogHandleTest, set_options)
//{
//    QFileDialog::Options options = QFileDialog::ShowDirsOnly | QFileDialog::HideNameFilterDetails;
//    handler->setOptions(options);
//    QFileDialog::Options result = handler->options();
//    EXPECT_EQ(options, result);
//}

//TEST_F(DFileDialogHandleTest, test_option)
//{
//    QFileDialog::Options options = QFileDialog::ShowDirsOnly | QFileDialog::HideNameFilterDetails;
//    handler->setOptions(options);
//    bool result = handler->testOption(QFileDialog::ShowDirsOnly);
//    EXPECT_EQ(result, true);
//}

//TEST_F(DFileDialogHandleTest, set_current_input_name)
//{
//    QString name("hello");
//    handler->setCurrentInputName(name);
//}

//TEST_F(DFileDialogHandleTest, custom_widgets)
//{
//    handler->addCustomWidget(0, "0");
//    handler->addCustomWidget(1, "1");
//    handler->getCustomWidgetValue(0, "0");
//    handler->getCustomWidgetValue(1, "1");
//    auto result = handler->allCustomWidgetsValue(1);
//    EXPECT_TRUE(!result.empty());
//}

//TEST_F(DFileDialogHandleTest, begin_end_add_custom_widget)
//{
//    handler->beginAddCustomWidget();
//    handler->endAddCustomWidget();
//}

//TEST_F(DFileDialogHandleTest, set_hide_on_accept)
//{
//    bool on = true;
//    handler->setHideOnAccept(on);
//    EXPECT_EQ(on, handler->hideOnAccept());
//}

//TEST_F(DFileDialogHandleTest, test_show)
//{
//    handler->show();
//}

//TEST_F(DFileDialogHandleTest, test_hide)
//{
//    handler->hide();
//}

//TEST_F(DFileDialogHandleTest, test_accept)
//{
//    handler->accept();
//}

//TEST_F(DFileDialogHandleTest, test_done)
//{
//    handler->done(0);
//    handler->done(1);
//}

//TEST_F(DFileDialogHandleTest, test_open)
//{
//    handler->open();
//}

//TEST_F(DFileDialogHandleTest, test_reject)
//{
//    handler->reject();
//}
