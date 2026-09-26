/*
 * Copyright (c) 2020-2025, Ilya Kotov <forkotov02@ya.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include "qt6ct.h"
#include "fontconfigdialog.h"
#include "ui_fontconfigdialog.h"

FontConfigDialog::FontConfigDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::FontConfigDialog)
{
    m_ui->setupUi(this);

    m_ui->hintingStyleComboBox->addItem(tr("None"), u"hintnone"_s);
    m_ui->hintingStyleComboBox->addItem(tr("Slight"), u"hintslight"_s);
    m_ui->hintingStyleComboBox->addItem(tr("Medium"), u"hintmedium"_s);
    m_ui->hintingStyleComboBox->addItem(tr("Full"), u"hintfull"_s);

    m_ui->rgbaComboBox->addItem(tr("None"), u"none"_s);
    m_ui->rgbaComboBox->addItem(u"rgb"_s, u"rgb"_s);
    m_ui->rgbaComboBox->addItem(u"bgr"_s, u"bgr"_s);
    m_ui->rgbaComboBox->addItem(u"vrgb"_s, u"vrgb"_s);
    m_ui->rgbaComboBox->addItem(u"vbgr"_s, u"vbgr"_s);

    m_ui->lcdFilterComboBox->addItem(u"lcdnone"_s);
    m_ui->lcdFilterComboBox->addItem(u"lcddefault"_s);
    m_ui->lcdFilterComboBox->addItem(u"lcdlight"_s);
    m_ui->lcdFilterComboBox->addItem(u"lcdlegacy"_s);
}

FontConfigDialog::~FontConfigDialog()
{
    delete m_ui;
}

void FontConfigDialog::accept()
{
    QDir::home().mkpath(u".config/fontconfig/"_s);
    QString path = QDir::homePath() + u"/.config/fontconfig/fonts.conf"_s;
    qDebug("FontConfigDialog: fontconfig path: %s", qPrintable(path));


    if(QFile::exists(path))
    {
        if(QMessageBox::question(this, tr("Font Configuration"),
                                 tr("<i>%1</i> already exists. Do you want to replace it?").arg(path),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        {
            QDialog::reject();
            return;
        }

        QFile::remove(path + u".back"_s);
        QFile::copy(path, path + u".back"_s);
    }

    QFile file(path);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning("FontConfigDialog: unable to open file: %s", qPrintable(file.errorString()));
        return;
    }

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);

    stream.writeStartDocument();
    stream.writeDTD("<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">"_L1);
    stream.writeStartElement("fontconfig"_L1);

    stream.writeStartElement("match"_L1);
    stream.writeAttribute("target"_L1, "font"_L1);
    writeOption(&stream, u"antialias"_s, m_ui->antialisingCheckBox->isChecked());
    writeOption(&stream, u"hinting"_s, m_ui->hintingCheckBox->isChecked());
    writeOption(&stream, u"hintstyle"_s, m_ui->hintingStyleComboBox->currentData().toString());
    writeOption(&stream, u"rgba"_s, m_ui->rgbaComboBox->currentData().toString());
    writeOption(&stream, u"autohint"_s, m_ui->autohinterCheckBox->isChecked());
    writeOption(&stream, u"lcdfilter"_s, m_ui->lcdFilterComboBox->currentText());
    writeOption(&stream, u"dpi"_s, m_ui->dpiSpinBox->value());
    stream.writeEndElement();

    if(m_ui->disableBoldAutohintCheckBox->isChecked())
    {
        stream.writeStartElement("match"_L1);
        stream.writeAttribute("target"_L1, "font"_L1);

        stream.writeStartElement("test"_L1);
        stream.writeAttribute("name"_L1, "weight"_L1);
        stream.writeAttribute("compare"_L1, "more"_L1);
        stream.writeTextElement("const"_L1, "medium"_L1);
        stream.writeEndElement();

        writeOption(&stream, u"autohint"_s, m_ui->autohinterCheckBox->isChecked());

        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndDocument();

    QDialog::accept();
}

void FontConfigDialog::writeOption(QXmlStreamWriter *stream, const QString &name, const QVariant &value)
{
    stream->writeStartElement("edit"_L1);
    stream->writeAttribute("name"_L1, name);
    stream->writeAttribute("mode"_L1, "assign"_L1);
    if(value.typeId() == QMetaType::QString)
        stream->writeTextElement("const"_L1, value.toString());
    else if(value.typeId() == QMetaType::Int)
        stream->writeTextElement("double"_L1, QString::number(value.toInt()));
    else if(value.typeId() == QMetaType::Bool)
        stream->writeTextElement("bool"_L1, value.toBool() ? QStringLiteral("true") : QStringLiteral("false"));
    stream->writeEndElement();
}
