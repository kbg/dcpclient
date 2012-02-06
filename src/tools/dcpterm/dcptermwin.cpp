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

#include "dcptermwin.h"
#include "ui_dcptermwin.h"
#include <dcpclient/message.h>
#include <QtCore/QtDebug>
#include <QtGui/QtGui>

inline static QDebug operator << (QDebug debug, const Dcp::Message &msg) {
    debug.nospace()
        << ((msg.flags() & Dcp::Message::PaceFlag) != 0 ? "p" : "-")
        << ((msg.flags() & Dcp::Message::GrecoFlag) != 0 ? "g" : "-")
        << (msg.isUrgent() ? "u" : "-")
        << (msg.isReply() ? "r" : "-")
        << hex << " [0x" << msg.flags() << dec << "] "
        << "#" << msg.snr() << " "
        << msg.source() << " -> "
        << msg.destination() << " "
        << "[" << msg.data().size() << "] "
        << msg.data();
    return debug.space();
}

inline static QString formatMessageOutput(const Dcp::Message &msg, bool incoming)
{
    QString result;
    QTextStream os(&result);
    os << (incoming ? "<< [" : ">> [") << msg.snr() << "] "
       << (msg.isUrgent() ? "u" : "-")
       << (msg.isReply() ? "r" : "-")
       << hex << " 0x" << msg.flags() << dec << " "
       << "<" << (incoming ? msg.source() : msg.destination()) << "> "
       << "[" << msg.data().size() << "] "
       << msg.data();

    return result;
}

DcpTermWin::DcpTermWin(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::DcpTermWin),
      m_dcp(new Dcp::Client),
      m_connectionStatusLabel(new QLabel)
{
    ui->setupUi(this);

    // customize ui
    ui->comboMessage->setCompleter(0);
    ui->statusbar->addPermanentWidget(m_connectionStatusLabel);
    connect(ui->comboMessage->lineEdit(), SIGNAL(returnPressed()),
            SLOT(messageInputFinished()));

    // setup dcp client
    m_dcp->setAutoReconnect(true);
    m_dcp->setReconnectInterval(5000);
    dcp_stateChanged(m_dcp->state());
    connect(m_dcp, SIGNAL(stateChanged(Dcp::Client::State)),
                   SLOT(dcp_stateChanged(Dcp::Client::State)));
    connect(m_dcp, SIGNAL(error(Dcp::Client::Error)),
                   SLOT(dcp_error(Dcp::Client::Error)));
    connect(m_dcp, SIGNAL(messageReceived()), SLOT(dcp_messageReceived()));

    // load settings and try to connect
    loadSettings();
    QTimer::singleShot(0, ui->actionConnect, SLOT(trigger()));
}

DcpTermWin::~DcpTermWin()
{
    delete m_dcp;
    delete ui;
}

void DcpTermWin::loadSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "diib.de", "dcpterm");

    bool ok;

    // server settings and device name
    settings.beginGroup("Server");
    m_deviceName = settings.value("DeviceName", "dcpterm").toByteArray();
    m_serverName = settings.value("ServerName", "localhost").toString();
    uint serverPort = settings.value("ServerPort", 2001).toUInt(&ok);
    m_serverPort = (ok && serverPort <= 0xffff) ? quint16(serverPort) : 2001;
    m_dcp->setAutoReconnect(settings.value("AutoReconnect", true).toBool());
    ui->actionAutoReconnect->setChecked(m_dcp->autoReconnect());
    int reconnectInverval = settings.value("ReconnectInterval").toInt(&ok);
    if (ok && reconnectInverval > 0)
        m_dcp->setReconnectInterval(reconnectInverval);
    settings.endGroup();

    // UI settings (position, size, ...)
    settings.beginGroup("Settings");

    QVariant geometry = settings.value("WindowGeometry");
    if (geometry.isValid())
        restoreGeometry(geometry.toByteArray());

    bool verboseOutput = settings.value("VerboseOutput", true).toBool();
    ui->actionVerboseOutput->setChecked(verboseOutput);

    settings.endGroup();

    // device name history
    settings.beginGroup("DeviceNameHistory");
    QVariant deviceNameCount = settings.value("DeviceNameCount");
    if (deviceNameCount.isValid())
    {
        int n = deviceNameCount.toInt(&ok);
        if (!ok || n < 0)
            n = 0;
        QStringList deviceNameList;
        for (int i = 0; i < n; ++i) {
            QString deviceName = settings.value(
                        QString("DeviceName%1").arg(i)).toString();
            if (!deviceName.isEmpty())
                deviceNameList << deviceName;
        }
        ui->comboDevice->addItems(deviceNameList);
    }
    QString currentDeviceName = settings.value("CurrentDeviceName").toString();
    if (!currentDeviceName.isEmpty()) {
        int idx = ui->comboDevice->findText(currentDeviceName);
        if (idx != -1)
            ui->comboDevice->setCurrentIndex(idx);
        else {
            ui->comboDevice->addItem(currentDeviceName);
            ui->comboDevice->setCurrentIndex(
                    ui->comboDevice->findText(currentDeviceName));
        }
    }
    settings.endGroup();

    // message history
    settings.beginGroup("MessageHistory");
    QVariant messageCount = settings.value("MessageCount");
    if (messageCount.isValid())
    {
        int n = messageCount.toInt(&ok);
        if (!ok || n < 0)
            n = 0;
        QStringList messageList;
        for (int i = 0; i < n; ++i) {
            QString message = settings.value(
                        QString("Message%1").arg(i)).toString();
            if (!message.isEmpty())
                messageList << message;
        }
        ui->comboMessage->addItems(messageList);
    }
    if (ui->comboMessage->count() > 0) {
        ui->comboMessage->addItem("");
        ui->comboMessage->setCurrentIndex(ui->comboMessage->count()-1);
    }
    settings.endGroup();
}

