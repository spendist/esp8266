# 使用 ESP8266制作一个投影仪网络控制器#
@(spender)[esp8266]
## 需求 ##
我前一段时间京东网上买了一个 viewsonic 的投影仪， 型号：PX727-4K/PX747-4K, 这是一个4K的家用投影，看电影效果还可以， 唯一遗憾的是不支持网络。 
阅读说明书以后，发现里面有串口控制的信息。于是突发奇想，用esp8266 实现远程控制，岂不是很好。
说干就干。硬件框架： esp8266 nodemcu 一块，串口调试器一块。arduino 开发环境一个。
其中 Nodemcu +5V，Gnd 管脚连接一个usb用于模块供电，未来直接插投影仪背面的usb口，软件实现附件串口， 自带的usb用于调试。
## 实现思路：##

- 1：使用 Wifimanager 库，通过配置SSID/Password连接家庭网络
- 2：连接成功后提供web服务，通过不同的url发送请求给8266。
- 3：根据请求，打开8266相应的数据文件（其实就是控制指令）
- 4：把读到的数据发送给虚拟串口至投影仪，实现自动化控制。
- 5: After
- 6: 使用家里的 domoticz IOT 软件集成， 实现基于互联网的物联网控制。

## 网页概述 ##
首页是一个 index.html， 放在esp8266 文件系统中， 可以通过 Arundio 上传， 其他控制器，根据说明说制作成一个一个的小文件， 后缀.dat，内容就是所有控制字符。
网页提供一个一个简易表格， 把这些控制命令index出来， 方便手机、电脑等客户端通过浏览器控制投影仪
### 网页代码部分 ###

``` html
<HTML>
<HEAD>
<meta http-equiv="content-type" content="text/html;charset=utf-8">
<TITLE> ViewSonic Control</TITLE>
<style type="text/css">
body{
	background:#FFFFFF;
	color:#000000
}
.imgBox{
width:100%;
text-align: center;
}
</style>
</HEAD>
<BODY>
<span class="STYLE1"></span>
<table width="380" border="0" cellspacing="3" cellpadding="2">
  <tr>
    <td width="100%"><h2>ViewSonic PX727-4K/PX747-4K <br />
      Simple Control Ststen<br />
    </h2>
      <div class="imgBox"> <a href="https://www.viewsonic.com.cn/"><img src="logo.jpg" alt="logo" longdesc="https://www.viewsonic.com.cn/" /></a> </div>
      <table border="0" cellpadding="2" cellspacing="3" id="1">
        <tr>
          <td>配置</td>
          <td colspan="4"><a href="/reset">复位</a></td>
        </tr>
        <tr>
          <td>电源</td>
          <td><a href="/pwn_on">On</a></td>
          <td><a href="/pwn_off">Off</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>消息提示</td>
          <td><a href="/msg_on">On</a></td>
          <td><a href="/msg_off">Off</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>投影模式</td>
          <td><a href="/view_1">正投</a></td>
          <td><a href="/view_2">背投</td>
          <td><a href="/view_3">吊装正投</td>
          <td><a href="/view_4">吊装背投</td>
        </tr>
        <tr>
          <td>图像模式</td>
          <td>明亮</td>
          <td>电影</td>
          <td>标准</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>信号源</td>
          <td><a href="/source_comp">电脑</a></td>
          <td><a href="/source_hdmi1">HDMI1</a></td>
          <td><a href="/source_hdmi2">HDMI2</a></td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>自动扫描</td>
          <td><a href="/autoscan_on">On</a></td>
          <td><a href="/autoscan_off">Off</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>静音</td>
          <td><a href="/mute_on">On</a></td>
          <td><a href="/mute_on">Off</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>音量</td>
          <td><a href="/volume_up">加大+</a></td>
          <td><a href="/volume_down">减小-</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>语言</td>
          <td><a href="/lang_eng">英文</a></td>
          <td><a href="/lang_cn">中文</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>CEC</td>
          <td><a href="/cec_on">On</a></td>
          <td><a href="/cec_off">Off</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
      </table></td>
  </tr>
</table>
<h3>Copyright by <a href="mailto:spender@myhomedomain.net">Spender</a></h3>

</BODY>
```
支持curl命令行控制，结合 domoticz 的脚本功能，实现常用的控制。（不属于本文档范围，略）

## 程序框架 ##

使用比较成熟的demo程序修改。 1：自动化配置wifi连接(setup()) , 如果连接成功，继续。 2:检查可用的控制数据文件（实质上为了简单， 我仅仅做了一个list）
3：创建web服务， 注册相关网页， 接收用户点击或者curl请求， 按命令打开相关控制数据文件， 发送文件内容至虚拟串口，从而实现投影仪控制。



