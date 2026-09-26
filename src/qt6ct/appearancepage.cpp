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

#include <QStyleFactory>
#include <QMdiSubWindow>
#include <QSettings>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QIcon>
#include <QStringList>
#include <qpa/qplatformthemefactory_p.h>
#include <qpa/qplatformtheme.h>
#if defined KF_CONFIGCORE_LIB && defined KF_COLORSCHEME_LIB
#include <KConfigGroup>
#include <KSharedConfig>
#endif
#include "qt6ct.h"
#include "appearancepage.h"
#include "paletteeditdialog.h"
#include "ui_appearancepage.h"
#include "ui_previewform.h"

AppearancePage::AppearancePage(QWidget *parent) :
    TabPage(parent),
    m_ui(new Ui::AppearancePage)
{
    m_ui->setupUi(this);
    QStringList keys = QStyleFactory::keys();
    keys.removeAll(u"qt6ct-style"_s); //hide qt6ct proxy style
    keys.removeAll(u"qt5ct-style"_s); //hide qt5ct proxy style
    keys.removeAll(u"qt5gtk2"_s); //hide qt5gtk2 alias
    keys.removeAll(u"gtk2"_s); //hide gtk2 alias
    m_ui->styleComboBox->addItems(keys);

    connect(m_ui->paletteComboBox, &QComboBox::activated, this, &AppearancePage::updatePalette);

    m_previewWidget = new QWidget(this);
    m_previewUi = new Ui::PreviewForm();
    m_previewUi->setupUi(m_previewWidget);
    QMdiSubWindow *w = m_ui->mdiArea->addSubWindow(m_previewWidget, Qt::SubWindow | Qt::CustomizeWindowHint
                                                   | Qt::WindowMinMaxButtonsHint
                                                   | Qt::WindowTitleHint
                                                   | Qt::WindowDoesNotAcceptFocus);
    w->setFocusPolicy(Qt::NoFocus);
    w->move(10, 10);

    QMenu *menu = new QMenu(this);
    menu->addAction(QIcon::fromTheme(u"document-new"_s), tr("Create"), this, qOverload<>(&AppearancePage::createColorScheme));
    m_changeColorSchemeAction = menu->addAction(QIcon::fromTheme(u"accessories-text-editor"_s), tr("Edit"), this, &AppearancePage::changeColorScheme);
    m_copyColorSchemeAction = menu->addAction(QIcon::fromTheme(u"edit-copy"_s), tr("Create a Copy"), this, &AppearancePage::copyColorScheme);
    m_renameColorSchemeAction = menu->addAction(tr("Rename"), this, &AppearancePage::renameColorScheme);
    menu->addSeparator();
    m_removeColorSchemeAction = menu->addAction(QIcon::fromTheme(u"edit-delete"_s), tr("Remove"), this, &AppearancePage::removeColorScheme);
    m_ui->colorSchemeButton->setMenu(menu);

    m_changeColorSchemeAction->setIcon(QIcon::fromTheme(u"accessories-text-editor"_s));
    m_removeColorSchemeAction->setIcon(QIcon::fromTheme(u"list-remove"_s));
    connect(menu, &QMenu::aboutToShow, this, &AppearancePage::updateActions);

    keys = QPlatformThemeFactory::keys();
    m_ui->dialogComboBox->addItem(tr("Default"), u"default"_s);
    if(keys.contains(u"gtk2"_s) || keys.contains(u"qt6gtk2"_s))
        m_ui->dialogComboBox->addItem(u"GTK2"_s, u"gtk2"_s);
    else if(keys.contains(u"gtk3"_s) || keys.contains(u"qt6gtk3"_s))
        m_ui->dialogComboBox->addItem(u"GTK3"_s, u"gtk3"_s);
    if(keys.contains(u"kde"_s))
        m_ui->dialogComboBox->addItem(u"KDE"_s, u"kde"_s);
    if(keys.contains(u"xdgdesktopportal"_s))
        m_ui->dialogComboBox->addItem(u"XDG Desktop Portal"_s, u"xdgdesktopportal"_s);

    readSettings();
}

AppearancePage::~AppearancePage()
{
    delete m_selectedStyle;
    delete m_ui;
    delete m_previewUi;
}

