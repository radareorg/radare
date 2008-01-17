///////////////////////////////////////////////////////////////////////////////////////////////
//					USB Gecko - Flash library	- (c) Nuke	-  www.usbgecko.com				 //
///////////////////////////////////////////////////////////////////////////////////////////////


#include "flash.h"
#include "usb.h"
#include "delay.h"

///////////////////////////////////////////////////////////////////////////////////////////////
//								Function: flash_writecommand								 //	 
//					Write command to flash memory, not called directly						 //
///////////////////////////////////////////////////////////////////////////////////////////////

void flash_writecommand (unsigned int flashaddress, unsigned char flashdata)
{
	unsigned int datatosend;
	unsigned int flashcommand = 0xE0000000;		
	datatosend = flashcommand | (flashaddress<<9) | (flashdata<<1);

	exi_chan1sr = 0x000000C0;					// Memory Card Port B (Channel 1, Device 0, Frequnecy 3 (16Mhz Clock)
	exi_chan1data = datatosend;					
	exi_chan1cr = 0x35;							// Write / Imm / 4 bytes
	while((exi_chan1cr)&1);						// Wait for Transfer to Complete   	
	exi_chan1sr = 0x000000C0;					
	exi_chan1cr = 0x35;							
	while((exi_chan1cr)&1);						
	exi_chan1sr = 0x0;							
}

///////////////////////////////////////////////////////////////////////////////////////////////
//							Function: flash_readcommand										 //	 
//							Read from flash memory address									 //
///////////////////////////////////////////////////////////////////////////////////////////////

unsigned char flash_readcommand (unsigned int flashaddress)
{
	unsigned int datatosend;
	unsigned int flashcommand = 0xF0000000;	
	unsigned int readbyte = 0;
	datatosend = flashcommand | (flashaddress<<9);
	exi_chan1sr = 0x0;
	exi_chan1sr = 0x000000C0;					// Memory Card Port B (Channel 1, Device 0, Frequnecy 3 (16Mhz Clock)
	exi_chan1data = datatosend;					
	exi_chan1cr = 0x35;							// Write / Imm / 4 bytes 39
	while((exi_chan1cr)&1);						// Wait for Transfer to Complete   	
	exi_chan1sr = 0x000000C0;
	exi_chan1cr = 0x39;							
	while((exi_chan1cr)&1);						 
	readbyte = exi_chan1data<<1;					
	exi_chan1sr = 0x0;	
	return ((readbyte>>24)&0xff);				
												
}
///////////////////////////////////////////////////////////////////////////////////////////////
//							Function: flash_chiperase										 //	 
//						Erases chip, call before programming								 //
///////////////////////////////////////////////////////////////////////////////////////////////

void flash_chiperase()
{
	flash_writecommand (0x5555,0xAA);		
	flash_writecommand (0x2AAA,0x55);
	flash_writecommand (0x5555,0x80);
	flash_writecommand (0x5555,0xAA);
	flash_writecommand (0x2AAA,0x55);
	flash_writecommand (0x5555,0x10);
	mdelay(100);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//							Function: flash_programbyte										 //	 
//							Programs a byte at given address								 //
///////////////////////////////////////////////////////////////////////////////////////////////

void flash_programbyte(unsigned long flashaddress, unsigned char flashdata)

{
	flash_writecommand (0x5555,0xAA);				// SST chip erase commands		
	flash_writecommand (0x2AAA,0x55);				
	flash_writecommand (0x5555,0xA0);			
	flash_writecommand (flashaddress,flashdata);	// Send flash address and data
	flash_togglewait();
}

///////////////////////////////////////////////////////////////////////////////////////////////
//							Function: flash_togglewait										 //	 
//			Togglewait, not called directly, could be replaced by timing (see datasheet)	 //
///////////////////////////////////////////////////////////////////////////////////////////////

void flash_togglewait()
{
    unsigned char temp = 0;
    unsigned char temp1 = flash_readcommand (0x0000);	// Read byte
    
	do											// loop
    {
        temp = temp1;							// Store temp
        temp1 = flash_readcommand (0x0000);		// Read same byte
    } while (temp!=temp1);						// If D6 match then toggle end
   
	flash_readcommand (0x0000);					// Two reads of same address (See datasheet)
    flash_readcommand (0x0000);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//							Function: flash_softwareentry									 //	 
//				Enter flash software mode, used by verify function							 //
///////////////////////////////////////////////////////////////////////////////////////////////

void flash_softwareentry()
{
	flash_writecommand (0x5555,0xAA);			
	flash_writecommand (0x2AAA,0x55);
	flash_writecommand (0x5555,0x90);	
}

///////////////////////////////////////////////////////////////////////////////////////////////
//							Function: flash_softwareexit									 //	 
//				Exit flash software mode, used by verify function							 //
///////////////////////////////////////////////////////////////////////////////////////////////

void flash_softwareexit()

{
	flash_writecommand (0x5555,0xAA);
	flash_writecommand (0x2AAA,0x55);
	flash_writecommand (0x5555,0xF0);		
}

///////////////////////////////////////////////////////////////////////////////////////////////
//							Function: flash_verify											 //	 
//					Verify flash by device and manufacturing ID								 //
///////////////////////////////////////////////////////////////////////////////////////////////

int flash_verify()

{
	unsigned char deviceid;
	unsigned char manuid;
	
	flash_softwareentry();
	manuid = flash_readcommand (0);		// address 0 is Manufacture ID
	deviceid = flash_readcommand (1);	// address 1 is Device ID
	flash_softwareexit();

	if((manuid == 0xBF) && (deviceid == 0xD7)) {
		return 1;
	}
	else {
		return 0;
	}
}
