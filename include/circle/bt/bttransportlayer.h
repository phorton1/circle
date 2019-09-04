//
// bttransportlayer.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2016  R. Stange <rsta2@o2online.de>
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
#ifndef _circle_bt_bttransportlayer_h
#define _circle_bt_bttransportlayer_h

#include <circle/types.h>		// to get PRH_MODS

enum TBTTransportType
	// prh - this enum retained for compatability with existing Circle code
	// but it does NOT define the different types of transports that are
	// available, or the one that is being used, in my system.  Only != 
	// BTTransportTypeUART is used in the circle "deviceManager" to branch
	// through vendor states (loading the firmware) or not.
{
	BTTransportTypeUSB,
	BTTransportTypeUART,
	BTTransportTypeUnknown
};


#ifdef PRH_MODS

	#include <circle/device.h>

	#define HCI_PREFIX_CMD      0x01
	#define HCI_PREFIX_DATA     0x02
	#define HCI_PREFIX_SDATA    0x03
	#define HCI_PREFIX_EVENT    0x04

	typedef void TBTHCIEventHandler(void *pHCILayer, u8 type, const void *pBuffer, unsigned nLength);
	
	// I'm about to do away with registering a single event handler, and instead,
	// having the transport derive from a transportBase class, and registering
	// that object with the HCI layer, so that it can call the event handlers
	// orthogonally and not care what kind of transport it is ... using
	// multiple inheritance.  The problem is that as I clean up my code, I have
	// to continually back-modify the Circle code for API compatability, and
	// I'm not regression testing my changes in it.  The base layer still needs
	// to know if it's a uart to the rPi controller to load the vendor stuff,
	// but that could be handled by a virtual method on the object itself.
	
	class btTransportBase 
	{
		public:
			virtual boolean isUartTransport() { return false; }
			virtual void registerPacketHandler(void *pHCILayer, TBTHCIEventHandler *pHandler) = 0;
			virtual boolean SendHCIData(const void *pBuffer, unsigned length) = 0;
			virtual boolean SendHCICommand(const void *pBuffer, unsigned length) = 0;
	};
	
	
#else
	typedef void TBTHCIEventHandler (const void *pBuffer, unsigned nLength);
#endif


#endif
