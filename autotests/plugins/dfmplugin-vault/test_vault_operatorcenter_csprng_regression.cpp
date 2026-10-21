// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests for OperatorCenter::autoGeneratePassword.
// Covers: BUG-379745 (autoGeneratePassword used srand(secsTo(currentTime))
// as seed — only 86400 possible values — and rand() (non-cryptographic PRNG),
// making vault passwords predictable and enumerable. The fix replaced
// srand/rand with QRandomGenerator::system()->bounded() which uses
// /dev/urandom (CSPRNG), and uses str.length() instead of hardcoded values
// for bounded() upper bound to eliminate modulo bias).

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QString>
#include <QSet>

#include "utils/encryption/operatorcenter.h"
#include "utils/vaultdefine.h"

DPVAULT_USE_NAMESPACE

namespace {
const QString kDigits = QStringLiteral("0123456789");
const QString kSpecial = QStringLiteral("`~!@#$%^&*");
const QString kLetters = QStringLiteral("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
const QString kAllChars = kDigits + kSpecial + kLetters;
}   // namespace

class OperatorCenterCSPRNGRegressionTest : public testing::Test
{
protected:
    void SetUp() override
    {
        operatorCenter = OperatorCenter::getInstance();
        QDir().mkpath(kVaultBasePath);
    }

    void TearDown() override
    {
        // singleton — no cleanup needed for pure-logic password generation
    }

