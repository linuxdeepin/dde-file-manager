// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>

void errorCode_dfmio_trans()
{
    // DFM_IO_ERROR_FAILED
    QObject::tr("Generic error condition for when an operation fails and no more specific DFMIOErrorEnum value is defined");
    // DFM_IO_ERROR_NOT_FOUND
    QObject::tr("File not found");
    // DFM_IO_ERROR_EXISTS
    QObject::tr("File already exists");
    // DFM_IO_ERROR_IS_DIRECTORY
    QObject::tr("File is a directory");
    // DFM_IO_ERROR_NOT_DIRECTORY
    QObject::tr("File is not a directory");
    // DFM_IO_ERROR_NOT_EMPTY
    QObject::tr("File is a directory that isn't empty");
    // DFM_IO_ERROR_NOT_REGULAR_FILE
    QObject::tr("File is not a regular file");
    // DFM_IO_ERROR_NOT_SYMBOLIC_LINK
    QObject::tr("File is not a symbolic link");
    // DFM_IO_ERROR_NOT_MOUNTABLE_FILE
    QObject::tr("File cannot be mounted");
    // DFM_IO_ERROR_FILENAME_TOO_LONG
    QObject::tr("Filename has too many characters");
    // DFM_IO_ERROR_INVALID_FILENAME
    QObject::tr("Filename is invalid or contains invalid characters");
    // DFM_IO_ERROR_TOO_MANY_LINKS
    QObject::tr("File contains too many symbolic links");
    // DFM_IO_ERROR_NO_SPACE
    QObject::tr("No space left on drive");
    // DFM_IO_ERROR_INVALID_ARGUMENT
    QObject::tr("Invalid argument");
    // DFM_IO_ERROR_PERMISSION_DENIED
    QObject::tr("Permission denied");
    // DFM_IO_ERROR_NOT_SUPPORTED
    QObject::tr("Operation (or one of its parameters) not supported");
    // DFM_IO_ERROR_NOT_MOUNTED
    QObject::tr("File isn't mounted");
    // DFM_IO_ERROR_ALREADY_MOUNTED
    QObject::tr("File is already mounted");
    // DFM_IO_ERROR_CLOSED
    QObject::tr("File was closed");
    // DFM_IO_ERROR_CANCELLED
    QObject::tr("Operation was cancelled");
    // DFM_IO_ERROR_PENDING
    QObject::tr("Operations are still pending");
    // DFM_IO_ERROR_READ_ONLY
    QObject::tr("File is read-only");
    // DFM_IO_ERROR_CANT_CREATE_BACKUP
    QObject::tr("Backup couldn't be created");
    // DFM_IO_ERROR_WRONG_ETAG
    QObject::tr("File's Entity Tag was incorrect");
    // DFM_IO_ERROR_TIMED_OUT
    QObject::tr("Operation timed out");
    // DFM_IO_ERROR_WOULD_RECURSE
    QObject::tr("Operation would be recursive");
    // DFM_IO_ERROR_BUSY
    QObject::tr("File is busy");
    // DFM_IO_ERROR_WOULD_BLOCK
    QObject::tr("Operation would block");
    // DFM_IO_ERROR_HOST_NOT_FOUND
    QObject::tr("Host couldn't be found (remote operations)");
    // DFM_IO_ERROR_WOULD_MERGE
    QObject::tr("Operation would merge files");
    // DFM_IO_ERROR_TOO_MANY_OPEN_FILES
    QObject::tr("The current process has too many files open and can't open any more. Duplicate descriptors do count toward this limit");
    // DFM_IO_ERROR_NOT_INITIALIZED
    QObject::tr("The object has not been initialized");
    // DFM_IO_ERROR_ADDRESS_IN_USE
    QObject::tr("The requested address is already in use");
    // DFM_IO_ERROR_PARTIAL_INPUT
    QObject::tr("Need more input to finish operation");
    // DFM_IO_ERROR_INVALID_DATA
    QObject::tr("The input data was invalid");
    // DFM_IO_ERROR_DBUS_ERROR
    QObject::tr("A remote object generated an error(dbus)");
    // DFM_IO_ERROR_HOST_UNREACHABLE
    QObject::tr("Host unreachable");
    // DFM_IO_ERROR_NETWORK_UNREACHABLE
    QObject::tr("Network unreachable");
    // DFM_IO_ERROR_CONNECTION_REFUSED
    QObject::tr("Connection refused");
    // DFM_IO_ERROR_PROXY_FAILED
    QObject::tr("Connection to proxy server failed");
    // DFM_IO_ERROR_PROXY_AUTH_FAILED
    QObject::tr("Proxy authentication failed");
    // DFM_IO_ERROR_PROXY_NEED_AUTH
    QObject::tr("Proxy server needs authentication");
    // DFM_IO_ERROR_PROXY_NOT_ALLOWED
    QObject::tr("Proxy connection is not allowed by ruleset");
    // DFM_IO_ERROR_BROKEN_PIPE
    QObject::tr("Broken pipe");
    // DFM_IO_ERROR_CONNECTION_CLOSED
    QObject::tr("Connection closed by peer");
    // DFM_IO_ERROR_NOT_CONNECTED
    QObject::tr("Transport endpoint is not connected");
    // DFM_IO_ERROR_MESSAGE_TOO_LARGE
    QObject::tr("Message too large");
    // DFM_IO_ERROR_OPEN_FAILED
    QObject::tr("Failed to open the file");
    // DFM_IO_ERROR_OPEN_FLAG_ERROR
    QObject::tr("File open flag error");
    // DFM_IO_ERROR_INFO_NO_ATTRIBUTE
    QObject::tr("File info has no attribute");
    // DFM_IO_ERROR_HOST_IS_DOWN
    QObject::tr("Host is down");
    // Custom error for file manager
    QObject::tr("Target Trash File Not exist");
}
