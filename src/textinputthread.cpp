#include "textinputthread.h"
#include <QString>
#include <QTextStream>

TextInputThread::TextInputThread(QObject *parent)
    : QThread(parent)
{
}

void TextInputThread::run()
{
    QString line;
    QTextStream is(stdin);

    do {
        line = is.readLine();
        emit lineAvailable(line);
    } while (!line.isNull());
}
