/* RCS Id: $Id: SiteWindow.c 1.692 1996/07/20 00:28:38 lilja Exp $
   Locked version: $Revision: 1.692 $
*/

#include "AmiFTP.h"
#include "gui.h"
//#include <intuition/sghooks.h>


extern void PrintSiteList(void);
BOOL AddToSiteList=FALSE;
BOOL HotListChanged=FALSE;

struct Window *EditSiteWindow;
Object *EditSiteLayout;
Object *EditSiteWin_Object;

struct Window *OpenEditSiteWindow(struct SiteNode *sn);
void CloseEditSiteWindow(struct SiteNode *sn);
int LocPath_clicked(void);
void SL_Up(void);
void SL_Down(void);
void SL_Top(void);
void SL_Bottom(void);
int NewGroup(void);
void swapEntries(struct Node *n, struct Node *pos_n);

enum {
    ESG_SiteName=0, ESG_SiteAddress, ESG_Port, ESG_RemDir, ESG_LocalDir,//ESG_LocString, ESG_LocGad,
    ESG_Anonymous, ESG_LoginPsw , ESG_LoginName, ESG_Password, ESG_LSType, ESG_HotList,
    ESG_Proxy, ESG_OK, ESG_Cancel, ESG_ADT,
    NumGadgets_ESG
};

Object *ESG_List[NumGadgets_ESG];

static STRPTR lsnames[]= {
    "UNIX",
    "VMS",
    "DOS",
    NULL
};

struct List *objlist;
//struct Hook EditHook;

#if 0
char RealString[100];
char Original[100];
#endif

struct SiteNode *Curr_sn;

/* SiteList window */

struct Window *SiteListWindow;
Object *SiteListLayout;
Object *SiteListWin_Object;

enum {
    SLG_SiteList=0, SLG_Connect, SLG_New, SLG_Edit, SLG_Remove, 
    SLG_Cancel, SLG_Up, SLG_Down, SLG_Top, SLG_Bottom,SLG_AddGroup, SLG_BarLabel,
    NumGadgets_SLG
};

Object *SLG_List[NumGadgets_SLG];

struct Window *OpenSiteWindow(const BOOL Connect);
void CloseSiteListWindow(void);
void UpdateSLGGadgets(const ULONG selected, const int type);

int EditClicked(void);
int RemoveClicked(void);
int NewClicked(void);

//static ULONG lsecs,lmics;

static char buf1[50],buf2[100],buf3[100],buf4[200],buf5[40],buf6[90];

int OpenEditWindow(struct SiteNode *sn)
{
	ULONG wait, esignal,ssignal, /*mainsignal,*/ done=FALSE;

	if (!OpenEditSiteWindow(sn)) return 0;

	GetAttr(WINDOW_SigMask, EditSiteWin_Object, &esignal);
	GetAttr(WINDOW_SigMask, SiteListWin_Object, &ssignal);
	//GetAttr(WINDOW_SigMask, MainWin_Object, &mainsignal);

	while (!done) {
		wait=Wait(esignal|ssignal/*|mainsignal*/|AG_Signal|SIGBREAKF_CTRL_C);
		if (wait & SIGBREAKF_CTRL_C) done=TRUE;
		if (wait & AG_Signal ) HandleAmigaGuide();
		if (wait & esignal   ) done=HandleEditSiteIDCMP();
		if (wait & ssignal   ) HandleSiteListIDCMP();
		//if (wait & mainsignal) HandleMainWindowIDCMP(FALSE);
	}

	CloseEditSiteWindow(sn);

	return 1;
}

ULONG HandleEditSiteIDCMP(void)
{
	ULONG result, done=FALSE;
	uint16 code;

	//while ((result=CA_HandleInput(EditSiteWin_Object, &code))!=WMHI_LASTMSG) {
	while ((result=IDoMethod(EditSiteWin_Object, WM_HANDLEINPUT, &code))!=WMHI_LASTMSG) {
		code = result&WMHI_KEYMASK;

		switch (result & WMHI_CLASSMASK) {
			case WMHI_CLOSEWINDOW:
				done=TRUE;
			break;
			case WMHI_RAWKEY:
				if (code==69) done=TRUE;
				else if (code==95) SendAGMessage(AG_SITECONF);
			break;
			case WMHI_GADGETUP:
				switch (result & WMHI_GADGETMASK) {
					case ESG_Anonymous:
					{
						ULONG attr;

						GetAttr(GA_Selected, ESG_List[ESG_Anonymous], &attr);

						SetGadgetAttrs((struct Gadget*)ESG_List[ESG_LoginName], EditSiteWindow, NULL,
						               GA_Disabled,attr, TAG_DONE);
						/*if (SetGadgetAttrs((struct Gadget*)ESG_List[ESG_LoginName], EditSiteWindow, NULL,
						                   GA_Disabled,attr, TAG_DONE))
							RefreshGList((struct Gadget*)ESG_List[ESG_LoginName], EditSiteWindow, NULL, 1);*/

						SetGadgetAttrs((struct Gadget*)ESG_List[ESG_Password], EditSiteWindow, NULL,
						               GA_Disabled,attr, TAG_DONE);
						/*if (SetGadgetAttrs((struct Gadget*)ESG_List[ESG_Password], EditSiteWindow, NULL,
						                   GA_Disabled,attr, TAG_DONE))
							RefreshGList((struct Gadget*)ESG_List[ESG_Password], EditSiteWindow, NULL, 1);*/
							RefreshGList((struct Gadget*)ESG_List[ESG_LoginPsw], EditSiteWindow, NULL, -1);
					}
					break;
					case ESG_LocalDir:
						LocPath_clicked();
					break;
					case ESG_OK:
						AddToSiteList=TRUE;
						done=TRUE;
					break;
					case ESG_Cancel:
						done=TRUE;
					break;
				}
			break;
		}

	}

	return done;
}

struct Window *OpenEditSiteWindow(struct SiteNode *sn)
{
    //struct LayoutLimits limits;
    Object *l1, /**l2,*/ *l3, *l4, *l5, *l6;

    if (EditSiteWindow)
      return EditSiteWindow;

    objlist=ChooserLabelsA(lsnames);
    if (!objlist)
      return NULL;

    AddToSiteList=FALSE;

    if (sn->sn_Node.ln_Name)
      strncpy(buf1, sn->sn_Node.ln_Name, sizeof(buf1));
    else
      buf1[0]=0;
    if (sn->sn_SiteAddress)
      strcpy(buf2, sn->sn_SiteAddress);
    else
      buf2[0]=0;
    if (sn->sn_RemoteDir)
      strcpy(buf3, sn->sn_RemoteDir);
    else
      buf3[0]=0;
    if (sn->sn_LocalDir)
      strcpy(buf4, sn->sn_LocalDir);
    else if (MainPrefs.mp_LocalDir)
      strcpy(buf4, MainPrefs.mp_LocalDir);
    else
      buf4[0]=0;
    if (sn->sn_LoginName)
      strcpy(buf5, sn->sn_LoginName);
    else
      buf5[0]=0;

	memset(buf6, 0, sizeof(buf6));
    if (sn->sn_Password) {
		strncpy(buf6, sn->sn_Password, sizeof(buf6));
    }


