/* RCS Id: $Id: dirlist.c 1.795 1996/09/28 13:32:58 lilja Exp $
   Locked version: $Revision: 1.795 $
*/

#include "AmiFTP.h"
#include "gui.h"
#include <locale.h>

#define ISDIR(x) (x)&0x4000

struct dirlist *new_direntry(char *remoteName, char *name,char *date,char *owner,
                             char *group,mode_t mode,int64 size)
{
    struct dirlist *tmp;

    tmp = (struct dirlist *)calloc(sizeof(struct dirlist),1);
    if (!tmp)
      return NULL;
    bzero((char *)tmp,sizeof(struct dirlist));
    tmp->name = (char *)malloc((unsigned int)(strlen(name) + 1));
    if (tmp->name == NULL) {
	free_direntry(tmp);
	return NULL;
    }

     tmp->remoteName = (char *)malloc((unsigned int)(strlen(remoteName) + 1));
    if (tmp->remoteName == NULL) {
	free_direntry(tmp);
	return NULL;
    }

    if (date) {
	tmp->date = (char *)malloc((unsigned int)(strlen(date) + 1));
	if (tmp->date == NULL) {
	    free_direntry(tmp);
	    return NULL;
	}
    }

    if (owner) {
	tmp->owner = (char *)malloc((unsigned int)(strlen(owner) + 1));
	if (tmp->owner == NULL) {
	    free_direntry(tmp);
	    return NULL;
	}
    }

    if (group) {
	tmp->group = (char *)malloc((unsigned int)(strlen(group) + 1));
	if (tmp->group == NULL) {
	    free_direntry(tmp);
	    return NULL;
	}
    }

/* Fix: Add tmp->lname=linkname(name); plus null-terminate tmp->name before the ->
    if (S_ISLNK(mode))
      lname=linkname(name);
    else
      lname=NULL;
    if (lname)
      free(lname);
*/

    strcpy(tmp->name, name);
    strcpy(tmp->remoteName, remoteName);

    if (date)
      strcpy(tmp->date, date);
    if (owner)
      strcpy(tmp->owner, owner);
    if (group)
      strcpy(tmp->group, group);
    tmp->mode = mode;
//    tmp->link = S_ISLNK(mode);
//    tmp->dir  = S_ISDIR(mode);
//    tmp->file = S_ISREG(mode);
    tmp->size = size;

    tmp->stringSize = (STRPTR)ASPrintf("%llD",size); // "automagically" (aka locale) adds groups separator
    /*STRPTR tmpString = (STRPTR)ASPrintf("%lld",size);
    int c = strlen(tmpString);

	if (c>separatorSize)
    {
        int i = 0;
    	int sepCounter=separatorSize;

        c = (c-1)/separatorSize + c;
        tmp->stringSize = (STRPTR) AllocVecTags(c*sizeof(char)+1, AVT_ClearWithValue, 0, TAG_DONE);
        c--;

        for (i = strlen(tmpString)-1; i >= 0; i-- )
        {
        	sepCounter--;
            tmp->stringSize[c--] = tmpString[i];
            if (sepCounter==0 && c>0)
            {
                tmp->stringSize[c--] = groupSeparator;// WAS decimalSperator;
                sepCounter=seperatorSize;
            }
        }
        if (c!=-1) printf("do c: %i strlen: %i\n", c, strlen(tmpString));
        FreeVec(tmpString);
    }
    else
    {
        tmp->stringSize = tmpString;
    }*/

    //
    /*
    if (size >= 1000000000000)
    {
        tmp->stringSize = ASPrintf("%ld GB",(int)(size/(1024*1024*1024)));
    }
    else if (size>=100000000)
    {
        tmp->stringSize = ASPrintf("%ld MB",(int)(size/(1024*1024)));
    }
    else if (size > 10000000)
    {
        tmp->stringSize = ASPrintf("%ld kB",(int)(size/1024));
    }
    else
    {
        tmp->stringSize = ASPrintf("%ld B",(int)(size));
    }
    */
    return tmp;
}

