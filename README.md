<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
</head>
<body style="margin:0; font-family: monospace; background:#0a0a0f; color:#00ffcc;">

<div style="font-family: monospace; background:#0a0a0f; color:#00ffcc; padding: 5px;">

<div style="padding:20px; border-bottom:1px solid #00ffcc22;">

  <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
    <h3>Releases</h3>
    <p>
      <a href="https://github.com/NYC0DEV/Joystick_Go_Store/releases" style="color:#00ffcc;">
        Open Releases Page
      </a>
    </p>
  </div>

  <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
    <p>
      <b>Joystick Go Store</b> is an open-source community store for GameStation Go.  
      It allows users to browse and download game and app binaries.
    </p>
  </div>

  <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
    <p>
      Download <b>Joystick_Go_Store.zip</b>, extract into:
    </p>
    <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">sd:\bin\Joystick_Go_Store</pre>
  </div>

  <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
    <h3>Controls</h3>
    <table border="1" cellpadding="6" style="color:#00ffcc; border-color:#00ffcc33; border-collapse: collapse;">
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

  <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
    <h3>Instructions</h3>
    <p>To compile and run the store application, ensure your build environment targets the ARM32 architecture and all required system headers are present.</p>
  </div>

</div>

<div style="padding:20px; border-bottom:1px solid #00ffcc22;">
  <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight: bold;">System Requirements</div>
  <ul style="line-height:1.6;">
    <li>ARM32 Toolchain (arm-linux-gnueabihf)</li>
    <li>SDL2 development libraries</li>
    <li>FFmpeg (libavformat, libavcodec, libavutil, libswresample, libswscale)</li>
    <li>libcurl</li>
    <li>nlohmann-json</li>
    <li>build tools (gcc, make, cmake optional)</li>
  </ul>
</div>

<div style="padding:20px; border-bottom:1px solid #00ffcc22;">
  <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight: bold;">Install Dependencies (Ubuntu / Debian)</div>
  <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">sudo apt update && sudo apt install libsdl2-dev libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev libcurl4-openssl-dev nlohmann-json3-dev build-essential</pre>
</div>

<div style="padding:20px; border-bottom:1px solid #00ffcc22;">
  <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight: bold;">ARM32 Toolchain</div>
  <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads</pre>
  <p>Target: arm-linux-gnueabihf (ARMv7 32-bit)</p>
</div>

<div style="padding:20px; border-bottom:1px solid #00ffcc22;">
  <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight: bold;">Build Instructions</div>
  <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">cd Joystick_Go_Store-main
arm-linux-gnueabihf-g++ -O2 -std=c++17 store.cpp -o Joystick_Go_Store -lSDL2 -lavformat -lavcodec -lswresample -lavutil -lswscale -lpthread -ldl -lm</pre>
</div>

<div style="padding:20px; border-bottom:1px solid #00ffcc22;">
  <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight: bold;">Notes</div>
  <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">store.cpp must be inside project folder
run build command from project root</pre>
</div>

</div>
</body>
</html>
