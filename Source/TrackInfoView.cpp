#include "TrackInfoView.h"

#include <stdio.h>

TrackInfoView::TrackInfoView( BRect rect )
:	BView( rect, "info", B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP, B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE ),
	m_track( NULL )
{
	SetViewColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
}

TrackInfoView::~TrackInfoView()
{
	if ( m_track )
		delete m_track;
}

void
TrackInfoView::Draw( BRect rect )
{
	SetLowColor( ViewColor() );
	SetHighColor(0,0,0);
	
	// static line
	SetFont( be_plain_font );
	float static_length = StringWidth( m_static.String() );
	DrawString( m_static.String(), BPoint( Bounds().right-static_length-5, 20) );
	
	if ( !m_track )
		return;
	
	BString str;
	str << m_track->GetTrack() << ", ";
	
	// first line
	SetFont( be_bold_font );
	DrawString( m_track->GetArtist(), BPoint(5,20) );
	DrawString( ", " );
	
	DrawString( m_track->GetAlbum() );
	
	// second line
	MovePenTo( BPoint(5,40) );
	
	if ( m_track->GetTrack() )
	{ // track no
		SetFont( be_plain_font );
		DrawString( "# " );
		
		SetFont( be_bold_font );
		DrawString( str.String() );
	}
	
	DrawString( m_track->GetTitle() );
	
	if ( m_track->GetLength() > 0 )
	{
		SetFont( be_plain_font );
		
		char text[64];
		sprintf(text, ", %ld:%02ld",
			m_track->GetLength()/60,
			m_track->GetLength()%60
		);
		DrawString(text);
	}
}

void
TrackInfoView::SetTrack( const Track * track )
{
	if ( m_track )
		delete m_track;
	
	if ( track )
	{
		m_track = new Track(*track);
	} else
	{
		m_track = NULL;
	}
	
	Invalidate();
}

void
TrackInfoView::SetStaticInfo( const char * info )
{
	m_static = info;
	Invalidate();
}
