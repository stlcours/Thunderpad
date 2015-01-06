//
//  This file is part of Thunderpad
//
//  Copyright (c) 2013-2015 Alex Spataru <alex_spataru@outlook.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301
//  USA
//

#include "window.h"

#include "menubar.h"
#include "toolbar.h"
#include "statusbar.h"
#include "searchdialog.h"

#define CURRENT_YEAR QDateTime::currentDateTime().toString("yyyy")

#define GNU_WARRANTY_WARNING "The program is provided AS IS with NO WARRANTY \
OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, \
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE."

#define LICENSE_LINK "http://www.gnu.org/copyleft/gpl.html"
#define DONATE_LINK "http://thunderpad.sf.net/donate"
#define HELP_LINK "http://thunderpad.sf.net/support"
#define FEED_BACK_LINK "mailto:alex_spataru@outlook.com"
#define REPORT_ISSUES_LINK "http://github.com/alex-97/thunderpad/issues"
#define CONTRIBUTE_LINK "http://thunderpad.sf.net/contribute"
#define WEBSITE_LINK "http://thunderpad.sourceforge.net"

Window::Window (void)
{
    setObjectName ("window");
    setAttribute (Qt::WA_DeleteOnClose);

    // Create the core components
    m_editor = new Editor (this);
    m_menu = new MenuBar (this);
    m_toolbar = new ToolBar (this);
    m_statusbar = new StatusBar (this);
    m_updater = new QSimpleUpdater (this);
    m_search_dialog = new SearchDialog (this);
    m_settings = new QSettings (APP_COMPANY, APP_NAME);

    // Connect slots between the text editor and window
    connect (editor(), SIGNAL (updateTitle()), this, SLOT (updateTitle()));
    connect (editor(), SIGNAL (textChanged()), this, SLOT (updateTitle()));
    connect (editor(), SIGNAL (settingsChanged()), this, SIGNAL (settingsChanged()));
    connect (qApp, SIGNAL (aboutToQuit()), this, SLOT (close()));
    connect (this, SIGNAL (updateSettings()), editor(), SLOT (updateSettings()));

    // Configure all widgets
    updateTitle();
    updateSettings();
    setCentralWidget (editor());

    // Set window geometry
    setMinimumSize (420, 420);
    resize (m_settings->value ("size", QSize (640, 420)).toSize());
    move (m_settings->value ("position", QPoint (200, 200)).toPoint());
    m_settings->value ("maximized", false).toBool() ? showMaximized() : showNormal();
}

Editor *Window::editor (void) const
{
    return m_editor;
}

ToolBar *Window::toolbar (void) const
{
    return m_toolbar;
}

void Window::closeEvent (QCloseEvent *event)
{
    saveWindowState();
    m_editor->maybeSave() ? event->accept() : event->ignore();
}

void Window::openFile (const QString &file_name)
{
    Q_ASSERT (!file_name.isEmpty());

    // Open the file in the same window
    if (m_editor->documentTitle().isEmpty() && !m_editor->document()->isModified())
        m_editor->readFile (file_name);

    // Open the file in another window
    else
        {
        Window *_window = new Window();
        configureWindow (_window);
        _window->editor()->readFile (file_name);
        }
}

void Window::newFile (void)
{
    Window *_window = new Window();
    configureWindow (_window);
}

void Window::open (void)
{
    QStringList _files = QFileDialog::getOpenFileNames (this, tr ("Open"), QDir::homePath());

    // Open each file separately
    for (int i = 0; i < _files.count(); ++i)
        if (!_files.at (i).isEmpty())
            openFile (_files.at (i));
}

void Window::setReadOnly (bool ro)
{
    m_editor->setReadOnly (ro);
    m_toolbar->setReadOnly (ro);

    emit readOnlyChanged (ro);
}

void Window::setWordWrap (bool ww)
{
    m_settings->setValue ("wordwrap-enabled", ww);
    syncSettings();
}

void Window::setToolbarText (bool tt)
{
    m_settings->setValue ("toolbar-text", tt);
    syncSettings();
}

void Window::setToolbarEnabled (bool tb)
{
    m_settings->setValue ("toolbar-enabled", tb);
    syncSettings();
}

void Window::setStatusBarEnabled (bool sb)
{
    m_settings->setValue ("statusbar-enabled", sb);
    syncSettings();
}

