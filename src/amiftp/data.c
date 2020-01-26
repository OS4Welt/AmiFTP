/* RCS Id: $Id: data.c 1.625 1996/07/04 17:50:58 lilja Exp $
   Locked version: $Revision: 1.625 $
*/

#include "AmiFTP.h"
#include "gui.h"

struct sockaddr_in data_addr;
struct sockaddr_in myctladdr;
int      data;
int      verbose;
int      code;
int      cpend;
int      curtype;
int      crflag;
int      sendport;
int      connected;
int      timedout;
int      which_up_cmd;
int      SortMode;
int      remote_sort_direction;
int      non_unix;
int      remote_os_type;
int      current_year;
int      current_month;

int      cin=-1,cout=-1;

int      ftp_port;

char	response_line[MAXLINE];
char	scratch[1024 + 1024 + 10];
LONG opts[OPT_COUNT];
struct RDArgs *argsptr;

BOOL    ConfigChanged;

char    *header_name;
char    *other_dir_pattern;
/*              "PERMS LINKS USER GROUP SIZE MONTH DAY TIME NAME" */
char    *unix_dir_pattern= "\001 \002 \003 \004 \005 \006 \007 \010 \011";
char    *defaultanonymouspw;

struct Library    *IntuitionBase;
struct Library          *UtilityBase;
struct Library          *GfxBase;
struct Library          *DiskfontBase;
struct Library          *AslBase;
struct Library          *IFFParseBase;
struct Library          *RexxSysBase;
struct Library         *IconBase;
struct Library    *WorkbenchBase;
struct Library          *LocaleBase;
struct Library          *AmigaGuideBase;
struct Device          *TimerBase;
struct AGInfo ag;

struct MsgPort          *RexxPort;
struct MsgPort          *TimerPort;
struct TimeRequest *TimeRequest;
ULONG AG_Signal=0UL;

struct List *FileList;
struct List TempList;

struct CurrentState CurrentState;

BOOL MenuNeedsUpdate=FALSE; /* Tells main idcmp routine to update the menus when it's safe to do so */
int TransferMode=BINARY;
           
//little helper for amigaos4
#include <gadgets/listbrowser.h>
#include <gadgets/chooser.h>
#include <proto/chooser.h>
#include <proto/requester.h>
#include <classes/requester.h>

__attribute__((linearvarargs)) void LBEditNode(Object *list, struct Window *window, struct Requester *r, struct Node *n, ULONG tag, ...)
{
    static struct lbEditNode msg =
    {
        LBM_EDITNODE,      /* LBM_EDITNODE */
        NULL,     /* to provide rendering info */
        NULL,      /* modify this node */
        NULL/* according to this tag list */
    };


    va_list args;
    va_startlinear (args, tag);
    msg.lbe_Node = n;
    msg.lbe_NodeAttrs = va_getlinearva(args, struct TagItem*);
    IDoMethodA(list, (Msg) &msg);
    va_end(args);
}

struct List *ChooserLabelsA(STRPTR *nameList)
{
    struct List *newList;
    newList = (struct List *)AllocSysObjectTags(ASOT_LIST, TAG_DONE);


	if (newList && nameList)
    {
        while(*nameList)
        {

          AddTail(newList, AllocChooserNode(CNA_Text, *nameList, TAG_END));
          nameList++;
        }
    }

    return newList;
}

void FreeChooserLabels(struct List *list)
{
    if (list)
    {
         FreeSysObject(ASOT_LIST, (APTR)list);
    }
}

void strmfp(char *file, char *path, char *node,int size)
{
    file[0]='\0';
	if (path)
        AddPart(file, path, size);

    if (node) AddPart(file, node, size);

}

