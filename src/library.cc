// Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2011, Alexandre Dupas <alexandre.dupas@gmail.com>
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
#include "library.hh"

#include <QPixmap>

#include "main-window.hh"
#include "utils/utils.hh"

CLibrary::CLibrary(CMainWindow *parent)
  : QAbstractTableModel()
  , m_parent(parent)
  , m_directory()
  , m_songs()
{
  QPixmapCache::insert("cover-missing-small", QIcon::fromTheme("image-missing", QIcon(":/icons/tango/image-missing")).pixmap(24, 24));
  QPixmapCache::insert("cover-missing-full", QIcon::fromTheme("image-missing", QIcon(":/icons/tango/image-missing")).pixmap(128, 128));
  QPixmapCache::insert("lilypond-checked", QIcon::fromTheme("audio-x-generic", QIcon(":/icons/tango/audio-x-generic")).pixmap(24,24));
  QPixmapCache::insert("french", QIcon::fromTheme("flag-fr", QIcon(":/icons/tango/scalable/places/flag-fr.svg")).pixmap(24,24));
  QPixmapCache::insert("english", QIcon::fromTheme("flag-en", QIcon(":/icons/tango/scalable/places/flag-en.svg")).pixmap(24,24));
  QPixmapCache::insert("spanish", QIcon::fromTheme("flag-es", QIcon(":/icons/tango/scalable/places/flag-es.svg")).pixmap(24,24));

  connect(this, SIGNAL(directoryChanged(const QDir&)), SLOT(update()));
}

CLibrary::~CLibrary()
{
  m_songs.clear();
}

QDir CLibrary::directory() const
{
  return m_directory;
}

void CLibrary::setDirectory(const QString &directory)
{
  setDirectory(QDir(directory));
}

void CLibrary::setDirectory(const QDir &directory)
{
  if (m_directory != directory)
    {
      m_directory = directory;
      emit(directoryChanged(m_directory));
    }
}

CMainWindow* CLibrary::parent() const
{
  return m_parent;
}

QVariant CLibrary::headerData (int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
      switch (section)
	{
	case 0:
	  return tr("Title");
	case 1:
	  return tr("Artist");
	case 2:
	  return tr("Lilypond");
	case 3:
	  return tr("Path");
	case 4:
	  return tr("Album");
	case 5:
	  return tr("Language");
	}
    }
  return QVariant();
}

QVariant CLibrary::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  switch (role)
    {
    case Qt::DisplayRole:
      switch (index.column())
	{
	case 0:
	  return data(index, TitleRole);
	case 1:
	  return data(index, ArtistRole);
	case 2:
	  return QVariant();
	case 3:
	  return data(index, PathRole);
	case 4:
	  return data(index, AlbumRole);
	}
      break;
    case TitleRole:
      return m_songs[index.row()].title;
    case ArtistRole:
      return m_songs[index.row()].artist;
    case AlbumRole:
      return m_songs[index.row()].album;
    case CoverRole:
      return QString("%1/%2.jpg")
	.arg(m_songs[index.row()].coverPath)
	.arg(m_songs[index.row()].coverName);
    case LilypondRole:
      return m_songs[index.row()].isLilypond;
    case LanguageRole:
      return m_songs[index.row()].language;
    case PathRole:
      return m_songs[index.row()].path;
    case CoverSmallRole:
      {
	QPixmap pixmap;
	QPixmapCache::find("cover-missing-small", &pixmap);
	QFileInfo file = QFileInfo(data(index, CoverRole).toString());
	if (file.exists()
	    && !QPixmapCache::find(file.baseName()+"-small", &pixmap))
	  {
	    pixmap = QPixmap::fromImage(QImage(file.filePath()).scaledToWidth(24));
	    QPixmapCache::insert(file.baseName()+"-small", pixmap);
	  }
	return pixmap;
      }
    case CoverFullRole:
      {
	QPixmap pixmap;
	QPixmapCache::find("cover-missing-full", &pixmap);
	QFileInfo file = QFileInfo(data(index, CoverRole).toString());
	if (file.exists()
	    && !QPixmapCache::find(file.baseName()+"-full", &pixmap))
	  {
	    pixmap = QPixmap::fromImage(QImage(file.filePath()).scaled(128,128));
	    QPixmapCache::insert(file.baseName()+"-full", pixmap);
	  }
	return pixmap;
      }
    }

  switch (index.column())
    {
    case 2:
      if (data(index, LilypondRole).toBool())
	{
	  QPixmap pixmap;
	  QPixmapCache::find("lilypond-checked", &pixmap);

	  if (role == Qt::DecorationRole)
	    return pixmap;

	  if (role == Qt::SizeHintRole)
	    return pixmap.size();
	  
	  if (role == Qt::ToolTipRole)
	    return tr("Lilypond music sheet");
	}
      break;
    case 4:
      if (role == Qt::DecorationRole)
	return data(index, CoverSmallRole);
      break;
    case 5:
      {
	QString language = data(index, LanguageRole).toString();
	QPixmap pixmap;
	if (QPixmapCache::find(language, &pixmap))
	  {
	    if (role == Qt::DecorationRole)
	      return pixmap;
	    
	    if (role == Qt::SizeHintRole)
	      return pixmap.size();
	    
	    if (role == Qt::ToolTipRole)
	      return language;
	    
	    if (role == Qt::DisplayRole)
	      return QString();
	  }
	else
	  {
	    if (role == Qt::DisplayRole)
	      return language;
	  }
      break;
      }
    }
  return QVariant();
}