    OperatorCenter *operatorCenter = nullptr;
};

// BUG-379745: The old srand(time)-based seeding meant two calls within the
// same second produced identical passwords. With QRandomGenerator::system()
// (CSPRNG backed by /dev/urandom), consecutive calls must produce different
// results.
TEST_F(OperatorCenterCSPRNGRegressionTest, BUG379745_ConsecutiveCallsProduceDifferentPasswords)
{
    const int kSampleCount = 20;
    QSet<QString> passwords;
    for (int i = 0; i < kSampleCount; ++i) {
        passwords.insert(operatorCenter->autoGeneratePassword(16));
    }
    // With CSPRNG, the probability of any collision among 20 16-char passwords
    // is astronomically small. The old code would produce all-identical results
    // when called within the same second.
    EXPECT_GT(passwords.size(), 1)
        << "CSPRNG should produce different passwords on consecutive calls";
}

// BUG-379745: The first character must come from the digit set (0-9).
TEST_F(OperatorCenterCSPRNGRegressionTest, BUG379745_FirstCharIsDigit)
{
    for (int i = 0; i < 50; ++i) {
        QString pwd = operatorCenter->autoGeneratePassword(8);
        ASSERT_EQ(pwd.length(), 8);
        EXPECT_TRUE(kDigits.contains(pwd.at(0)))
            << "First char should be a digit, got: " << pwd.at(0).toLatin1();
    }
}

// BUG-379745: The second character must come from the special char set.
TEST_F(OperatorCenterCSPRNGRegressionTest, BUG379745_SecondCharIsSpecial)
{
    for (int i = 0; i < 50; ++i) {
        QString pwd = operatorCenter->autoGeneratePassword(8);
        ASSERT_EQ(pwd.length(), 8);
        EXPECT_TRUE(kSpecial.contains(pwd.at(1)))
            << "Second char should be special, got: " << pwd.at(1).toLatin1();
    }
}

// BUG-379745: The third character must come from the letter set (a-zA-Z).
TEST_F(OperatorCenterCSPRNGRegressionTest, BUG379745_ThirdCharIsLetter)
{
    for (int i = 0; i < 50; ++i) {
        QString pwd = operatorCenter->autoGeneratePassword(8);
        ASSERT_EQ(pwd.length(), 8);
        EXPECT_TRUE(kLetters.contains(pwd.at(2)))
            << "Third char should be a letter, got: " << pwd.at(2).toLatin1();
    }
}

// BUG-379745: All characters in the generated password must be from the
// combined valid character set (digits + special + letters). The old code
// used rand() % 52 for the tail which only covered 52 of 72 chars (modulo
// bias + incomplete coverage).
TEST_F(OperatorCenterCSPRNGRegressionTest, BUG379745_AllCharsFromValidSet)
{
    for (int i = 0; i < 100; ++i) {
        QString pwd = operatorCenter->autoGeneratePassword(24);
        ASSERT_EQ(pwd.length(), 24);
        for (int j = 0; j < pwd.length(); ++j) {
            EXPECT_TRUE(kAllChars.contains(pwd.at(j)))
                << "Char at position " << j << " is not from valid set: '"
                << pwd.at(j).toLatin1() << "' in password: " << pwd.toStdString();
        }
    }
}

// BUG-379745: The fix uses str.length() instead of hardcoded values for
// bounded() upper bound. This ensures the special char set (10 chars) and
// the all-chars set (72 chars) are fully covered without modulo bias.
// Verify that special characters beyond the first 10 indices are reachable
// by checking that all special chars appear across many generations.
TEST_F(OperatorCenterCSPRNGRegressionTest, BUG379745_SpecialCharsFullyCovered)
{
    QSet<QChar> seenSpecial;
    for (int i = 0; i < 500; ++i) {
        QString pwd = operatorCenter->autoGeneratePassword(8);
        ASSERT_EQ(pwd.length(), 8);
        seenSpecial.insert(pwd.at(1));   // second char is always special
    }
    // With CSPRNG and str.length() as upper bound, all 10 special chars should
    // appear across 500 generations. The old rand() % 10 also covered all 10,
    // but this test guards against regressions where the bound is wrong.
    EXPECT_EQ(seenSpecial.size(), kSpecial.length())
        << "Not all special chars covered; seen " << seenSpecial.size();
}

// BUG-379745: Verify that the password structure is consistent — the first
// three positions always have the correct character type (digit, special,
// letter) regardless of the requested length.
TEST_F(OperatorCenterCSPRNGRegressionTest, BUG379745_PasswordStructureConsistentAcrossLengths)
{
    for (int len : {3, 4, 8, 16, 24, 32}) {
        for (int i = 0; i < 30; ++i) {
            QString pwd = operatorCenter->autoGeneratePassword(len);
            ASSERT_EQ(pwd.length(), len)
                << "Password length mismatch for requested length " << len;
            EXPECT_TRUE(kDigits.contains(pwd.at(0)));
            EXPECT_TRUE(kSpecial.contains(pwd.at(1)));
            EXPECT_TRUE(kLetters.contains(pwd.at(2)));
        }
    }
}

// BUG-379745: The tail characters (position 3+) should come from the full
// combined set including digits, special chars, and letters. The old code
// used rand() % 52 which only covered letters (52 chars), missing digits
// and special chars in the tail. With the fix using bounded(strAllChar.length()),
// all 72 chars are reachable.
TEST_F(OperatorCenterCSPRNGRegressionTest, BUG379745_TailCharsIncludeAllTypes)
{
    bool hasDigitInTail = false;
    bool hasSpecialInTail = false;
    bool hasLetterInTail = false;

    for (int i = 0; i < 500; ++i) {
        QString pwd = operatorCenter->autoGeneratePassword(16);
        ASSERT_EQ(pwd.length(), 16);
        for (int j = 3; j < pwd.length(); ++j) {
            if (kDigits.contains(pwd.at(j)))
                hasDigitInTail = true;
            if (kSpecial.contains(pwd.at(j)))
                hasSpecialInTail = true;
            if (kLetters.contains(pwd.at(j)))
                hasLetterInTail = true;
        }
        if (hasDigitInTail && hasSpecialInTail && hasLetterInTail)
            break;
    }
    EXPECT_TRUE(hasDigitInTail) << "Tail should contain digits";
    EXPECT_TRUE(hasSpecialInTail) << "Tail should contain special chars";
    EXPECT_TRUE(hasLetterInTail) << "Tail should contain letters";
}

// BUG-379745: Edge case — minimum valid length (3) produces exactly 3 chars
// with correct structure (digit, special, letter) and no tail characters.
TEST_F(OperatorCenterCSPRNGRegressionTest, BUG379745_MinLength3ProducesStructuredPassword)
{
    QString pwd = operatorCenter->autoGeneratePassword(3);
    ASSERT_EQ(pwd.length(), 3);
    EXPECT_TRUE(kDigits.contains(pwd.at(0)));
    EXPECT_TRUE(kSpecial.contains(pwd.at(1)));
    EXPECT_TRUE(kLetters.contains(pwd.at(2)));
}
