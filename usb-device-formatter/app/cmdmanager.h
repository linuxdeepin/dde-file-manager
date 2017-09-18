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

#ifndef CMDMANAGER_H
#define CMDMANAGER_H

#include <QObject>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>

class CMDManager : public QObject
{
    Q_OBJECT
public:
    static CMDManager* instance();
    void process(const QApplication &app);
    void init();
    bool isSet(const QString& name);
    QString getPath();
    int getWinId();

signals:

public slots:

private:
    explicit CMDManager(QObject *parent = 0);
    QCommandLineParser m_parser;
    QCommandLineOption m_modelModeOpt;
};

#endif // CMDMANAGER_H
