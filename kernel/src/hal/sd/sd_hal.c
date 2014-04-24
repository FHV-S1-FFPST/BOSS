/*
 * sd_hal.c
 *
 *  Created on: 16.04.2014
 *      Author: Thaler
 */

#include "sd_hal.h"

#include "../../common/common.h"
#include "../../core/core.h"

// module-local data //////////////////////////////////////////////
// module-local functions /////////////////////////////////////////
// INITIALZATION & IDENTIFICATION
static void enableIfaceAndFunctionalClock( void );
static void softwareReset( void );
static void voltageCapInit( void );
static void defaultInit( void );
static void	initProcedureStart( void );
static void	preCardIdentificationConfig( void );
static uint32_t identify( void );

// ACCESS TO CARD
static uint32_t readBlock( uint32_t address, uint8_t* buffer );
static uint32_t sendCommand( uint8_t cmdId, uint32_t arg );

//static void configIdleAndWakeup( void );
//static void configControllerBus( void );

// HELPERS
static uint8_t waitForStatCC( void );
static uint32_t isCardBusy( void );
static void awaitDataLineAvailable( void );
static void awaitBufferReadReady( void );
static uint32_t isTransferComplete( void );
static void resetMMCIDataLine( void );
static void resetMMCICmdLine( void );
static void awaitCommandLineAvailable( void );
static uint32_t awaitCommandResponse( void );

// COMMANDS USED DURING INITIALZATION & IDENTIFICATION
static uint32_t sendCmd0( void );
static uint32_t sendCmd1( void );
static uint32_t sendCmd2( void );
static uint32_t sendCmd3( void );
static uint32_t sendCmd5( void );
static uint32_t sendCmd7( void );
static uint32_t sendCmd8( void );
static uint32_t sendCmd16( void );
static uint32_t sendCmd18( uint32_t addr );
static uint32_t sendCmd23( void );
static uint32_t sendCmd55( void );
static uint32_t sendACmd41( void );
///////////////////////////////////////////////////////////////////

// NOTE: omap35x.pdf page 3189

#define MMCHS1_ADDR			0x4809C000
#define MMCHS2_ADDR			0x480B4000
#define MMCHS3_ADDR			0x480AD000

#define SELECTED_CHS		MMCHS1_ADDR

#define MMCHS_SYSCONFIG( addr )		READ_REGISTER_OFFSET( addr, 0x010 ) 	// RW
#define MMCHS_SYSSTATUS( addr )		READ_REGISTER_OFFSET( addr, 0x014 ) 	// R
#define MMCHS_CSRE( addr )			READ_REGISTER_OFFSET( addr, 0x024 ) 	// RW
#define MMCHS_SYSTEST( addr )		READ_REGISTER_OFFSET( addr, 0x028 ) 	// RW
#define MMCHS_CON( addr )			READ_REGISTER_OFFSET( addr, 0x02C ) 	// RW
#define MMCHS_PWCNT( addr )			READ_REGISTER_OFFSET( addr, 0x030 ) 	// RW
#define MMCHS_BLK( addr )			READ_REGISTER_OFFSET( addr, 0x104 ) 	// RW
#define MMCHS_ARG( addr )			READ_REGISTER_OFFSET( addr, 0x108 ) 	// RW
#define MMCHS_CMD( addr )			READ_REGISTER_OFFSET( addr, 0x10C ) 	// RW
#define MMCHS_RSP10( addr )			READ_REGISTER_OFFSET( addr, 0x110 ) 	// R
#define MMCHS_RSP32( addr )			READ_REGISTER_OFFSET( addr, 0x114 ) 	// R
#define MMCHS_RSP54( addr )			READ_REGISTER_OFFSET( addr, 0x118 ) 	// R
#define MMCHS_RSP76( addr )			READ_REGISTER_OFFSET( addr, 0x11C ) 	// R
#define MMCHS_DATA( addr )			READ_REGISTER_OFFSET( addr, 0x120 ) 	// RW
#define MMCHS_PSTATE( addr )		READ_REGISTER_OFFSET( addr, 0x124 ) 	// R
#define MMCHS_HCTL( addr )			READ_REGISTER_OFFSET( addr, 0x128 ) 	// RW
#define MMCHS_SYSCTL( addr )		READ_REGISTER_OFFSET( addr, 0x12C ) 	// RW
#define MMCHS_STAT( addr )			READ_REGISTER_OFFSET( addr, 0x130 ) 	// RW
#define MMCHS_IE( addr )			READ_REGISTER_OFFSET( addr, 0x134 ) 	// RW
#define MMCHS_ISE( addr )			READ_REGISTER_OFFSET( addr, 0x138 ) 	// RW
#define MMCHS_AC12( addr )			READ_REGISTER_OFFSET( addr, 0x13C ) 	// R
#define MMCHS_CAPA( addr )			READ_REGISTER_OFFSET( addr, 0x140 ) 	// RW
#define MMCHS_CUR_CAPA( addr )		READ_REGISTER_OFFSET( addr, 0x148 ) 	// RW
#define MMCHS_REV( addr )			READ_REGISTER_OFFSET( addr, 0x1FC ) 	// R

