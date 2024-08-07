/* RCS Id: $Id: Menu.c 1.795 1996/09/28 13:32:58 lilja Exp $
   Locked version: $Revision: 1.795 $
*/

#include "AmiFTP.h"
#include "gui.h"
#include "MenuClass.h"


static int menu_Connect(struct MenuItem *menuitem);
//static int menu_Disconnect(struct MenuItem *menuitem);
static int menu_Parent(struct MenuItem *menuitem);
static int menu_Get(struct MenuItem *menuitem);
static int menu_Put(struct MenuItem *menuitem);
static int menu_View(struct MenuItem *menuitem);
static int menu_Quit(struct MenuItem *menuitem);
//static int menu_CreateDir(struct MenuItem *menuitem);
#ifndef __amigaos4__
static int menu_SortByName(struct MenuItem *menuitem);
static int menu_SortByDate(struct MenuItem *menuitem);
#endif
static int menu_Iconify(struct MenuItem *menuitem);
static int menu_SelectAll(struct MenuItem *menuitem);
static int menu_UnselectAll(struct MenuItem *menuitem);
//static int menu_Delete(struct MenuItem *menuitem);
//static int menu_Move(struct MenuItem *menuitem);
//static int menu_ClearCache(struct MenuItem *menuitem);
static int menu_HostlistPrefs(struct MenuItem *menuitem);
//static int menu_BinaryTransferMode(struct MenuItem *menuitem);
//static int menu_AsciiTransferMode(struct MenuItem *menuitem);
static int menu_ResetADT(struct MenuItem *menuitem);
	static int menu_ToggleADT(uint32 selected);
	static int menu_ToggleDotFiles(uint32 selected);
	static int menu_LogWindow(uint32 selected);
static int menu_AddToSitelist(struct MenuItem *menuitem);
//static int menu_LoadPrefs(struct MenuItem *menuitem);
//static int menu_SavePrefs(struct MenuItem *menuitem);
//static int menu_SavePrefsAs(struct MenuItem *menuitem);
//static int menu_About(struct MenuItem *menuitem);
static int menu_Reconnect(struct MenuItem *menuitem);
static int menu_OpenPrefsWindow(struct MenuItem *menuitem);
static int menu_PatternSelect(struct MenuItem *menuitem);
static int menu_ViewReadme(struct MenuItem *menuitem);
//static int menu_RawCommand(struct MenuItem *menuitem);
//static int menu_Rename(struct MenuItem *menuitem);


Object *menustripobj, *hotlistmenu;
int32 mimgsize = 24;


//struct Menu *menu;


struct Image *MenuImage(CONST_STRPTR name, struct Screen *screen)
{
	struct Image *i = NULL;
	APTR prev_win;
	BPTR dir, prev_dir;
	STRPTR name_s, name_g;
	uint32 len, totlen;

	len = Strlen(name);
	totlen = 2*len + 6; // [name_LENGHT + 3] + [name_LENGTH + 3] -> [name_s] + [name_g]

	name_s = AllocVecTags(totlen, TAG_END);
	if(name_s)
	{
		name_g = name_s + len + 3;

		Strlcpy(name_s, name, totlen);
		Strlcat(name_s, "_s", totlen);

		Strlcpy(name_g, name, totlen);
		Strlcat(name_g, "_g", totlen);

		prev_win = SetProcWindow( (APTR)-1 ); // Disable requesters
		dir = Lock("TBIMAGES:", SHARED_LOCK);
		SetProcWindow(prev_win);              // Re-enable requesters
		if(dir != ZERO)
		{
			prev_dir = SetCurrentDir(dir);

			i = (struct Image *)NewObject(BitMapClass, NULL,//BitMapObject,
			                     (mimgsize!=24)? IA_Scalable : TAG_IGNORE, TRUE,
			                     //IA_Width,24, IA_Height,24+2,
			                     BITMAP_SourceFile,         name,
			                     BITMAP_SelectSourceFile,   name_s,
			                     BITMAP_DisabledSourceFile, name_g,
			                     BITMAP_Screen, screen,
			                     BITMAP_Masking, TRUE,
			                    TAG_END);
			if(i)
				SetAttrs( (Object *)i, IA_Height,mimgsize+2, IA_Width,mimgsize, TAG_END);
				//SetAttrs( (Object *)i, IA_Height,24+2, IA_Width,24, TAG_END);

			SetCurrentDir(prev_dir);
			UnLock(dir);
		}

		FreeVec(name_s);
	}

