#include "clip.h"
#include "debug.h"

cobraClip::cobraClip()
{
    debug(LOW, "cobraClip initializing...\n");
}

cobraClip::~cobraClip()
{
    debug(LOW, "cobraClip dieing...\n");
}

cobraClipList::cobraClipList(QSqlDriver* parent) :QSqlDatabase(parent)
{
    debug(LOW, "cobraClipList initializing...\n");
}

cobraClipList::~cobraClipList()
{
    debug(LOW, "cobraClipList dieing...\n");
}

bool
cobraClipList::updateClip(cobraClip& clip)
{

}

bool
cobraClipList::removeClip(int uid)
{

}

bool
cobraClipList::addClip(cobraClip& clip)
{

}