    EditSiteLayout=NewObject(LayoutClass, NULL,//LayoutObject,
                     //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                     GA_DrawInfo, DrawInfo,
                     GA_TextAttr, AmiFTPAttrF,
                     LAYOUT_DeferLayout, TRUE,
                     LAYOUT_SpaceOuter,  FALSE,
                     LAYOUT_Orientation, LAYOUT_ORIENT_VERT,

                     LAYOUT_AddChild, l3=NewObject(LayoutClass, NULL,//VLayoutObject,
                       LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                       LAYOUT_SpaceOuter,  TRUE,

//LAYOUT_AddChild, VLayoutObject,
                       LAYOUT_AddChild, ESG_List[ESG_SiteName]=NewObject(StringClass, NULL,//StringObject,
                         GA_ID,        ESG_SiteName,
                         GA_RelVerify, TRUE,
                         GA_TabCycle,  TRUE,
                         STRINGA_Buffer,   buf1,
                         STRINGA_MaxChars, sizeof(buf1),//50,
                       TAG_DONE),
                       Label(GetAmiFTPString(SCW_Name)),

                       LAYOUT_AddChild, ESG_List[ESG_SiteAddress]=NewObject(StringClass, NULL,//StringObject,
                         GA_ID,        ESG_SiteAddress,
                         GA_RelVerify, TRUE,
                         GA_TabCycle,  TRUE,
                         STRINGA_Buffer,     buf2,
                         STRINGA_MaxChars,   sizeof(buf2),//100,
                         STRINGA_MinVisible, 10,
                       TAG_DONE),
                       Label(GetAmiFTPString(SCW_Site)),

                       LAYOUT_AddChild, ESG_List[ESG_Port]=NewObject(IntegerClass, NULL,//IntegerObject,
                         GA_ID,        ESG_Port,
                         GA_RelVerify, TRUE,
                         GA_TabCycle,  TRUE,
                         INTEGER_Number,   sn->sn_Port,
                         //INTEGER_Arrows,   FALSE,
                         INTEGER_MaxChars, 5,
                         INTEGER_Minimum,  0,
                         INTEGER_Maximum,  65535,
                       TAG_DONE),
                       //CHILD_NominalSize, TRUE,
                       //CHILD_WeightedWidth, 0,
                       Label(GetAmiFTPString(SCW_Port)),
//EndGroup,

                       LAYOUT_AddChild, ESG_List[ESG_RemDir]=NewObject(StringClass, NULL,//StringObject,
                         GA_ID,        ESG_RemDir,
                         GA_RelVerify, TRUE,
                         GA_TabCycle,  TRUE,
                         STRINGA_Buffer,   buf3,
                         STRINGA_MaxChars, sizeof(buf3),//100,
                       TAG_DONE),
                       Label(GetAmiFTPString(SCW_RemDir)),

//                     LAYOUT_AddChild, l1=HLayoutObject, Spacing(FALSE),

                       LAYOUT_AddChild, l1=ESG_List[ESG_LocalDir]= NewObject(GetFileClass, NULL,//GetFileObject,
                         GA_ID,        MG_DLGetFile,
                         GA_RelVerify, TRUE,
                         GETFILE_DrawersOnly, TRUE,
                         GETFILE_TitleText,   GetAmiFTPString(Str_SelectDLPath),
                         GETFILE_Drawer,      buf4,
                         GETFILE_ReadOnly,    TRUE,
	                      TAG_DONE),
	                      CHILD_WeightedHeight, 0,
	                      Label(GetAmiFTPString(SCW_LocDir)),
/*
                       LAYOUT_AddChild, ESG_List[ESG_LocString]=StringObject,
                         GA_ID, ESG_LocString,
                         GA_RelVerify, TRUE,
                         GA_TabCycle, TRUE,
                         STRINGA_Buffer, buf4,
                         STRINGA_MaxChars, 50,
                       TAG_DONE),

                       LAYOUT_AddChild, ESG_List[ESG_LocGad]=ButtonObject,
                         BUTTON_AutoButton, BAG_POPDRAWER,
                         GA_ID, ESG_LocGad,
                         GA_RelVerify, TRUE,
                         TAG_DONE),
                         CHILD_WeightedWidth, 0,
                         CHILD_WeightedHeight, 0,
                       TAG_DONE),    
                       Label(GetAmiFTPString(SCW_LocDir)),
*/
                       LAYOUT_AddChild, ESG_List[ESG_Anonymous]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                         GA_ID,        ESG_Anonymous,
                         GA_RelVerify, TRUE,
                         GA_Text,      GetAmiFTPString(SCW_Anonymous),
                         GA_Selected,  sn->sn_Anonymous,
                         CHECKBOX_TextPlace, PLACETEXT_LEFT,
                       TAG_DONE),

                       LAYOUT_AddChild,ESG_List[ESG_LoginPsw]=NewObject(LayoutClass, NULL,//VLayoutObject,
                         LAYOUT_Orientation, LAYOUT_ORIENT_VERT,

                         LAYOUT_AddChild, ESG_List[ESG_LoginName]=NewObject(StringClass, NULL,//StringObject,
                           GA_ID,        ESG_LoginName,
                           GA_RelVerify, TRUE,
                           GA_Disabled,  sn->sn_Anonymous,
                           GA_TabCycle,  TRUE,
                           STRINGA_Buffer,   buf5,
                           STRINGA_MaxChars, sizeof(buf5),//20,
                           TAG_DONE),
                           Label(GetAmiFTPString(SCW_LoginName)),

                         LAYOUT_AddChild, ESG_List[ESG_Password]=NewObject(StringClass, NULL,//StringObject,
                           GA_ID,        ESG_Password,
                           GA_RelVerify, TRUE,
                           GA_Disabled,  sn->sn_Anonymous,
                           GA_TabCycle,  TRUE,
                           STRINGA_Buffer,   buf6,
                           STRINGA_MaxChars, sizeof(buf6),//90,
                           STRINGA_HookType, SHK_PASSWORD,
                           //STRINGA_EditHook, (ULONG)&EditHook,
                         TAG_DONE),
                         Label(GetAmiFTPString(SCW_Password)),
                       TAG_DONE),
                       CHILD_WeightedHeight, 0,
                       TAG_DONE),

