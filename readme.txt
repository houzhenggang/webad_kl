����
netfilter

ע��
����ں˲�֧��netfilter
	ִ��make memuconfig������netfilterѡ��
	Networking-->Networking Options-->Network Packet Filtering Framework-->
	Core Netfilter Configuration(����Netfilter����)��IP��Netfilter Configuration ��IP��Netfilter���ã�

��ֲ
1����������õ�kernel-3.10/drivers/webad
2�����obj-$(CONFIG_WEBAD) += webad.o��
	webad-y := main.o capture.o mnetlink.o mstring.o http_session.o plug.o plug_extern.o ��
	kernel-3.10/drivers/webad/Makefile
3�����obj-$(CONFIG_WEBAD)		+= webad/ �� ../kernel-3.10/drivers/Makefile
4�����source "drivers/webad/Kconfig" �� ../kernel-3.10/drivers/Kconfig

���밲װ
1����android��Ŀ¼ִ��make����
2��������������ļ�����ˢ��