#define MMCHS_SYSCONFIG_AUTOIDLE_FREE_BIT			0x1
#define MMCHS_SYSCONFIG_SOFTRESET_BIT 				0x2
#define MMCHS_SYSCONFIG_ENAWAKEUP_BIT 				0x4
#define MMCHS_SYSCONFIG_SIDLE_WAKEUP_BIT 			0x20
#define MMCHS_SYSCONFIG_CLCKACT_MAINTALL_BIT		0x300

#define MMCHS_SYSSTATUS_RESETDONE_BIT				0x1

#define MMCHS_HCTL_DTW_BIT 							0x2
#define MMCHS_HCTL_SDBP_BIT							0x100
#define MMCHS_HCTL_SDVS_18V_BIT 					0xA00
#define MMCHS_HCTL_SDVS_30V_BIT 					0xC00
#define MMCHS_HCTL_SDVS_33V_BIT 					0xE00
#define MMCHS_HCTL_IWE_BIT							0x1000000
#define MMCHS_HCTL_INS_BIT							0x2000000

#define MMCHS_CIRQ_ENABLE_BIT						0x100

#define MMCHS_SYSCTL_ICE_BIT 						0x1
#define MMCHS_SYSCTL_ICS_BIT 						0x2
#define MMCHS_SYSCTL_CEN_BIT						0x4
#define MMCHS_SYSCTL_CLKD_BIT						0x8000
#define MMCHS_SYSCTL_SRC_BIT 						0x2000000
#define MMCHS_SYSCTL_SRD_BIT 						0x4000000

#define MMCHS_CON_OD_BIT 							0x1
#define MMCHS_CON_INIT_BIT 							0x2
#define MMCHS_CON_DW8_BIT 							0x20
#define MMCHS_CON_CEATA_BIT 						0x1000

#define MMCHS_CAPA_VS33_BIT							0x1000000
#define MMCHS_CAPA_VS30_BIT							0x2000000
#define MMCHS_CAPA_VS18_BIT							0x4000000

#define MMCHS_STAT_CC_BIT							0x1
#define MMCHS_STAT_TC_BIT 							0x2
#define MMCHS_STAT_BWR_BIT							0x10
#define MMCHS_STAT_BRR_BIT							0x20
#define MMCHS_STAT_CTO_BIT 							0x10000
#define MMCHS_STAT_CCRC_BIT 						0x20000
#define MMCHS_STAT_CEB_BIT							0x40000
#define MMCHS_STAT_CIE_BIT							0x80000
#define MMCHS_STAT_DTO_BIT 							0x100000
#define MMCHS_STAT_DCRC_BIT 						0x200000
#define MMCHS_STAT_DEB_BIT 							0x400000
#define MMCHS_STAT_CERR_BIT							0x10000000

#define MMCHS_RSP10_BUSY_BIT						0x80000000

#define MMCHS_PSTATE_CMDI_BIT						0x1
#define MMCHS_PSTATE_DATI_BIT						0x24

#define MMCHS_BLK_NBLK_BITS 						0xFFFF0000
#define MMCHS_BLK_BLEN_BITS 						0x7FF

#define MMCHS_CMD_DMA_BIT 							0x1
#define MMCHS_CMD_BCE_BIT 							0x2
#define MMCHS_CMD_ACEN_BIT							0x4
#define MMCHS_CMD_DDIR_BIT 							0x10
#define MMCHS_CMD_MSBS_BIT 							0x20
#define MMCHS_CMD_RSP_TYPE_136_BIT 					0x10000
#define MMCHS_CMD_RSP_TYPE_48_BIT 					0x20000
#define MMCHS_CMD_RSP_TYPE_48BUSY_BIT 				0x30000
#define MMCHS_CMD_CCCE_BIT 							0x80000
#define MMCHS_CMD_CICE_BIT 							0x100000

#define MMCHS_IE_CC_CTO_CCRC_CEB_CIE_DTO_DCRC_DEB_CERR_BITS 0x107F0001

// TODO: this should be moved to a different module //////////////////////////////////////////
// NOTE: see OMAP35x.pdf page 452
#define CORE_CM_ADDR 					0x48004A00
#define CONTROL_PADCONF_MMC1_ADDR		0x48002144

#define CM_ICLKEN1_CORE				READ_REGISTER_OFFSET( CORE_CM_ADDR, 0x010 ) 	// RW
#define CM_FCLKEN1_CORE				READ_REGISTER_OFFSET( CORE_CM_ADDR, 0x0 ) 		// RW

#define CM_EN_MMCHS1_BIT			0x1000000
/////////////////////////////////////////////////////////////////////////////////////////////

#define BLOCK_LEN 		512

// TODO: remove after test
static uint8_t blockBuffer[ 512 ];

uint32_t
sdHalInit()
{
	// TODO: need to get an interrupt when card is inserted

	// NOTE: the initializatino follows the programming flow from OMAP35x.pdf at page 3177
	enableIfaceAndFunctionalClock();
	softwareReset();
	voltageCapInit();
	defaultInit();
	initProcedureStart();
	preCardIdentificationConfig();
	identify();

	readBlock( 0, blockBuffer );

	//configIdleAndWakeup();
	//configControllerBus();

	return 0;
}

