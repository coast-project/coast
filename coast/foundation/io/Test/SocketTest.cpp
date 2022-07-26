/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SocketTest.h"

#include "PoolAllocator.h"
#include "Socket.h"
#include "TestSuite.h"
#if !defined(WIN32)
#include <fcntl.h>
#include <netinet/in.h>	 // for INADDR_ANY
#endif

void SocketTest::simpleConstructorTest() {
	Connector connector(GetConfig()["SocketConnectSuccessHost"]["ip"].AsString(),
						GetConfig()["SocketConnectSuccessHost"]["port"].AsLong());
	Socket *socket = connector.MakeSocket();

	if (t_assertm(socket != NULL, (const char *)connector.GetAddress())) {
		long socketfd = socket->GetFd();
		t_assert(socketfd > 0);

		std::iostream *Ios = socket->GetStream();
		t_assert(Ios != NULL);
	}
	delete socket;
}

void SocketTest::allocatorConstructorTest() {
	PoolAllocator pa(1, 8 * 1024, 21);
	TestStorageHooks tsh(&pa);

	Connector connector(GetConfig()["SocketConnectSuccessHost"]["ip"].AsString(),
						GetConfig()["SocketConnectSuccessHost"]["port"].AsLong());
	connector.SetThreadLocal();
	Socket *socket = connector.MakeSocket();

	if (t_assertm(socket != NULL, (const char *)connector.GetAddress())) {
		long socketfd = socket->GetFd();
		t_assert(socketfd > 0);

		std::iostream *Ios = socket->GetStream();
		t_assert(Ios != NULL);
	}
	delete socket;
}

void SocketTest::faultyConstructorTest() {
	Socket socket1(-1);
	long socketfd = socket1.GetFd();

	assertEqual(-1, socketfd);

	std::iostream *Ios = socket1.GetStream();
	assertEqual((long)NULL, (long)Ios);

	assertEqual(false, socket1.IsReadyForWriting());

	assertEqual(false, socket1.IsReadyForReading());

	assertEqual(false, socket1.ShutDown());
	assertEqual(false, socket1.ShutDownReading());
	assertEqual(false, socket1.ShutDownWriting());
	assertEqual(false, socket1.SetNoDelay());
}

void SocketTest::httpClientTest() {
	Connector connector(GetConfig()["HTTPReplyHost"]["ip"].AsString(), GetConfig()["HTTPReplyHost"]["port"].AsLong());
	Socket *socket = connector.MakeSocket();

	assertEqual(GetConfig()["HTTPReplyHost"]["ip"].AsString(), connector.GetAddress());
	assertEqual(GetConfig()["HTTPReplyHost"]["port"].AsLong(), connector.fPort);
	assertEqual((long)NULL, (long)connector.fSocket);

	if (t_assertm(socket != NULL, (const char *)connector.GetAddress())) {	// fails without HTTP Server
		long socketfd = socket->GetFd();
		t_assert(socketfd > 0L);

		// this one sets the connect timeout
		socket->SetTimeout(GetConfig()["GetStreamTimeout"].AsLong(5000L));
		std::iostream *Ios = socket->GetStream();
		t_assert(Ios != NULL);
		if (Ios != 0) {
			String query("GET / HTTP/1.0");
			String reply;
			long lRetCode = 0;
			if (t_assertm(socket->IsReadyForWriting(GetConfig()["ReadyForWritingTimeout"].AsLong(5000L), lRetCode),
						  TString("expected no timeout for sending http request to [")
							  << connector.GetAddress() << ':' << connector.fPort << "]") &&
				t_assert(lRetCode > 0)) {
				(*Ios) << query << std::endl << std::endl;
				t_assert(!!(*Ios));
				// make sure Ios is valid
				if (t_assertm(socket->IsReadyForReading(GetConfig()["ReadyForReadingTimeout"].AsLong(5000L), lRetCode),
							  TString("expected no timeout for reading HTTP reply [")
								  << connector.GetAddress() << ':' << connector.fPort << "]") &&
					t_assert(lRetCode > 0)) {
					(*Ios) >> reply;
					assertEqual("HTTP", reply.SubString(0, 4));
					// test first line of reply by http server
					t_assert(!!(*Ios));
				}
			}
		}
		delete socket;
		socket = 0;
		assertEqual(-1L, ::closeSocket(socketfd));
		// did we really close the socket??
	}
}

