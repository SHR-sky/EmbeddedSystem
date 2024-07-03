package main

import (
	"WebModule/Router"
	"fmt"

	"github.com/gin-gonic/gin"
)

func main() {
	var temp string
	address,err := Router.GetLocalAddress()
	fmt.Printf("请输入网页登陆密码:\n")
	fmt.Scan(&Router.Pswd)
	if(err != nil) {
		fmt.Printf("获取本机ip地址错误！请手动查询")
	}
	fmt.Println("请登录 "+address+":8080 来访问操作网页")
	fmt.Printf("输入start启动网页\n")
	fmt.Scan(&temp)
	if(temp=="start") {
		r := gin.Default()
		Router.RouterInit(r)
		defer Router.TcpConn.Close()
	}
	fmt.Printf("Exit.")
}