uint32_t
sdHalReadBytes( uint32_t* address, uint32_t* buffer, uint32_t bufferSize )
{
	memset( buffer, 0, bufferSize );

	// TODO: implement

	return 0;
}

void
enableIfaceAndFunctionalClock( void )
{
	// NOTE: see OMAP35x.pdf at page 3178

	// 1. Enable the interface clock for the MMCHS1 controller
	BIT_SET( CM_ICLKEN1_CORE, CM_EN_MMCHS1_BIT );
	// 2. Enable the functional clock for the MMCHS1 controller
	BIT_SET( CM_FCLKEN1_CORE, CM_EN_MMCHS1_BIT );
}

void
softwareReset( void )
{
	// NOTE: see OMAP35x.pdf page 3160f and at page 3178

	// 1. trigger module reset
	BIT_SET( MMCHS_SYSCONFIG( SELECTED_CHS ), MMCHS_SYSCONFIG_SOFTRESET_BIT );
	// 2. await finishing of module reset
	AWAIT_BITS_SET( MMCHS_SYSSTATUS( SELECTED_CHS ), MMCHS_SYSSTATUS_RESETDONE_BIT );
}

void
voltageCapInit( void )
{
	// NOTE: see OMAP35x.pdf page 3160f and at page 3178

	// activate 1.8 and 3.0 voltage in capa register
	BIT_SET( MMCHS_CAPA( SELECTED_CHS ), MMCHS_CAPA_VS18_BIT | MMCHS_CAPA_VS30_BIT );
}

void
defaultInit( void )
{
	// NOTE: see OMAP35x.pdf page 3178f

	// NOTE: using magic numbers copied from manual because it would lead to contradicting values if following steps 1-6
	MMCHS_HCTL( SELECTED_CHS ) = 0x00000b00;
	MMCHS_SYSCTL( SELECTED_CHS ) = 0x0000a007;
	MMCHS_CON( SELECTED_CHS ) = MMCHS_CON_OD_BIT;
}

void
initProcedureStart( void )
{
	// NOTE: see OMAP35x.pdf page 3179

	BIT_SET( MMCHS_CON( SELECTED_CHS ), MMCHS_CON_INIT_BIT );
	MMCHS_CMD( SELECTED_CHS ) = 0x0;
	READ_REGISTER( CONTROL_PADCONF_MMC1_ADDR ) = 0x100;
}

void
preCardIdentificationConfig( void )
{
	// NOTE: see OMAP35x.pdf page 3179

	MMCHS_HCTL( SELECTED_CHS ) = 0x00000b00;
	MMCHS_SYSCTL( SELECTED_CHS ) = 0x00003C07;
	MMCHS_CON( SELECTED_CHS ) = MMCHS_CON_OD_BIT;
}

/*
void
configIdleAndWakeup( void )
{
	// NOTE: see OMAP35x.pdf page 3162

	// enable wakeup capability
	BIT_SET( MMCHS_SYSCONFIG( SELECTED_CHS ), MMCHS_SYSCONFIG_ENAWAKEUP_BIT );
	// Wake-up event enable on SD card interrupt.
	BIT_SET( MMCHS_HCTL( SELECTED_CHS ), MMCHS_HCTL_IWE_BIT );
	// Wake-up event enable on SD card insertion
	// BIT_SET( MMCHS_HCTL( SELECTED_CHS ), MMCHS_HCTL_INS_BIT );

	// SDIO ONLY
	// BIT_SET( SELECTED_CHS, MMCHS_CIRQ_ENABLE_BIT );
}

void
configControllerBus( void )
{
	// NOTE: see OMAP35x.pdf page 3163

	// opendrain only for MMC cards, thus set to 0
	BIT_CLEAR( MMCHS_CON( SELECTED_CHS ), MMCHS_CON_OD_BIT );
	// 8-bit mode MMC select only for MMC cards, thus set to 0
	BIT_CLEAR( MMCHS_CON( SELECTED_CHS ), MMCHS_CON_DW8_BIT );
	// CE-ATA control mode only for MMC cards, thus set to 0
	BIT_CLEAR( MMCHS_CON( SELECTED_CHS ), MMCHS_CON_CEATA_BIT );

	// check if 3.0 volt is supported
	if ( MMCHS_CAPA( MMCHS_CAPA_VS30_BIT ) )
	{
		// SD bus voltage select (All cards) to 3.0 volt
		BIT_SET( MMCHS_HCTL( SELECTED_CHS ), MMCHS_HCTL_SDVS_30V_BIT );
	}
	// 3.0 volt not supported, check if 3.3 volt is supported
	else if ( MMCHS_CAPA( MMCHS_CAPA_VS33_BIT ) )
	{
		// SD bus voltage select (All cards) to 3.3 volt
		BIT_SET( MMCHS_HCTL( SELECTED_CHS ), MMCHS_HCTL_SDVS_33V_BIT );
	}
	else
	{
		// TODO: handle problem: neither 3.0 or 3.3 volt supported
	}

	// SD bus power
	BIT_SET( MMCHS_HCTL( SELECTED_CHS ), MMCHS_HCTL_SDBP_BIT );
	// Data transfer width
	BIT_SET( MMCHS_HCTL( SELECTED_CHS ), MMCHS_HCTL_DTW_BIT );

	// check if configuration is supported
	// AWAIT_BITS_SET( MMCHS_HCTL( SELECTED_CHS ), MMCHS_HCTL_SDBP_BIT );
	if ( BIT_CHECK( MMCHS_HCTL( SELECTED_CHS ), MMCHS_HCTL_SDBP_BIT ) )
	{
		// TODO: handle problem: configuration not supported
	}

	BIT_SET( MMCHS_SYSCTL( SELECTED_CHS ), MMCHS_SYSCTL_ICE_BIT );

	BIT_SET( CONTROL_PADCONF_MMC1_ADDR, 0x100 );

	// Clock frequency select
	BIT_SET( MMCHS_SYSCTL( SELECTED_CHS ), MMCHS_SYSCTL_CLKD_BIT );
	// await Internal clock stable
	AWAIT_BITS_SET( MMCHS_SYSCTL( SELECTED_CHS ), MMCHS_SYSCTL_ICS_BIT );

	BIT_SET( MMCHS_SYSCONFIG( SELECTED_CHS ), MMCHS_SYSCONFIG_CLCKACT_MAINTALL_BIT );
	BIT_SET( MMCHS_SYSCONFIG( SELECTED_CHS ), MMCHS_SYSCONFIG_SIDLE_WAKEUP_BIT );
	BIT_CLEAR( MMCHS_SYSCONFIG( SELECTED_CHS ), MMCHS_SYSCONFIG_AUTOIDLE_FREE_BIT );
}
*/

