/* RCS Id:  $Id: main.c 1.802 1996/11/30 13:15:06 lilja Exp lilja $
   Locked version: $Revision: 1.802 $
*/

#include "AmiFTP.h"
#include "gui.h"
#include "AmiFTP_rev.h"

const char *version = VERSTAG;


struct Library *IntuitionBase;
struct Library *UtilityBase;
struct Library *GfxBase;
struct Library *DiskfontBase;
struct Library *AslBase;
struct Library *IFFParseBase;
struct Library *IconBase;
struct Library *RexxSysBase;
struct Library *WorkbenchBase;
struct Library *LocaleBase;
struct Library *AmigaGuideBase;
struct Device  *TimerBase;
struct Library *ApplicationLib;
//struct Library *SocketBase;

struct IntuitionIFace   *IIntuition;
struct UtilityIFace     *IUtility;
struct GraphicsIFace    *IGraphics;
struct DiskfontIFace    *IDiskfont;
struct AslIFace         *IAsl;
struct IFFParseIFace    *IIFFParse;
struct IconIFace        *IIcon;
struct RexxSysIFace     *IRexxSys;
struct WorkbenchIFace   *IWorkbench;
struct LocaleIFace      *ILocale;
struct AmigaGuideIFace  *IAmigaGuide;
struct TimerIFace       *ITimer;
struct ApplicationIFace *IApplication;
//struct SocketIFace      *ISocket;

struct Library *ARexxBase = NULL;
struct ARexxIFace *IARexx = NULL;
struct Library *ClickTabBase = NULL, *ListBrowserBase = NULL, *LayoutBase = NULL,
               *ChooserBase = NULL, *SpeedBarBase = NULL;
// the class library base
struct ClassLibrary *ButtonBase = NULL, *BitMapBase = NULL, *CheckBoxBase = NULL,
                    *LabelBase = NULL, *WindowBase = NULL, *StringBase = NULL,
                    *GetFileBase = NULL, *GetFontBase = NULL, *FuelGaugeBase = NULL,
                    *IntegerBase = NULL, *RequesterBase = NULL, *TextEditorBase = NULL,
                    *ScrollerBase = NULL;
// the class pointer
Class *ClickTabClass, *ListBrowserClass, *ButtonClass, *LabelClass, *StringClass,
      *GetFileClass, *GetFontClass, *SpeedBarClass, *IntegerClass, *ARexxClass,
      *FuelGaugeClass, *CheckBoxClass, *ChooserClass, *BitMapClass, *LayoutClass,
      *WindowClass, *RequesterClass, *TextEditorClass, *ScrollerClass;
// some interfaces needed
struct ListBrowserIFace *IListBrowser = NULL;
struct ClickTabIFace *IClickTab = NULL;
struct LayoutIFace *ILayout = NULL;
struct ChooserIFace *IChooser = NULL;
struct SpeedBarIFace *ISpeedBar = NULL;


extern struct TextAttr AmiFTPAttr, ListViewAttr;
extern Object *ARexx_Object;

BOOL InitRexx(void);
void PrintSiteList(void);
void GetToolTypes(struct WBStartup *msg);
//void SetupLocaleStrings(void);
//void ConvertFontName(char *dest, int *size, char *source);

extern struct Screen *myScn;

ULONG appID; // notification/application_lib

char decimalSeparator = '.';
//char groupSeparator = ',';
//uint8 separatorSize = 3;

long __stack = 20480;
struct WBStartup *wBenchMsg;
extern STRPTR _ProgramName;

char *ConfigName;
char *AMIFTPPREFS = "AmiFTP.prefs";
char *PROGDIRPREFS = "PROGDIR:AmiFTP.prefs";
char *ENVPREFS = "ENVARC:AmiFTP.prefs";

BOOL DEBUG = FALSE;
BOOL TCPStack = FALSE;

struct MsgPort *AppPort;

struct CLIArguments *cliargs = NULL;
extern STRPTR context[];
int CXBRK()
{
	return 0;
}
void chkabort(void)
{
	return;
}

//char *ErrorOpenLib = "Couldn't open version %ld of %s.";
//char *ErrorNoPort = "Couldn't open messageport.";

