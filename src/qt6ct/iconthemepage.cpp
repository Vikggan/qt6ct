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
#include <QFileInfo>
#include <QFileInfoList>
#include <QDir>
#include <QTreeWidgetItem>
#include <QProgressBar>
#include <QMetaObject>
#include <QThread>
#ifdef KF_CONFIGCORE_LIB
#include <KConfigGroup>
#endif
#include "qt6ct.h"
#include "iconthemepage.h"
#include "ui_iconthemepage.h"

IconThemePage::IconThemePage(QWidget *parent) :
    TabPage(parent),
    m_ui(new Ui::IconThemePage)
{
    m_ui->setupUi(this);
    m_progressBar = new QProgressBar(m_ui->treeWidget);
    m_progressBar->resize(200, m_progressBar->height());
    m_progressBar->setRange(0, 100);

    m_thread = QThread::create(&IconThemePage::loadThemes, this);
    m_thread->setParent(this);
    connect(m_thread, &QThread::finished, this, &IconThemePage::onFinished);
    m_thread->start();
}

IconThemePage::~IconThemePage()
{
    if(m_thread->isRunning())
    {
        m_stopped = true;
        m_thread->wait();
    }

    delete m_ui;
}

void IconThemePage::writeSettings(QSettings *settings)
{
    QTreeWidgetItem *item = m_ui->treeWidget->currentItem();
    if(item)
        settings->setValue("Appearance/icon_theme"_L1, item->data(3, Qt::UserRole));
}

#ifdef KF_CONFIGCORE_LIB
void IconThemePage::writeSettings(KSharedConfigPtr config)
{
    QTreeWidgetItem *item = m_ui->treeWidget->currentItem();
    if(!item)
        return;

    KConfigGroup group(config, "Icons");
    group.writeEntry("Theme", item->data(3, Qt::UserRole));
    group.sync();
}
#endif

void IconThemePage::onFinished()
{
    m_ui->treeWidget->addTopLevelItems(m_items);
    m_ui->treeWidget->resizeColumnToContents(0);
    m_ui->treeWidget->resizeColumnToContents(1);
    m_ui->treeWidget->resizeColumnToContents(2);
    m_ui->treeWidget->resizeColumnToContents(3);
    m_progressBar->hide();
    m_items.clear();
    readSettings();
}

void IconThemePage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_progressBar->move(m_ui->treeWidget->width() - m_progressBar->width(),
                        m_ui->treeWidget->height() - m_progressBar->height());
}

void IconThemePage::readSettings()
{
    QSettings settings(Qt6CT::configFile(), QSettings::IniFormat);
    QString name = settings.value("Appearance/icon_theme"_L1).toString();

    if(name.isEmpty())
        return;

    for(int i = 0; i < m_ui->treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = m_ui->treeWidget->topLevelItem(i);
        if(item->data(3, Qt::UserRole).toString() == name)
        {
            m_ui->treeWidget->setCurrentItem(item);
            break;
        }
    }
}

void IconThemePage::loadThemes()
{
    m_ui->treeWidget->clear();
    m_items.clear();
    m_stopped = false;

    QFileInfoList themeFileList;
    for(const QString &path : Qt6CT::iconPaths())
    {
        QDir dir(path);
        dir.setFilter(QDir::Dirs | QDir::NoDotDot | QDir::NoDot);
        for(const QFileInfo &info : dir.entryInfoList())
        {
            QDir themeDir(info.absoluteFilePath());
            themeFileList << themeDir.entryInfoList({ u"index.theme"_s }, QDir::Files);
        }

        if(m_stopped)
            return;
    }

    int i = 0;
    for(const QFileInfo &info : themeFileList)
    {
        QTreeWidgetItem *item = loadTheme(info.canonicalFilePath());
        if(item)
            m_items << item;

        if(m_stopped)
        {
            qDeleteAll(m_items);
            m_items.clear();
            return;
        }

        QMetaObject::invokeMethod(m_progressBar, "setValue", Qt::QueuedConnection, Q_ARG(int, ++i * 100 / themeFileList.count()));
    }
}

