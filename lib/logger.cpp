//
// logger.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
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
#include <circle/logger.h>
#include <circle/string.h>
#include <circle/synchronize.h>
#include <circle/startup.h>
#include <circle/multicore.h>
#include <circle/util.h>
#include <circle/sysconfig.h>
#include <circle/machineinfo.h>
#include <circle/version.h>
#include <circle/debug.h>

#define LOGGER_BUFSIZE	0x4000

// PRH comments
//
// The ONLY thing the buffering is used for is lib/net/syslogdaemon
// which is the only method in circle that calls ReadEvent().
// CSysLogDaemon is part of the net library, and is ONLY called
// in circle/sample/33-syslog kernel.cpp.






struct TLogEvent
{
	TLogSeverity	Severity;
	char		Source[LOG_MAX_SOURCE];
	char		Message[LOG_MAX_MESSAGE];
	time_t		Time;
	unsigned	nHundredthTime;
	int		nTimeZone;			// minutes diff to UTC
};

CLogger *CLogger::s_pThis = 0;

CLogger::CLogger (unsigned nLogLevel, CTimer *pTimer)
:	m_nLogLevel (nLogLevel),
	m_pTimer (pTimer),
	m_pTarget (0),
	m_pBuffer (0),
	m_nInPtr (0),
	m_nOutPtr (0),
	m_nEventInPtr (0),
	m_nEventOutPtr (0),
	m_pEventNotificationHandler (0),
	m_pPanicHandler (0)
{
	m_pBuffer = new char[LOGGER_BUFSIZE];
#ifdef PRH_MODS
	m_default_color_code = 97;
#endif
	s_pThis = this;
}

CLogger::~CLogger (void)
{
	s_pThis = 0;

	while (m_nEventInPtr != m_nEventOutPtr)
	{
		delete m_pEventQueue[m_nEventOutPtr];

		if (++m_nEventOutPtr == LOG_QUEUE_SIZE)
		{
			m_nEventOutPtr = 0;
		}
	}

	delete [] m_pBuffer;
	m_pBuffer = 0;

	m_pTarget = 0;
	m_pTimer = 0;
}

boolean CLogger::Initialize (CDevice *pTarget)
{
	m_pTarget = pTarget;

	Write ("logger", LogNotice, CIRCLE_NAME " " CIRCLE_VERSION_STRING " started on %s"
#if AARCH == 64
	       " (AArch64)"
#endif
	       , CMachineInfo::Get ()->GetMachineName ());

	return TRUE;
}

void CLogger::Write (const char *pSource, TLogSeverity Severity, const char *pMessage, ...)
{
	va_list var;
	va_start (var, pMessage);

	WriteV (pSource, Severity, pMessage, var);

	va_end (var);
}

void CLogger::WriteV (const char *pSource, TLogSeverity Severity, const char *pMessage, va_list Args)
{
	CString Message;
	Message.FormatV (pMessage, Args);

	WriteEvent (pSource, Severity, Message);

	if (Severity > m_nLogLevel)
	{
		return;
	}

	CString Buffer;

	if (Severity == LogPanic)
	{
		Buffer = "\x1b[1m";
	}
#ifdef PRH_MODS
	else if (Severity == LogError)
	{
		Buffer = "\x1b[31m";
	}
	else if (Severity == LogWarning)
	{
		Buffer = "\x1b[93m";
	}
	else if (m_default_color_code)
	{
		Buffer.Format("\x1b[%dm",m_default_color_code);
	}
#endif

	if (m_pTimer != 0)
	{
		CString *pTimeString = m_pTimer->GetTimeString ();
		if (pTimeString != 0)
		{
			Buffer.Append (*pTimeString);
			Buffer.Append (" ");

			delete pTimeString;
		}
	}

	Buffer.Append (pSource);
	Buffer.Append (": ");

	Buffer.Append (Message);

	if (Severity == LogPanic)
	{
		Buffer.Append ("\x1b[0m");
	}

	Buffer.Append ("\n");

	Write (Buffer);

	if (Severity == LogPanic)
	{
		if (m_pPanicHandler != 0)
		{
			(*m_pPanicHandler) ();
		}

#ifndef PRH_MODS
	// When using a CSerialDevice with Logger, halting the
	// processor here stops the serial output before you
	// get a chance to see the assert that is causing the
	// panic. Without these calls, the CPU(s) continue(s) to
	// run, which might cause other catastrophic problems,
	// I suppose, but at least I can see the assert that
	// caused the panic, and often a few more asserts (or
	// even the best case that the program continues to work)
	
	#ifndef USE_RPI_STUB_AT
	#ifndef ARM_ALLOW_MULTI_CORE
			halt ();
	#else
			CMultiCoreSupport::HaltAll ();
	#endif
	#else
			Breakpoint (0);
	#endif
	
#endif	// !PRH_MODS

	}
}

void CLogger::WriteNoAlloc (const char *pSource, TLogSeverity Severity, const char *pMessage)
{
	if (Severity > m_nLogLevel)
	{
		return;
	}

	char Buffer[200];
	Buffer[0] = '\0';

	// TODO: assert (4+strlen (pSource)+2+strlen (pMessage)+4+1 < sizeof Buffer);

	if (Severity == LogPanic)
	{
		strcpy (Buffer, "\x1b[1m");
	}

	strcat (Buffer, pSource);
	strcat (Buffer, ": ");
	strcat (Buffer, pMessage);

	if (Severity == LogPanic)
	{
		strcat (Buffer, "\x1b[0m");
	}

	strcat (Buffer, "\n");

	Write (Buffer);

	if (Severity == LogPanic)
	{
#ifndef USE_RPI_STUB_AT
#ifndef ARM_ALLOW_MULTI_CORE
		halt ();
#else
		CMultiCoreSupport::HaltAll ();
#endif
#else
		Breakpoint (0);
#endif
	}
}