__attribute__ ((linearvarargs))
void PrintError(const STRPTR errmsg, ...)
{
	va_list ap;
	va_startlinear(ap, errmsg);
	APTR args = va_getlinearva(ap, APTR);
	struct EasyStruct req = {
		sizeof(struct EasyStruct), 0, NULL, NULL, NULL
	};

	if (wBenchMsg && IntuitionBase)
	{
		req.es_Title = GetAmiFTPString(Str_AmiFTPError);//"AmiFTP Error";
		req.es_TextFormat = errmsg;
		req.es_GadgetFormat = GetAmiFTPString(Str_OK);//"Ok";
		EasyRequestArgs(NULL, &req, NULL, args);
	}
	else
	{
		VPrintf(errmsg, args);
		Printf("\n");
	}
	va_end(ap);
}

struct as225passwd {
	char *pw_name;
	char *pw_dir;
	char *pw_passwd;
	char *pw_gecos;
	uid_t pw_uid;
	gid_t pw_gid;
	char *pw_shell;				/* unused */
	char *pw_comment;
};


/*extern struct ListBrowserIFace *IListBrowser;
extern struct LayoutIFace *ILayout;
extern struct LabelIFace *ILabel;
extern struct WindowIFace *IWindow;
extern struct ChooserIFace *IChooser;
extern struct StringIFace *IString;
extern struct ClickTabIFace *IClickTab;
extern struct CheckBoxIFace *ICheckBox;
extern struct BevelIFace *IBevel;
extern struct ARexxIFace *IARexx;
extern struct SpeedBarIFace *ISpeedBar;
extern struct FuelGaugeIFace *IFuelGauge;
extern struct IntegerIFace *IInteger;
extern struct GadToolsIFace *IGadTools;*/


//extern struct ClassLibrary *TextEditorBase;


BOOL UseAS225 = FALSE;
static struct Process *ME;

