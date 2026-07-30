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
      <a href="https://github.com/NYC0DEV/Joystick_Go_Store/releases](https://github.com/NYC0DEV/Joystick_Go_Store/releases/tag/v1.0.0b" style="color:#00ffcc;">
        Open Releases Page
      </a>
    </p>
  </div>

  <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
    <p>
      <b>Joystick Go Manager is an open-source application for GameStation Go. It allows users to browse the Joystick Go Store, download games and applications to their device, then launch and manage them from within Joystick Go Manager.
    </p>
  </div>

  <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
    <p>
      Download <b>Joystick_Go_Manager.zip</b>, extract into:
    </p>
    <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">sd:\bin\Joystick_Go_Manager</pre>
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
        <td>Launch</td>
      </tr>
      <tr>
        <td>B</td>
        <td>Quit</td>
      </tr>
       <tr>
        <td>L/R</td>
        <td>Page</td>
      </tr>
      <tr>
        <td>SELECT</td>
        <td>Mute Music</td>
      </tr>
    </table>
  </div>

  <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
    <h3>Instructions</h3>
    <p>To compile and run the manager application, ensure your build environment targets the ARM32 architecture and all required system headers are present.</p>
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
  <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">cd Joystick_Go_Manager-manager
arm-linux-gnueabihf-g++ -O2 -std=c++17 manager.cpp -o Joystick_Go_Manager -lSDL2 -lavformat -lavcodec -lswresample -lavutil -lswscale -lpthread -ldl -lm</pre>
</div>

<div style="padding:20px; border-bottom:1px solid #00ffcc22;">
  <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight: bold;">Notes</div>
  <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">manager.cpp must be inside project folder
run build command from project root</pre>
</div>

</div>
</body>
</html>