void DcpTermWin::saveSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "diib.de", "dcpterm");

    // server settings and device name
    settings.beginGroup("Server");
    settings.setValue("ServerName", m_serverName);
    settings.setValue("ServerPort", m_serverPort);
    settings.setValue("DeviceName", QString(m_deviceName));
    settings.setValue("AutoReconnect", ui->actionAutoReconnect->isChecked());
    settings.setValue("ReconnectInterval", m_dcp->reconnectInterval());
    settings.endGroup();

    // UI settings (position, size, ...)
    settings.beginGroup("Settings");
    settings.setValue("WindowGeometry", saveGeometry());
    settings.setValue("VerboseOutput", ui->actionVerboseOutput->isChecked());
    settings.endGroup();

    // device name history
    settings.remove("DeviceNameHistory");
    settings.beginGroup("DeviceNameHistory");
    settings.setValue("CurrentDeviceName", ui->comboDevice->currentText());
    settings.setValue("DeviceNameCount", ui->comboDevice->count());
    for (int i = 0; i < ui->comboDevice->count(); i++)
        settings.setValue(QString("DeviceName%1").arg(i),
                          ui->comboDevice->itemText(i));
    settings.endGroup();

    // message history
    settings.remove("MessageHistory");
    settings.beginGroup("MessageHistory");
    settings.setValue("MessageCount", ui->comboMessage->count());
    for (int i = 0; i < ui->comboMessage->count(); i++)
        settings.setValue(QString("Message%1").arg(i),
                          ui->comboMessage->itemText(i));
    settings.endGroup();
}

void DcpTermWin::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

bool DcpTermWin::verboseOutput() const
{
    return ui->actionVerboseOutput->isChecked();
}

void DcpTermWin::printError(const QString &errorText)
{
    printLine(tr("Error: %1.").arg(errorText), Qt::red);
}

void DcpTermWin::printLine(const QString &text, const QColor &color)
{
    QTextCharFormat charFormat = ui->textOutput->currentCharFormat();
    charFormat.setForeground(color);
    ui->textOutput->setCurrentCharFormat(charFormat);
    ui->textOutput->appendPlainText(text);
}

void DcpTermWin::messageInputFinished()
{
    QComboBox *cb = ui->comboMessage;

    // Do nothing if the combo box is empty
    if (cb->count() == 0)
        return;

    // Save the current text
    QString messageText = cb->currentText();

    // Clear the text and make sure that an empty entry is at the end of
    // the combo box. This allows to press the up key to get the last entry.
    for (int i = cb->count()-1; i > 0; --i)
        if (cb->itemText(i).isEmpty()) {
            cb->removeItem(i);
            break;
        }
    cb->addItem("");
    cb->setCurrentIndex(cb->count()-1);

    // finally send the message
    if (!messageText.isEmpty()) {
        QByteArray destination = ui->comboDevice->currentText().toAscii();
        QByteArray data = messageText.toAscii();
        quint32 snr = m_dcp->sendMessage(destination, data).snr();

        if (verboseOutput()) {
            Dcp::Message msg(snr, m_dcp->deviceName(), destination, data, 0);
            printLine(formatMessageOutput(msg, false), Qt::blue);
        }
    }
}

void DcpTermWin::updateWindowTitle(Dcp::Client::State state)
{
    if (state == Dcp::Client::ConnectingState ||
            state == Dcp::Client::ConnectedState) {
        setWindowTitle(tr("%1 - %2:%3 - DCP Terminal")
                       .arg(QString(m_dcp->deviceName()))
                       .arg(m_dcp->serverName())
                       .arg(m_dcp->serverPort()));
    }
    else {
        setWindowTitle(tr("%1 - DCP Terminal")
                       .arg(QString(m_dcp->deviceName())));
    }
}

