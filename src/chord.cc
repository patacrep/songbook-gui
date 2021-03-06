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
#include "chord.hh"
#include "diagram-editor.hh"
#include "utils/tango-colors.hh"

#include <QPixmap>
#include <QPixmapCache>
#include <QRect>
#include <QPainter>
#include <QDebug>

const QRegExp Chord::reChordWithFret(
    "\\\\[ug]tab[\\*]?\\{([^\\}]+)\\}\\{(\\d):([^\\}]+)");
const QRegExp
    Chord::reChordWithoutFret("\\\\[ug]tab[\\*]?\\{([^\\}]+)\\}\\{([^\\}]+)");

const QColor Chord::_guitarChordColor(_TangoSkyBlue1);
const QColor Chord::_importantGuitarChordColor(_TangoSkyBlue3);
const QColor Chord::_ukuleleChordColor(_TangoPlum1);
const QColor Chord::_importantUkuleleChordColor(_TangoPlum3);

Chord::Chord(const QString &chord, QObject *parent)
    : QObject(parent), m_isValid(true), m_drawBorder(false), m_pixmap(0)
{
    fromString(chord);
}

Chord::~Chord()
{
    delete m_pixmap;
    m_pixmap = 0;
}

QString Chord::toString()
{
    QString str;
    switch (m_instrument) {
    case Guitar:
        str.append("\\gtab");
        break;
    case Ukulele:
        str.append("\\utab");
        break;
    default:
        qWarning() << tr("Chord::toString unsupported chord type");
    }

    if (isImportant())
        str.append("*");

    // the chord name such as Am
    str.append(QString("{%1}{").arg(name()));
    // the fret
    str.append(QString("%2").arg(fret()));
    // the strings such as X32010 (C chord)
    if (!fret().isEmpty())
        str.append(":");
    str.append(QString("%3}").arg(strings()));

    return str;
}

void Chord::fromString(const QString &str)
{
    QString copy(str);

    if (str.contains("gtab"))
        m_instrument = Guitar;
    else if (str.contains("utab"))
        m_instrument = Ukulele;
    else
        m_isValid = false;

    setImportant(str.contains("*"));

    if (reChordWithFret.indexIn(str) != -1) {
        setName(reChordWithFret.cap(1));
        setFret(reChordWithFret.cap(2));
        setStrings(reChordWithFret.cap(3));
    } else if (reChordWithoutFret.indexIn(copy.remove("~:")) != -1) {
        setName(reChordWithoutFret.cap(1));
        setStrings(reChordWithoutFret.cap(2));
    }

    if (name().isEmpty())
        m_isValid = false;
}

bool Chord::isValid() const { return m_isValid; }