CLogger *CLogger::Get (void)
{
	return s_pThis;
}

void CLogger::Write (const char *pString)
{
	unsigned long nLength = strlen (pString);

#ifdef PRH_MODS
	// prh - don't access the device if the logger
	// has not been initialized yet.  Needed if you
	// are going to use a Serial device for logging,
	// as other objects are initialized, and call this
	// method, before the Serial device can be initialized.
	if (m_pTarget)
#endif
		m_pTarget->Write (pString, nLength);

	m_SpinLock.Acquire ();

	while (nLength--)
	{
		char c = *pString++;
		if (c == '\r')
		{
			continue;
		}

		m_pBuffer[m_nInPtr] = c;

		m_nInPtr = (m_nInPtr + 1) % LOGGER_BUFSIZE;

		if (m_nInPtr == m_nOutPtr)
		{
			m_nInPtr = (m_nInPtr - 1) % LOGGER_BUFSIZE;

			break;
		}
	}

	m_SpinLock.Release ();
}

int CLogger::Read (void *pBuffer, unsigned nCount)
{
	m_SpinLock.Acquire ();

	if (m_nInPtr == m_nOutPtr)
	{
		m_SpinLock.Release ();

		return -1;
	}

	char *pchBuffer = (char *) pBuffer;
	int nResult = 0;

	while (nCount--)
	{
		*pchBuffer++ = m_pBuffer[m_nOutPtr];

		m_nOutPtr = (m_nOutPtr + 1) % LOGGER_BUFSIZE;

		nResult++;

		if (m_nInPtr == m_nOutPtr)
		{
			break;
		}
	}

	m_SpinLock.Release ();

	return nResult;
}

void CLogger::WriteEvent (const char *pSource, TLogSeverity Severity, const char *pMessage)
{
	TLogEvent *pEvent = new TLogEvent;
	if (pEvent == 0)
	{
		return;
	}

	pEvent->Severity = Severity;

	strncpy (pEvent->Source, pSource, LOG_MAX_SOURCE);
	pEvent->Source[LOG_MAX_SOURCE-1] = '\0';

	strncpy (pEvent->Message, pMessage, LOG_MAX_MESSAGE);
	pEvent->Message[LOG_MAX_MESSAGE-1] = '\0';

	unsigned nSeconds, nMicroSeconds;
	if (   m_pTimer != 0
	    && m_pTimer->GetLocalTime (&nSeconds, &nMicroSeconds))
	{
		pEvent->Time = nSeconds;
		pEvent->nHundredthTime = nMicroSeconds / 10000;
		pEvent->nTimeZone = m_pTimer->GetTimeZone ();
	}
	else
	{
		pEvent->Time = 0;
		pEvent->nHundredthTime = 0;
		pEvent->nTimeZone = 0;
	}

	m_EventSpinLock.Acquire ();

	m_pEventQueue[m_nEventInPtr] = pEvent;

	if (++m_nEventInPtr == LOG_QUEUE_SIZE)
	{
		m_nEventInPtr = 0;
	}

	// drop oldest entry, if event queue is full
	TLogEvent *pDropEvent = 0;
	if (m_nEventInPtr == m_nEventOutPtr)
	{
		pDropEvent = m_pEventQueue[m_nEventOutPtr];

		if (++m_nEventOutPtr == LOG_QUEUE_SIZE)
		{
			m_nEventOutPtr = 0;
		}
	}

	m_EventSpinLock.Release ();

	if (pDropEvent != 0)
	{
		delete pDropEvent;
	}

	if (m_pEventNotificationHandler != 0)
	{
		(*m_pEventNotificationHandler) ();
	}
}

boolean CLogger::ReadEvent (TLogSeverity *pSeverity, char *pSource, char *pMessage,
			    time_t *pTime, unsigned *pHundredthTime, int *pTimeZone)
{
	m_EventSpinLock.Acquire ();

	if (m_nEventInPtr == m_nEventOutPtr)
	{
		m_EventSpinLock.Release ();

		return FALSE;
	}

	TLogEvent *pEvent = m_pEventQueue[m_nEventOutPtr];

	if (++m_nEventOutPtr == LOG_QUEUE_SIZE)
	{
		m_nEventOutPtr = 0;
	}

	m_EventSpinLock.Release ();

	*pSeverity = pEvent->Severity;
	strcpy (pSource, pEvent->Source);
	strcpy (pMessage, pEvent->Message);
	*pTime = pEvent->Time;
	*pHundredthTime = pEvent->nHundredthTime;
	*pTimeZone = pEvent->nTimeZone;

	delete pEvent;

	return TRUE;
}

void CLogger::RegisterEventNotificationHandler (TLogEventNotificationHandler *pHandler)
{
	m_pEventNotificationHandler = pHandler;
}

void CLogger::RegisterPanicHandler (TLogPanicHandler *pHandler)
{
	m_pPanicHandler = pHandler;
}