void DcpTermWin::dcp_stateChanged(Dcp::Client::State state)
{
    QString stateText;
    Qt::GlobalColor color = Qt::blue;

    // update widget states
    if (state == Dcp::Client::ConnectedState) {
        ui->comboMessage->setEnabled(true);
        ui->comboMessage->setFocus();
        ui->comboDevice->setEnabled(true);
        ui->buttonSend->setEnabled(true);
    }
    else {
        ui->comboDevice->setEnabled(false);
        ui->buttonSend->setEnabled(false);
        ui->comboMessage->setEnabled(false);
    }

    // update window title
    updateWindowTitle(state);

    // update status bar
    switch (state)
    {
    case Dcp::Client::UnconnectedState:
        ui->actionConnect->setChecked(false);
        stateText = tr("Not connected");
        color = Qt::red;
        break;
    case Dcp::Client::HostLookupState:
    case Dcp::Client::ConnectingState:
        stateText = tr("Connecting");
        break;
    case Dcp::Client::ConnectedState:
        ui->actionConnect->setChecked(true);
        stateText = tr("Connected");
        if (verboseOutput())
            printLine(tr("Connected to %1:%2 as %3.").arg(m_dcp->serverName())
                         .arg(m_dcp->serverPort())
                         .arg(QString(m_dcp->deviceName())),
                      Qt::blue);
        break;
    case Dcp::Client::ClosingState:
        stateText = tr("Disconnecting");
        break;
    }
    m_connectionStatusLabel->setText(tr("<font color=\"%1\">%2<font>")
        .arg(QColor(color).name()).arg(stateText));
}

void DcpTermWin::dcp_error(Dcp::Client::Error error)
{
    Q_UNUSED(error)
    printError(m_dcp->errorString());
}

void DcpTermWin::dcp_messageReceived()
{
    Dcp::Message msg = m_dcp->readMessage();

    if (verboseOutput())
        printLine(formatMessageOutput(msg, true), Qt::blue);

    if (msg.isReply())
    {
        QByteArray data = msg.data().simplified();

        // handle replies
        if (data.isEmpty())
            printLine(tr("Invalid reply message"), Qt::red);
        else if (data == "0 ACK")
            ; // do nothing
        else if (data == "2 ACK")
            printLine(tr("Unknown command"), Qt::red);
        else if (data == "3 ACK")
            printLine(tr("Parameter error"), Qt::red);
        else if (data == "5 ACK")
            printLine(tr("Wrong mode"), Qt::red);
        else
        {
            QStringList args = QString(data).split(" ");

            bool ok;
            int errcode = args.takeFirst().toInt(&ok);

            if (!ok)
                printLine(tr("Invalid error code in reply message"));
            else if (errcode > 0)
                printLine(tr("Command failed, errcode: %1").arg(errcode),
                          Qt::red);
            else if (errcode < 0) {
                printLine(tr("Command returned with warning, errorcode %1")
                          .arg(errcode), Qt::red);
                printLine(args.join(" "));
            }
            else {
                if (args.count() == 1 && args[0] == "FIN")
                    printLine("OK");
                else
                    printLine(args.join(" "));
            }
        }
    }
    else
    {
        // handle commands sent to us
        Dcp::Message outMsg(msg.snr(), msg.destination(), msg.source(), "", 0);

        // only "set nop" is a valid command
        if (msg.data().simplified() != "set nop")
        {
            outMsg.setFlags(Dcp::Message::AckFlags);
            outMsg.setData("2 ACK");
            m_dcp->sendMessage(outMsg);

            if (verboseOutput())
                printLine(formatMessageOutput(outMsg, false), Qt::blue);
        }
        else
        {
            outMsg.setFlags(Dcp::Message::AckFlags);
            outMsg.setData("0 ACK");
            m_dcp->sendMessage(outMsg);

            if (verboseOutput())
                printLine(formatMessageOutput(outMsg, false), Qt::blue);

            outMsg.setFlags(Dcp::Message::ReplyFlag);
            outMsg.setData("0 FIN");
            m_dcp->sendMessage(outMsg);

            if (verboseOutput())
                printLine(formatMessageOutput(outMsg, false), Qt::blue);
        }
    }
}

void DcpTermWin::on_actionConnect_triggered(bool checked)
{
    if (checked && m_dcp->isUnconnected())
        m_dcp->connectToServer(m_serverName, m_serverPort, m_deviceName);
    else if (!checked && m_dcp->isConnected())
        m_dcp->disconnectFromServer();
}

void DcpTermWin::on_buttonSend_clicked()
{
    QString text = ui->comboMessage->currentText();
    if (!text.isEmpty()) {
        ui->comboMessage->addItem(text);
        messageInputFinished();
    }
}

void DcpTermWin::on_actionAutoReconnect_triggered(bool checked)
{
    m_dcp->setAutoReconnect(checked);
}

void DcpTermWin::on_textOutput_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = ui->textOutput->createStandardContextMenu();
    menu->addAction(ui->actionClearOutput);
    menu->addSeparator();
    menu->addAction(ui->actionVerboseOutput);
    menu->exec(ui->textOutput->mapToGlobal(pos));
    delete menu;
}

void DcpTermWin::on_comboDevice_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = ui->comboDevice->lineEdit()->createStandardContextMenu();
    menu->addAction(ui->actionClearDeviceList);
    menu->exec(ui->comboDevice->lineEdit()->mapToGlobal(pos));
    delete menu;
}

void DcpTermWin::on_comboMessage_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = ui->comboMessage->lineEdit()->createStandardContextMenu();
    menu->addAction(ui->actionClearMessageHistory);
    menu->exec(ui->comboMessage->lineEdit()->mapToGlobal(pos));
    delete menu;
}
