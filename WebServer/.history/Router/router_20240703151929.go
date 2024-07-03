package Router

import (
	"fmt"
	"net"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
)

type Login struct {
   // binding:"required"修饰的字段，若接收为空值，则报错，是必须字段
   User    string `form:"formUsername" json:"user" uri:"user" xml:"user" binding:"required"`
   Pssword string `form:"formPassword" json:"password" uri:"password" xml:"password" binding:"required"`
}

var Pswd string
var TcpConn net.Conn
var isConnect bool
var LocalIp string

func RouterInit(r *gin.Engine) {
	r.LoadHTMLGlob("HTML/*")
	r.GET("/",func(c *gin.Context) {
		c.HTML(http.StatusOK,"index.html",gin.H{
			"title":"网页指令助手",
			"content":"内容",

		})
	})
	r.GET("ping",func(c *gin.Context) {
		c.JSON(200,gin.H{
			"message":"pong",
		})
	})
	r.GET("user/:name/*action",func(c *gin.Context) {
		name := c.Param("name")
		action := c.Param("action")
		action = strings.Trim(action,"/")
		c.String(http.StatusOK,name+"is"+action)
	})
	r.GET("greet",func(c *gin.Context) {
		name := c.DefaultQuery("name","Jack")
		c.String(http.StatusOK, fmt.Sprintf("hello %s",name))
	})
	r.POST("form",func(c *gin.Context) {
		types := c.DefaultPostForm("type","post")
		username := c.PostForm("username")
		password := c.PostForm("userpassword")
		c.String(http.StatusOK,fmt.Sprintf("username:%s ,password:%s ,type:%s",username,password,types))
	})
	r.MaxMultipartMemory = 8 << 20
	r.POST("upload",func(c *gin.Context) {
		file, err := c.FormFile("file")
		if err != nil {
			c.String(500,"Error File")
		}
		c.SaveUploadedFile(file,file.Filename)
		c.String(http.StatusOK,file.Filename)
	})
	v1 := r.Group("v1")
	{
		v1.GET("login",login)
		v1.GET("submit",submit)
	}
	r.POST("loginJSON",func (c *gin.Context){
		var json Login
		err := c.ShouldBindJSON(&json)
		if err != nil {
			c.JSON(http.StatusBadRequest,gin.H{
				"error":err.Error(),
			})
		} else if json.User != "root" || json.Pssword != "admin" {
			c.JSON(http.StatusBadRequest,gin.H{
				"status":"304",
			})
		} else {
			c.JSON(http.StatusOK,gin.H{
				"status":"200",
			})
		}
		
	})
	r.POST("loginForm",func (c *gin.Context){
		var form Login

		err := c.Bind(&form)
		if err != nil {
			c.String(http.StatusBadRequest,fmt.Sprintf("err:%s",err.Error()))
		} else if form.User != "root" || form.Pssword != "admin" {
			c.String(http.StatusBadRequest,fmt.Sprintf("status 304"))
		} else {
			c.String(http.StatusOK,"status ok.")
		}
	})
	r.GET("redirect/:website",func(c *gin.Context) {
		website := c.Param("website")
		if website[0:3] == "www" {
			website = "http://" + website
		} else {
			website = "/" + website
		}
		// c.Redirect(http.StatusMovedPermanently, website)
		fmt.Printf("%s",website)
		c.String(http.StatusOK,website)
	})
	r.Run("0.0.0.0:8080")
}

func login(c *gin.Context) {
	name := c.DefaultQuery("name", "Jack")
	c.String(200, fmt.Sprintf("Login: hello %s\n", name))
}

func submit(c *gin.Context) {
	name := c.DefaultQuery("name", "Lily")
	c.String(200, fmt.Sprintf("Submit: hello %s\n", name))
}

func TCPConnect(c *gin.Context) error {
	var err error
	TcpConn, err = net.Dial("tcp", "127.0.0.1:8080")
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
		_, err := fmt.Fprint(TcpConn, s)
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