char donk[]="donk";
extern int filePen;
extern int drawerPen;
BOOL add_direntry(struct List *filelist, char *name, char *date,
                  char *owner, char *group, mode_t mode, int64 size,
                  int sort_mode, int sort_direction)
{
    struct dirlist *tmp;
    struct dirlist *oldprev=NULL;
    ULONG flags;
DBUG("add_direntry()\n",NULL);
	if (size == 0 && strlen(name) == 255) return TRUE;

    if ((!MainPrefs.mp_Showdotfiles) && name[0]=='.')
      flags=LBFLG_CUSTOMPENS|LBFLG_HIDDEN;
    else
      flags=LBFLG_CUSTOMPENS;
#ifndef __amigaos4__
    if (non_unix)
      sort_mode = SORTBYNAME;
    switch (sort_mode) {
      default:
//	Printf("Unknown sort mode in add_dirname.\n");
	// Fall through
      case SORTBYNAME:
	if (sort_direction == ASCENDING)
	  oldprev = sortupbyname(filelist, name, ISDIR(mode));
	else
	  oldprev = sortdownbyname(filelist, name, ISDIR(mode));
	break;
      case SORTBYDATE:
	if (sort_direction == ASCENDING)
	  oldprev = sortupbydate(filelist, date);
	else
	  oldprev = sortdownbydate(filelist, date);
	break;
 #if 0
      case SORTBYSIZE:
	if (sort_direction == ASCENDING)
	  oldprev = sortupbysize(filelist, size);
	else
	  oldprev = sortdownbysize(filelist, size);
	break;
 #endif
    }
#endif
    tmp = new_direntry(name, name, date, owner, group, mode, size);
    if (tmp) {
	struct Node *tmp2=AllocListBrowserNode(TOT_COLS,
					       LBNA_Flags,flags,
					       LBNA_Priority,tmp->mode&0x4000? -10 : S_ISLNK(tmp->mode)? 0:10, // sort by name 1st dirs then files
					       LBNA_UserData,(uint32)tmp, // store node for compare_func()
					       LBNA_Column,COL_NAME,
					        LBNCA_Text,tmp->name,
					        LBNCA_FGPen,tmp->mode&0x4000? drawerPen : filePen,
					       LBNA_Column,COL_SIZE,
					        LBNCA_Text,tmp->stringSize,
					        LBNCA_Justification,LCJ_RIGHT,
					        LBNCA_FGPen,tmp->mode&0x4000? drawerPen : filePen,
					       LBNA_Column,COL_TYPE,
					        LBNCA_Text,tmp->mode&0x4000? "(dir)" : S_ISLNK(tmp->mode)?"(link)":"",
					        LBNCA_Justification,LCJ_CENTRE,//RIGHT,
					        LBNCA_FGPen,tmp->mode&0x4000?drawerPen : filePen,
					       LBNA_Column,COL_DATE,
					        LBNCA_Text,tmp->date,
					        LBNCA_Justification,LCJ_RIGHT,
					        LBNCA_FGPen,tmp->mode&0x4000? drawerPen : filePen,
					       LBNA_Column,COL_OWN,
					        LBNCA_FGPen,tmp->mode&0x4000? drawerPen : filePen,
					        LBNCA_Justification,LCJ_RIGHT,
					        LBNCA_Text,tmp->owner,
					       LBNA_Column,COL_GRP,
					        LBNCA_Justification,LCJ_RIGHT,
					        LBNCA_FGPen,tmp->mode&0x4000? drawerPen : filePen,
					        LBNCA_Text,tmp->group,
					       TAG_DONE);
	if (tmp2) {
	    tmp2->ln_Name=(void *)tmp;
	    Insert(filelist,tmp2,(struct Node *)oldprev);
	    return TRUE;
	}
    }
    return FALSE;
}