uint32_t
identify( void )
{
	uint8_t cardBusyFlag = 0;
	// NOTE: see OMAP35x.pdf page 3164

	// start send initialization stream
	BIT_SET( MMCHS_CON( SELECTED_CHS ), MMCHS_CON_INIT_BIT );
	MMCHS_CMD( SELECTED_CHS ) = 0x0;

	// WAIT 1ms to allow card initializing internal state
	uint64_t sysMillis = getSysMillis();
	while ( 1 > getSysMillis() - sysMillis ) { }

	// clear status: Command complete
	BIT_SET( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CC_BIT );
	// end initialization sequence
	BIT_CLEAR( MMCHS_CON( SELECTED_CHS ), MMCHS_CON_INIT_BIT );
	// Clear MMCHS_STAT register
	MMCHS_STAT( SELECTED_CHS ) = 0xFFFFFFFF;

	// TODO: Change clock frequency to fit protocol

	// send GO_IDLE_STATE
	if ( sendCmd0() )
	{
		// TODO: an error occured sending CMD0
		return 1;
	}

	// cmd5 is reserved for I/O cards: SDIO. will return 0 if it is SDIO
	if ( 0 == sendCmd5() )
	{
		// TODO: handle problem: we don't do SDIO for now
		return 1;
	}

	// send SEND_IF_COND
	if ( 0 == sendCmd8() )
	{
		// NOTE: it is an SD card compliant with standard 2.0 or later
		goto cardIdentified;
	}

	do
	{
		// send APP_CMD to notify card that next command will be application specific
		if ( sendCmd55() )
		{
			// TODO: an error occured sending CMD55
			return 1;
		}

		// send SD_SEND_OP_COND
		if ( 0 == sendACmd41() )
		{
			// NOTE: it is a SD card compliant with standard 1.x

			// only in this case we need to check if card is busy
			cardBusyFlag = isCardBusy();
			// if card is busy, repeat again, otherwise card is identified
			if ( !cardBusyFlag )
			{
				goto cardIdentified;
			}
		}
	} while ( cardBusyFlag );

	// NOTE: at this point we are a MMC card

	do
	{
		if ( sendCmd1() )
		{
			// TODO: handle unknown type of card
			return 1;
		}

	} while ( isCardBusy() );

cardIdentified:
	// send ALL_SEND_CID
	if ( sendCmd2() )
	{
		return 1;
	}

	// send SEND_RELATIVE_ADDR to ask card to publish new realtive address
	if ( sendCmd3() )
	{
		return 1;
	}

	// NOTE: assume if MMC card only one MMC card connected to bus

	// send SELECT/DESELECT_CARD to select card
	if ( sendCmd7() )
	{
		return 1;
	}

	// NOTE: at this point the card is initialized, identified and ready to be used

	return 0;
}