                       LAYOUT_AddChild, l4=NewObject(LayoutClass, NULL,//VLayoutObject,
                         LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
                         LAYOUT_SpaceOuter,     TRUE,
                         LAYOUT_HorizAlignment, LALIGN_RIGHT,
                         LAYOUT_AddChild, ESG_List[ESG_LSType]=NewObject(ChooserClass, NULL,//ChooserObject,
                           GA_ID,        ESG_LSType,
                           GA_RelVerify, TRUE,
                           CHOOSER_Labels,  objlist,
                           //CHOOSER_AutoFit, TRUE,
                           CHOOSER_PopUp,   TRUE,
                           CHOOSER_Active,  sn->sn_VMSDIR,
                         TAG_DONE),
                         CHILD_WeightedWidth, 0,
                         Label(GetAmiFTPString(SCW_OS)),

                         LAYOUT_AddChild, l5=NewObject(LayoutClass, NULL,//HLayoutObject,
                           //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                           LAYOUT_InnerSpacing, FALSE,//Spacing(FALSE),
                           LAYOUT_AddChild, ESG_List[ESG_Proxy]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                             GA_ID,        ESG_Proxy,
                             GA_RelVerify, TRUE,
                             GA_Text,      GetAmiFTPString(SCW_Proxy),
                             GA_Selected,  sn->sn_Proxy,
                             CHECKBOX_TextPlace, PLACETEXT_LEFT,
                           TAG_DONE),
                           LAYOUT_AddChild, l6=NewObject(LayoutClass, NULL,//VLayoutObject,
                             LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                             LAYOUT_AddChild, ESG_List[ESG_HotList]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                               GA_ID,        ESG_HotList,
                               GA_RelVerify, TRUE,
                               GA_Text,      GetAmiFTPString(SCW_HotList),
                               GA_Selected,  sn->sn_HotList,
                               CHECKBOX_TextPlace, PLACETEXT_LEFT,
                             TAG_DONE),
                             LAYOUT_AddChild, ESG_List[ESG_ADT]=NewObject(CheckBoxClass, NULL,//CheckBoxObject,
                               GA_ID,        ESG_ADT,
                               GA_RelVerify, TRUE,
                               GA_Text,      GetAmiFTPString(SCW_AminetMode),
                               GA_Selected,  sn->sn_ADT,
                               CHECKBOX_TextPlace, PLACETEXT_LEFT,
                             TAG_DONE),
                           TAG_DONE),TAG_DONE),
                           CHILD_WeightedHeight, 0,
                           CHILD_WeightedWidth, 0,
                       TAG_DONE),
                       CHILD_WeightedHeight, 0,

                       /*LAYOUT_AddImage, BevelObject,
                         BEVEL_Style, BVS_SBAR_VERT,
                       BevelEnd,*/

                       LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartHGroup,
                         //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                         LAYOUT_EvenSize,   TRUE,//EvenSized,
                         LAYOUT_SpaceOuter, TRUE,
                         LAYOUT_BevelStyle, BVS_SBAR_VERT,

                         LAYOUT_AddChild, ESG_List[ESG_OK]=NewObject(ButtonClass, NULL,//ButtonObject,
                           GA_ID,        ESG_OK,
                           GA_RelVerify, TRUE,
                           GA_Text,      GetAmiFTPString(SCW_OK),
                         TAG_DONE),
                         //CHILD_NominalSize, TRUE,
                         CHILD_WeightedWidth, 0,

                         LAYOUT_AddChild, ESG_List[ESG_Cancel]=NewObject(ButtonClass, NULL,//ButtonObject,
                           GA_ID,        ESG_Cancel,
                           GA_RelVerify, TRUE,
                           GA_Text,      GetAmiFTPString(SCW_Cancel),
                         TAG_DONE),
                         //CHILD_NominalSize, TRUE,
                         CHILD_WeightedWidth, 0,
                       TAG_DONE),
                       CHILD_WeightedHeight, 0,
                       //CHILD_WeightMinimum, TRUE,
                   TAG_DONE);

    if (!EditSiteLayout)
      return NULL;

    if (sn->sn_MenuType==SLN_PARENT) {
	SetAttrs(l4, LAYOUT_RemoveChild, ESG_List[ESG_LSType], TAG_DONE);
	SetAttrs(l5, LAYOUT_RemoveChild, ESG_List[ESG_Proxy], TAG_DONE);
	SetAttrs(l6, LAYOUT_RemoveChild, ESG_List[ESG_ADT], TAG_DONE);
	SetAttrs(l3,
	         LAYOUT_RemoveChild, ESG_List[ESG_SiteAddress],
	         LAYOUT_RemoveChild, ESG_List[ESG_Port],
	         LAYOUT_RemoveChild, ESG_List[ESG_RemDir],
	         LAYOUT_RemoveChild, l1,
	         LAYOUT_RemoveChild, ESG_List[ESG_LoginPsw],//l2
	         LAYOUT_RemoveChild, ESG_List[ESG_Anonymous],
	        TAG_DONE);
    }

    //LayoutLimits((struct Gadget *)EditSiteLayout, &limits, PropFont, Screen);
    //limits.MinHeight+=Screen->WBorTop+Screen->WBorBottom;
    //limits.MinWidth+=Screen->WBorLeft+Screen->WBorRight;

    EditSiteWin_Object = NewObject(WindowClass, NULL,//WindowObject,
                          WA_Title,        sn->sn_MenuType==SLN_PARENT? GetAmiFTPString(SCW_GroupConfigurationWindow) : GetAmiFTPString(SCW_SiteConfigurationWindow),
                          WA_PubScreen,    Screen,
                          WA_DepthGadget,  TRUE,
                          WA_DragBar,      TRUE,
                          WA_CloseGadget,  TRUE,
                          WA_Activate,     TRUE,
                          WA_SmartRefresh, TRUE,
                          WA_SizeGadget,   TRUE,
                          //WA_Top, MainWindow->TopEdge+(MainWindow->Height-limits.MinHeight)/2,
                          //WA_Left, MainWindow->LeftEdge+(MainWindow->Width-limits.MinWidth)/2,
                          WA_InnerWidth, PropFont->tf_XSize*30,
                          WA_IDCMP, IDCMP_RAWKEY,
                          WINDOW_Position,  WPOS_CENTERWINDOW,
                          WINDOW_RefWindow, SiteListWindow,
                          WINDOW_Layout, EditSiteLayout,
                          WINDOW_InterpretUserData, WGUD_FUNC,
                          WINDOW_LockHeight, TRUE,
                        TAG_DONE);

    if (!EditSiteWin_Object)
      return NULL;

    //if (EditSiteWindow=CA_OpenWindow(EditSiteWin_Object)) {
    if ( (EditSiteWindow=(struct Window *)IDoMethod(EditSiteWin_Object, WM_OPEN)) ) {
	return EditSiteWindow;
    }
    DisposeObject(EditSiteLayout);
    EditSiteLayout=NULL;

    return NULL;
}

void CloseEditSiteWindow(struct SiteNode *sn)
{
	if (EditSiteWin_Object) {
		if (AddToSiteList) {
			ULONG attr;

DBUG("1)'%s' (%s)\n",buf1,sn->sn_Node.ln_Name);
			if (sn->sn_Node.ln_Name) {
					free(sn->sn_Node.ln_Name);
					sn->sn_Node.ln_Name=NULL;
DBUG("free(sn->sn_Node.ln_Name)\n",NULL);
			}
			if (strlen(buf1)) sn->sn_Node.ln_Name=strdup(buf1);
//			if(sn->sn_Node.ln_Name) { FreeString(&sn->sn_Node.ln_Name); }
//			sn->sn_Node.ln_Name = DupStr(buf1, -1);
DBUG("2)'%s' (%s)\n",buf1,sn->sn_Node.ln_Name);

			if (sn->sn_MenuType!=SLN_PARENT) {
				if (sn->sn_SiteAddress) {
					free(sn->sn_SiteAddress);
					sn->sn_SiteAddress=NULL;
				}
				if (strlen(buf2)) sn->sn_SiteAddress=strdup(buf2);

				if (sn->sn_RemoteDir) {
					free(sn->sn_RemoteDir);
					sn->sn_RemoteDir=NULL;
				}
				if (strlen(buf3)) sn->sn_RemoteDir=strdup(buf3);

				if (sn->sn_LocalDir) {
					free(sn->sn_LocalDir);
					sn->sn_LocalDir=NULL;
				}
				if (strlen(buf4)) sn->sn_LocalDir=strdup(buf4);

				if (sn->sn_LoginName) {
					free(sn->sn_LoginName);
					sn->sn_LoginName=NULL;
				}
				if (strlen(buf5)) sn->sn_LoginName=strdup(buf5);

				if (sn->sn_Password) {
					free(sn->sn_Password);
					sn->sn_Password=NULL;
				}
				if (strlen(buf6)) sn->sn_Password = strdup(buf6);

				GetAttr(GA_Selected, ESG_List[ESG_Anonymous], &attr);
				sn->sn_Anonymous=attr;

				GetAttr(INTEGER_Number, ESG_List[ESG_Port], &attr);
				sn->sn_Port=attr;

				GetAttr(GA_Selected, ESG_List[ESG_Proxy], &attr);
				sn->sn_Proxy=attr;

				GetAttr(GA_Selected, ESG_List[ESG_ADT], &attr);
				sn->sn_ADT=attr;

				GetAttr(CHOOSER_Active, ESG_List[ESG_LSType], &attr);
				sn->sn_VMSDIR=attr;
			}

			GetAttr(GA_Selected, ESG_List[ESG_HotList], &attr);
			if (sn->sn_HotList!=attr) HotListChanged=TRUE;

			sn->sn_HotList=attr;
		}

		DisposeObject(EditSiteWin_Object);
		EditSiteWindow=NULL;
		EditSiteWin_Object=NULL;
		EditSiteLayout=NULL;
	}

	FreeChooserLabels(objlist);
	objlist=NULL;
}

int LocPath_clicked(void)
{
	struct gfileRequest pathList = {GFILE_REQUEST, EditSiteWindow};
	uint32 result = IDoMethodA(ESG_List[ESG_LocalDir], (struct _Msg *)&pathList);
	if (result)
	{
		char *strBuffer = NULL;

		GetAttr(GETFILE_Drawer, ESG_List[ESG_LocalDir], (ULONG*)&strBuffer);
		if (strBuffer)
		{
			if (Strlen(strBuffer) > 0)
			{
				Strlcpy(buf4, strBuffer, sizeof(buf4));//-1);
			}
			else
			{
				Strlcpy(buf4, "RAM:", sizeof(buf4));//-1);
				SetAttrs(MG_List[MG_DLGetFile], GETFILE_FullFile,"RAM:", TAG_DONE);
			}
		}
	}

	return 1;
}