	return (i);
}

void initMenuHotlist(void)
{
DBUG("initMenuHotlist() 0x%08lx (0x%08lx)\n",hotlistmenu,menustripobj);
	//Clear hotlist menu items
	IDoMethod(menustripobj, OM_REMMEMBER, hotlistmenu);
	DisposeObject(hotlistmenu);
	//"Reattach" hotlist menu
	hotlistmenu = NewObject(NULL, "menuclass",
	                        MA_Type,T_MENU, MA_Label,GetAmiFTPString(MENU_HotList),
	                        MA_ID, MNU_HotListTitle,
	                       TAG_END),
	IDoMethod(menustripobj, OM_ADDMEMBER, hotlistmenu);
}

void updateMenuHotlist(void)
{
	struct SiteNode *ptr;
	struct Node *lbn;
	Object *menuitem, *menugroup;
	uint32 item_ID = MNU_HotListTitle;
DBUG("updateMenuHotlist() 0x%08lx\n",hotlistmenu);
	initMenuHotlist(); //clear hotlist menu items & reattach

	menugroup = hotlistmenu;

	for ( lbn=GetHead(&SiteList); lbn; lbn=GetSucc(lbn) ) {
		GetListBrowserNodeAttrs(lbn, LBNA_UserData,&ptr, TAG_DONE);
		if(ptr) {
			if(ptr->sn_HotList) {
DBUG("sn_MenuType=%ld '%s' (0x%08lx)\n",ptr->sn_MenuType,ptr->sn_Node.ln_Name,ptr);
				//Separator
				if(ptr->sn_BarLabel) { menuitem = NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END); }
				//Group
				else if(ptr->sn_MenuType==SLN_PARENT) {
					menuitem = NewObject(NULL, "menuclass",
					                     MA_Type,T_ITEM, MA_Label,ptr->sn_Node.ln_Name,
					                     MA_UserData,(uint32)ptr,
					                    TAG_END);
					menugroup = hotlistmenu; // "back" to root
				}
				//Site
				else { menuitem = NewObject(NULL, "menuclass",
				                            MA_Type,T_ITEM, MA_Label,ptr->sn_Node.ln_Name,
				                            MA_ID,++item_ID, MA_UserData,(uint32)ptr,
				                           TAG_END);
				}
				//SetAttrs(hotlistmenu, MA_AddChild,menuitem, TAG_END);
				IDoMethod(menugroup, OM_ADDMEMBER, menuitem);
//DBUG("\t0x%08lx (0x%08lx)\n",item_ID,ptr);
				if(ptr->sn_MenuType==SLN_PARENT) { menugroup = menuitem; } // insert sites on this group
			}
		}
	}

}

Object *BuildMenuClass(struct Screen *scr)
{
	Object *menuobj = NewObject(NULL, "menuclass",
//PROJECT
     MA_AddChild, NewObject(NULL, "menuclass",
      MA_Type,T_MENU, MA_Label,GetAmiFTPString(MENU_Project),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Connect),
        MA_ID,    MNU_Connect,
        MA_Image, MenuImage("connect",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Disconnect),
        MA_ID,       MNU_Disconnect,
        MA_Image,    MenuImage("disconnect",scr),
        MA_Disabled, TRUE, // disabled by default
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Reconnect),
        MA_ID,       MNU_Reconnect,
        MA_Image,    MenuImage("sync",scr),
        MA_Disabled, TRUE, // disabled by default
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_RawCommand),
        MA_ID,       MNU_RawCommand,
        MA_Image,    MenuImage("shell",scr),
        MA_Disabled, TRUE, // disabled by default
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_AddToSiteList),
        MA_ID,    MNU_AddToSiteList,
        MA_Image, MenuImage("bookmarkadd",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_ResetADT),
        MA_ID,    MNU_ResetADT,
        MA_Image, MenuImage("reset",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Iconify),
        MA_ID,    MNU_Iconify,
        MA_Image, MenuImage("iconify",scr),
      TAG_END),
      //MA_AddChild, NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_About),
        MA_ID,    MNU_About,
        MA_Image, MenuImage("info",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Quit),
        MA_ID,    MNU_Quit,
        MA_Image, MenuImage("quit",scr),
      TAG_END),
     TAG_END),
