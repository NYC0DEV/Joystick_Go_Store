<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Joystick Go Store</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
            line-height: 1.6;
            color: #24292f;
            background-color: #ffffff;
            max-width: 850px;
            margin: 0 auto;
            padding: 32px;
        }
        h1, h2, h3 {
            margin-top: 24px;
            margin-bottom: 16px;
            font-weight: 600;
            line-height: 1.25;
            padding-bottom: 0.3em;
        }
        h1 {
            font-size: 2em;
            border-bottom: 1px solid #d0d7de;
        }
        h2 {
            font-size: 1.5em;
            border-bottom: 1px solid #d0d7de;
        }
        h3 {
            font-size: 1.25em;
        }
        p {
            margin-top: 0;
            margin-bottom: 16px;
        }
        a {
            color: #0969da;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
        table {
            border-spacing: 0;
            border-collapse: collapse;
            margin-bottom: 16px;
            width: 100%;
        }
        table th, table td {
            padding: 6px 13px;
            border: 1px solid #d0d7de;
        }
        table tr:nth-child(2n) {
            background-color: #f6f8fa;
        }
        pre {
            background-color: #f6f8fa;
            border-radius: 6px;
            padding: 16px;
            overflow: auto;
            font-family: ui-monospace, SFMono-Regular, SF Mono, Menlo, Consolas, Liberation Mono, monospace;
            font-size: 85%;
            line-height: 1.45;
            margin-bottom: 16px;
        }
        code {
            font-family: ui-monospace, SFMono-Regular, SF Mono, Menlo, Consolas, Liberation Mono, monospace;
            font-size: 85%;
            background-color: rgba(175,184,193,0.2);
            padding: 0.2em 0.4em;
            border-radius: 6px;
        }
        pre code {
            background-color: transparent;
            padding: 0;
        }
        ul {
            padding-left: 2em;
            margin-bottom: 16px;
        }
        blockquote {
            padding: 0 1em;
            color: #57606a;
            border-left: 0.25em solid #d0d7de;
            margin: 0 0 16px 0;
        }
        hr {
            height: 0.25em;
            padding: 0;
            margin: 24px 0;
            background-color: #d0d7de;
            border: 0;
        }
    </style>
</head>
<body>

    <h1>Joystick Go Store</h1>

    <p><strong>Joystick Go Store</strong> is an open-source community store for GameStation Go. It allows users to browse and download game and app binaries.</p>

    <p>📦 <strong><a href="https://github.com/NYC0DEV/Joystick_Go_Store/releases">Open Releases Page</a></strong></p>

    <h3>Installation</h3>
    <p>Download <strong>Joystick_Go_Store.zip</strong>, and extract it into:</p>
    <pre><code>sdcard/bin/Joystick_Go_Store/games</code></pre>

    <hr>

    <h2>🎮 Controls</h2>
    <table>
        <thead>
            <tr>
                <th>Button</th>
                <th>Action</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td><strong>D-Pad</strong></td>
                <td>Navigate</td>
            </tr>
            <tr>
                <td><strong>A</strong></td>
                <td>Select</td>
            </tr>
            <tr>
                <td><strong>B</strong></td>
                <td>Back / Quit</td>
            </tr>
            <tr>
                <td><strong>START</strong></td>
                <td>Launch application</td>
            </tr>
        </tbody>
    </table>

    <hr>

    <h2>📦 ZIP Package Requirements</h2>
    <p>Applications uploaded to the Joystick Go Store must be packaged as a <strong>.zip</strong> archive containing a <strong>Linux ARM32 (arm-linux-gnueabihf)</strong> executable.</p>
    <p>The executable inside the archive <strong>must have the exact same name as the .zip file</strong> (without the <code>.zip</code> extension).</p>

    <p><strong>Example:</strong></p>
    <pre><code>MyGame.zip
    ├── MyGame
    ├── assets/
    ├── data/
    └── config.json</code></pre>

    <p>When a user installs your application, the contents are extracted into a new folder within <code>sdcard/bin/Joystick_Go_Store/games/</code>. The store will launch the executable using the path: <code>sdcard/bin/Joystick_Go_Store/games/[FolderName]/[FileName]</code>.</p>

    <hr>

    <h2>🛠️ Build Instructions</h2>
    <p>To compile and run the store application, ensure your build environment targets the ARM32 architecture and all required system headers are present.</p>

    <h3>System Requirements</h3>
    <ul>
        <li>ARM32 Toolchain (<code>arm-linux-gnueabihf</code>)</li>
        <li>SDL2 development libraries</li>
        <li>FFmpeg (<code>libavformat</code>, <code>libavcodec</code>, <code>libavutil</code>, <code>libswresample</code>, <code>libswscale</code>)</li>
        <li>libcurl</li>
        <li>nlohmann-json</li>
        <li>build tools (gcc, make, cmake optional)</li>
    </ul>

    <h3>Install Dependencies (Ubuntu / Debian)</h3>
    <pre><code>sudo apt update && sudo apt install libsdl2-dev libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev libcurl4-openssl-dev nlohmann-json3-dev build-essential</code></pre>

    <h3>ARM32 Toolchain</h3>
    <p>Download from: <a href="https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads">ARM GNU Toolchain Downloads</a></p>
    <ul>
        <li><strong>Target:</strong> <code>arm-linux-gnueabihf</code> (ARMv7 32-bit)</li>
    </ul>

    <h3>Compiling</h3>
    <pre><code>cd Joystick_Go_Store-main
    arm-linux-gnueabihf-g++ -O2 -std=c++17 store.cpp -o Joystick_Go_Store -lSDL2 -lavformat -lavcodec -lswresample -lavutil -lswscale -lpthread -ldl -lm</code></pre>

    <blockquote>
        <p><strong>Notes:</strong></p>
        <ul>
            <li><code>store.cpp</code> must be inside the project folder.</li>
            <li>Run the build command from the project root.</li>
        </ul>
    </blockquote>

</body>
</html>
