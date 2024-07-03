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
        if err != nil {
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
		n, _ := conn.Read(buf[:])
		recvStr := string(buf[:n])
        fmt.Printf("Received message from %s: %s\n", conn.RemoteAddr(), recvStr)

        // 这里可以根据收到的消息进行逻辑处理

        // 回复消息给客户端
        // reply := "Message received: " + message + "\n"
        // _, err := conn.Write([]byte(reply))
        // if err != nil {
        //     log.Printf("Error writing to connection: %v", err)
        //     return
        // }
    }

    if err := scanner.Err(); err != nil {
        log.Printf("Scanner error: %v", err)
    }

    fmt.Printf("Closing connection from %s\n", conn.RemoteAddr())
}
