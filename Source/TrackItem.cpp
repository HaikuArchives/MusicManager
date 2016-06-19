#include "TrackItem.h"

#include <String.h>
#include <View.h>

#include <stdio.h>

TrackItem::TrackItem( Track * track, bool in_plain_list )
:	BetterListItem(),
	m_track( track )
{
	BString str;
	
	if ( in_plain_list )
	{
		// full info item [artist :  album [: track] : title]
		str << m_track->GetArtist() << " ; ";
		str << m_track->GetAlbum() << " ; ";
	}
	
	if ( m_track->GetTrack() )
		str << m_track->GetTrack() << " ; ";
	str << m_track->GetTitle();
	
	m_string = str;
}

TrackItem::~TrackItem()
{
	delete m_track;
}

const Track *
TrackItem::GetTrack() const
{
	return m_track;
}

bool
TrackItem::IsNode( ino_t node )
{
	return m_track->GetNode() == node;
}

void
TrackItem::DrawItem( BView * owner, BRect frame, bool complete )
{
	rgb_color background;
	rgb_color foreground;
	
	if ( IsSelected() )
	{
		background = (rgb_color){181,203,247,255};
		foreground = (rgb_color){0,0,0,255};
	} else
	{
		background = owner->ViewColor();
		foreground = (rgb_color){0,0,0,255};
	}
	
	// draw background
	owner->SetHighColor( background );
	owner->FillRect( frame );
	
	// draw text
	owner->SetLowColor( background );
	owner->SetHighColor( foreground );
	
	owner->DrawString( m_string.String(), BPoint(frame.left+4, frame.bottom-2) );
}

bool
TrackItem::operator < (const BetterListItem & _b ) const
{
	const TrackItem * b = dynamic_cast<const TrackItem*>(&_b);
	
	if ( !b )
	{
//		printf("TrackItem::operator < called with non-TrackItem\n");
		return (const void*)this < &_b;
	}
	
//	printf("TrackItem::operator < called with TrackItem\n");

	const Track * track_a = GetTrack();
	const Track * track_b = b->GetTrack();
	
	int res = 0;
	
	if ( !res )
		res = strcasecmp(track_a->GetArtist(), track_b->GetArtist());

	if ( !res )
		res = strcasecmp(track_a->GetAlbum(), track_b->GetAlbum());
	
	if ( !res )
		res = track_a->GetTrack() - track_b->GetTrack();
	
	if ( !res )
		res = strcasecmp(track_a->GetTitle(), track_b->GetTitle());
	
	return res < 0;
}
