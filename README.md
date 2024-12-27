# BundleFusion

## 一、环境配置<br>
>操作系统：win11 x64<br>
>集成平台：Visual Studio 2013<br>
>计算架构：cuda10.2<br>
>开发工具：DXSDK_Jun10<br>
>深度相机：Intel RealSense D435<br>
>显卡：NVIDIA GeForce RTX3050(官方采用NVIDIA GeForce 950MX，只要计算能力达到5.0即可)<br>
## 二、项目配置<br>
1、mLib external文件夹在本项目的main分支，下载后解压并放在BundleFusion-master同级目录<br>
2、修改cuda版本,如果cuda不是10.2版本，可以打开BundleFusion-master\FriedLiver\FriedLiver.vcxproj 文件，把CUDA 10.2.targets部分修改为当前CUDA版本<br>
！[image]([img/修改cuda1.png](https://github.com/yrz1817/BundleFusion/blob/master/img/%E4%BF%AE%E6%94%B9cuda1.png))
