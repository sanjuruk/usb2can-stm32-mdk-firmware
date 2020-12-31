/*----------------------------------------------------------------------------
 *      R T L  -  C A N   D r i v e r
 *----------------------------------------------------------------------------
 *      Name:    CAN_Hw.c
 *      Purpose: CAN Driver, Hardware specific module for STR91x
 *      Rev.:    V3.40
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>                      /* RTX kernel functions & defines      */
#include "RTX_CAN.h"                  /* CAN Generic functions & defines     */
#include "CAN_Hw.h"                   /* CAN hw specific functions & defines */


/*----------------------------------------------------------------------------
 *      CAN RTX Hardware Specific Driver Functions
 *----------------------------------------------------------------------------
 *  Functions implemented in this module:
 *    static      void CAN_set_timing          (U32 tseg1, U32 tseg2, U32 sjw, U32 brp)
 *    static CAN_ERROR CAN_hw_set_baudrate     (U32 ctrl, U32 baudrate)
 *           CAN_ERROR CAN_hw_setup            (U32 ctrl)
 *           CAN_ERROR CAN_hw_init             (U32 ctrl, U32 baudrate)
 *           CAN_ERROR CAN_hw_start            (U32 ctrl)
 *           CAN_ERROR CAN_hw_testmode         (U32 ctrl, U32 testmode)
 *           CAN_ERROR CAN_hw_tx_empty         (U32 ctrl)
 *           CAN_ERROR CAN_hw_wr               (U32 ctrl, CAN_msg *msg)
 *    static      void CAN_hw_rd               (U32 ctrl, U32 ch, CAN_msg *msg)
 *           CAN_ERROR CAN_hw_set              (U32 ctrl, CAN_msg *msg)
 *           CAN_ERROR CAN_hw_rx_object        (U32 ctrl, U32 ch, U32 id, CAN_FORMAT format)
 *           CAN_ERROR CAN_hw_tx_object        (U32 ctrl, U32 ch, CAN_FORMAT format)
 *    Interrupt fuction
 *---------------------------------------------------------------------------*/

/* Static functions used only in this module                                 */
static void CAN_hw_rd                (U32 ctrl, U32 ch, CAN_msg *msg);
static void CAN_set_timing           (U32 tseg1, U32 tseg2, U32 sjw, U32 brp);


/************************* Auxiliary Functions *******************************/

/*--------------------------- CAN_set_timing --------------------------------
 *
 *  Setup the CAN timing with specific parameters
 *
 *  Parameter:  tseg1: specifies Time Segment before the sample point
 *              tseg2: Time Segment after the sample point
 *              sjw  : Synchronisation Jump Width
 *              brp  : Baud Rate Prescaler
 *
 *---------------------------------------------------------------------------*/
static void CAN_set_timing(U32 tseg1, U32 tseg2, U32 sjw, U32 brp) {

  CAN->BTR &= ~(((          0x03) << 24) | ((            0x07) << 20) | ((            0x0F) << 16) | (          0x1FF)); 
  CAN->BTR |=  ((((sjw-1) & 0x03) << 24) | (((tseg2-1) & 0x07) << 20) | (((tseg1-1) & 0x0F) << 16) | ((brp-1) & 0x1FF));
}


/*--------------------------- CAN_set_baudrate ------------------------------
 *
 *  Setup the requested baudrate
 *
 *  Parameter:  ctrl:       Ignored
 *              baudrate:   Baudrate
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

static CAN_ERROR CAN_hw_set_baudrate (U32 ctrl, U32 baudrate)  {
  U32 brp = 36000000;

  /* Note: this calculations fit for PCLK1 = 36MHz */
  /* Determine which nominal time to use for requested baudrate and set
     appropriate bit timing                                                  */
  if (baudrate <= 500000)  {
    brp  = (brp / 18) / baudrate;
                                                                          
    /* Load the baudrate registers BTR                                       */
    /* so that sample point is at about 72% bit time from bit start          */
    /* TSEG1 = 12, TSEG2 = 5, SJW = 4 => 1 CAN bit = 18 TQ, sample at 72%    */
    CAN_set_timing( 12, 5, 4, brp);
  }  else if (baudrate <= 1000000)  {
    brp  = (brp / 9) / baudrate;
                                                                          
    /* Load the baudrate registers BTR                                       */
    /* so that sample point is at about 72% bit time from bit start          */
    /* TSEG1 = 5, TSEG2 = 3, SJW = 3 => 1 CAN bit = 9 TQ, sample at 66%    */
    CAN_set_timing( 5, 3, 3, brp);
  }  else  {
    return CAN_BAUDRATE_ERROR;
  }  

  return CAN_OK;
}


