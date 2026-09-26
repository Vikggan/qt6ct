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

#ifndef QT6CTPLATFORMTHEME_H
#define QT6CTPLATFORMTHEME_H

#include <qpa/qplatformtheme.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
#include <private/qgenericunixtheme_p.h>
#else
#include <private/qgenericunixthemes_p.h>
#endif
#include <QObject>
#include <QFont>
#include <QPalette>
#include <QLoggingCategory>
#include <QIcon>
#include <QFileInfo>
#include <memory>
#include <optional>

class QStyle;

Q_DECLARE_LOGGING_CATEGORY(lqt6ct)

class Qt6CTPlatformTheme : public QObject, public QGenericUnixTheme
{
    Q_OBJECT
public:
    Qt6CTPlatformTheme();

    virtual ~Qt6CTPlatformTheme();

    //virtual QPlatformMenuItem* createPlatformMenuItem() const;
    //virtual QPlatformMenu* createPlatformMenu() const;
    //virtual void showPlatformMenuBar() {}
    virtual bool usePlatformNativeDialog(DialogType type) const override;
    virtual QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    virtual Qt::ColorScheme colorScheme() const override;
#endif
    virtual const QPalette *palette(Palette type = SystemPalette) const override;
    virtual const QFont *font(Font type = SystemFont) const override;
    virtual QVariant themeHint(ThemeHint hint) const override;
    virtual QIcon fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions iconOptions = {}) const override;
    //virtual QPixmap standardPixmap(StandardPixmap sp, const QSizeF &size) const;
    //virtual QPixmap fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size,
    //                               QPlatformTheme::IconOptions iconOptions = 0) const;

#ifdef KF_ICONTHEMES_LIB
    virtual QIconEngine *createIconEngine(const QString &iconName) const override;
#endif
    //virtual QList<QKeySequence> keyBindings(QKeySequence::StandardKey key) const;
    //virtual QString standardButtonText(int button) const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void applySettings();
    void createFSWatcher();
    void updateSettings();

private:
    void readSettings();
    QPlatformTheme *dialogTheme() const;
#ifdef QT_WIDGETS_LIB
    bool hasWidgets();
#endif
    QString loadStyleSheets(const QStringList &paths);
    QString m_schemePath, m_iconTheme, m_dialogTheme, m_userStyleSheet, m_prevStyleSheet;
    std::optional<QPalette> m_palette;
    QFont m_generalFont, m_fixedFont;
    int m_doubleClickInterval;
    int m_cursorFlashTime;
    int m_uiEffects;
    int m_buttonBoxLayout;
    int m_keyboardScheme;
    bool m_update = false;
    bool m_usePalette = true;
    int m_toolButtonStyle = Qt::ToolButtonFollowStyle;
    int m_wheelScrollLines = 3;
    bool m_showShortcutsInContextMenus = false;
    bool m_isIgnored = false;
    mutable bool m_inColorScheme = false;
    mutable std::unique_ptr<QPlatformTheme> m_theme;
#ifdef QT_WIDGETS_LIB
    std::unique_ptr<QStyle> m_style;
#endif
};

Q_DECLARE_LOGGING_CATEGORY(lqt6ct)

#endif // QT6CTPLATFORMTHEME_H
