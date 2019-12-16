#include <Font.h>
#include <View.h>
#include "BookRow.h"

BookRow::BookRow(ConfigFile *file, bool marked)
	: BRow()
{
	
	SetFile(file);
}

BookRow::~BookRow(void)
{
}
