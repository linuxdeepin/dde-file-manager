#include "funcwrapper.h"

#include <gtest/gtest.h>

TEST(toBase64, testCommonVault)
{
    unsigned char data[6] = {0x12, 0x32, 0x56, 0x78, 0x90, 0xab};
    int size = 6;
    int options = 1;
    char *str = toBase64(data, size, options);
    EXPECT_STREQ("EjJWeJCr", str);
}
