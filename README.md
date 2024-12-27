# BundleFusion

## 一、环境配置<br>
>操作系统：win11 x64<br>
>集成平台：Visual Studio 2013<br>
>计算架构：cuda10.2<br>
>开发工具：DXSDK_Jun10<br>
>深度相机：Intel RealSense D435<br>
>显卡：NVIDIA GeForce RTX3050(官方采用NVIDIA GeForce 950MX，只要计算能力达到5.0即可)<br>
## 二、项目配置<br>
>mLib external文件夹在本项目的main分支，下载后解压并放在BundleFusion-master同级目录
>最终的代码目录结构如下
>>bundlefusion
	BundleFusion-master/
	  external/
	    mLib/ # this is the submodule you replaced
	      data/
	      src/
	      [...]
	  FriedLiver/
	    [...]
	    FriedLiver.sln
	    [...]
	mLibExternal/ # you downloaded this from Dropbox
	  include
	  libsWindows
	  [...]
