# Terminal Chatting Application

This is a terminal-based chatting application built in C using the `socket.h` library. It allows multiple clients to connect to a single server for real-time messaging.

---

## Features
- Simple and lightweight terminal-based chat interface.
- Multi-client support through a single server.
- Built using C and the `socket.h` library for efficient network communication.

---

## Getting Started

### Prerequisites
Ensure you have the following installed:
- GCC compiler
- `make` utility
- Linux operating system

### Building the Program

1. **Clean Previous Builds**
   Run the following command to remove any existing compiled files:
   ```bash
   make clean
   ```

2. **Compile the Program**
   Build the server and client executables with:
   ```bash
   make compile
   ```

---

## Usage

### Starting the Server
One user needs to start the server. Use the following command to launch it:
```bash
./obj/tcp_server
```

The server will display its IP address and port. Share these details with anyone who wants to connect.

### Connecting to the Server
On a new terminal, clients can connect to the server using:
```bash
./obj/tcp_client <ip_address> <port>
```
Replace `<ip_address>` and `<port>` with the details provided by the server.

Once connected, you can start chatting with your friends in real time!

---

## Example Workflow

1. **Server Terminal**:
   ```bash
   ./obj/tcp_server
   ```
   Output:
   ```
   Server is running on IP: 192.168.1.10, Port: 8080
   ```

2. **Client Terminal**:
   ```bash
   ./obj/tcp_client 192.168.1.10 8080
   ```

3. **Chat Away!**
   Begin sending messages back and forth between the connected terminals.

---

## License

This project is open-source and available under the [MIT License](LICENSE).

Feel free to use, modify, and distribute the application as per the terms of the license.

---

## Contributions
Contributions are welcome! If you find a bug or have a feature request, feel free to open an issue or submit a pull request.

---

Happy chatting!

