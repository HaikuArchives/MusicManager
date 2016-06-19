#ifndef LN_QUERY_H
#define LN_QUERY_H

#include <Messenger.h>
#include <Looper.h>
#include <Query.h>
#include <Volume.h>

class VolumeQuery : public BLooper
{
	protected:
		friend int32 volume_query_thread( void * );
		
		BMessenger		m_target;
		BVolume			m_volume;
		BQuery			m_query;
		thread_id		m_thread;
		char			m_volume_name[512];
		
	public:
		VolumeQuery( BMessenger, BVolume, const char * );
		virtual ~VolumeQuery();
		
		virtual void MessageReceived( BMessage * );
		
		enum {
			BEGIN_FETCH = 'vqBF',
			END_FETCH = 'vqEF'
		};
};

class LnQuery : public BLooper
{
	protected:
		BMessenger		m_target;
		
	public:
		LnQuery( BMessenger, const char * predicate );
		virtual ~LnQuery();

		virtual void MessageReceived( BMessage * );
};

#endif
