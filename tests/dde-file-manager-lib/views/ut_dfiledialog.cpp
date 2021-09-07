#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "interfaces/dfilesystemmodel.h"
#include "views/dfmsidebar.h"
#include "interfaces/dfmstandardpaths.h"
#include "interfaces/dfilemenumanager.h"
#include "stub.h"
#include <QEventLoop>
#include <QLineEdit>
#include <DDialog>
#include "../stub-ext/stubext.h"
#include "../views/private/dfiledialog_p.h"

#define private public
#define protected public
#include "views/dfiledialog.h"


namespace  {
    class TestDFileDialog : public testing::Test
    {
    public:
        DFileDialog* m_fileDialog;

        virtual void SetUp() override
        {
            stub_ext::StubExt stu;
            stu.set_lamda(ADDR(DFileMenuManager, needDeleteAction), [](){return true;});

            m_fileDialog = new DFileDialog();
            m_fileDialog->setDirectory(DUrl::fromLocalFile("/usr").toLocalFile());
            m_fileDialog->openNewTab(DUrl::fromLocalFile("/usr"));

            std::cout << "start TestDFileDialog" << std::endl;
        }

        virtual void TearDown() override
        {
            m_fileDialog->clearActions();

            delete m_fileDialog;
            m_fileDialog = nullptr;

            eventloop();

            std::cout << "end TestDFileDialog" << std::endl;
        }

        void eventloop()
        {
            QEventLoop loop;
            QTimer::singleShot(10, nullptr, [&loop]() {
                loop.exit();
            });
            loop.exec();
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
    m_fileDialog->setDirectoryUrl(DUrl(QUrl::fromLocalFile("/home")));
    QUrl url = m_fileDialog->directoryUrl();

    QString p = url.path();
    EXPECT_EQ(url.path(), "/home");
}

TEST_F(TestDFileDialog, tst_set_get_selectFile)
{
    m_fileDialog->show();

    QList<QUrl> (*st_selectedUrls)() = []()->QList<QUrl> {
        QList<QUrl> urls;
        urls << QUrl("/home");
        urls << QUrl("/");
        return  urls;
    };

    Stub stub;
    stub.set(ADDR(DFileDialog, selectedUrls), st_selectedUrls);
    m_fileDialog->selectedFiles();
}

TEST_F(TestDFileDialog, tst_set_get_selectUrl)
{
    m_fileDialog->selectUrl(QUrl(""));
    m_fileDialog->setAcceptMode(QFileDialog::AcceptSave);

    m_fileDialog->setDirectory(DFMStandardPaths::location(DFMStandardPaths::HomePath));

#if 0 // stub will be broken.
    QModelIndexList (*st_selectedIndexes)() = []()->QModelIndexList{
        QModelIndexList indexList;
        indexList << QModelIndex();
        return indexList;
    };
    Stub stub;
    stub.set(ADDR(DFileView, selectedIndexes), st_selectedIndexes);
#endif

    m_fileDialog->selectedUrls();
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

    m_fileDialog->setAcceptMode(QFileDialog::AcceptSave);

    m_fileDialog->selectNameFilterByIndex(1);
    selectedIndex = m_fileDialog->selectedNameFilterIndex();
    EXPECT_EQ(1, selectedIndex);

    m_fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    m_fileDialog->setFileMode(QFileDialog::DirectoryOnly);
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
#if 0
TEST_F(TestDFileDialog, tst_set_get_viewMode)
{
#if 0 // may cause crash
    m_fileDialog->setViewMode(DFileView::IconMode);
    EXPECT_EQ(DFileView::IconMode, m_fileDialog->viewMode());
#endif
}

TEST_F(TestDFileDialog, tst_set_setFileMode)
{
#if 0 // may cause crash
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->setFileMode(QFileDialog::AnyFile));
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->setFileMode(QFileDialog::DirectoryOnly));
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->setFileMode(QFileDialog::ExistingFiles));
#endif
}
#endif
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

    m_fileDialog->setLabelText(QFileDialog::Reject, "label");
    EXPECT_EQ("label", m_fileDialog->labelText(QFileDialog::Reject));
}

TEST_F(TestDFileDialog, tst_options)
{
#if 0 // may cause crash
    m_fileDialog->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    EXPECT_EQ(QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly, m_fileDialog->options());

    m_fileDialog->setOption(QFileDialog::ShowDirsOnly, false);
    EXPECT_FALSE(m_fileDialog->testOption(QFileDialog::ShowDirsOnly));

    m_fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
    EXPECT_TRUE(m_fileDialog->testOption(QFileDialog::ShowDirsOnly));
#endif
}

TEST_F(TestDFileDialog, tst_setCurrentInputName)
{
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->setCurrentInputName("/home"));
}

