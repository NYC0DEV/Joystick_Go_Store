
<div class="container">
<!-- ======= RELEASES ======= -->
<div class="box">
<h2>Releases</h2>
<p>
<a href="https://github.com/NYC0DEV/Joystick_Go_Store/releases/tag/v1.0.0" target="_blank">
                Joystick Go Store release
            </a>
</p>
<p style="margin-top: 0.8rem;">
            Download <strong>Joystick_Go_Store.zip</strong>, extract into:
        </p>
<pre>sd:\bin\Joystick_Go_Store</pre>
</div>
<!-- ======= CUSTOM FIRMWARE ======= -->
<div class="firmware-box">
<h2>Custom Firmware — required to launch</h2>
<p style="font-size: 1.05rem;">
<strong>To run this application on your device, you must install the custom firmware.</strong>
</p>
<p>
            All instructions, patcher, and SD card setup are available on the official topic:
        </p>
<p style="font-size: 1.2rem; margin: 0.8rem 0;">
<a href="https://forums.atariage.com/topic/391778-gamestation-go-12-patcher-v2-and-sd-card-26-official-release/" target="_blank">
                GameStation Go 1.2 Patcher v2 + SD Card 2.6 Official Release
            </a>
</p>
<p>
<span class="badge">.</span>
<span class="badge">..amp; .</span>
</p>
<p style="margin-top: 0.8rem;">
            After you have the custom firmware installed and your SD card prepared, 
            you can download and launch my application directly on your device.
        </p>
<p style="color: #ffbb66; font-style: italic;">
            Follow the patcher instructions carefully — once the firmware is ready, 
            the Joystick Go Store will work as expected.
        </p>
</div>
<!-- ======= START_LOCAL_SD WARNING ======= -->
<div class="box warning">
<h3>⚠️start_local_sd.sh required⚠️</h3>
<p>
<strong>Download <a href="https://github.com/NYC0DEV/Joystick_Go_Store/blob/main/start_local_sd.sh" target="_blank">start_local_sd.sh</a></strong>
            and place it in:
        </p>
<pre>sd:\start_local_sd.sh</pre>
<p>
            This will overwrite the existing <strong>start_local_sd.sh</strong> script.
        </p>
<p>
            Without this file, the application will not launch from <span class="code-inline">sd:\Games\Misc\Joystick_Go_Store.zip</span>.
        </p>
<p>
            This version is fully compatible with 
            <a href="https://forums.atariage.com/topic/391778-gamestation-go-12-patcher-v2-and-sd-card-26-official-release/" target="_blank">
                SD_Card_Folders_Files_2.6
            </a>.
        </p>
</div>
<!-- ======= LAUNCH ======= -->
<div class="box">
<h3>Launch</h3>
<pre>sd:\Games\Misc\Joystick_Go_Store.zip</pre>
</div>
<!-- ======= CONTROLS ======= -->
<div class="box">
<h2>Controls</h2>
<table>
<tr><th>Button</th><th>Action</th></tr>
<tr><td>D-Pad</td><td>Navigate</td></tr>
<tr><td>A</td><td>Select</td></tr>
<tr><td>B</td><td>Back / Quit</td></tr>
<tr><td>SELECT</td><td>Pause Music / Hold to Change Music</td></tr>
<tr><td>START</td><td>Launch application / Update</td></tr>
</table>
</div>
<!-- ======= UPLOAD APPLICATION ======= -->
<div class="box">
<h2>Upload Application</h2>
<!-- website banner -->
<div class="upload-website-banner">
<strong>🌐 Upload your ARM32 app via:</strong>
<span class="url">https://joystick-go-store.pages.dev</span>
</div>
<div class="highlight-box">
<p>Follow these steps below to submit your ARM32 app to the Joystick Go Store.</p>
</div>
<h3>Step 1: Upload &amp; Binary</h3>
<p>Upload your <strong>.zip</strong> file containing your ARM32 native binary.</p>
<p>After uploading, select the executable ARM32 binary from inside the ZIP.</p>
<p><strong>Example:</strong></p>
<pre>
ZIP: Your_App.zip
Selected Binary: Your_App/bin/Your_App/Your_App</pre>
<p>After selection, the website shows the full device launch path:</p>
<pre>/mnt/sdcard/bin/Joystick_Go_Store/games/user/Your_App/bin/Your_App/Your_App</pre>
<p>The selected binary defines the ARM32 executable that will launch on the GameStation Go device.</p>
<hr style="margin: 1.2rem 0;"/>
<h3>Step 2: Details</h3>
<p>Complete information about your application:</p>
<ul>
<li>Title</li>
<li>Version</li>
<li>Category</li>
<li>Description</li>
<li>Application image</li>
<li>etc.</li>
</ul>
<p>This information defines how your application appears in the Joystick Go Store.</p>
<hr style="margin: 1.2rem 0;"/>
<h3>Step 3: Review &amp; Submit</h3>
<p>Review your application information and submit.</p>
<p>After submitting, the application status becomes:</p>
<pre>Pending</pre>
<p>Wait for approval before your application is published.</p>
<div class="highlight-box" style="margin-top: 1.2rem;">
<p><strong>💡 Tip:</strong> Make sure your binary is compiled for <strong>ARM32 (arm-linux-gnueabihf)</strong> and all dependencies are included or available on the device.</p>
</div>
</div>
<!-- ======= INSTALL ON GAMESTATION GO ======= -->
<div class="box">
<h2>Installing on GameStation Go</h2>
<p>After approval, users can download from Joystick Go Store.</p>
<p>The application ZIP is installed to:</p>
<pre>sd:\bin\Joystick_Go_Store\games\user\Your_App\</pre>
<p>After installation, Joystick Go Store creates or updates:</p>
<pre>sd:\bin\Joystick_Go_Store\installed.txt</pre>
<h3>installed.txt format</h3>
<pre>Title|Install Path|Binary Path|Display Name|Category|Size|Install Date|Version</pre>
<h4>Example:</h4>
<pre>Your_App|sd:\bin\Joystick_Go_Store\games\user\Your_App\|sd:\bin\Joystick_Go_Store\games\user\Your_App\bin\Your_App\Your_App|Your_App|tool|6 KB|2026-07-30 05:41:08|1.0</pre>
<p>The <strong>Binary Path</strong> saved in installed.txt is the same ARM32 executable path selected during upload.</p>
</div>
<!-- ======= LAUNCHING THE APP ======= -->
<div class="box">
<h2>Launching the Application</h2>
<p>User opens Joystick Go Store on the device.</p>
<p>User selects the installed application.</p>
<p>When the user presses <strong>START</strong>, Joystick Go Store reads the saved Binary Path from:</p>
<pre>sd:\bin\Joystick_Go_Store\installed.txt</pre>
<p>It launches the selected ARM32 binary that the uploader chose.</p>
<p><strong>Example:</strong></p>
<pre>sd:\bin\Joystick_Go_Store\games\user\Your_App\bin\Your_App\Your_App</pre>
<p>The application launches directly on the GameStation Go device.</p>
</div>
<!-- ======= JOYSTICK MANAGER ======= -->
<div class="box">
<h2>Joystick Go Manager</h2>
<p>
<a href="https://github.com/NYC0DEV/Joystick_Go_Store/releases/tag/v1.0.0b" style="font-size: 1.1rem;" target="_blank">
                Joystick Go Manager Release
            </a>
