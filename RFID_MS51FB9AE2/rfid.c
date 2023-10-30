#include "MS51_16K.H"
#include "spi.h"
#include "rfid.h"




 unsigned char  RC522_SPI_Transfer( unsigned char  Data)
{
	 unsigned char  rx_Data;
	
	SPI_Tx(Data);
	
	Timer0_Delay(16000000,200,100);            /* check delay.c*/
	
	SPI_Rx(rx_Data);
	
	Timer0_Delay(16000000,200,100);            /* check delay.c*/
	
	return rx_Data;
}

void Write_MFRC522( unsigned char  addr,  unsigned char  val) {
	/* CS LOW */
	SS= 0x00; 
	//The address is located:0XXXXXX0
	RC522_SPI_Transfer((addr << 1) & 0x7E);
	RC522_SPI_Transfer(val);

	/* CS HIGH */
	SS = 0xFF; 
}

 unsigned char  Read_MFRC522( unsigned char  addr) {
	unsigned char  val;

	/* CS LOW */
	SS = 0x00;

	//The address is located:1XXXXXX0
	RC522_SPI_Transfer(((addr << 1) & 0x7E) | 0x80);
	val = RC522_SPI_Transfer(0x00);

	/* CS HIGH */
	SS = 0xFF;
	return val;

}

void SetBitMask( unsigned char  reg,  unsigned char  mask) {
	 unsigned char  tmp;
	tmp = Read_MFRC522(reg);
	Write_MFRC522(reg, tmp | mask);  // set bit mask
}

void ClearBitMask( unsigned char  reg,  unsigned char  mask) {
	 unsigned char  tmp;
	tmp = Read_MFRC522(reg);
	Write_MFRC522(reg, tmp & (~mask));  // clear bit mask
}

void AntennaOn(void) {

	Read_MFRC522(TxControlReg);

	SetBitMask(TxControlReg, 0x03);
}

void AntennaOff(void) {
	ClearBitMask(TxControlReg, 0x03);
}

void MFRC522_Reset(void) {
	Write_MFRC522(CommandReg, PCD_RESETPHASE);
}

void MFRC522_Init(void) {

	//GPIO_SetBits(MFRC522_CS_GPIO,MFRC522_CS_PIN);						// Activate the RFID reader
	SS = 0x00;
	P17 = 0xFF;
	//GPIO_SetBits(MFRC522_RST_GPIO,MFRC522_RST_PIN);					// not reset

	// spi config
	//MFRC522_SPI_Init();

	MFRC522_Reset();

	//Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
	Write_MFRC522(TModeReg, 0x8D);		//auto=1; f(Timer) = 6.78MHz/TPreScaler
	Write_MFRC522(TPrescalerReg, 0x3E);	//TModeReg[3..0] + TPrescalerReg
	Write_MFRC522(TReloadRegL, 30);
	Write_MFRC522(TReloadRegH, 0);

	Write_MFRC522(TxAutoReg, 0x40);		//100%ASK
	Write_MFRC522(ModeReg, 0x3D);		//CRC Original value 0x6363	???

	//ClearBitMask(Status2Reg, 0x08);		//MFCrypto1On=0
	//Write_MFRC522(RxSelReg, 0x86);		//RxWait = RxSelReg[5..0]
	//Write_MFRC522(RFCfgReg, 0x7F);   		//RxGain = 48dB

	AntennaOn();		//Mo Anten
}

 unsigned char  MFRC522_ToCard( unsigned char  command,  unsigned char  *sendData,  unsigned char  sendLen,  unsigned char  *backData,  unsigned int  *backLen)
{
	 unsigned char  status = MI_ERR;
	 unsigned char  irqEn = 0x00;
	 unsigned char  waitIRq = 0x00;
	 unsigned char  lastBits;
	 unsigned char  n;
	 unsigned int  i;

	switch (command) {
	case PCD_AUTHENT:		//Acknowledging the liver
	{
		irqEn = 0x12;
		waitIRq = 0x10;
		break;
	}
	case PCD_TRANSCEIVE:	// FIFO Data collection
	{
		irqEn = 0x77;
		waitIRq = 0x30;
		break;
	}
	default:
		break;
	}

	Write_MFRC522(CommIEnReg, irqEn | 0x80);	//Yeu cau ngat
	ClearBitMask(CommIrqReg, 0x80);			//Clear all the bits
	SetBitMask(FIFOLevelReg, 0x80);			//FlushBuffer=1, FIFO

	Write_MFRC522(CommandReg, PCD_IDLE);//NO action; Huy bo lenh hien hanh	???

	// Record in FIFO
	for (i = 0; i < sendLen; i++) {
		Write_MFRC522(FIFODataReg, sendData[i]);
	}

	//chay
	Write_MFRC522(CommandReg, command);
	if (command == PCD_TRANSCEIVE) {
		SetBitMask(BitFramingReg, 0x80);//StartSend=1,transmission of Data starts  
	}

	//The team is allowed to be stored
	i = 2000;//i tuy thuoc tan so thach anh, thoi gian toi da cho the M1 la 25ms
	do {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = Read_MFRC522(CommIrqReg);
		i--;
	} while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

	ClearBitMask(BitFramingReg, 0x80);			//StartSend=0

	if (i != 0) {
		if (!(Read_MFRC522(ErrorReg) & 0x1B))//BufferOvfl Collerr CRCErr ProtecolErr
		{
			status = MI_OK;
			if (n & irqEn & 0x01) {
				status = MI_NOTAGERR;			//??   
			}

			if (command == PCD_TRANSCEIVE) {
				n = Read_MFRC522(FIFOLevelReg);
				lastBits = Read_MFRC522(ControlReg) & 0x07;
				if (lastBits) {
					*backLen = (n - 1) * 8 + lastBits;
				} else {
					*backLen = n * 8;
				}

				if (n == 0) {
					n = 1;
				}
				if (n > MAX_LEN) {
					n = MAX_LEN;
				}

				//FIFO doc in the received Data
				for (i = 0; i < n; i++) {
					backData[i] = Read_MFRC522(FIFODataReg);
				}
			}
		} else {
			status = MI_ERR;
		}

	}

	//SetBitMask(ControlReg,0x80);           //timer stops
	//Write_MFRC522(CommandReg, PCD_IDLE); 

	return status;
}

 unsigned char  MFRC522_Request( unsigned char  reqMode,  unsigned char  *TagType) {
	 unsigned char  status;
	 unsigned int  backBits;			//The bits are manipulated

	Write_MFRC522(BitFramingReg, 0x07);	//TxLastBists = BitFramingReg[2..0]	???

	TagType[0] = reqMode;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK) || (backBits != 0x10)) {
		status = MI_ERR;
	}

	return status;
}

 unsigned char  MFRC522_Anticoll( unsigned char  *serNum) {
	 unsigned char  status;
	 unsigned char  i;
	 unsigned char  serNumCheck = 0;
	 unsigned int  unLen;

	//ClearBitMask(Status2Reg, 0x08);		//TempSensclear
	//ClearBitMask(CollReg,0x80);			//ValuesAfterColl
	Write_MFRC522(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]

	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK) {
		//Check the serial number
		for (i = 0; i < 4; i++) {
			serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i]) {
			status = MI_ERR;
		}
	}

	//SetBitMask(CollReg, 0x80);		//ValuesAfterColl=1

	return status;
}

