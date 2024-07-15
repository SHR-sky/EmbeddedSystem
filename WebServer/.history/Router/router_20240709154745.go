package Router

import (
	"bufio"
	"fmt"
	"log"
	"net"
	"net/http"
	"strconv"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/shirou/gopsutil/host"
	"github.com/shirou/gopsutil/mem"
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
	r.POST("sendCommand", func(c *gin.Context) {
		command := c.PostForm("command")
		TCPSend(command)
		c.String(http.StatusOK,command)
	})
	r.POST("setTime", func(c *gin.Context) {
		err := TCPSend("TIME")
		if err != nil {
			c.String(http.StatusOK,"指令发送失败")
		} else {
			c.String(http.StatusOK,"成功发送指令")
		}
	})
	r.POST("weather", func(c *gin.Context) {
		err := TCPSend("WEATHER")
		if err != nil {
			c.String(http.StatusOK,"指令发送失败")
		} else {
			c.String(http.StatusOK,"成功发送指令")
		}
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
	TCPListener, err = net.Listen("tcp", ipAddr)
    if err != nil {
        log.Fatalf("Failed to listen: %v", err)
    }
	fmt.Printf("Listening on %s\n", TCPListener.Addr().String())
	go waitForConnect()
	return err
}

func waitForConnect() error {
	for {
        // 等待客户端连接
		var err error
        TcpConn, err = TCPListener.Accept()
        if err != nil && TcpConn != nil{
            log.Printf("Failed to accept connection: %v", err)
            isConnect = false;
			continue
        }
        fmt.Printf("Accepted connection from %s\n", TcpConn.RemoteAddr())
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

func GetSysInfo() {
	hostInfo, err := host.Info()
	vmStat, err := mem.VirtualMemory()
    if err != nil {
        fmt.Printf("获取主机信息失败: %v\n", err)
        return
    }
	var info = ""
	info = info + hostInfo.Hostname + "\n" + strconv.Itoa((int)(hostInfo.Uptime)) + "\n" + strconv.Itoa((int)(vmStat.Total)) + "\n"
	+ strconv.Itoa((int)(vmStat.Free)) + strconv.Itoa((int)(vmStat.UsedPercent*100))
	
	// 获取磁盘信息
	/*
	partitions, _ := disk.Partitions(false)
	var diskMapArr []map[string]interface{}
	for _, partition := range partitions {
		diskMap := make(map[string]interface{})
		usage, _ := disk.Usage(partition.Mountpoint)
		diskMap["disk"] = partition.Mountpoint     //第几块磁盘
		diskMap["total"] = usage.Total             //总大小
		diskMap["free"] = usage.Free               //剩余空间
		diskMap["used"] = usage.Used               //已使用空间
		diskMap["usedPercent"] = usage.UsedPercent //百分比
		diskMapArr = append(diskMapArr, diskMap)
		fmt.Printf("%v:,%v,%v,%v,%v\n",partition.Mountpoint,usage.Total,usage.Free,usage.Used,usage.UsedPercent)
	}
	*/
}