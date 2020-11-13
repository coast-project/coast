/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SocketStream.h"

#include "SystemBase.h"
#include "SystemLog.h"
#include "Tracer.h"

using namespace coast;

//#define STREAM_TRACE

#if !defined(WIN32)
#include <errno.h>
#include <sys/socket.h>	 // used for send and recv
#endif

iosITOSocket::iosITOSocket(Socket *s, long timeout, long sockbufsz, int mode)
	: fSocketBuf(s, timeout, sockbufsz, mode), fAllocator(s->GetAllocator()) {
	init(&fSocketBuf);
}

SocketStream::SocketStream(Socket *s, long timeout, long sockbufsz)
	: iosITOSocket(s, timeout, sockbufsz, std::ios::in | std::ios::out), std::iostream(&fSocketBuf) {}

void SocketStreamBuf::SetTimeout(long t) {
	if (fSocket != 0) {
		fSocket->SetTimeout(t);
	}
}

long SocketStreamBuf::GetTimeout() {
	return fSocket != 0 ? fSocket->GetTimeout() : 0L;
}

TimeoutModifier::TimeoutModifier(SocketStream *Ios, long timeout) : fOriginalTimeout(0), fStream(Ios) {
	if (fStream != 0) {
		fOriginalTimeout = fStream->rdbuf()->GetTimeout();
		fStream->rdbuf()->SetTimeout(timeout);
	}
}

TimeoutModifier::~TimeoutModifier() {
	if (fStream != 0) {
		fStream->rdbuf()->SetTimeout(fOriginalTimeout);
	}
}

SocketStreamBuf::SocketStreamBuf(Socket *psocket, long timeout, long sockbufsz, int mode)
	: fReadBufStorage((mode & std::ios::in) != 0 ? sockbufsz : 0L),
	  fWriteBufStorage((mode & std::ios::out) != 0 ? sockbufsz : 0L),
	  fSocket(psocket),
	  fReadCount(0),
	  fWriteCount(0) {
	SetTimeout(timeout);
	xinit();
}

SocketStreamBuf::SocketStreamBuf(const SocketStreamBuf &ssbuf)
	: fReadBufStorage(ssbuf.fReadBufStorage.Capacity()),
	  fWriteBufStorage(ssbuf.fWriteBufStorage.Capacity()),
	  fSocket(ssbuf.fSocket),
	  fReadCount(ssbuf.fReadCount),
	  fWriteCount(ssbuf.fWriteCount) {
	int mode = 0;
	if (fReadBufStorage.Capacity() > 0) {
		mode |= std::ios::in;
	}
	if (fWriteBufStorage.Capacity() > 0) {
		mode |= std::ios::out;
	}
	xinit();
}

void SocketStreamBuf::xinit() {
	// setb(start(), end(), 0); // holding area
	setg(0, 0, 0);
	setp(0, 0);
}

SocketStreamBuf::~SocketStreamBuf() {
	SocketStreamBuf::sync();  // clear the buffer
	setg(0, 0, 0);
	setp(0, 0);
}

int SocketStreamBuf::overflow(int c) {
	if (pptr() == 0) {
		setp(startw(), endw());	 // reinitialize put area
	} else {
		Assert(pptr() - pbase() <= fWriteBufStorage.Capacity());

		if (sync() == EOF) {  // write full buffer
			return EOF;
		}

		setp(startw(), endw());	 // reinitialize put area
	}

	if (c != EOF && (pptr() < epptr())) {  // guard against recursion
		sputc(c);
	}
	return 0L;	// return 0 if successful
}  // overflow

int SocketStreamBuf::underflow() {
	long count = 0;

	if (gptr() < egptr()) {	 //(in_avail())
		// data is still available
		return (int)(unsigned char)*gptr();
	}

	if (pptr() - pbase() > 0) {
		if (sync() == EOF) {
			return EOF;
		}
	}

	if ((count = DoRead(startr(), fReadBufStorage.Capacity())) <= 0) {
		return EOF;	 // might mean eofbit or failbit or badbit
	}
	AddReadCount(count);

	setg(startr(), startr(), startr() + count);
	return (int)(unsigned char)*gptr();
}  // underflow

int SocketStreamBuf::sync() {
	long count = 0;

	if (((count = pptr() - pbase()) > 0) && (DoWrite(pbase(), count) == EOF)) {
		return (EOF);
	}
	if (pptr() != 0) {
		setp(startw(), endw());	 // reinitialize put area
	}
	return 0;  // no error
}  // sync

