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
#include "label.hh"

#include <QFontMetrics>

Label::Label(QWidget *parent)
    : QLabel(parent), m_elideMode(Qt::ElideRight), m_textCache()
{
    setFixedWidth(175);
}

Label::~Label() {}

Qt::TextElideMode Label::elideMode() const { return m_elideMode; }

void Label::setElideMode(Qt::TextElideMode mode) { m_elideMode = mode; }

void Label::paintEvent(QPaintEvent *event)
{
    if (m_textCache != text()) {
        m_textCache = text();
        setText(fontMetrics().elidedText(text(), m_elideMode, width()));
    }
    QLabel::paintEvent(event);
}