/*************************** Module Functions ********************************/

/*--------------------------- CAN_hw_setup ----------------------------------
 *
 *  Setup CAN transmit and receive PINs and interrupt vectors
 *
 *  Parameter:  ctrl:       Ignored
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_hw_setup (U32 ctrl)  {


  /* 1. Enable CAN controller Clock                                          */
  RCC->APB1ENR |= RCC_APB1ENR_CANEN;

#if CAN_Remap_Enable 
  /* Note: MCBSTM32 uses PB8 and PB9 for CAN */
  /* 2. Setup CAN Tx and Rx pins                                             */
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;               // enable clock for Alternate Function
  AFIO->MAPR   &= 0xFFFF9FFF;                       // reset CAN remap
  AFIO->MAPR   |= 0x00004000;                       //   set CAN remap, use PB8, PB9
  /* GPIOB clock enable */
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
  /* Configure CAN pin: RX PB.8 input push pull */
  GPIOB->CRH &= ~(0x0F<<0);
  GPIOB->CRH |=  (0x08<<0);
  
  /* Configure CAN pin: TX PB.9 alternate output push pull */
  GPIOB->CRH &= ~(0x0F<<4);
  GPIOB->CRH |=  (0x0B<<4);
#else                     
  /* 2. Setup CAN Tx and Rx pins                                             */
  /* GPIOA clock enable */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
  /* Configure CAN pin: RX PA.11 input push pull */
  GPIOA->CRH &= ~(0x0F<<12);
  GPIOA->CRH |=  (0x08<<12);
  
  /* Configure CAN pin: TX PA.12 alternate output push pull */
  GPIOA->CRH &= ~(0x0F<<16);
  GPIOA->CRH |=  (0x0B<<16);
#endif

  /* 3. Setup IRQ vector for CAN interrupt                                   */
  /* not necessary */


  /* 4. Enable CAN interrupt                                                 */
  NVIC->IPR [4] |= 0x10000000;                /* set priority lower than SVC */
  NVIC->ISER[0] |= (1 << (USB_HP_CAN_TX_IRQChannel  & 0x1F));

                                              /* only FIFO 0 is used         */
  NVIC->IPR [5] |= 0x00000010;                /* set priority lower than SVC */
  NVIC->ISER[0] |= (1 << (USB_LP_CAN_RX0_IRQChannel & 0x1F)); 

  return CAN_OK;
}

/*--------------------------- CAN_hw_init -----------------------------------
 *
 *  Initialize the CAN hardware
 *
 *  Parameter:  ctrl:       Ignored
 *              baudrate:   Baudrate
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/
U32 gCtrl;
U32 gBaudrate;
CAN_ERROR CAN_hw_init (U32 ctrl, U32 baudrate)
{
//徐成
#if CAN_ART_Enable  
								 
  	CAN->MCR = CAN_MCR_INRQ;
#else
  	CAN->MCR = (CAN_MCR_NART | CAN_MCR_INRQ);  /* init mode,                   */
                                             /* disable auto. retransmission */
#endif
                                  /* only FIFO 0, transmit mailbox 0 used    */
  	CAN->IER = (CAN_IER_FMPIE0 | CAN_IER_TMEIE); /* FIFO 0 msg pending,        */ 
                                               /* Transmit mbx empty         */     

  	if (CAN_hw_set_baudrate(ctrl, baudrate) != CAN_OK)         /* Set baudrate */
    	return CAN_BAUDRATE_ERROR;
	
  	gCtrl = ctrl;
  	gBaudrate = baudrate;

  	return CAN_OK;
}