QPixmap *Chord::toPixmap()
{
    if (m_pixmap)
        return m_pixmap;

    if (!isValid())
        return 0;

    m_pixmap = new QPixmap(100, 120);
    m_pixmap->fill(Qt::white);

    if (!QPixmapCache::find(toString(), m_pixmap)) {
        QPainter painter;
        painter.begin(m_pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);

        int cellWidth = 12, cellHeight = 12;
        int width = (strings().length() - 1) * cellWidth;
        int padding = 13;

        // draw chord name
        painter.setPen(QPen(Qt::white));
        QRect chordRect(10, padding, 70, 10 + padding);
        QPainterPath path;
        path.addRoundedRect(chordRect, 4, 4);
        painter.fillPath(path, color());
        painter.setFont(QFont("Helvetica [Cronyx]", 10, QFont::Bold));
        painter.drawText(chordRect, Qt::AlignCenter,
                         name().replace("&", QChar(0x266D)));

        // border
        if (m_drawBorder) {
            painter.setPen(QPen(color()));
            painter.setBrush(QBrush());
            QPainterPath border;
            QRect borderRect(3, padding - 5, 82, 110);
            border.addRoundedRect(borderRect, 4, 4);
            painter.drawPath(border);
        }

        // draw horizontal lines
        int max = 4;
        foreach (QChar c, strings())
            if (c.digitValue() > max)
                max = c.digitValue();

        // grid background
        int hOffset =
            (instrument() == Guitar) ? 0 : cellWidth; // offset from the left
        int vOffset = 45;                             // offset from the top
        QRect gridRect(4, vOffset, 80, cellHeight * max + padding + 5);

        painter.setPen(QPen(Qt::black));
        painter.fillRect(gridRect, QBrush(QColor(Qt::white)));

        Q_ASSERT(max < 10);
        for (int i = 0; i < max + 1; ++i) {
            painter.drawLine(
                padding + hOffset, i * cellHeight + padding + vOffset,
                width + padding + hOffset, i * cellHeight + padding + vOffset);
        }

        int height = max * cellHeight;
        // draw a vertical line for each string
        for (int i = 0; i < strings().length(); ++i) {
            painter.drawLine(
                i * cellWidth + padding + hOffset, padding + vOffset,
                i * cellWidth + padding + hOffset, height + padding + vOffset);
        }

        // draw played strings
        for (int i = 0; i < strings().length(); ++i) {
            QRect stringRect(0, 0, cellWidth - 4, cellHeight - 4);
            int value = strings()[i].digitValue();
            if (value == -1) {
                stringRect.moveTo((i * cellWidth) + cellWidth / 2.0 + 3 +
                                      hOffset,
                                  3 + vOffset);
                painter.setFont(QFont("Arial", 9));
                painter.drawText(stringRect, Qt::AlignCenter, "X");
            } else {
                stringRect.moveTo((i * cellWidth) + cellWidth / 2.0 + 3 +
                                      hOffset,
                                  value * cellHeight + 3 + vOffset);
                if (value == 0)
                    painter.drawEllipse(stringRect);
                else
                    fillEllipse(&painter, stringRect,
                                QBrush(QColor(Qt::black)));
            }
        }

        // draw fret
        QRect fretRect(padding - (cellWidth - 2) + hOffset,
                       padding + (cellHeight + vOffset) / 2.0, cellWidth - 4,
                       cellHeight + vOffset);
        painter.setFont(QFont("Arial", 9));
        painter.drawText(fretRect, Qt::AlignCenter, fret());

        painter.end();
        QPixmapCache::insert(toString(), *m_pixmap);
    }

    return m_pixmap;
}

void Chord::fillEllipse(QPainter *painter, const QRect &rect,
                        const QBrush &brush)
{
    QPainterPath path;
    path.addEllipse(rect.topLeft().x(), rect.topLeft().y(), rect.width(),
                    rect.height());
    ;
    painter->fillPath(path, brush);
}

QColor Chord::color()
{
    if (isImportant()) {
        if (instrument() == Guitar)
            return _importantGuitarChordColor;
        else if (instrument() == Ukulele)
            return _importantUkuleleChordColor;
    } else {
        if (instrument() == Guitar)
            return _guitarChordColor;
        else if (instrument() == Ukulele)
            return _ukuleleChordColor;
    }

    return QColor(Qt::white);
}

void Chord::setDrawBorder(bool value) { m_drawBorder = value; }

QString Chord::name() const { return m_name; }

void Chord::setName(const QString &str)
{
    if (m_name != str) {
        m_name = str;
        emit nameChanged();
    }
}

QString Chord::fret() const { return m_fret; }

void Chord::setFret(const QString &str)
{
    if (m_fret != str) {
        m_fret = str;
        emit fretChanged();
    }
}

QString Chord::strings() const { return m_strings; }

void Chord::setStrings(const QString &str)
{
    if (m_strings != str) {
        m_strings = str;
        emit stringsChanged();
    }
}

Chord::Instrument Chord::instrument() const { return m_instrument; }

void Chord::setInstrument(const Chord::Instrument &instru)
{
    if (m_instrument != instru) {
        m_instrument = instru;
        emit instrumentChanged();
    }
}

void Chord::switchInstrument(bool value)
{
    if (value) {
        if (instrument() == Guitar)
            setInstrument(Ukulele);
        else
            setInstrument(Guitar);
    }
}

bool Chord::isImportant() const { return m_important; }

void Chord::setImportant(bool value) { m_important = value; }
