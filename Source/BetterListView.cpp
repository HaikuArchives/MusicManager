#include "BetterListView.h"

#include <stack>
#include <algorithm>
#include <ScrollView.h>
#include <Window.h>

#include <stdio.h>

#define SUBLEVEL_OFFSET 10

BetterListView::BetterListView(
	BRect rect, 
	const char * name, 
	list_view_type type, 
	uint32 resizing_mode,
	uint32 flags
	)
: BView( rect, name, resizing_mode, flags ),
	m_list_height(0.0),
	m_view_position(0,0),
	m_supress_updates(false),
	m_tried_to_init_drag( false ),
	m_mouse_down( false )
{
	SetViewColor( 255,255,255, 255 );
}

BetterListView::~BetterListView()
{
}

BPoint
BetterListView::DrawItem( InternalItem & item, BPoint where )
{
	BRect item_rect(
		where.x,where.y,
		Bounds().right-2,where.y+item.item->Height()
	);
	
	item.item->DrawItem(this,item_rect);
	
	if ( item.sub.empty() == false )
	{ // draw expander thingie
		const char * thingie = "+";
		if ( item.item->IsExpanded() )
			thingie = "-";
		DrawString(thingie,BPoint(item_rect.left-7,item_rect.bottom-2));
	}
	
	return BPoint(item_rect.left,item_rect.bottom+1);
}


BPoint
BetterListView::DrawVector( vector<InternalItem> & items, BPoint where )
{
	vector<InternalItem>::iterator iter = items.begin();
	
	while ( iter != items.end() && where.y < Bounds().bottom )
	{
		where = DrawItem( *iter, where );
		
		if ( (*iter).item->IsExpanded() && (*iter).sub.empty() != true )
		{ // has sub items
			where = DrawVector( (*iter).sub, BPoint( where.x+SUBLEVEL_OFFSET,where.y ) );
			where.x -= SUBLEVEL_OFFSET;
		}
		
		iter++;
	}
	
	return where;
}

void
BetterListView::Draw( BRect update_rect )
{
	font_height fh;
	be_plain_font->GetHeight(&fh);
	
//	float y = Bounds().top - m_list_height * (ScrollBar(B_VERTICAL)->Value() / m_list_height);
	
//	printf("Draw, view position: %.2f, bar value: %.2f\n", y, ScrollBar(B_VERTICAL)->Value() );
	
	DrawVector( m_items, BPoint(Bounds().left+SUBLEVEL_OFFSET, -m_view_position.y) );	
}

void
BetterListView::AddItem( BetterListItem * item )
{
	AddUnder( item, NULL );
}

void
BetterListView::AddUnder( BetterListItem * item, BetterListItem * parent )
{
	InternalItem new_item;
	new_item.item = item;

	if ( parent == NULL )
	{
		m_items.push_back( new_item );
	} else {
		InternalItem * i = FindInternalItem( parent );
		
		if ( i )
		{
			i->sub.push_back( new_item );
		} else {
			// trying to add to non-existing item, error
			return;
		}
	}
	
	item->SetExpanded( false );
	item->Deselect();
	
	item->Update( this, be_plain_font );
	
	if ( !m_supress_updates )
	{
		UpdateListHeight();
	
		Invalidate();
	}
}

void
BetterListView::SortItemsUnder( 
	BetterListItem * parent, 
	bool one_level_only
)
{
	vector<InternalItem> * target;
	
	// find target vector
	if ( parent == NULL )
	{
		target = &m_items;
	} else {
		InternalItem * item = FindInternalItem( parent );
		if ( item )
		{
			target = &item->sub;
		} else {
			// parent item not found, exit
			return;
		}
	}
	
	// sort!
	sort( target->begin(), target->end() );
	
	// sort sub-vectors if needed
	if ( one_level_only == false )
	{
		vector<InternalItem>::iterator i = target->begin();
		
		while ( i != target->end() )
		{
			SortItemsUnder( (*i).item, false );
			
			i++;
		}
	}
	
	// redraw
	if ( !m_supress_updates )
		Invalidate();
}

