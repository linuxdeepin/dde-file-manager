#include "global.h"

#include "utils/utils.h"

#include <QDebug>
#include <QGuiApplication>
#include <QFileInfo>
#include <QProcess>

#include <cstdio>

#include "chinese2pinyin.h"

QString Global::wordWrapText(const QString &text, int width, QTextOption::WrapMode wrapMode, int *height)
{
    QTextLayout textLayout(text);
    QTextOption &text_option = *const_cast<QTextOption*>(&textLayout.textOption());
    text_option.setWrapMode(wrapMode);

    textLayout.beginLayout();

    QTextLine line = textLayout.createLine();
    QString  str;

    int text_height = 0;

    while (line.isValid()) {
        line.setLineWidth(width);

        const QString &tmp_str = text.mid(line.textStart(), line.textLength());

        str += tmp_str;

        if (tmp_str.indexOf('\n') >= 0)
            text_height += TEXT_LINE_HEIGHT;

        text_height += TEXT_LINE_HEIGHT;
        line = textLayout.createLine();

        if(line.isValid())
            str.append("\n");
    }

    textLayout.endLayout();

    if(height)
        *height = text_height;

    return str;
}

QString Global::elideText(const QString &text, const QSize &size, const QFontMetrics &fontMetrics,
                          QTextOption::WrapMode wordWrap, Qt::TextElideMode mode, int flags)
{
    int height = 0;

    QTextLayout textLayout(text);
    QString str;

    const_cast<QTextOption*>(&textLayout.textOption())->setWrapMode(wordWrap);

    textLayout.beginLayout();

    QTextLine line = textLayout.createLine();

    while (line.isValid()) {
        height += TEXT_LINE_HEIGHT;

        if(height + TEXT_LINE_HEIGHT >= size.height()) {
            str += fontMetrics.elidedText(text.mid(line.textStart() + line.textLength() + 1), mode, size.width(), flags);

            break;
        }

        line.setLineWidth(size.width());

        const QString &tmp_str = text.mid(line.textStart(), line.textLength());

        if (tmp_str.indexOf('\n'))
            height += TEXT_LINE_HEIGHT;

        str += tmp_str;

        line = textLayout.createLine();

        if(line.isValid())
            str.append("\n");
    }

    textLayout.endLayout();

    return str;
}

QString Global::toPinyin(const QString &text)
{
    return Pinyin::Chinese2Pinyin(text);
}

bool Global::startWithHanzi(const QString &text)
{
    const QVector<uint> list = text.toUcs4();

    return !list.isEmpty() && list.first() >= 0x4e00 && list.first() <= 0x9fbf;
}

bool Global::keyShiftIsPressed()
{
    return qApp->keyboardModifiers() == Qt::ShiftModifier;
}

bool Global::keyCtrlIsPressed()
{
    return qApp->keyboardModifiers() == Qt::ControlModifier;
}

bool Global::fileNameCorrection(const QString &filePath)
{
    QFileInfo info(filePath);
    QProcess ls;

    ls.start("ls", QStringList() << "-1" << "--color=never" << info.absolutePath());
    ls.waitForFinished();

    const QByteArray &request = ls.readAllStandardOutput();

    for (const QByteArray &name : request.split('\n')) {
        const QString str_fileName = QString::fromLocal8Bit(name);

        if (str_fileName == info.fileName() && str_fileName.toLocal8Bit() != name) {
            return fileNameCorrection(joinPath(info.absolutePath().toLocal8Bit(), name));
        }
    }

    return false;
}

bool Global::fileNameCorrection(const QByteArray &filePath)
{
    const QByteArray &newFilePath = QString::fromLocal8Bit(filePath).toLocal8Bit();

    if (filePath == newFilePath)
        return true;

    return std::rename(filePath.constData(), newFilePath.constData());
}
