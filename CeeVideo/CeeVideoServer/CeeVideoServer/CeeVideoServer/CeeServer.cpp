#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#include <opencv2\opencv.hpp>
#include "opencv2/opencv.hpp"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
using namespace cv;

// USING ENVIRONMENT VARIABLE "OPENCV_INC" POINTING TOWARDS THE C:\Users\Anthony\Downloads\opencv\build\include -- C++ GENERAL ADDITIONAL INCLUDE DIRECTORIES
// USING ENVIRONMENT VARIABLE "OPENCV_LIB" POINTING TOWARDS THE C:\Users\Anthony\Downloads\opencv\build\x64\vc14\lib -- LINKER GENERAL ADDITIONAL LIBRARY DIRECTORIES

// ADDL DEPENDENCIES:  opencv_world320.lib;opencv_world320d.lib

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 307200
#define DEFAULT_PORT "8080"

void play_video(Mat frame){
	imshow("Server Frame", frame);
	cvWaitKey(10);
}


String type2str(int type) {
	String r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}
int __cdecl main(void)
{
	printf("Server starting\n");
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}


	// Receive until the peer shuts down the connection
	Mat myMat;
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		uchar* dataMat = (uchar*)recvbuf;
		cv::Mat outputMat;
		
		cv::Mat my_mat(480,640, CV_8UC1, dataMat);
		

		//my_mat.convertTo(my_mat, CV_8UC1);
		String ty = type2str(my_mat.type());
		printf("Matrix: %s %dx%d \n", ty.c_str(), my_mat.cols, my_mat.rows);
		//cv::Mat my_mat(480 * 640, 1, CV_32FC1, dataMat);
		play_video(my_mat);
		
		if (iResult > 0) {
			//printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			/*
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
			*/
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else  {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	// No longer need server socket
	closesocket(ListenSocket);
	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}