BetterListView::InternalItem *
BetterListView::FindInternalItem( BetterListItem * item, vector<InternalItem> * curr_vect )
{
	if ( curr_vect == NULL )
	{
		curr_vect = &m_items;
	}
	
	vector<InternalItem>::iterator i = curr_vect->begin();
	
	while ( i != curr_vect->end() )
	{
		if ( (*i).item == item )
			return &(*i);
		
		if ( (*i).sub.empty() != true )
		{ // sub-vector present, search it
			InternalItem * result = FindInternalItem(item,&(*i).sub);
			
			if ( result )
				return result;
		}
		
		i++;
	}
	
	return NULL;
}

BetterListItem *
BetterListView::ItemAt( int32 index, BetterListItem * parent )
{
	vector<InternalItem> * target_sub;
	
	if ( parent == NULL )
	{
		target_sub = &m_items;
	} else {
		InternalItem * parent_item = FindInternalItem(parent);
		
		if ( parent_item )
		{
			target_sub = &parent_item->sub;
		} else {
			// no such parent
			return NULL; 
		}
	}
	
	if ( index < (int32)target_sub->size() )
		return (*target_sub)[index].item;
	else
		return NULL;
}

BetterListItem *
BetterListView::CurrentSelection( int32 index )
{
	if ( (index >= 0) && (index < (int32)m_selected_items.size()) )
	{
		list<InternalItem>::iterator i = m_selected_items.begin();
		
		for ( int a=0; a<index; a++ )
			i++;
		
		return (*i).item;
	}
	
	return NULL;
}

void
BetterListView::Collapse( BetterListItem * item )
{
	InternalItem * internal = FindInternalItem( item );
	
	if ( internal && internal->sub.size() > 0 )
	{
		item->SetExpanded(false);
		UpdateListHeight();
		Invalidate();
	}
}

void
BetterListView::Expand( BetterListItem * item )
{
	InternalItem * internal = FindInternalItem( item );
	
	if ( internal && internal->sub.size() > 0 )
	{
		item->SetExpanded(true);
		UpdateListHeight();
		Invalidate();
	}
}

void
BetterListView::RemoveItem( BetterListItem * )
{
}

int32
BetterListView::InternalCountItemsUnder( vector<InternalItem> * curr_vect )
{
	vector<InternalItem>::iterator i = curr_vect->begin();
	
	int32 count = 0;
	
	while ( i != curr_vect->end() )
	{
		count++;
		
		if ( !(*i).sub.empty() )
			count += InternalCountItemsUnder( &(*i).sub );
		
		i++;
	}
	
	return count;
}

int32
BetterListView::CountItemsUnder( BetterListItem * target, bool one_level_only )
{
	InternalItem * internal = FindInternalItem(target);
	
	if ( one_level_only )
	{
		return internal->sub.size();
	} else 
	{
		return InternalCountItemsUnder( &internal->sub );
	}
	
	return 0;
}

BetterListView::InternalItem *
BetterListView::FindInternalSuperitem( BetterListItem * target, vector<InternalItem> * curr_vect, InternalItem * parent )
{
	if ( !curr_vect )
	{
		curr_vect = &m_items;
	}
	
	vector<InternalItem>::iterator i = curr_vect->begin();
	
	while ( i != curr_vect->end() )
	{
		if ( (*i).item == target )
			return parent;
		
		if ( !(*i).sub.empty() )
		{
			InternalItem * result = FindInternalSuperitem(target,&(*i).sub,&(*i));
			if ( result )
				return result;
		}
		
		i++;
	}
	
	return NULL;
}

BetterListItem *
BetterListView::Superitem( BetterListItem * target )
{
	InternalItem * item = FindInternalSuperitem(target);
	
	if ( item )
		return item->item;
	else
		return NULL;
}

