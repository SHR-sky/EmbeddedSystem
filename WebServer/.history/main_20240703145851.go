package main

import (
	"WebModule/Router"
	"fmt"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	address,err := Router.GetLocalAddress()
	fmt.Printf("请输入网页登陆密码:\n")
	fmt.Scan(&Router.Pswd)
	if(err != nil) {
		fmt.Printf("获取本机ip地址错误！请手动查询")
	}
	fmt.Printf("请登录 %s:8080 来访问操作网页\n",address)
	
	Router.RouterInit(r)
	defer Router.TcpConn.Close()
}