SocketStreamBuf::pos_type SocketStreamBuf::seekpos(SocketStreamBuf::pos_type p, SocketStreamBuf::openmode mode) {
	sync();
	return pos_type(0);
}  // seekpos

SocketStreamBuf::pos_type SocketStreamBuf::seekoff(SocketStreamBuf::off_type of, SocketStreamBuf::seekdir dir,
												   SocketStreamBuf::openmode mode) {
	sync();
	return pos_type(0);
}  // seekoff

void SocketStreamBuf::AddReadCount(long bytes) {
	StartTrace(SocketStreamBuf.AddReadCount);
	fReadCount += bytes;
}

void SocketStreamBuf::AddWriteCount(long bytes) {
	StartTrace(SocketStreamBuf.AddWriteCount);
	fWriteCount += bytes;
}

long SocketStreamBuf::DoWrite(const char *buf, long len) {
	long bytesSent = 0;
	std::iostream *Ios = fSocket != 0 ? fSocket->GetStream() : 0;  // neeed for errorhandling

	while (len > bytesSent && (Ios != 0) && Ios->good()) {
		long nout = 0;
		if (fSocket->IsReadyForWriting()) {
			do {
				nout = send(fSocket->GetFd(), (char *)buf + bytesSent, len - bytesSent, 0);
			} while (nout < 0 && system::SyscallWasInterrupted());
			if (nout > 0) {
				bytesSent += nout;
				continue;
			}
		} else if (fSocket->HadTimeout()) {
			Ios->clear(std::ios::failbit);
			break;
		}
		String logMsg("socket on send: ");
		logMsg << fSocket->GetFd() << " failed - socket error number " << static_cast<long>(SOCKET_ERROR) << " <"
			   << SystemLog::LastSysError() << ">"
			   << " transmitted: " << bytesSent;

		SystemLog::Error(logMsg);
		Ios->clear(std::ios::badbit);
	}

	if (bytesSent > 0) {
		AddWriteCount(bytesSent);
#if defined(STREAM_TRACE)
		SystemLog::WriteToStderr(buf, bytesSent);
#endif
	}

	return ((Ios != 0) && Ios->good()) ? bytesSent : EOF;
}

long SocketStreamBuf::DoRead(char *buf, long len) const {
	long bytesRead = EOF;  // assume EOF is negative

	if (fSocket != 0) {
		std::iostream *Ios = fSocket->GetStream();
		if (fSocket->IsReadyForReading()) {
			do {
				bytesRead = recv(fSocket->GetFd(), buf, len, 0);
			} while (bytesRead < 0 && system::SyscallWasInterrupted());

			if (bytesRead < 0) {
				String msg("Socket Error: <");
				msg << static_cast<long>(errno) << ">=" << SystemLog::SysErrorMsg(errno);
				SystemLog::Error(msg);
				Ios->clear(std::ios::badbit);
			} else if (bytesRead == 0) {
#if defined(STREAM_TRACE)
				String msg("Socket:    end of data (read)              on file descriptor: ");
				msg << fSocket->GetFd();
				SystemLog::Info(msg);
#endif
				// socket is closed, stream recognizes this via
				// streambuf::underflow() returning eof, if no more bytes are available
			}
		} else {
			Ios->clear(fSocket->HadTimeout() ? std::ios::failbit : std::ios::badbit);
		}
#ifdef STREAM_TRACE
		if (bytesRead > 0) {
			SystemLog::WriteToStderr(buf, bytesRead);
		}
#endif
	}
	return bytesRead;
}

std::ostream &operator<<(std::ostream &os, SocketStreamBuf *ssbuf) {
	StartTrace(ostream.socketstreambuf);
	if (ssbuf != 0) {
		const int bufSz = 4096;
		char buf[bufSz];
		long totBytesRead = 0;

		long bytesRead = 0;
		do {
			bytesRead = ssbuf->DoRead(buf, bufSz);
			if (bytesRead > 0) {
				totBytesRead += bytesRead;
				SubTrace(content, buf);
				os.write(buf, bytesRead);
			} else if (bytesRead < 0) {
				String logMsg("Socket error on recv: ");
				logMsg << static_cast<long>(errno);
				SystemLog::Error(logMsg);
			}  // if
			else {
				SystemLog::Error("Socket closed on recv: ");
			}
		} while (bytesRead > 0);

		if (totBytesRead > 0) {
			ssbuf->AddReadCount(totBytesRead);
		}
		os.flush();
	}
	return os;
}