void CalulateCRC( unsigned char  *pInData,  unsigned char  len,  unsigned char  *pOutData) {
	 unsigned char  i, n;

	ClearBitMask(DivIrqReg, 0x04);			//CRCIrq = 0
	SetBitMask(FIFOLevelReg, 0x80);			//Con tro FIFO
	//Write_MFRC522(CommandReg, PCD_IDLE);

	//Record in FIFO
	for (i = 0; i < len; i++) {
		Write_MFRC522(FIFODataReg, *(pInData + i));
	}
	Write_MFRC522(CommandReg, PCD_CALCCRC);

	// Let the CRC computer complete
	i = 0xFF;
	do {
		n = Read_MFRC522(DivIrqReg);
		i--;
	} while ((i != 0) && !(n & 0x04));			//CRCIrq = 1

	//Doc results in CRC calculation
	pOutData[0] = Read_MFRC522(CRCResultRegL);
	pOutData[1] = Read_MFRC522(CRCResultRegM);
}

 unsigned char  MFRC522_SelectTag( unsigned char  *serNum) {
	 unsigned char  i;
	 unsigned char  status;
	 unsigned char  size;
	 unsigned int  recvBits;
	 unsigned char  buffer[9];

	//ClearBitMask(Status2Reg, 0x08);			//MFCrypto1On=0

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) {
		buffer[i + 2] = *(serNum + i);
	}
	CalulateCRC(buffer, 7, &buffer[7]);		//??
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18)) {
		size = buffer[0];
	} else {
		size = 0;
	}

	return size;
}

 unsigned char  MFRC522_Auth( unsigned char  authMode,  unsigned char  BlockAddr,  unsigned char  *Sectorkey,
	 unsigned char  *serNum) {
	 unsigned char  status;
	 unsigned int  recvBits;
	 unsigned char  i;
	 unsigned char  buff[12];

	//Confirmation + Address + password + quick number
	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i = 0; i < 6; i++) {
		buff[i + 2] = *(Sectorkey + i);
	}
	for (i = 0; i < 4; i++) {
		buff[i + 8] = *(serNum + i);
	}
	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

	if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08))) {
		status = MI_ERR;
	}

	return status;
}

 unsigned char  MFRC522_Read( unsigned char  blockAddr,  unsigned char  *recvData) {
	 unsigned char  status;
	 unsigned int  unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	CalulateCRC(recvData, 2, &recvData[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

	if ((status != MI_OK) || (unLen != 0x90)) {
		status = MI_ERR;
	}

	return status;
}

 unsigned char  MFRC522_Write( unsigned char  blockAddr,  unsigned char  *writeData) {
	 unsigned char  status;
	 unsigned int  recvBits;
	 unsigned char  i;
	 unsigned char  buff[18];

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	CalulateCRC(buff, 2, &buff[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) {
		status = MI_ERR;
	}

	if (status == MI_OK) {
		for (i = 0; i < 16; i++)		//16 FIFO bytes recorded
				{
			buff[i] = *(writeData + i);
		}
		CalulateCRC(buff, 16, &buff[16]);
		status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

		if ((status != MI_OK) || (recvBits != 4)
				|| ((buff[0] & 0x0F) != 0x0A)) {
			status = MI_ERR;
		}
	}

	return status;
}

void MFRC522_Halt(void) {
	 unsigned int  unLen;
	 unsigned char  buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	CalulateCRC(buff, 2, &buff[2]);

	MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}
