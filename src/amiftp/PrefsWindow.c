/* RCS Id: $Id: PrefsWindow.c 1.815 1996/12/14 12:13:03 lilja Exp lilja $
   Locked version: $Revision: 1.815 $
*/

#include "AmiFTP.h"
#include "gui.h"

struct Window *MainPrefsWindow;
Object *MainPrefsLayout;
Object *MainPrefsWin_Object;

struct Window *OpenMainPrefsWindow(void);
void CloseMainPrefsWindow(void);
void UpdateConfig(void);
void FreeClicktabList(struct List *list);
int BuildClicktabList(void);

//static int SelectFont(char *FontName, UWORD *YSize);

enum {
	MPG_Tab=0, MPG_Page,
	MPG_AnonymousPW, MPG_ViewCommand, MPG_DefaultDLDir,// MPG_DefaultDLDirGad,
	MPG_ProxyHost, MPG_ProxyPort, MPG_ProxyDefault, MPG_IgnoreCase,
	MPG_BufferSize,MPG_CacheSize,MPG_DeleteOnExit,MPG_DisplayBeep,
	MPG_ADTPattern, MPG_PreserveDir, MPG_ShowMOTD, MPG_GetReadme, 
	MPG_DefaultScreen, MPG_PublicScreen,
	MPG_DefaultFonts, /*MPG_InterfaceFont,*/ MPG_InterfaceGetFontB, /*MPG_FilelistFont,*/
	MPG_FilelistGetFontB, /*MPG_ShowButtons,*/ MPG_ToolBar, MPG_Log, MPG_LogFile,
	MPG_OK, MPG_Cancel,
	NumGadgets_MPG
};

Object *MPG_List[NumGadgets_MPG];

static char buf1[100],buf2[100],buf3[100],buf4[100],/*buf5[100],*/buf6[513];
static char showLog;
static char pubscreen[256];
struct TextAttr fileListFontTextAttrs, interfaceFontTextAttrs;
//static struct DrawInfo *drinfo;
static struct List tablist;
static Object *pagelayout;
static BOOL MainInterfaceRestart=FALSE, SpeedBarUpdated=FALSE;
static ULONG LastPage=0;

static char intfontname[64];
static char listfontname[64];
static char intfont[60];
static char listfont[60];
static UWORD intsize;
static UWORD listsize;

struct cltabs {
	APTR String;
	ULONG Enabled;
};

/*struct cltabs clicktabs[] = {
	{MPW_General,    TRUE},
	{MPW_ADT,        FALSE},
	{MPW_Display,    FALSE},
	{MPW_Proxy,      FALSE},
	{MENU_LogWindow, FALSE},
};*/

int OpenPrefsWindow(void)
{
	ULONG done=FALSE;
	ULONG signal,wait,mainwinsignal;

	MainInterfaceRestart=SpeedBarUpdated=FALSE;

	if (!BuildClicktabList()) {
		FreeClicktabList(&tablist);
		return 0;
	}

	if (!OpenMainPrefsWindow()) {
		struct Node *node;
		while( (node=RemHead(&tablist)) ) {
			FreeClickTabNode(node);
		}
		return 0;
	}

	GetAttr(WINDOW_SigMask, MainPrefsWin_Object, &signal);
	GetAttr(WINDOW_SigMask, MainWin_Object, &mainwinsignal);
	LockWindow(MainWin_Object);
	//APTR lw = SleepWindow(MainWindow);

	while (!done) {
		wait=Wait(signal|mainwinsignal|AG_Signal);
		if (wait&AG_Signal)     { HandleAmigaGuide(); }
		if (wait&signal)        { done=HandleMainPrefsIDCMP(); }
		if (wait&mainwinsignal) { HandleMainWindowIDCMP(FALSE); }
	}

	CloseMainPrefsWindow();
	UnlockWindow(MainWin_Object);
	//WakeWindow(MainWindow,lw);

DBUG("MainInterfaceRestart %ld\n",MainInterfaceRestart);
	if (MainInterfaceRestart) {
		CloseMainWindow();
		OpenFTPWindow(FALSE);
	}

	if(SpeedBarUpdated) {
		uint32 val=0;
		GetAttr(SPEEDBAR_Buttons, MG_List[MG_SpeedBar], &val);
//DBUG("UpdatedSpeedBar (%ld) 0x%08lx\n",MainPrefs.mp_ShowToolBar,val);
		if(MainPrefs.mp_ShowToolBar) {
			extern struct List SpeedBarList;
			SetAttrs(MG_List[MG_SpeedBar], SPEEDBAR_Buttons,&SpeedBarList, SPEEDBAR_ButtonType,MainPrefs.mp_ShowToolBar-1, TAG_END);
			if(SPEEDBAR_Buttons == ~0) IDoMethod(MG_List[MG_SpeedBarGroup], OM_ADDMEMBER, MG_List[MG_SpeedBar]);
		}
		else {
			SetAttrs(MG_List[MG_SpeedBar], SPEEDBAR_Buttons,~0, TAG_END);
			IDoMethod(MG_List[MG_SpeedBarGroup], OM_REMMEMBER, MG_List[MG_SpeedBar]);
		}

		if(MainWindow) { IDoMethod(MainWin_Object, WM_RETHINK); }
	}


	FreeClicktabList(&tablist);

	return 1;
}

