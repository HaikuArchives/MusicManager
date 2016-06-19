#ifndef TRACK_INFO_VIEW_H
#define TRACK_INFO_VIEW_H

#include <View.h>

#include "Track.h"

class TrackInfoView : public BView
{
	private:
		Track		* m_track;
		BString		m_static;
		
	public:
		TrackInfoView( BRect );
		virtual ~TrackInfoView();
		
		virtual void Draw( BRect );
		
		void SetTrack( const Track * );
		void SetStaticInfo( const char * );
};

#endif