QTreeWidgetItem *IconThemePage::loadTheme(const QString &path)
{
    QSettings config(path, QSettings::IniFormat);

    config.beginGroup("Icon Theme"_L1);
    QStringList dirs = config.value("Directories"_L1).toStringList();
    if(dirs.isEmpty() || config.value("Hidden"_L1, false).toBool())
        return nullptr;

    QString lang = QLocale::system().name();

    QString name = config.value(QStringLiteral("Name[%1]").arg(lang)).toString();
    QString comment = config.value(QStringLiteral("Comment[%1]").arg(lang)).toString();

    if(lang.contains(QLatin1Char('_')))
        lang = lang.split(QLatin1Char('_')).first();

    if(name.isEmpty())
        name = config.value(QStringLiteral("Name[%1]").arg(lang)).toString();

    if(comment.isEmpty())
        comment = config.value(QStringLiteral("Comment[%1]").arg(lang)).toString();

    if(name.isEmpty())
        name = config.value("Name"_L1).toString();

    if(comment.isEmpty())
        comment = config.value("Comment"_L1).toString();

    config.endGroup();

    QIcon icon1 = findIcon(path, 24, u"document-save"_s);
    QIcon icon2 = findIcon(path, 24, u"document-print"_s);
    QIcon icon3 = findIcon(path, 24, u"media-playback-stop"_s);

    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setIcon(0, icon1);
    item->setIcon(1, icon2);
    item->setIcon(2, icon3);
    item->setText(3, name);
    item->setData(3, Qt::UserRole, QFileInfo(path).path().section(QLatin1Char('/'), -1));
    item->setToolTip(3, comment);
    item->setSizeHint(0, QSize(24,24));
    return item;
}

QIcon IconThemePage::findIcon(const QString &themePath, int size, const QString &name)
{
    QStringList visited;
    return findIconHelper(themePath, size, name, &visited);
}

QIcon IconThemePage::findIconHelper(const QString &themePath, int size, const QString &name, QStringList *visited)
{
    QSettings config(themePath, QSettings::IniFormat);
    config.beginGroup("Icon Theme"_L1);
    QStringList dirs = config.value("Directories"_L1).toStringList();
    QStringList parents = config.value("Inherits"_L1).toStringList();
    visited->append(config.value("Name"_L1).toString());
    bool haveInherits = !parents.isEmpty();
    config.endGroup();

    QString iconPath;
    int iconSize = 0;

    for(const QString &dir : dirs)
    {
        config.beginGroup(dir);
        QDir iconDir = QFileInfo(themePath).path() + QLatin1Char('/') + dir;

        QString p;

        if(iconDir.exists(name + u".png"_s))
            p = iconDir.absoluteFilePath(name + u".png"_s);
        else if(iconDir.exists(name + u".svg"_s))
            p = iconDir.absoluteFilePath(name + u".svg"_s);
        else
        {
            iconDir.setFilter(QDir::Files);
            iconDir.setNameFilters(QStringList () << name + u"-*.*"_s);
            if(!iconDir.entryInfoList().isEmpty())
                p = iconDir.entryInfoList().constFirst().absoluteFilePath();
        }

        if(p.isEmpty())
            continue;

        if(!iconSize || abs(size - iconSize) > abs(size - config.value("Size"_L1).toInt()))
        {
            iconSize = config.value("Size"_L1).toInt();
            iconPath = p;

            if(iconSize == size)
            {
                config.endGroup();
                break;
            }
        }
        config.endGroup();
    }

    if(!iconPath.isEmpty())
        return QIcon(iconPath);

    if (!haveInherits)
        return QIcon();

    parents.append(u"hicolor"_s); //add fallback themes
    parents.append(u"gnome"_s);
    parents.removeDuplicates();

    for(const QString &parent : parents)
    {
        QString parentThemePath = QDir(QFileInfo(themePath).path() + u"/../"_s + parent).canonicalPath() + u"/index.theme"_s;

        if(!QFile::exists(parentThemePath) || visited->contains(parent)) //protect against recursion
            continue;

        QIcon icon = findIconHelper(parentThemePath, size, name, visited);
        if(!icon.isNull())
            return icon;
    }

    return QIcon();
}
