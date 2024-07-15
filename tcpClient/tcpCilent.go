package main

import (
	"bufio"
	"fmt"
	"log"
	"net"
)

func main() {
    // 监听地址和端口
    var address string
    fmt.Printf("输入监听网站:\n")
    fmt.Scan(&address)
    //address := "192.168.66.48:9999"

    // 创建一个 TCP 服务器监听器
    listener, err := net.Listen("tcp", address)
    if err != nil {
        log.Fatalf("Failed to listen: %v", err)
    }
    defer listener.Close()
    fmt.Printf("Listening on %s\n", address)

    // 循环接收连接
    for {
        // 等待客户端连接
        conn, err := listener.Accept()
        if err != nil && conn != nil {
            log.Printf("Failed to accept connection: %v", err)
            continue
        }
        fmt.Printf("Accepted connection from %s\n", conn.RemoteAddr())

        // 启动一个协程处理连接
        go handleConnection(conn)
    }
}

var buf [256]byte

// 处理连接函数
func handleConnection(conn net.Conn) {
    defer conn.Close()

    // 读取客户端发送的消息
    scanner := bufio.NewScanner(conn)
    for {
        // 读取一行消息
		n, err := conn.Read(buf[:])
		recvStr := string(buf[:n])
        if(recvStr!="") {
            fmt.Printf("Received message from %s: %s\n", conn.RemoteAddr(), recvStr)
        }
        if err != nil {
            if err == net.ErrClosed {
                fmt.Print("Client Close Connection.\n")
                break;
            } else {
                break;
            }
        }
    }

    if err := scanner.Err(); err != nil {
        log.Printf("Scanner error: %v", err)
    }

    fmt.Printf("Closing connection from %s\n", conn.RemoteAddr())
}
