/* RCS Id: $Id: SpeedBar.c 1.815 1996/12/14 12:13:03 lilja Exp lilja $
   Version: $Revision: 1.815 $
*/

#include "AmiFTP.h"
#include "gui.h"

enum {SB_CONNECT, SB_PARENT, SB_RELOAD, SB_DOWNLOAD, SB_UPLOAD, SB_MKDIR, SB_DELETE, SB_RENAME, SB_SHOW, SB_SETTINGS, SB_END, SB_DISCONNECT = SB_END};
int spacing[SB_END]=
		{4,		   8,			4,			 8,		   4,			8,		 4,			  4,		   4,		8};
struct List SpeedBarList;
static struct Image *speedImages[SB_END+1];
static STRPTR text[SB_END+1];


void InitSpeedBarList(void)
{
    struct Node *node;
    WORD i=0;
    extern struct Screen *Screen;

    APTR oldvalue = SetProcWindow((APTR)-1L);
    NewList(&SpeedBarList);
    text[i++] =  GetAmiFTPString(MW_Connect);
    text[i++] = "Parent";
    text[i++] = GetAmiFTPString(MW_Reload);
    text[i++] = GetAmiFTPString(MW_Get);
    text[i++] = GetAmiFTPString(MW_Put);
    text[i++] = "Create Dir";
    text[i++] = "Delete";
    text[i++] = "Rename";
    text[i++] = GetAmiFTPString(MW_View);
    text[i++] = "Settings";
    text[i++] =  GetAmiFTPString(MW_Disconnect);

    speedImages[SB_CONNECT]= (struct Image *)NewObject(NULL, "bitmap.image",
			BITMAP_Screen, Screen,
			BITMAP_SourceFile,  "TBImages:connect",
			BITMAP_SelectSourceFile, "TBImages:connect_s",
			BITMAP_DisabledSourceFile, "TBImages:connect_g",
			BITMAP_Masking, TRUE,
			BITMAP_Transparent, TRUE,
			IA_Height, 32,
			IA_Width, 32,
			TAG_DONE);
    		 
    speedImages[SB_DISCONNECT]= (struct Image *)NewObject(NULL, "bitmap.image",
			BITMAP_Screen, Screen,
			BITMAP_SourceFile,  "TBImages:disconnect",
			BITMAP_SelectSourceFile, "TBImages:disconnect_s",
			BITMAP_DisabledSourceFile, "TBImages:disconnect_g",
			BITMAP_Masking, TRUE,
			BITMAP_Transparent, TRUE,
			IA_Height, 32,
			IA_Width, 32,
			TAG_DONE);

    speedImages[SB_PARENT]= (struct Image *)NewObject(NULL, "bitmap.image",
			BITMAP_Screen, Screen,
			BITMAP_SourceFile,  "TBImages:parent",
			BITMAP_SelectSourceFile, "TBImages:parent_s",
			BITMAP_DisabledSourceFile, "TBImages:parent_g",
			BITMAP_Masking, TRUE,
			BITMAP_Transparent, TRUE,
			IA_Height, 32,
			IA_Width, 32,
			TAG_DONE);

    speedImages[SB_RELOAD]= (struct Image *)NewObject(NULL, "bitmap.image",
		   BITMAP_Screen, Screen,
		   BITMAP_SourceFile,  "TBImages:reload",
		   BITMAP_SelectSourceFile, "TBImages:reload_s",
		   BITMAP_DisabledSourceFile, "TBImages:reload_g",
		   BITMAP_Masking, TRUE,
		   BITMAP_Transparent, TRUE,
		   IA_Height, 32,
		   IA_Width, 32,
		   TAG_DONE);

    speedImages[SB_DOWNLOAD]= (struct Image *)NewObject(NULL, "bitmap.image",
		   BITMAP_Screen, Screen,
		   BITMAP_SourceFile,  "TBImages:download",
		   BITMAP_SelectSourceFile, "TBImages:download_s",
		   BITMAP_DisabledSourceFile, "TBImages:download_g",
		   BITMAP_Masking, TRUE,
		   BITMAP_Transparent, TRUE,
		   IA_Height, 32,
		   IA_Width, 32,
		   TAG_DONE);

    speedImages[SB_UPLOAD]= (struct Image *)NewObject(NULL, "bitmap.image",
		   BITMAP_Screen, Screen,
		   BITMAP_SourceFile,  "TBImages:upload",
		   BITMAP_SelectSourceFile, "TBImages:upload_s",
		   BITMAP_DisabledSourceFile, "TBImages:upload_g",
		   BITMAP_Masking, TRUE,
		   BITMAP_Transparent, TRUE,
		   IA_Height, 32,
		   IA_Width, 32,
		   TAG_DONE);

    speedImages[SB_RELOAD]= (struct Image *)NewObject(NULL, "bitmap.image",
		   BITMAP_Screen, Screen,
		   BITMAP_SourceFile,  "TBImages:reload",
		   BITMAP_SelectSourceFile, "TBImages:reload_s",
		   BITMAP_DisabledSourceFile, "TBImages:reload_g",
		   BITMAP_Masking, TRUE,
		   BITMAP_Transparent, TRUE,
		   IA_Height, 32,
		   IA_Width, 32,
		   TAG_DONE);

    speedImages[SB_MKDIR]= (struct Image *)NewObject(NULL, "bitmap.image",
		   BITMAP_Screen, Screen,
		   BITMAP_SourceFile,  "TBImages:folder",
		   BITMAP_SelectSourceFile, "TBImages:folder_s",
		   BITMAP_DisabledSourceFile, "TBImages:folder_g",
		   BITMAP_Masking, TRUE,
		   BITMAP_Transparent, TRUE,
		   IA_Height, 32,
		   IA_Width, 32,
		   TAG_DONE);

    speedImages[SB_DELETE]= (struct Image *)NewObject(NULL, "bitmap.image",
		   BITMAP_Screen, Screen,
		   BITMAP_SourceFile,  "TBImages:delete",
		   BITMAP_SelectSourceFile, "TBImages:delete_s",
		   BITMAP_DisabledSourceFile, "TBImages:delete_g",
		   BITMAP_Masking, TRUE,
		   BITMAP_Transparent, TRUE,
		   IA_Height, 32,
		   IA_Width, 32,
		   TAG_DONE);

    speedImages[SB_RENAME]= (struct Image *)NewObject(NULL, "bitmap.image",
		   BITMAP_Screen, Screen,
		   BITMAP_SourceFile,  "TBImages:rename",
		   BITMAP_SelectSourceFile, "TBImages:rename_s",
		   BITMAP_DisabledSourceFile, "TBImages:rename_g",
		   BITMAP_Masking, TRUE,
		   BITMAP_Transparent, TRUE,
		   IA_Height, 32,

		   IA_Width, 32,
		   TAG_DONE);

    speedImages[SB_SHOW]= (struct Image *)NewObject(NULL, "bitmap.image",
		   BITMAP_Screen, Screen,
		   BITMAP_SourceFile,  "TBImages:show",
		   BITMAP_SelectSourceFile, "TBImages:show_s",
		   BITMAP_DisabledSourceFile, "TBImages:show_g",
		   BITMAP_Masking, TRUE,
		   BITMAP_Transparent, TRUE,
		   IA_Height, 32,
		   IA_Width, 32,
		   TAG_DONE);

    speedImages[SB_SETTINGS]= (struct Image *)NewObject(NULL, "bitmap.image",
		   BITMAP_Screen, Screen,
		   BITMAP_SourceFile,  "TBImages:settings",
		   BITMAP_SelectSourceFile, "TBImages:settings_s",
		   BITMAP_DisabledSourceFile, "TBImages:settings_g",
		   BITMAP_Masking, TRUE,
		   BITMAP_Transparent, TRUE,
		   IA_Height, 32,
		   IA_Width, 32,
		   TAG_DONE);
    for (i=0; i < SB_END; i++) {
	
	  if (node = (struct Node *)AllocSpeedButtonNode(i,
						       SBNA_Image, speedImages[i],
						       SBNA_Top, 0,
						       SBNA_Left, 0,
						       SBNA_Enabled, TRUE,
						       SBNA_Spacing, spacing[i],
						       SBNA_Highlight, SBH_IMAGE,
                               SBNA_Text, text[i],
                               SBNA_ButtonID, i,
						       TAG_DONE))
	  AddTail(&SpeedBarList, node);
    }

    SetProcWindow(oldvalue);
}