//FILES
     MA_AddChild, NewObject(NULL, "menuclass",
      MA_Type,T_MENU, MA_Label,GetAmiFTPString(MENU_Files),
      MA_ID,       MNU_FilesTitle,
      MA_Disabled, TRUE, // FILES menu disabled by default
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_TagAll),
        MA_ID,    MNU_SelectAll,
        MA_Image, MenuImage("file_selectall",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_UntagAll),
        MA_ID,    MNU_UnselectAll,
        MA_Image, MenuImage("file_unselectall",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_PatternSelect),
        MA_ID,    MNU_PatternSelect,
        MA_Image, MenuImage("pattern",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_TransferMode),
        MA_ID,    MNU_TransferMode,
        MA_Image, MenuImage("nettransfer",scr),
          MA_AddChild, NewObject(NULL, "menuclass",
            MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Binary),
            MA_ID,      MNU_BinaryTransferMode,
            //MA_Image, MenuImage("nettransfer",scr),
            MA_MX,       ~(1<<0) & 0xFFFFFFF, // POS 1st on this subitem -> ~1 (AND mask)
            MA_Selected, TRUE,
          TAG_END),
          MA_AddChild, NewObject(NULL, "menuclass",
            MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_ASCII),
            MA_ID,       MNU_AsciiTransferMode,
            MA_MX,       ~(1<<1) & 0xFFFFFFF,
            MA_Selected, FALSE,
            //MA_Image, MenuImage("nettransfer",scr),
         TAG_END),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_ClearCache),
        MA_ID,    MNU_ClearCache,
        MA_Image, MenuImage("dbsync",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Parent),
        MA_ID,    MNU_Parent,
        MA_Image, MenuImage("parent",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Get),
        MA_ID,    MNU_Get,
        MA_Image, MenuImage("download",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Put),
        MA_ID,    MNU_Put,
        MA_Image, MenuImage("upload",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_View),
        MA_ID,    MNU_View,
        MA_Image, MenuImage("show",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_ViewReadme),
        MA_ID,    MNU_ViewReadme,
        MA_Image, MenuImage("filetext",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_CreateDir),
        MA_ID,    MNU_CreateDir,
        MA_Image, MenuImage("newdrawer",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Delete),
        MA_ID,    MNU_Delete,
        MA_Image, MenuImage("delete",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Rename),
        MA_ID,    MNU_Rename,
        MA_Image, MenuImage("rename",scr),
      TAG_END),
     TAG_END),
#ifndef __amigaos4__
//SORT
     MA_AddChild, NewObject(NULL, "menuclass",
      MA_Type,T_MENU, MA_Label,GetAmiFTPString(MENU_Sort),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_SortbyName),
        MA_ID,       MNU_SortByName,
        MA_MX,       ~(1<<0) & 0xFFFFFFF, // POS 1st on this subitem -> ~1 (AND mask)
        MA_Selected, TRUE,
        MA_Image, MenuImage("text",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_SortbyDate),
        MA_ID,       MNU_SortByDate,
        MA_MX,       ~(1<<1) & 0xFFFFFFF,
        MA_Selected, FALSE,
        MA_Image, MenuImage("date",scr),
      TAG_END),
     TAG_END),