void add_direntry_struct(struct List *filelist,
                         struct dirlist *dlist, int sort_mode, int sort_direction)
{
    struct Node *tmp,*tmp2;
    ULONG flags;
DBUG("add_direntry_struct()\n",NULL);
    if ((!MainPrefs.mp_Showdotfiles) && dlist->name[0]=='.')
      flags=LBFLG_CUSTOMPENS|LBFLG_HIDDEN;
    else
      flags=LBFLG_CUSTOMPENS;
    tmp2=AllocListBrowserNode(TOT_COLS,
			      LBNA_Flags,flags,
			      LBNA_Priority,dlist->mode&0x4000? -10 : S_ISLNK(dlist->mode)? 0:10, // sort by name 1st dirs then files
			      LBNA_UserData,(uint32)dlist, // store node for compare_func()
			      LBNA_Column,COL_NAME,
			       LBNCA_Text,dlist->name,
			       LBNCA_FGPen,dlist->mode&0x4000?drawerPen:filePen,
			      LBNA_Column,COL_SIZE,
			       LBNCA_Text,dlist->stringSize,
			       LBNCA_Justification,LCJ_RIGHT,
			       LBNCA_FGPen,dlist->mode&0x4000?drawerPen:filePen,
			      LBNA_Column,COL_TYPE,
			       LBNCA_Text,dlist->mode&0x4000?"(dir)":S_ISLNK(dlist->mode)?"(link)":"",
			       LBNCA_Justification,LCJ_CENTRE,//RIGHT,
			       LBNCA_FGPen,dlist->mode&0x4000?drawerPen:filePen,
			      LBNA_Column,COL_DATE,
			       LBNCA_Text,dlist->date,
			       LBNCA_Justification,LCJ_RIGHT,
			       LBNCA_FGPen,dlist->mode&0x4000?drawerPen:filePen,
			      LBNA_Column,COL_OWN,
			       LBNCA_Text,dlist->owner,
			       LBNCA_Justification,LCJ_RIGHT,
			       LBNCA_FGPen,dlist->mode&0x4000?drawerPen:filePen,
			      LBNA_Column,COL_GRP,
			       LBNCA_Text,dlist->group,
			       LBNCA_Justification,LCJ_LEFT,
			       LBNCA_FGPen,dlist->mode&0x4000?drawerPen:filePen,
			      TAG_DONE);
    if (!tmp2)
      return;

    tmp2->ln_Name=(void *)dlist;
#ifndef __amigaos4__
    if (non_unix)
      sort_mode = SORTBYNAME;
    switch (sort_mode) {
      default:
	// Fall through
      case SORTBYNAME:
	if (sort_direction == ASCENDING)
	  tmp = (struct Node *)sortupbyname(filelist, dlist->name,ISDIR(dlist->mode));
	else
	  tmp = (struct Node *)sortdownbyname(filelist, dlist->name,ISDIR(dlist->mode));
	break;
      case SORTBYDATE:
	if (sort_direction == ASCENDING)
	  tmp = (struct Node *)sortupbydate(filelist, dlist->date);
	else
	  tmp = (struct Node *)sortdownbydate(filelist, dlist->date);
	break;
#if 0
      case SORTBYSIZE:
	if (sort_direction == ASCENDING)
	  tmp = (struct Node *)sortupbysize(filelist, dlist->size);
	else
	  tmp = (struct Node *)sortdownbysize(filelist, dlist->size);
	break;
#endif
    }
#endif
    Insert(filelist,tmp2,tmp);
}

void free_direntry(struct dirlist *tmp)
{
    if (tmp->name)
      free(tmp->name);
     if (tmp->remoteName)
      free(tmp->remoteName);
    if (tmp->date)
      free(tmp->date);
    if (tmp->owner)
      free(tmp->owner);
    if (tmp->group)
      free(tmp->group);
    if (tmp->stringSize)
    	FreeVec(tmp->stringSize);
    free(tmp);
}