static struct SiteNode *retnode;
static BOOL ConnectMode;
static ULONG lsel;
static WORD width=0,height=0;
//static struct SiteNode *lastnode;

struct SiteNode *OpenSiteListWindow(const BOOL Connect)
{
	ULONG signal,wait,/*mainsignal,*/done=FALSE;

	retnode=NULL;
	ConnectMode=Connect;

	if (!OpenSiteWindow(Connect)) return NULL;

	HotListChanged=FALSE;

	GetAttr(WINDOW_SigMask, SiteListWin_Object, &signal);
	//GetAttr(WINDOW_SigMask, MainWin_Object, &mainsignal);

	lsel=-1;

	while (done==0) {
		wait=Wait(signal|AG_Signal|SIGBREAKF_CTRL_C);

		if (SIGBREAKF_CTRL_C) done=TRUE;
		if (wait & AG_Signal ) HandleAmigaGuide();
		if (wait & signal    ) done=HandleSiteListIDCMP();
		//if (wait & mainsignal) HandleMainWindowIDCMP(FALSE);
	}

	CloseSiteListWindow();
	if (HotListChanged) UpdateMenus();

	return retnode;
}

/*static int SortSiteNodes(const void *a,const void *b)
{
    const struct Node *nodea=(struct Node *)*(ULONG *)a;
    const struct Node *nodeb=(struct Node *)*(ULONG *)b;

    return stricmp((char *)nodea->ln_Name, (char *)nodeb->ln_Name);
}*/

ULONG HandleSiteListIDCMP(void)
{
    ULONG result, done=FALSE;
    struct wmHandle wcode={0};
    int16 code;

    //while((result=CA_HandleInput(SiteListWin_Object, &code))!=WMHI_LASTMSG) {
    while ((result=IDoMethod(SiteListWin_Object, WM_HANDLEINPUT, &wcode))!=WMHI_LASTMSG) {

     code = result&WMHI_KEYMASK;
	switch (result & WMHI_CLASSMASK) {
	  case WMHI_CLOSEWINDOW:
	    done=TRUE;
	    break;
	  case WMHI_RAWKEY:
	    if (code==69)
		done=TRUE;
	    else if (code==RAWKEY_CURSORUP) {
		if (GetHead(&SiteList)!=NULL) {
		    LONG attr;
		    struct Node *selnode;
		    //struct SiteNode *sn;

		    GetAttrs(SLG_List[SLG_SiteList],
			     LISTBROWSER_Selected, &attr,
			     LISTBROWSER_SelectedNode, &selnode,
			     TAG_DONE);
		    if (attr>0) { // Kolla på attr==-1, annars blir det en hit  ¿:-/
			struct SiteNode *sn;
			struct Node *node;
			ULONG i;

			if ( (node=GetPred(selnode)) ) {
			    ULONG flags;
			    GetListBrowserNodeAttrs(node, LBNA_Flags,&flags, TAG_DONE);
			    while (flags&LBFLG_HIDDEN) {
				node=GetPred(node);
				if (node)
				  GetListBrowserNodeAttrs(node, LBNA_Flags,&flags, TAG_DONE);
				else
				  flags=0UL;
			    }
			    if (node)
			      selnode=node;
			}
			else selnode=GetHead(&SiteList);

			for (i=0,node=GetHead(&SiteList);node;node=GetSucc(node),i++)
			  if (selnode==node) break;

			SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
				       LISTBROWSER_Selected, i,
				       LISTBROWSER_MakeVisible, i,
				       TAG_DONE);

			GetListBrowserNodeAttrs(selnode, LBNA_UserData,&sn, TAG_DONE);

			UpdateSLGGadgets(TRUE, sn->sn_MenuType);
		    }
		    else if (attr == -1) {
			struct SiteNode *sn;
			struct Node *selnode;

			selnode=GetHead(&SiteList);
			GetListBrowserNodeAttrs(selnode, LBNA_UserData,&sn, TAG_DONE);
			SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
				       LISTBROWSER_Selected, 0,
				       LISTBROWSER_MakeVisible, 0,
				       TAG_DONE);
			UpdateSLGGadgets(TRUE, sn->sn_MenuType);
		    }
		}
	    }
	    else if (code==RAWKEY_CURSORDOWN) {
		if (GetHead(&SiteList)!=NULL) {
		    ULONG attr;
		    struct Node *selnode;
		    struct SiteNode *sn;
		    ULONG i;
		    struct Node *node;

		    GetAttrs(SLG_List[SLG_SiteList],
			     LISTBROWSER_Selected, &attr,
			     LISTBROWSER_SelectedNode, &selnode,
			     TAG_DONE);
		    if (selnode) {
			if ( (node=GetSucc(selnode)) ) {
			    ULONG flags;
			    GetListBrowserNodeAttrs(node, LBNA_Flags,&flags, TAG_DONE);
			    while (flags&LBFLG_HIDDEN) {
				node=GetSucc(node);
				if (node)
				  GetListBrowserNodeAttrs(node, LBNA_Flags,&flags, TAG_DONE);
				else
				  flags=0UL;
			    }
			    if (node)
			      selnode=node;
			}
		    }
		    else selnode=GetHead(&SiteList);
		    for (i=0,node=GetHead(&SiteList);node;node=GetSucc(node),i++)
		      if (selnode==node) break;
		    SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
				   LISTBROWSER_Selected, i,
				   LISTBROWSER_MakeVisible, i,
				   TAG_DONE);
		    GetListBrowserNodeAttrs(selnode, LBNA_UserData,&sn, TAG_DONE);
		    UpdateSLGGadgets(TRUE, sn->sn_MenuType);
		}
	    }
	    else if (code==95)
		SendAGMessage(AG_SITECONF);
	    else if (code==68) {
		struct Node *node;

		GetAttrs(SLG_List[SLG_SiteList], LISTBROWSER_SelectedNode,&node, TAG_DONE);
		if (node) {
		    GetListBrowserNodeAttrs(node, LBNA_UserData,&retnode, TAG_DONE);
		    if (!retnode->sn_BarLabel && retnode->sn_MenuType!=SLN_PARENT)
		      done=TRUE;
		    else {
			done=FALSE;
			retnode=NULL;
		    }
		}
		else retnode=FALSE;
	    }
	    break;
	  case WMHI_GADGETUP:
        code = result&WMHI_GADGETMASK;
	    switch (result & WMHI_GADGETMASK) {
	      case SLG_SiteList:
		{
		    ULONG action;
		    struct Node *node;

		    GetAttrs(SLG_List[SLG_SiteList], LISTBROWSER_RelEvent,&action, TAG_DONE);
		    if (action&LBRE_DOUBLECLICK && lsel==code) {
			GetAttrs(SLG_List[SLG_SiteList], LISTBROWSER_SelectedNode,&node, TAG_DONE);
			GetListBrowserNodeAttrs(node, LBNA_UserData,&retnode, TAG_DONE);
			if (!retnode->sn_BarLabel) {
			    if (ConnectMode) {
				if (retnode->sn_MenuType!=SLN_PARENT)
				  done=TRUE;
				else {
				    done=FALSE;
				    retnode=NULL;
				}
			    }
			    else {
				EditClicked();
			    }
			} else retnode=NULL;
		    }
		    else if (action&LBRE_NORMAL) {
			GetAttrs(SLG_List[SLG_SiteList], LISTBROWSER_SelectedNode,&node, TAG_DONE);
			if (node) {
			    struct SiteNode *sn;
			    GetListBrowserNodeAttrs(node, LBNA_UserData,&sn, TAG_DONE);
			    UpdateSLGGadgets(TRUE, sn->sn_BarLabel? SLN_BARLABEL : sn->sn_MenuType);
			}
			else
			  UpdateSLGGadgets(FALSE, 0);
		    }
		    lsel=code;
		}
		break;
	      case SLG_Top:
		SL_Top();
		break;
	      case SLG_Bottom:
		SL_Bottom();
		break;
	      case SLG_Up:
		SL_Up();
		break;
	      case SLG_Down:
		SL_Down();
		break;
	      case SLG_Edit:
		EditClicked();
		break;
	      case SLG_New:
		NewClicked();
		break;
	      case SLG_AddGroup:
		NewGroup();
		break;
	      case SLG_BarLabel:
		{
		    struct SiteNode *sn;
		    struct Node *lb,*lbn;
		    ULONG i;

		    sn=(struct SiteNode *)AllocVecTags(sizeof (struct SiteNode), AVT_Type, MEMF_SHARED, AVT_ClearWithValue, 0, TAG_DONE);
		    sn->sn_BarLabel=1;
		    sn->sn_HotList=1;
		    sn->sn_MenuType=SLN_REGULAR;
		    sn->sn_Node.ln_Name=strdup("===============");
//		    sn->sn_Node.ln_Name=DupStr("===============",15);

		    SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
		                    LISTBROWSER_Labels,~0, TAG_DONE);
		    lbn=AddLBNTail(&SiteList, sn);
		    for (i=0,lb=GetHead(&SiteList); lb; i++,lb=GetSucc(lb)) {
		      if (lb==lbn) break;
		    }
		    SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
				   LISTBROWSER_Labels, &SiteList,
				   LISTBROWSER_Selected, i,
				   LISTBROWSER_MakeVisible, i,
				   TAG_DONE);
		    ConfigChanged=TRUE;
		    HotListChanged=TRUE;
		    UpdateSLGGadgets(TRUE,sn->sn_MenuType);
		}
		break;
	      case SLG_Connect:
		{
		    struct Node *node;
		    GetAttr(LISTBROWSER_SelectedNode, SLG_List[SLG_SiteList], (ULONG *)&node);
		    GetListBrowserNodeAttrs(node, LBNA_UserData,&retnode, TAG_DONE);
		    if (retnode->sn_MenuType!=SLN_PARENT && !retnode->sn_BarLabel)
		      done=TRUE;
		    else {
			done=FALSE;
			retnode=NULL;
		    }
		    break;
		}
	      case SLG_Cancel:
		done=TRUE;
		retnode=NULL;
		break;
	    }
	    break;
	}
    }
    return done;
}