void AppearancePage::writeSettings(QSettings *settings)
{
    settings->beginGroup("Appearance"_L1);
    settings->setValue("style"_L1, m_ui->styleComboBox->currentText());

    if(m_ui->colorSchemeComboBox->currentData().toString() == QLatin1String("system"))
    {
        settings->setValue("custom_palette"_L1, false);
    }
    else if(m_ui->colorSchemeComboBox->currentData().toString() == QLatin1String("style"))
    {
        settings->setValue("custom_palette"_L1, true);
        settings->setValue("color_scheme_path"_L1, Qt6CT::styleColorSchemeFile());
        createColorScheme(Qt6CT::styleColorSchemeFile(), m_previewWidget->palette());
    }
    else
    {
        settings->setValue("custom_palette"_L1, true);
        settings->setValue("color_scheme_path"_L1, m_ui->colorSchemeComboBox->currentData().toString());
    }

    settings->setValue("standard_dialogs"_L1, m_ui->dialogComboBox->currentData().toString());
    settings->endGroup();
}

#ifdef KF_CONFIGCORE_LIB
void AppearancePage::writeSettings(KSharedConfigPtr config)
{
    KConfigGroup group(config, "KDE");
    group.writeEntry("widgetStyle", "qt6ct-style");
    group.sync();
}
#endif

void AppearancePage::on_styleComboBox_textActivated(const QString &text)
{
    QStyle *style = QStyleFactory::create(text);
    if(!style)
        return;
    setStyle(m_previewWidget, style);

    delete m_selectedStyle;
    m_selectedStyle = style;

    if(m_ui->colorSchemeComboBox->currentData().toString() == QLatin1String("style"))
        m_customPalette = m_selectedStyle->standardPalette();

    updatePalette();
}

void AppearancePage::on_colorSchemeComboBox_activated(int)
{
    QString data = m_ui->colorSchemeComboBox->currentData().toString();

    if(data == QLatin1String("system"))
    {
        QPlatformTheme t;
        m_customPalette = *t.palette();
    }
    else if(data == QLatin1String("style"))
    {
        m_customPalette = m_selectedStyle->standardPalette();
    }
    else
    {
        m_customPalette = Qt6CT::loadColorScheme(data).value_or(palette());
    }
    updatePalette();
}

void AppearancePage::createColorScheme()
{
    QString name = QInputDialog::getText(this, tr("Enter Color Scheme Name"), tr("File name:"));
    if(name.isEmpty())
        return;

    if(!name.endsWith(u".conf"_s, Qt::CaseInsensitive))
        name.append(u".conf"_s);

    if(m_ui->colorSchemeComboBox->findText(name.section(QLatin1Char('.'),0,0)) != -1)
    {
        QMessageBox::warning(this, tr("Error"), tr("The color scheme \"%1\" already exists")
                             .arg(name.section(QLatin1Char('.'),0,0)));
        return;
    }

    QString schemePath = Qt6CT::userColorSchemePath() + QLatin1Char('/') + name;

    createColorScheme(schemePath, palette());
    m_ui->colorSchemeComboBox->addItem(name.section(QLatin1Char('.'),0,0), schemePath);
}

void AppearancePage::changeColorScheme()
{
    if(m_ui->colorSchemeComboBox->currentIndex() < 0)
        return;

    if(!QFileInfo(m_ui->colorSchemeComboBox->currentData().toString()).isWritable())
    {
        QMessageBox::information(this, tr("Warning"), tr("The color scheme \"%1\" is read only")
                                 .arg(m_ui->colorSchemeComboBox->currentText()));
        return;
    }

    PaletteEditDialog d(m_customPalette, m_selectedStyle, this);
    connect(&d, &PaletteEditDialog::paletteChanged, this, &AppearancePage::setPreviewPalette);
    if(d.exec() == QDialog::Accepted)
    {
        m_customPalette = d.selectedPalette();
        createColorScheme(m_ui->colorSchemeComboBox->currentData().toString(), m_customPalette);
    }
    updatePalette();
}

void AppearancePage::removeColorScheme()
{
    int index = m_ui->colorSchemeComboBox->currentIndex();
    if(index < 0 || m_ui->colorSchemeComboBox->count() <= 1)
        return;

    if(!QFileInfo(m_ui->colorSchemeComboBox->currentData().toString()).isWritable())
    {
        QMessageBox::information(this, tr("Warning"), tr("The color scheme \"%1\" is read only")
                                 .arg(m_ui->colorSchemeComboBox->currentText()));
        return;
    }

    int button = QMessageBox::question(this, tr("Confirm Remove"),
                                       tr("Are you sure you want to remove color scheme \"%1\"?")
                                       .arg(m_ui->colorSchemeComboBox->currentText()),
                                       QMessageBox::Yes | QMessageBox::No);
    if(button != QMessageBox::Yes)
        return;

    if(QFile::remove(m_ui->colorSchemeComboBox->currentData().toString()))
    {
        m_ui->colorSchemeComboBox->removeItem(index);
        on_colorSchemeComboBox_activated(0);
    }
}

