#ifndef DFMGLOBAL_H
#define DFMGLOBAL_H

#include <QString>

class DFMGlobal
{
public:
    static QString organizationName();
    static QString applicationName();
    // libdde-file-manager version
    static QString applicationVersion();

private:
    DFMGlobal();
};

#endif // DFMGLOBAL_H