int main(int argc, char **argv)
{
	struct servent *servent;
	APTR oldwptr;

	if (!argc) { wBenchMsg = (struct WBStartup *) argv; }

	if(MyOpenLibs() == FALSE) { CleanUp(); return(10); }

	/*if (!IDOS || !IListBrowser || !ILayout || !ILabel || !IWindow || !IChooser
	    || !IString || !IClickTab || !ICheckBox || !IBevel || !IARexx || !ISpeedBar
	    || !IFuelGauge || !IInteger || !IGadTools)
		{ CleanUp(); return 10; }*/

	//memset(&MainPrefs, 0, sizeof(MainPrefs));
	//memset(&CurrentState, 0, sizeof(CurrentState));
	ClearMem(&MainPrefs, sizeof(MainPrefs));
	ClearMem(&CurrentState, sizeof(CurrentState));

	ME = (struct Process *) FindTask(NULL);
	oldwptr = ME->pr_WindowPtr;

	//MyOpenLibs();
	ag.ag_NAG.nag_BaseName   = "AmiFTP";
	ag.ag_NAG.nag_Name       = "AmiFTP.guide";
	ag.ag_NAG.nag_ClientPort = "AMIFTP_HELP";
	ag.ag_NAG.nag_Context    = context;

	MainPrefs.mp_ViewCommand         = strdup("WBRUN %F");
	MainPrefs.mp_CacheSize           = 5;
	MainPrefs.mp_BufferSize          = 8192;
	MainPrefs.mp_DeleteFiles         = TRUE;
	MainPrefs.mp_UseDefaultFonts     = TRUE;
	MainPrefs.mp_OpenOnDefaultScreen = TRUE;
	MainPrefs.mp_ShowToolBar         = 1;//TRUE;

//	if (!argc)
//	{
//		//WB2CLI(_WBenchMsg, __stack, DOSBase);
//		wBenchMsg = (struct WBStartup *) argv;
	if(wBenchMsg)
	{
		GetToolTypes(wBenchMsg);
	}
	else
	{
		argsptr = ReadArgs(TEMPLATE, opts, NULL);
		if (argsptr == NULL)
		{
			PrintFault(IoErr(), NULL);
			CleanUp();
			//exit(1);
			return(1);
		}
		cliargs = (void *) &opts[0];
		if (cliargs->pubscreen)
        {
			strncpy(CurrentState.ScreenName, (char *) cliargs->pubscreen, 256);
		}
        if (cliargs->iconified)
			CurrentState.Iconified = 1;
		if (cliargs->portname)
			strncpy(CurrentState.RexxPort, cliargs->portname, 50);
		if (cliargs->settings)
			ConfigName = strdup((char *) cliargs->settings);

		if (cliargs->debug)
			DEBUG = TRUE;
		if (cliargs->as225)
			UseAS225 = TRUE;
	}
	if (!strlen(CurrentState.RexxPort))
	{
		strcpy(CurrentState.RexxPort, "AMIFTP");
	}

	if (!InitRexx())
	{
		CleanUp();
		//exit(10);
		return(10);
	}

	if(IApplication) {
		appID = RegisterApplication(CurrentState.RexxPort,//"AmiFTP",
		                            REGAPP_UniqueApplication, FALSE,
		                            REGAPP_URLIdentifier,     "os4welt",
		                            REGAPP_FileName,          "PROGDIR:AmiFTP",
		                            REGAPP_NoIcon,            TRUE,
		                            REGAPP_Description,       GetAmiFTPString(Str_Notify_Description),
		                           TAG_DONE);
	}

	/* This has to be here, can't put this in MyOpenLibs() */
	/* Has to check for mlink */
	ME->pr_WindowPtr = (APTR) - 1;
	if (UseAS225 == FALSE)
	{
		if (FindPort("MLINK"))
		{
			UseAS225 = TRUE;
		}
	}
	TCPStack = OpenTCP(UseAS225);
/*    if (!(TCPStack=OpenTCP(UseAS225))) {
	ME->pr_WindowPtr=oldwptr;
	PrintError("Can't open any TCP/IP-stack\n");
	CleanUp();
	exit(10);
    }*/
	ME->pr_WindowPtr = oldwptr;

	/* FTP inits */
	data = -1;
	verbose = 0;
	code = -1;
	cpend = 0;
	curtype = 0;
	crflag = 1;
	sendport = -1;

	/*
	 * Determine port numbers to use
	 */
	if (TCPStack)
	{
		servent = tcp_getservbyname("ftp", "tcp");
		if (servent != NULL)
		{
			ftp_port = servent->s_port;
		}
		else
		{
			ftp_port = htons(FTP_PORT);
		}
	}

	/* Set current_month + current_year so date-sorting works */
	{
		time_t t;
		struct tm *tm;

		t = time((time_t *) NULL);
		tm = localtime(&t);
		current_year = tm->tm_year;
		current_month = tm->tm_mon;
	}

	/* Make sure the lists are initialized */
	NewList(&SiteList);
	NewList(&TempList);

	/* Copy the fontnames if the user specified any */
	if (CurrentState.PropFontSize)
	{
		AmiFTPAttr.ta_Name = CurrentState.PropFontName;
		AmiFTPAttr.ta_YSize = CurrentState.PropFontSize;
	}
	if (CurrentState.FixedFontSize)
	{
		ListViewAttr.ta_Name = CurrentState.FixedFontName;
		ListViewAttr.ta_YSize = CurrentState.FixedFontSize;
	}

	if (SetSignal(0L, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
	{
		CleanUp();
		//exit(1);
		return(1);
	}
	InitCache();
	{
		struct passwd *pw;
		struct as225passwd *as225pw;
		char dirname[MAXPATHLEN + 1];
		char dname[MAXHOSTNAMELEN];
		char *cp;
		BPTR lock;

		defaultanonymouspw = NULL;
		memset(dirname, 0, sizeof(dirname));
		if (TCPStack)
		{
			if (SocketBase)
			{
				if (cp = tcp_getlogin())
				{
					if (pw = tcp_getpwnam(cp))
					{
						if (!tcp_gethostname(dname, sizeof(dname)))
						{
							sprintf(dirname, "%s@%s", pw->pw_name ? pw->pw_name : "user", dname);
							defaultanonymouspw = strdup(dirname);
						}
						memset(dirname, 0, sizeof(dirname));
						if (pw->pw_dir)
							strmfp(dirname, pw->pw_dir, AMIFTPPREFS, MAXPATHLEN + 1);
						tcp_endpwent();
					}
				}
			}
			else
			{
				if (cp = tcp_getlogin())
				{
					if (as225pw = (struct as225passwd *) tcp_getpwnam(cp))
					{
						if (!tcp_gethostname(dname, sizeof(dname)))
						{
							sprintf(dirname, "%s@%s", as225pw->pw_name ? as225pw->pw_name : "user", dname);
							defaultanonymouspw = strdup(dirname);
						}
						memset(dirname, 0, sizeof(dirname));
						if (as225pw->pw_dir)
							strmfp(dirname, as225pw->pw_dir, AMIFTPPREFS, MAXPATHLEN + 1);
						tcp_endpwent();
					}
				}
			}
		}
		/*
		   Search order for configfile:
		   SETTINGS-tooltype/cli argument
		   CurrentDir
		   PROGDIR:
		   ENVARC:
		   ~user/AmiFTP.prefs (dirname)
		   and perhaps AmiTCP:db/AmiFTP.prefs
		 */
		if (ConfigName)
		{						/* If ConfigName is set the user told us via tooltypes */
			if (lock = Lock(ConfigName, SHARED_LOCK))//ACCESS_READ))
			{
				UnLock(lock);
				ReadConfigFile(ConfigName);
			}
		}
		else
		{
			if (lock = Lock(AMIFTPPREFS, SHARED_LOCK))//ACCESS_READ))
			{
				UnLock(lock);
				ConfigName = strdup(AMIFTPPREFS);
				ReadConfigFile(ConfigName);
			}
			else if (lock = Lock(PROGDIRPREFS, SHARED_LOCK))//ACCESS_READ))
			{
				UnLock(lock);
				ConfigName = strdup(PROGDIRPREFS);
				ReadConfigFile(ConfigName);
			}
			else if (lock = Lock(ENVPREFS, SHARED_LOCK))//ACCESS_READ))
			{
				UnLock(lock);
				ConfigName = strdup(ENVPREFS);
				ReadConfigFile(ConfigName);
			}
			else if (dirname[0] && (lock = Lock(dirname, SHARED_LOCK)))//ACCESS_READ)))
			{
				ConfigName = strdup(dirname);
				UnLock(lock);
				ReadConfigFile(ConfigName);
			}
			else
			{
				ConfigName = strdup(PROGDIRPREFS);
			}
		}
	}

	if (!MainPrefs.mp_AnonymousPW)	/* Check for lilja@tomcat.rydnet.lysator.liu.se */
		MainPrefs.mp_AnonymousPW = defaultanonymouspw;
	else if (!strcmp(MainPrefs.mp_AnonymousPW, "lilja@tomcat.rydnet.lysator.liu.se"))
	{
		free(MainPrefs.mp_AnonymousPW);
		MainPrefs.mp_AnonymousPW = defaultanonymouspw;
		ConfigChanged = TRUE;
	}

	if (cliargs && cliargs->pubscreen)
	{
		//char *a = strdup(cliargs->pubscreen);
		MainPrefs.mp_PubScreen = strdup(cliargs->pubscreen);//a;
	}
	ag.ag_NAG.nag_PubScreen = MainPrefs.mp_PubScreen;

	ConfigChanged = FALSE;
	if (DEBUG)
		PrintSiteList();

	if (!CurrentState.ScreenName[0])
		ag.ag_NAG.nag_PubScreen = NULL;

	ftpWindow();

	CloseLogWindow();

	if (ConfigChanged)
		WriteConfigFile(ConfigName);

	{
		struct Node *node;
		while (node = RemHead(&TempList))
		{
			if (MainPrefs.mp_DeleteFiles)
				Delete(node->ln_Name);
			free(node->ln_Name);
			free(node);
		}
	}

	CleanUp();
	return(0);
}

BOOL MyOpenLibs(void)
{
//should open required libs here too
	struct Locale *currentLocale = NULL;

 LocaleBase = OpenLibrary("locale.library", 52);
 if(!LocaleBase)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenLib),52,"locale.library");
  return FALSE;
 }
 ILocale = (struct LocaleIFace *)GetInterface(LocaleBase, "main", 1, NULL);
 if(!ILocale)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenIFace),"LocaleIFace");
  return FALSE;
 }

	if ( (currentLocale=OpenLocale(NULL)) )
	{
		if (currentLocale->loc_DecimalPoint  ) decimalSeparator = currentLocale->loc_DecimalPoint[0];
		//if (currentLocale->loc_GroupSeparator) groupSeparator = currentLocale->loc_GroupSeparator[0];
		//if (currentLocale->loc_Grouping      ) separatorSize = *currentLocale->loc_Grouping;

		CloseLocale(currentLocale);
	}
