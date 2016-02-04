依赖
netfilter

注意
如果内核不支持netfilter
	执行make memuconfig，开启netfilter选项
	Networking-->Networking Options-->Network Packet Filtering Framework-->
	Core Netfilter Configuration(核心Netfilter配置)和IP：Netfilter Configuration （IP：Netfilter配置）

移植
1、将代码放置到kernel-3.10/drivers/webad
2、添加obj-$(CONFIG_WEBAD) += webad.o和
	webad-y := main.o capture.o mnetlink.o mstring.o http_session.o plug.o 
	plug_extern.o nf_nat_helper.o 到
	kernel-3.10/drivers/webad/Makefile
3、添加obj-$(CONFIG_WEBAD)		+= webad/ 到 ../kernel-3.10/drivers/Makefile
4、添加source "drivers/webad/Kconfig" 到 ../kernel-3.10/drivers/Kconfig

编译安装
1、在android根目录执行make命令
2、将编译出来的文件进行刷机