CAN_ERROR CAN_hw_init_ex	(U32 ctrl, U32 baudrate, U8 bArt)
{
	//徐成
	if(bArt > 0)
	{
		CAN->MCR = CAN_MCR_INRQ;
	}
	else
	{
		CAN->MCR = (CAN_MCR_NART | CAN_MCR_INRQ);
	}
	                              								/* only FIFO 0, transmit mailbox 0 used    */
	CAN->IER = (CAN_IER_FMPIE0 | CAN_IER_TMEIE); 				/* FIFO 0 msg pending,        */ 
	                                       						/* Transmit mbx empty         */     
	
	if (CAN_hw_set_baudrate(ctrl, baudrate) != CAN_OK)         	/* Set baudrate */
	return CAN_BAUDRATE_ERROR;
	
	gCtrl = ctrl;
	gBaudrate = baudrate;
	
	return CAN_OK;
}

CAN_ERROR CAN_hw_reinit(U32 ctrl, U32 baudrate)
{
	//CAN强行复位
	CAN->MCR = CAN_MCR_RESET;
	CAN->MCR &= ~CAN_MCR_SLEEP;
	
	return CAN_hw_init(ctrl, baudrate);
}

CAN_ERROR CAN_hw_reinit_ex(U32 ctrl, U32 baudrate, U8 bArt)
{
	//CAN强行复位
	CAN->MCR = CAN_MCR_RESET;
	CAN->MCR &= ~CAN_MCR_SLEEP;
	
	return CAN_hw_init_ex(ctrl, baudrate, bArt);
}




/*--------------------------- CAN_hw_start ----------------------------------
 *
 *  reset CAN initialisation mode
 *
 *  Parameter:  ctrl:       Ignored
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_hw_start (U32 ctrl)  
{

  CAN->MCR &= ~CAN_MCR_INRQ;          /* normal operating mode, reset INRQ   */
 //徐成
#if CAN_ABOM_Enable
  CAN->MCR |= CAN_MCR_ABOM;
#endif
  while (CAN->MSR & CAN_MCR_INRQ);

  return CAN_OK;
}

CAN_ERROR CAN_hw_start_ex(U32 ctrl, U8 bABOM)
{
	CAN->MCR &= ~CAN_MCR_INRQ;          /* normal operating mode, reset INRQ   */
	
	if(bABOM > 0)
	  CAN->MCR |= CAN_MCR_ABOM;
  
	while (CAN->MSR & CAN_MCR_INRQ);
	
	return CAN_OK;
}

/*--------------------------- CAN_set_testmode ------------------------------
 *
 *  Setup the CAN testmode
 *
 *  Parameter:  ctrl:       Ignored
 *              testmode:   kind of testmode
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/
CAN_ERROR CAN_hw_testmode (U32 ctrl, U32 testmode) {

  CAN->BTR &= ~(CAN_BTR_SILM | CAN_BTR_LBKM); 
  CAN->BTR |=  (testmode & (CAN_BTR_SILM | CAN_BTR_LBKM));

  return CAN_OK;
}

/*--------------------------- CAN_hw_tx_empty -------------------------------
 *
 *  Check if transmit mailbox 0 is available for usage (empty)
 *
 *  Parameter:  ctrl:       Ignored
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_hw_tx_empty (U32 ctrl)  {

  if ((os_sem_wait (wr_sem[ctrl-1], 0) != OS_R_TMO)){ /* If semaphore is free*/
  if ((CAN->TSR & CAN_TSR_TME0) != 0) /* Transmit mailbox 0 is empty         */
    return CAN_OK;
    else 
      os_sem_send(wr_sem[ctrl-1]);    /* Return a token back to semaphore    */
  }

  return CAN_TX_BUSY_ERROR;
}

