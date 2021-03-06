/*  
 *  TimestampTextField.m
 *  MPlayer OSX Extended
 *  
 *  Created on 01.01.2010
 *  
 *  Description:
 *	Controller for the inspector pane.
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#import "TimestampTextField.h"

#import "Preferences.h"

@implementation TimestampTextField
@synthesize displayType;

- (void)awakeFromNib
{
	[self setDisplayMode:[PREFS integerForKey:[self autosaveName]]];
	
	[PREFS addObserver:self
			forKeyPath:[self autosaveName]
			   options:NSKeyValueObservingOptionNew
			   context:nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if ([keyPath isEqualToString:[self autosaveName]]) {
		displayType = [[change objectForKey:NSKeyValueChangeNewKey] integerValue];
		[self updateTimestamp];
	}
}

- (NSString *)autosaveName
{
	return @"MPEPlayerControlsTimestamp";
}

- (void)updateTimestamp
{
	if (displayType == MPETimestampCurrent && lastCurrentTime == 0) {
		[self setStringValue:@"00:00:00"];
		return;
	}
	
	if (displayType == MPETimestampRemaining && lastCurrentTime == 0) {
		[self setStringValue:@"-00:00:00"];
		return;
	}
	
	if (displayType == MPETimestampRemaining && (lastTotalTime == 0 || lastCurrentTime > lastTotalTime)) {
		[self setStringValue:@"-xx:xx:xx"];
		return;
	}
	
	if (displayType == MPETimestampTotal && lastTotalTime == 0) {
		[self setStringValue:@"xx:xx:xx"];
		return;
	}
	
	float seconds;
	
	if (displayType == MPETimestampRemaining)
		seconds = lastCurrentTime - lastTotalTime;
	else if (displayType == MPETimestampTotal)
		seconds = lastTotalTime;
	else
		seconds = lastCurrentTime;
	
	int iseconds = (int)seconds;
	
	NSString *timestamp = [NSString stringWithFormat:@"%0*.0f:%02d:%02d", 
						   seconds < 0 ? 3 : 2,
						   floorf(fabsf(seconds)/3600) * (seconds >= 0 ? 1 : -1),
						   (abs(iseconds)%3600)/60,
						   (abs(iseconds)%60)];
	
	[self setStringValue:timestamp];
}

- (void)setDisplayMode:(MPETimestampDisplayType)mode
{
	displayType = mode;
	[self updateTimestamp];
	
	[PREFS setInteger:displayType forKey:[self autosaveName]];
}

- (void)changeDisplayMode:(id)sender
{
	[self setDisplayType:[sender tag]];
}

- (void)cycleDisplayMode:(id)sender
{
	NSUInteger newType = displayType + 1;
	if (newType > MPETimestampTotal)
		newType = 0;
	
	[self setDisplayMode:newType];
}

- (void)setTimestamptWithCurrentTime:(float)currentTime andTotalTime:(float)totalTime
{
	lastCurrentTime = currentTime;
	lastTotalTime = totalTime;
	[self updateTimestamp];
}

- (void)mouseDown:(NSEvent *)theEvent
{
	[self cycleDisplayMode:self];
	
	[super mouseDown:theEvent];
}

- (NSMenu *)menuForEvent:(NSEvent *)theEvent
{
	NSMenu *context = [[NSMenu new] autorelease];
	
	NSMenuItem *item = [context addItemWithTitle:@"Show Time:"
										  action:nil keyEquivalent:@""];
	[item setEnabled:NO];
	[item setTag:-1];
	
	[[context addItemWithTitle:@"Current"
						action:@selector(changeDisplayMode:) keyEquivalent:@""] setTag:0];
	[[context addItemWithTitle:@"Remaining"
						action:@selector(changeDisplayMode:) keyEquivalent:@""] setTag:1];
	[[context addItemWithTitle:@"Total"
						action:@selector(changeDisplayMode:) keyEquivalent:@""] setTag:2];
	[[context itemWithTag:displayType] setState:NSOnState];
	
	return context;
}

@end
