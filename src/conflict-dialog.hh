// Copyright (C) 2009-2013, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2013, Alexandre Dupas <alexandre.dupas@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
//******************************************************************************

#ifndef __CONFLICT_DIALOG_HH
#define __CONFLICT_DIALOG_HH

#include "main-window.hh"
#include "progress-bar.hh"

#include <QDialog>
#include <QString>
#include <QMap>
#include <QStatusBar>

class QLabel;
class QTableWidget;
class QTableWidgetItem;
class QPushButton;
class QPixmap;

class ProgressBar;
class FileCopier;

/*!
  \file conflict-dialog.hh
  \class ConflictDialog
  \brief ConflictDialog is a dialog to solve conflicts when importing already
  existing songs.

  A conflict dialog displays side-by-side two list of songs
  \li source: the list of conflictig songs that are being imported
  \li target: the list of already existing songs from the current library

  The dialog then presents actions to:
  \li overwrite (replace target with source)
  \li preserve  (keep target and ignore source)
  \li show differences (a diff view that relies on the <a
  href="http://code.google.com/p/google-diff-match-patch/">diff_match_patch</a>
  library)

  \image html conflict-dialog.png

*/
class ConflictDialog : public QDialog
{
    Q_OBJECT
    Q_ENUMS(ConflictSolveMode)

public:
    /// Constructor.
    ConflictDialog(QWidget *parent = 0);

    /// Destructor.
    virtual ~ConflictDialog();

    /*!
    Define \a map as the double list of existing / to be imported files
    Key: path to source file (new song)
    Value: path to target file (existing song)
  */
    void setSourceTargetFiles(const QMap<QString, QString> &map);

    /*!
    Determines whether there are some conflicts between
    source and target files. Returns \a true if
    there are some conflicting names, \a false otherwise.
  */
    bool conflictsFound() const;

    /*!
    Returns the parent window of this dialog.
  */
    MainWindow *parent() const;

    /*!
    Sets \a parent as the parent window of this dialog.
  */
    void setParent(MainWindow *parent);

    /*!
    Display an informative \a message in the status
    bar of the parent window.
  */
    void showMessage(const QString &message);

    /*!
    Returns the progress bar of the parent window.
  */
    ProgressBar *progressBar() const;

public slots:
    /*!
    Resolves existing conflicts according to
    the desired action (overwriting / preserving)
  */
    bool resolve();

    /*!
    Displays differences between conflicting
    source and target files in a new dialog

    \image html conflict-diff.png

  */
    void showDiff();

private slots:
    void updateItemDetails(QTableWidgetItem *item);
    void openItem(QTableWidgetItem *item);
    void cancelCopy();

private:
    MainWindow *m_parent;
    bool m_conflictsFound;
    QMap<QString, QString> m_conflicts;
    QMap<QString, QString> m_noConflicts;

    QLabel *m_mainLabel;

    QTableWidget *m_conflictView;

    QLabel *m_titleLabel;
    QLabel *m_artistLabel;
    QLabel *m_albumLabel;
    QLabel *m_pathLabel;
    QLabel *m_coverLabel;
    QPixmap *m_pixmap;

    QPushButton *m_overwriteButton;
    QPushButton *m_keepOriginalButton;
    QPushButton *m_diffButton;

    FileCopier *m_fileCopier;
};

/*!
  \file conflict-dialog.hh
  \class FileCopier
  \brief FileCopier manages the copy of potentially conflicting files
*/
class FileCopier : public QObject
{
    Q_OBJECT

public:
    /// Constructor
    FileCopier(QWidget *parent) : m_cancelCopy(false)
    {
        setParent(static_cast<MainWindow *>(parent));
    }

    /// Define \a files as the list of source / target files to be copied
    void setSourceTargets(QMap<QString, QString> &files)
    {
        m_sourceTargets = files;
    }

    /// Returns \a true if the copy can be interrupted
    bool cancelCopy() const { return m_cancelCopy; }

    /// Returns the parent widget
    MainWindow *parent() const { return m_parent; }

    /// Defines the parent widget
    void setParent(MainWindow *parent) { m_parent = parent; }

    /// If value is \a true, the copy can be interrupted
    void setCancelCopy(bool value) { m_cancelCopy = value; }

public slots:

    /*!
    Performs the copy operation of sources to targets
    \sa setSourceTargets
  */
    void copy()
    {
        int count = 0;
        ProgressBar *progressBar = parent()->progressBar();
        progressBar->setRange(0, m_sourceTargets.size());
        progressBar->show();

        QMap<QString, QString>::const_iterator it =
            m_sourceTargets.constBegin();
        while (it != m_sourceTargets.constEnd()) {
            if (cancelCopy())
                break;

            QFile source(it.key());
            QFile target(it.value());
            if (!target.exists() && !source.copy(target.fileName())) {
                parent()->statusBar()->showMessage(
                    tr("An unexpected error occurred while copying: %1 to %2")
                        .arg(source.fileName())
                        .arg(target.fileName()));
            }
            progressBar->setValue(++count);
            ++it;
        }
        progressBar->hide();
    }

private:
    MainWindow *m_parent;
    bool m_cancelCopy;
    QMap<QString, QString> m_sourceTargets;
};

#endif // __CONFLICT_DIALOG_HH
