// Copyright (C) 2009-2012, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2012, Alexandre Dupas <alexandre.dupas@gmail.com>
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
#include "diagram.hh"
#include "diagram-editor.hh"

#include <QPixmap>
#include <QPixmapCache>
#include <QRect>
#include <QPainter>
#include <QDebug>

QRegExp CDiagram::reChord("\\\\[ug]tab[\\*]?\\{([^\\}]+)");
QRegExp CDiagram::reFret("\\\\[ug]tab[\\*]?\\{.+\\{(\\d):");
QRegExp CDiagram::reStringsFret(":([^\\}]+)");
QRegExp CDiagram::reStringsNoFret("\\\\[ug]tab[\\*]?\\{.+\\{([^\\}]+)");

CDiagram::CDiagram(const QString & chord, QObject *parent)
  : QObject(parent)
  , m_pixmap(0)
{
  fromString(chord);
}

CDiagram::~CDiagram()
{
  delete m_pixmap;
  m_pixmap = 0;
}

QString CDiagram::toString()
{
  QString str;
  switch(m_type)
    {
    case GuitarChord:
      str.append("\\gtab");
      break;
    case UkuleleChord:
      str.append("\\utab");
      break;
    default:
      qWarning() << tr("CDiagram::toString unsupported chord type");
    }

  if (isImportant())
    str.append("*");

  //the chord name such as Am
  str.append( QString("{%1}{").arg(chord()) );
  //the fret
  str.append(QString("%2").arg(fret()));
  //the strings such as X32010 (C chord)
  if (!fret().isEmpty())
    str.append(":");
  str.append(QString("%3}").arg(strings()));

  return str;
}

void CDiagram::fromString(const QString & str)
{
  if (str.contains("gtab"))
    m_type = GuitarChord;
  else if (str.contains("utab"))
    m_type = UkuleleChord;
  else
    qWarning() << tr("CDiagram::fromString unsupported chord type");

  setImportant(str.contains("*"));

  reChord.indexIn(str);
  setChord(reChord.cap(1));

  reFret.indexIn(str);
  setFret(reFret.cap(1));

  if (fret().isEmpty())
    {
      QString copy(str);
      reStringsNoFret.indexIn(copy.replace("~:",""));
      setStrings(reStringsNoFret.cap(1));
    }
  else
    {
      reStringsFret.indexIn(str);
      setStrings(reStringsFret.cap(1));
    }
}

{

QPixmap* CDiagram::toPixmap()
{
  if (m_pixmap)
    return m_pixmap;

  if (!isValid())
    return 0;

  m_pixmap = new QPixmap(100, 120);
  m_pixmap->fill(Qt::white);

  if (!QPixmapCache::find(toString(), m_pixmap))
    {
      QPainter painter;
      painter.begin(m_pixmap);
      painter.setRenderHint(QPainter::Antialiasing, true);

      int cellWidth = 12, cellHeight = 12;
      int width = (strings().length() - 1)*cellWidth;
      int padding = 13;

      //draw chord name
      painter.setPen(QPen(Qt::white));
      QRect chordRect(10, padding, 70, 10+padding);
      QPainterPath path;
      path.addRoundedRect(chordRect, 4, 4);
      painter.setFont(QFont("Helvetica [Cronyx]", 10, QFont::Bold));
      painter.drawText(chordRect, Qt::AlignCenter, chord().replace("&", QChar(0x266D)));


      //draw horizontal lines
      int max = 4;
      foreach (QChar c, strings())
	if (c.digitValue() > max)
	  max = c.digitValue();

      // grid background
      int hOffset = (type() == GuitarChord) ? 0 : cellWidth; //offset from the left
      int vOffset = 45; //offset from the top
      QRect gridRect(4, vOffset, 80, cellHeight*max+padding+5);

      painter.setPen(QPen(Qt::black));
      painter.fillRect(gridRect, QBrush(QColor(Qt::white)));

      Q_ASSERT(max < 10);
      for (int i=0; i<max+1; ++i)
	{
	  painter.drawLine(padding+hOffset, i*cellHeight+padding+vOffset, width+padding+hOffset, i*cellHeight+padding+vOffset);
	}

      int height = max*cellHeight;
      //draw a vertical line for each string
      for (int i=0; i<strings().length(); ++i)
	{
	  painter.drawLine(i*cellWidth+padding+hOffset, padding+vOffset, i*cellWidth+padding+hOffset, height+padding+vOffset);
	}

      //draw played strings
      for (int i=0; i<strings().length(); ++i)
	{
	  QRect stringRect(0, 0, cellWidth-4, cellHeight-4);
	  int value = strings()[i].digitValue();
	  if (value == -1)
	    {
	      stringRect.moveTo( (i*cellWidth)+cellWidth/2.0 +3+hOffset, 3+vOffset );
	      painter.setFont(QFont("Arial", 9));
	      painter.drawText(stringRect, Qt::AlignCenter, "X");
	    }
	  else
	    {
	      stringRect.moveTo( (i*cellWidth)+cellWidth/2.0 +3+hOffset, value*cellHeight+3+vOffset );
	      if (value == 0)
		painter.drawEllipse(stringRect);
	      else
		fillEllipse(&painter, stringRect, QBrush(QColor(Qt::black)));
	    }
	}

      //draw fret
      QRect fretRect(padding-(cellWidth-2)+hOffset, padding+(cellHeight+vOffset)/2.0, cellWidth-4, cellHeight+vOffset);
      painter.setFont(QFont("Arial", 9));
      painter.drawText(fretRect, Qt::AlignCenter, fret());

      painter.end();
      QPixmapCache::insert(toString(), *m_pixmap);
    }

  return m_pixmap;
}

void CDiagram::fillEllipse(QPainter* painter, const QRect & rect, const QBrush & brush)
{
  QPainterPath path;
  path.addEllipse(rect.topLeft().x(), rect.topLeft().y(), rect.width(), rect.height());;
  painter->fillPath(path, brush);
}

QString CDiagram::chord() const
{
  return m_chord;
}

void CDiagram::setChord(const QString & str)
{
  m_chord = str;
}

QString CDiagram::fret() const
{
  return m_fret;
}

void CDiagram::setFret(const QString & str)
{
  m_fret = str;
}

QString CDiagram::strings() const
{
  return m_strings;
}

void CDiagram::setStrings(const QString & str)
{
  m_strings = str;
}

CDiagram::ChordType CDiagram::type() const
{
  return m_type;
}

void CDiagram::setType(const CDiagram::ChordType & type)
{
  m_type = type;
}

bool CDiagram::isImportant() const
{
  return m_important;
}

void CDiagram::setImportant(bool value)
{
  m_important = value;
}