float
BetterListView::GetVectorHeight( vector<InternalItem> & v, int level, float tot_height )
{
	vector<InternalItem>::iterator i = v.begin();;
	
	float height = 0.0f;
	
	while ( i != v.end() )
	{
		// update InternalItem values
		(*i).pos = height + tot_height;
		(*i).level = level;
		
		// update get height etc
		height += (*i).item->Height()+1;
		if ( (*i).item->IsExpanded() )
			height += GetVectorHeight( (*i).sub, level+1, height+tot_height );
		i++;
	}
	
	return height;
}

void
BetterListView::UpdateListHeight()
{
	m_list_height = GetVectorHeight( m_items, 0, 0.0f );
	
	BScrollBar * vbar = ScrollBar( B_VERTICAL );
	
	if ( vbar )
	{
		vbar->SetRange( 0.0, m_list_height-Bounds().Height() );
//		float h = m_list_height - Bounds().Height();
		
		if ( m_list_height > 0 )
			vbar->SetProportion( Bounds().Height() / m_list_height );
		
		if ( m_items.size() > 0 )
		{
			vbar->SetSteps( 
				m_items[0].item->Height(), 
				(m_items[0].item->Height())*10
			);
		}
	}
}

void
BetterListView::ScrollTo( BPoint p )
{
	printf("ScrollTo:\n");
	p.PrintToStream();
	
	bool inv = false;
	
	p.y = ScrollBar(B_VERTICAL)->Value();
	
	// ful-fix
	if ( p.y < 30 )
	{
//		printf("Setting y to 0, was %.2f\n", p.y);
		p.y = 0;
		
		inv = true;
	}
	
	float diff = p.y - m_view_position.y;
	
	m_view_position = p;
	
	CopyBits( Bounds().OffsetByCopy(0,diff), Bounds() );
	
	if ( inv )
	{
		BRect r = Bounds();
		r.bottom = 70;
		
		Invalidate(r);
	}
}

int32
BetterListView::GetItemSublevel( BetterListItem * item )
{
	int32 level = 0;
	
	while ( item )
	{
		item = Superitem(item);
		level++;
	}
	
	return level;
}

void
BetterListView::MouseDown( BPoint where )
{
	m_was_selected = false;
	
	float curry=0.0f;
	
	where.y += m_view_position.y;
	
	InternalItem * item = FindInternalItemByPosition(where,&m_items,curry);
	
	if ( item )
	{
//		InternalSelect( *item, !item->item->IsSelected() );
		int32 level = GetItemSublevel(item->item);
		
		if ( where.x < (level+1) * SUBLEVEL_OFFSET )
		{ // expand/collapse
			if ( item->item->IsExpanded() )
				Collapse( item->item );
			else
				Expand( item->item );
		} else
		{ // select/deselect
			int32 clicks = 0;
			
			Window()->CurrentMessage()->FindInt32("clicks",&clicks);
			
			if ( clicks > 1 )
			{ // double-click
				Invoke();
			} else 
			{
				uint32 mod = modifiers();
				
				if ( mod & B_SHIFT_KEY )
				{ // multiple select
					if ( !item->item->IsSelected() )
					{
						// select on mouse-down, deselect on mouse-up
						// (if not m_was_selected, that is)
						InternalSelect( *item, true );
						m_was_selected = true;
					}
				} else 
				{ // single select
					DeselectAll();
					
					InternalSelect( *item, true );
				}
			}
		}
	}
	
	where.y -= m_view_position.y;
	
	m_tried_to_init_drag = false;
	m_mouse_down = true;
	m_mouse_down_where = where;
}

void
BetterListView::MouseMoved( BPoint where, uint32 transit, const BMessage * drag_msg )
{
	if ( m_mouse_down && !drag_msg && !m_tried_to_init_drag )
	{
		BPoint moved = where - m_mouse_down_where;
		
		if ( abs((int)moved.x) > 3 || abs((int)moved.y) > 3 )
		{
			m_tried_to_init_drag = true;
		
			if ( InitiateDrag( where, -1, false ) )
			{ // why do we care if a drag is initiade?
			}
		}
	}
}