#endif
//SETTINGS
     MA_AddChild, NewObject(NULL, "menuclass",
      MA_Type,T_MENU, MA_Label,GetAmiFTPString(MENU_Settings),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Global),
        MA_ID,    MNU_OpenPrefsWindow,
        MA_Image, MenuImage("prefs",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_Hostlist),
        MA_ID,    MNU_HostlistPrefs,
        MA_Image, MenuImage("bookmark",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_LogWindow),
        MA_ID,       MNU_LogWindow,
        MA_Toggle,   TRUE,
        MA_Selected, MainPrefs.mp_Log,
        //MA_Image, MenuImage("text",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_ToggleDotFiles),
        MA_ID,       MNU_ToggleDotFiles,
        MA_Toggle,   TRUE,
        MA_Selected, MainPrefs.mp_Showdotfiles,
        //MA_Image, MenuImage("date",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_ToggleADTFiles),
        MA_ID,       MNU_ToggleADT,
        MA_Toggle,   TRUE,
        MA_Selected, MainPrefs.mp_ShowAllADTFiles,
        //MA_Image, MenuImage("text",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_LoadSettings),
        MA_ID,    MNU_LoadPrefs,
        MA_Image, MenuImage("open",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_SaveSettings),
        MA_ID,    MNU_SavePrefs,
        MA_Image, MenuImage("save",scr),
      TAG_END),
      MA_AddChild, NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetAmiFTPString(MENU_SaveSettingsAs),
        MA_ID,    MNU_SavePrefsAs,
        MA_Image, MenuImage("saveas",scr),
      TAG_END),
     TAG_END),

//Added/created on updateMenuHotlist()
     /*MA_AddChild, hotlistmenu = NewObject(NULL, "menuclass",
      MA_Type,T_MENU, MA_Label,GetAmiFTPString(MENU_HotList),
      MA_ID, MNU_HotListTitle,
     TAG_END),*/
//Added/created on updateMenuHotlist()

	TAG_DONE);
DBUG("BuildMenuClass() 0x%08lx\n",menuobj);
	return(menuobj);
}

int HandleMenus(int32 mitem)
{
	int res = 1;
	uint32 value;
	Object *obj = (Object *)IDoMethod(menustripobj, MM_FINDID, 0, mitem);

	switch(mitem) {
//PROJECT
		case MNU_Connect:
			res = menu_Connect(NULL);
		break;
		case MNU_Disconnect:
			Disconnect_clicked();
		break;
		case MNU_Reconnect:
			res = menu_Reconnect(NULL);
		break;
		case MNU_RawCommand:
			RawCommand_clicked();
		break;
		case MNU_AddToSiteList:
			res = menu_AddToSitelist(NULL);
		break;
		case MNU_ResetADT:
			res = menu_ResetADT(NULL);
		break;
		case MNU_Iconify:
			res = menu_Iconify(NULL);
		break;
		case MNU_About:
			About();
		break;
		case MNU_Quit:
			res = menu_Quit(NULL);
		break;
//FILES
		case MNU_SelectAll:
			res = menu_SelectAll(NULL);
		break;
		case MNU_UnselectAll:
			res = menu_UnselectAll(NULL);
		break;
		case MNU_PatternSelect:
			res = menu_PatternSelect(NULL);
		break;
		case MNU_BinaryTransferMode:
			TransferMode = BINARY;
//DBUG("TransferMode = %ld\n",TransferMode);
		break;
		case MNU_AsciiTransferMode:
			TransferMode = ASCII;
//DBUG("TransferMode = %ld\n",TransferMode);
		break;
		case MNU_ClearCache:
			ClearCache(FALSE);
		break;
		case MNU_Parent:
			menu_Parent(NULL);
		break;
		case MNU_Get:
			menu_Get(NULL);
		break;
		case MNU_Put:
			menu_Put(NULL);
		break;
		case MNU_View:
			menu_View(NULL);
		break;
		case MNU_ViewReadme:
			menu_ViewReadme(NULL);
		break;
		case MNU_CreateDir:
			CreateDir_clicked();
		break;
		case MNU_Delete:
			Delete_clicked();
		break;
		case MNU_Rename:
			Rename_clicked();
		break;
#ifndef __amigaos4__
//SORT
		case MNU_SortByName:
DBUG("MNU_SortByName\n",NULL);
			menu_SortByName(NULL);
		break;
		case MNU_SortByDate:
DBUG("MNU_SortByDate\n",NULL);
			menu_SortByDate(NULL);
		break;
#endif
//SETTINGS
		case MNU_OpenPrefsWindow:
			res = menu_OpenPrefsWindow(NULL);
		break;
		case MNU_HostlistPrefs:
			res= menu_HostlistPrefs(NULL);
		break;
		case MNU_ToggleDotFiles:
			GetAttr(MA_Selected, obj, (uint32 *)&value);
			menu_ToggleDotFiles(value);
		break;
		case MNU_ToggleADT:
			GetAttr(MA_Selected, obj, (uint32 *)&value);
			menu_ToggleADT(value);
		break;
		case MNU_LogWindow:
			GetAttr(MA_Selected, obj, (uint32 *)&value);
			menu_LogWindow(value);
		break;
		case MNU_LoadPrefs:
			LoadPrefs();
		break;
		case MNU_SavePrefs:
			SavePrefs();
		break;
		case MNU_SavePrefsAs:
			SavePrefsAs();
		break;
//HOTLIST (ID 100 an up)
		default:
		{
			struct SiteNode *sn_ptr;
			GetAttr(MA_UserData, obj, (uint32 *)&sn_ptr);
//DBUG("'%s' 0x%08lx\n",sn_ptr->sn_Node.ln_Name,sn_ptr);
			ConnectSite(sn_ptr, 0);
			if(connected) { strncpy(CurrentState.LastLVSite, sn_ptr->sn_Node.ln_Name, sizeof(CurrentState.LastLVSite)); }
		}
		break;
	}

	return(res);
}