ULONG HandleMainPrefsIDCMP(void)
{
    ULONG result,done=FALSE;
    uint16 code;
    //while ((result=CA_HandleInput(MainPrefsWin_Object, &code))!=WMHI_LASTMSG) {
    while ((result=IDoMethod(MainPrefsWin_Object, WM_HANDLEINPUT, &code))!=WMHI_LASTMSG) {

         code = result&WMHI_KEYMASK;
	switch (result & WMHI_CLASSMASK) {
	  case WMHI_CLOSEWINDOW:
	    done=TRUE;
	    break;
	  case WMHI_RAWKEY:
	    if (code==69)
	      done=TRUE;
	    else if (code==95)
	      SendAGMessage(AG_GLOBPREFS);
	    break;
	  case WMHI_GADGETUP:
	    switch (result & WMHI_GADGETMASK) {
	      case MPG_Tab: {
		  ULONG tabnum, pagenum;

		  GetAttrs(MPG_List[MPG_Tab], CLICKTAB_Current, &tabnum, TAG_DONE);
		  GetAttrs(MPG_List[MPG_Page], PAGE_Current, &pagenum, TAG_DONE);
		  if (tabnum!=pagenum) {
		      SetGadgetAttrs((struct Gadget*)MPG_List[MPG_Page], MainPrefsWindow, NULL,
				     PAGE_Current, tabnum, TAG_DONE);
		      RethinkLayout((struct Gadget*)pagelayout, MainPrefsWindow, NULL, TRUE);
		  }
	    	break;
	      }

        case MPG_FilelistGetFontB:
        {
    	 	struct gfRequest fontRequest = {GFONT_REQUEST, MainPrefsWindow};
    		uint32 result = IDoMethodA(MPG_List[MPG_FilelistGetFontB], (struct _Msg *)&fontRequest);
            if (result)
            {
                struct TextAttr *newAttrib =NULL;
    			GetAttr(GETFONT_TextAttr, MPG_List[MPG_FilelistGetFontB], (ULONG*)&newAttrib);

    			if (newAttrib)
                {
                    listsize = newAttrib->ta_YSize;
                    Strlcpy(listfont, newAttrib->ta_Name, sizeof(listfont));//-1);
    				SNPrintf(listfontname, sizeof(listfontname), "%s/%ld", listfont,listsize);
                }
        	}
		}
    	break;

		case MPG_InterfaceGetFontB:
        {
    	 	struct gfRequest fontRequest = {GFONT_REQUEST, MainPrefsWindow};
    		uint32 result = IDoMethodA(MPG_List[MPG_InterfaceGetFontB], (struct _Msg *)&fontRequest);
            if (result)
            {
                     
                struct TextAttr *newAttrib =NULL;
    			GetAttr(GETFONT_TextAttr, MPG_List[MPG_InterfaceGetFontB], (ULONG*)&newAttrib);
         
    			if (newAttrib)
                {
                    intsize = newAttrib->ta_YSize;
                    Strlcpy(intfont, newAttrib->ta_Name, sizeof(intfont));//-1);
                    SNPrintf(intfontname, sizeof(intfontname), "%s/%ld", intfont, intsize);
                }
        	}
		}
    	break;

		case MPG_DefaultDLDir:
            {
                struct gfileRequest pathList = {GFILE_REQUEST, MainPrefsWindow};
    			uint32 result = IDoMethodA(MPG_List[MPG_DefaultDLDir], (struct _Msg *)&pathList);
                if (result)
                {
                    char *strBuffer = NULL;
        			GetAttr(GETFILE_Drawer, MPG_List[MPG_DefaultDLDir], (ULONG*)&strBuffer);
        			if (strBuffer)
                    {
                        Strlcpy(buf3, strBuffer, sizeof(buf3));//-1);

                        }
                    }

    		 }
            break;

	     case MPG_DefaultScreen:
		{
		    ULONG attr;
		    GetAttr(GA_Selected, MPG_List[MPG_DefaultScreen], &attr);
		    SetGadgetAttrs((struct Gadget*)MPG_List[MPG_PublicScreen], MainPrefsWindow, NULL,
				       GA_Disabled, attr? TRUE : FALSE,
				       TAG_DONE);
			RefreshGList((struct Gadget*)MPG_List[MPG_PublicScreen], MainPrefsWindow, NULL, 1);
		}
		break;
	      case MPG_DefaultFonts:
		{
		    ULONG attr;
		    //int i;
            GetAttr(GA_Selected, MPG_List[MPG_DefaultFonts], &attr);
            SetAttrs(MPG_List[MPG_InterfaceGetFontB], GA_Disabled, attr?TRUE:FALSE, TAG_DONE);
            SetAttrs(MPG_List[MPG_FilelistGetFontB], GA_Disabled, attr?TRUE:FALSE, TAG_DONE);

            RefreshGList((struct Gadget*)MPG_List[MPG_InterfaceGetFontB], MainPrefsWindow, NULL, 1);
            RefreshGList((struct Gadget*)MPG_List[MPG_FilelistGetFontB], MainPrefsWindow, NULL, 1);
		}
		break;
	      case MPG_OK: {
		  done=TRUE;
		  UpdateConfig();
		  break;
	      }
	      case MPG_Cancel: {
		  done=TRUE;
		  break;
	      }
	    }
	    break;
	}
    }
    return done;
}

struct Window *OpenMainPrefsWindow(void)
{
    //struct LayoutLimits limits;

    if (!MainPrefs.mp_FontName) {
	strcpy(intfont, AmiFTPAttrF->ta_Name);
	intsize=AmiFTPAttrF->ta_YSize;
    }
    else {
	strcpy(intfont, MainPrefs.mp_FontName);
	intsize=MainPrefs.mp_FontSize;
    }

    if (!MainPrefs.mp_ListFontName) {
	strcpy(listfont, ListViewAttrF->ta_Name);
	listsize=ListViewAttrF->ta_YSize;
    }
    else {
	strcpy(listfont, MainPrefs.mp_ListFontName);
	listsize=MainPrefs.mp_ListFontSize;
    }

    memcpy(&interfaceFontTextAttrs, AmiFTPAttrF, sizeof(interfaceFontTextAttrs));

    memcpy(&fileListFontTextAttrs, ListViewAttrF, sizeof(fileListFontTextAttrs));

