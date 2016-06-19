#ifndef TRACK_LIST_INFO_H
#define TRACK_LIST_INFO_H

#include <View.h>

#include "Track.h"

class TrackListInfo : public BView
{
	private:
		int32		m_num_tracks;
		
	public:
		TrackListInfo( BRect );
		virtual ~TrackListInfo();

		virtual void Draw( BRect );
				
		void TrackAdded( const Track * );
		void TrackRemoved( const Track * );
};

#endif