void AppearancePage::copyColorScheme()
{
    if(m_ui->colorSchemeComboBox->currentIndex() < 0)
        return;

    QString name = QInputDialog::getText(this, tr("Enter Color Scheme Name"), tr("File name:"),
                                         QLineEdit::Normal,
                                         tr("%1 (copy)").arg(m_ui->colorSchemeComboBox->currentText()));
    if(name.isEmpty() || name == m_ui->colorSchemeComboBox->currentText())
        return;

    if(!name.endsWith(u".conf"_s, Qt::CaseInsensitive))
        name.append(u".conf"_s);

    if(m_ui->colorSchemeComboBox->findText(name.section(QLatin1Char('.'),0,0)) != -1)
    {
        QMessageBox::warning(this, tr("Error"), tr("The color scheme \"%1\" already exists")
                             .arg(name.section(QLatin1Char('.'),0,0)));
        return;
    }

    QString newPath = Qt6CT::userColorSchemePath() + QLatin1String("/") + name;
    if(!QFile::copy(m_ui->colorSchemeComboBox->currentData().toString(), newPath))
    {
        QMessageBox::warning(this, tr("Error"), tr("Unable to copy file"));
        return;
    }
    m_ui->colorSchemeComboBox->addItem(name.section(QLatin1Char('.'),0,0), newPath);
}

void AppearancePage::renameColorScheme()
{
    int index = m_ui->colorSchemeComboBox->currentIndex();

    if(index < 0)
        return;

    if(!QFileInfo(m_ui->colorSchemeComboBox->currentData().toString()).isWritable())
    {
        QMessageBox::information(this, tr("Warning"), tr("The color scheme \"%1\" is read only")
                                 .arg(m_ui->colorSchemeComboBox->currentText()));
        return;
    }

    QString name = QInputDialog::getText(this, tr("Enter Color Scheme Name"), tr("File name:"),
                                         QLineEdit::Normal, m_ui->colorSchemeComboBox->currentText());
    if(name.isEmpty() || name == m_ui->colorSchemeComboBox->currentText())
        return;

    if(!name.endsWith(u".conf"_s, Qt::CaseInsensitive))
        name.append(u".conf"_s);

    if(m_ui->colorSchemeComboBox->findText(name.section(QLatin1Char('.'),0,0)) != -1)
    {
        QMessageBox::warning(this, tr("Error"), tr("The color scheme \"%1\" already exists")
                             .arg(name.section(QLatin1Char('.'),0,0)));
        return;
    }

    QString newPath = Qt6CT::userColorSchemePath() + QLatin1Char('/') + name;
    QFile::rename(m_ui->colorSchemeComboBox->currentData().toString(), newPath);
    m_ui->colorSchemeComboBox->setItemText(index, name.section(QLatin1Char('.'),0,0));
    m_ui->colorSchemeComboBox->setItemData(index, newPath);
}

void AppearancePage::updatePalette()
{
    if(!m_selectedStyle)
        return;

    setPreviewPalette(m_customPalette);
}

void AppearancePage::setPreviewPalette(const QPalette &p)
{
    QPalette previewPalette = palette();

    QPalette::ColorGroup colorGroup = QPalette::Disabled;

    if(m_ui->paletteComboBox->currentIndex() == 0)
    {
        colorGroup = QPalette::Active;
    }
    else if(m_ui->paletteComboBox->currentIndex() == 1)
    {
        colorGroup = QPalette::Inactive;
    }

    for (int i = 0; i < QPalette::NColorRoles; i++)
    {
        QPalette::ColorRole role = QPalette::ColorRole(i);
        previewPalette.setColor(QPalette::Active, role, p.color(colorGroup, role));
        previewPalette.setColor(QPalette::Inactive, role, p.color(colorGroup, role));
    }

    setPalette(m_ui->mdiArea, previewPalette);
}

void AppearancePage::updateActions()
{
    m_copyColorSchemeAction->setVisible(!Qt6CT::isKColorScheme(m_ui->colorSchemeComboBox->currentData().toString()));
    if(m_ui->colorSchemeComboBox->count() == 0 ||
            !QFileInfo(m_ui->colorSchemeComboBox->currentData().toString()).isWritable())
    {
        m_changeColorSchemeAction->setVisible(false);
        m_renameColorSchemeAction->setVisible(false);
        m_removeColorSchemeAction->setVisible(false);
    }
    else
    {
        m_changeColorSchemeAction->setVisible(true);
        m_renameColorSchemeAction->setVisible(true);
        m_removeColorSchemeAction->setVisible(m_ui->colorSchemeComboBox->count() > 1);
    }
}