void FreeSpeedBarList(void)
{
    struct Node *node;

    while (node=RemHead(&SpeedBarList))
      FreeSpeedButtonNode(node);

    NewList(&SpeedBarList);
}

int HandleSpeedBar(int button)
{
    extern int prev_state;

    switch (button)
    {
     case SB_CONNECT:
        if (connected)
        {
            Disconnect_clicked();
            }
        else
        {
            Connect_clicked();
            }
     break;

     case SB_PARENT:
        Parent_clicked();
        break;

     case SB_RELOAD:
        Dir_clicked();
    break;

    case SB_DOWNLOAD:
        Get_clicked();
    break;
    case SB_UPLOAD:
        Put_clicked();
    break;

    case SB_MKDIR:
        CreateDir_clicked();
    break;

    case SB_DELETE:
        Delete_clicked();
    break;
    case SB_RENAME:
        Rename_clicked();
	break;
	case SB_SHOW:
        if (FileList && connected)
      		View_clicked(FALSE);
    break;
    case SB_SETTINGS:
        OpenPrefsWindow();
        break;
    }
    return 1;
}

extern Object *MainWindowLayout;
void UpdateSpeedBar(int state)
{
    extern int prev_state;

    if (prev_state!=MB_DISCONNECTED && state==MB_DISCONNECTED) {
	    struct Node *node;
	    node=GetHead(&SpeedBarList);
        int i =0;
	    for (; node; node=GetSucc(node), i++) {
        if (i==SB_CONNECT)
        { 
            SetSpeedButtonNodeAttrs(node, SBNA_Image, speedImages[SB_CONNECT], SBNA_Text, text[SB_CONNECT], TAG_DONE);
            SetGadgetAttrs((struct Gadget*)MG_List[MG_SpeedBar], MainWindow, NULL,
			 SPEEDBAR_OnButton, i,
			 TAG_DONE);
            }
		else if(i<SB_SETTINGS)
        {
            SetSpeedButtonNodeAttrs(node, SBNA_Disabled, TRUE, TAG_DONE);
            }
	    }
	if (MainWindow)
	  RefreshGList((struct Gadget*)MG_List[MG_SpeedBar], MainWindow, NULL, 1);
    }
    else if (prev_state==MB_DISCONNECTED && state!=MB_DISCONNECTED) {
	    struct Node *node;
	    node=GetHead(&SpeedBarList);
        int i = 0;
	    for (; node; node=GetSucc(node),i++) {
        if (i==SB_CONNECT)
        {  
            SetSpeedButtonNodeAttrs(node, SBNA_Image, speedImages[SB_DISCONNECT], SBNA_Text, text[SB_DISCONNECT], TAG_DONE);
            SetGadgetAttrs((struct Gadget*)MG_List[MG_SpeedBar], MainWindow, NULL,
			 SPEEDBAR_OnButton, i,
			 TAG_DONE);
            }
        else if(i<SB_SETTINGS)
        {
            SetSpeedButtonNodeAttrs(node, SBNA_Disabled, FALSE, TAG_DONE);
            }
	    }
	if (MainWindow)
	  RefreshGList((struct Gadget*)MG_List[MG_SpeedBar], MainWindow, NULL, 1);
    }
}
