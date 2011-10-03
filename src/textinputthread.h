#ifndef TEXTINPUTTHREAD_H
#define TEXTINPUTTHREAD_H

#include <QThread>

class TextInputThread : public QThread
{
    Q_OBJECT

public:
    explicit TextInputThread(QObject *parent = 0);

signals:
    void lineAvailable(const QString &line);

protected:
    void run();
};

#endif // TEXTINPUTTHREAD_H
