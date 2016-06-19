#include <stdio.h>
#include <Looper.h>
#include <Application.h>
#include <Messenger.h>
#include <stdlib.h>

#include "LnQuery.h"
#include "TrackWindow.h"
#include "PlayList.h"

class MyLooper : public BLooper
{
	public:
		MyLooper()
		{
		};
		virtual ~MyLooper()
		{
		};
		
		virtual void MessageReceived( BMessage * msg )
		{
			switch ( msg->what )
			{
				case B_QUERY_UPDATE:
				{
					printf("match: %s\n", msg->FindString("name") );
				}	break;
				default:
					BLooper::MessageReceived(msg);
			}
		};
};

int main()
{
	srandom( system_time() );
	
	BApplication app("application/x-vnd.m_eiman.MusicManager2");
	
	PlayList * playlist = new PlayList;
	playlist->Show();
	
	TrackWindow * win = new TrackWindow;
	win->SetPlaylistMessenger( BMessenger(playlist) );
	win->Show();
	
	new LnQuery(win, "Audio:Title==*" );
	
	app.Run();
	
	return 0;
}

