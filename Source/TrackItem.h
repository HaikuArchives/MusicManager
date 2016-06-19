#ifndef TRACK_ITEM_H
#define TRACK_ITEM_H

#include <String.h>

#include "BetterListItem.h"

#include "Track.h"

class TrackItem : public BetterListItem
{
	private:
		friend int compare_track_items( const void *, const void * );
		
		Track		* m_track;
		BString		m_string;
		
	public:
		TrackItem( Track *, bool in_plain_list = true );
		virtual ~TrackItem();
		
		const Track *	GetTrack() const;
		
		bool IsNode( ino_t );
		
		virtual void DrawItem( BView *, BRect, bool );
		
		virtual bool operator < ( const BetterListItem &) const;
};

#endif