    SNPrintf(intfontname, sizeof(intfontname), "%s/%ld", intfont,intsize);
    SNPrintf(listfontname, sizeof(listfontname), "%s/%d", listfont,listsize);

    showLog = MainPrefs.mp_Log;

    if (MainPrefsWindow)
      return MainPrefsWindow;

    if (MainPrefs.mp_AnonymousPW)
      strcpy(buf1, MainPrefs.mp_AnonymousPW);
    else
      memset(buf1, 0, sizeof(buf1));

    if (MainPrefs.mp_ViewCommand)
      strcpy(buf2, MainPrefs.mp_ViewCommand);
    else
      memset(buf2, 0, sizeof(buf2));

    if (MainPrefs.mp_LocalDir)
      strcpy(buf3, MainPrefs.mp_LocalDir);
    else
      memset(buf3, 0, sizeof(buf3));

    if (MainPrefs.mp_ProxyServer)
      strcpy(buf4, MainPrefs.mp_ProxyServer);
    else
      memset(buf4, 0, sizeof(buf4));

    if (MainPrefs.mp_HideADTPattern)
      strcpy(buf6, MainPrefs.mp_HideADTPattern);
    else
      memset(buf6, 0, sizeof(buf6));

    if (MainPrefs.mp_PubScreen)
    {
      strcpy(pubscreen, MainPrefs.mp_PubScreen);
    }
    else
    {
      memset(pubscreen, 0, sizeof(pubscreen));
    }

STRPTR toolbar_array[] = {NULL,NULL,NULL,NULL, NULL};//{"No","Images & Text","only Text","only Images", NULL};
toolbar_array[0] = (STRPTR)GetAmiFTPString(MPW_ShowToolbar_NO);
toolbar_array[1] = (STRPTR)GetAmiFTPString(MPW_ShowToolbar_BOTH);
toolbar_array[2] = (STRPTR)GetAmiFTPString(MPW_ShowToolbar_TEXT);
toolbar_array[3] = (STRPTR)GetAmiFTPString(MPW_ShowToolbar_IMAGES);

STRPTR notify_array[] = {NULL,NULL,NULL,NULL, NULL};//{"Beep","Notifiction","Beep & Notify","Do nothing", NULL};
notify_array[0] = (STRPTR)GetAmiFTPString(MPW_AfterTransf_Beep);
notify_array[1] = (STRPTR)GetAmiFTPString(MPW_AfterTransf_Notify);
notify_array[2] = (STRPTR)GetAmiFTPString(MPW_AfterTransf_Both);
notify_array[3] = (STRPTR)GetAmiFTPString(MPW_AfterTransf_DoNothing);

