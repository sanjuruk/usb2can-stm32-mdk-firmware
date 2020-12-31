/*----------------------------------------------------------------------------
 *      R T L  -  C A N   D r i v e r
 *----------------------------------------------------------------------------
 *      Name:    CAN_Cfg.h
 *      Purpose: Header for STR91x CAN Configuration Settings
 *      Rev.:    V3.40
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#ifndef _CAN_CFG_H
#define _CAN_CFG_H

#include "CAN_Error.h"                /* CAN Error definition                */

/* 
// *** <<< Use Configuration Wizard in Context Menu >>> *** 
*/

/*
// <o>Number of transmit objects for controllers <1-1024>
// <i> Determines the size of software message buffer for transmitting.
// <i> Default: 20
*/
#define CAN_No_SendObjects     20

/*
// <o>Number of receive objects for controllers <1-1024>
// <i> Determines the size of software message buffer for receiving.
// <i> Default: 20
*/
#define CAN_No_ReceiveObjects  20

/*
// <q>CAN Remap Enable
*/
#define CAN_Remap_Enable     1

//Ðì³É
/*
// <q>CAN Automatic bus-off management Enable
// <i>Enable Automatic bus-off management Function
*/
#define CAN_ABOM_Enable 	1

/*
// <q>CAN Automatic retransmission Enable
// <i>Enable Automatic retransmission Function
*/
#define CAN_ART_Enable 		1

/*
// <q>CAN Mutex Enable
// <i>Enable Mutex
*/
#define CAN_MUT_Enable 		1

/*
// *** <<< End of Configuration section             >>> *** 
*/

/* Maximum index of used CAN controller 1 (only one CAN controller exists)
   Needed for memory allocation for CAN messages.                            */

#define CAN_CTRL_MAX_NUM      1

#endif /* _CAN_CFG_H */


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