DBUG("decimalSeperator: %c\n",decimalSeparator);
	OpenAmiFTPCatalog(NULL, NULL);
#ifndef _MENUCLASS_
	SetupLocaleStrings();
#endif

 UtilityBase = OpenLibrary("utility.library", 52);
 if(!UtilityBase)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenLib),52,"utility.library");
  return FALSE;
 }
 IUtility = (struct UtilityIFace *)GetInterface(UtilityBase, "main", 1, NULL);
 if(!IUtility) {
  PrintError(GetAmiFTPString(Str_ErrorOpenIFace),"UtilityIFace");
  return FALSE;
 }

 IntuitionBase = OpenLibrary("intuition.library", 52);
 if(!IntuitionBase)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenLib),52,"intuition.library");
  return FALSE;
 }
 IIntuition = (struct IntuitionIFace *)GetInterface(IntuitionBase, "main", 1, NULL);
 if(!IIntuition)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenIFace),"IntuitionIFace");
  return FALSE;
 }

 IconBase = OpenLibrary("icon.library", 52);
 if(!IconBase)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenLib),52,"icon.library");
  return FALSE;
 }
 IIcon = (struct IconIFace *)GetInterface(IconBase, "main", 1, NULL);
 if(!IIcon)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenIFace),"IconIFace");
  return FALSE;
 }

 AslBase = OpenLibrary("asl.library", 52);
 if(!AslBase)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenLib),52,"asl.library");
  return FALSE;
 }
 IAsl = (struct AslIFace *)GetInterface(AslBase, "main", 1, NULL);
 if(!IAsl)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenIFace),"AslIFace");
  return FALSE;
 }

 WorkbenchBase = OpenLibrary("workbench.library", 52);
 if(!WorkbenchBase)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenLib),52,"workbench.library");
  return FALSE;
 }
 IWorkbench = (struct WorkbenchIFace *)GetInterface(WorkbenchBase, "main", 1, NULL);
 if(!IWorkbench)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenIFace),"WorkbenchIFace");
  return FALSE;
 }

 IFFParseBase = OpenLibrary("iffparse.library", 52);
 if(!IFFParseBase)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenLib),52,"iffparse.library");
  return FALSE;
 }
 IIFFParse = (struct IFFParseIFace *)GetInterface(IFFParseBase, "main", 1, NULL);
 if(!IIFFParse)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenIFace),"IFFParseIFace");
  return FALSE;
 }

 DiskfontBase = OpenLibrary("diskfont.library", 52);
 if(!DiskfontBase)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenLib),52,"diskfont.library");
  return FALSE;
 }
 IDiskfont = (struct DiskfontIFace *)GetInterface(DiskfontBase, "main", 1, NULL);
 if(!IDiskfont)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenIFace),"DiskfontIFace");
  return FALSE;
 }

 GfxBase = OpenLibrary("graphics.library", 52);
 if(!GfxBase)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenLib),52,"graphics.library");
  return FALSE;
 }
 IGraphics = (struct GraphicsIFace *)GetInterface(GfxBase, "main", 1, NULL);
 if(!IGraphics)
 {
  PrintError(GetAmiFTPString(Str_ErrorOpenIFace),"GraphicsIFace");
  return FALSE;
 }

	ApplicationLib = OpenLibrary("application.library", 52);
	if(ApplicationLib)
		IApplication = (struct ApplicationIFace *)GetInterface(ApplicationLib, "application", 2, NULL);

	RexxSysBase = OpenLibrary("rexxsyslib.library", 0L);
	if (RexxSysBase)
		IRexxSys = (struct RexxSysIFace *) GetInterfaceTags(RexxSysBase, "main", 1, TAG_END);
	if (!RexxSysBase || !IRexxSys)
	{
		//if (RexxSysBase) CloseLibrary(RexxSysBase);

		PrintError(GetAmiFTPString(Str_ErrorOpenLib), 0, "rexxsyslib.library");
		//CleanUp();
		//exit(10);
		return(FALSE);
	}

	AmigaGuideBase = OpenLibrary("amigaguide.library", 36);
	if (AmigaGuideBase)
		IAmigaGuide = (struct AmigaGuideIFace *) GetInterfaceTags(AmigaGuideBase, "main", 1, TAG_END);
	if (!AmigaGuideBase || !IAmigaGuide)
	{
		//if (AmigaGuideBase) CloseLibrary(AmigaGuideBase);

		PrintError(GetAmiFTPString(Str_ErrorOpenLib), 36, "amigaguide.library");
		//CleanUp();
		//exit(10);
		return(FALSE);
	}

	AppPort = (struct MsgPort *) AllocSysObjectTags(ASOT_PORT, TAG_DONE);
	if (!AppPort)
	{
		PrintError(GetAmiFTPString(Str_ErrorNoPort));//ErrorNoPort);
		//CleanUp();
		//exit(10);
		return(FALSE);
	}

	TimerPort = (struct MsgPort *) AllocSysObjectTags(ASOT_PORT, TAG_DONE);
	if (!TimerPort)
	{
		PrintError(GetAmiFTPString(Str_ErrorNoPort));//ErrorNoPort);
		//CleanUp();
		//exit(10);
		return(FALSE);
	}

	if (!(TimeRequest = (struct TimeRequest *) AllocSysObjectTags(ASOT_IOREQUEST,
	                                            ASOIOR_Size, sizeof(struct TimeRequest),
	                                            ASOIOR_ReplyPort, TimerPort, TAG_END)))
	{
		PrintError(GetAmiFTPString(Str_ErrorTimerDevice));//"Couldn't allocate timerreq");
		//CleanUp();
		//exit(10);
		return(FALSE);
	}

	if (OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest*)TimeRequest, 0))
	{
		PrintError(GetAmiFTPString(Str_ErrorTimerDevice));//"Couldn't open timer.device");
		//CleanUp();
		//exit(10);
		return(FALSE);
	}

	TimerBase = TimeRequest->Request.io_Device;
	ITimer = (struct TimerIFace *) GetInterface((struct Library *) TimerBase, (CONST_STRPTR) "main", 1, NULL);

