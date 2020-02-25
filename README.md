1. 本範例使用 GPIO 17 為 Input PIN ; GPIO 18 為 Output PIN , 複製 GPIO 17 訊號.
2. 可用 echo 0 > /sys/kernel/debug/ir/dump , 顯示擷取的 IR raw data.

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
* 將 MACHINE?= 這行改為 MACHINE ?= "raspberrypi3" 或你要用的 model
* 另外添加此2行,如下:
* #行1表示將 driver 編譯到系統內;行2表示 開機自動載入 ko driver
* MACHINE_ESSENTIAL_EXTRA_RRECOMMENDS += "ir-mod"       
* KERNEL_MODULE_AUTOLOAD += "ir-mod"   

Step 6: Build image
* bitbake core-image-base

Step 7: Burn iamge
* 在 poky/build/tmp/deploy/image/raspberrypi3 下面尋找 core-image-base-raspberrypi3-20200214040007.rootfs.rpi-sdimg
* 將這個 .rpi-sdimg 燒到SD card  (.rpi-sdimg檔名每次編譯會有一些差異, 請自行參考實際檔名)

Step 8: 測試
* 開機
* #檢查 ir-mod.ko 是否被載入.
* lsmod      
* 對著 IR 接收器,按下遙控器
* #查看波形資料
* echo 0 > /sys/kernel/debug/ir/dump      