void Window::setHCLineEnabled (bool hc)
{
    m_settings->setValue ("hc-line-enabled", hc);
    syncSettings();
}

void Window::setUseLargeIcons (bool li)
{
    m_settings->setValue ("large-icons", li);
    syncSettings();
}

void Window::setLineNumbersEnabled (bool ln)
{
    m_settings->setValue ("line-numbers-enabled", ln);
    syncSettings();
}

void Window::setColorscheme (const QString &colorscheme)
{
    m_settings->setValue ("color-scheme", colorscheme);
    syncSettings();
}

void Window::showFindReplaceDialog (void)
{
    m_search_dialog->show();
}

void Window::setIconTheme (const QString &theme)
{
    m_settings->setValue ("icon-theme", theme);
    syncSettings();
}

void Window::aboutThunderpad (void)
{
    QString _message = QString ("<h2>%1 %2</h2>").arg (APP_NAME, APP_VERSION) +
                       "<p>" + tr ("Built on %1 at %2").arg (__DATE__, __TIME__) + "</p>" +
                       "<p>" + tr ("Copyright &copy; 2013-%1 %2").arg (CURRENT_YEAR, APP_COMPANY) + "</p>" +
                       "<p>" + tr (GNU_WARRANTY_WARNING) + "</p>";

    QMessageBox::about (this, tr ("About %1").arg (APP_NAME), _message);
}

void Window::license (void)
{
    QDesktopServices::openUrl (QUrl (LICENSE_LINK));
}

void Window::donate (void)
{
    QDesktopServices::openUrl (QUrl (DONATE_LINK));
}

void Window::viewHelp (void)
{
    QDesktopServices::openUrl (QUrl (HELP_LINK));
}

void Window::sendFeedback (void)
{
    QDesktopServices::openUrl (QUrl (FEED_BACK_LINK));
}

void Window::reportBug (void)
{
    QDesktopServices::openUrl (QUrl (REPORT_ISSUES_LINK));
}

void Window::makeContribution (void)
{
    QDesktopServices::openUrl (QUrl (CONTRIBUTE_LINK));
}

void Window::officialWebsite (void)
{
    QDesktopServices::openUrl (QUrl (WEBSITE_LINK));
}

void Window::updateTitle (void)
{
    // Use "Untitled" while editing new documents
    QString _title = editor()->documentTitle().isEmpty() ?
                     tr ("Untitled") :
                     shortFileName (editor()->documentTitle());

    // Add a "*" if the document was modifed
    QString _star = editor()->document()->isModified() ?  "* - " : " - ";
    setWindowTitle (_title + _star + APP_NAME);

    // Configure the behavior of the 'smart' save actions
    bool _save_enabled = ! (!m_editor->documentTitle().isEmpty() &&
                            !m_editor->document()->isModified());

    m_menu->setSaveEnabled (_save_enabled);
    m_toolbar->setSaveEnabled (_save_enabled);
}

void Window::syncSettings (void)
{
    emit updateSettings();
    emit settingsChanged();
}

void Window::saveWindowState (void)
{
    m_settings->setValue ("maximized", isMaximized());

    if (!isMaximized())
        {
        m_settings->setValue ("size", size());
        m_settings->setValue ("position", pos());
        }
}

void Window::configureWindow (Window *window)
{
    Q_ASSERT (window != NULL);

    window->saveWindowState();
    connect (window, SIGNAL (checkForUpdates()), this, SIGNAL (checkForUpdates()));

    // Sync settings across windows
    foreach (QWidget *widget, QApplication::topLevelWidgets())
        {
        if (widget->objectName() == objectName())
            {
            connect (widget, SIGNAL (settingsChanged()), window, SIGNAL (updateSettings()));
            connect (window, SIGNAL (settingsChanged()), widget, SIGNAL (updateSettings()));
            }
        }

    // Show the window normally if the current window is maximized
    if (isMaximized())
        window->showNormal();

    // Resize the window and position it to match the current window
    else
        {
        window->resize (size());
        window->move (window->x() + 30, window->y() + 30);
        m_settings->setValue ("position", QPoint (window->x(), window->y()));
        }
}

QString Window::shortFileName (const QString &file)
{
    Q_ASSERT (!file.isEmpty());
    return QFileInfo (file).fileName();
}