/*--------------------------- CAN_hw_wr -------------------------------------
 *
 *  Write CAN_msg to the hardware registers of the requested controller
 *
 *  Parameter:  ctrl:       Ignored
 *              msg:        Pointer to CAN message to be written to hardware
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_hw_wr (U32 ctrl, CAN_msg *msg)  {

  /* Reset TIR register                                                      */
  CAN->sTxMailBox[0].TIR  = (U32)0;                        /* reset TXRQ bit */

  /* Setup the identifier information                                        */
  if (msg->format == STANDARD_FORMAT)  {                   /*    Standard ID */
      CAN->sTxMailBox[0].TIR |= (U32)(msg->id << 21) | CAN_ID_STD;
  }  else  {                                               /* Extended ID    */
      CAN->sTxMailBox[0].TIR |= (U32)(msg->id <<  3) | CAN_ID_EXT;
  }

  /* Setup type information                                                  */
  if (msg->type == DATA_FRAME)  {                          /* DATA FRAME     */
      CAN->sTxMailBox[0].TIR |= CAN_RTR_DATA;
  }
  else {
      CAN->sTxMailBox[0].TIR |= CAN_RTR_REMOTE;
  }

  /* Setup data bytes                                                        */
  CAN->sTxMailBox[0].TDLR = (((U32)msg->data[3] << 24) | 
                             ((U32)msg->data[2] << 16) |
                             ((U32)msg->data[1] <<  8) | 
                             ((U32)msg->data[0])        );
  CAN->sTxMailBox[0].TDHR = (((U32)msg->data[7] << 24) | 
                             ((U32)msg->data[6] << 16) |
                             ((U32)msg->data[5] <<  8) |
                             ((U32)msg->data[4])        );

  /* Setup length                                                            */
  CAN->sTxMailBox[0].TDTR &= ~CAN_TDTxR_DLC;
  CAN->sTxMailBox[0].TDTR |=  (msg->len & CAN_TDTxR_DLC);

  CAN->IER |= CAN_IER_TMEIE;                       /*  enable  TME interrupt */ 

  /*  transmit message                                                       */
  CAN->sTxMailBox[0].TIR |=  CAN_TIxR_TXRQ;                /*   set TXRQ bit */

  return CAN_OK;
}

/*--------------------------- CAN_hw_rd -------------------------------------
 *
 *  Read CAN_msg from the hardware registers of the requested controller
 *
 *  Parameter:  ctrl:       Ignored
 *              ch:         Ignored
 *              msg:        Pointer where CAN message will be read
 *
 *  Return:     none
 *---------------------------------------------------------------------------*/

static void CAN_hw_rd (U32 ctrl, U32 ch, CAN_msg *msg)  {

  /* Read identifier information                                             */
  if ((CAN->sFIFOMailBox[0].RIR & CAN_ID_EXT) == 0) {       /* Standard ID   */
    msg->format = STANDARD_FORMAT;
    msg->id     = (u32)0x000007FF & (CAN->sFIFOMailBox[0].RIR >> 21);
  }  else  {                                                /* Extended ID   */
    msg->format = EXTENDED_FORMAT;
    msg->id     = (u32)0x1FFFFFFF & (CAN->sFIFOMailBox[0].RIR >> 3);
  }

  /* Read type information                                                   */
  if ((CAN->sFIFOMailBox[0].RIR & CAN_RTR_REMOTE) == 0) {
    msg->type =   DATA_FRAME;                               /* DATA   FRAME  */
  }  else  {
    msg->type = REMOTE_FRAME;                               /* REMOTE FRAME  */
  }

  /* Read length (number of received bytes)                                  */
  msg->len = (U32)0x0000000F & CAN->sFIFOMailBox[0].RDTR;

  /* Read data bytes                                                         */
  msg->data[0] = (U32)0x000000FF & (CAN->sFIFOMailBox[0].RDLR);
  msg->data[1] = (U32)0x000000FF & (CAN->sFIFOMailBox[0].RDLR >> 8);
  msg->data[2] = (U32)0x000000FF & (CAN->sFIFOMailBox[0].RDLR >> 16);
  msg->data[3] = (U32)0x000000FF & (CAN->sFIFOMailBox[0].RDLR >> 24);

  msg->data[4] = (U32)0x000000FF & (CAN->sFIFOMailBox[0].RDHR);
  msg->data[5] = (U32)0x000000FF & (CAN->sFIFOMailBox[0].RDHR >> 8);
  msg->data[6] = (U32)0x000000FF & (CAN->sFIFOMailBox[0].RDHR >> 16);
  msg->data[7] = (U32)0x000000FF & (CAN->sFIFOMailBox[0].RDHR >> 24);

  CAN->RF0R |= CAN_RF0R_RFOM0;              /* Release FIFO 0 output mailbox */
}