    MainPrefsLayout=NewObject(LayoutClass, NULL,//LayoutObject,
                     GA_DrawInfo, DrawInfo,
                     GA_TextAttr, AmiFTPAttrF,
                     LAYOUT_DeferLayout, TRUE,
                     LAYOUT_SpaceOuter,  FALSE,
                     LAYOUT_Orientation, LAYOUT_ORIENT_VERT,

                     LAYOUT_AddChild, MPG_List[MPG_Tab]=NewObject(ClickTabClass, NULL,//ClickTabObject,
                          GA_ID,        MPG_Tab,
                          GA_RelVerify, TRUE,
                          CLICKTAB_Labels,  &tablist,
                          CLICKTAB_Current, LastPage,
                     TAG_DONE),

                     LAYOUT_AddChild, pagelayout=NewObject(LayoutClass, NULL,//LayoutObject,
                       LAYOUT_SpaceOuter, TRUE,
                     LAYOUT_AddChild, MPG_List[MPG_Page]=NewObject(NULL,"page.gadget",//PageObject,

                     PAGE_Add, NewObject(LayoutClass, NULL,//LayoutObject, /* Start of page 0 General */
                     GA_TextAttr, AmiFTPAttrF,
                     LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
                     LAYOUT_HorizAlignment, LALIGN_RIGHT,
                     LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartVGroup,
                     LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                     LAYOUT_AddChild, MPG_List[MPG_AnonymousPW]=NewObject(StringClass, NULL,//StringObject,
                       GA_ID,        MPG_AnonymousPW,
                       GA_RelVerify, TRUE,
                       GA_TabCycle,  TRUE,
                       STRINGA_Buffer,   (ULONG)buf1,
                       STRINGA_MaxChars, 50,
                       TAG_DONE),
                       Label(GetAmiFTPString(MPW_Password)),

                     LAYOUT_AddChild, MPG_List[MPG_ViewCommand]=NewObject(StringClass, NULL,//StringObject,
                       GA_ID,        MPG_ViewCommand,
                       GA_RelVerify, TRUE,
                       GA_TabCycle,  TRUE,
                       STRINGA_Buffer,   (ULONG)buf2,
                       STRINGA_MaxChars, 50,
                       TAG_DONE),
                       Label(GetAmiFTPString(MPW_ViewCommand)),
                       LAYOUT_AddChild, MPG_List[MPG_DefaultDLDir] = NewObject(GetFileClass, NULL,//GetFileObject,
                       GA_ID,        MG_DLGetFile,
                       GA_RelVerify, TRUE,
                       GETFILE_DrawersOnly, TRUE,
                       GETFILE_TitleText,   GetAmiFTPString(Str_SelectDLPath),
                       GETFILE_Drawer,      buf3,
                       GETFILE_ReadOnly,    TRUE,
	                      TAG_DONE),
                       Label(GetAmiFTPString(MPW_DefDownloadDir)),

                     LAYOUT_AddChild, MPG_List[MPG_BufferSize]=NewObject(IntegerClass, NULL,//IntegerObject,
                       GA_ID,        MPG_BufferSize,
                       GA_RelVerify, TRUE,
                       GA_TabCycle,  TRUE,
                       INTEGER_Minimum,  512,
                       INTEGER_Number,   (ULONG)MainPrefs.mp_BufferSize,
                       INTEGER_MaxChars, 8,
                       TAG_DONE),
//                       CHILD_NominalSize, TRUE,
//                       CHILD_WeightedWidth, 0,
                       Label(GetAmiFTPString(MPW_BufferSize)),

                     LAYOUT_AddChild, MPG_List[MPG_CacheSize]=NewObject(IntegerClass, NULL,//IntegerObject,
                       GA_ID,        MPG_CacheSize,
                       GA_RelVerify, TRUE,
                       GA_TabCycle,  TRUE,
                       INTEGER_Number,   (ULONG)MainPrefs.mp_CacheSize,
                       INTEGER_Minimum,   1,
                       INTEGER_Maximum,  10,
                       INTEGER_MaxChars,  6,
                       TAG_DONE),
//                       CHILD_NominalSize,TRUE,
//                       CHILD_WeightedWidth,0,
                       Label(GetAmiFTPString(MPW_DirCacheSize)),
                     TAG_DONE),
                     LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartHGroup,
                     LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartVGroup,
                     LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                     LAYOUT_AddChild, MPG_List[MPG_DeleteOnExit]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                       GA_ID,        MPG_DeleteOnExit,
                       GA_RelVerify, TRUE,
                       GA_Text,      GetAmiFTPString(MPW_DeleteViewed),
                       GA_Selected,  MainPrefs.mp_DeleteFiles,
                       CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),
                     TAG_DONE),

                     LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartVGroup,
                     LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                     LAYOUT_HorizAlignment, LALIGN_RIGHT,
                     LAYOUT_AddChild, MPG_List[MPG_IgnoreCase]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                       GA_ID,        MPG_IgnoreCase,
                       GA_RelVerify, TRUE,
                       GA_Text,      GetAmiFTPString(MPW_IgnoreCase),
                       GA_Selected,  MainPrefs.mp_IgnoreCase,
                       CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),
                       //CHILD_WeightMinimum, TRUE,
                     LAYOUT_AddChild, MPG_List[MPG_DisplayBeep]=NewObject(ChooserClass, NULL,//ChooserObject,
                     //LAYOUT_AddChild, MPG_List[MPG_DisplayBeep]=CheckBoxObject,
                       GA_ID,        MPG_DisplayBeep,
                       GA_RelVerify, TRUE,
                       //GA_Text,      GetAmiFTPString(MPW_Beep),
                       CHOOSER_LabelArray, &notify_array,
                       CHOOSER_Selected,   MainPrefs.mp_DisplayBeep,
                       //GA_Selected,  MainPrefs.mp_DisplayBeep,
                       //CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),
                       Label(GetAmiFTPString(MPW_AfterTransf)),
                       CHILD_WeightedWidth, 0,
                     TAG_DONE),
                     CHILD_WeightedHeight, 0,
                     TAG_DONE),
                     //CHILD_WeightedWidth,  0,
                     CHILD_WeightedHeight, 0,
                   TAG_DONE), /* End of page 0 */

                   PAGE_Add, NewObject(LayoutClass, NULL,//VGroupObject,
                   LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                   LAYOUT_VertAlignment, LALIGN_TOP,//TAligned, //LayoutObject, /* Start of page 1 ADT */
                     GA_TextAttr, AmiFTPAttrF,
                     LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartVGroup,
                     LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                     LAYOUT_VertAlignment, LALIGN_TOP,//TAligned, 
                     LAYOUT_AddChild, MPG_List[MPG_ADTPattern]=NewObject(StringClass, NULL,//StringObject,
                       GA_ID,     MPG_ADTPattern,
                       GA_RelVerify, TRUE,
                       GA_TabCycle,  TRUE,
                       STRINGA_Buffer,   (ULONG)buf6,
                       STRINGA_MaxChars, 511,
                       TAG_DONE),
                       Label(GetAmiFTPString(MPW_HideDirectories)),

                     LAYOUT_AddChild, MPG_List[MPG_PreserveDir]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                       GA_ID,        MPG_PreserveDir,
                       GA_RelVerify, TRUE,
                       GA_Disabled,  TRUE,
                       GA_Text,      GetAmiFTPString(MPW_PreservePath),
                       CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),
                       //CHILD_WeightMinimum, TRUE,

                     LAYOUT_AddChild, MPG_List[MPG_ShowMOTD]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                       GA_ID,        MPG_ShowMOTD,
                       GA_RelVerify, TRUE,
                       GA_Text,      GetAmiFTPString(MPW_ShowMOTD),
                       GA_Selected,  MainPrefs.mp_ShowMOTD,
                       CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),
                       //CHILD_WeightMinimum, TRUE,

                     LAYOUT_AddChild, MPG_List[MPG_GetReadme]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                       GA_ID,        MPG_GetReadme,
                       GA_RelVerify, TRUE,
                       GA_Selected,  MainPrefs.mp_GetReadme,
                       GA_Text,      GetAmiFTPString(MPW_GetReadme),
                       CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),
                       //CHILD_WeightMinimum, TRUE,
                   TAG_DONE),
                   CHILD_WeightedHeight, 0,
                   TAG_DONE), /* End of page 1 */

                   PAGE_Add, NewObject(LayoutClass, NULL,//VGroupObject,
                   LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                   LAYOUT_VertAlignment, LALIGN_TOP,//TAligned, /* Start of page 3 display */
                     GA_TextAttr, AmiFTPAttrF,
                     LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartVGroup,
                     LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                     LAYOUT_VertAlignment, LALIGN_TOP,//TAligned,
                     LAYOUT_HorizAlignment, LALIGN_RIGHT,
                     LAYOUT_AddChild, MPG_List[MPG_DefaultScreen]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                       GA_ID,        MPG_DefaultScreen,
                       GA_RelVerify, TRUE,
                       GA_Selected,  MainPrefs.mp_OpenOnDefaultScreen,
                       GA_Text,      GetAmiFTPString(MPW_DefaultScreen),
                       CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),
                       //CHILD_WeightMinimum, TRUE,

                     LAYOUT_AddChild, MPG_List[MPG_PublicScreen]=NewObject(StringClass, NULL,//StringObject,
                       GA_ID,        MPG_PublicScreen,
                       GA_Disabled,  MainPrefs.mp_OpenOnDefaultScreen? TRUE : FALSE,
                       GA_RelVerify, TRUE,
                       GA_TabCycle,  TRUE,
                       STRINGA_Buffer,   (ULONG)pubscreen,
                       STRINGA_MaxChars, 255,
                       TAG_DONE),
                       Label(GetAmiFTPString(MPW_PublicScreen)),

                     LAYOUT_AddChild, MPG_List[MPG_DefaultFonts]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                       GA_ID,        MPG_DefaultFonts,
                       GA_RelVerify, TRUE,
                       GA_Selected,  MainPrefs.mp_UseDefaultFonts,
                       GA_Text,      GetAmiFTPString(MPW_SystemFonts),
                       CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),
                       //CHILD_WeightMinimum, TRUE,

                       LAYOUT_AddChild, MPG_List[MPG_InterfaceGetFontB] = NewObject(GetFontClass, NULL,//GetFontObject,
                       GA_ID,        MPG_InterfaceGetFontB,
                       GA_RelVerify, TRUE,
                       GA_TextAttr,  AmiFTPAttrF,
                       GA_Disabled,  MainPrefs.mp_UseDefaultFonts?TRUE:FALSE,
                       //GETFONT_TitleText, GetAmiFTPString(MPW_SelectFont),//"Select font...",
                       GETFONT_TextAttr,  &interfaceFontTextAttrs,
                       TAG_DONE),
                       CHILD_WeightedHeight, 0,
                     Label(GetAmiFTPString(MPW_InterfaceFont)),

                     LAYOUT_AddChild, MPG_List[MPG_FilelistGetFontB] = NewObject(GetFontClass, NULL,//GetFontObject,
                       GA_ID,        MPG_FilelistGetFontB,
                       GA_RelVerify, TRUE,
                       GA_TextAttr,  AmiFTPAttrF,
                       GA_Disabled,  MainPrefs.mp_UseDefaultFonts?TRUE:FALSE,
                       //GETFONT_TitleText, GetAmiFTPString(MPW_SelectFont),//"Select font...",
                       GETFONT_TextAttr,  &fileListFontTextAttrs,
                       TAG_DONE),
                       CHILD_WeightedHeight, 0,
                     Label(GetAmiFTPString(MPW_FilelistFont)),

                     /*LAYOUT_AddChild, MPG_List[MPG_ShowButtons]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                       GA_ID, MPG_ShowButtons,
                       GA_RelVerify, TRUE,
                       GA_Selected, MainPrefs.mp_ShowButtons,
                       GA_Text, GetAmiFTPString(MPW_ShowButtons),
                       CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       CheckBoxEnd,
                       CHILD_WeightMinimum, TRUE,  */

                     LAYOUT_AddChild, MPG_List[MPG_ToolBar]=NewObject(ChooserClass, NULL,//ChooserObject,
                     //LAYOUT_AddChild, MPG_List[MPG_ToolBar]=CheckBoxObject,
                       GA_ID,        MPG_ToolBar,
                       GA_RelVerify, TRUE,
                       CHOOSER_LabelArray, &toolbar_array,
                       CHOOSER_Selected,   MainPrefs.mp_ShowToolBar,
                       //GA_Selected, MainPrefs.mp_ShowToolBar,
                       //GA_Text, GetAmiFTPString(MPW_ShowToolbar),
                       //CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       //CheckBoxEnd,
                       TAG_DONE),
                       Label(GetAmiFTPString(MPW_ShowToolbar)),
                       CHILD_WeightedWidth, 0,
                       //CHILD_WeightMinimum, TRUE,
                   TAG_DONE),
                   CHILD_WeightedHeight, 0,
                   TAG_DONE), /* End of page 3 */

                   PAGE_Add, NewObject(LayoutClass, NULL,//VGroupObject,
                   LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                   LAYOUT_VertAlignment, LALIGN_TOP,//TAligned, /* Start of page 2 proxy */
                     GA_TextAttr, AmiFTPAttrF,
                     LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartVGroup,
                     LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                     LAYOUT_VertAlignment, LALIGN_TOP,//TAligned, 
                     LAYOUT_AddChild, MPG_List[MPG_ProxyHost]=NewObject(StringClass, NULL,//StringObject,
                       GA_ID,        MPG_ProxyHost,
                       GA_RelVerify, TRUE,
                       GA_TabCycle,  TRUE,
                       STRINGA_Buffer,   (ULONG)buf4,
                       STRINGA_MaxChars, 40,
                       TAG_DONE),
                       Label(GetAmiFTPString(MPW_ProxyHost)),

                     LAYOUT_AddChild, MPG_List[MPG_ProxyPort]=NewObject(IntegerClass, NULL,//IntegerObject,
                       GA_ID,        MPG_ProxyPort,
                       GA_RelVerify, TRUE,
                       GA_TabCycle,  TRUE,
                       INTEGER_Number,   MainPrefs.mp_ProxyPort,
                       //INTEGER_Arrows,   FALSE,
                       INTEGER_MaxChars, 5,
                       INTEGER_MinVisible, 5,
                       INTEGER_Minimum, 0,
                       INTEGER_Maximum, 65535,
                       TAG_DONE),
                       //CHILD_NominalSize, TRUE,
                       CHILD_WeightedWidth, 0,
                       Label(GetAmiFTPString(MPW_ProxyPort)),
                     LAYOUT_AddChild, MPG_List[MPG_ProxyDefault]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                       GA_ID,        MPG_ProxyDefault,
                       GA_RelVerify, TRUE,
                       GA_Text,      GetAmiFTPString(MPW_ProxyDefault),
                       GA_Selected,  MainPrefs.mp_DefaultProxy,
                       CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),
                       //CHILD_WeightMinimum, TRUE,
                   TAG_DONE),
                   CHILD_WeightedHeight, 0,
                   TAG_DONE), /* End of page 2 */

                   PAGE_Add, NewObject(LayoutClass, NULL,//VGroupObject,
                   LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                   LAYOUT_VertAlignment, LALIGN_TOP,//TAligned, /* Start of page log */
                     GA_TextAttr, AmiFTPAttrF,
                     LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartVGroup,
                     LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                     LAYOUT_VertAlignment, LALIGN_TOP,//TAligned,
                     /*
                     LAYOUT_AddChild, IntegerObject,
                       GA_ID, MPG_ProxyPort,
                       GA_RelVerify, TRUE,
                       GA_TabCycle, TRUE,
                       INTEGER_Number, MainPrefs.mp_ProxyPort,
                       INTEGER_Arrows,FALSE,
                       INTEGER_MaxChars, 6,
                       IntegerEnd,
                       CHILD_NominalSize, TRUE,
                       CHILD_WeightedWidth, 0,
                       Label("Log: size"),  */
                     LAYOUT_AddChild, MPG_List[MPG_Log] = NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                       GA_ID,        MPG_ProxyDefault,
                       GA_RelVerify, TRUE,
                       GA_Text,      GetAmiFTPString(MPW_ShowLogWindow),
                       GA_Selected,  showLog!=0,
                       CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),
                       //CHILD_WeightMinimum, TRUE,
                   TAG_DONE),
                   //CHILD_WeightedHeight, 0,
                   TAG_DONE), /* End of page 2 */

                   PAGE_Current, LastPage, TAG_DONE), /* End of pageobject */
                   TAG_DONE), /* End of layout containing pageobject */

                   /*LAYOUT_AddImage, BevelObject,
                     BEVEL_Style, BVS_SBAR_VERT,
                   BevelEnd,*/

                   LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartHGroup,
                   //
                   LAYOUT_EvenSize, TRUE,//EvenSized,
                     LAYOUT_SpaceOuter, TRUE,
                     LAYOUT_BevelStyle, BVS_SBAR_VERT,

                     LAYOUT_AddChild, MPG_List[MPG_OK]=NewObject(ButtonClass, NULL,//ButtonObject,
                       GA_Text,      GetAmiFTPString(MPW_OK),
                       GA_ID,        MPG_OK,
                       GA_RelVerify, TRUE,
                     TAG_DONE),
                     //CHILD_NominalSize, TRUE,
                     CHILD_WeightedWidth, 0,

                     LAYOUT_AddChild, MPG_List[MPG_Cancel]=NewObject(ButtonClass, NULL,//ButtonObject,
                       GA_Text,      GetAmiFTPString(MPW_Cancel),
                       GA_ID,        MPG_Cancel,
                       GA_RelVerify, TRUE,
                     TAG_DONE),
                     //CHILD_NominalSize, TRUE,
                     CHILD_WeightedWidth, 0,
                   TAG_DONE),
                   CHILD_WeightedHeight, 0,
                   //CHILD_WeightMinimum, TRUE,
                   TAG_DONE);

    if (!MainPrefsLayout)
      return NULL;

    SetAttrs(MPG_List[MPG_Page], PAGE_Current, LastPage, TAG_DONE);

    //LayoutLimits((struct Gadget *)MainPrefsLayout, &limits, PropFont, Screen);
    //limits.MinHeight+=Screen->WBorTop+Screen->WBorBottom;
    //limits.MinWidth+=Screen->WBorLeft+Screen->WBorRight;

    MainPrefsWin_Object = NewObject(WindowClass, NULL,//WindowObject,
                          WA_Title,        GetAmiFTPString(MPW_WinTitle),
                          WA_PubScreen,    Screen,
                          WA_DepthGadget,  TRUE,
                          WA_DragBar,      TRUE,
                          WA_CloseGadget,  TRUE,
                          WA_Activate,     TRUE,
                          WA_SmartRefresh, TRUE,
                          WA_SizeGadget,   TRUE,
                          //WA_Top, MainWindow->TopEdge+(MainWindow->Height-limits.MinHeight)/2,
                          //WA_Left, MainWindow->LeftEdge+(MainWindow->Width-limits.MinWidth)/2,
                          WA_IDCMP, IDCMP_RAWKEY,
                          WINDOW_Position,  WPOS_CENTERWINDOW,
                          WINDOW_RefWindow, MainWindow,
                          WINDOW_Layout,    MainPrefsLayout,
                          WINDOW_GadgetUserData, WGUD_FUNC,
                          WINDOW_LockHeight, TRUE,
                        TAG_DONE);

    if (!MainPrefsWin_Object)
      return NULL;

    if ( (MainPrefsWindow=(struct Window *)IDoMethod(MainPrefsWin_Object, WM_OPEN)) ) { //CA_OpenWindow(MainPrefsWin_Object)) {
	return MainPrefsWindow;
    }
    DisposeObject(MainPrefsLayout);
    MainPrefsLayout=NULL;

    return NULL;
}

