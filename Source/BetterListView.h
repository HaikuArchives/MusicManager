#ifndef BETTER_LIST_VIEW_H
#define BETTER_LIST_VIEW_H

// this is supposed to be removed..
#include "TrackItem.h"

#include <View.h>
#include <ListView.h>

#include <vector>
#include <list>

#include "BetterListItem.h"

class BetterListView : public BView, public BInvoker
{
	private:
		class InternalItem {
			public:
				BetterListItem *		item;
				vector<InternalItem>	sub;
				float					pos;
				int						level;
				
				bool operator < ( const InternalItem & i ) const
				{
					return *item < *i.item;
				}
				
				bool operator == ( const InternalItem & i ) const
				{
					return item == i.item;
				}
		};
		
		// data
		vector<InternalItem>	m_items;
		float					m_list_height;
		BPoint					m_view_position;
//		BScrollView	*			m_scroll_view;
		list<InternalItem>		m_selected_items;
		bool					m_supress_updates;
		bool					m_tried_to_init_drag;
		bool					m_mouse_down;
		BPoint					m_mouse_down_where;
		bool					m_was_selected;
		
		// house keeping functions
		BPoint		DrawItem( InternalItem & item, BPoint where );
		BPoint		DrawVector( vector<InternalItem> & items, BPoint where );
		
		InternalItem *	FindInternalItem( 
							BetterListItem *, 
							vector<InternalItem> * curr_vect = NULL 
						);
		InternalItem *	FindInternalItemByPosition( 
							BPoint where, 
							vector<InternalItem> * curr_vect, 
							float & curry 
						);
		InternalItem *	FindInternalSuperitem( 
							BetterListItem *, 
							vector<InternalItem> * curr_vect = NULL, 
							InternalItem * parent = NULL 
						);
		
		int32	InternalCountItemsUnder( vector<InternalItem> * curr_vect );
		
		int32	GetItemSublevel( BetterListItem * );
		
		float GetVectorHeight( vector<InternalItem> &, int level, float tot_height );
		void UpdateListHeight();
		
		void InternalSelect( InternalItem & item, bool select );
		
		void InternalEachItemUnder(
				vector<InternalItem> *,
				bool one_level_only,
				BetterListItem * (*eachfunc)(BetterListItem *, void *), 
				void * 
			);
		
	public:
		BetterListView( 
			BRect rect, 
			const char * name, 
			list_view_type type = B_SINGLE_SELECTION_LIST, 
			uint32 resizing_mode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
			uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE
		);
		virtual ~BetterListView();
		
		virtual void Draw( BRect );
		
		virtual void ScrollTo( BPoint );
		
		virtual void MouseDown( BPoint );
		virtual void MouseMoved( BPoint, uint32, const BMessage * );
		virtual void MouseUp( BPoint );
		
//		virtual void TargetedByScrollView( BScrollView * );
		
		virtual void SupressUpdates(bool);
		
		virtual void InvalidateItem( BetterListItem * );
		
		virtual void AddItem( BetterListItem * item );
		virtual void AddUnder( BetterListItem * item, BetterListItem * parent );
		
		virtual void SortItemsUnder( 
			BetterListItem * parent, 
			bool one_level_only
		);
		
		virtual BetterListItem * ItemAt( int32, BetterListItem * parent = NULL );
		virtual BetterListItem * CurrentSelection( int32 index = 0 );
		
		virtual void Collapse( BetterListItem * );
		virtual void Expand( BetterListItem * );
		
		virtual BetterListItem * Superitem( BetterListItem * );
		
		virtual void RemoveItem( BetterListItem * );
		
		virtual int32	CountItemsUnder( BetterListItem *, bool one_level_only );
		virtual void	EachItemUnder( 
							BetterListItem *, 
							bool one_level_only, 
							BetterListItem * (*eachfunc)(BetterListItem *, void *), 
							void * data
						);
		
		virtual void Deselect( int32 index );
		virtual void DeselectAll();
		
		virtual bool InitiateDrag( BPoint where, int32 index, bool was_selected );
		
		virtual void AttachedToWindow();
//		virtual void SetInvocationMessage
};

#endif
