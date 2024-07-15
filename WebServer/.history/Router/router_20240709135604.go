package Router

import (
	"bufio"
	"fmt"
	"log"
	"net"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
)

var Pswd string
var TcpConn net.Conn
var isConnect bool
var LocalIp string
var isLogin bool
var ESP32IP string
var ListenAddr string
var TCPListener net.Listener

func RouterInit(r *gin.Engine) {
	r.LoadHTMLGlob("HTML/*")
	// 首页
	r.GET("/",func(c *gin.Context) {
		c.HTML(http.StatusOK,"login.html",gin.H{
			"title":"网页指令助手",
			"content":"内容",
			"address":"http://"+LocalIp+":8080",
		})
	})
	// ping测试
	r.GET("ping",func(c *gin.Context) {
		c.JSON(200,gin.H{
			"message":"pong",
		})
	})

	r.GET("greet",func(c *gin.Context) {
		name := c.DefaultQuery("name","1037")
		c.String(http.StatusOK, fmt.Sprintf("hello %s",name))
	})

	// 登陆
	r.POST("login",func(c *gin.Context) {
		password := c.PostForm("userpassword")
		{
			if(password == Pswd) {
				isLogin = true
				c.Redirect(http.StatusMovedPermanently,"http://"+LocalIp+":8080/main")
			} else {
				c.String(http.StatusOK,fmt.Sprint("密码错误，请返回重试"))
			}
		}
	})
	r.GET("exit",func(c *gin.Context) {
		isLogin = false
		c.String(http.StatusOK,"退出成功！")
	})

	r.GET("main",func(c *gin.Context) {
		if(isLogin) {
			c.HTML(http.StatusOK,"index.html",gin.H{
				"title":"网页指令助手",
				"content":"内容",
				"address":"http://"+LocalIp+":8080",
			})
		} else {
			c.String(http.StatusOK,"请先登陆")
		}
	})
	r.POST("sleep", func(c *gin.Context) {
		err := TCPSend("sleep")
		if err != nil {
			c.String(http.StatusOK,"指令发送失败")
		} else {
			c.String(http.StatusOK,"成功发送指令")
		}
		
	})
	r.POST("music", func(c *gin.Context) {
		err := TCPSend("music")
		if err != nil {
			c.String(http.StatusOK,"指令发送失败")
		} else {
			c.String(http.StatusOK,"成功发送指令")
		}
	})
	r.GET("reConnect",func(c *gin.Context) {
		var err error
		TcpConn, err = net.Dial("tcp", ESP32IP)
    	if err != nil {
			c.String(http.StatusOK,err.Error())
			isConnect = false
    	} else {
			c.String(http.StatusOK,"重连成功")
			isConnect = true
		}
	})
	r.POST("sendCommand", func(c *gin.Context) {
		command := c.PostForm("command")
		TCPSend(command)
		c.String(http.StatusOK,command)
	})

	r.Run("0.0.0.0:8080")
}

func TCPConnect() error {
	var err error
	TcpConn, err = net.Dial("tcp", ESP32IP)
    if err != nil {
        fmt.Println("Error connecting:", err)
		isConnect = false
        return err
    }
	isConnect = true
	return nil
} 

func TCPSend(s string) error {
	if(isConnect) {
		_, err := TcpConn.Write([]byte(s))
		if err != nil {
			return fmt.Errorf("error sending message")
		}
		return nil
	} else {
		return fmt.Errorf("connection error! please try again later");
	}
}

func GetLocalAddress() (string,error) {
	conn, err := net.Dial("udp", "8.8.8.8:80")
    if err != nil {
        fmt.Println("Error:", err)
        return "",err
    }
    defer conn.Close()

    localAddr := conn.LocalAddr().(*net.UDPAddr)
	LocalIp = string(strings.Split(localAddr.String(),":")[0])
	return LocalIp,nil
}

func StartServer(ipAddr string) error {
	ListenAddr = ipAddr
	var err error
	TCPListener, err = net.Listen("tcp", ListenAddr)
    if err != nil {
        log.Fatalf("Failed to listen: %v", err)
    }
    defer TCPListener.Close()
	go waitForConnect()
	return err
}

func waitForConnect() error {
	for {
        // 等待客户端连接
		var err error
        TcpConn, err = TCPListener.Accept()
        if err != nil {
            log.Printf("Failed to accept connection: %v", err)
            isConnect = false;
			continue
        }
        fmt.Printf("Accepted connection from %s\n", conn.RemoteAddr())
		isConnect = true;
        // 启动一个协程处理连接
        go handleConnection(TcpConn)
    }
}

var buf [256]byte

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