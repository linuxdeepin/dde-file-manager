#include "global.h"

#ifdef Q_OS_LINUX
#include <X11/XKBlib.h>
#include <QX11Info>
#elif defined(Q_OS_WIN32)
#include <Windows.h>
#endif

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
        str += text.mid(line.textStart(), line.textLength());
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
    qreal height = 0;

    QTextLayout textLayout(text);
    QString str;

    const_cast<QTextOption*>(&textLayout.textOption())->setWrapMode(wordWrap);

    textLayout.beginLayout();

    QTextLine line = textLayout.createLine();

    while (line.isValid()) {
        height += TEXT_LINE_HEIGHT;

        if(height + TEXT_LINE_HEIGHT >= size.height()) {
            str += fontMetrics.elidedText(text.mid(line.textStart() + line.textLength() + 1),
                                          mode, size.width(), flags);
            break;
        }

        line.setLineWidth(size.width());
        str += text.mid(line.textStart(), line.textLength());

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
#ifdef Q_OS_LINUX
    Display *dpy = QX11Info::display();
    XkbStateRec sate;

    XkbGetState(dpy, XkbUseCoreKbd, &sate);

    return sate.mods & 1;
#elif defined(Q_OS_WIN32)
    short status = GetKeyState(VK_SHIFT);

    return status == 0xF0;
#endif
}

bool Global::keyCtrlIsPressed()
{
#ifdef Q_OS_LINUX
    Display *dpy = QX11Info::display();
    XkbStateRec sate;

    XkbGetState(dpy, XkbUseCoreKbd, &sate);

    return sate.mods & 4;
#elif defined(Q_OS_WIN32)
    short status = GetKeyState(VK_CTRL);

    return status == 0xF0;
#endif
}