uint32_t
readBlock( uint32_t addr, uint8_t* buffer )
{
	// NOTE: see OMAP35x.pdf page 3168

	awaitDataLineAvailable();

	if ( sendCmd16() )
	{
		// an error occured during sending the command, return immediately
		return 1;
	}

	if ( sendCmd23() )
	{
		// an error occured during sending the command, return immediately
		return 1;
	}

	if ( sendCmd18( addr ) )
	{
		// an error occured during sending the command, return immediately
		return 1;
	}

	// NOTE: Repeat MMCHS_DATA access (BLEN + 3)/4 times.
	uint32_t readTimes = ( BLOCK_LEN + 3 ) / 4;

	do
	{
		awaitBufferReadReady();

		// NOTE: read 4bytes of data from data-address, will be moved by controller automatically
		uint32_t data = MMCHS_DATA( SELECTED_CHS );

		buffer[ readTimes * 4 + 0 ] = 0xFF & ( data >> 0 );
		buffer[ readTimes * 4 + 1 ] = 0xFF & ( data >> 8 );
		buffer[ readTimes * 4 + 2 ] = 0xFF & ( data >> 16 );
		buffer[ readTimes * 4 + 3 ] = 0xFF & ( data >> 24 );

	} while ( --readTimes );

checkTransferComplete:
	if ( isTransferComplete() )
	{
		// TODO: need to distinguish between finite and infinite transfer
		// TODO: when finite: END
	}
	else
	{
		// check for Data End Bit error, Data CRC Error or Data timeout error
		if ( BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_DEB_BIT ) ||
				BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_DCRC_BIT ) ||
				BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_DTO_BIT ) )
		{
			// finished with an error
			resetMMCIDataLine();
			return 1;
		}
		else
		{
			// no error occured -> check again if transfer complete
			goto checkTransferComplete;
		}
	}

	// transfer complete, no error occured
	return 0;
}

uint32_t
sendReadCommand( uint8_t cmdId, uint32_t arg )
{
	// TODO: implement
	return 0;
}

uint32_t
sendCommandNoResponse( uint8_t cmdId, uint32_t arg )
{
	// TODO: implement
	return 0;
}

uint32_t
sendCommandWithResponse( uint8_t cmdId, uint32_t arg )
{
	// TODO: implement
	return 0;
}

uint32_t
sendCommand( uint8_t cmdId, uint32_t arg )
{
	// NOTE: see OMAP35x.pdf page 3172

	awaitCommandLineAvailable();

	// TODO: check when must be set to 0
	MMCHS_CON( SELECTED_CHS ) = 0x0;

	// TODO: set correct value
	// MMCHS_CSRE( SELECTED_CHS ) = 0x0;

	// TODO: set correct value
	MMCHS_BLK( SELECTED_CHS ) |= MMCHS_BLK_NBLK_BITS;
	// TODO: set correct valueval & bits
	MMCHS_BLK( SELECTED_CHS ) |= MMCHS_BLK_BLEN_BITS;

	// TODO: set correct value
	// BIT_SET( MMCHS_SYSCTL( SELECTED_CHS ), MMCHS_SYSCTL_DTO_BIT ) = 0x0;

	// TODO: set correct value
	MMCHS_ARG( SELECTED_CHS ) = arg;

	// NOTE: we will check for errors later on so enable interrupts for CC, CTO, CCRC, CIE, CEB, CERR
	// NOTE: also set DEB, DCRC, DTO because if its a data-command, need to check it later
	MMCHS_IE( SELECTED_CHS ) = MMCHS_IE_CC_CTO_CCRC_CEB_CIE_DTO_DCRC_DEB_CERR_BITS;
	// NOTE: we are doing polling => IE is enough, no need to set ISE
	//MMCHS_ISE( SELECTED_CHS ) = 0x00040001;

	// NOTE: need to construct value written to CMD-register because write will immediately lead to transmission of command
	uint32_t cmdToken = ( cmdId & 0x3F ) << 24;		// write command to bit 24 and use only the bits 0-5 of it
	// NOTE: enable command index check
	BIT_SET( cmdToken, MMCHS_CMD_CICE_BIT );
	// NOTE: enable command CRC check
	BIT_SET( cmdToken, MMCHS_CMD_CCCE_BIT );
	/* TODO: need to select correct response type
	 * 0 (don't set anything): no response
	 */
	BIT_SET( cmdToken, MMCHS_CMD_RSP_TYPE_136_BIT );
	BIT_SET( cmdToken, MMCHS_CMD_RSP_TYPE_48_BIT );
	BIT_SET( cmdToken, MMCHS_CMD_RSP_TYPE_48BUSY_BIT );

	// TODO: if MULTI-BLOCK select set, otherwise leave 0
	BIT_SET( cmdToken, MMCHS_CMD_MSBS_BIT );

	// TODO: if read-command set, otherwise leave 0
	BIT_SET( cmdToken, MMCHS_CMD_DDIR_BIT );

	// TODO: enable auto-command 12: controller will automatically send CMD12 (STOP_TRANSMISSION) after transfer of last block is completed
	BIT_SET( cmdToken, MMCHS_CMD_DDIR_BIT );

	// TODO: enable block count for multiple block transfers
	BIT_SET( cmdToken, MMCHS_CMD_BCE_BIT );

	// NOTE: DMA is not used in this implementation
	BIT_CLEAR( cmdToken, MMCHS_CMD_DMA_BIT );

	// special cases, not yet implemented
	// NOTE: see OMAP35x.pdf page 3203
	/* if ( 52 == cmdId || 12 == cmdId )
	{
		// NOTE: need to handle "Bus Suspend" / "Function Select" / "I/O Abort"
	}
	*/
	// NOTE: if data is present (only during writing) need to set DP bit
	// BIT_SET( cmdToken, MMCHS_CMD_DP_BIT );

	// issue command by writing previously constructedvalue
	MMCHS_CMD( SELECTED_CHS ) = cmdToken;

	return awaitCommandResponse();
}