void UpdateConfig(void)
{
	ULONG attr=0;

	GetAttr(GA_Selected, MPG_List[MPG_ProxyDefault], &attr);
	if (attr!=MainPrefs.mp_DefaultProxy) {
		MainPrefs.mp_DefaultProxy=attr;
		ConfigChanged=TRUE;
	}

	//GetAttr(GA_Selected, MPG_List[MPG_DisplayBeep], &attr);
	GetAttr(CHOOSER_Selected, MPG_List[MPG_DisplayBeep], &attr);
	if (attr!=MainPrefs.mp_DisplayBeep) {
		MainPrefs.mp_DisplayBeep=attr;
		ConfigChanged=TRUE;
	}

	GetAttr(GA_Selected, MPG_List[MPG_DeleteOnExit], &attr);
	if (attr!=MainPrefs.mp_DeleteFiles) {
		MainPrefs.mp_DeleteFiles=attr;
		ConfigChanged=TRUE;
	}

	GetAttr(GA_Selected, MPG_List[MPG_IgnoreCase], &attr);
	if (attr!=MainPrefs.mp_IgnoreCase) {
		MainPrefs.mp_IgnoreCase=attr;
		ConfigChanged=TRUE;
	}

	if (!MainPrefs.mp_AnonymousPW) {
		if(strlen(buf1)) {
			MainPrefs.mp_AnonymousPW=strdup(buf1);
			ConfigChanged=TRUE;
		}
	}
	else {
		if (strecmp(MainPrefs.mp_AnonymousPW, buf1)) {
			free(MainPrefs.mp_AnonymousPW);
			MainPrefs.mp_AnonymousPW=NULL;

			if (strlen(buf1)) MainPrefs.mp_AnonymousPW=strdup(buf1);

			ConfigChanged=TRUE;
		}
	}

	if (!MainPrefs.mp_ViewCommand) {
		if(strlen(buf2)) {
			MainPrefs.mp_ViewCommand=strdup(buf2);
			ConfigChanged=TRUE;
		}
	}
	else {
		if (strecmp(MainPrefs.mp_ViewCommand, buf2)) {
			free(MainPrefs.mp_ViewCommand);
			MainPrefs.mp_ViewCommand=NULL;

			if (strlen(buf2)) MainPrefs.mp_ViewCommand=strdup(buf2);

			ConfigChanged=TRUE;
		}
	}

	if (!MainPrefs.mp_LocalDir) {
		if(strlen(buf3)) {
			MainPrefs.mp_LocalDir=strdup(buf3);
			ConfigChanged=TRUE;
		}
	}
	else {
		if (strecmp(MainPrefs.mp_LocalDir, buf3)) {
			free(MainPrefs.mp_LocalDir);
			MainPrefs.mp_LocalDir=NULL;

			if (strlen(buf3)) MainPrefs.mp_LocalDir=strdup(buf3);

			ConfigChanged=TRUE;
		}
	}

	if (!MainPrefs.mp_ProxyServer) {
		if(strlen(buf4)) {
			MainPrefs.mp_ProxyServer=strdup(buf4);
			ConfigChanged=TRUE;
		}
	}
	else {
		if (strecmp(MainPrefs.mp_ProxyServer, buf4)) {
			free(MainPrefs.mp_ProxyServer);
			MainPrefs.mp_ProxyServer=NULL;

			if (strlen(buf4)) MainPrefs.mp_ProxyServer=strdup(buf4);

			ConfigChanged=TRUE;
		}
	}

	GetAttr(INTEGER_Number, MPG_List[MPG_ProxyPort], &attr);
	if (attr!=MainPrefs.mp_ProxyPort) {
		MainPrefs.mp_ProxyPort=attr;
		ConfigChanged=TRUE;
	}

	if (!MainPrefs.mp_HideADTPattern) {
		if(strlen(buf6)) {
			MainPrefs.mp_HideADTPattern=strdup(buf6);
			ConfigChanged=TRUE;
		}
	}
	else {
		if (strecmp(MainPrefs.mp_HideADTPattern, buf6)) {
			free(MainPrefs.mp_HideADTPattern);
			MainPrefs.mp_HideADTPattern=NULL;

			if (strlen(buf6)) MainPrefs.mp_HideADTPattern=strdup(buf6);

			ConfigChanged=TRUE;
		}
	}

	if (!MainPrefs.mp_FontName) {
		MainPrefs.mp_FontName=strdup(intfont);
		MainPrefs.mp_FontSize=intsize;
		ConfigChanged=TRUE;
		MainInterfaceRestart=TRUE;
	}
	else {
		if (strecmp(MainPrefs.mp_FontName, intfont) || MainPrefs.mp_FontSize!=intsize) {
			free(MainPrefs.mp_FontName);
			MainPrefs.mp_FontName=strdup(intfont);
			MainPrefs.mp_FontSize=intsize;
			ConfigChanged=TRUE;
			MainInterfaceRestart=TRUE;
		}
	}

	if (!MainPrefs.mp_ListFontName) {
		MainPrefs.mp_ListFontName=strdup(listfont);
		MainPrefs.mp_ListFontSize=listsize;
		ConfigChanged=TRUE;
		MainInterfaceRestart=TRUE;
	}
	else {
		if (strecmp(MainPrefs.mp_ListFontName, listfont) || MainPrefs.mp_ListFontSize!=listsize) {
			free(MainPrefs.mp_ListFontName);
			MainPrefs.mp_ListFontName=strdup(listfont);
			MainPrefs.mp_ListFontSize=listsize;
			ConfigChanged=TRUE;
			MainInterfaceRestart=TRUE;
		}
	}

	GetAttr(GA_Selected, MPG_List[MPG_ShowMOTD], &attr);
	if (attr!=MainPrefs.mp_ShowMOTD) {
		MainPrefs.mp_ShowMOTD=attr;
		ConfigChanged=TRUE;
	}

	GetAttr(GA_Selected, MPG_List[MPG_GetReadme], &attr);
	if (attr!=MainPrefs.mp_GetReadme) {
		MainPrefs.mp_GetReadme=attr;
		ConfigChanged=TRUE;
	}

	GetAttr(INTEGER_Number, MPG_List[MPG_BufferSize], &attr);
	if (attr!=MainPrefs.mp_BufferSize) {
		MainPrefs.mp_BufferSize=attr;
		ConfigChanged=TRUE;
	}

	GetAttr(INTEGER_Number, MPG_List[MPG_CacheSize], &attr);
	if (attr!=MainPrefs.mp_CacheSize) {
		MainPrefs.mp_CacheSize=attr;
		ConfigChanged=TRUE;
	}

	GetAttr(GA_Selected, MPG_List[MPG_DefaultFonts], &attr);
	if (attr!=MainPrefs.mp_UseDefaultFonts) {
		MainPrefs.mp_UseDefaultFonts=attr;
		ConfigChanged=TRUE;
		MainInterfaceRestart=TRUE;
	}

	GetAttr(GA_Selected, MPG_List[MPG_DefaultScreen], &attr);
	if (attr!=MainPrefs.mp_OpenOnDefaultScreen) {
		MainPrefs.mp_OpenOnDefaultScreen=attr;
		ConfigChanged=TRUE;
		MainInterfaceRestart=TRUE;
	}

     /*
    GetAttr(GA_Selected, MPG_List[MPG_ShowButtons], &attr);
    if (attr!=MainPrefs.mp_ShowButtons) {
	MainPrefs.mp_ShowButtons=attr;
	ConfigChanged=TRUE;
	MainInterfaceRestart=TRUE;
    }
    */

    //GetAttr(GA_Selected, MPG_List[MPG_ToolBar], &attr);
	GetAttr(CHOOSER_Selected, MPG_List[MPG_ToolBar], &attr);
	if (attr!=MainPrefs.mp_ShowToolBar) {
		MainPrefs.mp_ShowToolBar=attr;
		ConfigChanged=TRUE;
		//MainInterfaceRestart=TRUE;
		SpeedBarUpdated = TRUE;
	}

	if (!MainPrefs.mp_PubScreen) {
		if (strlen(pubscreen)) {
			MainPrefs.mp_PubScreen=strdup(pubscreen);
			ConfigChanged=TRUE;
			MainInterfaceRestart=TRUE;
		}
	}
	else {
		if (strecmp(MainPrefs.mp_PubScreen, pubscreen)) {
			free(MainPrefs.mp_PubScreen);
			MainPrefs.mp_PubScreen=NULL;

			if (strlen(pubscreen)) MainPrefs.mp_PubScreen=strdup(pubscreen);

			ConfigChanged=TRUE;
			MainInterfaceRestart=TRUE;
		}
	}

	extern BPTR LogWindow;
	ULONG logenabled = 0;
	GetAttr(GA_Selected, MPG_List[MPG_Log], &logenabled);
	if ((logenabled==0) != (LogWindow==0)) {
		MainPrefs.mp_Log = logenabled!=0;

		if (logenabled) OpenLogWindow();
		else CloseLogWindow();

		ConfigChanged=TRUE;
		//MainInterfaceRestart=TRUE;
	}

}