int BuildMenu(void)
{
	return 1;
}

void UpdateMenus(void)
{
DBUG("UpdateMenus()\n",NULL);
	updateMenuHotlist();
}

static int menu_Quit(struct MenuItem *menuitem)
{
    return 0;
}

#ifndef __amigaos4__
static int menu_SortByName(struct MenuItem *menuitem)
{
	SortMode=SORTBYNAME;

	if (FileList) {
		LockWindow(MainWin_Object);
		DetachToolList();

		if (!CurrentState.ADTMode)
			FileList=sort_filelist(FileList, SORTBYNAME, ASCENDING);
		else
			FileList=sort_ADT(FileList, SORTBYNAME);

		AttachToolList(FALSE);
		UnlockWindow(MainWin_Object);
	}

	return 1;
}

static int menu_SortByDate(struct MenuItem *menuitem)
{
	SortMode=SORTBYDATE;

	if (FileList) {
		LockWindow(MainWin_Object);
		DetachToolList();

		if (!CurrentState.ADTMode) 
			FileList=sort_filelist(FileList, SORTBYDATE, DESCENDING);
		else
			FileList=sort_ADT(FileList, SORTBYDATE);

		AttachToolList(FALSE);
		UnlockWindow(MainWin_Object);
	}

	return 1;
}
#endif

static int menu_Iconify(struct MenuItem *menuitem)
{
    //if (CA_Iconify(MainWin_Object))
    if (IDoMethod(MainWin_Object, WM_ICONIFY))
      MainWindow=NULL;

    return 2;
}

static int menu_SelectAll(struct MenuItem *menuitem)
{
	struct Node *node;
	ULONG n=0,flags;

	/* Fix disconnect/connect of listview */
	if (FileList) {
		for (node=GetHead(FileList);node;node=GetSucc(node)) {
			GetListBrowserNodeAttrs(node, LBNA_Flags, &flags, TAG_DONE);
			if (!(flags&LBFLG_HIDDEN)) {
				SetListBrowserNodeAttrs(node, LBNA_Selected,TRUE, TAG_DONE);
				n=1;
			}
		}

		if (n) {
			RefreshGList((struct Gadget*) MG_List[MG_ListView], MainWindow, NULL, 1);
			UpdateMainButtons(MB_FILESELECTED);
		}
	}

	UpdateWindowTitle();

	return 1;
}

