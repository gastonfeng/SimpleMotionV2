//Global SimpleMotion functions & definitions
//Copyright (c) Granite Devices Oy

#ifndef SIMPLEMOTION_H
#define SIMPLEMOTION_H

#ifdef WIN32
//dll specs
#ifdef BUILD_DLL
    #define LIB __declspec(dllexport)
#else
//    #define LIB __declspec(dllimport)
#define LIB
#endif
#else
#define LIB
#endif

#include <stdio.h>
#include "simplemotion_defs.h"


#ifdef __cplusplus
extern "C"{
#endif

//possible return values (SM_STATUS type)
#define SM_NONE 0
#define SM_OK 1
#define SM_ERR_NODEVICE 2
#define SM_ERR_BUS 4
#define SM_ERR_COMMUNICATION 8
#define SM_ERR_PARAMETER 16
#define SM_ERR_LENGTH 32

///////////////////////////////////////////////////////////////////////////////////////
//TYPES & VALUES //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//declare SM lib types
typedef long smbus;
typedef unsigned long smuint32;
typedef unsigned short smuint16;
typedef unsigned char smuint8;
typedef long smint32;
typedef short smint16;
typedef char smint8;
typedef char smbool;
#define smtrue 1
#define smfalse 0
typedef int SM_STATUS;
typedef smuint8 smaddr;

//comment out to disable, gives smaller & faster code
#define ENABLE_DEBUG_PRINTS

typedef enum _smVerbosityLevel {Off,Low,Mid,High,Trace} smVerbosityLevel;



//max number of simultaneously opened buses. change this and recompiple SMlib if
//necessary (to increase channels or reduce to save memory)
//#define SM_MAX_BUSES 5
///////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

/** Open SM RS485 communication bus. Parameters:
	-devicename: "USB2VSD" or com port as "COMx" where x=1-16
	-return value: handle to be used with all other commands, -1 if fails
	*/
LIB smbus smOpenBus( const char * devicename );

/** Change baudrate of SM communication port. This does not affect already opened ports but the next smOpenBus will be opened at the new speed. 
	Calling this is optional. By default SM bus and all slave devices operates at 460800 BPS speed.
	Parameters:
	-bps: bus speed in bits per second. for possible choices, see rs232.c (but note that all speeds are not necessarily supported by SM devices)
	Typical usage is:
	- first call smSetParameter(handle,0,SMP_BUS_SPEED,N) to change speed of all connected slaves to N PBS
	- then close port with smCloseBus
	- then call smSetBaudrate(N)
	- then open bus again with smOpenBus
	
	Note that in upcoming SM device firmware versions, bitrate will be reset to default (460800) if device side SM bus watchdog timer has been enabled, and it timeouts.
	This allows re-establishing connection at defautl speed if connection breaks up and SM bus watchdog timeout gets exceeded. To identify is device supports this, 
	read parameter SMP_SM_VERSION. Values above 25 support this feature. Value 25 and below will not reset baudrate.
	
	Note also that SMP_BUS_SPEED will not be saved in device flash memory - it will reset to default at every reset & power on.
	*/
LIB void smSetBaudrate( unsigned long pbs );

/** Set timeout of how long to wait reply packet from bus. Must be set before smOpenBus and cannot be changed afterwards
 * max value 5000ms. In unix this is rounded to 100ms (rounding downwards), so 99 or less gives 0ms timeout.
 *
 *This is the only function that returns SM_STATUS which doesn't accumulate status bits to be read with getCumulativeStatus because it has no bus handle
 */
LIB SM_STATUS smSetTimeout( smuint16 millsecs );

/** Close connection to given bus handle number. This frees communication link therefore makes it available for other apps for opening.
  -return value: a SM_STATUS value, i.e. SM_OK if command succeed
*/
LIB SM_STATUS smCloseBus( const smbus bushandle );


/** Return SM lib version number in hexadecimal format.
Ie V 2.5.1 would be 0x020501 and 1.2.33 0x010233 */
LIB smuint32 smGetVersion();


/** Set stream where debug output is written. By default nothing is written. */
LIB void smSetDebugOutput( smVerbosityLevel level, FILE *stream );

/** This function returns all occurred SM_STATUS bits after smOpenBus or resetCumulativeStatus call*/
LIB SM_STATUS getCumulativeStatus( const smbus handle );
/** Reset cululative status so getCumultiveStatus returns 0 after calling this until one of the other functions are called*/
LIB SM_STATUS resetCumulativeStatus( const smbus handle );


/** SMV2 Device communication functionss */
LIB SM_STATUS smAppendCommandToQueue( smbus handle, smuint8 cmdid, smuint16 param  );
LIB SM_STATUS smExecuteCommandQueue( const smbus bushandle, const smaddr targetaddress );
LIB smuint16  smGetQueuedCommandReturnValue(  const smbus bushandle, smuint16 cmdnumber );

LIB SM_STATUS smUploadCommandQueueToDeviceBuffer( const smbus bushandle, const smaddr targetaddress );
LIB SM_STATUS smBytesReceived( const smbus bushandle, smint32 *bytesinbuffer );

LIB SM_STATUS smAppendSMCommandToQueue( smbus handle, int smpCmdType, smint32 paramvalue  );
LIB SM_STATUS smGetQueuedSMCommandReturnValue(  const smbus bushandle, smint32 *retValue );

LIB SM_STATUS smAppendGetParamCommandToQueue( smbus handle, smint16 paramAddress );
LIB SM_STATUS smGetQueuedGetParamReturnValue(  const smbus bushandle, smint32 *retValue  );
LIB SM_STATUS smAppendSetParamCommandToQueue( smbus handle, smint16 paramAddress, smint32 paramValue );
LIB SM_STATUS smGetQueuedSetParamReturnValue(  const smbus bushandle, smint32 *retValue  );

/** Simple read & write of parameters with internal queueing, so only one call needed.
Use these for non-time critical operations. */
LIB SM_STATUS smRead1Parameter( const smbus handle, const smaddr nodeAddress, const smint16 paramId1, smint32 *paramVal1 );
LIB SM_STATUS smRead2Parameters( const smbus handle, const smaddr nodeAddress, const smint16 paramId1, smint32 *paramVal1,const smint16 paramId2, smint32 *paramVal2 );
LIB SM_STATUS smRead3Parameters( const smbus handle, const smaddr nodeAddress, const smint16 paramId1, smint32 *paramVal1,const smint16 paramId2, smint32 *paramVal2 ,const smint16 paramId3, smint32 *paramVal3 );
LIB SM_STATUS smSetParameter( const smbus handle, const smaddr nodeAddress, const smint16 paramId, smint32 paramVal );


LIB SM_STATUS smGetBufferClock( const smbus handle, const smaddr targetaddr, smuint16 *clock );

/** smFastUpdateCycle uses special SimpleMotion command to perform fast turaround communication. May be used with cyclic real time control. Parameter & return data
 *content are application specific and defined . */
LIB SM_STATUS smFastUpdateCycle( smbus handle, smuint8 nodeAddress, smuint16 write1, smuint16 write2, smuint16 *read1, smuint16 *read2);

#ifdef __cplusplus
}
#endif
#endif // SIMPLEMOTION_H
