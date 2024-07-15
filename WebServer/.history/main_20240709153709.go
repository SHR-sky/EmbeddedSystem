package main

import (
	"WebModule/Router"
	"fmt"

	"github.com/gin-gonic/gin"
	"github.com/shirou/gopsutil/disk"
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
	fmt.Printf("启动时间:%v ，运行时间: %v\n", hostInfo.Uptime,hostInfo.BootTime)
    fmt.Printf("操作系统: %v %v\n", hostInfo.OS, hostInfo.PlatformVersion)


	vmStat, err := mem.VirtualMemory()
	if err != nil {
		fmt.Printf("Failed to get memory info: %v", err)
		return
	}
	fmt.Printf("Total: %v, Free: %v, UsedPercent: %.2f%%\n", vmStat.Total, vmStat.Free, vmStat.UsedPercent)
	
	// 获取磁盘信息
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