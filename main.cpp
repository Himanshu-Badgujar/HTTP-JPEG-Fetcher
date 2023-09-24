#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

// Define macros and types for simplification
#define SOCKADDR_IN struct sockaddr_in
#define SOCKADDR struct sockaddr
#define HOSTENT struct hostent
#define SOCKET int

// Function to print error messages and exit
void PrintErrorExit(const char *msg, int ret_code) {
    fprintf(stderr, "%s, Error: %s\n", msg, strerror(ret_code));
    exit(ret_code);
}

int main() {
    // Variable Declarations
    char *site, *host, *file_name;
    char url[256], send_buf[256], recv_buf[256];
    long rc;
    SOCKET s;
    SOCKADDR_IN addr{};
    HOSTENT *hent;

    // Initialize sockaddr_in structure
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    // Input HTTP URL
    printf("URL: ");
    scanf("%s", url);

    // Parse URL to separate host and site
    if (strncmp("http://", url, 7) == 0)
        host = url + 7;
    else
        host = url;

    if ((site = strchr(host, '/')) != 0)
        *site++ = '\0';
    else
        site = host + strlen(host);

    printf("Host: %s\nSite: %s\nConnecting....\n", host, site);

    // Resolve hostname to IP address
    if ((addr.sin_addr.s_addr = inet_addr((const char *) host)) == INADDR_NONE) {
        if (!(hent = gethostbyname(host)))
            PrintErrorExit("Cannot resolve Host", h_errno);
        strncpy((char *) &addr.sin_addr.s_addr, hent->h_addr, 4);
        if (addr.sin_addr.s_addr == INADDR_NONE)
            PrintErrorExit("Cannot resolve Host", h_errno);
    }

    // Create a socket
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        PrintErrorExit("Cannot create Socket", errno);

    // Connect to the remote host
    if (connect(s, (SOCKADDR *) &addr, sizeof(SOCKADDR)))
        PrintErrorExit("Cannot connect", errno);

    printf("Connected to %s\n", host);

    // Send an HTTP GET request to the server
    sprintf(send_buf, "GET /%s HTTP/1.0\r\n\r\n", site);
    if ((send(s, send_buf, strlen(send_buf), 0)) < strlen(send_buf))
        PrintErrorExit("Cannot send Data", errno);

    // Receive data from the server and write it to a file
    while ((rc = recv(s, recv_buf, 1, 0)) > 0) {
        // File handling to store data in hexadecimal format
        FILE *http_raw = fopen("/home/himbad/CLionProjects/HTTPJPEGFetcher/http_raw.txt", "ab");
        if (http_raw == NULL) {
            PrintErrorExit("Error in opening a file", 1);
        } else {
            recv_buf[rc] = '\0';
            fprintf(http_raw, "%02hhX", *recv_buf);
            if (fclose(http_raw) != 0) {
                PrintErrorExit("Error in closing file", 2);
            }
        }
    }

    // Open a file to read data, find a line separator, and manipulate the file data
    FILE *http_raw = fopen("/home/himbad/CLionProjects/HTTPJPEGFetcher/http_raw.txt", "r");
    if (http_raw == NULL) {
        PrintErrorExit("Error in opening a file", 3);
    } else {
        char buffer[2000000];
        if (fgets(buffer, sizeof(buffer), http_raw) == NULL) {
            PrintErrorExit("Error in reading a file", 4);
        } else {
            char *result = strstr(buffer, "D0A0D0A");
            if (result == NULL) {
                PrintErrorExit("Failed to detect a line separator", 5);
            } else {
                if ((file_name = strrchr(site, '/')) != 0)
                    *file_name++ = '\0';
                else
                    file_name = site + strlen(site);

                FILE *file_data_raw = fopen("/home/himbad/CLionProjects/HTTPJPEGFetcher/file_data_raw.txt", "wb");
                if (file_data_raw == NULL) {
                    PrintErrorExit("Failed to open a file", 6);
                } else {
                    fprintf(file_data_raw, "%s", result + 7);

                    if (fclose(file_data_raw) != 0) {
                        PrintErrorExit("Error in closing file", 7);
                    }
                    if (fclose(http_raw) != 0) {
                        PrintErrorExit("Error in closing file", 8);
                    }

                    char command[512];
                    snprintf(command, sizeof(command), "cd /home/himbad/CLionProjects/HTTPJPEGFetcher/ && xxd -r -p file_data_raw.txt %s", file_name);
                    system(command);

                    if (remove("/home/himbad/CLionProjects/HTTPJPEGFetcher/http_raw.txt") != 0) {
                        PrintErrorExit("Error in deleting file", 9);
                    }
                    if (remove("/home/himbad/CLionProjects/HTTPJPEGFetcher/file_data_raw.txt") != 0) {
                        PrintErrorExit("Error in deleting file", 10);
                    }

                    printf("Image downloaded successfully!");
                }
            }
        }
    }

    // Close the socket connection
    close(s);
    return 0;
}
