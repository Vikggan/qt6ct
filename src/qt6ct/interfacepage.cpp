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

#include <QSettings>
#include <QApplication>
#include <QDialogButtonBox>
#include <qpa/qplatformtheme.h>
#include "qt6ct.h"
#include "interfacepage.h"
#include "ui_interfacepage.h"

InterfacePage::InterfacePage(QWidget *parent) :
    TabPage(parent),
    m_ui(new Ui::InterfacePage)
{
    m_ui->setupUi(this);

    m_ui->buttonLayoutComboBox->addItem(u"Windows"_s, QDialogButtonBox::WinLayout);
    m_ui->buttonLayoutComboBox->addItem(u"Mac OS X"_s, QDialogButtonBox::MacLayout);
    m_ui->buttonLayoutComboBox->addItem(u"KDE"_s, QDialogButtonBox::KdeLayout);
    m_ui->buttonLayoutComboBox->addItem(u"GNOME"_s, QDialogButtonBox::GnomeLayout);

    m_ui->keyboardSchemeComboBox->addItem(u"Windows"_s, QPlatformTheme::WindowsKeyboardScheme);
    m_ui->keyboardSchemeComboBox->addItem(u"Mac OS X"_s, QPlatformTheme::MacKeyboardScheme);
    m_ui->keyboardSchemeComboBox->addItem(u"X11"_s, QPlatformTheme::X11KeyboardScheme);
    m_ui->keyboardSchemeComboBox->addItem(u"KDE"_s, QPlatformTheme::KdeKeyboardScheme);
    m_ui->keyboardSchemeComboBox->addItem(u"GNOME"_s, QPlatformTheme::GnomeKeyboardScheme);
    m_ui->keyboardSchemeComboBox->addItem(u"CDE"_s, QPlatformTheme::CdeKeyboardScheme);

    m_ui->toolButtonStyleComboBox->addItem(tr("Only display the icon"), Qt::ToolButtonIconOnly);
    m_ui->toolButtonStyleComboBox->addItem(tr("Only display the text"), Qt::ToolButtonTextOnly);
    m_ui->toolButtonStyleComboBox->addItem(tr("The text appears beside the icon"), Qt::ToolButtonTextBesideIcon);
    m_ui->toolButtonStyleComboBox->addItem(tr("The text appears under the icon"), Qt::ToolButtonTextUnderIcon);
    m_ui->toolButtonStyleComboBox->addItem(tr("Follow the application style"), Qt::ToolButtonFollowStyle);

    readSettings();
}

InterfacePage::~InterfacePage()
{
    delete m_ui;
}

void InterfacePage::writeSettings(QSettings *settings)
{
    settings->beginGroup("Interface"_L1);
    settings->setValue("double_click_interval"_L1, m_ui->doubleClickIntervalSpinBox->value());
    settings->setValue("cursor_flash_time"_L1, m_ui->cursorFlashTimeSpinBox->value());
    settings->setValue("buttonbox_layout"_L1, m_ui->buttonLayoutComboBox->currentData());
    settings->setValue("keyboard_scheme"_L1, m_ui->keyboardSchemeComboBox->currentData());
    settings->setValue("menus_have_icons"_L1, m_ui->menuIconsCheckBox->isChecked());
    settings->setValue("show_shortcuts_in_context_menus"_L1, m_ui->showShortcutsInMenusCheckBox->isChecked());

    settings->setValue("underline_shortcut"_L1, m_ui->shortcutUnderlineCheckBox->checkState());
    settings->setValue("activate_item_on_single_click"_L1, m_ui->singleClickCheckBox->checkState());
    settings->setValue("dialog_buttons_have_icons"_L1, m_ui->dialogIconsCheckBox->checkState());
    settings->setValue("toolbutton_style"_L1, m_ui->toolButtonStyleComboBox->currentData());
    settings->setValue("wheel_scroll_lines"_L1, m_ui->wheelScrollLinesSpinBox->value());

    QStringList effects;
    if(m_ui->guiEffectsCheckBox->isChecked())
        effects << u"General"_s;

    if(m_ui->menuEffectComboBox->currentIndex() == 1)
        effects << u"AnimateMenu"_s;
    else if(m_ui->menuEffectComboBox->currentIndex() == 2)
        effects << u"FadeMenu"_s;

    if(m_ui->comboBoxEffectComboBox->currentIndex() == 1)
        effects << u"AnimateCombo"_s;

    if(m_ui->toolTipEffectComboBox->currentIndex() == 1)
        effects << u"AnimateTooltip"_s;
    else if(m_ui->toolTipEffectComboBox->currentIndex() == 2)
        effects << u"FadeTooltip"_s;

    if(m_ui->toolBoxEffectComboBox->currentIndex() == 1)
        effects << u"AnimateToolBox"_s;

    settings->setValue("gui_effects"_L1, effects);
    settings->endGroup();
}

