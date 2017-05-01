#define WIN32_LEAN_AND_MEAN
//#include <opencv2\opencv.hpp>
#include <windows.h>
#include <winsock2.h>
#include <opencv2\opencv.hpp>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

using namespace cv;

// SET COMMAND LINE RUN OPTIONS WITH DEBUG->PROPERTIES->Under debugging -> command parameters

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 307200
#define DEFAULT_PORT "8080"
Mat captureVideo(int, SOCKET, char*);
void sendData(int, SOCKET, char*, Mat);

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

void captureVideo(){
	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		printf("Failed");

	Mat edges;
	namedWindow("THIS IS A TEST", 1);
	for (;;)
	{
		Mat frame;
		cap >> frame; // get a new frame from camera
		unsigned char* dataMat = frame.data;
		printf("%d", frame.rows);
		//cvtColor(frame, edges, COLOR_BGR2GRAY);
		GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		Canny(edges, edges, 0, 30, 3);
		imshow("THIS IS A TEST", edges);

		cvWaitKey(10);
	}
}

Mat captureVideo(int iResult, SOCKET ConnectSocket, char *sendbuf){
	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		printf("Failed");

	Mat kluge;
	char * dataMat;
	Mat frame;
	//namedWindow("edges", 1);
	for (;;)
	{
		
		cap >> frame; // get a new frame from camera
		cvtColor(frame, kluge, CV_BGR2BGRA);
		dataMat = (char *)frame.data;
		printf("Frame: COL: %d, ROW: %d", frame.cols, frame.rows);
		String ty = type2str(frame.type());
		///printf("Matrix: %s %dx%d \n", ty.c_str(), frame.rows, frame.cols);
		cv::Mat kluge(480,640, CV_8UC4, (char *)frame.data);
		
		String tya = type2str(kluge.type());
		printf("kuge: %s %dx%d \n", tya.c_str(), kluge.cols, kluge.rows);
		//dataMat = (unsigned char *)kluge.data;
		//frame = (frame.reshape(0, 1));*/
		int imgSize = frame.total()*frame.elemSize();
		
		
		/*GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		Canny(edges, edges, 0, 30, 3);
		*/
		//char *sendbuf = const_cast<char *> (dataMat.toString().c_str());
		iResult = send(ConnectSocket, (char *)kluge.data, DEFAULT_BUFLEN, 0);

		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
		}
		cvWaitKey(10);
		//sendData(iResult, ConnectSocket, frame.data, edges);
	}
}
void sendData(int iResult, SOCKET ConnectSocket, char *sendbuf, Mat frame){
	
	//printf(sendbuf);
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}
	//printf("Bytes Sent: %ld\n", iResult);
}
int __cdecl main(int argc, char **argv)
{
	printf("Hey it's a client\n");
	//captureVideo();
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char *sendbuf = "this is a test there is a poop";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	captureVideo(iResult, ConnectSocket, sendbuf);
	//captureVideo();
	/*
	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	
	printf("Bytes Sent: %ld\n", iResult);
	*/
	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// Receive until the peer closes the connection
	do {

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}
