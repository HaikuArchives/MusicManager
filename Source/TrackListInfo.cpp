#include "TrackListInfo.h"

TrackListInfo::TrackListInfo( BRect rect )
:	BView(
		rect,
		"list info",
		B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP,
		B_WILL_DRAW
	),
	m_num_tracks( 0 )
{
	SetViewColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
}

TrackListInfo::~TrackListInfo()
{
}

void
TrackListInfo::Draw( BRect )
{
	SetLowColor( ViewColor() );
	SetHighColor( 0,0,0 );
	
	BString str;
	str << m_num_tracks;
	
	SetFont( be_bold_font );
	DrawString( str.String(), BPoint(5,20) );
	
	SetFont( be_plain_font );
	DrawString(" tracks currently in list");
}

void
TrackListInfo::TrackAdded( const Track * )
{
	m_num_tracks++;
	Invalidate();
}

void
TrackListInfo::TrackRemoved( const Track * )
{
	m_num_tracks--;
	Invalidate();
}
