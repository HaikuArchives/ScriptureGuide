#include <stdio.h>
#include "BStringList.h"

BStringList::BStringList(int32 itemsPerBlock)
 : fList(itemsPerBlock)
{
}

BStringList::~BStringList()
{
	MakeEmpty(true);
}

bool BStringList::AddItem(const char *string)
{
	if(!string)
		return false;
	
	return fList.AddItem(new BString(string));
}

bool BStringList::AddItem(const char *string, int32 atIndex)
{
	if(!string)
		return false;
	
	return fList.AddItem(new BString(string),atIndex);
}

bool BStringList::AddList(BStringList *newItems)
{
	if(!newItems)
		return false;

	for(int32 i=0; i<newItems->CountItems(); i++)
		fList.AddItem(newItems->ItemAtFast(i));
	return true;
}

bool BStringList::AddList(BStringList *newItems, int32 atIndex)
{
	if(!newItems)
		return false;

	for(int32 i=0; i<newItems->CountItems(); i++)
		fList.AddItem(newItems->ItemAtFast(i),atIndex);
	return true;
}

bool BStringList::RemoveItem(BString *item)
{
	return fList.RemoveItem(item);
}

bool BStringList::RemoveItem(const char *string)
{
	BString *item=FindItem(string);
	if(!item)
		return false;

	return fList.RemoveItem(item);
}

BString *BStringList::RemoveItem(int32 index)
{
	return (BString*)fList.RemoveItem(index);
}

bool BStringList::RemoveItems(int32 index, int32 count)
{
	return (BString*)fList.RemoveItems(index,count);
}

bool BStringList::ReplaceItem(int32 index, BString *newItem)
{
	return fList.ReplaceItem(index,newItem);
}

bool BStringList::HasItem(const char *string) const
{
	if(!string)
		return false;
	
	for(int32 i=0; i<CountItems(); i++)
	{
		BString *item=ItemAt(i);
		if(item->Compare(string)==0)
			return true;
	}
	return false;
}

BString *BStringList::FindItem(const char *string) const
{
	if(!string)
		return NULL;
	
	for(int32 i=0; i<CountItems(); i++)
	{
		BString *item=ItemAt(i);
		if(item->Compare(string)==0)
			return item;
	}
	return NULL;
}

void BStringList::MakeEmpty(bool freemem)
{
	if(freemem)
	{
		BString *str;
		for(int32 i=0; i<fList.CountItems(); i++)
		{
			str=(BString*)fList.ItemAtFast(i);
			if(str)
				delete str;
		}
	}
	fList.MakeEmpty();
}

void BStringList::PrintToStream(void)
{
	BString *str;
	
	printf("BStringList: %ld items\n",fList.CountItems());
	
	for(int32 i=0; i<fList.CountItems(); i++)
	{
		str=ItemAt(i);
		printf("%ld: %s\n",i,(str)?str->String():NULL);
	}
}
