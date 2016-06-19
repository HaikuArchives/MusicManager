#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <Window.h>
#include <ScrollView.h>
#include <ListView.h>
#include <FilePanel.h>

#include "TrackInfoView.h"

typedef struct PlayerCheckerData {
	BMessenger		player;
	BMessenger		owner;
	BMessage		stop_message;
	// for thread use
	BMessage		msg, reply;
	float			res;
	bigtime_t 		snooze_time;
};

class PlayList : public BWindow
{
	private:
		BListView		* m_list;
		BScrollView		* m_scroller;
		BListView		* m_random_list;
		BScrollView		* m_random_scroller;
		TrackInfoView	* m_info;
		int32			m_total_length;
		
		PlayerCheckerData	m_thread_data;
		thread_id			m_thread;
		
		BFilePanel	* m_save_panel, * m_load_panel;
		
		// settings
		bool		m_playing;
		int32		m_last_index;
		
		bool CheckType( const char * );
		bool CheckType( const entry_ref * );
		
		void UpdateTotalLength();
		void PlayFile( const entry_ref * );
		
	public:
		PlayList();
		virtual ~PlayList();
		
		virtual void MessageReceived( BMessage * );
		
		void PlayTrack( const Track * );
		
		enum {
			LIST_INVOKE = 1,
			PLAYER_STOPPED,
			START_STOP,
			LOAD_REQUEST,
			LOAD_CONFIRM,
			SAVE_REQUEST,
			SAVE_CONFIRM,
			UPDATE_TOTAL_LENGTH,

			PLAY_FILE = 'Play',
			NEXT = 'Next'
		};
};

#endif