uint32_t
awaitCommandResponse()
{
	while ( 1 )
	{
		//
		volatile uint32_t stat = MMCHS_STAT( SELECTED_CHS );
		volatile uint32_t cto = stat & MMCHS_STAT_CTO_BIT;
		volatile uint32_t ccrc = stat & MMCHS_STAT_CCRC_BIT;
		volatile uint32_t cc = stat & MMCHS_STAT_CC_BIT;

		volatile uint32_t cmdTimeout = BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CTO_BIT );
		volatile uint32_t cmdCrcError = BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CCRC_BIT );
		volatile uint32_t cmdComplete = BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CC_BIT );

		// check for errors
		if ( cmdTimeout && cmdCrcError )
		{
			// Command Timeout Error AND Command CRC Error occured
			// NOTE: This is a particular case that occurs when there is a conflict on the mmci_cmd line

			resetMMCICmdLine();
			// return 1 to indicate failure
			return 1;
		}
		// check for errors
		else if ( cmdTimeout && 0 == cmdCrcError )
		{
			// Command Timeout Error but no Command CRC Error occured
			resetMMCICmdLine();
			// return 1 to indicate failure
			return 1;
		}
		// command complete?
		else if ( cmdComplete )
		{
			// yes command is complete

			// check if a response is waiting
			if ( BIT_CHECK( MMCHS_CMD( SELECTED_CHS ), MMCHS_CMD_RSP_TYPE_48BUSY_BIT ) )
			{
				// a response is waiting

				uint32_t rsp0 = MMCHS_RSP10( SELECTED_CHS ) & 0x0000FFFF;
				uint32_t rsp1 = MMCHS_RSP10( SELECTED_CHS ) >> 16;
				uint32_t rsp2 = MMCHS_RSP32( SELECTED_CHS ) & 0x0000FFFF;
				uint32_t rsp3 = MMCHS_RSP32( SELECTED_CHS ) >> 16;
				uint32_t rsp4 = MMCHS_RSP54( SELECTED_CHS ) & 0x0000FFFF;
				uint32_t rsp5 = MMCHS_RSP54( SELECTED_CHS ) >> 16;
				uint32_t rsp6 = MMCHS_RSP76( SELECTED_CHS ) & 0x0000FFFF;
				uint32_t rsp7 = MMCHS_RSP76( SELECTED_CHS ) >> 16;

				// check for errors
				if ( BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CIE_BIT ) ||
						BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CEB_BIT ) ||
						BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CCRC_BIT ) ||
						BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CERR_BIT ) )
				{
					// Command index error / Command end bit error / Command CRC Error /  Card error ?
					// return 1 to indicate failure
					return 1;
				}
			}

			return 0;
		}
		else
		{
			// not yet completed, check again
		}
	}
}

void
resetMMCIDataLine( void )
{
	BIT_SET( MMCHS_SYSCTL( SELECTED_CHS ), MMCHS_SYSCTL_SRD_BIT );
	AWAIT_BITS_CLEARED( MMCHS_SYSCTL( SELECTED_CHS ), MMCHS_SYSCTL_SRD_BIT );
}

void
resetMMCICmdLine( void )
{
	BIT_SET( MMCHS_SYSCTL( SELECTED_CHS ), MMCHS_SYSCTL_SRC_BIT );
	AWAIT_BITS_CLEARED( MMCHS_SYSCTL( SELECTED_CHS ), MMCHS_SYSCTL_SRC_BIT );
}

uint32_t
isTransferComplete( void )
{
	return BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_TC_BIT );
}

void
awaitBufferReadReady( void )
{
	AWAIT_BITS_SET( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_BRR_BIT );
}

void
awaitDataLineAvailable( void )
{
	// NOTE: if bit 1 (DATI) of MMCHS_PSTATE is 1 data-lines are busy
	AWAIT_BITS_CLEARED( MMCHS_PSTATE( SELECTED_CHS ), MMCHS_PSTATE_DATI_BIT );
}

void
awaitCommandLineAvailable( void )
{
	AWAIT_BITS_CLEARED( MMCHS_PSTATE( SELECTED_CHS ), MMCHS_PSTATE_CMDI_BIT );
}

uint32_t
isCardBusy()
{
	// NOTE: if bit 31 of RSP10 is 1 card is not busy
	return ! BIT_CHECK( MMCHS_RSP10( SELECTED_CHS ), MMCHS_RSP10_BUSY_BIT );
}

/* NOTE: GO_IDLE_STATE
 * Resets all cards to idle state
 */
uint32_t
sendCmd0( void )
{
	// NOTE: this command resets the MMC card (see OMAP35x.pdf page 3180)

	MMCHS_CON( SELECTED_CHS ) = 0x00000001;
	MMCHS_IE( SELECTED_CHS ) = 0x00040001;
	MMCHS_ISE( SELECTED_CHS ) = 0x00040001;
	MMCHS_CMD( SELECTED_CHS ) = 0x00000000;

	return awaitCommandResponse();
}

