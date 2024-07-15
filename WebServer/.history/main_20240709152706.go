package main

import (
	"WebModule/Router"
	"fmt"

	"github.com/gin-gonic/gin"
	"github.com/shirou/gopsutil/host"
	"github.com/shirou/gopsutil/mem"
)

func main() {
    hostInfo, err := host.Info()
    if err != nil {
        fmt.Printf("获取主机信息失败: %v\n", err)
        return
    }

    fmt.Printf("主机名: %v\n", hostInfo.Hostname)
    fmt.Printf("操作系统: %v %v\n", hostInfo.OS, hostInfo.PlatformVersion)

    // 获取CPU信息
    cpuInfo, err := host.SensorsTemperatures()
    if err != nil {
        fmt.Printf("获取CPU温度失败: %v\n", err)
    } else {
        for _, temp := range cpuInfo {
            fmt.Printf("传感器类型: %v, 温度: %.2f 度\n", temp.SensorKey, temp.Temperature)
        }
    }

	vmStat, err := mem.VirtualMemory()
	if err != nil {
		fmt.Printf("Failed to get memory info: %v", err)
		return
	}
	fmt.Printf("Total: %v, Free: %v, UsedPercent: %.2f%%\n", vmStat.Total, vmStat.Free, vmStat.UsedPercent)
	
	var temp string
	address,err := Router.GetLocalAddress()
	fmt.Printf("请输入网页登陆密码:\n")
	fmt.Scan(&Router.Pswd)
	if(err != nil) {
		fmt.Printf("获取本机ip地址错误！请手动查询")
	}
	err = Router.StartServer(fmt.Sprint(address+":9999")) // ip:9999监听地址
	defer Router.TCPListener.Close()
	fmt.Println("请登录 "+address+":8080 来访问操作网页")
	//err = Router.TCPConnect()
	if(err != nil) {
		fmt.Print(err.Error()+"\n")

	}
	fmt.Printf("输入start启动网页\n")
	fmt.Scan(&temp)
	if(temp=="start") {
		r := gin.Default()
		Router.RouterInit(r)
		defer Router.TcpConn.Close()
	}
	fmt.Printf("Exit.")
}