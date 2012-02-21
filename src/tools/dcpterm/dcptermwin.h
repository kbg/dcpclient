/*
 * Copyright (c) 2011 Kolja Glogowski
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef DCPTERMWIN_H
#define DCPTERMWIN_H

#include <dcpclient/client.h>
#include <QtGui/QMainWindow>

class QColor;
class QLabel;
namespace Ui { class DcpTermWin; }

class DcpTermWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit DcpTermWin(QWidget *parent = 0);
    ~DcpTermWin();

protected:
    void loadSettings();
    void saveSettings();
    void closeEvent(QCloseEvent *event);
    bool verboseOutput() const;

protected slots:
    void printError(const QString &errorText);
    void printLine(const QString &text, const QColor &color = Qt::black);

private slots:
    void messageInputFinished();
    void updateWindowTitle(Dcp::Client::State state);
    void dcp_stateChanged(Dcp::Client::State state);
    void dcp_error(Dcp::Client::Error error);
    void dcp_messageReceived();

    // autoconnect slots
    void on_actionConnect_triggered(bool checked);
    void on_buttonSend_clicked();
    void on_actionAutoReconnect_triggered(bool checked);
    void on_textOutput_customContextMenuRequested(const QPoint &pos);
    void on_comboDevice_customContextMenuRequested(const QPoint &pos);
    void on_comboMessage_customContextMenuRequested(const QPoint &pos);
    void on_actionSettings_triggered();

private:
    Ui::DcpTermWin *ui;
    Dcp::Client *m_dcp;
    QByteArray m_deviceName;
    QString m_serverName;
    quint16 m_serverPort;
    QLabel *m_connectionStatusLabel;
};

#endif // DCPTERMWIN_H
