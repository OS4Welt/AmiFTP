/* RCS Id:  $Id: main.c 1.802 1996/11/30 13:15:06 lilja Exp lilja $
   Locked version: $Revision: 1.802 $
*/

#include "AmiFTP.h"
#include "gui.h"

extern struct TextAttr AmiFTPAttr, ListViewAttr;
extern Object *ARexx_Object;

BOOL InitRexx(void);
void PrintSiteList(void);
void GetToolTypes(struct WBStartup *msg);
void SetupLocaleStrings(void);
void ConvertFontName(char *dest, int *size, char *source);

extern struct Screen *myScn;

char decimalSeperator = '.';
uint8 seperatorSize = 3;

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

char *ErrorOpenLib = "Couldn't open version %ld of %s.";
char *ErrorNoPort = "Couldn't open messageport.";

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
		req.es_Title = "AmiFTP Error";
		req.es_TextFormat = errmsg;
		req.es_GadgetFormat = "Ok";
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



extern struct ListBrowserIFace *IListBrowser;
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
extern struct GadToolsIFace *IGadTools;

BOOL UseAS225 = FALSE;
static struct Process *ME;

int main(int argc, char **argv)
{
	struct servent *servent;
	APTR oldwptr;



	if (!IDOS || !IListBrowser || !ILayout || !ILabel || !IWindow || !IChooser || !IString || !IClickTab || !ICheckBox || !IBevel || !IARexx || !ISpeedBar
		|| !IFuelGauge || !IInteger || !IGadTools)
		return 10;

	memset(&MainPrefs, 0, sizeof(MainPrefs));
	memset(&CurrentState, 0, sizeof(CurrentState));
	ME = (struct Process *) FindTask(NULL);
	oldwptr = ME->pr_WindowPtr;

	MyOpenLibs();
	ag.ag_NAG.nag_BaseName = "AmiFTP";
	ag.ag_NAG.nag_Name = "AmiFTP.guide";
	ag.ag_NAG.nag_ClientPort = "AMIFTP_HELP";
	ag.ag_NAG.nag_Context = context;

	MainPrefs.mp_ViewCommand = strdup("WBRUN %F");
	MainPrefs.mp_CacheSize = 5;
	MainPrefs.mp_BufferSize = 8192;
	MainPrefs.mp_DeleteFiles = TRUE;
	MainPrefs.mp_UseDefaultFonts = TRUE;
	MainPrefs.mp_OpenOnDefaultScreen = TRUE;
	MainPrefs.mp_ShowToolBar = TRUE;

	if (!argc)
	{
		//WB2CLI(_WBenchMsg, __stack, DOSBase);
		wBenchMsg = (struct WBStartup *) argv;
		GetToolTypes(wBenchMsg);
	}
	else
	{
		argsptr = ReadArgs(TEMPLATE, opts, NULL);
		if (argsptr == NULL)
		{
			PrintFault(IoErr(), NULL);
			CleanUp();
			exit(1);
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
		exit(10);
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
		exit(1);
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
			if (lock = Lock(ConfigName, ACCESS_READ))
			{
				UnLock(lock);
				ReadConfigFile(ConfigName);
			}
		}
		else
		{
			if (lock = Lock(AMIFTPPREFS, ACCESS_READ))
			{
				UnLock(lock);
				ConfigName = strdup(AMIFTPPREFS);
				ReadConfigFile(ConfigName);
			}
			else if (lock = Lock(PROGDIRPREFS, ACCESS_READ))
			{
				UnLock(lock);
				ConfigName = strdup(PROGDIRPREFS);
				ReadConfigFile(ConfigName);
			}
			else if (lock = Lock(ENVPREFS, ACCESS_READ))
			{
				UnLock(lock);
				ConfigName = strdup(ENVPREFS);
				ReadConfigFile(ConfigName);
			}
			else if (dirname[0] && (lock = Lock(dirname, ACCESS_READ)))
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
		char *a = strdup(cliargs->pubscreen);
		MainPrefs.mp_PubScreen = a;
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
}


struct IntuitionIFace *IIntuition = NULL;
struct UtilityIFace *IUtility = NULL;
struct GraphicsIFace *IGraphics = NULL;
struct DiskfontIFace *IDiskfont = NULL;
struct AslIFace *IAsl = NULL;
struct IFFParseIFace *IIFFParse = NULL;
struct IconIFace *IIcon = NULL;
struct RexxSysIFace *IRexxSys = NULL;
struct WorkbenchIFace *IWorkbench = NULL;
struct LocaleIFace *ILocale = NULL;
struct AmigaGuideIFace *IAmigaGuide = NULL;
struct TimerIFace *ITimer = NULL;



void MyOpenLibs()
{
	char *lib;

	lib = "intuition.library";
	IntuitionBase = OpenLibrary(lib, 36);
	if (IntuitionBase)
		IIntuition = (struct IntuitionIFace *) GetInterfaceTags(IntuitionBase, "main", 1, TAG_END);

	if (!IntuitionBase || !IIntuition)
	{

		if (IntuitionBase)
			CloseLibrary(IntuitionBase);

		PrintError(ErrorOpenLib, 36, lib);
		CleanUp();
		exit(10);
	}

	LocaleBase = OpenLibrary("locale.library", 38L);

	if (LocaleBase)
		ILocale = (struct LocaleIFace *) GetInterfaceTags(LocaleBase, "main", 1, TAG_END);

	if (!LocaleBase || !ILocale)
	{

		if (LocaleBase)
			CloseLibrary(LocaleBase);

		PrintError(ErrorOpenLib, 38, lib);
		CleanUp();
		exit(10);
	}

	lib = "graphics.library";
	GfxBase = OpenLibrary(lib, 36);

	if (GfxBase)
		IGraphics = (struct GraphicsIFace *) GetInterfaceTags(GfxBase, "main", 1, TAG_END);

	if (!GfxBase || !IGraphics)
	{

		if (GfxBase)
			CloseLibrary(GfxBase);
		PrintError(ErrorOpenLib, 36, lib);
		CleanUp();
		exit(10);
	}

	lib = "utility.library";
	UtilityBase = OpenLibrary(lib, 36);

	if (UtilityBase)
		IUtility = (struct UtilityIFace *) GetInterfaceTags(UtilityBase, "main", 1, TAG_END);

	if (!UtilityBase || !IUtility)
	{

		if (UtilityBase)
			CloseLibrary(UtilityBase);

		PrintError(ErrorOpenLib, 36, lib);
		CleanUp();
		exit(10);
	}

	lib = "diskfont.library";
	DiskfontBase = OpenLibrary(lib, 36);

	if (DiskfontBase)
		IDiskfont = (struct DiskfontIFace *) GetInterfaceTags(DiskfontBase, "main", 1, TAG_END);

	if (!DiskfontBase || !IDiskfont)
	{
		if (DiskfontBase)
			CloseLibrary(DiskfontBase);
		PrintError(ErrorOpenLib, 36, lib);
		CleanUp();
		exit(10);
	}

	lib = "icon.library";
	IconBase = OpenLibrary(lib, 36);

	if (IconBase)
		IIcon = (struct IconIFace *) GetInterfaceTags(IconBase, "main", 1, TAG_END);

	if (!IconBase || !IIcon)
	{
		if (IconBase)
			CloseLibrary(IconBase);

		PrintError(ErrorOpenLib, 36, lib);
		CleanUp();
		exit(10);
	}
	lib = "workbench.library";
	WorkbenchBase = OpenLibrary(lib, 36);
	if (WorkbenchBase)
		IWorkbench = (struct WorkbenchIFace *) GetInterfaceTags(WorkbenchBase, "main", 1, TAG_END);

	if (!WorkbenchBase || !IWorkbench)
	{
		if (WorkbenchBase)
			CloseLibrary(WorkbenchBase);
		PrintError(ErrorOpenLib, 36, lib);
		CleanUp();
		exit(10);
	}

	/*
	   lib="reqtools.library";
	   ReqToolsBase=(struct ReqToolsBase *)OpenLibrary(lib, 38);
	   if (!ReqToolsBase) {
	   PrintError(ErrorOpenLib, 38, lib);
	   CleanUp();
	   exit(10);
	   }
	 */

	lib = "asl.library";
	AslBase = OpenLibrary(lib, 36);

	if (AslBase)
		IAsl = (struct AslIFace *) GetInterfaceTags(AslBase, "main", 1, TAG_END);

	if (!AslBase || !IAsl)
	{
		if (AslBase)
			CloseLibrary(AslBase);
		PrintError(ErrorOpenLib, 36, lib);
		CleanUp();
		exit(10);
	}

	lib = "iffparse.library";
	IFFParseBase = OpenLibrary(lib, 36);
	if (IFFParseBase)
		IIFFParse = (struct IFFParseIFace *) GetInterfaceTags(IFFParseBase, "main", 1, TAG_END);

	if (!IFFParseBase || !IIFFParse)
	{
		if (IFFParseBase)
			CloseLibrary(IFFParseBase);
		PrintError(ErrorOpenLib, 36, lib);
		CleanUp();
		exit(10);
	}


	lib = "rexxsyslib.library";
	RexxSysBase = OpenLibrary(lib, 0L);
	if (RexxSysBase)
		IRexxSys = (struct RexxSysIFace *) GetInterfaceTags(RexxSysBase, "main", 1, TAG_END);

	if (!RexxSysBase || !IRexxSys)
	{
		if (RexxSysBase)
			CloseLibrary(RexxSysBase);
		PrintError(ErrorOpenLib, 45, lib);
		CleanUp();
		exit(10);
	}

	AmigaGuideBase = OpenLibrary("amigaguide.library", 36);
	if (AmigaGuideBase)
		IAmigaGuide = (struct AmigaGuideIFace *) GetInterfaceTags(AmigaGuideBase, "main", 1, TAG_END);

	if (!AmigaGuideBase || !IAmigaGuide)
	{
		if (AmigaGuideBase)
			CloseLibrary(AmigaGuideBase);
		PrintError(ErrorOpenLib, 0, lib);
		CleanUp();
		exit(10);
	}

	AppPort = (struct MsgPort *) CreateMsgPort();
	if (!AppPort)
	{
		PrintError(ErrorNoPort);
		CleanUp();
		exit(10);
	}

	TimerPort = (struct MsgPort *) CreateMsgPort();
	if (!TimerPort)
	{
		PrintError(ErrorNoPort);
		CleanUp();
		exit(10);
	}
	if (!(TimeRequest = (struct TimeRequest *) CreateIORequest(TimerPort, sizeof(struct TimeRequest))))
	{
		PrintError("Couldn't allocate timerreq");
		CleanUp();
		exit(10);
	}

	if (OpenDevice("timer.device", UNIT_VBLANK, TimeRequest, 0))
	{
		PrintError("Couldn't open timer.device");
		CleanUp();
		exit(10);
	}

	TimerBase = &TimeRequest->Request.io_Device->dd_Library;

	ITimer = (struct TimerIFace *) GetInterface((struct Library *) TimerBase, (CONST_STRPTR) "main", 1, NULL);

    struct Locale *currentLocale = OpenLocale(NULL);
    if (currentLocale)
    {
        if (currentLocale->loc_GroupSeparator)
        	decimalSeperator = currentLocale->loc_GroupSeparator[0];
        if (currentLocale->loc_Grouping)
        	seperatorSize = *currentLocale->loc_Grouping;
        CloseLocale(currentLocale);
    }
	OpenAmiFTPCatalog(NULL, NULL);

	SetupLocaleStrings();
}

void CleanUp()
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


	if (ITimer)
	{
		DropInterface((struct Interface *) ITimer);
		ITimer = NULL;

	}

	if (TimeRequest)
	{
		if (TimeRequest->Request.io_Device)
			CloseDevice(TimeRequest);

		DeleteIORequest(TimeRequest);
		TimeRequest = NULL;
	}
	if (TimerPort)
	{
		DeleteMsgPort(TimerPort);
		TimerPort = NULL;
	}

	if (AmigaGuideBase)
	{
		if (ag.ag_AmigaGuide)
			CloseAmigaGuide(ag.ag_AmigaGuide);
		CloseLibrary(AmigaGuideBase);
	}

	CloseAmiFTPCatalog();


	if (ILocale)
		DropInterface((struct Interface *) ILocale);

	if (LocaleBase)
		CloseLibrary(LocaleBase);

	if (ARexx_Object)
		DisposeObject(ARexx_Object);

	if (AppPort)
		DeleteMsgPort(AppPort);


	if (IRexxSys)
		DropInterface((struct Interface *) IRexxSys);
	if (RexxSysBase)
		CloseLibrary((struct Library *) RexxSysBase);

	if (argsptr)
	{
		FreeArgs(argsptr);
	}

	if (IWorkbench)
		DropInterface((struct Interface *) IWorkbench);

	if (WorkbenchBase)
		CloseLibrary((struct Library *) WorkbenchBase);

	if (IIntuition)
		DropInterface((struct Interface *) IIntuition);
	if (IntuitionBase)
		CloseLibrary((struct Library *) IntuitionBase);

	if (IGraphics)
		DropInterface((struct Interface *) IGraphics);
	if (GfxBase)
		CloseLibrary((struct Library *) GfxBase);

	if (IUtility)
		DropInterface((struct Interface *) IUtility);
	if (UtilityBase)
		CloseLibrary((struct Library *) UtilityBase);

	if (IDiskfont)
		DropInterface((struct Interface *) IDiskfont);
	if (DiskfontBase)
		CloseLibrary((struct Library *) DiskfontBase);

	if (IAsl)
		DropInterface((struct Interface *) IAsl);
	if (AslBase)
		CloseLibrary((struct Library *) AslBase);

	if (IIFFParse)
		DropInterface((struct Interface *) IIFFParse);
	if (IFFParseBase)
		CloseLibrary((struct Library *) IFFParseBase);

	if (IIcon)
		DropInterface((struct Interface *) IIcon);
	if (IconBase)
		CloseLibrary((struct Library *) IconBase);
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
			Printf("RemoteDir: %s\n", sn->sn_RemoteDir ? sn->sn_RemoteDir : "");
			Printf("LocalDir: %s\n", sn->sn_LocalDir ? sn->sn_LocalDir : "");
			Printf("Loginname: %s\n", sn->sn_LoginName ? sn->sn_LoginName : "");
			Printf("DirString: %s\n", sn->sn_DirString ? sn->sn_DirString : "");
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
				strncpy(CurrentState.ScreenName, key, 255);
			}
			if (key = FindToolType(infoobj->do_ToolTypes, "ICONIFIED"))
			{
				CurrentState.Iconified = 1;
			}
			if (key = FindToolType(infoobj->do_ToolTypes, "PORTNAME"))
			{
				strncpy(CurrentState.RexxPort, key, 50);
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

void ConvertFontName(char *dest, int *size, char *source)
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
}

/* EOF */
