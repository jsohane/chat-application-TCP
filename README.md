# CSE 489/589: Text Chat Application

This repository contains the implementation of a text chat application developed as part of the CSE 489/589 Networks Lab assignment. The project involves creating a robust client-server model to facilitate real-time messaging between multiple clients over TCP connections.

## Objectives

The main objectives of this assignment were to:

1. **Develop Client and Server Components**: Implement a chat server and multiple chat clients that communicate over TCP connections.
2. **Implement Basic and Advanced Features**: The project was divided into two stages:
   - **Stage 1**: Focused on basic login functionality.
   - **Stage 2**: Implemented advanced features such as message broadcasting, client blocking, and statistics collection.
3. **Adherence to Standards**: Ensure the implementation adheres to the strict output and performance standards necessary for automated testing and grading.

## Features

- **TCP Socket Communication**: The application utilizes TCP sockets for reliable message exchange between clients and the server.
- **Shell Commands Integration**: Both the client and server components include a shell interface that accepts and processes specific commands in real-time.
- **Selective Handling of Connections**: Using the `select()` system call, the application efficiently handles multiple socket connections without multi-threading.
- **Client Login and Logout**: Clients can log in to the server, which maintains a list of active connections and handles message routing.
- **Message Broadcasting**: Clients can send broadcast messages to all other connected clients.
- **Blocking Mechanism**: Clients have the ability to block messages from specific IP addresses.
- **Statistics Tracking**: The server tracks and displays statistics about client activities, including messages sent and received.

## Installation and Compilation

To compile and run the application, follow these steps:

1. **Clone the Repository**:
    ```bash
    git clone https://github.com/yourusername/network-chat-application.git
    cd network-chat-application
    ```

2. **Compile the Code**:
    - Use the provided Makefile to compile the code.
    ```bash
    make
    ```

3. **Running the Server**:
    - Start the server on a specific port.
    ```bash
    ./assignment1 s <port_number>
    ```

4. **Running a Client**:
    - Start a client instance on a specific port.
    ```bash
    ./assignment1 c <port_number>
    ```

## Command List

### Common Commands (Client/Server)
- **AUTHOR**: Displays a statement verifying the student's adherence to the course's academic integrity policy.
- **IP**: Displays the IP address of the machine running the application.
- **PORT**: Displays the port number the application is listening on.
- **LIST**: Lists all currently logged-in clients (clients only).

### Server-Specific Commands
- **STATISTICS**: Displays a list of all clients that have ever logged in and statistics about each one.
- **BLOCKED <client-ip>**: Displays a list of clients blocked by the specified IP address.

### Client-Specific Commands
- **LOGIN <server-ip> <server-port>**: Logs in to the server.
- **REFRESH**: Refreshes the list of currently logged-in clients.
- **SEND <client-ip> <msg>**: Sends a message to a specific client.
- **BROADCAST <msg>**: Sends a broadcast message to all logged-in clients.
- **BLOCK <client-ip>**: Blocks messages from the specified IP address.
- **UNBLOCK <client-ip>**: Unblocks a previously blocked client.
- **LOGOUT**: Logs out from the server.
- **EXIT**: Logs out from the server and exits the application.

## Implementation Details

- **Sockets and Select System Call**: The application uses TCP sockets for communication and the `select()` system call for handling multiple client connections simultaneously.
- **Error Handling**: Comprehensive error handling ensures the application responds appropriately to invalid inputs and network errors.
- **Logging**: A custom logging function is used to record all interactions, which is crucial for debugging and automated grading.

## Requirements

- **C/C++ Compiler**: The application is developed in C and requires GCC for compilation.
- **Dedicated Hosts**: The application is tested on specific CSE hosts provided by the instructor.

## Conclusion

This project demonstrates a solid understanding of network programming principles and provides a functional and efficient text chat application that meets the assignment's rigorous requirements.