/* NOTE: reserved
 * ?
 */
uint32_t
sendCmd1( void )
{
	// NOTE: Once the card response is available in register MMCHS1.MMCHS_RSP10, the software is responsible to
	//			compare Card OCR and Host OCR, and then send a second CMD1 command with the cross-checked
	//			OCR (see OMAP35x.pdf page 3181 )

	MMCHS_CON( SELECTED_CHS ) = 0x00000001;
	MMCHS_IE( SELECTED_CHS ) = 0x00050001;
	MMCHS_ISE( SELECTED_CHS ) = 0x00050001;
	MMCHS_CMD( SELECTED_CHS ) = 0x01020000;

	return awaitCommandResponse();
}

/* NOTE: ALL_SEND_CID
 * Asks any card to send the CID num-bers on the CMD line (any card that is connected to the host will respond)
 */
uint32_t
sendCmd2( void )
{
	// NOTE: This command asks the MMC card to send its CID register's content (see OMAP35x.pdf page 3182)

	MMCHS_CON( SELECTED_CHS ) = 0x00000001;
	MMCHS_IE( SELECTED_CHS ) = 0x00070001;
	MMCHS_ISE( SELECTED_CHS ) = 0x00070001;
	MMCHS_CMD( SELECTED_CHS ) = 0x02090000;

	return awaitCommandResponse();

	// NOTE: The response is 128 bits wide and is received in MMCHS1.MMCHS_RSP10, MMCHS1.MMCHS_RSP32, MMCHS1.MMCHS_RSP54 and MMCHS1.MMCHS_RSP76 registers.
}

/* NOTE: SEND_RELATIVE_ADDR
 * Ask the card to publish a new relative address (RCA)
 */
uint32_t
sendCmd3( void )
{
	// NOTE: This command sets MMC card address (see Table 22-19). Useful when MMCHS controller switches to addressed mode (see OMAP35x.pdf page 3182)

	MMCHS_CON( SELECTED_CHS ) = 0x00000001;
	MMCHS_IE( SELECTED_CHS ) = 0x100f0001;
	MMCHS_ISE( SELECTED_CHS ) = 0x100f0001;
	MMCHS_ARG( SELECTED_CHS ) = 0x00010000;
	MMCHS_CMD( SELECTED_CHS ) = 0x031a0000;

	return awaitCommandResponse();
}

// NOTE: reserved for I/O cards (refer to the "SDIO Card Specification")
uint32_t
sendCmd5( void )
{
	// NOTE: This command asks a SDIO card to send its operating conditions. This command will fail if there is no SDIO card (see OMAP35x.pdf page 3180)

	MMCHS_CON( SELECTED_CHS ) = 0x00000001;
	MMCHS_IE( SELECTED_CHS ) = 0x00050001;
	MMCHS_ISE( SELECTED_CHS ) = 0x00050001;
	MMCHS_CMD( SELECTED_CHS ) = 0x05020000;

	return awaitCommandResponse();

	// NOTE: In case of success the response will be in MMCHS1.MMCHS_RSP10 register
}

/* NOTE: SELECT/DESELECT_CARD
 * ommand toggles a card between the stand-by and transfer states or between the programming and disconnect states.
 * In both cases, the card is selected by its own relative address and gets deselected by any other address; address 0 deselects all.
 * In the case that the RCA equals 0, then the host may do one of the following:
 * 		- Use other RCA number to perform card de-selection.
 * 		- Re-send CMD3 to change its RCA number to other than 0 and then use CMD7 with RCA=0 for card de- selection.
 */
uint32_t
sendCmd7( void )
{
	// NOTE: see OMAP35x.pdf page 3184

	MMCHS_CON( SELECTED_CHS ) = 0x00000000;
	MMCHS_IE( SELECTED_CHS ) = 0x100f0001;
	MMCHS_ISE( SELECTED_CHS ) = 0x100f0001;
	MMCHS_ARG( SELECTED_CHS ) = 0x00010000;
	MMCHS_CMD( SELECTED_CHS ) = 0x071a0000;

	return awaitCommandResponse();
}

/* NOTE: SEND_IF_COND
 * Sends SD Memory Card interface condition, which includes host supply voltage information and asks the card whether card supports voltage.
 * Reserved bits shall be set to '0'.
 */
uint32_t
sendCmd8( void )
{
	// NOTE: This command asks a SD card version 2.X to send its operating conditions (see OMAP35x.pdf page 3180f)

	MMCHS_CON( SELECTED_CHS ) = 0x00000001;
	MMCHS_IE( SELECTED_CHS ) = 0x100f0001;
	MMCHS_ISE( SELECTED_CHS ) = 0x100f0001;
	MMCHS_CMD( SELECTED_CHS ) = 0x81a0000;

	return awaitCommandResponse();
	// NOTE: In case of success the response will be in MMCHS1.MMCHS_RSP10 register
}