/*--------------------------- CAN_hw_set ------------------------------------
 *  Set a message that will automatically be sent as an answer to the REMOTE
 *  FRAME message
 *
 *  Parameter:  ctrl:       Ignored
 *              msg:        Pointer to CAN message to be set
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_hw_set (U32 ctrl, CAN_msg *msg)  {

  return CAN_NOT_IMPLEMENTED_ERROR;
}

/*--------------------------- CAN_hw_rx_object ------------------------------
 *
 *  This function setups object that is going to be used for the message 
 *  reception
 *
 *  Parameter:  ctrl:       Ignored
 *              ch:         Index of object used for reception
 *              id:         Identifier of receiving messages
 *              CAN_FORMAT: Format of CAN identifier (standard or extended)
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

static S16 CAN_filterIdx = 0;
CAN_ERROR CAN_hw_rx_object (U32 ctrl, U32 ch, U32 id, CAN_FORMAT format)  {
  U32 CAN_msgId     = 0;
  
  /* check if Filter Memory is full                                          */
  if (CAN_filterIdx > 13) {
    return CAN_OBJECTS_FULL_ERROR;	      /* no more filter number available */
  }
  /* Setup the identifier information                                        */
  if (format == STANDARD_FORMAT)  {       /* Standard ID                     */
      CAN_msgId  |= (U32)(id << 21) | CAN_ID_STD;
  }  else  {                              /* Extended ID                     */
      CAN_msgId  |= (U32)(id <<  3) | CAN_ID_EXT;
  }

  CAN->FMR  |=  CAN_FMR_FINIT; /*   set Initialisation mode for filter banks */
  CAN->FA1R &=  ~(U32)(1 << CAN_filterIdx);               /* activate filter */

  /* initialize filter 0 */   
  CAN->FS1R |= (U32)(1 << CAN_filterIdx);  /* set 32-bit scale configuration */
  CAN->FM1R |= (U32)(1 << CAN_filterIdx);/* set 2 32-bit identifier list mode*/

  CAN->sFilterRegister[CAN_filterIdx].FR1 = CAN_msgId; /*  32-bit identifier */
  CAN->sFilterRegister[CAN_filterIdx].FR2 = CAN_msgId; /*  32-bit identifier */
    													   
  CAN->FFA1R &= ~(U32)(1 << CAN_filterIdx);       /* assign filter to FIFO 0 */
  CAN->FA1R  |=  (U32)(1 << CAN_filterIdx);               /* activate filter */

  CAN->FMR &= ~CAN_FMR_FINIT;  /* reset Initialisation mode for filter banks */

  CAN_filterIdx += 1;                              /* increase filter number */

  return CAN_OK;
}
    
