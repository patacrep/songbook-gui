// Copyright (C) 2009,2010 Romain Goffe, Alexandre Dupas
//
// Songbook Creator is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Songbook Creator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//******************************************************************************

#include "dialog-new-song.hh"

CDialogNewSong::CDialogNewSong()
  : QDialog()
{
  m_title = "";
  m_artist =  "";
  m_nbColumns = 0;
  m_capo = 0;
  
  setModal(true);

  //Required fields
  //title
  QLabel* titleLabel = new QLabel(tr("Title: "));
  QLineEdit* titleEdit = new QLineEdit;

  //artist
  QLabel* artistLabel = new QLabel(tr("Artist: "));
  QLineEdit* artistEdit = new QLineEdit;
  
  //Optional fields
  //album
  QLabel* albumLabel = new QLabel(tr("Album: "));
  QLineEdit* albumEdit = new QLineEdit;

  //cover
  QLabel* coverLabel = new QLabel(tr("Cover: "));
  m_coverEdit = new QLineEdit(QString());
  QPushButton *browseCoverButton = new QPushButton(tr("Browse"));

  //nb columns
  QLabel* nbColumnsLabel = new QLabel(tr("Number of columns: "));
  QSpinBox* nbColumnsEdit = new QSpinBox;
  nbColumnsEdit->setRange(0,20);

  //capo
  QLabel* capoLabel = new QLabel(tr("Capo: "));
  QSpinBox* capoEdit      = new QSpinBox;
  capoEdit->setRange(0,20);
  
  // Action buttons
  QDialogButtonBox * buttonBox = new QDialogButtonBox;
  QPushButton * buttonAccept = new QPushButton(tr("Ok"));
  QPushButton * buttonClose = new QPushButton(tr("Close"));
  buttonAccept->setDefault(true);
  buttonBox->addButton(buttonAccept, QDialogButtonBox::ActionRole);
  buttonBox->addButton(buttonClose, QDialogButtonBox::DestructiveRole);
 
  connect( buttonAccept, SIGNAL(clicked()), this, SLOT(accept()) );
  connect( buttonClose, SIGNAL(clicked()), this, SLOT(close()) );

  QGroupBox* requiredFieldsBox = new QGroupBox(tr("Required fields"));
  QGridLayout* requiredLayout = new QGridLayout;
  requiredLayout->addWidget(titleLabel,   0,0,1,1);
  requiredLayout->addWidget(titleEdit,  0,1,1,1);
  requiredLayout->addWidget(artistLabel,  1,0,1,1);
  requiredLayout->addWidget(artistEdit, 1,1,1,1);
  requiredFieldsBox->setLayout(requiredLayout);

  QGroupBox* optionalFieldsBox = new QGroupBox(tr("Optional fields"));
  QGridLayout* optionalLayout = new QGridLayout;
  optionalLayout->addWidget(albumLabel,   0,0,1,1);
  optionalLayout->addWidget(albumEdit,  0,1,1,3);
  optionalLayout->addWidget(coverLabel,   1,0,1,1);
  optionalLayout->addWidget(m_coverEdit,  1,1,1,2);
  optionalLayout->addWidget(browseCoverButton,  1,3,1,1);
  optionalLayout->addWidget(nbColumnsLabel,   2,0,1,1);
  optionalLayout->addWidget(nbColumnsEdit,  2,1,1,1);
  optionalLayout->addWidget(capoLabel,  2,2,1,1);
  optionalLayout->addWidget(capoEdit, 2,3,1,1);
  optionalFieldsBox->setLayout(optionalLayout);
  
  QBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(requiredFieldsBox);
  layout->addWidget(optionalFieldsBox);
  layout->addWidget(buttonBox);

  setLayout(layout);

  //Connections
  connect(titleEdit,  SIGNAL(textChanged(QString)), this, SLOT(setTitle(QString)) );
  connect(artistEdit, SIGNAL(textChanged(QString)), this, SLOT(setArtist(QString)) );
  connect(nbColumnsEdit, SIGNAL(valueChanged(int)), this, SLOT(setNbColumns(int)) );
  connect(capoEdit, SIGNAL(valueChanged(int)), this, SLOT(setCapo(int)) );
  connect(albumEdit, SIGNAL(textChanged(QString)), this, SLOT(setAlbum(QString)) );
  connect(browseCoverButton, SIGNAL(clicked()), this, SLOT(browseCover()) );
  connect(m_coverEdit, SIGNAL(textChanged(QString)), this, SLOT(setCover(QString)) );

  setWindowTitle(tr("New song"));
  setMinimumWidth(450);
  show();
}
//------------------------------------------------------------------------------
QString CDialogNewSong::title() const
{
  return m_title;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setTitle(QString ATitle )
{
  m_title = ATitle;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::artist() const
{
  return m_artist;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setArtist(QString AArtist )
{
  m_artist = AArtist;
}

//------------------------------------------------------------------------------
int CDialogNewSong::nbColumns() const
{
  return m_nbColumns;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setNbColumns(int ANbColumns)
{
  m_nbColumns = ANbColumns;
}
//------------------------------------------------------------------------------
int CDialogNewSong::capo() const
{
  return m_capo;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setCapo(int ACapo)
{
  m_capo = ACapo;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::album() const
{
  return m_album;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setAlbum(QString AAlbum )
{
  m_album = AAlbum;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::cover() const
{
  return m_cover;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setCover(QString ACover )
{
  m_cover = ACover;
}
//------------------------------------------------------------------------------
void CDialogNewSong::browseCover()
{
  QString directory = QFileDialog::getOpenFileName(this, tr("Select cover image"),
						  "/home",
						  tr("Images (*.jpg)"));
 
  if (!directory.isEmpty())
      m_coverEdit->setText(directory);
}