//should open required classes here too
	TextEditorBase = OpenClass("gadgets/texteditor.gadget", 52, &TextEditorClass);
	ScrollerBase   = OpenClass("gadgets/scroller.gadget", 52, &ScrollerClass);
	RequesterBase = OpenClass("requester.class", 52, &RequesterClass);
	GetFileBase   = OpenClass("gadget/getfile.gadget", 52, &GetFileClass);
	GetFontBase   = OpenClass("gadget/getfont.gadget", 52, &GetFontClass);
	StringBase    = OpenClass("gadgets/string.gadget", 52, &StringClass);
	IntegerBase   = OpenClass("gadgets/integer.gadget", 52, &IntegerClass);
	CheckBoxBase  = OpenClass("gadgets/checkbox.gadget", 52, &CheckBoxClass);
	FuelGaugeBase = OpenClass("gadgets/fuelgauge.gadget", 52, &FuelGaugeClass);
	ButtonBase    = OpenClass("gadgets/button.gadget", 52, &ButtonClass);
	BitMapBase    = OpenClass("images/bitmap.image", 52, &BitMapClass);
	LabelBase     = OpenClass("images/label.image", 52, &LabelClass);
	WindowBase    = OpenClass("window.class", 52, &WindowClass);

 //ARexxBase = OpenLibrary("arexx.class", 52);
 ARexxBase = (struct Library *)OpenClass("arexx.class", 53, &ARexxClass);
 IARexx    = (struct ARexxIFace *)GetInterface(ARexxBase, "main", 1, NULL);
	ListBrowserBase = (struct Library *)OpenClass("gadgets/listbrowser.gadget", 52, &ListBrowserClass);
	IListBrowser    = (struct ListBrowserIFace *)GetInterface( (struct Library *)ListBrowserBase, "main", 1, NULL );
	ClickTabBase = (struct Library *)OpenClass("gadgets/clicktab.gadget", 52, &ClickTabClass);
	IClickTab    = (struct ClickTabIFace *)GetInterface( (struct Library *)ClickTabBase, "main", 1, NULL );
	LayoutBase = (struct Library *)OpenClass("gadgets/layout.gadget", 52, &LayoutClass);
	ILayout    = (struct LayoutIFace *)GetInterface( (struct Library *)LayoutBase, "main", 1, NULL );
	ChooserBase = (struct Library *)OpenClass("gadgets/chooser.gadget", 52, &ChooserClass);
	IChooser    = (struct ChooserIFace *)GetInterface( (struct Library *)ChooserBase, "main", 1, NULL );
	SpeedBarBase = (struct Library *)OpenClass("gadgets/speedbar.gadget", 52, &SpeedBarClass);
	ISpeedBar    = (struct SpeedBarIFace *)GetInterface( (struct Library *)SpeedBarBase, "main", 1, NULL );

	if(!IARexx || !IListBrowser || !IClickTab  || !ILayout || !IChooser
	   || !ISpeedBar) return FALSE;
	/*struct Locale *currentLocale = OpenLocale(NULL);
	if (currentLocale)
	{
		if (currentLocale->loc_GroupSeparator)
			decimalSeperator = currentLocale->loc_GroupSeparator[0];
		if (currentLocale->loc_Grouping)
			separatorSize = *currentLocale->loc_Grouping;
		CloseLocale(currentLocale);
	}

	OpenAmiFTPCatalog(NULL, NULL);

	SetupLocaleStrings();*/

	return(TRUE);
}

