/* RCS Id:  $Id: gui.h 1.795 1996/09/28 13:32:58 lilja Exp $
   Locked version: $Revision: 1.795 $
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxbase.h>
#include <graphics/rpattr.h>
#include <string.h>
#include <math.h>

#include <proto/window.h>
#include <classes/window.h>

#include <proto/layout.h>
#include <gadgets/layout.h>

#include <proto/button.h>
#include <gadgets/button.h>

#include <proto/string.h>
#include <gadgets/string.h>

#include <proto/arexx.h>
#include <classes/arexx.h>

#include <proto/clicktab.h>
#include <gadgets/clicktab.h>

#include <proto/speedbar.h>
#include <gadgets/speedbar.h>

#include <proto/checkbox.h>
#include <gadgets/checkbox.h>

#include <proto/fuelgauge.h>
#include <gadgets/fuelgauge.h>

#include <proto/integer.h>
#include <gadgets/integer.h>

#include <proto/chooser.h>
#include <gadgets/chooser.h>

#include <proto/listbrowser.h>
#include <gadgets/listbrowser.h>

#include <proto/penmap.h>
#include <images/penmap.h>

#include <proto/bitmap.h>
#include <images/bitmap.h>

#include <proto/bevel.h>
#include <images/bevel.h>

#include <proto/label.h>
#include <images/label.h>

#include <proto/requester.h>
#include <classes/requester.h>

#include <reaction/reaction_macros.h>

#include <libraries/gadtools.h>


#ifndef QUALIFIER_SHIFT
#define RAWKEY_CURSORUP 76
#define RAWKEY_CURSORDOWN 77
#define QUALIFIER_SHIFT 0x03
#define QUALIFIER_ALT 0x30
#define QUALIFIER_CTRL 0x08
#endif

extern struct RastPort *NonPropRPort,*PropRPort;
extern UWORD PropFHigh,MinHigh,MinWide,PropBLine;
extern UWORD NonPropFHigh,NonPropBLine;

extern char *PropFontName,*NonPropFontName,*PubScreenName;
extern int PropFontSize,NonPropFontSize;
extern struct TextFont *Propdfont,*NonPropdfont;
extern struct TextAttr Propta,NonPropta;

void ComputeMinSize(UWORD *minwide,UWORD *minheight);
BOOL LayoutElements(struct Gadget **gadg,UWORD MinWide,UWORD MinWidth);
struct Window *OpenFTPWindow(const BOOL StartIconified);
int CloseFTPWindow(void);

int Parent_clicked(void);
int File_clicked(void);
int Put_clicked(void);
int Connect_clicked(void);
int DLPath_clicked(void);
int Disconnect_clicked(void);
int Site_clicked(void);
int Dir_clicked(void);
int Abort_clicked(void);

#define GetString( g ) ((( struct StringInfo * )g->SpecialInfo)->Buffer)
#define GetNumber( g ) ((( struct StringInfo * )g->SpecialInfo)->LongInt)

extern struct Screen *Screen;
extern struct DrawInfo *DrawInfo;

extern Object *MainWin_Object;
extern Object *MG_List[];

extern struct ColumnInfo columninfo[];

enum {
    MG_ListView=0,
    MG_SiteName, MG_DirName, MG_CacheList,
    MG_DLString, MG_DLButton,
    MG_Parent, MG_Get, MG_Put, MG_View,
    MG_Readme, MG_Get2, MG_Put2, MG_View2,
    MG_Connect, MG_Disconnect, MG_Reload,
    MG_Page2, MG_SpeedBar,
    NumGadgets_main
  };
void LBEditNode(Object *list, struct Window *window, struct Requester *r, struct Node *n, ULONG tag, ...);
struct List *ChooserLabelsA(STRPTR *nameList);
void FreeChooserLabels(struct List *list);
/* EOF */