void free_dirlist(struct List *filelist)
{
    struct Node *node;
    struct dirlist *tmp;
DBUG("free_dirlist(0x%08lx)\n",filelist);
    while ( (node=RemHead(filelist)) ) {
//DBUG("  node=0x%08lx\n",node);
	tmp=(void *)node->ln_Name;
//DBUG("  tmp=0x%08lx\n",tmp);
	if (tmp) {
	    if (tmp->name)
	      free(tmp->name);
	    if (tmp->date)
	      free(tmp->date);
	    if (tmp->owner)
	      free(tmp->owner);
	    if (tmp->group)
	      free(tmp->group);
	    if (tmp->stringSize)
	      FreeVec(tmp->stringSize);
	    free(tmp);
tmp = NULL;
	}
//Remove(node);
//DBUG("  1\n",NULL);
	FreeListBrowserNode(node);
node = NULL;
    }
//DBUG("  2\n",NULL);
FreeListBrowserList(filelist);
//DBUG("  3\n",NULL);
    NewList(filelist);
//DBUG("  4\n",NULL);
}

#ifndef __amigaos4__
/* alphabetical order */
struct dirlist *sortupbyname(struct List *filelist,char *name, int dir)
{
    struct dirlist *tmp;
    struct Node *tmp2;
    int rval;

    for (tmp2=GetHead(filelist); tmp2 != NULL; tmp2=GetSucc(tmp2)) {
	tmp=(void *)tmp2->ln_Name;
	if ((dir)==(ISDIR(tmp->mode))) {
	    if (MainPrefs.mp_IgnoreCase)
	      rval=stricmp(name,tmp->name);
	    else
	      rval=strcmp(name,tmp->name);
	    if (rval < 0)
	      break;
	}
	else if (dir)
	  break;
    }
    if (tmp2)
      return (struct dirlist *)GetPred((struct Node *)tmp2);
    else {

    return (struct dirlist *)GetTail(filelist);

//	if (!EmptyList(filelist))
//	  return (struct dirlist *)GetTail(filelist);
//	else
//	  return NULL;

    }
}

struct dirlist *sortdownbyname(struct List *filelist,char *name,int dir)
{
    struct Node *tmp2;
    struct dirlist *tmp;
    int rval;

    for (tmp2=GetHead(filelist); tmp2 != NULL; tmp2=GetSucc(tmp2)) {
	tmp=(void *)tmp2->ln_Name;
	if ((dir)==(ISDIR(tmp->mode))) {
	    if (MainPrefs.mp_IgnoreCase)
	      rval=stricmp(name,tmp->name);
	    else
	      rval=strcmp(name,tmp->name);
	    if (rval > 0)
	      break;
	}
	else if (dir)
	  break;
    }
    if (tmp2)
      return (struct dirlist *)GetPred((struct Node *)tmp2);
    else {
        return (struct dirlist *)GetTail(filelist);

//	if (!EmptyList(filelist))
//	  return (struct dirlist *)GetTail(filelist);
//	else
//	  return NULL;

    }
}

/* least recently modified */

struct dirlist *sortupbydate(struct List *filelist,char *date)
{
    struct dirlist *tmp;
    struct Node *tmp2;

    for (tmp2=GetHead(filelist); tmp2 != NULL; tmp2=GetSucc(tmp2)) {
	tmp=(void *)tmp2->ln_Name;
	if (isearlier(date,tmp->date))
	  break;
    }
    if (tmp2)
      return (struct dirlist *)GetPred((struct Node *)tmp2);
    else {
        return (struct dirlist *)GetTail(filelist);

//	if (!EmptyList(filelist))
//	  return (struct dirlist *)GetTail(filelist);
//	else
//	  return NULL;

    }
}

struct dirlist *sortdownbydate(struct List *filelist, char *date)
{
    struct dirlist *tmp;
    struct Node *tmp2;

    for (tmp2 = GetHead(filelist); tmp2 != NULL; tmp2 = GetSucc(tmp2)) {
	tmp=(void *)tmp2->ln_Name;
	if (!isearlier(date, tmp->date))
	  break;		// need to go before next entry.
    }
    if (tmp2)
      return (struct dirlist *)GetPred((struct Node *)tmp2);
    else {
        return (struct dirlist *)GetTail(filelist);

//	if (!EmptyList(filelist))
//	  return (struct dirlist *)GetTail(filelist);
//	else
//	  return NULL;

    }
}

 #if 0
