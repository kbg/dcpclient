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

#include "dcptimeclient.h"
#include <QtGui/QtGui>
using namespace Dcp;

static QTextStream cerr(stderr, QIODevice::WriteOnly);

DcpTimeClient::DcpTimeClient(QWidget *parent)
    : QWidget(parent),
      m_labelDate(new QLabel),
      m_labelTime(new QLabel),
      m_labelJulian(new QLabel),
      m_comboTimeZone(new QComboBox),
      m_timer(new QTimer),
      m_dateMsgId(0),
      m_timeMsgId(0),
      m_julianMsgId(0),
      m_modeMsgId(0)
{
    m_dcp.setAutoReconnect(true);
    connect(&m_dcp, SIGNAL(error(Dcp::DcpClient::Error)),
                    SLOT(error(Dcp::DcpClient::Error)));
    connect(&m_dcp, SIGNAL(stateChanged(Dcp::DcpClient::State)),
                    SLOT(stateChanged(Dcp::DcpClient::State)));
    connect(&m_dcp, SIGNAL(messageReceived()), SLOT(messageReceived()));
    connect(m_timer, SIGNAL(timeout()), SLOT(requestValues()));

    QList<QLabel *> labelList;
    labelList << m_labelDate << m_labelTime << m_labelJulian;
    foreach (QLabel *label, labelList) {
        label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        label->setBackgroundRole(QPalette::Base);
        label->setAutoFillBackground(true);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        label->setMinimumHeight(20);
    }
    m_comboTimeZone->addItem(tr("UTC"));
    m_comboTimeZone->addItem(tr("Local"));

    QFormLayout *layout = new QFormLayout;
    layout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layout->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->setLabelAlignment(Qt::AlignRight);
    layout->addRow(tr("Date:"), m_labelDate);
    layout->addRow(tr("Time:"), m_labelTime);
    layout->addRow(tr("Julian date:"), m_labelJulian);
    layout->addRow(tr("Time zone:"), m_comboTimeZone);
    setLayout(layout);
    resize(250, 150);
}

DcpTimeClient::~DcpTimeClient()
{
    m_dcp.disconnectFromServer();
}

void DcpTimeClient::connectToServer(const QString &serverName,
                                    quint16 serverPort,
                                    const QByteArray &deviceName)
{
    m_dcp.connectToServer(serverName, serverPort, deviceName);
}

void DcpTimeClient::error(DcpClient::Error error)
{
    cerr << "Error: " << m_dcp.errorString() << "." << endl;
}

void DcpTimeClient::stateChanged(DcpClient::State state)
{
    switch (state)
    {
    case DcpClient::ConnectingState:
        break;
    case DcpClient::ConnectedState:
        m_stopWatch.start();
        requestValues();
        m_timer->start(200);
        break;
    case DcpClient::UnconnectedState:
        m_timer->stop();
        break;
    default:
        break;
    }
}

void DcpTimeClient::messageReceived()
{
    DcpMessage msg = m_dcp.readMessage();

    if (!msg.isReply())
        return;

    if (!m_parser.parse(msg) || m_parser.arguments().isEmpty()) {
        cerr << "Error: Cannot parse reply message." << endl;
        return;
    }

    if (m_parser.isAckReply())
        return;

    if (m_parser.errorCode() != 0) {
        cerr << "Error: Reply message has errorcode " << m_parser.errorCode()
             << "." << endl;
        return;
    }

    quint32 snr = msg.snr();
    QString text = m_parser.arguments()[0];
    if (snr == m_dateMsgId) {
        m_labelDate->setText(text);
        m_dateMsgId = 0;
    } else if (snr == m_timeMsgId) {
        m_labelTime->setText(text);
        m_timeMsgId = 0;
    } else if (snr == m_julianMsgId) {
        m_labelJulian->setText(text);
        m_julianMsgId = 0;
    } else if (snr == m_modeMsgId) {
        m_comboTimeZone->setCurrentIndex(
            m_comboTimeZone->findText(text, Qt::MatchFixedString));
        m_modeMsgId = 0;
    }
}

void DcpTimeClient::requestValues()
{
    if ((m_dateMsgId == 0 && m_timeMsgId == 0 && m_julianMsgId == 0 &&
         m_modeMsgId == 0) ||
        m_stopWatch.hasExpired(1000))
    {
        m_stopWatch.start();
        m_dateMsgId = m_dcp.sendMessage("dcptime", "get date");
        m_timeMsgId = m_dcp.sendMessage("dcptime", "get time");
        m_julianMsgId = m_dcp.sendMessage("dcptime", "get julian");
        m_modeMsgId = m_dcp.sendMessage("dcptime", "get mode");
    }
}

#include "dcptimeclient.moc"
