#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

#include "base64.h"
#include "sha1.h"
#include "cvsupport.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "convertimage.h"
#include "serversettings.h"
 
using namespace std;
 
#pragma comment (lib, "Ws2_32.lib")

std::string collectivemsgs[MAX_CLIENTS] = {"","","","",""};

struct client_type
{
    SOCKET socket;
    int id;
    bool isImageReader;
};

cv::Mat str2Mat(std::string encoded){
    ImagemConverter conv = ImagemConverter();
    cv::Mat img = conv.str2mat(encoded);
    return img;
}

string mat2Str(const cv::Mat& image){
    ImagemConverter conv = ImagemConverter();
    std::string encoded = conv.mat2str(image);
    return encoded;
}

int process_client(client_type &new_client);
int main();
 
int process_client(client_type &new_client)
{
    char received_message[DEFAULT_BUFLEN];
    while (1)
    {
        memset(received_message, 0, DEFAULT_BUFLEN);
 
        if (new_client.socket != 0)
        {
            int iResult = recv(new_client.socket, received_message, DEFAULT_BUFLEN, 0);
 
            if (iResult != SOCKET_ERROR){
                std::string msg = received_message;
                collectivemsgs[new_client.id] += msg;
                if (!collectivemsgs[new_client.id].empty() && msg[msg.length()-1] == '\n') {
                    // remove new line character from end of string
                    collectivemsgs[new_client.id].erase(collectivemsgs[new_client.id].length()-1);
                    // check string is not empty
                    if (!collectivemsgs[new_client.id].empty()){
                        cout << "Message received of size: " << collectivemsgs[new_client.id].size() << endl;
                        if (new_client.isImageReader){
                            cv::Mat image = str2Mat(collectivemsgs[new_client.id]);
                            cv::Mat display_image;
                            if (image.type() == CV_32FC1){
                                CVSupport::disparity2colormap(image,display_image);
                            } else {
                                display_image = image;
                            }
                            cv::imshow("test", display_image);
                            cv::waitKey(1);
                        } else {
                            cout << "Full message: " << collectivemsgs[new_client.id] << endl;
                        }

                        collectivemsgs[new_client.id] = "";
                    } else {
                        cout << "Message is empty after removing newline" << endl;
                    }
                } else if (collectivemsgs[new_client.id].size() >= collectivemsgs[new_client.id].max_size()){
                    // reset string if exeedingly large
                    collectivemsgs[new_client.id] = "";
                } else {
                    //cout << "Message segment: " << msg << endl;
                }
            }
            else
            {
                cout << "recv() failed: " << WSAGetLastError() << endl;
                break;
            }
        }
    }
 
    if (WSAGetLastError() == WSAECONNRESET)
        cout << "The server has disconnected" << endl;
 
    return 0;
}
 
int main()
{
    bool isImageReader = true;
    WSAData wsa_data;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    string sent_message = "";
    client_type client = { INVALID_SOCKET, -1, isImageReader};
    int iResult = 0;
    string message;
 
    cout << "Starting Client...\n";
 
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (iResult != 0) {
        cout << "WSAStartup() failed with error: " << iResult << endl;
        return 1;
    }
 
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
 
    cout << "Connecting...\n";
 
    // Resolve the server address and port
    iResult = getaddrinfo(static_cast<LPCTSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo() failed with error: " << iResult << endl;
        WSACleanup();
        system("pause");
        return 1;
    }
 
    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
 
        // Create a SOCKET for connecting to server
        client.socket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (client.socket == INVALID_SOCKET) {
            cout << "socket() failed with error: " << WSAGetLastError() << endl;
            WSACleanup();
            system("pause");
            return 1;
        }
 
        // Connect to server.
        iResult = connect(client.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(client.socket);
            client.socket = INVALID_SOCKET;
            continue;
        }
        break;
    }
 
    freeaddrinfo(result);
 
    if (client.socket == INVALID_SOCKET) {
        cout << "Unable to connect to server!" << endl;
        WSACleanup();
        system("pause");
        return 1;
    }
   
 
    cout << "Successfully Connected" << endl;
 
    //Obtain id from server for this client;
    char received_message[DEFAULT_BUFLEN];
    recv(client.socket, received_message, DEFAULT_BUFLEN, 0);
    message = received_message;
 
    if (message != "Server is full")
    {
        client.id = atoi(received_message);
 
        thread my_thread(process_client, client);

        cv::Mat image;
 
        while (1)
        {
            getline(cin, sent_message);
            Sleep(100);

            iResult = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);
 
            if (iResult <= 0)
            {
                cout << "send() failed: " << WSAGetLastError() << endl;
                break;
            }
        }
 
        //Shutdown the connection since no more data will be sent
        my_thread.detach();
    }
    else
        cout << received_message << endl;
 
    cout << "Shutting down socket..." << endl;
    iResult = shutdown(client.socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        cout << "shutdown() failed with error: " << WSAGetLastError() << endl;
        closesocket(client.socket);
        WSACleanup();
        system("pause");
        return 1;
    }
 
    closesocket(client.socket);
    WSACleanup();

    system("pause");
    return 0;
}