struct Window *OpenSiteWindow(const BOOL Connect)
{
    //Object *l;
    //struct LayoutLimits limits;
    Object *l1;

    if (SiteListWindow)
      return SiteListWindow;

    SiteListLayout = NewObject(LayoutClass, NULL,//LayoutObject,
                       GA_DrawInfo, DrawInfo,
                       GA_TextAttr, AmiFTPAttrF,
                       LAYOUT_DeferLayout, FALSE,
                       LAYOUT_SpaceOuter, TRUE,
                       LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                       LAYOUT_HorizAlignment, LALIGN_CENTRE,
                       LAYOUT_Label, GetAmiFTPString(SLW_SiteList),

                       /*LAYOUT_AddImage, l=NewObject(LabelClass, NULL,//LabelObject,
                         LABEL_Justification, LABEL_CENTRE,
                         LABEL_Text, GetAmiFTPString(SLW_SiteList),
                         TAG_DONE),
                         CHILD_MinWidth, ((struct Image*)l)->Width,
                         CHILD_MinHeight, ((struct Image*)l)->Height,
                         CHILD_WeightedWidth, 0,
                         CHILD_WeightedHeight, 0,*/

                       LAYOUT_AddChild, NewObject(LayoutClass, NULL,//StartHGroup, // 1
                         LAYOUT_AddChild,NewObject(LayoutClass, NULL,//StartVGroup, // 2
                         LAYOUT_Orientation,LAYOUT_ORIENT_VERT,

                         LAYOUT_AddChild,NewObject(LayoutClass, NULL,//StartVGroup,
                         LAYOUT_Orientation,LAYOUT_ORIENT_VERT,
                         //LAYOUT_BackFill,LAYERS_BACKFILL,
                           LAYOUT_AddChild, SLG_List[SLG_SiteList]=NewObject(ListBrowserClass, NULL,//ListBrowserObject,
                           GA_ID,        SLG_SiteList,
                           GA_RelVerify, TRUE,
                           LISTBROWSER_Labels,       (ULONG)&SiteList,
                           LISTBROWSER_ShowSelected, TRUE,
                           LISTBROWSER_MinVisible,   9,
                           LISTBROWSER_Hierarchical, TRUE,
                           TAG_DONE),
//                           CHILD_MinWidth, PropFont->tf_XSize*30,
                           //CHILD_MinHeight, PropFont->tf_YSize*9,
                          TAG_DONE), // Classact bugfix
                         LAYOUT_AddChild,NewObject(LayoutClass, NULL,////StartVGroup,
                         LAYOUT_Orientation,LAYOUT_ORIENT_VERT,
                         LAYOUT_EvenSize, TRUE,//EvenSized, // 3

                           LAYOUT_AddChild,NewObject(LayoutClass, NULL,//StartHGroup,
                           LAYOUT_EvenSize, TRUE,//EvenSized, // 4

                             LAYOUT_AddChild, SLG_List[SLG_New]=NewObject(ButtonClass, NULL,//ButtonObject,
                               GA_ID,        SLG_New,
                               GA_RelVerify, TRUE,
                               GA_Text,      GetAmiFTPString(SLW_New),
//                               GA_UserData, (ULONG)NewClicked,
                               TAG_DONE),

			     LAYOUT_AddChild, SLG_List[SLG_AddGroup]=NewObject(ButtonClass, NULL,//ButtonObject,
			      GA_ID,        SLG_AddGroup,
			      GA_RelVerify, TRUE,
			      GA_Text,      GetAmiFTPString(SLW_NewGroup),
			     TAG_DONE),

                             LAYOUT_AddChild, SLG_List[SLG_Edit]=NewObject(ButtonClass, NULL,//ButtonObject,
                               GA_ID,        SLG_Edit,
                               GA_RelVerify, TRUE,
                               GA_Text,      GetAmiFTPString(SLW_Edit),
                               GA_Disabled,  TRUE,
                               //GA_UserData, (ULONG)EditClicked,
                               TAG_DONE),

                             LAYOUT_AddChild, SLG_List[SLG_Remove]=NewObject(ButtonClass, NULL,//ButtonObject,
                               GA_ID,        SLG_Remove,
                               GA_RelVerify, TRUE,
                               GA_Text,      GetAmiFTPString(SLW_Remove),
                               GA_Disabled,  TRUE,
                               GA_UserData, (ULONG)RemoveClicked,
                               TAG_DONE),
			       //CHILD_NominalSize, TRUE,
                             TAG_DONE), /* End of New/edit/remove-group */ // -4

                           LAYOUT_AddChild, l1=NewObject(LayoutClass, NULL,//HLayoutObject,
                           LAYOUT_EvenSize, TRUE,//EvenSized, // 5

                             LAYOUT_AddChild, SLG_List[SLG_Connect]=NewObject(ButtonClass, NULL,//ButtonObject,
                               GA_ID, SLG_Connect,
                               GA_RelVerify, TRUE,
                               GA_Text, GetAmiFTPString(SLW_Connect),
			       GA_Disabled, TRUE,
                               TAG_DONE),

                             LAYOUT_AddChild, SLG_List[SLG_Cancel]=NewObject(ButtonClass, NULL,//ButtonObject,
                               GA_ID, SLG_Cancel,
                               GA_RelVerify, TRUE,
                               GA_Text, Connect? GetAmiFTPString(SLW_Cancel) : GetAmiFTPString(SLW_OK),
                               TAG_DONE),
			       CHILD_NominalSize, TRUE,
                             TAG_DONE), /* End of connect/disconnect-group */ //-5
                             CHILD_WeightMinimum, TRUE,
                           TAG_DONE), /* End of two row button-group */ //-3
                           CHILD_WeightedHeight, 0,
                         TAG_DONE), /* End of listlist/button group */ //-2

                           LAYOUT_AddChild,NewObject(LayoutClass, NULL,//StartVGroup,
                           LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                           LAYOUT_EvenSize, TRUE,//EvenSized,
                             LAYOUT_AddChild, SLG_List[SLG_Top]=NewObject(ButtonClass, NULL,//ButtonObject,
                              GA_Text, GetAmiFTPString(SLW_Top),
                              GA_ID, SLG_Top,
                              GA_RelVerify, TRUE,
			      GA_Disabled, TRUE,
                             TAG_DONE),

                             LAYOUT_AddChild, SLG_List[SLG_Up]=NewObject(ButtonClass, NULL,//ButtonObject,
                              GA_Text, GetAmiFTPString(SLW_Up),
                              GA_ID, SLG_Up,
                              GA_RelVerify, TRUE,
			      GA_Disabled, TRUE,
                             TAG_DONE),

                             LAYOUT_AddChild, SLG_List[SLG_Down]=NewObject(ButtonClass, NULL,//ButtonObject,
                              GA_Text, GetAmiFTPString(SLW_Down),
                              GA_ID, SLG_Down,
                              GA_RelVerify, TRUE,
			      GA_Disabled, TRUE,
                             TAG_DONE),

                             LAYOUT_AddChild, SLG_List[SLG_Bottom]=NewObject(ButtonClass, NULL,//ButtonObject,
                              GA_Text, GetAmiFTPString(SLW_Bottom),
                              GA_ID, SLG_Bottom,
                              GA_RelVerify, TRUE,
			      GA_Disabled, TRUE,
                             TAG_DONE),

                             LAYOUT_AddChild, SLG_List[SLG_BarLabel]=NewObject(ButtonClass, NULL,//ButtonObject,
                              GA_Text, GetAmiFTPString(SLW_AddBar),
                              GA_ID, SLG_BarLabel,
                              GA_RelVerify, TRUE,
                             TAG_DONE),
			     CHILD_NominalSize, TRUE,

                           TAG_DONE), /* End of Button-group */
                           CHILD_WeightedWidth, 0,
                           CHILD_WeightedHeight, 0,
			   TAG_DONE),
		       TAG_DONE); /* End of Layoutobject */

