<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
</head>
<body style="margin:0; font-family: monospace; background:#0a0a0f; color:#00ffcc;">
    <div style="font-family: monospace; background:#0a0a0f; color:#00ffcc; padding: 5px;">
        
        <!-- Top Section -->
        <div style="padding:20px; border-bottom:1px solid #00ffcc22;">
            <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
                <h3>Releases</h3>
                <p>
                    <a href="https://github.com/NYC0DEV/Joystick_Go_Store/releases" style="color:#00ffcc;">Open Releases Page</a>
                </p>
            </div>
            <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
                <p><b>Joystick Go Store</b> is an open-source community store for GameStation Go. It allows users to browse and download game and app binaries.</p>
            </div>
            <div style="padding:15px; border:1px solid #00ffcc33; margin-bottom:15px;">
                <p>Download <b>Joystick_Go_Store.zip</b>, extract into:</p>
                <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">sdcard/bin/Joystick_Go_Store/games</pre>
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

        <!-- ZIP Package Requirements -->
        <div style="padding:20px; border-bottom:1px solid #00ffcc22;">
            <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight:bold;">
                ZIP Package Requirements
            </div>
            <div style="padding:15px; border:1px solid #00ffcc33;">
                <p>Applications uploaded to the Joystick Go Store must be packaged as a <b>.zip</b> archive containing a <b>Linux ARM32 (arm-linux-gnueabihf)</b> executable.</p>
                <p>The executable inside the archive <b>must have the exact same name as the .zip file</b> (without the <code>.zip</code> extension).</p>
                
                <p><b>Example:</b></p>
                <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">MyGame.zip
├── MyGame
├── assets/
├── data/
└── config.json</pre>
                
                <p>When a user installs your application, the contents are extracted into a new folder within <code>sdcard/bin/Joystick_Go_Store/games/</code>. The store will launch the executable using the path: <code>sdcard/bin/Joystick_Go_Store/games/[FolderName]/[FileName]</code>.</p>
            </div>
        </div>

        <!-- System Requirements -->
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
        
        <!-- Install Dependencies -->
        <div style="padding:20px; border-bottom:1px solid #00ffcc22;">
            <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight: bold;">Install Dependencies (Ubuntu / Debian)</div>
            <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">sudo apt update && sudo apt install libsdl2-dev libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev libcurl4-openssl-dev nlohmann-json3-dev build-essential</pre>
        </div>
        
        <!-- ARM32 Toolchain -->
        <div style="padding:20px; border-bottom:1px solid #00ffcc22;">
            <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight: bold;">ARM32 Toolchain</div>
            <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads</pre>
            <p>Target: arm-linux-gnueabihf (ARMv7 32-bit)</p>
        </div>
        
        <!-- Build Instructions -->
        <div style="padding:20px; border-bottom:1px solid #00ffcc22;">
            <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight: bold;">Build Instructions</div>
            <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">cd Joystick_Go_Store-main
arm-linux-gnueabihf-g++ -O2 -std=c++17 store.cpp -o Joystick_Go_Store -lSDL2 -lavformat -lavcodec -lswresample -lavutil -lswscale -lpthread -ldl -lm</pre>
        </div>
        
        <!-- Notes -->
        <div style="padding:20px; border-bottom:1px solid #00ffcc22;">
            <div style="font-size:18px; margin-bottom:10px; color:#00ffcc; font-weight: bold;">Notes</div>
            <pre style="background:#00000055; padding:12px; border:1px solid #00ffcc33; overflow:auto; white-space:pre-wrap; font-family: monospace; color:#00ffcc;">store.cpp must be inside project folder
run build command from project root</pre>
        </div>

    </div>
</body>
</html>
