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
static void enableIfaceAndFunctionalClock( void );
static void softwareReset( void );
static void voltageCapInit( void );
static void defaultInit( void );
static void	initProcedureStart( void );
static void	preCardIdentificationConfig( void );
static void identify( void );

static void configIdleAndWakeup( void );
static void configControllerBus( void );

static uint8_t waitForStatCC( void );
static void doAndAwaitSysCtlSrc( void );
static uint32_t isCardBusy( void );

static void sendCmd0( void );
static void sendCmd1( void );
static void sendCmd2( void );
static void sendCmd3( void );
static void sendCmd5( void );
static void sendCmd7( void );
static void sendCmd8( void );
static void sendCmd55( void );
static void sendACmd41( void );
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

#define MMCHS_CON_OD_BIT 							0x1
#define MMCHS_CON_INIT_BIT 							0x2
#define MMCHS_CON_DW8_BIT 							0x20
#define MMCHS_CON_CEATA_BIT 						0x1000

#define MMCHS_CAPA_VS33_BIT							0x1000000
#define MMCHS_CAPA_VS30_BIT							0x2000000
#define MMCHS_CAPA_VS18_BIT							0x4000000

#define MMCHS_STAT_CC_BIT							0x1
#define MMCHS_STAT_CTO_BIT 							0x10000

#define MMCHS_RSP10_BUSY_BIT						0x80000000

// TODO: this should be moved to a different module //////////////////////////////////////////
// NOTE: see OMAP35x.pdf page 452
#define CORE_CM_ADDR 					0x48004A00
#define CONTROL_PADCONF_MMC1_ADDR		0x48002144

#define CM_ICLKEN1_CORE				READ_REGISTER_OFFSET( CORE_CM_ADDR, 0x010 ) 	// RW
#define CM_FCLKEN1_CORE				READ_REGISTER_OFFSET( CORE_CM_ADDR, 0x0 ) 		// RW

#define CM_EN_MMCHS1_BIT			0x1000000
/////////////////////////////////////////////////////////////////////////////////////////////

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
	BIT_SET( MMCHS_HCTL( SELECTED_CHS ), 0x00000b00 );
	BIT_SET( MMCHS_SYSCTL( SELECTED_CHS ), 0x0000a007 );
	MMCHS_CON( SELECTED_CHS ) = MMCHS_CON_OD_BIT;
}

void
initProcedureStart( void )
{
	// NOTE: see OMAP35x.pdf page 3179

	BIT_SET( MMCHS_CON( SELECTED_CHS ), MMCHS_CON_INIT_BIT );
	BIT_SET( MMCHS_CMD( SELECTED_CHS ), 0x0 );
	BIT_SET( READ_REGISTER( CONTROL_PADCONF_MMC1_ADDR ), 0x100 );
}

void
preCardIdentificationConfig( void )
{
	// NOTE: see OMAP35x.pdf page 3179

	BIT_SET( MMCHS_HCTL( SELECTED_CHS ), 0x00000b00 );
	BIT_SET( MMCHS_SYSCTL( SELECTED_CHS ), 0x00003C07 );
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

void
identify( void )
{
	// NOTE: see OMAP35x.pdf page 3164

	// start send initialization stream
	BIT_SET( MMCHS_CON( SELECTED_CHS ), MMCHS_CON_INIT_BIT );
	MMCHS_CMD( SELECTED_CHS ) = 0x0;

	// WAIT 1ms
	uint64_t sysMillis = getSysMillis();
	while ( 1 > getSysMillis() - sysMillis ) { }

	// clear status: Command complete
	BIT_SET( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CC_BIT );
	// end initialization sequence
	BIT_CLEAR( MMCHS_CON( SELECTED_CHS ), MMCHS_CON_INIT_BIT );
	// Clear MMCHS_STAT register
	MMCHS_STAT( SELECTED_CHS ) = 0xFFFFFFFF;

	// TODO: Change clock frequency to fit protocol

	sendCmd0();

	sendCmd5();

	if ( waitForStatCC() )
	{
		// TODO: handle problem: we don't do SDIO for now
	}

	doAndAwaitSysCtlSrc();

	sendCmd8();

	if ( waitForStatCC() )
	{
		// TODO: it is an SD card compliant with standard 2.0 or later
	}

	doAndAwaitSysCtlSrc();

	uint8_t cardBusyFlag = 0;
	do
	{
		sendCmd55();

		sendACmd41();

		if ( waitForStatCC() )
		{
			// TODO: it is a SD card compliant with standard 1.x
			cardBusyFlag = isCardBusy();
			goto cardIdentified;
		}
	} while ( cardBusyFlag );

	// NOTE: at this point we are a MMC card

	doAndAwaitSysCtlSrc();

	do
	{
		sendCmd1();

		if ( waitForStatCC() )
		{
			// TODO: unknown type of card
		}

	} while ( isCardBusy() );

cardIdentified:
	sendCmd2();

	sendCmd3();

	// NOTE: assume if MMC card only one MMC card connected to bus

	sendCmd7();
}

uint32_t
isCardBusy()
{
	// NOTE: if bit 31 of RSP10 is 1 card is not busy
	return ! BIT_CHECK( MMCHS_RSP10( SELECTED_CHS ), MMCHS_RSP10_BUSY_BIT );
}

void
doAndAwaitSysCtlSrc( void )
{
	// do Software reset for mmci_cmd line
	BIT_SET( MMCHS_SYSCTL( SELECTED_CHS ), MMCHS_SYSCTL_SRC_BIT );
	// await until software reset is completed
	AWAIT_BITS_CLEARED( MMCHS_SYSCTL( SELECTED_CHS ), MMCHS_SYSCTL_SRC_BIT );
}

uint8_t
waitForStatCC( void )
{
	do
	{
		if ( BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CC_BIT ) )
		{
			// already set, return value
			return 1;
		}

	// not yet set, check CC again
	} while ( ! BIT_CHECK( MMCHS_STAT( SELECTED_CHS ), MMCHS_STAT_CTO_BIT ) );

	return 0;
}