// NOTE: SET_BLOCKLEN to 512
uint32_t
sendCmd16()
{
	// NOTE: Issuing CMD16 allows to set the block length (see OMAP35x.pdf page 3186)

	MMCHS_CON( SELECTED_CHS ) = 0x00000020;	// MMC bus is in push-pull mode. DW8 is enabled.
	MMCHS_IE( SELECTED_CHS ) = 0x100f0001;	// Enables CERR, CIE, CCRC, CC, CTO and CEB events to occur.
	MMCHS_ISE( SELECTED_CHS ) = 0x100f0001;	// Enables CERR, CIE, CCRC, CC, CTO and CEB interrupts to rise.
	MMCHS_ARG( SELECTED_CHS ) = 0x00000200;	// Block length is 512 = 0x200
	MMCHS_CMD( SELECTED_CHS ) = 0x101a0000;	// Sends CMD16 whose opcode is 16, response type is 48 bits with CICE and CCCE enabled.

	return awaitCommandResponse();
}

// NOTE: READ_MULTIPLE_BLOCK from address
uint32_t
sendCmd18( uint32_t addr )
{
	// NOTE: Issuing CMD18 starts the finite, multiple block read transfer. (see OMAP35x.pdf page 3187)

	MMCHS_CON( SELECTED_CHS ) = 0x00000020;	// MMC bus is in push-pull mode. DW8 is enabled.
	MMCHS_IE( SELECTED_CHS ) = 0x107f0023;	// Enables CERR, CIE, CCRC, CC, TC, BRR, CTO, DTO, DCRC, DEB and CEB events to occur.
	MMCHS_ISE( SELECTED_CHS ) = 0x107f0023;	// Enables CERR, CIE, CCRC, CC, TC, BRR, CTO, DTO, DCRC, DEB and CEB interrupts to rise.
	MMCHS_ARG( SELECTED_CHS ) = addr;	// address
	MMCHS_BLK( SELECTED_CHS ) = 0x00080200;	// (number_blocks << 16) | (block_length)
	MMCHS_CMD( SELECTED_CHS ) = 0x123A0032;	// Sends CMD18 whose opcode is 18, response type is 48 bits, with CICE, DP, MSBS, BCE, DE, DDIR and CCCE enabled.

	return awaitCommandResponse();
}

// NOTE: SET_BLOCK_COUNT to 1
uint32_t
sendCmd23()
{
	// NOTE: Issuing CMD23 allows to set the number of how many 512-byte blocks the MMC card should expect from the MMCHS controller (see OMAP35x.pdf page 3186)

	MMCHS_CON( SELECTED_CHS ) = 0x00000020;	// MMC bus is in push-pull mode. DW8 is enabled.
	MMCHS_IE( SELECTED_CHS ) = 0x100f0001;	// Enables CERR, CIE, CCRC, CC, CTO and CEB events to occur.
	MMCHS_ISE( SELECTED_CHS ) = 0x100f0001; // Enables CERR, CIE, CCRC, CC, CTO and CEB interrupts to rise.
	MMCHS_ARG( SELECTED_CHS ) = 0x00000008; // Number of 512-byte blocks in 4 KB buffer is 8.
	MMCHS_CMD( SELECTED_CHS ) = 0x101a0000; // Sends CMD23 whose opcode is 23, response type is 48 bits with CICE and CCCE enabled.

	return awaitCommandResponse();
}

/* NOTE: SD_SEND_OP_COND
 * Sends host capacity support information (HCS) and asks the accessed card to send its operating condition register (OCR) content in the response on the CMD line.
 * HCS is effective when card receives SEND_IF_COND command. Sends request to switch to 1.8V signaling (S18R).
 * Reserved bit shall be set to '0'. CCS bit is assigned to OCR[30].
 * XPC controls the maximum power in the default speed mode of SDXC card.
 * XPC=0 means 0.36W (100mA at 3.6V on VDD1) (max.) but speed class is not supported.
 * XPC=1 means 0.54W (150mA at 3.6V on VDD1) (max.) and speed class is supported.
 */
uint32_t
sendACmd41( void )
{
	// TODO: implement
/*
	MMCHS_CON( SELECTED_CHS ) = 0x00000001;

	MMCHS_IE( SELECTED_CHS ) = 0x100f0001; // TODO: implement
	MMCHS_ISE( SELECTED_CHS ) = 0x100f0001; // TODO: implement

	MMCHS_ARG( SELECTED_CHS ) = 0x00010000; // TODO: implement

	MMCHS_CMD( SELECTED_CHS ) = 0x81a0000; // TODO: implement
	*/

	return 0;
}

/* NOTE: APP_CMD
 * Indicates to the card that the next command is an application specific command rather than a standard command
 */
uint32_t
sendCmd55( void )
{
	// This is a special command used to prevent the card that the following command is going to be an application one (see OMAP35x.pdf page 3181)

	MMCHS_CON( SELECTED_CHS ) = 0x00000001;
	MMCHS_IE( SELECTED_CHS ) = 0x100f0001;
	MMCHS_ISE( SELECTED_CHS ) = 0x100f0001;
	MMCHS_CMD( SELECTED_CHS ) = 0x371a0000;

	return awaitCommandResponse();
}