void CLibrary::update()
{
  m_songs.clear();

  // get the path of each song in the library
  QStringList filter = QStringList() << "*.sg";
  QString path = directory().absoluteFilePath("songs/");
  QStringList paths;

  QDirIterator it(path, filter, QDir::NoFilter, QDirIterator::Subdirectories);
  while(it.hasNext())
    paths.append(it.next());

  parent()->progressBar()->show();
  parent()->progressBar()->setTextVisible(true);
  parent()->progressBar()->setRange(0, paths.size());

  addSongs(paths);

  parent()->progressBar()->setTextVisible(false);
  parent()->progressBar()->hide();
  parent()->statusBar()->showMessage(tr("Song database updated."));
}

void CLibrary::addSongs(const QStringList &paths)
{
  // run through the library songs files
  uint count = 0;
  QStringListIterator filepath(paths);
  while (filepath.hasNext())
    {
      parent()->progressBar()->setValue(++count);
      addSong(filepath.next());
    }
  reset();
  emit(wasModified());
}

QRegExp CLibrary::reTitle = QRegExp("beginsong\\{([^[\\}]+)");
QRegExp CLibrary::reArtist("by=([^[,|\\]]+)");
QRegExp CLibrary::reAlbum(",album=([^[\\]]+)");
QRegExp CLibrary::reLilypond("\\\\lilypond");
QRegExp CLibrary::reLanguage("selectlanguage\\{([^[\\}]+)");
QRegExp CLibrary::reCoverName(",cov=([^[,]+)");

bool CLibrary::parseSong(const QString &path, Song &song)
{
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      qWarning() << "CLibrary::parseSong: unable to open " << path;
      return false;
    }

  QTextStream stream (&file);
  stream.setCodec("UTF-8");
  QString fileStr = stream.readAll();
  file.close();

  song.path = path;

  reArtist.indexIn(fileStr);
  song.artist = SbUtils::latexToUtf8(reArtist.cap(1));

  reTitle.indexIn(fileStr);
  song.title = SbUtils::latexToUtf8(reTitle.cap(1));
  
  reAlbum.indexIn(fileStr);
  song.album = SbUtils::latexToUtf8(reAlbum.cap(1));

  song.isLilypond = QBool(reLilypond.indexIn(fileStr) > -1);

  reCoverName.indexIn(fileStr);
  song.coverName = reCoverName.cap(1);

  song.coverPath = QFileInfo(path).absolutePath();

  reLanguage.indexIn(fileStr);
  song.language = reLanguage.cap(1);
    
  return true;
}

void CLibrary::addSong(const QString &path)
{
  Song song;
  parseSong(path, song);
  m_songs << song;
}

void CLibrary::removeSong(const QString &path)
{
  for (int i = 0; i < m_songs.size(); ++i)
    {
      if (m_songs[i].path == path)
	m_songs.removeAt(i);
    }
  emit(wasModified());
}

void CLibrary::updateSong(const QString &path)
{
  removeSong(path);
  addSong(path);
  emit(wasModified());
}

bool CLibrary::containsSong(const QString &path)
{
  for (int i = 0; i < m_songs.size(); ++i)
    {
      if (m_songs[i].path == path)
	return true;
    }
  return false;
}

int CLibrary::rowCount(const QModelIndex &) const
{
  return m_songs.size();
}

int CLibrary::columnCount(const QModelIndex &) const
{
  return 6;
}
