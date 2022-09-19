// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALDEFINE_H
#define GLOBALDEFINE_H

enum ResultType {
    Unknown = 0,
    Success,
    PasswordWrong,
    AccessDiskFailed,   // Unable to get the encrypted disk list
    PasswordInconsistent,   // Passwords of disks are different
    InitFailed   // Initialization failed
};

#endif   // GLOBALDEFINE_H