void AppearancePage::changeEvent(QEvent *event)
{
    //restore preview palette after stylesheet change
    if(event->type() == QEvent::ThemeChange)
        updatePalette();

    TabPage::changeEvent(event);
}

void AppearancePage::readSettings()
{
    QSettings settings(Qt6CT::configFile(), QSettings::IniFormat);
    settings.beginGroup("Appearance"_L1);
    QString style = settings.value("style"_L1, u"Fusion"_s).toString();
    m_ui->styleComboBox->setCurrentText(style);

    QString colorSchemePath = settings.value("color_scheme_path"_L1).toString();
    colorSchemePath = Qt6CT::resolvePath(colorSchemePath); //replace environment variables

    m_ui->colorSchemeComboBox->addItem(tr("Default"), u"system"_s);
    m_ui->colorSchemeComboBox->addItem(tr("Style's colors"), u"style"_s);
    m_ui->colorSchemeComboBox->insertSeparator(2);

    QDir::root().mkpath(Qt6CT::userColorSchemePath());
    findColorSchemes(Qt6CT::userColorSchemePath());
    findColorSchemes(Qt6CT::sharedColorSchemePaths());

    if(settings.value("custom_palette"_L1, false).toBool())
    {
        int index = m_ui->colorSchemeComboBox->findData(colorSchemePath);
        if(index < 0 && colorSchemePath == Qt6CT::styleColorSchemeFile())
            index = m_ui->colorSchemeComboBox->findData(u"style"_s);

        m_ui->colorSchemeComboBox->setCurrentIndex(index);
        m_customPalette = Qt6CT::loadColorScheme(colorSchemePath).value_or(palette());
    }
    else
    {
        int index = m_ui->colorSchemeComboBox->findData("system"_L1);
        m_ui->colorSchemeComboBox->setCurrentIndex(index);
        QPlatformTheme t;
        m_customPalette = *t.palette();
    }

    on_styleComboBox_textActivated(m_ui->styleComboBox->currentText());

    int index = m_ui->dialogComboBox->findData(settings.value("standard_dialogs"_L1).toString());
    m_ui->dialogComboBox->setCurrentIndex(qMax(index, 0));

    settings.endGroup();
}

void AppearancePage::setStyle(QWidget *w, QStyle *s)
{
    for(QObject *o : w->children())
    {
        if(o->isWidgetType())
        {
            setStyle(qobject_cast<QWidget *>(o), s);
        }
    }
    w->setStyle(s);
}

void AppearancePage::setPalette(QWidget *w, QPalette p)
{
    for(QObject *o : w->children())
    {
        if(o->isWidgetType())
        {
            setPalette(qobject_cast<QWidget *>(o), p);
        }
    }
    w->setPalette(p);
}

void AppearancePage::findColorSchemes(const QString &path)
{
    QDir dir(path);
    dir.setFilter(QDir::Files);
    QStringList nameFilters = { u"*.conf"_s };
#if defined KF_CONFIGCORE_LIB && defined KF_COLORSCHEME_LIB
    nameFilters << u"*.colors"_s;
#endif
    dir.setNameFilters(nameFilters);

    for(const QFileInfo &info : dir.entryInfoList())
    {
        QString name = info.baseName();
        const QString path = info.filePath();
#if defined KF_CONFIGCORE_LIB && defined KF_COLORSCHEME_LIB
        if(Qt6CT::isKColorScheme(path))
        {
            KSharedConfigPtr config = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
            KConfigGroup group(config, "General");
            name = group.readEntry("Name", name) + QLatin1String(" (KColorScheme)");
        }
#endif
        m_ui->colorSchemeComboBox->addItem(name, path);
    }
}

void AppearancePage::findColorSchemes(const QStringList &paths)
{
    for(const QString &p : paths)
        findColorSchemes(p);
}

void AppearancePage::createColorScheme(const QString &name, const QPalette &palette)
{
    QSettings settings(name, QSettings::IniFormat);
    settings.beginGroup("ColorScheme"_L1);

    QStringList activeColors, inactiveColors, disabledColors;
    for (int i = 0; i < QPalette::NColorRoles; i++)
    {
        QPalette::ColorRole role = QPalette::ColorRole(i);
        activeColors << palette.color(QPalette::Active, role).name(QColor::HexArgb);
        inactiveColors << palette.color(QPalette::Inactive, role).name(QColor::HexArgb);
        disabledColors << palette.color(QPalette::Disabled, role).name(QColor::HexArgb);
    }

    settings.setValue("active_colors"_L1, activeColors);
    settings.setValue("inactive_colors"_L1, inactiveColors);
    settings.setValue("disabled_colors"_L1, disabledColors);

    settings.endGroup();
}