void
sendCmd0( void )
{
	// NOTE: this command resets the MMC card (see OMAP35x.pdf page 3180)

	BIT_SET( MMCHS_CON( SELECTED_CHS ), 0x00000001 );
	BIT_SET( MMCHS_IE( SELECTED_CHS ), 0x00040001 );
	BIT_SET( MMCHS_ISE( SELECTED_CHS ), 0x00040001 );
	BIT_SET( MMCHS_CMD( SELECTED_CHS ), 0x00000000 );

	// NOTE: this command has no response
}

void
sendCmd1( void )
{
	// NOTE: Once the card response is available in register MMCHS1.MMCHS_RSP10, the software is responsible to
	//			compare Card OCR and Host OCR, and then send a second CMD1 command with the cross-checked
	//			OCR (see OMAP35x.pdf page 3181 )

	BIT_SET( MMCHS_CON( SELECTED_CHS ), 0x00000001 );
	BIT_SET( MMCHS_IE( SELECTED_CHS ), 0x00050001 );
	BIT_SET( MMCHS_ISE( SELECTED_CHS ), 0x00050001 );
	BIT_SET( MMCHS_CMD( SELECTED_CHS ), 0x01020000 );
}

void
sendCmd2( void )
{
	// NOTE: This command asks the MMC card to send its CID register's content (see OMAP35x.pdf page 3182)

	BIT_SET( MMCHS_CON( SELECTED_CHS ), 0x00000001 );
	BIT_SET( MMCHS_IE( SELECTED_CHS ), 0x00070001 );
	BIT_SET( MMCHS_ISE( SELECTED_CHS ), 0x00070001 );
	BIT_SET( MMCHS_CMD( SELECTED_CHS ), 0x02090000 );

	// NOTE: The response is 128 bits wide and is received in MMCHS1.MMCHS_RSP10, MMCHS1.MMCHS_RSP32, MMCHS1.MMCHS_RSP54 and MMCHS1.MMCHS_RSP76 registers.
}

void
sendCmd3( void )
{
	// NOTE: This command sets MMC card address (see Table 22-19). Useful when MMCHS controller switches to addressed mode (see OMAP35x.pdf page 3182)

	BIT_SET( MMCHS_CON( SELECTED_CHS ), 0x00000001 );
	BIT_SET( MMCHS_IE( SELECTED_CHS ), 0x100f0001 );
	BIT_SET( MMCHS_ISE( SELECTED_CHS ), 0x100f0001 );
	BIT_SET( MMCHS_CMD( SELECTED_CHS ), 0x031a0000 );
	BIT_SET( MMCHS_ARG( SELECTED_CHS ), 0x00010000 );
}

void
sendCmd5( void )
{
	// NOTE: This command asks a SDIO card to send its operating conditions. This command will fail if there is no SDIO card (see OMAP35x.pdf page 3180)

	BIT_SET( MMCHS_CON( SELECTED_CHS ), 0x00000001 );
	BIT_SET( MMCHS_IE( SELECTED_CHS ), 0x00050001 );
	BIT_SET( MMCHS_ISE( SELECTED_CHS ), 0x00050001 );
	BIT_SET( MMCHS_CMD( SELECTED_CHS ), 0x05020000 );

	// NOTE: In case of success the response will be in MMCHS1.MMCHS_RSP10 register
}

void
sendCmd7( void )
{
	// NOTE: see OMAP35x.pdf page 3184

	BIT_SET( MMCHS_CON( SELECTED_CHS ), 0x00000000 );
	BIT_SET( MMCHS_IE( SELECTED_CHS ), 0x100f0001 );
	BIT_SET( MMCHS_ISE( SELECTED_CHS ), 0x100f0001 );
	BIT_SET( MMCHS_CMD( SELECTED_CHS ), 0x071a0000 );
	BIT_SET( MMCHS_ARG( SELECTED_CHS ), 0x00010000 );
}

void
sendCmd8( void )
{
	// NOTE: This command asks a SD card version 2.X to send its operating conditions (see OMAP35x.pdf page 3180f)

	BIT_SET( MMCHS_CON( SELECTED_CHS ), 0x00000001 );
	BIT_SET( MMCHS_IE( SELECTED_CHS ), 0x100f0001 );
	BIT_SET( MMCHS_ISE( SELECTED_CHS ), 0x100f0001 );
	BIT_SET( MMCHS_CMD( SELECTED_CHS ), 0x81a0000 );

	// NOTE: In case of success the response will be in MMCHS1.MMCHS_RSP10 register
}

void
sendACmd41( void )
{
	// TODO: implement
}

void
sendCmd55( void )
{
	// This is a special command used to prevent the card that the following command is going to be an application one (see OMAP35x.pdf page 3181)

	BIT_SET( MMCHS_CON( SELECTED_CHS ), 0x00000001 );
	BIT_SET( MMCHS_IE( SELECTED_CHS ), 0x100f0001 );
	BIT_SET( MMCHS_ISE( SELECTED_CHS ), 0x100f0001 );
	BIT_SET( MMCHS_CMD( SELECTED_CHS ), 0x371a0000 );
}
