#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "views/dfiledialog.h"
#include "interfaces/dfilesystemmodel.h"
#include "views/dfmsidebar.h"


namespace  {
    class TestDFileDialog : public testing::Test
    {
    public:
        QSharedPointer<DFileDialog> m_fileDialog;

        virtual void SetUp() override
        {
            m_fileDialog = QSharedPointer<DFileDialog>(new DFileDialog());
            m_fileDialog->setDirectory(DUrl::fromLocalFile("/usr").toLocalFile());
            m_fileDialog->openNewTab(DUrl::fromLocalFile("/usr"));

            std::cout << "start TestDFileDialog" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestDFileDialog" << std::endl;
        }
    };
}

TEST_F(TestDFileDialog, can_set_get_dir)
{
    QDir dir = m_fileDialog->directory();
    EXPECT_TRUE(dir.exists());
    EXPECT_EQ("/usr", dir.path());

    m_fileDialog->setDirectory("/home");
    EXPECT_EQ("/home", m_fileDialog->directory().path());
}

TEST_F(TestDFileDialog, can_set_get_url)
{
    m_fileDialog->setDirectoryUrl(QUrl::fromLocalFile("/home"));
    QUrl url = m_fileDialog->directoryUrl();

    QString p = url.path();
    EXPECT_EQ(url.path(), "/home");
}

TEST_F(TestDFileDialog, tst_set_get_selectFile)
{
}

TEST_F(TestDFileDialog, tst_set_get_selectUrl)
{
}

TEST_F(TestDFileDialog, tst_addDisableUrlScheme)
{
    m_fileDialog->addDisableUrlScheme(COMPUTER_SCHEME);
    EXPECT_TRUE(m_fileDialog->getLeftSideBar()->disableUrlSchemes().contains(COMPUTER_SCHEME));
}

TEST_F(TestDFileDialog, tst_set_get_NameFilters)
{
    QStringList filters;
    filters << "exclude";
    m_fileDialog->setNameFilters(filters);
    EXPECT_EQ(filters, m_fileDialog->nameFilters());
}

TEST_F(TestDFileDialog, tst_selectNameFilter)
{
    QStringList filters;
    filters << "f0" << "f1";
    m_fileDialog->setNameFilters(filters);

    m_fileDialog->selectNameFilter("f0");
    EXPECT_EQ("f0", m_fileDialog->selectedNameFilter());

    m_fileDialog->selectNameFilter("f1");
    EXPECT_EQ("f1", m_fileDialog->selectedNameFilter());
}

TEST_F(TestDFileDialog, tst_modelCurrentNameFilter)
{
    QStringList filters;
    filters << "f0" << "f1";
    m_fileDialog->setNameFilters(filters);

    m_fileDialog->selectNameFilter("f0");

    EXPECT_EQ("f0", m_fileDialog->modelCurrentNameFilter());
}

TEST_F(TestDFileDialog, tst_set_get_fileterIndex)
{
    QStringList filters;
    filters << "f0" << "f1";
    m_fileDialog->setNameFilters(filters);

    m_fileDialog->selectNameFilterByIndex(0);
    int selectedIndex = m_fileDialog->selectedNameFilterIndex();
    EXPECT_EQ(static_cast<int>(0), selectedIndex);

    m_fileDialog->selectNameFilterByIndex(1);
    selectedIndex = m_fileDialog->selectedNameFilterIndex();
    EXPECT_EQ(1, selectedIndex);
}

TEST_F(TestDFileDialog, tst_set_get_filter)
{
    QDir::Filters filter;
    filter.setFlag(QDir::Files);
    m_fileDialog->setFilter(filter);
    EXPECT_EQ(filter, m_fileDialog->filter());
}

TEST_F(TestDFileDialog, tst_set_get_viewMode)
{
    m_fileDialog->setViewMode(DFileView::IconMode);
    EXPECT_EQ(DFileView::IconMode, m_fileDialog->viewMode());
}

TEST_F(TestDFileDialog, tst_set_setFileMode)
{
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->setFileMode(QFileDialog::AnyFile));
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->setFileMode(QFileDialog::DirectoryOnly));
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->setFileMode(QFileDialog::ExistingFiles));
}

TEST_F(TestDFileDialog, tst_setAllowMixedSelection)
{
    m_fileDialog->setAllowMixedSelection(true);
}

TEST_F(TestDFileDialog, tst_set_get_AcceptMode)
{
    m_fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    EXPECT_EQ(QFileDialog::AcceptOpen, m_fileDialog->acceptMode());
}

TEST_F(TestDFileDialog, tst_set_get_labelText)
{
    m_fileDialog->setLabelText(QFileDialog::Accept, "label");
    EXPECT_EQ("label", m_fileDialog->labelText(QFileDialog::Accept));
}

TEST_F(TestDFileDialog, tst_options)
{
    m_fileDialog->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    EXPECT_EQ(QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly, m_fileDialog->options());

    m_fileDialog->setOption(QFileDialog::ShowDirsOnly, false);
    EXPECT_FALSE(m_fileDialog->testOption(QFileDialog::ShowDirsOnly));

    m_fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
    EXPECT_TRUE(m_fileDialog->testOption(QFileDialog::ShowDirsOnly));
}

TEST_F(TestDFileDialog, tst_setCurrentInputName)
{
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->setCurrentInputName("/home"));
}

TEST_F(TestDFileDialog, tst_customWidget)
{
    m_fileDialog->beginAddCustomWidget();
    //! add widget.
    m_fileDialog->endAddCustomWidget();
    EXPECT_TRUE(m_fileDialog->getCustomWidgetValue(DFileDialog::LineEditType, "").isNull());
    EXPECT_TRUE(m_fileDialog->getCustomWidgetValue(DFileDialog::ComboBoxType, "").isNull());
    EXPECT_TRUE(m_fileDialog->allCustomWidgetsValue(DFileDialog::LineEditType).isEmpty());
}

TEST_F(TestDFileDialog, tst_set_get_onAccept)
{
    m_fileDialog->setHideOnAccept(true);
    EXPECT_TRUE(m_fileDialog->hideOnAccept());
}

TEST_F(TestDFileDialog, tst_getFileView)
{
    EXPECT_NE(nullptr, m_fileDialog->getFileView());
}

//! public solots:
TEST_F(TestDFileDialog, tst_slots)
{
    m_fileDialog->show();
    EXPECT_FALSE(m_fileDialog->isHidden());
    m_fileDialog->reject();
    EXPECT_TRUE(m_fileDialog->isHidden());
}

