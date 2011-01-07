/*
 *  MPlayerInterface.h
 *  MPlayer OS X
 *
 *	version 1.1
 *
 *	Description:
 *		Interface to MPlayer binary application that is supposed to simplify the access
 *	to the application controlls and state values while handling inconsistent states of
 *	player
 *
 *  Created by Jan Volf
 *	<javol@seznam.cz>
 
 *  Copyright (c) 2003 Jan Volf. All rights reserved.
 */


#import <Cocoa/Cocoa.h>
#import "MovieInfo.h"

#import "Debug.h"

@class MPlayerInterface;

@protocol MPlayerInterfaceClientProtocol <NSObject>
@optional
- (void) interface:(MPlayerInterface *)mi hasChangedStateTo:(NSNumber *)newState fromState:(NSNumber *)oldState;
- (void) interface:(MPlayerInterface *)mi timeUpdate:(NSNumber *)newTime;
- (void) interface:(MPlayerInterface *)mi streamUpate:(MovieInfo *)item;
- (void) interface:(MPlayerInterface *)mi hasSelectedStream:(NSNumber *)streamId ofType:(NSNumber *)type;
- (void) interface:(MPlayerInterface *)mi statsUpdate:(NSDictionary *)stats;
- (void) interface:(MPlayerInterface *)mi volumeUpdate:(NSNumber *)volume isMuted:(NSNumber *)muted;
@end

enum {
	MIStateFinished,
	MIStateStopped,
	MIStateError,
	MIStatePlaying,
	MIStatePaused,
	MIStateOpening,
	MIStateBuffering,
	MIStateIndexing,
	MIStateInitializing,
	MIStateSeeking
};
typedef NSUInteger MIState;

enum {
	// Play/Paused masks dividing all states to either playing or paused
	MIStatePPPlayingMask = (1<<MIStatePlaying|1<<MIStateOpening|1<<MIStateBuffering
						   |1<<MIStateIndexing),
	MIStatePPPausedMask  = (1<<MIStatePaused|1<<MIStateStopped|1<<MIStateFinished|1<<MIStateError),
	// Mask for initializing states
	MIStateStartupMask   = (1<<MIStateOpening|1<<MIStateBuffering|1<<MIStateIndexing|1<<MIStateInitializing), 
	// Mask to extend playing state to seeking
	MIStatePlayingMask   = (1<<MIStatePlaying|1<<MIStateSeeking),
	// States in which MPlayer can seek
	MIStateCanSeekMask   = (1<<MIStatePlaying|1<<MIStatePaused),
	// States in which MPlayer is not running
	MIStateStoppedMask   = (1<<MIStateStopped|1<<MIStateFinished|1<<MIStateError),
	// Intermediate Progress Mask
	MIStateIntermediateMask = (1<<MIStateOpening|1<<MIStateBuffering|1<<MIStateInitializing),
	// Absolute Progress Mask
	MIStatePositionMask  = (1<<MIStateIndexing|1<<MIStatePlaying|1<<MIStateSeeking|1<<MIStatePaused),
	// MPlayer is able to respond to commands
	MIStateRespondMask   = (1<<MIStatePlaying|1<<MIStatePaused|1<<MIStateSeeking)
};

enum {
	MISeekingModeRelative,
	MISeekingModePercent,
	MISeekingModeAbsolute
};
typedef NSUInteger MISeekingMode;

enum {
	MISurpressCommandOutputAlways,
	MISurpressCommandOutputConditionally,
	MISurpressCommandOutputNever
};
typedef NSUInteger MICommandOutputSurpression;

enum {
	MICommandPausingNone,
	MICommandPausingKeep,
	MICommandPausingToggle,
	MICommandPausingKeepForce
};
typedef NSUInteger MICommandPausingMode;

extern NSString* const MIStatsStatusStringKey;
extern NSString* const MIStatsCPUUsageKey;
extern NSString* const MIStatsAudioCPUUsageKey;
extern NSString* const MIStatsVideoCPUUsageKey;
extern NSString* const MIStatsCacheUsageKey;
extern NSString* const MIStatsAVSyncKey;
extern NSString* const MIStatsDroppedFramesKey;

