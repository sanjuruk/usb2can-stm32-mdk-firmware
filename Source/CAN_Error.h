/*----------------------------------------------------------------------------
 *      R T L  -  C A N   D r i v e r
 *----------------------------------------------------------------------------
 *      Name:    CAN_Error.h
 *      Purpose: Header for CAN Error definition
 *      Rev.:    V3.40
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

/* Error values that functions can return                                    */
typedef enum   
{ CAN_OK = 0,                       /* No error                              */
  CAN_NOT_IMPLEMENTED_ERROR,        /* Function has not been implemented     */
  CAN_MEM_POOL_INIT_ERROR,          /* Memory pool initialization error      */
  CAN_BAUDRATE_ERROR,               /* Baudrate was not set                  */
  CAN_TX_BUSY_ERROR,                /* Transmitting hardware busy            */
  CAN_OBJECTS_FULL_ERROR,           /* No more rx or tx objects available    */
  CAN_ALLOC_MEM_ERROR,              /* Unable to allocate memory from pool   */
  CAN_DEALLOC_MEM_ERROR,            /* Unable to deallocate memory           */
  CAN_TIMEOUT_ERROR,                /* Timeout expired                       */
  CAN_UNEXIST_CTRL_ERROR,           /* Controller does not exist             */
  CAN_UNEXIST_CH_ERROR,             /* Channel does not exist                */
}  CAN_ERROR;

/* ¥ÌŒÛ¿‡–Õ */
typedef enum
{
	CAN_STUFF_ERROR = 1, 
	CAN_FORM_ERROR ,
	CAN_ACK_ERROR,
	CAN_BIT_REC_ERROR,
	CAN_BIT_DOM_ERROR,
	CAN_CRC_ERROR,
	CAN_OTHER_ERROR,
}CAN_HW_ERR;