// Not used at the moment
// smallest to largest
struct dirlist *sortupbysize(struct List *filelist,int64 size)
{
    struct dirlist *tmp;
    struct Node *tmp2;
    for (tmp2=ListHead(filelist);ListEnd(tmp2);tmp2=ListNext(tmp2)) {
	tmp=(void *)tmp2->ln_Name;
	if (size<=tmp->size)
	  break;
    }
    if (tmp2)
      return (struct dirlist *)GetPred((struct Node *)tmp2);
    else {
	if (!EmptyList(filelist))
	  return (struct dirlist *)GetTail(filelist);
	else
	  return NULL;
    }
}
struct dirlist *sortdownbysize(struct List *filelist,int64 size)
{
    struct dirlist *tmp;
    struct Node *tmp2;

    for (tmp2=ListHead(filelist);ListEnd(tmp2);tmp2=ListNext(tmp2)) {
	tmp=(void *)tmp2->ln_Name;
	if (size>=tmp->size)
	  break;
    }
    if (tmp2)
      return (struct dirlist *)GetPred((struct Node *)tmp2);
    else {
	if (!EmptyList(filelist))
	  return (struct dirlist *)GetTail(filelist);
	else
	  return NULL;
    }
}
 #endif
#endif

int cummonthdays[] = {
	0,
	31,
	59,
	90,
	120,
	151,
	181,
	212,
	243,
	273,
	304,
	334
};

long	datetotime(char *date)
{
    struct tm tm;

    /* "Aug 19 19:47" */
    /* "Jan 10  1990" */

    if (index(date, ':')) {
	hour_time(date, &tm);
	if (tm.tm_mon > current_month)
	  tm.tm_year = current_year - 1;
	else
	  tm.tm_year = current_year;
    } else {
	year_time(date, &tm);
	tm.tm_hour = 0;
	tm.tm_min = 0;
    }
    return tm.tm_min + tm.tm_hour * 60
	    + (tm.tm_year * 365 + tm.tm_mday +
	       cummonthdays[tm.tm_mon]) * 1440;
}

#ifndef __amigaos4__
int isearlier(char *date1, char *date2)
{
    long	time1;
    long	time2;

    time1 = datetotime(date1);
    time2 = datetotime(date2);

    if (time1 < time2)
      return 1;
    return 0;
}

struct List *sort_filelist(struct List *old_filelist, int sort_mode, int sort_direction)
{
    struct Node *tmp,*next;
    struct List *new_filelist;

    if (!GetHead(old_filelist))
      return old_filelist;
    new_filelist=malloc(sizeof(struct List));
    if (!new_filelist)
      return old_filelist;
    NewList(new_filelist);
    tmp=GetHead(old_filelist);
    while(tmp) {
	next=GetSucc(tmp);
	Remove(tmp);
	add_direntry_struct(new_filelist, (struct dirlist *)tmp->ln_Name,
			    sort_mode, sort_direction);
	FreeListBrowserNode(tmp);
	tmp=next;
    }
    free(old_filelist);
    return new_filelist;
}
#endif

char *abbrev_month[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

void hour_time(char *date, struct tm *tm)
{
    char	month[10];
    int		i;

    bzero((char *)tm, sizeof (struct tm));
    sscanf(date, "%s%d%d:%d", month, &tm->tm_mday,
	   &tm->tm_hour, &tm->tm_min);
    for (i = 0; i < 12; i++)
      if (!strncmp(month, abbrev_month[i], 3))
	break;
    if (i != 12)
      tm->tm_mon = i;
}

void year_time(char *date, struct tm *tm)
{
    char	month[10];
    int		i;

    bzero((char *)tm, sizeof (struct tm));
    sscanf(date, "%s%d%d", month, &tm->tm_mday, &tm->tm_year);
    for (i = 0; i < 12; i++)
      if (!strncmp(month, abbrev_month[i], 3))
	break;
    if (i != 12)
      tm->tm_mon = i;
    if (tm->tm_year > 1900)
      tm->tm_year -= 1900;
}
