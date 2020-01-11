#include "LinkedTextItem.h"

LinkedTextItem::LinkedTextItem()
{
	prev	= NULL;
	next	= NULL;
	length	= 0;
}


LinkedTextItem::LinkedTextItem(LinkedTextItem &newPrev,
								LinkedTextItem &newNext, int32 newLength)
{
	prev = &newPrev;
	next = &newNext;
}


LinkedTextItem::LinkedTextItem(const LinkedTextItem& other)
{
	prev=other.prev;
	next=other.next;
	length=other.length;
}


LinkedTextItem&
LinkedTextItem::operator=(const LinkedTextItem& other)
{
	prev = other.prev;
	next = other.next;
	length = other.length;
	return *this;
}


bool
LinkedTextItem::operator==(const LinkedTextItem& other) const
{
	return prev == other.prev
		&& next == other.next
		&& length == other.length;
}


bool
LinkedTextItem::operator!=(const LinkedTextItem& other) const
{
	return !(*this == other);
}


int32 LinkedTextItem::Start()
{
	if (prev== NULL)
		return 0;
	else
		return prev->End();
}


int32 LinkedTextItem::End()
{
	if (prev == NULL)
		return length;
	else
		return (prev->End()+length);
}


int32 LinkedTextItem::Index()
{
	if (prev == NULL)
		return 0;
	else
		return prev->Index()+1;
}