void CleanUp(void)
{
	CloseTCP();

#define FREE(x) if (x) free(x)
	FREE(MainPrefs.mp_LocalDir);
	FREE(MainPrefs.mp_AnonymousPW);
	FREE(MainPrefs.mp_LoginName);
	FREE(MainPrefs.mp_DefaultSite);
	FREE(MainPrefs.mp_ProxyServer);
	FREE(MainPrefs.mp_ViewCommand);
	FREE(MainPrefs.mp_HideADTPattern);
	FREE(MainPrefs.mp_FontName);
	FREE(MainPrefs.mp_ListFontName);
	FREE(MainPrefs.mp_PubScreen);
#undef FREE

//Close CLASSES
 DropInterface( (struct Interface *)IARexx );
 CloseLibrary(ARexxBase);

	if(IIntuition) {
		CloseClass(ScrollerBase);
		CloseClass(TextEditorBase);
		CloseClass(GetFileBase);
		CloseClass(GetFontBase);
		CloseClass(RequesterBase);
		CloseClass(IntegerBase);
		CloseClass(StringBase);
		CloseClass(LabelBase);
		CloseClass(BitMapBase);
		CloseClass(ButtonBase);
		CloseClass(FuelGaugeBase);
		CloseClass(CheckBoxBase);
		CloseClass(WindowBase);

		DropInterface( (struct Interface *)ISpeedBar );
		CloseClass( (struct ClassLibrary *)SpeedBarBase );
		DropInterface( (struct Interface *)IChooser );
		CloseClass( (struct ClassLibrary *)ChooserBase );
		DropInterface( (struct Interface *)ILayout );
		CloseClass( (struct ClassLibrary *)LayoutBase );
		DropInterface( (struct Interface *)IClickTab );
		CloseClass( (struct ClassLibrary *)ClickTabBase );
		DropInterface( (struct Interface *)IListBrowser );
		CloseClass( (struct ClassLibrary *)ListBrowserBase );
	}

//Close LIBS
	if(IApplication)
	{
		UnregisterApplication(appID, TAG_DONE);
		DropInterface( (struct Interface *)IApplication );
		CloseLibrary(ApplicationLib);
	}

	if(ITimer) { DropInterface((struct Interface *) ITimer); }
	if (TimeRequest)
	{
		if (TimeRequest->Request.io_Device) { CloseDevice((struct IORequest*)TimeRequest); }

		FreeSysObject(ASOT_IOREQUEST, TimeRequest);
		TimeRequest = NULL;
	}
	if (TimerPort)
	{
		FreeSysObject(ASOT_PORT, TimerPort);
		TimerPort = NULL;
	}

	if (AmigaGuideBase)
	{
		if (ag.ag_AmigaGuide) { CloseAmigaGuide(ag.ag_AmigaGuide); }

		DropInterface((struct Interface *) IAmigaGuide);
		CloseLibrary(AmigaGuideBase);
	}

	CloseAmiFTPCatalog();

	if (ARexx_Object) { DisposeObject(ARexx_Object); }

	if (AppPort)
	{
		FreeSysObject(ASOT_PORT, AppPort);
		AppPort = NULL;
	}

	if (argsptr) { FreeArgs(argsptr); }

	DropInterface( (struct Interface *)IRexxSys );
	CloseLibrary(RexxSysBase);
	DropInterface( (struct Interface *)ILocale );
	CloseLibrary(LocaleBase);
	DropInterface( (struct Interface *)IWorkbench );
	CloseLibrary(WorkbenchBase);
	DropInterface( (struct Interface *)IIcon );
	CloseLibrary(IconBase);
	DropInterface( (struct Interface *)IIFFParse );
	CloseLibrary(IFFParseBase);
	DropInterface( (struct Interface *)IAsl );
	CloseLibrary(AslBase);
	DropInterface( (struct Interface *)IDiskfont );
	CloseLibrary(DiskfontBase);
	DropInterface( (struct Interface *)IGraphics );
	CloseLibrary(GfxBase);
	DropInterface( (struct Interface *)IUtility );
	CloseLibrary(UtilityBase);
	DropInterface( (struct Interface *)IIntuition );
	CloseLibrary(IntuitionBase);
}