    if (!SiteListLayout)
      return NULL;

    if (!Connect) {
	SetAttrs(l1, LAYOUT_RemoveChild,SLG_List[SLG_Connect], TAG_DONE);	
	SLG_List[SLG_Connect]=NULL;
	SLG_List[SLG_Cancel]=NULL;
    }

    /*LayoutLimits((struct Gadget *)SiteListLayout, &limits, PropFont, Screen);
    limits.MinHeight+=Screen->WBorTop+Screen->WBorBottom;
    limits.MinWidth+=Screen->WBorLeft+Screen->WBorRight;*/

    SiteListWin_Object = NewObject(WindowClass, NULL,//WindowObject,
                           WA_Title,        GetAmiFTPString(SLW_SiteListWindow),
                           WA_PubScreen,    Screen,
                           WA_SizeGadget,   TRUE,
                           WA_SizeBBottom,  TRUE,
                           WA_DepthGadget,  TRUE,
                           WA_DragBar,      TRUE,
                           WA_CloseGadget,  TRUE,
                           WA_Activate,     TRUE,
                           WA_SmartRefresh, TRUE,
                           //WA_Top, MainWindow->TopEdge+(MainWindow->Height-limits.MinHeight)/2,
                           //WA_Left, MainWindow->LeftEdge+(MainWindow->Width-limits.MinWidth)/2,
//                           WA_InnerHeight, height? height: limits.MinHeight,
//                           WA_InnerWidth, width? width: limits.MinWidth,
                           WA_IDCMP, IDCMP_RAWKEY,
                           WINDOW_Position,  WPOS_CENTERWINDOW,
                           WINDOW_RefWindow, MainWindow,
                           WINDOW_Layout,    SiteListLayout,
                           WINDOW_GadgetUserData, WGUD_FUNC,
                         TAG_DONE);

    if (!SiteListWin_Object) {
	DisposeObject(SiteListLayout);
	return NULL;
    }

    //if (SiteListWindow=CA_OpenWindow(SiteListWin_Object)) {
    if ( (SiteListWindow=(struct Window *)IDoMethod(SiteListWin_Object, WM_OPEN)) ) {
	return SiteListWindow;
    }
    DisposeObject(SiteListWin_Object);
    SiteListWin_Object=NULL;
    SiteListLayout=NULL;

    return NULL;
}

void CloseSiteListWindow(void)
{
	if (SiteListWin_Object) {
		height=SiteListWindow->Height-SiteListWindow->BorderBottom-SiteListWindow->BorderTop;
		width=SiteListWindow->Width-SiteListWindow->BorderLeft-SiteListWindow->BorderRight;

		DisposeObject(SiteListWin_Object);
		SiteListLayout=NULL;
		SiteListWin_Object=NULL;
		SiteListWindow=NULL;
	}
}

int NewGroup(void)
{
	struct SiteNode *sn;

	sn=(struct SiteNode *)AllocVecTags(sizeof (struct SiteNode), AVT_Type,MEMF_SHARED, AVT_ClearWithValue,0, TAG_DONE);
	if (sn) {
		struct Node *lb,*lbn;
		ULONG i;

		sn->sn_Node.ln_Name=strdup(GetAmiFTPString(SLW_Newstring));
//		sn->sn_Node.ln_Name=DupStr(GetAmiFTPString(SLW_Newstring),-1);
		sn->sn_MenuType=SLN_PARENT;
		sn->sn_HotList=TRUE;

		LockWindow(SiteListWin_Object);
		//APTR lw = SleepWindow(SiteListWindow);
		OpenEditWindow(sn);
		UnlockWindow(SiteListWin_Object);
		//WakeWindow(SiteListWindow,lw);

		if (AddToSiteList) {
			SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
			               LISTBROWSER_Labels,~0, TAG_DONE);

			lbn=AddLBNTail(&SiteList, sn);
			for (i=0,lb=GetHead(&SiteList); lb; i++,lb=GetSucc(lb))
				if (lb==lbn) break;

			SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
			               LISTBROWSER_Labels, &SiteList,
			               LISTBROWSER_Selected, i,
			               LISTBROWSER_MakeVisible, i,
			              TAG_DONE);

			ConfigChanged=TRUE;
			HotListChanged=TRUE;
			UpdateSLGGadgets(TRUE, sn->sn_MenuType);
			RefreshGList((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL, 1);
		}
		else FreeVec(sn);
	}

	return 1;
}

void SL_Up(void)
{
	ULONG prevgen=SLN_REGULAR, hidden, /*haschildren,*/ nodegen;
	struct Node *node, *prevnode;
	struct SiteNode *sn;
DBUG("SL_Up2():\n",NULL);
	GetAttrs(SLG_List[SLG_SiteList], LISTBROWSER_SelectedNode,&node, TAG_DONE);
	if(node == NULL) { return; }

	GetListBrowserNodeAttrs(node, //LBNA_Flags,&haschildren,
	                        LBNA_Generation,&nodegen, LBNA_UserData,&sn, TAG_DONE);
DBUG("  LBNA_Generation=0x%08lx\n",nodegen);

	// CASE: don't allow move SLN_PARENT
	if(nodegen == SLN_PARENT) { return; }

	// CASE: we are on top -> do nothing
	if( (prevnode=GetPred(node)) == NULL ) { return; }

//	if( (prevnode=GetPred(node)) ) {
	GetListBrowserNodeAttrs(prevnode, LBNA_Flags,&hidden, LBNA_Generation,&prevgen, TAG_DONE);
//	}

	// CASE: previous is SLN_CHILD -> change to "child"
	if(prevgen==SLN_CHILD  &&  nodegen==SLN_REGULAR) {
		sn->sn_MenuType = SLN_CHILD;
		SetListBrowserNodeAttrs(node, LBNA_Generation,SLN_CHILD, TAG_DONE);
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
		               LISTBROWSER_Labels,&SiteList);
		return;
	}

	// CASE: node is first child -> change to "regular" & swap
	if(nodegen==SLN_CHILD  &&  prevgen!=SLN_CHILD) {
		sn->sn_MenuType = SLN_REGULAR;
		SetListBrowserNodeAttrs(node, LBNA_Generation,SLN_REGULAR, TAG_DONE);
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
		               LISTBROWSER_Labels,&SiteList);
		swapEntries( node, GetPred(prevnode) ); // GetPred(prevnode) -> Insert AFTER a given node
		return;
	}

	// CASE: previous is SLN_PARENT -> change to "regular"
	if(prevgen == SLN_PARENT) {
		sn->sn_MenuType = SLN_REGULAR;
		if( !(hidden&LBFLG_SHOWCHILDREN) ) { // SLN_PARENT is hidden -> expand it
			DoGadgetMethod((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
			               LBM_SHOWCHILDREN, NULL, prevnode, 1);
		}
	}

	swapEntries( node, GetPred(prevnode) ); // GetPred(prevnode) -> Insert AFTER a given node
}

