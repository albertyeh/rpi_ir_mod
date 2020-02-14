本範例使用 GPIO 17 為 Input PIN ; GPIO 18 為 Output PIN , 複製 GPIO 17 訊號.

How to use the driver
===========

Step 1: 下載 Poky

* git clone -b sumo git://git.yoctoproject.org/poky


Step 2: 下載 Poky

* cd poky
* git clone -b sumo git://git.yoctoproject.org/meta-raspberrypi
* git clone -b sumo git://git.openembedded.org/meta-openembedded

Step 3: 下載 IR sample code
* git clone https://github.com/albertyeh/rpi_ir_mod.git
* mv rpi_ir_mod/ir-mod ../meta-skeleton

Step 4: 配置 Layer
* source oe-init-build-env
* bitbake-layers add-layer ../meta-mender/meta-mender-core
* bitbake-layers add-layer ../meta-openembedded/meta-oe
* bitbake-layers add-layer ../meta-raspberrypi
* bitbake-layers add-layer ../meta-skeleton

Step 5: 修改 build/conf/local.conf 
* 修改MACCHINE這行為 MACHINE ?= "raspberrypi3"

Step 6: Build image
* bitbake core-image-base

Step 7: 測試
* * lsmod   # 檢查 ir-mod.ko 是否被載入.
* 對著 IR 發射器 按下遙控器
* echo 0 > /sys/kernel/debug/ir/dump      #就可看到 波形資料