static int menu_UnselectAll(struct MenuItem *menuitem)
{
	struct Node *node;
	ULONG n=0;

	/* Fix: Add disconnect/connect of listview */
	if (FileList) {
		for (node=GetHead(FileList);node;node=GetSucc(node)) {
			SetListBrowserNodeAttrs(node, LBNA_Selected,FALSE, TAG_DONE);
			n=1;
		}

		if (n)
			RefreshGList((struct Gadget*) MG_List[MG_ListView], MainWindow, NULL, 1);

		UpdateMainButtons(MB_NONESELECTED);
	}

	UpdateWindowTitle();

	return 1;
}

/*static int menu_Delete(struct MenuItem *menuitem)
{
    Delete_clicked();
    return 1;
}*/

/*static int menu_CreateDir(struct MenuItem *menuitem)
{
    CreateDir_clicked();
    return 1;
}*/

/*static int menu_RawCommand(struct MenuItem *menuitem)
{
    //CreateDir_clicked();
    return 1;
}*/

/*static int menu_Rename(struct MenuItem *menuitem)
{
    Rename_clicked();
    return 1;
}*/

/*static int menu_Move(struct MenuItem *menuitem)
{
    return 1;
}*/

/*static int menu_ClearCache(struct MenuItem *menuitem)
{
    ClearCache(FALSE);
    return 1;
}*/

static int menu_HostlistPrefs(struct MenuItem *menuitem)
{
    LockWindow(MainWin_Object);
    OpenSiteListWindow(FALSE);
    UnlockWindow(MainWin_Object);

    return 17;
}

/*static int menu_BinaryTransferMode(struct MenuItem *menuitem)
{
	if (TransferMode!=BINARY) {
		TransferMode=BINARY;
	}

	return 1;
}*/

/*static int menu_AsciiTransferMode(struct MenuItem *menuitem)
{
	if (TransferMode!=ASCII) {
		TransferMode=ASCII;
	}

	return 1;
}*/

static int menu_ResetADT(struct MenuItem *menuitem)
{
    MainPrefs.mp_LastAminetDate=0;
    MainPrefs.mp_LastAMOTD=0;
    return 1;
}

static int menu_ToggleADT(uint32 selected)//struct MenuItem *menuitem)
{
	struct Node *node;
	ULONG attr;

	if (!CurrentState.ADTMode) return 1;

	if(selected) {

			if (FileList) {
				DetachToolList();
				node=GetTail(FileList);
				if (node->ln_Name==NULL) {
					Remove(node);
					FreeListBrowserNode(node);
				}
				for (node=GetHead(FileList);node;node=GetSucc(node)) {
					SetListBrowserNodeAttrs(node, LBNA_Flags,LBFLG_CUSTOMPENS, TAG_DONE);
				}
				if (SetGadgetAttrs((struct Gadget*) MG_List[MG_ListView], MainWindow, NULL,
				                   LISTBROWSER_Labels, FileList,
				                   //LISTBROWSER_AutoFit, TRUE,
				                   LISTBROWSER_MakeVisible, 0, TAG_DONE))
				{
					RefreshGList((struct Gadget*) MG_List[MG_ListView], MainWindow, NULL, 1);
				}
			}
			MainPrefs.mp_ShowAllADTFiles=TRUE;
		}
		else {
			if (FileList) {
				DetachToolList();
				for (node=GetHead(FileList);node;node=GetSucc(node)) {
					struct dirlist *n=(struct dirlist *)node->ln_Name;
					if (!n->new || n->hide) {
						SetListBrowserNodeAttrs(node,
						                        LBNA_Flags, LBFLG_CUSTOMPENS|LBFLG_HIDDEN,
						                        LBNA_Selected, FALSE,
						                       TAG_DONE);
					}
				}
				for (node=GetHead(FileList);node;node=GetSucc(node)) {
					ULONG flags;

					GetListBrowserNodeAttrs(node,LBNA_Flags, &flags, TAG_DONE);
					if (!(flags&LBFLG_HIDDEN)) break;
				}

				if (!node) {
					SetLBColumnInfoAttrs(columninfo,
					                    LBCIA_Column,COL_NAME, LBCIA_Weight,95,
					                    LBCIA_Column,COL_SIZE, LBCIA_Weight, 1, LBCIA_Separator,FALSE,
					                    LBCIA_Column,COL_TYPE, LBCIA_Weight, 1, LBCIA_Separator,FALSE,
					                    LBCIA_Column,COL_DATE, LBCIA_Weight, 1, LBCIA_Separator,FALSE,
					                    LBCIA_Column,COL_OWN,  LBCIA_Weight, 1, LBCIA_Separator,FALSE,
					                    LBCIA_Column,COL_GRP,  LBCIA_Weight, 1, LBCIA_Separator,FALSE,
					                   TAG_DONE);

					node=AllocListBrowserNode(TOT_COLS,
					      LBNA_Column, COL_NAME,
					        LBNCA_Text, GetAmiFTPString(Str_NoNewAminetFiles),
					      LBNA_Column, COL_SIZE,
					        LBNCA_Text, "",
					      LBNA_Column, COL_TYPE,
					        LBNCA_Text, "",
					      LBNA_Column, COL_DATE,
					        LBNCA_Text, "",
					      LBNA_Column, COL_OWN,
					        LBNCA_Text, "",
					      LBNA_Column, COL_GRP,
					        LBNCA_Text, "",
					     TAG_DONE);

					if (node) AddTail(FileList,node);
				}

				if (SetGadgetAttrs((struct Gadget*) MG_List[MG_ListView], MainWindow, NULL,
				                   LISTBROWSER_Labels, FileList,
				                   //LISTBROWSER_AutoFit, TRUE,
				                   LISTBROWSER_MakeVisible, 0, TAG_DONE))
				{
					RefreshGList((struct Gadget*) MG_List[MG_ListView], MainWindow, NULL, 1);
				}

				GetAttr(LISTBROWSER_Selected, MG_List[MG_ListView], &attr);
				if (attr) { UpdateMainButtons(MB_FILESELECTED); }
				else { UpdateMainButtons(MB_NONESELECTED); }
			}

			MainPrefs.mp_ShowAllADTFiles=FALSE;
	}

	UpdateWindowTitle();

	return 1;
}