void SL_Down(void)
{
	ULONG nextgen=SLN_REGULAR, hidden, /*haschildren,*/ nodegen;
	struct Node *node, *nextnode = NULL;
	struct SiteNode *sn;
DBUG("SL_Down2():\n",NULL);
	GetAttrs(SLG_List[SLG_SiteList], LISTBROWSER_SelectedNode,&node, TAG_DONE);
	if(node == NULL) { return; }

	GetListBrowserNodeAttrs(node, //LBNA_Flags,&haschildren,
	                        LBNA_Generation,&nodegen, LBNA_UserData,&sn, TAG_DONE);
DBUG("  LBNA_Generation=0x%08lx\n",nodegen);
//DBUG("  LBNA_Flags=0x%08lx (LBNA_Generation=0x%08lx)\n",haschildren,nodegen);

	// CASE: don't allow move SLN_PARENT
	if(nodegen == SLN_PARENT) { return; }

	if( (nextnode=GetSucc(node)) ) {
		GetListBrowserNodeAttrs(nextnode, LBNA_Flags,&hidden, LBNA_Generation,&nextgen, TAG_DONE);
	}
DBUG("nextgen=0x%08lx\n",nextgen);

	// CASE: node is last child -> change to "regular"
	if(nodegen==SLN_CHILD  &&  nextgen!=SLN_CHILD) {
		sn->sn_MenuType = SLN_REGULAR;
		SetListBrowserNodeAttrs(node, LBNA_Generation,SLN_REGULAR, TAG_DONE);
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
		               LISTBROWSER_Labels,&SiteList);
		return;
	}

	// CASE: we are at bottom -> do nothing
	if(nextnode == NULL) { return; }

	// CASE: next node is SLN_PARENT -> change to "child"
	if(nextgen == SLN_PARENT) {
		sn->sn_MenuType = SLN_CHILD;
		if( !(hidden&LBFLG_SHOWCHILDREN) ) { // SLN_PARENT is hidden -> expand it
			DoGadgetMethod((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
			               LBM_SHOWCHILDREN, NULL, nextnode, 1);
		}
	}

	swapEntries(node, nextnode);
}

void SL_Top(void)
{
	ULONG thisgen;
	struct Node *thisnod;
	struct SiteNode *sn;
DBUG("SL_Top():\n",NULL);
	GetAttrs(SLG_List[SLG_SiteList], LISTBROWSER_SelectedNode,&thisnod, TAG_DONE);
	if(thisnod == NULL) { return; }

	GetListBrowserNodeAttrs(thisnod, LBNA_Generation,&thisgen, LBNA_UserData,&sn, TAG_DONE);
DBUG("  LBNA_Generation=0x%08lx\n",thisgen);

	// CASE: don't allow move SLN_PARENT
	if(thisgen == SLN_PARENT) { return; }

	// CASE: we are on top -> do nothing
	if(GetPred(thisnod) == NULL) { return; }

	// CASE: node is child -> change to "regular"
	if(thisgen == SLN_CHILD) {
		sn->sn_MenuType = SLN_REGULAR;
		SetListBrowserNodeAttrs(thisnod, LBNA_Generation,SLN_REGULAR, TAG_DONE);
	}

	Remove(thisnod);
	AddHead(&SiteList, thisnod);

	// Select top position of "moved" node
	SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
	               LISTBROWSER_Labels,&SiteList, LISTBROWSER_SelectedNode,thisnod,
	               LISTBROWSER_MakeNodeVisible,thisnod, TAG_DONE);

	HotListChanged = TRUE;
	ConfigChanged  = TRUE;
}

void SL_Bottom(void)
{
	ULONG thisgen;
	struct Node *thisnod;
	struct SiteNode *sn;
DBUG("SL_Bottom():\n",NULL);
	GetAttrs(SLG_List[SLG_SiteList], LISTBROWSER_SelectedNode,&thisnod, TAG_DONE);
	if(thisnod == NULL) { return; }

	GetListBrowserNodeAttrs(thisnod, LBNA_Generation,&thisgen, LBNA_UserData,&sn, TAG_DONE);
DBUG("  LBNA_Generation=0x%08lx\n",thisgen);

	// CASE: don't allow move SLN_PARENT
	if(thisgen == SLN_PARENT) { return; }

	// CASE: we are at bottom -> do nothing
	if(GetSucc(thisnod) == NULL) { return; }

	// CASE: node is child -> change to "regular"
	if(thisgen == SLN_CHILD) {
		sn->sn_MenuType = SLN_REGULAR;
		SetListBrowserNodeAttrs(thisnod, LBNA_Generation,SLN_REGULAR, TAG_DONE);
	}

	Remove(thisnod);
	AddTail(&SiteList, thisnod);

	// Select top position of "moved" node
	SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
	               LISTBROWSER_Labels,&SiteList, LISTBROWSER_SelectedNode,thisnod,
	               LISTBROWSER_MakeNodeVisible,thisnod, TAG_DONE);

	HotListChanged = TRUE;
	ConfigChanged  = TRUE;
}

int NewClicked(void)
{
	struct SiteNode *sn;
	struct Node *lbn;

    //geta4();

	sn=(struct SiteNode *)AllocVecTags(sizeof (struct SiteNode), AVT_Type,MEMF_SHARED, AVT_ClearWithValue,0, TAG_DONE);
	if (sn) {
		sn->sn_Node.ln_Name = strdup(GetAmiFTPString(SLW_Newstring));
//		sn->sn_Node.ln_Name = DupStr(GetAmiFTPString(SLW_Newstring),-1);
		sn->sn_Port         = 21;
		sn->sn_Proxy        = MainPrefs.mp_DefaultProxy;
		sn->sn_Anonymous    = 1;
		sn->sn_MenuType     = SLN_REGULAR;

		LockWindow(SiteListWin_Object);
		//APTR lw = SleepWindow(SiteListWindow);
		OpenEditWindow(sn);
		UnlockWindow(SiteListWin_Object);
		//WakeWindow(SiteListWindow,lw);

		if (AddToSiteList) {
			long i;
			struct Node *lb;

			SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
			               LISTBROWSER_Labels,~0, TAG_DONE);

			lbn=AddLBNTail(&SiteList,sn);
			for (i=0,lb=GetHead(&SiteList);lb;i++,lb=GetSucc(lb))
				if (lb==lbn) break;

			SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
			               LISTBROWSER_Labels, &SiteList,
			               LISTBROWSER_Selected, i,
			               LISTBROWSER_MakeVisible, i,
			              TAG_DONE);

			ConfigChanged=TRUE;
			HotListChanged=TRUE;
			UpdateSLGGadgets(TRUE, sn->sn_MenuType);
			RefreshGList((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL, 1);
		}
		else FreeVec(sn);
	}

	return 1;
}


