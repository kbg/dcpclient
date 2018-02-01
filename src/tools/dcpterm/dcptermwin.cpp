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
#include "configdialog.h"
#include "cmdlineoptions.h"
#include <dcpclient/version.h>
#include <dcpclient/message.h>
#include <QtDebug>
#include <QtGui>

DcpTermWin::DcpTermWin(const CmdLineOptions &opts, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::DcpTermWin),
      m_dcp(new Dcp::Client),
      m_serverPort(0),
      m_encoding("UTF-8"),
      m_codec(0),
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

    // load settings from ini file, also sets m_codec
    loadSettings();
    Q_ASSERT(m_codec);

    // overwrite settings by command line options
    if (!opts.serverName.isEmpty())
        m_serverName = opts.serverName;
    if (opts.serverPort != 0)
        m_serverPort = opts.serverPort;
    if (!opts.deviceName.isEmpty())
        m_deviceName = opts.deviceName;
    if (!opts.destDeviceName.isEmpty()) {
        QString currentDeviceName = opts.destDeviceName;
        int idx = ui->comboDevice->findText(currentDeviceName);
        if (idx == -1) {
            ui->comboDevice->addItem(currentDeviceName);
            idx = ui->comboDevice->findText(currentDeviceName);
        }
        ui->comboDevice->setCurrentIndex(idx);
    }

    // try to connect
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
                       "Kis", "dcpterm");

    bool ok;

    // server settings and device name
    settings.beginGroup("Server");
    m_encoding = settings.value("Encoding", "UTF-8").toString();
    updateTextCodec();
    m_deviceName = settings.value("DeviceName", "").toString();
    m_serverName = settings.value("ServerName", "localhost").toString();
    uint serverPort = settings.value("ServerPort", 2001).toUInt(&ok);
    m_serverPort = (ok && serverPort <= 0xffff) ? quint16(serverPort) : 2001;
    m_dcp->setAutoReconnect(settings.value("AutoReconnect", false).toBool());
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
        if (idx == -1) {
            ui->comboDevice->addItem(currentDeviceName);
            idx = ui->comboDevice->findText(currentDeviceName);
        }
        ui->comboDevice->setCurrentIndex(idx);
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
                       "Kis", "dcpterm");

    // server settings and device name
    settings.beginGroup("Server");
    settings.setValue("ServerName", m_serverName);
    settings.setValue("ServerPort", m_serverPort);
    settings.setValue("DeviceName", m_deviceName);
    settings.setValue("AutoReconnect", ui->actionAutoReconnect->isChecked());
    settings.setValue("ReconnectInterval", m_dcp->reconnectInterval());
    settings.setValue("Encoding", m_encoding);
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

void DcpTermWin::sendMessage(const Dcp::Message &msg)
{
    m_dcp->sendMessage(msg);
    if (verboseOutput())
        printLine(formatMessageOutput(msg, false), Qt::blue);
}

QByteArray DcpTermWin::normalizedDeviceName() const
{
    QString deviceName = m_deviceName;
    if (deviceName.isEmpty() || deviceName.trimmed().isEmpty()) {
        QString code = QDateTime::currentDateTimeUtc().toString("hhmmsszzz");
        deviceName = "dcpterm" + code;
    }

    return m_codec->fromUnicode(deviceName);
}

void DcpTermWin::updateTextCodec()
{
    m_codec = 0;

    if (m_encoding.toLower() == "utf-8")
        m_codec = QTextCodec::codecForName("UTF-8");
    if (m_encoding.toLower() == "latin1")
        m_codec = QTextCodec::codecForName("ISO-8859-1");

    if (!m_codec) {
        m_encoding = "UTF-8";
        m_codec = QTextCodec::codecForName("UTF-8");
    }

    Q_ASSERT(m_codec);
}

QString DcpTermWin::formatMessageOutput(
        const Dcp::Message &msg, bool incoming) const
{
    QString device = m_codec->toUnicode(
                incoming ? msg.source() : msg.destination());

    QString result;
    QTextStream os(&result, QIODevice::WriteOnly);
    os << (incoming ? "<< [" : ">> [") << msg.snr() << "] "
       << (msg.isUrgent() ? "u" : "-")
       << (msg.isReply() ? "r" : "-")
       << hex << " 0x" << msg.flags() << dec << " "
       << "<" << device << "> "
       << "[" << msg.data().size() << "] "
       << m_codec->toUnicode(msg.data());

    return result;
}

void DcpTermWin::printError(const QString &errorText)
{
    printLine(tr("Error: %1.").arg(errorText), Qt::red);
}

void DcpTermWin::printLine(const QString &text, const QColor &color)
{
    QScrollBar *scrollBar = ui->textOutput->verticalScrollBar();
    bool needToScroll = (scrollBar->value() == scrollBar->maximum());

    QTextCharFormat charFormat = ui->textOutput->currentCharFormat();
    charFormat.setForeground(color);
    QTextCursor cursor = ui->textOutput->textCursor();
    cursor.movePosition(QTextCursor::End);
    if (cursor.position() != 0)
        cursor.insertText("\n", charFormat);
    cursor.insertText(text, charFormat);

    if (needToScroll)
        scrollBar->setValue(scrollBar->maximum());
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
        QString destText = ui->comboDevice->currentText();
        QByteArray destination = m_codec->fromUnicode(destText);
        QByteArray data = m_codec->fromUnicode(messageText);

        Dcp::Message msg = m_dcp->sendMessage(destination, data);
        if (verboseOutput())
            printLine(formatMessageOutput(msg, false), Qt::blue);
    }
}

