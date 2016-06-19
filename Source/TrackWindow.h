#ifndef TRACK_WINDOW_H
#define TRACK_WINDOW_H

#include <Window.h>
#include <ScrollView.h>
#include <List.h>

#include <map>
#include <string>

#include "TrackItem.h"
#include "TrackListInfo.h"
#include "BetterListView.h"

class Artist
{
	public:
		string						name;
		map<string,BetterListItem*>	albums;
		BetterListItem *			listitem;
};

class TrackWindow : public BWindow
{
	private:
		BetterListView		* m_list;
		BScrollView			* m_scroller;
		TrackListInfo		* m_info;
		BMessenger			m_playlist_messenger;
		
		enum {
			LIST_INVOKE = 1
		};
		
		int		m_fetching;
		BList	m_fetched_items;
		
		// artist ( map<string, map<string, BListItem*> > )
		//   -> album ( map<string, BListItem*> )
		//        -> track
		map<string, Artist*>	m_artist_map;
		
		void AddTrack( TrackItem *, bool should_sort=true );
		void SortTracks();
		
	public:
		TrackWindow();
		virtual ~TrackWindow();
		
		virtual void MessageReceived( BMessage * );
		
		void SetPlaylistMessenger( BMessenger );
};

#endif