void CloseMainPrefsWindow(void)
{
DBUG("CloseMainPrefsWindow()\n",NULL);
	if (MainPrefsWin_Object) {
		GetAttrs(MPG_List[MPG_Page], PAGE_Current,&LastPage, TAG_DONE);
		DisposeObject(MainPrefsWin_Object);
		MainPrefsWindow=NULL;
		MainPrefsWin_Object=NULL;
		MainPrefsLayout=NULL;
	}
}

/*static int SelectFont(char *FontName, UWORD *YSize)
{
    struct FontRequester *FontRequester;
    static ULONG font_tags[]={
	ASLFO_Window, 0UL,
	ASLFO_PrivateIDCMP, TRUE,
	ASLFO_SleepWindow, TRUE,
	ASLFO_TitleText,  0UL,
	ASLFO_InitialLeftEdge, 0UL,
	ASLFO_InitialTopEdge, 0UL,
	ASLFO_InitialName, 0UL,
	ASLFO_InitialSize, 0UL,
	TAG_END
      };
    int retval=0;

    font_tags[1]=(unsigned long)MainPrefsWindow;
    //font_tags[7]=(unsigned long)"Select font...";
    font_tags[9]=MainPrefsWindow->LeftEdge;
    font_tags[11]=MainPrefsWindow->TopEdge;
    font_tags[13]=(ULONG)FontName;
    font_tags[15]=(ULONG)*YSize;

    FontRequester=AllocAslRequest(ASL_FontRequest, NULL);
    if (!FontRequester)
      return 0;

    LockWindow(MainPrefsWin_Object);

    if (AslRequest(FontRequester, (struct TagItem *)font_tags)) {
	strcpy(FontName, FontRequester->fo_Attr.ta_Name);
	*YSize=FontRequester->fo_Attr.ta_YSize;
	retval=1;
    }
    FreeAslRequest(FontRequester);

    UnlockWindow(MainPrefsWin_Object);

    return retval;
}*/

int BuildClicktabList(void)
{
	struct Node *node;
	int i;
	struct cltabs clicktabs[] = {
		{MPW_General,    TRUE},
		{MPW_ADT,        FALSE},
		{MPW_Display,    FALSE},
		{MPW_Proxy,      FALSE},
		{MENU_LogWindow, FALSE}
	};

	NewList(&tablist);

	for (i=0; i<5; i++) {
		if ( (node=(struct Node *)AllocClickTabNode(TNA_Text,GetAmiFTPString(clicktabs[i].String),
		                                            TNA_Number, i,
		                                            //TNA_Enabled, clicktabs[i].Enabled,
		                                            TNA_Spacing, 6,
		                                           TAG_DONE)) ) {
			AddTail(&tablist, node);
		}
		else return 0;
	}

	return 1;
}

void FreeClicktabList(struct List *list)
{
	struct Node *node;

	while( (node=RemHead(list)) ) { FreeClickTabNode(node); }
}

// EOF
