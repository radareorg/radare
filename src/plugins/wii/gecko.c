
int gecko_opendevice()
{
	// Open by Serial Number
	status = FT_OpenEx("GECKUSB0", FT_OPEN_BY_SERIAL_NUMBER, &fthandle);
	if(status != FT_OK){
		eprintf("Error: Couldn't connect to USB Gecko. Please check Installation\n");
		return 0;
	}
	// Reset the Device
	status = FT_ResetDevice(fthandle);
	if(status != FT_OK){
		eprintf("Error: Couldnt Reset Device %d\n",status);
		status = FT_Close(fthandle);
		return 0;
	}
	// Set a 3 second timeout for this example
	status = FT_SetTimeouts(fthandle,3000,3000);
	if(status != FT_OK){
		eprintf("Error: Timeouts failed to set %d\n",status);
		status = FT_Close(fthandle);
		return 0;
	}
	// Purge RX buffer
	status = FT_Purge(fthandle,FT_PURGE_RX);
	if(status != FT_OK){
		eprintf("Error: Problem clearing buffers %d\n",status);
		status = FT_Close(fthandle);
		return 0;
	}
	// Purge TX buffer
	status = FT_Purge(fthandle,FT_PURGE_TX);
	if(status != FT_OK){
		eprintf("Error: Problem clearing buffers %d\n",status);
		status = FT_Close(fthandle);
		return 0;
	}
	// Set packet size in bytes - 65536 packet is maximum packet size (USB 2.0)
	status = FT_SetUSBParameters(fthandle,65536,0);
	if(status != FT_OK){
		eprintf("Error: Couldnt Set USB Parameters %d\n",status);
		status = FT_Close(fthandle);
		return 0;
	}
	// take breath
	sleep(1);
	return 1;
}

int gecko_sendbyte(unsigned char Textbyte)
{
	unsigned int ret = 0;

	ret = gecko_writebyte(&cmd_sendbyte, 1, &TxSent);
	if(ret == 0 || ret == 2) {
		eprintf("Couldn't send the command to console, check DOL is loaded and connection\n");
		status = FT_Close(fthandle);
		return 0;
	}
	gecko_writebyte(&Testbyte, 1, &TxSent);  // Send byte

	return 1;
}

int gecko_receivebyte()
{
	unsigned char Consoleresponse; // Holder for byte
	unsigned int ret = 0;

	ret = gecko_writebyte(&cmd_receivebyte, 1, &TxSent);
	if(ret == 0 || ret == 2) {
		eprintf("Couldn't send the command to console, check DOL is loaded and connection\n");
		status = FT_Close(fthandle);
		return -2;
	}

	ret = gecko_readbyte(&Consoleresponse, 1, &RxSent);
	if(ret == 1)
		return Consoleresponse;

	return -1;
}

int gecko_readbyte(LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpdwBytesReturned)
{
        // read data based on FTDI D2XX USB 2.0 API
        status = FT_Read(fthandle, lpBuffer, dwBytesToRead, lpdwBytesReturned);

        if (status == FT_OK) {
                if(*lpdwBytesReturned != dwBytesToRead)
                        return 2;
        } else {
                eprintf("Error: Read Error. Closing\n");
                status = FT_Close(fthandle);
                return 0;
        }
        return 1;

}

int gecko_writebyte(LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpdwBytesWritten)
{
        // write data based on FTDI D2XX USB 2.0 API
        status = FT_Write(fthandle, lpBuffer, dwBytesToWrite, lpdwBytesWritten);

        if (status == FT_OK) {
                if(*lpdwBytesWritten != dwBytesToWrite)
			return 2;      // Can be used for packet retry code ect
        } else {
                eprintf("Error: Write Error. Closing.\n");
                status = FT_Close(fthandle);                            // Close device if fatal error
                return 0;
        }
	// Packet Sent
        return 1;
}

int gecko_write(FT_HANDLE wii_fd, unsigned char *buf, int len)
{
	int ret;

	for (i=0;i<len;i++) {
		if (!gecko_sendbyte(buf[i])) {
			// TODO : use status
			return -1;
		}
	}

	return len;
}

int gecko_read(FT_HANDLE wii_fd, unsigned char *buf, int len)
{
	int ret;

	for (i=0;i<len;i++) {
		ret = gecko_receivebyte();
		if (ret == -1) {
			// TODO: retry n times with sleep
			return len;
		} else
		if (ret == -2) {
			return -1;
		}
		buf[i] = (unsigned char)ret;
	}

	return len;
}