int stcgfn(char *node, char *name, int size)
{
    CONST_STRPTR fileptr = FilePart(name);
    if (fileptr)
    {
        strncpy(node, fileptr, size);
        return strlen(node)+1;
        }

	return 0;
}

 __attribute__((linearvarargs)) int showStringRequester(struct Window *window, BOOL invisible, STRPTR icon, STRPTR Title, STRPTR Gadget, STRPTR buffer, uint32 maxChars, STRPTR Body, ...)
{
     static struct TagItem tags[] = {
     { REQ_Type, REQTYPE_STRING },
     { REQ_TitleText, 0UL},
     { REQ_BodyText, 0UL},
     { REQ_GadgetText, (ULONG)"_Ok|_Cancel" },
     { REQ_VarArgs, 0UL},
     { REQ_Image, 0UL} ,
     { REQS_Buffer, 0UL },
     { REQS_MaxChars, 0UL },
     {REQS_Invisible, 0UL},
     { TAG_END, 0 }
     };

    tags[6].ti_Data = (Tag) buffer;
    tags[7].ti_Data = (Tag) maxChars;
    if (invisible)
        tags[8].ti_Data = 1;
    static struct orRequest reqmsg;

    reqmsg.MethodID = RM_OPENREQ;
	reqmsg.or_Window = window;
	reqmsg.or_Screen = NULL;
	reqmsg.or_Attrs = tags;

    int result = 0;

    tags[5].ti_Data = REQIMAGE_INFO;
    Object *pictureImage = NULL;
    if ((Tag)icon>REQIMAGE_INSERTDISK)
    {
    	pictureImage = NewObject(NULL, "bitmap.image",
					   BITMAP_SourceFile, icon,
					   BITMAP_Screen, window->WScreen,
					   BITMAP_Precision, PRECISION_EXACT,
					   BITMAP_Masking, TRUE,
					   TAG_END);
		if (pictureImage)
        {
        	tags[5].ti_Data = (Tag)pictureImage;
            }
    }
    else tags[5].ti_Data = (Tag)icon;

    Object *requester = NewObject(NULL, "requester.class", TAG_DONE);
    if (requester==NULL) return result;


    va_list ap;

	va_startlinear(ap, Body);
    tags[1].ti_Data = (Tag)Title;
	tags[2].ti_Data = (Tag)Body;
    if (Gadget)
		tags[3].ti_Data = (Tag)Gadget;
    tags[4].ti_Data = (Tag)va_getlinearva(ap, void *);
    result = IDoMethodA(requester, (Msg) &reqmsg);
    va_end(ap);

    DisposeObject(requester);
    if (pictureImage) DisposeObject(pictureImage);
    return result;
}

 __attribute__((linearvarargs)) int showRequester(struct Window *window, STRPTR icon, STRPTR Title, STRPTR Gadget, STRPTR Body, ...)
{
     static struct TagItem tags[] = {
     { REQ_Type, REQTYPE_INFO },
     { REQ_TitleText, 0UL},
     { REQ_BodyText, 0UL},
     { REQ_GadgetText, (ULONG)"_Ok" },
     { REQ_VarArgs, 0UL},
     { REQ_Image, 0UL} ,
     { TAG_END, 0 }
     };
                
    static struct orRequest reqmsg;

    reqmsg.MethodID = RM_OPENREQ;
	reqmsg.or_Window = window;
	reqmsg.or_Screen = NULL;
	reqmsg.or_Attrs = tags;

    int result = 0;

    tags[5].ti_Data = REQIMAGE_INFO;
    Object *pictureImage = NULL;
    if ((Tag)icon>REQIMAGE_INSERTDISK)
    {
    	pictureImage = NewObject(NULL, "bitmap.image",
					   BITMAP_SourceFile, icon,
					   BITMAP_Screen, window->WScreen,
					   BITMAP_Precision, PRECISION_EXACT,
					   BITMAP_Masking, TRUE,
					   TAG_END);
		if (pictureImage)
        { 
        	tags[5].ti_Data = (Tag)pictureImage;
            }
    }
    else tags[5].ti_Data = (Tag)icon;

    Object *requester = NewObject(NULL, "requester.class", TAG_DONE);
    if (requester==NULL) return result;


    va_list ap;

	va_startlinear(ap, Body);
    tags[1].ti_Data = (Tag)Title;
	tags[2].ti_Data = (Tag)Body;
    if (Gadget)
		tags[3].ti_Data = (Tag)Gadget;
    tags[4].ti_Data = (Tag)va_getlinearva(ap, void *);
    result = IDoMethodA(requester, (Msg) &reqmsg);
    va_end(ap);

    DisposeObject(requester);
    if (pictureImage) DisposeObject(pictureImage);
    return result;
}

int getfa(CONST_STRPTR name)
{
    int result = 0;
    struct ExamineData *dat = ExamineObjectTags(EX_StringNameInput,name,TAG_END);
    if (dat)
    {
        if (EXD_IS_DIRECTORY(dat)) result = 1;
        else if (EXD_IS_FILE(dat)) result = -1;

        FreeDosObject(DOS_EXAMINEDATA,dat);
        }

    return result;
}



