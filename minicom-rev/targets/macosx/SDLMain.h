/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#import <Cocoa/Cocoa.h>
#import <IOKit/IOMessage.h>
#import <IOKit/pwr_mgt/IOPMLib.h>

@interface SDLApplication : NSApplication
- (void)terminate:(id)sender;
@end

@interface SDLMain : NSObject
{
	IBOutlet NSMenu *m_helpMenu;
	io_connect_t m_rootPowerDomainPort;
}
- (IBAction)toggleFullscreen:(id)sender;
- (IBAction)quit:(id)sender;
- (IBAction)quitImmediately:(id)sender;
- (IBAction)openManual:(id)sender;
- (void)registerForSleepNotifications;
- (void)allowSleepIfReady:(NSTimer *)timer;

// delegate method
- (void)receivedPowerNotification:(natural_t)_type withArgument:(void *)_argument;
@end

