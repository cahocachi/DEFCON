/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#import <SDL/SDL.h>
#import "SDLMain.h"
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

#import "universal_include.h"
#import "app.h"
#import "app/globals.h"
#import "lib/preferences.h"
#import "mainmenu.h"
#import "dockicon.h"

/* Use this flag to determine whether we use CPS (docking) or not */
#define		SDL_USE_CPS		1
#ifdef SDL_USE_CPS
/* Portions of CPS.h */
typedef struct CPSProcessSerNum
{
	UInt32		lo;
	UInt32		hi;
} CPSProcessSerNum;

extern OSErr	CPSGetCurrentProcess( CPSProcessSerNum *psn);
extern OSErr 	CPSEnableForegroundOperation( CPSProcessSerNum *psn, UInt32 _arg2, UInt32 _arg3, UInt32 _arg4, UInt32 _arg5);
extern OSErr	CPSSetFrontProcess( CPSProcessSerNum *psn);

#endif /* SDL_USE_CPS */

static int    gArgc;
static char  **gArgv;
static BOOL   gFinderLaunch;
static BOOL   gCalledAppMainline = FALSE;

@implementation SDLApplication
/* The standard -[NSApplication terminate:] mechanism shuts down SDL improperly */
- (void)terminate:(id)sender
{
	AttemptQuitImmediately();
}
@end

// IOKit callback: pass control back to SDLMain object
static void PowerNotificationReceived(void *_delegate, io_service_t _service,
							   natural_t _messageType, void *_messageArg)
{
	[(id)_delegate receivedPowerNotification:_messageType withArgument:_messageArg];
}

/* The main class of the application, the application's delegate */
@implementation SDLMain
- (IBAction)quit:(id)sender
{
	ConfirmExit( NULL );
}

- (IBAction)quitImmediately:(id)sender
{
	AttemptQuitImmediately();
}

- (IBAction)toggleFullscreen:(id)sender
{
	ToggleFullscreenAsync();
}

- (IBAction)openManual:(id)sender
{
	NSString *manual;
	
	manual = [[NSBundle mainBundle] pathForResource:@"DEFCON User Manual" ofType:@"pdf"];
	[[NSWorkspace sharedWorkspace] openFile:manual];
}

/* Set the working directory to the .app's parent directory */
- (void) setupWorkingDirectory
{
	char bundledir[MAXPATHLEN];
	CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());

	if (CFURLGetFileSystemRepresentation(url, true, (UInt8 *)bundledir, MAXPATHLEN)) {
		assert ( chdir (bundledir) == 0 );   /* chdir to the binary app's bundle directory */
		assert ( chdir ("Contents/Resources") == 0 );
	}
	CFRelease(url);
}

/*
 * Catch document open requests...this lets us notice files when the app
 *  was launched by double-clicking a document, or when a document was
 *  dragged/dropped on the app's icon. You need to have a
 *  CFBundleDocumentsType section in your Info.plist to get this message,
 *  apparently.
 *
 * Files are added to gArgv, so to the app, they'll look like command line
 *  arguments. Previously, apps launched from the finder had nothing but
 *  an argv[0].
 *
 * This message may be received multiple times to open several docs on launch.
 *
 * This message is ignored once the app's mainline has been called.
 */
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    const char *temparg;
    size_t arglen;
    char *arg;
    char **newargv;

    if (!gFinderLaunch)  /* MacOS is passing command line args. */
        return FALSE;

    if (gCalledAppMainline)  /* app has started, ignore this document. */
        return FALSE;

    temparg = [filename UTF8String];
    arglen = SDL_strlen(temparg) + 1;
    arg = (char *) SDL_malloc(arglen);
    if (arg == NULL)
        return FALSE;

    newargv = (char **) realloc(gArgv, sizeof (char *) * (gArgc + 2));
    if (newargv == NULL)
    {
        SDL_free(arg);
        return FALSE;
    }
    gArgv = newargv;

    SDL_strlcpy(arg, temparg, arglen);
    gArgv[gArgc++] = arg;
    gArgv[gArgc] = NULL;
    return TRUE;
}


/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status;

    /* Set the working directory to just inside the .app */
    [self setupWorkingDirectory];
	
	[self registerForSleepNotifications];

    /* Hand off to main application code */
    gCalledAppMainline = TRUE;
    status = SDL_main (gArgc, gArgv);

    /* We're done, thank you for playing */
    exit(status);
}

- (void)registerForSleepNotifications
{
	IONotificationPortRef  notifyPort;
	io_object_t notifier;
	
	m_rootPowerDomainPort = IORegisterForSystemPower(self, &notifyPort, PowerNotificationReceived, &notifier);
	if ( m_rootPowerDomainPort != 0 )
	{
		CFRunLoopAddSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(notifyPort),
						   kCFRunLoopCommonModes);
	}
}


// Force ourselves into window mode before going to sleep. This keeps Defcon from
// obstructing the authentication dialog if auth-on-wake is enabled.
- (void)receivedPowerNotification:(natural_t)_type withArgument:(void *)_argument
{
	long notificationID = (long)_argument;
	
	switch (_type)
	{
		// Handling this message is required, but we're effectively ignoring it
		case kIOMessageCanSystemSleep:
			IOAllowPowerChange(m_rootPowerDomainPort, notificationID);
			break;
		
		case kIOMessageSystemWillSleep:
			// Since we have to toggle the setting asynchronously, we poll to see when
			// it's been updated and we can go to sleep.
			if (!g_preferences->GetInt(PREFS_SCREEN_WINDOWED))
			{
				NSDictionary *userInfo;
				
				ToggleFullscreenAsync();
				userInfo = [NSDictionary dictionaryWithObject:[NSNumber numberWithLong:notificationID]
										 forKey:@"notificationID"];
				[NSTimer scheduledTimerWithTimeInterval:0.05 target:self
						 selector:@selector(allowSleepIfReady:)
						 userInfo:userInfo repeats:YES];
			}
			else
				IOAllowPowerChange(m_rootPowerDomainPort, notificationID);
			
			break;
	}
}

- (void)allowSleepIfReady:(NSTimer *)timer
{
	
	if (g_preferences->GetInt(PREFS_SCREEN_WINDOWED, false))
	{
		IOAllowPowerChange(m_rootPowerDomainPort,
						   [[[timer userInfo] objectForKey:@"notificationID"] longValue]);
		[timer invalidate];
	}
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
	DockIcon *dockIcon;
	
	if ( g_app && (dockIcon = (DockIcon *)g_app->GetStatusIcon()) )
	{
		g_app->m_hidden = NO;
		dockIcon->RemoveIcon();
		dockIcon->SetCaption("");
		dockIcon->Update();
	}
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
	return ((DockIcon *)g_app->GetStatusIcon())->GetDockMenu();
}
@end

#ifdef main
#  undef main
#endif

/* Main entry point to executable - should *not* be SDL_main! */
int main (int argc, char **argv)
{
    /* Copy the arguments into a global variable */
    /* This is passed if we are launched by double-clicking */
    if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
        gArgv = (char **) SDL_malloc(sizeof (char *) * 2);
        gArgv[0] = argv[0];
        gArgv[1] = NULL;
        gArgc = 1;
        gFinderLaunch = YES;
    } else {
        int i;
        gArgc = argc;
        gArgv = (char **) SDL_malloc(sizeof (char *) * (argc+1));
        for (i = 0; i <= argc; i++)
            gArgv[i] = argv[i];
        gFinderLaunch = NO;
    }

    NSApplicationMain (argc, (const char **)argv);
    return 0;
}