void DcpTermWin::updateWindowTitle(Dcp::Client::State state)
{
    QString dcpDeviceName = m_codec ?
                m_codec->toUnicode(m_dcp->deviceName()) : QString();

    if (state == Dcp::Client::ConnectingState ||
            state == Dcp::Client::ConnectedState) {
        setWindowTitle(tr("%1 - %2:%3 - DCP Terminal")
                       .arg(dcpDeviceName)
                       .arg(m_dcp->serverName())
                       .arg(m_dcp->serverPort()));
    }
    else {
        setWindowTitle(tr("%1 - DCP Terminal").arg(dcpDeviceName));
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
        if (verboseOutput()) {
            QString dcpDeviceName = m_codec->toUnicode(m_dcp->deviceName());
            printLine(tr("Connected to %1:%2 as %3.")
                         .arg(m_dcp->serverName())
                         .arg(m_dcp->serverPort())
                         .arg(dcpDeviceName),
                      Qt::blue);
        }
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
        // handle reply messages sent to us
        if (!m_reply.parse(msg)) {
            printLine(tr("Received invalid reply message"), Qt::red);
            return;
        }

        int errorCode = m_reply.errorCode();

        // not much to do for ACK replies, just print an error message (if
        // neccessary) and leave
        if (m_reply.isAckReply()) {
            if (errorCode != Dcp::AckNoError)
                printLine(Dcp::ackErrorString(errorCode), Qt::red);
            return;
        }

        // for regular replies, there is not much to do, if an error occurs
        if (errorCode > 0) {
            printLine(tr("Command failed, errcode: %1").arg(errorCode),
                      Qt::red);
            return;
        }

        // don't return for warnings, the message data is valid
        if (errorCode < 0)
            printLine(tr("Command returned with warning, errcode %1")
                      .arg(errorCode), Qt::red);

        // print the reply data, or OK for a FIN or an empty message
        if ((m_reply.numArguments() == 1 && m_reply.arguments()[0] == "FIN") ||
                !m_reply.hasArguments())
            printLine("OK");
        else
            printLine(m_codec->toUnicode(m_reply.joinedArguments()));
    }
    else
    {
        // handle command messages sent to us
        if (!m_command.parse(msg)) {
            sendMessage(msg.ackMessage(Dcp::AckUnknownCommandError));
            return;
        }

        if (m_command.cmdType() == Dcp::CommandParser::SetCmd &&
            m_command.identifier() == "nop")
        {
            // command: set nop
            if (m_command.hasArguments())
                sendMessage(msg.ackMessage(Dcp::AckParameterError));
            else {
                sendMessage(msg.ackMessage());
                sendMessage(msg.replyMessage("FIN"));
            }
        }
        else if (m_command.cmdType() == Dcp::CommandParser::GetCmd &&
                 m_command.identifier() == "echo")
        {
            // command: get echo [args]
            sendMessage(msg.ackMessage());
            sendMessage(msg.replyMessage(m_command.joinedArguments()));
        }
        else
            sendMessage(msg.ackMessage(Dcp::AckUnknownCommandError));
    }
}

void DcpTermWin::on_actionConnect_triggered(bool checked)
{
    if (checked && m_dcp->isUnconnected())
        m_dcp->connectToServer(m_serverName, m_serverPort,
                               normalizedDeviceName());
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

void DcpTermWin::on_actionSettings_triggered()
{
    ConfigDialog dlg(this);
    dlg.setServerName(m_serverName);
    dlg.setServerPort(m_serverPort);
    dlg.setDeviceName(m_deviceName);
    dlg.setEncoding(m_encoding);

    if (dlg.exec() != QDialog::Accepted)
        return;

    m_encoding = dlg.encoding();
    updateTextCodec();
    m_serverName = dlg.serverName();
    m_serverPort = dlg.serverPort();
    m_deviceName = dlg.deviceName();

    m_dcp->disconnectFromServer();
    if (m_dcp->waitForDisconnected())
        m_dcp->connectToServer(m_serverName, m_serverPort,
                               normalizedDeviceName());
}

void DcpTermWin::on_actionAbout_triggered()
{
    QString aboutText = tr(
                "<h2>DcpTerm %1</h2>" \
                "<p><b>Library Versions:</b><br>" \
                "&nbsp;&nbsp;DcpClient %2<br>" \
                "&nbsp;&nbsp;Qt %3</p>" \
                "<p>%4<br>%5</p>"
            )
            .arg(DCPCLIENT_VERSION_STRING)
            .arg(Dcp::versionString())
            .arg(qVersion())
            .arg(tr("Copyright (c) 2012 Kolja Glogowski"))
            .arg(tr("Kiepenheuer-Institut f&uuml;r Sonnenphysik"));

    QMessageBox::about(this, tr("About DcpTerm"), aboutText);
}
