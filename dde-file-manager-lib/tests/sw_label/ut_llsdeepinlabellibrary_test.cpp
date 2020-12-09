#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#define private public
#define protected public

#include "sw_label/llsdeepinlabellibrary.h"
#include "sw_label/llsdeepinlabellibrary.cpp"

using namespace testing;
namespace  {
    class LlsDeepinLabelLibraryTest : public Test
    {
    public:
        LlsDeepinLabelLibraryTest():Test()
        {

        }

        virtual void SetUp() override {
            p_library = new LlsDeepinLabelLibrary;
        }

        virtual void TearDown() override {
            delete p_library;
            p_library = nullptr;
        }

        LlsDeepinLabelLibrary * p_library;
    };
}

TEST_F(LlsDeepinLabelLibraryTest, get_library_name)
{
    QString result = p_library->LibraryName();
    QString expectResult("/usr/lib/sw_64-linux-gnu/dde-file-manager/libllsdeeplabel.so");

    EXPECT_EQ(result, expectResult);
}

TEST_F(LlsDeepinLabelLibraryTest, load_library)
{
    QString name("/usr/lib/sw_64-linux-gnu/dde-file-manager/libllsdeeplabel.so");
    p_library->loadLibrary(name);

    name = "libz";
    p_library->loadLibrary(name);
}

TEST_F(LlsDeepinLabelLibraryTest, check_simple_label)
{
    LlsDeepinLabelLibrary::func_lls_simplechecklabel result = p_library->lls_simplechecklabel();
    EXPECT_EQ(result, p_library->m_lls_simplechecklabel);
}

TEST_F(LlsDeepinLabelLibraryTest, set_simple_label)
{
    LlsDeepinLabelLibrary::func_lls_simplechecklabel result = p_library->lls_simplechecklabel();
    p_library->setLls_simplechecklabel(nullptr);
    EXPECT_EQ(nullptr, p_library->m_lls_simplechecklabel);
    p_library->setLls_simplechecklabel(result);
}

TEST_F(LlsDeepinLabelLibraryTest, get_fullcheck_label)
{
    LlsDeepinLabelLibrary::func_lls_fullchecklabel result = p_library->lls_fullchecklabel();
    EXPECT_EQ(result, p_library->m_lls_fullchecklabel);
}

TEST_F(LlsDeepinLabelLibraryTest, set_fullcheck_label)
{
    LlsDeepinLabelLibrary::func_lls_fullchecklabel result = p_library->lls_fullchecklabel();
    p_library->setLls_fullchecklabel(nullptr);
    EXPECT_EQ(nullptr, p_library->m_lls_fullchecklabel);
    p_library->setLls_fullchecklabel(result);
}

TEST_F(LlsDeepinLabelLibraryTest, get_check_privilege)
{
    LlsDeepinLabelLibrary::func_lls_checkprivilege result = p_library->lls_checkprivilege();
    EXPECT_EQ(result, p_library->m_lls_checkprivilege);
}

TEST_F(LlsDeepinLabelLibraryTest, set_check_privilege)
{
    LlsDeepinLabelLibrary::func_lls_checkprivilege result = p_library->lls_checkprivilege();
    p_library->setLls_checkprivilege(nullptr);
    EXPECT_EQ(nullptr, p_library->m_lls_checkprivilege);
    p_library->setLls_checkprivilege(result);
}

TEST_F(LlsDeepinLabelLibraryTest, get_error_desc)
{
    LlsDeepinLabelLibrary::func_lls_geterrordesc result = p_library->lls_geterrordesc();
    EXPECT_EQ(result, p_library->m_lls_geterrordesc);
}

TEST_F(LlsDeepinLabelLibraryTest, set_error_desc)
{
    LlsDeepinLabelLibrary::func_lls_geterrordesc result = p_library->lls_geterrordesc();
    p_library->setLls_geterrordesc(nullptr);
    EXPECT_EQ(nullptr, p_library->m_lls_geterrordesc);
    p_library->setLls_geterrordesc(result);
}

TEST_F(LlsDeepinLabelLibraryTest, get_is_completion)
{
    bool result = p_library->isCompletion();
    EXPECT_EQ(result, p_library->m_isCompletion);
}
