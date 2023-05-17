WIFI模块学习记录：
1. 使用的WIFI模块为：RTL8188EUS，将模块驱动的代码放入内核的代码中，并修改内核的Makefile和Kconfig文件；
2. 因为使用的模块是USB接口，所有使用make menuconfig进入配置内核的编译，配置内核支持USB和WIFI设备，并支持IEEE802.11的标准；
3. 之后编译内核模块并加载驱动；
4. 配置WIFI工具：wireless tools移植，用于操作WIFI的工具
                wpa_supplicant移植，依赖openssl，libnl库

编译 wpa_supplicant之后，就可以在/var/wpa_supplicant.conf下面配置用户名和密码了
接下来就可以使用wpa_supplicant来进行WiFi的连接了！