void PrintSiteList()
{
	struct SiteNode *sn;
	struct Node *lbn;

	for (lbn = GetHead(&SiteList); lbn != NULL; lbn = GetSucc(lbn))
	{
		GetListBrowserNodeAttrs(lbn, LBNA_UserData, &sn, TAG_DONE);
		if (sn)
		{
			Printf("Site: %s\n", sn->sn_Node.ln_Name);
			Printf("RemoteDir: %s\n", sn->sn_RemoteDir? sn->sn_RemoteDir : "");
			Printf("LocalDir : %s\n", sn->sn_LocalDir? sn->sn_LocalDir : "");
			Printf("Loginname: %s\n", sn->sn_LoginName? sn->sn_LoginName : "");
			Printf("DirString: %s\n", sn->sn_DirString? sn->sn_DirString : "");
		}
	}
}

void GetToolTypes(struct WBStartup *msg)
{
	char *key;
	
	if (msg->sm_NumArgs != 0)
	{
		BPTR olddir;
		struct DiskObject *infoobj;
		olddir = SetCurrentDir(msg->sm_ArgList->wa_Lock);
		if (infoobj = GetDiskObject(msg->sm_ArgList->wa_Name))
		{
			if (key = FindToolType(infoobj->do_ToolTypes, "PUBSCREEN"))
			{
				Strlcpy(CurrentState.ScreenName, key, sizeof(CurrentState.ScreenName));
			}
			if (key = FindToolType(infoobj->do_ToolTypes, "ICONIFIED"))
			{
				CurrentState.Iconified = 1;
			}
			if (key = FindToolType(infoobj->do_ToolTypes, "PORTNAME"))
			{
				Strlcpy(CurrentState.RexxPort, key, sizeof(CurrentState.RexxPort));
			}
			if (key = FindToolType(infoobj->do_ToolTypes, "SETTINGS"))
			{
				ConfigName = strdup(key);
			}
			if (key = FindToolType(infoobj->do_ToolTypes, "AS225"))
			{
				UseAS225 = TRUE;
			}

			FreeDiskObject(infoobj);
		}
		SetCurrentDir(olddir);
	}
}

/*void ConvertFontName(char *dest, int *size, char *source)
{
	char *ptr;
	if ((ptr = strstr(source, "/")))
	{
		*ptr = 0;
		ptr++;
		if (*ptr)
		{
			if (isdigit(*ptr))
			{
				*size = atol(ptr);
				strcpy(dest, source);
			}
		}
	}
}*/

/* EOF */