@interface MPlayerInterface : NSObject
{	
	// Properties
	// file paths
	NSString *myPathToPlayer;
	NSMutableArray *mySubtitlesFiles;
	
	MovieInfo *playingItem;
	
	NSDictionary *localPrefs;
	NSDictionary *prefs;
	
	NSString *buffer_name;
	
	// playback
	BOOL osdSilenced;
	int numberOfThreads;
	
	BOOL disableAppleRemote;
	
	// display
	NSString *screenshotPath;
	
	// text
	int osdLevel;
	
	// volume
	float playerVolume;
	BOOL playerMute;
	
	// properties
	BOOL isPreflight;
	
	// state variables
	MIState	state;
	unsigned int stateMask;
	MIState stateBeforeSeeking;
	BOOL playing;
	BOOL movieOpen;
	
	float mySeconds;
	float restartTime;
	
	// statistics
	BOOL myUpdateStatistics;
	float mySyncDifference;
	int myCPUUsage;
	float myAudioCPUUsage;
	int myVideoCPUUsage;
	int myCacheUsage;
	int	myDroppedFrames;
	int myPostProcLevel;
	
	// internal use
	NSTask *myMplayerTask;
	double myLastUpdate;
	BOOL restartingPlayer;
	BOOL pausedOnRestart;
	BOOL isRunning;
	BOOL isReading;
	NSMutableArray *myCommandsBuffer;
	int subtitleFileId;
	NSDictionary *lastMissedSeek;
	BOOL force32bitBinary;
	
	NSMutableArray *clients;
}

@property (nonatomic,getter=isPlaying) BOOL playing;
@property (nonatomic,getter=isMovieOpen) BOOL movieOpen;

@property (nonatomic,readonly) MIState state;
@property (nonatomic,readonly) unsigned int stateMask;

- (void) addClient:(id<MPlayerInterfaceClientProtocol>)client;
- (void) removeClient:(id<MPlayerInterfaceClientProtocol>)client;
- (void) notifyClientsWithSelector:(SEL)selector andObject:(id)object;
- (void) notifyClientsWithSelector:(SEL)selector andObject:(id)object andObject:(id)otherObject;

- (void) setBufferName:(NSString *)name;

- (void) playItem:(MovieInfo *)item;
- (void) play;
- (void) registerPlayingItem:(MovieInfo *)item;
- (void) unregisterPlayingItem;

- (void) stop;
- (void) pause;
- (void) seek:(float)seconds mode:(int)aMode;
- (void) seek:(float)seconds mode:(int)aMode force:(BOOL)forced;

- (void) setVolume:(float)volume isMuted:(BOOL)muted;
- (void) applyVolume;
- (void) takeScreenshot;

- (void) loadNewSubtitleFile:(NSNotification *)notification;
- (void) applyVideoEqualizer;

- (void) applySettingsWithRestart;

// info
- (void) loadInfo:(MovieInfo *)item;
- (MovieInfo *) info;

- (float) seconds;
- (BOOL) changesNeedRestart;
- (BOOL) localChangesNeedRestart;

- (void) setState:(MIState)newState;
- (BOOL) isRunning;

// statistics
- (void) setUpdateStatistics:(BOOL)aBool;
- (void) resetStatistics;

// advanced
- (void)sendCommand:(NSString *)aCommand withOSD:(uint)osdMode andPausing:(uint)pausing;
- (void)sendCommand:(NSString *)aCommand;
- (void)sendCommands:(NSArray *)aCommands withOSD:(uint)osdMode andPausing:(uint)pausing;
- (void)sendCommands:(NSArray *)aCommands;
- (void)runMplayerWithParams:(NSMutableArray *)aParams;
- (void)sendToMplayersInput:(NSString *)aCommand;
- (void)terminateMplayer;

- (int)mplayerOSDLevel;
- (void)reactivateOsdAfterDelay;
- (void)reactivateOsd;

// notification handlers
- (void) mplayerTerminated;
- (void) mplayerTermiantedAndFinishedReading;
- (void) readError:(NSNotification *)notification;
- (void) readOutputC:(NSNotification *)notification;

@end
