/***************************************************************************
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2007 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "searchlineedit.h"

// local includes
#include "animatedwidget.h"
#include "core/document.h"

// qt/kde includes
#include <qapplication.h>
#include <qlayout.h>
#include <qtimer.h>
#include <kcolorscheme.h>

SearchLineEdit::SearchLineEdit( QWidget * parent, Okular::Document * document )
    : KLineEdit( parent ), m_document( document ), m_minLength( 0 ),
      m_caseSensitivity( Qt::CaseInsensitive ),
      m_searchType( Okular::Document::AllDocument ), m_id( -1 ),
      m_moveViewport( false ), m_changed( false ), m_fromStart( true ),
      m_searchRunning( false )
{
    setObjectName( "SearchLineEdit" );
    setClearButtonShown( true );

    // a timer to ensure that we don't flood the document with requests to search
    m_inputDelayTimer = new QTimer(this);
    m_inputDelayTimer->setSingleShot(true);
    connect( m_inputDelayTimer, SIGNAL( timeout() ),
             this, SLOT( startSearch() ) );

    connect(this, SIGNAL( textChanged(const QString &) ), this, SLOT( slotTextChanged(const QString &) ));
    connect(this, SIGNAL( returnPressed(const QString &) ), this, SLOT( slotReturnPressed(const QString &) ));
    connect(document, SIGNAL( searchFinished(int, Okular::Document::SearchStatus) ), this, SLOT( searchFinished(int, Okular::Document::SearchStatus) ));
}

void SearchLineEdit::clearText()
{
    clear();
}

void SearchLineEdit::setSearchCaseSensitivity( Qt::CaseSensitivity cs )
{
    m_caseSensitivity = cs;
    m_changed = true;
}

void SearchLineEdit::setSearchMinimumLength( int length )
{
    m_minLength = length;
    m_changed = true;
}

void SearchLineEdit::setSearchType( Okular::Document::SearchType type )
{
    if ( type == m_searchType )
        return;

    m_searchType = type;
    m_changed = ( m_searchType != Okular::Document::NextMatch && m_searchType != Okular::Document::PreviousMatch );
}

void SearchLineEdit::setSearchId( int id )
{
    m_id = id;
    m_changed = true;
}

void SearchLineEdit::setSearchColor( const QColor &color )
{
    m_color = color;
    m_changed = true;
}

void SearchLineEdit::setSearchMoveViewport( bool move )
{
    m_moveViewport = move;
}

void SearchLineEdit::setSearchFromStart( bool fromStart )
{
    m_fromStart = fromStart;
}

bool SearchLineEdit::isSearchRunning() const
{
    return m_searchRunning;
}

void SearchLineEdit::restartSearch()
{
    m_inputDelayTimer->stop();
    m_inputDelayTimer->start( 700 );
    m_changed = true;
}

void SearchLineEdit::stopSearch()
{
    if ( m_id == -1 || !m_searchRunning )
        return;

    m_inputDelayTimer->stop();
    // ### this should just cancel the search with id m_id, not all of them
    m_document->cancelSearch();
    // flagging as "changed" so the search will be reset at the next one
    m_changed = true;
}

void SearchLineEdit::findNext()
{
    if ( m_id == -1 || m_searchType != Okular::Document::NextMatch )
        return;

    if ( !m_changed )
    {
        emit searchStarted();
        m_searchRunning = true;
        m_document->continueSearch( m_id, m_searchType );
    }
    else
        startSearch();
}

void SearchLineEdit::findPrev()
{
    if ( m_id == -1 || m_searchType != Okular::Document::PreviousMatch )
        return;

    if ( !m_changed )
    {
        emit searchStarted();
        m_searchRunning = true;
        m_document->continueSearch( m_id, m_searchType );
    }
    else
        startSearch();
}

void SearchLineEdit::slotTextChanged( const QString & text )
{
    prepareLineEditForSearch();
    restartSearch();
}

void SearchLineEdit::prepareLineEditForSearch()
{
    QPalette pal = palette();
    const int textLength = text().length();
    if ( textLength > 0 && textLength < m_minLength )
    {
        const KColorScheme scheme( QPalette::Active, KColorScheme::View );
        pal.setBrush( QPalette::Base, scheme.background( KColorScheme::NegativeBackground ) );
        pal.setBrush( QPalette::Text, scheme.foreground( KColorScheme::NegativeText ) );
    }
    else
    {
        const QPalette qAppPalette = QApplication::palette();
        pal.setColor( QPalette::Base, qAppPalette.color( QPalette::Base ) );
        pal.setColor( QPalette::Text, qAppPalette.color( QPalette::Text ) );
    }
    setPalette( pal );
}

void SearchLineEdit::slotReturnPressed( const QString &text )
{
    m_inputDelayTimer->stop();
    prepareLineEditForSearch();
    findNext();
}

void SearchLineEdit::startSearch()
{
    if ( m_id == -1 || !m_color.isValid() )
        return;

    if ( m_changed && ( m_searchType == Okular::Document::NextMatch || m_searchType == Okular::Document::PreviousMatch ) )
    {
        m_document->resetSearch( m_id );
    }
    m_changed = false;
    // search text if have more than 3 chars or else clear search
    QString thistext = text();
    if ( thistext.length() >= qMax( m_minLength, 1 ) )
    {
        emit searchStarted();
        m_searchRunning = true;
        m_document->searchText( m_id, thistext, m_fromStart, m_caseSensitivity,
                                m_searchType, m_moveViewport, m_color );
    }
    else
        m_document->resetSearch( m_id );
}

void SearchLineEdit::searchFinished( int id, Okular::Document::SearchStatus endStatus )
{
    // ignore the searches not started by this search edit
    if ( id != m_id )
        return;

    // if not found, use warning colors
    if ( endStatus == Okular::Document::NoMatchFound )
    {
        QPalette pal = palette();
        const KColorScheme scheme( QPalette::Active, KColorScheme::View );
        pal.setBrush( QPalette::Base, scheme.background( KColorScheme::NegativeBackground ) );
        pal.setBrush( QPalette::Text, scheme.foreground( KColorScheme::NegativeText ) );
        setPalette( pal );
    }
    else
    {
        QPalette pal = palette();
        const QPalette qAppPalette = QApplication::palette();
        pal.setColor( QPalette::Base, qAppPalette.color( QPalette::Base ) );
        pal.setColor( QPalette::Text, qAppPalette.color( QPalette::Text ) );
        setPalette( pal );
    }

    m_searchRunning = false;
    emit searchStopped();
}


SearchLineWidget::SearchLineWidget( QWidget * parent, Okular::Document * document )
    : QWidget( parent )
{
    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin( 0 );

    m_edit = new SearchLineEdit( this, document );
    layout->addWidget( m_edit );

    m_anim = new AnimatedWidget( "process-working", this );
    m_anim->setFixedSize( 22, 22 );
    layout->addWidget( m_anim );

    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    connect( m_timer, SIGNAL( timeout() ), m_anim, SLOT( start() ) );

    connect( m_edit, SIGNAL( searchStarted() ), this, SLOT( slotSearchStarted() ) );
    connect( m_edit, SIGNAL( searchStopped() ), this, SLOT( slotSearchStopped() ) );
}

SearchLineEdit* SearchLineWidget::lineEdit() const
{
    return m_edit;
}

void SearchLineWidget::slotSearchStarted()
{
    m_timer->start( 100 );
}

void SearchLineWidget::slotSearchStopped()
{
    m_timer->stop();
    m_anim->stop();
}

#include "searchlineedit.moc"