void InterfacePage::readSettings()
{
    QSettings settings(Qt6CT::configFile(), QSettings::IniFormat);
    settings.beginGroup("Interface"_L1);
    m_ui->doubleClickIntervalSpinBox->setValue(qApp->doubleClickInterval());
    m_ui->cursorFlashTimeSpinBox->setValue(qApp->cursorFlashTime());

    m_ui->guiEffectsCheckBox->setChecked(qApp->isEffectEnabled(Qt::UI_General));

    int layout = settings.value("buttonbox_layout"_L1, style()->styleHint(QStyle::SH_DialogButtonLayout)).toInt();
    int index = m_ui->buttonLayoutComboBox->findData(layout);
    if(index >= 0)
        m_ui->buttonLayoutComboBox->setCurrentIndex(index);

    int scheme = settings.value("keyboard_scheme"_L1, QPlatformTheme::X11KeyboardScheme).toInt();
    index = m_ui->keyboardSchemeComboBox->findData(scheme);
    if(index >= 0)
        m_ui->keyboardSchemeComboBox->setCurrentIndex(index);

    if(qApp->isEffectEnabled(Qt::UI_AnimateMenu))
        m_ui->menuEffectComboBox->setCurrentIndex(1);
    else if(qApp->isEffectEnabled(Qt::UI_FadeMenu))
        m_ui->menuEffectComboBox->setCurrentIndex(2);

    if(qApp->isEffectEnabled(Qt::UI_AnimateCombo))
        m_ui->comboBoxEffectComboBox->setCurrentIndex(1);

    if(qApp->isEffectEnabled(Qt::UI_AnimateTooltip))
        m_ui->toolTipEffectComboBox->setCurrentIndex(1);
    else  if(qApp->isEffectEnabled(Qt::UI_FadeTooltip))
        m_ui->toolTipEffectComboBox->setCurrentIndex(2);

    if(qApp->isEffectEnabled(Qt::UI_AnimateToolBox))
        m_ui->toolBoxEffectComboBox->setCurrentIndex(1);

    m_ui->singleClickCheckBox->setCheckState(Qt::CheckState(settings.value("activate_item_on_single_click"_L1, Qt::PartiallyChecked).toInt()));
    m_ui->dialogIconsCheckBox->setCheckState(Qt::CheckState(settings.value("dialog_buttons_have_icons"_L1, Qt::PartiallyChecked).toInt()));
    m_ui->shortcutUnderlineCheckBox->setCheckState(Qt::CheckState(settings.value("underline_shortcut"_L1, Qt::PartiallyChecked).toInt()));

    m_ui->menuIconsCheckBox->setChecked(!qApp->testAttribute(Qt::AA_DontShowIconsInMenus));
    m_ui->showShortcutsInMenusCheckBox->setChecked(settings.value("show_shortcuts_in_context_menus"_L1, true).toBool());
    m_ui->showShortcutsInMenusCheckBox->setVisible(false);

    int toolbarStyle = settings.value("toolbutton_style"_L1, Qt::ToolButtonFollowStyle).toInt();
    index = m_ui->toolButtonStyleComboBox->findData(toolbarStyle);
    if(index >= 0)
        m_ui->toolButtonStyleComboBox->setCurrentIndex(index);

    m_ui->wheelScrollLinesSpinBox->setValue(settings.value("wheel_scroll_lines"_L1, 3).toInt());

    settings.endGroup();
}