TEST_F(TestDFileDialog, tst_customWidget)
{
    m_fileDialog->beginAddCustomWidget();
    //! add widget.
    m_fileDialog->addCustomWidget(DFileDialog::LineEditType, "");
    m_fileDialog->addCustomWidget(DFileDialog::ComboBoxType, "");
    m_fileDialog->endAddCustomWidget();
    EXPECT_FALSE(m_fileDialog->getCustomWidgetValue(DFileDialog::LineEditType, "").isNull());
    EXPECT_TRUE(m_fileDialog->getCustomWidgetValue(DFileDialog::ComboBoxType, "").isNull());
    EXPECT_FALSE(m_fileDialog->allCustomWidgetsValue(DFileDialog::LineEditType).isEmpty());
    EXPECT_FALSE(m_fileDialog->allCustomWidgetsValue(DFileDialog::ComboBoxType).isEmpty());
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

TEST_F(TestDFileDialog, tst_acceptButtonClicked)
{
    m_fileDialog->setFileMode(QFileDialog::AnyFile);
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->onAcceptButtonClicked());
    m_fileDialog->setFileMode(QFileDialog::ExistingFiles);
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->onAcceptButtonClicked());
    m_fileDialog->setFileMode(QFileDialog::Directory);
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->onAcceptButtonClicked());

    // replace QLineEdit::text
    QString (*st_text)() = []()->QString {
         return "123.file";
    };
    Stub stub;
    stub.set(ADDR(QLineEdit, text), st_text);

    m_fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    m_fileDialog->onAcceptButtonClicked();

    // replace QLineEdit::text
    QString (*st_text_2)() = []()->QString {
         return ".file";
    };
    stub.set(ADDR(QLineEdit, text), st_text_2);

    // replace DDialog::exec
    int (*st_exec)() = []()->int{
        // do nothing.
        return DDialog::Accepted;
    };
    stub_ext::StubExt stubext;
    stubext.set(VADDR(DDialog, exec), st_exec);

    m_fileDialog->setOption(QFileDialog::DontConfirmOverwrite);
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->onAcceptButtonClicked());

    m_fileDialog->onAcceptButtonClicked();
}

TEST_F(TestDFileDialog, tst_onCurrentInputNameChanged)
{
    m_fileDialog->onCurrentInputNameChanged();
}

TEST_F(TestDFileDialog, tst_handleEnterPressed)
{
    m_fileDialog->handleEnterPressed();
}

TEST_F(TestDFileDialog, tst_updateAcceptButtonState)
{
    m_fileDialog->updateAcceptButtonState();
}

TEST_F(TestDFileDialog, tst_exec)
{
    int (*st_exec)(QEventLoop::ProcessEventsFlags) = [](QEventLoop::ProcessEventsFlags)->int {
        // do nothing.
        return 0;
    };


    Stub stub;
    stub.set(ADDR(QEventLoop, exec), st_exec);
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->exec());
}

TEST_F(TestDFileDialog, tst_closeEvent)
{
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->show());
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->close());
}

TEST_F(TestDFileDialog, tst_eventFilter)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_T, Qt::ControlModifier);

    QWindow* (*st_windowHandle)() = []()->QWindow* {
        return nullptr;
    };
    Stub stub;
    stub.set(ADDR(DFileDialog, windowHandle), st_windowHandle);

    m_fileDialog->eventFilter(nullptr, &event);

    QKeyEvent event2(QEvent::KeyPress, QKeySequence::Cancel, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->eventFilter(nullptr, &event2));

    QKeyEvent event3(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->eventFilter(nullptr, &event3));
}

TEST_F(TestDFileDialog, tst_fmEventFilter)
{
    QSharedPointer<DFMEvent> pointer = dMakeEventPointer<DFMEvent>(DFMEvent::OpenFile, nullptr);

    bool (*st_isActiveWindow)() = []()->bool {
        return true;
    };

    Stub stub;
    stub.set(ADDR(DFileDialog, isActiveWindow), st_isActiveWindow);
    m_fileDialog->fmEventFilter(pointer, nullptr, nullptr);
    pointer->setType(DFMEvent::OpenFiles);
    m_fileDialog->fmEventFilter(pointer, nullptr, nullptr);
    pointer->setType(DFMEvent::OpenFileByApp);
    m_fileDialog->fmEventFilter(pointer, nullptr, nullptr);
}

TEST_F(TestDFileDialog, tst_handleNewView)
{
    DFileView view;
    m_fileDialog->handleNewView(&view);
    view.setRootUrl(DUrl("/home"));
    view.selectAll();
    QEventLoop loop;
    QTimer::singleShot(100, nullptr, [&loop]{
        loop.exit();
    });
    loop.exec();
}

TEST_F(TestDFileDialog, tst_selectFile)
{
    m_fileDialog->selectFile("/home");
}

TEST_F(TestDFileDialog, tst_get_directory)
{
    EXPECT_FALSE(m_fileDialog->directoryUrl().isEmpty());
}

TEST_F(TestDFileDialog, tst_onRejectButtonClicked)
{
    EXPECT_NO_FATAL_FAILURE(m_fileDialog->onRejectButtonClicked());
}

TEST_F(TestDFileDialog, tst_selectedUrls)
{
    m_fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    m_fileDialog->selectedUrls();
    m_fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    m_fileDialog->selectedUrls();
}

TEST_F(TestDFileDialog, tst_orderedSelectedUrls)
{
    m_fileDialog->d_ptr->orderedSelectedList << QModelIndex() << QModelIndex();
    EXPECT_TRUE(m_fileDialog->d_ptr->orderedSelectedUrls().size() == 0);

    QModelIndexList (*st_selectedIndexes)() = []()->QModelIndexList {
        QModelIndexList index;
        index.append(QModelIndex());
        index.append(QModelIndex());
        return index;
    };
    stub_ext::StubExt stubext;
    stubext.set(VADDR(DFileView, selectedIndexes), st_selectedIndexes);

    m_fileDialog->d_ptr->orderedSelectedList << QModelIndex();
    m_fileDialog->d_ptr->orderedSelectedUrls();
    EXPECT_TRUE(m_fileDialog->d_ptr->orderedSelectedUrls().size() == 0);
}

TEST_F(TestDFileDialog, tst_adjustPostion)
{
    QPoint pt0 = m_fileDialog->pos();

    QWidget w;
    w.move(100,100);
    m_fileDialog->adjustPosition(&w);
    QPoint pt1 = m_fileDialog->pos();

    EXPECT_NE(pt0, pt1);
}