int EditClicked(void)
{
	struct Node *lbn;
	struct SiteNode *sn;

    //geta4();

	GetAttr(LISTBROWSER_SelectedNode, SLG_List[SLG_SiteList], (ULONG *)&lbn);
	if (lbn) {
		GetListBrowserNodeAttrs(lbn, LBNA_UserData,&sn, TAG_DONE);
		if (sn) {
			if (!sn->sn_BarLabel) {
				/* fix this: (sn) should be a copy of sn */
				LockWindow(SiteListWin_Object);
				//APTR lw = SleepWindow(SiteListWindow);
				OpenEditWindow(sn);

				if (AddToSiteList) {
					LBEditNode(SLG_List[SLG_SiteList], SiteListWindow, NULL, lbn,
					           LBNA_Column, 0,
					             LBNCA_CopyText, TRUE,
					             LBNCA_Text,     sn->sn_Node.ln_Name,
					          TAG_DONE);
					if (!sn->sn_HotList) {
						struct Node *lbnode=GetSucc(lbn);
						if (lbnode) {
							GetListBrowserNodeAttrs(lbnode, LBNA_UserData,&sn, TAG_DONE);
							while (sn->sn_MenuType==SLN_CHILD) {
								sn->sn_HotList=FALSE;
								if ( (lbnode=GetSucc(lbnode)) ) {
									GetListBrowserNodeAttrs(lbnode, LBNA_UserData,&sn, TAG_DONE);
								}
								else break;
							}
						}
					}

					ConfigChanged=TRUE;
					HotListChanged=TRUE;
				}

				UnlockWindow(SiteListWin_Object);
				//WakeWindow(SiteListWindow,lw);
			}
		}
	}

	return 1;
}

int RemoveClicked(void)
{
	struct Node *lbn;
	struct SiteNode *sn=NULL;
	struct Node *lbns;

//    geta4();

	GetAttr(LISTBROWSER_SelectedNode, SLG_List[SLG_SiteList], (ULONG *)&lbn);
	if (lbn) {
		LockWindow(SiteListWin_Object);
		int result = showRequester(SiteListWindow, REQIMAGE_WARNING, GetAmiFTPString(Str_AmiFTPRequest), GetAmiFTPString(MW_DeleteCancel), GetAmiFTPString(MW_DeleteRequest));
		UnlockWindow(SiteListWin_Object);
		if (result==0) return 0;

		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
		               LISTBROWSER_Labels,~0, TAG_DONE);
		lbns=GetSucc(lbn);
		Remove(lbn);
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
		               LISTBROWSER_Labels, &SiteList,
		               LISTBROWSER_Selected, -1,
		              TAG_DONE);
/*	GetListBrowserNodeAttrs(lbn,
				LBNA_UserData, &sn,
				TAG_DONE);*/
//	LBRemNode(SLG_List[SLG_SiteList], SiteListWindow, NULL, lbn);
		if (sn) {
			if (sn->sn_MenuType==SLN_PARENT) {
				FreeSiteNode(sn);
				if (lbns) {
					GetListBrowserNodeAttrs(lbns, LBNA_UserData,&sn, TAG_DONE);
					while (sn->sn_MenuType==SLN_CHILD) {
						sn->sn_MenuType=SLN_REGULAR;
						//SetListBrowserNodeAttrs(lbns, LBNA_Generation,1, TAG_DONE);
						SetListBrowserNodeAttrs(lbns, LBNA_Generation,SLN_REGULAR, TAG_DONE);
						lbns=GetSucc(lbns);
						if (lbns) { GetListBrowserNodeAttrs(lbns, LBNA_UserData,&sn, TAG_DONE); }
						else break;
					}
				}
			}
			else FreeSiteNode(sn);
		}

		ConfigChanged=TRUE;
		HotListChanged=TRUE;
		UpdateSLGGadgets(FALSE, 0);
		RefreshGList((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL, 1);
	}

	return 1;
}

//#define DisableGadget(gadget,disable) if(SetGadgetAttrs(gadget,SiteListWindow,NULL,GA_Disabled,disable,TAG_DONE) && SiteListWindow) RefreshGList(gadget,SiteListWindow,NULL,1);
void UpdateSLGGadgets(const ULONG selected, const int type)
{
	ULONG attr;//=(((struct Gadget *)SLG_List[SLG_Edit])->Flags & GFLG_DISABLED);
DBUG("UpdateSLGGadgets() 0x%08lx %ld\n",selected,type);
	if (!selected) {
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Edit], SiteListWindow, NULL,
		               GA_Disabled,TRUE, TAG_DONE);
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Remove], SiteListWindow, NULL,
		               GA_Disabled,TRUE, TAG_DONE);
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Top], SiteListWindow, NULL,
		               GA_Disabled,TRUE, TAG_DONE);
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Up], SiteListWindow, NULL,
		               GA_Disabled,TRUE, TAG_DONE);
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Down], SiteListWindow, NULL,
		               GA_Disabled,TRUE, TAG_DONE);
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Bottom], SiteListWindow, NULL,
		               GA_Disabled,TRUE, TAG_DONE);
		if (SLG_List[SLG_Connect]) {
			SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Connect], SiteListWindow, NULL,
			               GA_Disabled,TRUE, TAG_DONE);
		}
	}
	else {
		SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Edit], SiteListWindow, NULL,
		               GA_Disabled,type==SLN_BARLABEL? TRUE:FALSE, TAG_DONE);

		if (type==SLN_PARENT) {
			//attr=(((struct Gadget *)SLG_List[SLG_Edit])->Flags & GFLG_DISABLED);
//DBUG("attr=0x%08lx\n",attr);
//			if (attr) {
				/*SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Edit], SiteListWindow, NULL,
				               GA_Disabled,FALSE, TAG_DONE);*/
				SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Remove], SiteListWindow, NULL,
				               GA_Disabled,FALSE, TAG_DONE);
				SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Top], SiteListWindow, NULL,
				               GA_Disabled,TRUE, TAG_DONE);
				SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Up], SiteListWindow, NULL,
				               GA_Disabled,TRUE, TAG_DONE);
				SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Down], SiteListWindow, NULL,
				               GA_Disabled,TRUE, TAG_DONE);
				SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Bottom], SiteListWindow, NULL,
				               GA_Disabled,TRUE, TAG_DONE);
//			}
			if (SLG_List[SLG_Connect]) {
				attr=(((struct Gadget *)SLG_List[SLG_Connect])->Flags & GFLG_DISABLED);
				if (!attr) {
					SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Connect], SiteListWindow, NULL,
					               GA_Disabled,TRUE, TAG_DONE);
				}
			}
		}
		else {
			//attr=(((struct Gadget *)SLG_List[SLG_Edit])->Flags & GFLG_DISABLED);
//			if (attr) {
				/*SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Edit], SiteListWindow, NULL,
				               GA_Disabled,FALSE, TAG_DONE);*/
				SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Remove], SiteListWindow, NULL,
				               GA_Disabled,FALSE, TAG_DONE);
				SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Top], SiteListWindow, NULL,
				               GA_Disabled,FALSE, TAG_DONE);
				SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Up], SiteListWindow, NULL,
				               GA_Disabled,FALSE, TAG_DONE);
				SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Down], SiteListWindow, NULL,
				               GA_Disabled,FALSE, TAG_DONE);
				SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Bottom], SiteListWindow, NULL,
				               GA_Disabled,FALSE, TAG_DONE);
//			}
			if (SLG_List[SLG_Connect]) {
				attr=(((struct Gadget *)SLG_List[SLG_Connect])->Flags & GFLG_DISABLED);
				if (attr) {
					SetGadgetAttrs((struct Gadget*)SLG_List[SLG_Connect], SiteListWindow, NULL,
				                GA_Disabled,FALSE, TAG_DONE);
				}
			}
		}
	}
}
//#undef DisableGadget


void swapEntries(struct Node *n, struct Node *pos_n)
{
	struct SiteNode *sn;
DBUG("swapEntry()\n",NULL);
	GetListBrowserNodeAttrs(n, LBNA_UserData,&sn, TAG_DONE);

	// Update node's LBNA_Generation
DBUG("  sn->sn_MenuType=0x%08lx\n",sn->sn_MenuType);
	SetListBrowserNodeAttrs(n, LBNA_Generation,sn->sn_MenuType, TAG_DONE);

	// Swap entries
	Remove(n);
	Insert(&SiteList, n, pos_n);

	// Select updated position of "moved" node
	SetGadgetAttrs((struct Gadget*)SLG_List[SLG_SiteList], SiteListWindow, NULL,
	               LISTBROWSER_Labels,&SiteList, LISTBROWSER_SelectedNode,n,
	               LISTBROWSER_MakeNodeVisible,n, TAG_DONE);

	HotListChanged = TRUE;
	ConfigChanged  = TRUE;
}