void
BetterListView::MouseUp( BPoint where )
{
	float curry=0.0f;
	
	where.y += m_view_position.y;
	
	InternalItem * item = FindInternalItemByPosition(where,&m_items,curry);
	
	if ( item && !m_was_selected )
	{
//		InternalSelect( *item, !item->item->IsSelected() );
		int32 level = GetItemSublevel(item->item);
		
		if ( where.x >= (level+1) * SUBLEVEL_OFFSET )
		{ // select/deselect
			if ( modifiers() & B_SHIFT_KEY )
			{ // Multiple selection mode
				if ( item->item->IsSelected() )
				{
					// de-select here to allow dragging of an item without having to 
					// click i twice
					InternalSelect( *item, false );
				}
			}
		}
	}
	
	m_mouse_down = false;
}

BetterListView::InternalItem *
BetterListView::FindInternalItemByPosition( 
	BPoint where, 
	vector<InternalItem> * curr_vect, 
	float & curry
	)
{
	vector<InternalItem>::iterator i = curr_vect->begin();
	
	while ( i != curr_vect->end() )
	{
		curry += (*i).item->Height()+1;

		if ( curry > where.y )
		{
			return &(*i);
		}
		
		if ( (*i).item->IsExpanded() )
		{
			InternalItem * result = FindInternalItemByPosition(where,&((*i).sub),curry);
			if ( result )
				return result;
		}
		
		i++;
	}
	
	return NULL;
}

void
BetterListView::InternalSelect( InternalItem & item, bool select )
{
	if ( select )
	{
		item.item->Select();
		m_selected_items.push_back( item );
	} else {
		item.item->Deselect();
		m_selected_items.erase( find(m_selected_items.begin(),m_selected_items.end(),item) );
	}
	InvalidateItem(item.item);
}

void
BetterListView::SupressUpdates( bool s )
{
	m_supress_updates = s;
	
	if ( !s )
	{
		UpdateListHeight();
		Invalidate();
	}
}

void
BetterListView::Deselect( int32 index )
{
	list<InternalItem>::iterator i = m_selected_items.begin();
	
	int32 curr = 0;
	
	while ( i != m_selected_items.end() )
	{
		if ( curr == index )
		{
			(*i).item->Deselect();
		
			m_selected_items.erase(i);
			
			InvalidateItem( (*i).item );
			
			return;
		}
		i++;
		curr++;
	}
}

void
BetterListView::DeselectAll()
{
	while ( !m_selected_items.empty() )
		Deselect(0);
}

void
BetterListView::InternalEachItemUnder(
					vector<InternalItem> * curr_vect, 
					bool one_level_only, 
					BetterListItem * (*eachfunc)(BetterListItem *, void *), 
					void * data
				)
{
	vector<InternalItem>::iterator i = curr_vect->begin();
	
	while ( i != curr_vect->end() )
	{
		eachfunc( (*i).item, data );
		
		if ( !one_level_only && !(*i).sub.empty() )
		{
			InternalEachItemUnder( &(*i).sub, false, eachfunc, data );
		}
		
		i++;
	}
}

void
BetterListView::EachItemUnder(
					BetterListItem * item, 
					bool one_level_only, 
					BetterListItem * (*eachfunc)(BetterListItem *, void *), 
					void * data
				)
{
	InternalItem * internal = FindInternalItem( item );
	
	InternalEachItemUnder( &internal->sub, one_level_only, eachfunc, data );
}


bool
BetterListView::InitiateDrag( BPoint, int32, bool )
{
	return false;
}

void
BetterListView::AttachedToWindow()
{
	SetTarget( Parent() );
}

void
BetterListView::InvalidateItem( BetterListItem * item )
{
	InternalItem * internal = FindInternalItem( item );
	
	Invalidate( 
		BRect( 
			0, internal->pos - m_view_position.y, 
			Bounds().right, internal->pos + item->Height() - m_view_position.y
		)
	);
}