</p>
<p>Download <strong>Joystick_Go_Manager.zip</strong> and extract to:</p>
<pre>sd:\bin\Joystick_Go_Manager</pre>
<p>Manage applications and games downloaded from the Joystick Go Store.</p>
<h3 style="margin-top: 1rem;">Integration with Joystick Go Store</h3>
<p>Users do not always need to open Joystick Go Store to view installed apps.</p>
<p>Joystick Go Manager reads:</p>
<pre>sd:\bin\Joystick_Go_Store\installed.txt</pre>
<p>It uses this file to display all installed applications on the SD card.</p>
<p>The installed.txt file contains:</p>
<ul>
<li>Application title</li>
<li>Install location</li>
<li>ARM32 binary launch path</li>
<li>Category</li>
<li>Version</li>
<li>Other info</li>
</ul>
<p>Joystick Go Manager and Joystick Go Store share the same installed.txt file.</p>
<p>Any app downloaded through Joystick Go Store appears in Joystick Go Manager automatically.</p>
<p>When the user selects an app from Joystick Go Manager and presses <strong>START</strong>, it reads the saved ARM32 binary path from installed.txt and launches it.</p>
</div>
<!-- ======= INSTRUCTIONS ======= -->
<div class="box">
<h2>Instructions</h2>
<p>To compile and run the store application, ensure your build environment targets the <strong>ARM32</strong> architecture and all required system headers are present.</p>
</div>
<!-- ======= SYSTEM REQUIREMENTS ======= -->
<div class="box">
<h2>System Requirements</h2>
<ul>
<li>ARM32 Toolchain (arm-linux-gnueabihf)</li>
<li>SDL2 development libraries</li>
<li>FFmpeg (libavformat, libavcodec, libavutil, libswresample, libswscale)</li>
<li>libcurl</li>
<li>nlohmann-json</li>
<li>build tools (gcc, make, cmake optional)</li>
</ul>
</div>
<!-- ======= DEPENDENCIES ======= -->
<div class="box">
<h2>Install Dependencies (Ubuntu / Debian)</h2>
<pre>sudo apt update &amp;&amp; sudo apt install libsdl2-dev libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev libcurl4-openssl-dev nlohmann-json3-dev build-essential</pre>
</div>
<!-- ======= TOOLCHAIN ======= -->
<div class="box">
<h2>ARM32 Toolchain</h2>
<pre>https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads</pre>
<p>Target: <strong>arm-linux-gnueabihf</strong> (ARMv7 32-bit)</p>
</div>
<!-- ======= BUILD ======= -->
<div class="box">
<h2>Build Instructions</h2>
<pre>cd Joystick_Go_Store-main
arm-linux-gnueabihf-g++ -O2 -std=c++17 store.cpp -o Joystick_Go_Store -lSDL2 -lavformat -lavcodec -lswresample -lavutil -lswscale -lpthread -ldl -lm</pre>
</div>
<!-- ======= NOTES ======= -->
<div class="box">
<h2>Notes</h2>
<pre>store.cpp must be inside project folder
run build command from project root</pre>
</div>
<!-- ======= FOOTER ======= -->
<div class="footer">
<span></span>
</div>
</div>
