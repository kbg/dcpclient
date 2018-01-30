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
#include <dcpclient/message.h>
#include <QtGui>

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
    connect(&m_dcp, SIGNAL(error(Dcp::Client::Error)),
                    SLOT(error(Dcp::Client::Error)));
    connect(&m_dcp, SIGNAL(stateChanged(Dcp::Client::State)),
                    SLOT(stateChanged(Dcp::Client::State)));
    connect(&m_dcp, SIGNAL(messageReceived()), SLOT(messageReceived()));
    connect(m_timer, SIGNAL(timeout()), SLOT(requestValues()));
    connect(m_comboTimeZone, SIGNAL(activated(QString)),
                             SLOT(comboTimeZone_activated(QString)));

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

void DcpTimeClient::error(Dcp::Client::Error error)
{
    cerr << "Error: " << m_dcp.errorString() << "." << endl;
}

void DcpTimeClient::stateChanged(Dcp::Client::State state)
{
    switch (state)
    {
    case Dcp::Client::ConnectingState:
        break;
    case Dcp::Client::ConnectedState:
        m_stopWatch.start();
        requestValues();
        m_timer->start(20);
        break;
    case Dcp::Client::UnconnectedState:
        m_timer->stop();
        break;
    default:
        break;
    }
}

void DcpTimeClient::messageReceived()
{
    Dcp::Message msg = m_dcp.readMessage();

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
        int i = m_comboTimeZone->findText(text, Qt::MatchFixedString);
        if (m_comboTimeZone->currentIndex() != i)
            m_comboTimeZone->setCurrentIndex(i);
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
        m_dateMsgId = m_dcp.sendMessage("dcptime", "get date").snr();
        m_timeMsgId = m_dcp.sendMessage("dcptime", "get time").snr();
        m_julianMsgId = m_dcp.sendMessage("dcptime", "get julian").snr();
        m_modeMsgId = m_dcp.sendMessage("dcptime", "get mode").snr();
    }
}

void DcpTimeClient::comboTimeZone_activated(const QString &text)
{
    m_dcp.sendMessage("dcptime", QString("set mode %1")
                      .arg(text.toLower()).toLatin1());
}