CAN_ERROR CAN_hw_rx_object_mask (U32 ctrl, U32 ch, U32 id, U32 mask, CAN_FORMAT format)  {

         U32 CAN_msgId     = 0;
		 U32 CAN_msgMask     = 0;
  
  /* check if Filter Memory is full                                          */
  if (CAN_filterIdx > 13) {
    return CAN_OBJECTS_FULL_ERROR;	      /* no more filter number available */
  }
  /* Setup the identifier information                                        */
  if (format == STANDARD_FORMAT)  {       /* Standard ID                     */
      CAN_msgMask  |= (U32)(mask << 21) | CAN_ID_STD;
  }  else  {                              /* Extended ID                     */
      CAN_msgMask  |= (U32)(mask <<  3) | CAN_ID_EXT;
  }

   if (format == STANDARD_FORMAT)  {       /* Standard ID                     */
      CAN_msgId  |= (U32)(id << 21) | CAN_ID_STD;
  }  else  {                              /* Extended ID                     */
      CAN_msgId  |= (U32)(id <<  3) | CAN_ID_EXT;
  }

  CAN->FMR  |=  CAN_FMR_FINIT; /*   set Initialisation mode for filter banks */
  CAN->FA1R &=  ~(U32)(1 << CAN_filterIdx);               /* activate filter */

  /* initialize filter 0 */   
  CAN->FS1R |= (U32)(1 << CAN_filterIdx);  /* set 32-bit scale configuration */
  CAN->FM1R &= ~(U32)(1 << CAN_filterIdx); /* set 32-bit identifier + 32-bit Mask mode*/

  CAN->sFilterRegister[CAN_filterIdx].FR1 = CAN_msgId; 	 /*  32-bit identifier */
  CAN->sFilterRegister[CAN_filterIdx].FR2 = CAN_msgMask; /*  32-bit Mask */
    													   
  CAN->FFA1R &= ~(U32)(1 << CAN_filterIdx);       /* assign filter to FIFO 0 */
  CAN->FA1R  |=  (U32)(1 << CAN_filterIdx);               /* activate filter */

  CAN->FMR &= ~CAN_FMR_FINIT;  /* reset Initialisation mode for filter banks */

  CAN_filterIdx += 1;                              /* increase filter number */

  return CAN_OK;
}


//带MASK和INDEX的FILTER
CAN_ERROR CAN_hw_rx_object_mask_idx (U32 ctrl, U32 ch, U32 id, U32 mask, U8 idx, U8 enable, CAN_FORMAT format)  
{
  U32 CAN_msgId     = 0;
  U32 CAN_msgMask     = 0;
  
  /* check if Filter Memory is full                                          */
  if (idx > 13) {
    return CAN_OBJECTS_FULL_ERROR;	      /* no more filter number available */
  }
  /* Setup the identifier information                                        */
  if (format == STANDARD_FORMAT)  {       /* Standard ID                     */
      CAN_msgMask  |= (U32)(mask << 21) | CAN_ID_STD;
  }  else  {                              /* Extended ID                     */
      CAN_msgMask  |= (U32)(mask <<  3) | CAN_ID_EXT;
  }

   if (format == STANDARD_FORMAT)  {       /* Standard ID                     */
      CAN_msgId  |= (U32)(id << 21) | CAN_ID_STD;
  }  else  {                              /* Extended ID                     */
      CAN_msgId  |= (U32)(id <<  3) | CAN_ID_EXT;
  }

  CAN->FMR  |=  CAN_FMR_FINIT; /*   set Initialisation mode for filter banks */
  CAN->FA1R &=  ~(U32)(1 << idx);              				 /* activate filter */

  /* initialize filter 0 */   
  CAN->FS1R |= (U32)(1 << idx);  /* set 32-bit scale configuration */
  CAN->FM1R &= ~(U32)(1 << idx); /* set 32-bit identifier + 32-bit Mask mode*/

  CAN->sFilterRegister[idx].FR1 = CAN_msgId; 	 /*  32-bit identifier */
  CAN->sFilterRegister[idx].FR2 = CAN_msgMask; /*  32-bit Mask */
    													   
  CAN->FFA1R &= ~(U32)(1 << idx);       /* assign filter to FIFO 0 */
  
  if (enable)
  {
    CAN->FA1R |=  (U32)(1 << idx);
  }

  CAN->FMR &= ~CAN_FMR_FINIT;  /* reset Initialisation mode for filter banks */

  return CAN_OK;
}

/*--------------------------- CAN_hw_tx_object ------------------------------
 *
 *  This function setups object that is going to be used for the message 
 *  transmission, the setup of transmission object is not necessery so this 
 *  function is not implemented
 *
 *  Parameter:  ctrl:       Ignored
 *              ch:         Index of object used for transmission
 *              CAN_FORMAT: Format of CAN identifier (standard or extended)
 *
 *  Return:     CAN_ERROR:  Error code
 *---------------------------------------------------------------------------*/

