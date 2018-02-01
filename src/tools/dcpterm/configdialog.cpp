/*
 * Copyright (c) 2012 Kolja Glogowski
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

#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    ui->editDeviceName->selectAll();
    ui->editDeviceName->setFocus();
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

QString ConfigDialog::serverName() const
{
    return ui->editServerName->text();
}

void ConfigDialog::setServerName(const QString &serverName)
{
    ui->editServerName->setText(serverName);
}

quint16 ConfigDialog::serverPort() const
{
    return quint16(ui->spinServerPort->value());
}

void ConfigDialog::setServerPort(quint16 serverPort)
{
    ui->spinServerPort->setValue(serverPort);
}

QString ConfigDialog::deviceName() const
{
    return ui->editDeviceName->text();
}

void ConfigDialog::setDeviceName(const QString &deviceName)
{
    ui->editDeviceName->setText(deviceName);
}

QString ConfigDialog::encoding() const
{
    return ui->comboEncoding->currentText();
}

void ConfigDialog::setEncoding(const QString &encoding)
{
    int index = ui->comboEncoding->findText(encoding, Qt::MatchFixedString);
    if (index == -1)
        index = 0;  // select Latin1 if encoding is unknown
    ui->comboEncoding->setCurrentIndex(index);
}