static int menu_ToggleDotFiles(uint32 selected)//struct MenuItem *menuitem)
{
	struct Node *node;
	ULONG attr;

	if (CurrentState.ADTMode) return 1;

	if(selected) {

		if (FileList) {
			DetachToolList();
			for (node=GetHead(FileList); node!=NULL;node=GetSucc(node)) {
				SetListBrowserNodeAttrs(node, LBNA_Flags,LBFLG_CUSTOMPENS, TAG_DONE);
			}
			AttachToolList(FALSE);
		}
		MainPrefs.mp_Showdotfiles=TRUE;
		}
		else {
			if (FileList) {
				DetachToolList();
				for (node=GetHead(FileList);node != NULL;node=GetSucc(node)) {
					struct dirlist *n=(struct dirlist *)node->ln_Name;
					if (n->name[0]=='.') {
						SetListBrowserNodeAttrs(node,
						                        LBNA_Flags, LBFLG_CUSTOMPENS|LBFLG_HIDDEN,
						                        LBNA_Selected, FALSE, TAG_DONE);
					}
				}
				AttachToolList(FALSE);
				GetAttr(LISTBROWSER_Selected, MG_List[MG_ListView], &attr);
				if (attr) { UpdateMainButtons(MB_FILESELECTED); }
				else { UpdateMainButtons(MB_NONESELECTED); }
			}

			MainPrefs.mp_Showdotfiles=FALSE;
	}

	return 1;
}

static int menu_LogWindow(uint32 selected)
{
	if(selected) {
		OpenLogWindow();
		MainPrefs.mp_Log = 1;
	}
	else {
		CloseLogWindow();
		MainPrefs.mp_Log = 0;
	}

	return 1;
}