CAN_ERROR CAN_hw_tx_object (U32 ctrl, U32 ch, CAN_FORMAT format)  {

  return CAN_NOT_IMPLEMENTED_ERROR;
}


/************************* Interrupt Functions *******************************/

/*--------------------------- CAN_IRQ_Handler -------------------------------
 *
 *  CAN interrupt function 
 *  If transmit interrupt occured and there are messages in mailbox for 
 *  transmit it writes it to hardware and starts the transmission
 *  If receive interrupt occured it reads message from hardware registers 
 *  and puts it into receive mailbox
 *---------------------------------------------------------------------------*/

#define CTRL0                 0
#define CTRL                  1
#define CANc                  CAN

void USB_HP_CAN_TX_IRQHandler (void) {
  CAN_msg *ptrmsg;

  if (CAN->TSR & CAN_TSR_RQCP0) {                  /* request completed mbx 0 */
    CAN->TSR |= CAN_TSR_RQCP0;                /* reset request complete mbx 0 */

    /* If there is a message in the mailbox ready for send, read the 
       message from the mailbox and send it                                  */
    if (isr_mbx_receive (MBX_tx_ctrl[CTRL0], (void **)&ptrmsg) != OS_R_OK) {

      CAN_hw_wr (CTRL, ptrmsg);
      _free_box(CAN_mpool, ptrmsg);
    } else {
      isr_sem_send(wr_sem[CTRL0]);  /* Return a token back to semaphore      */
	                      
      CAN->IER &= ~CAN_IER_TMEIE;                  /* disable  TME interrupt */ 
    }
  }

}

void USB_LP_CAN_RX0_IRQHandler (void) {
  CAN_msg *ptrmsg;

  if (CAN->RF0R & CAN_RF0R_FMP0) {			            /* message pending ? */
    /* If the mailbox isn't full read the message from the hardware and
	   send it to the message queue                                          */
    if (isr_mbx_check (MBX_rx_ctrl[CTRL0]) > 0)  {

      ptrmsg = _alloc_box (CAN_mpool);
      CAN_hw_rd (CTRL, 0, ptrmsg);    /* Read and release received message   */
      isr_mbx_send (MBX_rx_ctrl[CTRL0], ptrmsg);
    }
	else {
      CAN->RF0R |= CAN_RF0R_RFOM0;          /* Release FIFO 0 output mailbox */
	}
  }
}

/*--------------------------- CAN_SetErrCallBack ------------------------------
 *
 *	设置错误处理回调函数
 *
 *  参数:  ceh：错误处理函数指针
 *---------------------------------------------------------------------------*/
static CANERRHANDLER gCanErrHandler = NULL;
void CAN_err_config(CANERRHANDLER ceh)
{
	//徐成改动
	NVIC_InitTypeDef NVIC_InitStructure;

	gCanErrHandler = ceh;
	
	//徐成
  NVIC_InitStructure.NVIC_IRQChannel = CAN_SCE_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	CAN_ITConfig(CAN_IT_ERR|CAN_IT_EWG|CAN_IT_EPV|CAN_IT_BOF, ENABLE);		//错误警告中断屏蔽|错误被动中断屏蔽|上次错误号中断屏蔽|错误中断屏蔽
}

void CAN_SCE_IRQHandler(void)
{
	if(gCanErrHandler != NULL)
	{
		u32 errIT;
		u8 errType;
		if(CAN_GetFlagStatus(CAN_FLAG_BOF) == SET)
		{
			errIT = CAN_FLAG_BOF;
		}
		else if(CAN_GetFlagStatus(CAN_FLAG_EPV) == SET)
		{
			errIT = CAN_FLAG_EPV;
		}
		else if(CAN_GetFlagStatus(CAN_FLAG_EWG) == SET)
		{
			errIT = CAN_FLAG_EWG;
		}
		
		errType = CAN->ESR & 0x00000070;
		
		gCanErrHandler((CAN_HW_ERR)(errType>>4), errIT);
	}
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