废话不多说了， 直接上源码： 有兴趣的朋友可以交流一下：


	```
	# 使用 ESP8266制作一个投影仪网络控制器#
@(spender)[esp8266]
## 需求 ##
我前一段时间京东网上买了一个 viewsonic 的投影仪， 型号：PX727-4K/PX747-4K, 这是一个4K的家用投影，看电影效果还可以， 唯一遗憾的是不支持网络。 
阅读说明书以后，发现里面有串口控制的信息。于是突发奇想，用esp8266 实现远程控制，岂不是很好。
说干就干。硬件框架： esp8266 nodemcu 一块，串口调试器一块。arduino 开发环境一个。
其中 Nodemcu +5V，Gnd 管脚连接一个usb用于模块供电，未来直接插投影仪背面的usb口，软件实现附件串口， 自带的usb用于调试。
## 实现思路：##

- 1：使用 Wifimanager 库，通过配置SSID/Password连接家庭网络
- 2：连接成功后提供web服务，通过不同的url发送请求给8266。
- 3：根据请求，打开8266相应的数据文件（其实就是控制指令）
- 4：把读到的数据发送给虚拟串口至投影仪，实现自动化控制。
- 5: After
- 6: 使用家里的 domoticz IOT 软件集成， 实现基于互联网的物联网控制。

## 网页概述 ##
首页是一个 index.html， 放在esp8266 文件系统中， 可以通过 Arundio 上传， 其他控制器，根据说明说制作成一个一个的小文件， 后缀.dat，内容就是所有控制字符。
网页提供一个一个简易表格， 把这些控制命令index出来， 方便手机、电脑等客户端通过浏览器控制投影仪
### 网页代码部分 ###

···
<HTML>
<HEAD>
<meta http-equiv="content-type" content="text/html;charset=utf-8">
<TITLE> ViewSonic Control</TITLE>
<style type="text/css">
body{
	background:#FFFFFF;
	color:#000000
}
.imgBox{
width:100%;
text-align: center;
}
</style>
</HEAD>
<BODY>
<span class="STYLE1"></span>
<table width="380" border="0" cellspacing="3" cellpadding="2">
  <tr>
    <td width="100%"><h2>ViewSonic PX727-4K/PX747-4K <br />
      Simple Control Ststen<br />
    </h2>
      <div class="imgBox"> <a href="https://www.viewsonic.com.cn/"><img src="logo.jpg" alt="logo" longdesc="https://www.viewsonic.com.cn/" /></a> </div>
      <table border="0" cellpadding="2" cellspacing="3" id="1">
        <tr>
          <td>配置</td>
          <td colspan="4"><a href="/reset">复位</a></td>
        </tr>
        <tr>
          <td>电源</td>
          <td><a href="/pwn_on">On</a></td>
          <td><a href="/pwn_off">Off</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>消息提示</td>
          <td><a href="/msg_on">On</a></td>
          <td><a href="/msg_off">Off</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>投影模式</td>
          <td><a href="/view_1">正投</a></td>
          <td><a href="/view_2">背投</td>
          <td><a href="/view_3">吊装正投</td>
          <td><a href="/view_4">吊装背投</td>
        </tr>
        <tr>
          <td>图像模式</td>
          <td>明亮</td>
          <td>电影</td>
          <td>标准</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>信号源</td>
          <td><a href="/source_comp">电脑</a></td>
          <td><a href="/source_hdmi1">HDMI1</a></td>
          <td><a href="/source_hdmi2">HDMI2</a></td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>自动扫描</td>
          <td><a href="/autoscan_on">On</a></td>
          <td><a href="/autoscan_off">Off</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>静音</td>
          <td><a href="/mute_on">On</a></td>
          <td><a href="/mute_on">Off</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>音量</td>
          <td><a href="/volume_up">加大+</a></td>
          <td><a href="/volume_down">减小-</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>语言</td>
          <td><a href="/lang_eng">英文</a></td>
          <td><a href="/lang_cn">中文</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
        <tr>
          <td>CEC</td>
          <td><a href="/cec_on">On</a></td>
          <td><a href="/cec_off">Off</a></td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
        </tr>
      </table></td>
  </tr>
</table>
<h3>Copyright by <a href="mailto:spender@myhomedomain.net">Spender</a></h3>

</BODY>
···
支持curl命令行控制，结合 domoticz 的脚本功能，实现常用的控制。（不属于本文档范围，略）

## 程序框架 ##

使用比较成熟的demo程序修改。 1：自动化配置wifi连接(setup()) , 如果连接成功，继续。 2:检查可用的控制数据文件（实质上为了简单， 我仅仅做了一个list）
3：创建web服务， 注册相关网页， 接收用户点击或者curl请求， 按命令打开相关控制数据文件， 发送文件内容至虚拟串口，从而实现投影仪控制。

## 欢迎交流 ##