static int menu_AddToSitelist(struct MenuItem *menuitem)
{
	struct SiteNode *sn;

	if (!TCPStack) return 1;

	if (CurrentState.CurrentSite[0]==0) return 1;

	sn=(struct SiteNode*) AllocVecTags(sizeof(struct SiteNode), AVT_Type, MEMF_SHARED, AVT_ClearWithValue, 0, TAG_DONE);
	if (sn) {
		sn->sn_Node.ln_Name=strdup(CurrentState.CurrentSite);
		if (!sn->sn_Node.ln_Name) {
			FreeVec(sn);
			return 1;
		}

		sn->sn_SiteAddress=strdup(CurrentState.CurrentSite);
		if (!sn->sn_Node.ln_Name) {
			free(sn->sn_Node.ln_Name);
			FreeVec(sn);
			return 1;
		}

		sn->sn_RemoteDir=strdup(CurrentState.CurrentRemoteDir);
		if (!sn->sn_RemoteDir) {
			free(sn->sn_Node.ln_Name);
			free(sn->sn_SiteAddress);
			FreeVec(sn);
			return 1;
		}

		sn->sn_Port=21;
		sn->sn_Proxy=MainPrefs.mp_DefaultProxy;
		sn->sn_Anonymous=1;
		AddLBNTail(&SiteList, sn);
		ConfigChanged=TRUE;
	}

	return 1;
}

/*static int menu_LoadPrefs(struct MenuItem *menuitem)
{
    LoadPrefs();
    return 1;
}*/

/*static int menu_SavePrefs(struct MenuItem *menuitem)
{
    SavePrefs();
    return 1;
}*/

/*static int menu_SavePrefsAs(struct MenuItem *menuitem)
{
    SavePrefsAs();
    return 1;
}*/

/*static int menu_About(struct MenuItem *menuitem)
{
    About();
    return 1;
}*/

static int menu_Reconnect(struct MenuItem *menuitem)
{
    Reconnect();
    return 17;
}

static int menu_OpenPrefsWindow(struct MenuItem *menuitem)
{
    OpenPrefsWindow();
    return 17;
}

static int menu_Connect(struct MenuItem *menuitem)
{
    Connect_clicked();
    return 17;
}

/*static int menu_Disconnect(struct MenuItem *menuitem)
{
    Disconnect_clicked();
    return 1;
}*/

static int menu_Parent(struct MenuItem *menuitem)
{
    if (FileList && connected && !CurrentState.ADTMode) Parent_clicked();

    return 1;
}

static int menu_Get(struct MenuItem *menuitem)
{
    if (FileList && connected) Get_clicked();

    return 1;
}

static int menu_Put(struct MenuItem *menuitem)
{
    if (connected) Put_clicked();

    return 1;
}

static int menu_View(struct MenuItem *menuitem)
{
    if (FileList && connected) View_clicked(FALSE);

    return 1;
}

static int menu_ViewReadme(struct MenuItem *menuitem)
{
    if (FileList && connected && CurrentState.ADTMode) View_clicked(TRUE);

    return 1;
}

static int menu_PatternSelect(struct MenuItem *menuitem)
{
	char patterbuf[50]="";

	if (!FileList || !connected) return 1;

	LockWindow(MainWin_Object);

	int result = showStringRequester(MainWindow, FALSE, REQIMAGE_INFO, GetAmiFTPString(Str_PatternRequest), GetAmiFTPString(Str_OkCancel), patterbuf, sizeof(patterbuf)-1, GetAmiFTPString(Str_SelectPattern));
	if(result) {
		char pattern[104];
		struct Node *node;
		ULONG flags;
		int n=0;

		ParsePatternNoCase(patterbuf, pattern, sizeof(pattern));
		for (node=GetHead(FileList); node; node=GetSucc(node)) {
			GetListBrowserNodeAttrs(node, LBNA_Flags, &flags, TAG_DONE);

			if (!(flags&LBFLG_HIDDEN)) {
				if (MatchPatternNoCase(pattern, ((struct dirlist *)node->ln_Name)->name)) {
					SetListBrowserNodeAttrs(node, LBNA_Selected, TRUE, TAG_DONE);
					n=1;
				}
			}
		}

		if (n) {
			RefreshGList((struct Gadget*) MG_List[MG_ListView], MainWindow, NULL, 1);
			UpdateMainButtons(MB_FILESELECTED);
		}
	}

	UnlockWindow(MainWin_Object);

	return 1;
}

/* EOF */
