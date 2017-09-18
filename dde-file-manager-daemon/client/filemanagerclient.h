/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILEMANAGERCLIENT_H
#define FILEMANAGERCLIENT_H

#include <QObject>

#include "dbusservice/dbusinterface/fileoperation_interface.h"

class FileManagerClient : public QObject
{
    Q_OBJECT
public:
    explicit FileManagerClient(QObject *parent = 0);
    ~FileManagerClient();

signals:

public slots:
    void testNewNameJob(const QString &oldFile, const QString &newFile);

private:
    FileOperationInterface* m_fileOperationInterface = NULL;

};

#endif // FILEMANAGERCLIENT_H
