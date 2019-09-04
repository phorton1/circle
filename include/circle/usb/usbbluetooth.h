//
// usbbluetooth.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2015-2016  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _circle_usb_usbbluetooth_h
#define _circle_usb_usbbluetooth_h

#include <circle/usb/usbfunction.h>
#include <circle/bt/bttransportlayer.h>
#include <circle/usb/usbendpoint.h>
#include <circle/usb/usbrequest.h>
#include <circle/types.h>

class CUSBBluetoothDevice : public CUSBFunction
    #ifdef PRH_MODS
        ,btTransportBase
    #endif
{
public:
	CUSBBluetoothDevice (CUSBFunction *pFunction);
	~CUSBBluetoothDevice (void);

	boolean Configure (void);

	boolean SendHCICommand (const void *pBuffer, unsigned nLength);
    #ifdef PRH_MODS
    	boolean SendHCIData (const void *pBuffer, unsigned nLength);
        void registerPacketHandler(void *pHCILayer, TBTHCIEventHandler *pHandler);
    #else
        void RegisterHCIEventHandler (TBTHCIEventHandler *pHandler);
    #endif


private:
	boolean StartRequest (void);

	void CompletionRoutine (CUSBRequest *pURB);
	static void CompletionStub (CUSBRequest *pURB, void *pParam, void *pContext);

private:
	CUSBEndpoint *m_pEndpointInterrupt;
	CUSBEndpoint *m_pEndpointBulkIn;
	CUSBEndpoint *m_pEndpointBulkOut;

	CUSBRequest *m_pURB;
	u8 *m_pEventBuffer;
    
    #ifdef PRH_MODS
        u8 *m_pDataBuffer;
        CUSBRequest *m_pDataURB;
        
    	boolean StartDataRequest (void);
        void DataCompletionRoutine (CUSBRequest *pURB);
        static void DataCompletionStub (CUSBRequest *pURB, void *pParam, void *pContext);
        
        void *m_pHCILayer;
    #endif

    TBTHCIEventHandler *m_pEventHandler;

	static unsigned s_nDeviceNumber;
};

#endif
