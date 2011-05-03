#include <vector>
#include <iostream>
#include <winsock2.h>
#include "stdafx.h"

#include <opencv2/highgui/highgui.hpp>

#include "Sockette.h"
#include "SURFMatcher.h"
#include "util.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 1111
#define DEFAULT_BUFLEN 65536

Logger *g_logger = NULL;
SURFMatcher *g_matcher = NULL;

CvMemStorage *storage = cvCreateMemStorage(0);
CvSURFParams params = cvSURFParams(400, 1);  // TODO: make sure matches SURFMatcher

// TODO: change the fact that query images must be smaller than training image
const string defaultQueryImageName = "data/boston_closeup.jpg";
const string defaultLibraryFile = "data/library/index.xml";
const string defaultDirToSaveResImages = "data/results";

#define QUERY_WIDTH  400  // TODO: make configurable, and let client know configs
#define QUERY_HEIGHT 300

// Deal with client
DWORD WINAPI ClientLoop(LPVOID sockette) {
	Sockette * clientSocket = (Sockette *) sockette;
	g_logger->Log(INFO, "New client (%lu) using port %u doing work!\n",
					    clientSocket->address(), clientSocket->port());

	int c = 0;
	while (true) {
		char *dataReceived = NULL;
		string dataToSend;
		// TODO: get rid of memory leaks
		if (clientSocket->Listen(&dataReceived)) {
			// Converting jpg to iplimage... 
			CvMat cvmat = cvMat(QUERY_WIDTH, QUERY_HEIGHT, CV_8UC3, (void *) dataReceived);
			IplImage *frame = cvDecodeImage(&cvmat, 1);  // TODO: need to release?
			
			// actual image is in probably color (3 channels)
			// but we need to use 1 channel to do surfmatching which uses grayscale
			// images (1 channel)
			IplImage *queryImage = cvCreateImage(cvSize(QUERY_WIDTH, QUERY_HEIGHT), IPL_DEPTH_8U, 1);
			cvCvtColor(frame, queryImage, CV_BGR2GRAY);

			struct tm timeinfo = Util::GetTimeInfo();
			char queryName[128];
			strftime(queryName, 128, "%m-%d-%y-%H-%M-%S.jpg", &timeinfo);
			std::cout << "saving to file " << queryName << endl;
			cvSaveImage(queryName, queryImage);
			
			//clientSocket->Send(queryName);
			CvSeq *queryKeyPoints = 0, *queryDescriptors = 0;
			double tt = (double) -cvGetTickCount();
			cvExtractSURF(queryImage, 0, &queryKeyPoints, &queryDescriptors, storage, params);
			printf("Query Descriptors %d\nQuery Extraction Time = %gm\n",
				queryDescriptors->total, ((tt + cvGetTickCount()) / cvGetTickFrequency()*1000.));
			
			dataToSend = g_matcher->MatchAgainstLibrary(queryName, queryImage, queryKeyPoints, queryDescriptors);
			if (dataToSend.empty()) {
				std::cout << "NO MATCH!\n";
			} else {
				clientSocket->Send(dataToSend);
			}

			

			//cvShowImage("magic?", frame);
			//cvWaitKey();  // need for showimage
		} else {
			break;
		}
	}
	std::cout << "Client peaceing out\n";
	delete clientSocket;
	return 0;
}

// Wait for incoming connections
/*
DWORD WINAPI ConnectLoop(LPVOID sockette) {
	Sockette * s = (Sockette *) sockette;

	vector<HANDLE> threads = vector<HANDLE>();
	while (true) {
		SOCKET clientSock = SOCKET_ERROR;
		while (clientSock == SOCKET_ERROR) {
			std::cout << "Waiting for incoming connections...\r\n";
			clientSock = accept(s->handle(), NULL, NULL);
		}

		cout << "We have a request!\r\n\r\n" << endl;
		const char *msg = "hello there!\n";
		send(clientSock, msg, strlen(msg), 0);

		if (clientSock == INVALID_SOCKET) {
			cerr << "TODO: Invalid socket :(\r\n\r\n" << endl;
		} else {
			// branch off a thread to deal with the new connection
			Sockette * newSockette = new Sockette(clientSock);
			HANDLE clientThread = CreateThread(0, 0, ClientLoop, (LPVOID) newSockette, 0, 0);
			if (clientThread == NULL) {
				cerr << "Could not create thread for incoming connection!\n";
				ExitProcess(1);
			} else {
				threads.push_back(clientThread);
			}
		}
	}
	return 0;
}
*/

int _tmain() {
	
	g_logger = new Logger();

	// build library
	g_matcher = new SURFMatcher(g_logger);
	g_matcher->Build(defaultLibraryFile);

	WSADATA WsaDat;
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0) {
		std::cout << "WSA Initialization failed!\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}
	
	Sockette *serverSock = new Sockette((u_short) PORT);
	serverSock->StartListening();

	vector<HANDLE> threads = vector<HANDLE>();
	while (true) {
		SOCKET clientSock = SOCKET_ERROR;
		std::cout << "Waiting for incoming connections...\n";
		clientSock = accept(serverSock->handle(), NULL, NULL);
		if (clientSock == INVALID_SOCKET || clientSock == SOCKET_ERROR) {
			cerr << "Error: Accepted bad socket!\n" << endl;
			continue;
		}

		cout << "We have a request!\r\n\r\n" << endl;

		//u_long iMode = 1;  // set non-blocking mode
		//if (ioctlsocket(clientSock, FIONBIO, &iMode)) {
		//	std::cerr << "Error: " << WSAGetLastError();
		//}

		// branch off a thread to deal with the new connection
		Sockette * newSockette = new Sockette(clientSock);

		HANDLE clientThread = CreateThread(0, 0, ClientLoop, (LPVOID) newSockette, 0, 0);
		if (clientThread == NULL) {
			cerr << "Not accepting incoming connections...\n";
			// We couldn't spin off a thread to deal with this connection
			WSACleanup();
			system("PAUSE");
		} else {
			threads.push_back(clientThread);
		}
	}

	WSACleanup();
	system("PAUSE");
	return 0;
}