void SocketTest::faultyClientTest() {
	Connector connector(GetConfig()["SocketNotAcceptingHost"]["ip"].AsString(),
						GetConfig()["SocketNotAcceptingHost"]["port"].AsLong(),
						GetConfig()["SocketNotAcceptingHost"]["timeout"].AsLong(5000L));
	Socket *socket = connector.MakeSocket();

	assertEqual(GetConfig()["SocketNotAcceptingHost"]["ip"].AsString(), connector.GetAddress());
	assertEqual(GetConfig()["SocketNotAcceptingHost"]["port"].AsLong(), connector.fPort);

	t_assert(socket == NULL);
	delete socket;
}

void SocketTest::MakeInetAddrTest() {
	ROAnything roTestConfig = GetConfig()["MakeInetAddrTest"]["IpAddr"];
	for (long i = 0; i < roTestConfig.GetSize(); i++) {
		assertEqualm(static_cast<uint32_t>(roTestConfig[i].AsLong()), ntohl(EndPoint::MakeInetAddr(roTestConfig.SlotName(i))),
					 TString("Failed at Addr [") << roTestConfig.SlotName(i) << "]");
	}
	unsigned long localInetAddr(GetConfig()["MakeInetAddrTest"]["LocalHost"].AsLong(0L));
	assertEqualm(localInetAddr, ntohl(EndPoint::MakeInetAddr("")), "Expected localhosts addr");
	assertEqualm(INADDR_ANY, EndPoint::MakeInetAddr("", true), "Expected any ip addr");
}

void SocketTest::AppendToClientInfoTest() {
	Anything clnInfo;
	clnInfo["REMOTE_ADDR"] = "127.0.0.1";
	clnInfo["REMOTE_PORT"] = 1234L;
	Socket sock(-1, clnInfo);
	sock.AppendToClientInfo("ServerNonce", Anything("Rnd8Byts"));
	ROAnything ci(sock.ClientInfo());
	assertEqual("Rnd8Byts", ci["ServerNonce"].AsCharPtr("oops"));
}

bool SocketTest::IsNonBlocking(int fd) {
#if defined(WIN32)
	return true;
#else
	int flags = 0;
	if (fd >= 0 && (flags = fcntl(fd, F_GETFL, 0)) >= 0) {
		return 0 != (flags & O_NONBLOCK);
	}
	return false;
#endif
}

void SocketTest::SetToNonBlockingTest() {
#if defined(WIN32)
	// set to non blocking only possible for sockets and because there is no way to query blocking/non-blocking
	// mode we would have to create a socket and call recv to check whether it blocks or not. Unfortunately we
	// would have to set a timeout which is only possible before binding which is not easy possible with the
	// current implementation of our socket classes.
#else
	// invalid fd
	t_assert(!Socket::SetToNonBlocking(-1));
	t_assert(!IsNonBlocking(-1));
	t_assert(!IsNonBlocking(0));
	t_assert(Socket::SetToNonBlocking(0));
	t_assert(IsNonBlocking(0));
	t_assert(Socket::SetToNonBlocking(0, false));
	t_assert(!IsNonBlocking(0));
#endif
}

Test *SocketTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SocketTest, simpleConstructorTest);
	ADD_CASE(testSuite, SocketTest, allocatorConstructorTest);
	ADD_CASE(testSuite, SocketTest, faultyConstructorTest);
	ADD_CASE(testSuite, SocketTest, httpClientTest);
	ADD_CASE(testSuite, SocketTest, faultyClientTest);
	ADD_CASE(testSuite, SocketTest, MakeInetAddrTest);
	ADD_CASE(testSuite, SocketTest, AppendToClientInfoTest);
	ADD_CASE(testSuite, SocketTest, SetToNonBlockingTest);

	return testSuite;
}
