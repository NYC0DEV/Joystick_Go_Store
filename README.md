<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">

<style>
body{
  margin:0;
  font-family: monospace;
  background:#0a0a0f;
  color:#00ffcc;
}

#topbar{
  padding:10px 20px;
  background:#05050a;
  border-bottom:1px solid #00ffcc33;
}

.section{
  padding:20px;
  border-bottom:1px solid #00ffcc22;
}

.title{
  font-size:18px;
  margin-bottom:10px;
  color:#00ffcc;
}

pre{
  background:#00000055;
  padding:12px;
  border:1px solid #00ffcc33;
  overflow:auto;
  white-space:pre-wrap;
}

ul{
  line-height:1.6;
}

a{
  color:#00ffcc;
}

.box{
  padding:15px;
  border:1px solid #00ffcc33;
  margin-bottom:15px;
}
</style>
</head>

<body>

<!-- ================= STORE ================= -->

<div class="section">

<div class="title" style="font-size100px; font-weight:bold;">
Joystick Go Store
</div>

<div class="box">
<h3>Releases</h3>
<p>
<a href="https://github.com/NYC0DEV/Joystick_Go_Store/releases">
Open Releases Page
</a>
</p>
</div>

<div class="box">
<p>
<b>Joystick Go Store</b> is an open-source community store for GameStation Go.  
It allows users to browse and download game and app binaries.
</p>
</div>

<div class="box">
<p>
Download <b>Joystick_Go_Store.zip</b>, extract into:
</p>

<pre>
sd:\bin\Joystick_Go_Store
</pre>

</div>

<div class="box">
<h3>Controls</h3>

<table border="1" cellpadding="6" style="color:#00ffcc;border-color:#00ffcc33;">
<tr>
<th>Button</th>
<th>Action</th>
</tr>
<tr>
<td>D-Pad</td>
<td>Navigate</td>
</tr>
<tr>
<td>A</td>
<td>Select</td>
</tr>
<tr>
<td>B</td>
<td>Back / Quit</td>
</tr>
<tr>
<td>START</td>
<td>Launch application</td>
</tr>
</table>

</div>

</div>

<!-- ================= SYSTEM REQUIREMENTS ================= -->

<div class="section">

<div class="title">System Requirements</div>

<ul>
<li>ARM32 Toolchain (arm-linux-gnueabihf)</li>
<li>SDL2 development libraries</li>
<li>FFmpeg (libavformat, libavcodec, libavutil, libswresample, libswscale)</li>
<li>libcurl</li>
<li>nlohmann-json</li>
<li>build tools (gcc, make, cmake optional)</li>
</ul>

</div>

<!-- ================= INSTALL ================= -->

<div class="section">

<div class="title">Install Dependencies (Ubuntu / Debian)</div>

<pre>
sudo apt update
sudo apt install libsdl2-dev libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev libcurl4-openssl-dev nlohmann-json3-dev build-essential
</pre>

</div>

<!-- ================= TOOLCHAIN ================= -->

<div class="section">

<div class="title">ARM32 Toolchain</div>

<pre>
https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
</pre>

<p>Target: arm-linux-gnueabihf (ARMv7 32-bit)</p>

</div>

<!-- ================= BUILD ================= -->

<div class="section">

<div class="title">Build Instructions</div>

<pre>
cd Joystick_Go_Store-main
arm-linux-gnueabihf-g++ -O2 -std=c++17 store.cpp -o Joystick_Go_Store -lSDL2 -lavformat -lavcodec -lswresample -lavutil -lswscale -lpthread -ldl -lm
</pre>

</div>

<!-- ================= NOTES ================= -->

<div class="section">

<div class="title">Notes</div>

<pre>
store.cpp must be inside project folder
run build command from project root
</pre>

</div>

</body>
</html>
