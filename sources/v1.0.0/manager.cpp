#include <SDL2/SDL.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
}
#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <map>
#include <cmath>

#define BTN_A        1
#define BTN_B        3
#define BTN_X        0
#define BTN_SELECT   6
#define BTN_START    7
#define BTN_L1       2
#define BTN_R1       5
#define BTN_DPUP     29
#define BTN_DPDOWN   32
#define BTN_DPLEFT   30
#define BTN_DPRIGHT  31

const std::string INSTALLED_LIST_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/installed.txt";
const std::string FONT_PNG_PATH = "/mnt/sdcard/bin/Joystick_Go_Manager/content/fonts/Arial_White.png";
const std::string FONT_XML_PATH = "/mnt/sdcard/bin/Joystick_Go_Manager/content/fonts/Arial_White.xml";
const std::string MUSIC_PATH = "/mnt/sdcard/bin/Joystick_Go_Manager/content/music/Music.mp3";
const std::string CLICK_SOUND_PATH = "/mnt/sdcard/bin/Joystick_Go_Manager/content/sound/click.ogg";
const std::string BACKGROUND_IMAGE = "/mnt/sdcard/bin/Joystick_Go_Manager/content/images/background.png";
const std::string TOPBAR_IMAGE = "/mnt/sdcard/bin/Joystick_Go_Manager/content/images/top.png";
const std::string BATTERY_BASE = "/mnt/sdcard/bin/Joystick_Go_Manager/content/images/battery/";

static SDL_AudioDeviceID g_audio_dev = 0;
static SDL_AudioDeviceID g_effect_audio = 0;
static std::atomic<bool> g_stop{false};
static std::thread g_bgAudioThread;
static std::atomic<int> g_musicVolume{100};
static std::atomic<int> g_previousMusicVolume{100};
static std::atomic<bool> g_muted{false};
static std::vector<int16_t> g_clickSamples;

struct App {
    std::string title;
    std::string install_folder;
    std::string binary_path;
    std::string description;
    std::string author;
    std::string version;
};
static std::vector<App> g_apps;

struct CharInfo {
    int x, y, w, h;
    int xoffset, yoffset, xadvance;
};
static std::map<int, CharInfo> g_fontChars;
static SDL_Texture* g_fontTexture = nullptr;
static bool g_fontLoaded = false;

static SDL_Texture* LoadImageFFmpeg(const std::string& path, SDL_Renderer* ren);
static std::string filterToASCII(const std::string& text);

static void drawRect(SDL_Renderer* ren, int x, int y, int w, int h,
                     Uint8 r, Uint8 g, Uint8 b, Uint8 a,
                     bool border = false, Uint8 br=0, Uint8 bg=0, Uint8 bb=0, Uint8 ba=0) {
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, r, g, b, a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(ren, &rect);
    if (border) {
        SDL_SetRenderDrawColor(ren, br, bg, bb, ba);
        SDL_RenderDrawRect(ren, &rect);
    }
}

static bool loadCustomFont(SDL_Renderer* ren) {
    g_fontTexture = LoadImageFFmpeg(FONT_PNG_PATH, ren);
    if (!g_fontTexture) return false;
    std::ifstream xml(FONT_XML_PATH);
    if (!xml.is_open()) { SDL_DestroyTexture(g_fontTexture); g_fontTexture = nullptr; return false; }
    std::string content((std::istreambuf_iterator<char>(xml)), std::istreambuf_iterator<char>());
    xml.close();
    size_t pos = 0;
    while ((pos = content.find("<char", pos)) != std::string::npos) {
        size_t end = content.find("/>", pos);
        if (end == std::string::npos) break;
        std::string tag = content.substr(pos, end - pos + 2);
        pos = end + 2;
        auto getAttr = [&](const std::string& name) -> int {
            size_t n = tag.find(name + "=\"");
            if (n == std::string::npos) return 0;
            n += name.size() + 2;
            size_t q = tag.find("\"", n);
            if (q == std::string::npos) return 0;
            return std::stoi(tag.substr(n, q - n));
        };
        int id = getAttr("id");
        if (id == 0) continue;
        CharInfo ci;
        ci.x = getAttr("x");
        ci.y = getAttr("y");
        ci.w = getAttr("width");
        ci.h = getAttr("height");
        ci.xoffset = getAttr("xoffset");
        ci.yoffset = getAttr("yoffset");
        ci.xadvance = getAttr("xadvance");
        if (ci.w == 0 || ci.h == 0) continue;
        g_fontChars[id] = ci;
    }
    if (g_fontChars.empty()) { SDL_DestroyTexture(g_fontTexture); g_fontTexture = nullptr; return false; }
    g_fontLoaded = true;
    return true;
}

static void drawText(SDL_Renderer* ren, const std::string& text, int x, int y, float scale = 1.0f, Uint8 r=255, Uint8 g=255, Uint8 b=255) {
    if (text.empty() || !ren || !g_fontLoaded || !g_fontTexture) return;
    std::string safeText = filterToASCII(text);
    SDL_SetTextureColorMod(g_fontTexture, r, g, b);
    int cursorX = x;
    for (unsigned char ch : safeText) {
        auto it = g_fontChars.find((int)ch);
        if (it == g_fontChars.end()) { cursorX += (int)(8 * scale); continue; }
        const CharInfo& ci = it->second;
        SDL_Rect src = {ci.x, ci.y, ci.w, ci.h};
        int dstX = cursorX + (int)(ci.xoffset * scale);
        int dstY = y + (int)(ci.yoffset * scale);
        int dstW = (int)(ci.w * scale);
        int dstH = (int)(ci.h * scale);
        SDL_Rect dst = {dstX, dstY, dstW, dstH};
        SDL_RenderCopy(ren, g_fontTexture, &src, &dst);
        cursorX += (int)(ci.xadvance * scale);
    }
    SDL_SetTextureColorMod(g_fontTexture, 255, 255, 255);
}

static SDL_Texture* renderStringToTextureColored(const std::string& text, SDL_Renderer* ren, Uint8 r, Uint8 g, Uint8 b, int* outW, int* outH) {
    if (text.empty() || !ren || !g_fontLoaded || !g_fontTexture) return nullptr;
    int totalW = 0, totalH = 0;
    for (unsigned char ch : text) {
        auto it = g_fontChars.find((int)ch);
        if (it != g_fontChars.end()) { totalW += it->second.xadvance; if (it->second.h > totalH) totalH = it->second.h; }
        else totalW += 8;
    }
    if (totalW == 0 || totalH == 0) return nullptr;
    SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, totalW, totalH);
    if (!tex) return nullptr;
    SDL_SetRenderTarget(ren, tex);
    SDL_SetRenderDrawColor(ren, 0,0,0,0);
    SDL_RenderClear(ren);
    drawText(ren, text, 0, 0, 1.0f, r, g, b);
    SDL_SetRenderTarget(ren, nullptr);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    if (outW) *outW = totalW;
    if (outH) *outH = totalH;
    return tex;
}

static std::string filterToASCII(const std::string& text) {
    std::string result;
    for (unsigned char ch : text) if (ch >= 32 && ch <= 126) result += ch;
    return result;
}

static SDL_Texture* LoadImageFFmpeg(const std::string& path, SDL_Renderer* ren) {
    AVFormatContext* fc = avformat_alloc_context();
    if (avformat_open_input(&fc, path.c_str(), nullptr, nullptr) != 0) return nullptr;
    avformat_find_stream_info(fc, nullptr);
    int vi = av_find_best_stream(fc, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (vi < 0) { avformat_close_input(&fc); return nullptr; }
    AVCodecContext* vc = avcodec_alloc_context3(avcodec_find_decoder(fc->streams[vi]->codecpar->codec_id));
    avcodec_parameters_to_context(vc, fc->streams[vi]->codecpar);
    avcodec_open2(vc, vc->codec, nullptr);
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frm = av_frame_alloc();
    SDL_Texture* tex = nullptr;
    while (av_read_frame(fc, pkt) >= 0) {
        if (pkt->stream_index == vi) {
            avcodec_send_packet(vc, pkt);
            if (avcodec_receive_frame(vc, frm) == 0) {
                SwsContext* sws = sws_getContext(frm->width, frm->height, (AVPixelFormat)frm->format,
                                                 frm->width, frm->height, AV_PIX_FMT_RGBA,
                                                 SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
                AVFrame* rgba = av_frame_alloc();
                rgba->format = AV_PIX_FMT_RGBA;
                rgba->width = frm->width;
                rgba->height = frm->height;
                av_frame_get_buffer(rgba, 32);
                sws_scale(sws, frm->data, frm->linesize, 0, frm->height, rgba->data, rgba->linesize);
                sws_freeContext(sws);
                tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, frm->width, frm->height);
                SDL_UpdateTexture(tex, nullptr, rgba->data[0], rgba->linesize[0]);
                SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
                av_frame_free(&rgba);
                break;
            }
        }
        av_packet_unref(pkt);
    }
    av_frame_free(&frm);
    av_packet_free(&pkt);
    avcodec_free_context(&vc);
    avformat_close_input(&fc);
    return tex;
}

static int getBatteryPercent() {
    std::ifstream f("/sys/class/power_supply/battery/capacity");
    if (!f.is_open()) return -1;
    int cap; f >> cap; return cap;
}
static bool isCharging() {
    std::ifstream f("/sys/class/power_supply/battery/status");
    if (!f.is_open()) return false;
    std::string status; f >> status;
    return (status == "Charging" || status == "Full");
}

static bool loadSound(const std::string& path, std::vector<int16_t>& buffer) {
    AVFormatContext* fc = avformat_alloc_context();
    if (avformat_open_input(&fc, path.c_str(), nullptr, nullptr) != 0 || !fc) return false;
    avformat_find_stream_info(fc, nullptr);
    int ai = av_find_best_stream(fc, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (ai < 0) { avformat_close_input(&fc); return false; }
    AVStream* as = fc->streams[ai];
    const AVCodec* cc = avcodec_find_decoder(as->codecpar->codec_id);
    AVCodecContext* ac = avcodec_alloc_context3(cc);
    avcodec_parameters_to_context(ac, as->codecpar);
    avcodec_open2(ac, cc, nullptr);
    SwrContext* swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_layout", av_get_default_channel_layout(ac->channels), 0);
    av_opt_set_int(swr, "out_channel_layout", 3, 0);
    av_opt_set_int(swr, "in_sample_rate", ac->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", 44100, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", ac->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    swr_init(swr);
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frm = av_frame_alloc();
    buffer.clear();
    while (av_read_frame(fc, pkt) >= 0) {
        if (pkt->stream_index == ai) {
            avcodec_send_packet(ac, pkt);
            while (true) {
                int ret = avcodec_receive_frame(ac, frm);
                if (ret < 0) break;
                int ns = av_rescale_rnd(swr_get_delay(swr, ac->sample_rate) + frm->nb_samples, 44100, ac->sample_rate, AV_ROUND_UP);
                int16_t* b = (int16_t*)av_malloc(ns * 4);
                uint8_t* od[1] = { (uint8_t*)b };
                int cn = swr_convert(swr, od, ns, (const uint8_t**)frm->data, frm->nb_samples);
                if (cn > 0) buffer.insert(buffer.end(), b, b + cn * ac->channels);
                av_free(b);
                av_frame_unref(frm);
            }
        }
        av_packet_unref(pkt);
    }
    av_frame_free(&frm);
    av_packet_free(&pkt);
    swr_free(&swr);
    avcodec_free_context(&ac);
    avformat_close_input(&fc);
    return !buffer.empty();
}

static void playSound(const std::vector<int16_t>& buffer) {
    if (buffer.empty()) return;
    size_t samplesToUse = std::min(buffer.size(), (size_t)8820);
    SDL_QueueAudio(g_effect_audio, buffer.data(), samplesToUse * sizeof(int16_t));
}

static std::vector<App> readApps() {
    std::vector<App> apps;
    std::ifstream in(INSTALLED_LIST_PATH);
    if (!in.is_open()) return apps;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::vector<std::string> fields;
        size_t pos = 0;
        while ((pos = line.find('|')) != std::string::npos) {
            fields.push_back(line.substr(0, pos));
            line.erase(0, pos + 1);
        }
        fields.push_back(line);
        if (fields.size() < 3) continue;
        App app;
        app.title = fields[0];
        app.install_folder = fields[1];
        app.binary_path = fields[2];
        if (fields.size() >= 4) app.description = fields[3];
        if (fields.size() >= 5) app.author = fields[4];
        if (fields.size() >= 9) app.version = fields[8];
        else {
            std::string vf = app.install_folder + ".version";
            std::ifstream vin(vf);
            if (vin.is_open()) { std::getline(vin, app.version); vin.close(); }
        }
        struct stat st;
        if (stat(app.binary_path.c_str(), &st) != 0 || !(st.st_mode & S_IFREG)) {
            printf("Skipping %s (binary missing)\n", app.title.c_str());
            continue;
        }
        apps.push_back(app);
    }
    return apps;
}

static bool removeDirectory(const std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) return false;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        std::string fullPath = path + "/" + name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) removeDirectory(fullPath);
            else unlink(fullPath.c_str());
        }
    }
    closedir(dir);
    rmdir(path.c_str());
    return true;
}

static bool uninstallAppInternal(App& app) {
    std::string folder = app.install_folder;
    if (folder.empty()) folder = "/mnt/sdcard/bin/Joystick_Go_Store/games/" + app.title + "/";
    if (!removeDirectory(folder)) return false;
    std::ifstream in(INSTALLED_LIST_PATH);
    if (!in.is_open()) return false;
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line.find(app.title + "|") != 0) lines.push_back(line);
    }
    in.close();
    std::ofstream out(INSTALLED_LIST_PATH);
    if (!out.is_open()) return false;
    for (const auto& l : lines) out << l << "\n";
    out.close();
    return true;
}

static void playAudioLoop(const std::string& path) {
    while (!g_stop) {
        AVFormatContext* fc = avformat_alloc_context();
        fc->interrupt_callback.callback = [](void*) -> int { return g_stop.load() ? 1 : 0; };
        if (avformat_open_input(&fc, path.c_str(), nullptr, nullptr) != 0 || !fc) { SDL_Delay(100); continue; }
        avformat_find_stream_info(fc, nullptr);
        int ai = av_find_best_stream(fc, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
        if (ai < 0) { avformat_close_input(&fc); continue; }
        AVStream* as = fc->streams[ai];
        const AVCodec* cc = avcodec_find_decoder(as->codecpar->codec_id);
        AVCodecContext* ac = avcodec_alloc_context3(cc);
        avcodec_parameters_to_context(ac, as->codecpar);
        avcodec_open2(ac, cc, nullptr);
        SwrContext* swr = swr_alloc();
        av_opt_set_int(swr, "in_channel_layout", av_get_default_channel_layout(ac->channels), 0);
        av_opt_set_int(swr, "out_channel_layout", 3, 0);
        av_opt_set_int(swr, "in_sample_rate", ac->sample_rate, 0);
        av_opt_set_int(swr, "out_sample_rate", 44100, 0);
        av_opt_set_sample_fmt(swr, "in_sample_fmt", ac->sample_fmt, 0);
        av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
        swr_init(swr);
        AVPacket* pkt = av_packet_alloc();
        AVFrame* frm = av_frame_alloc();
        const uint32_t MAX_AQ = 88200;
        bool eof = false;
        while (!g_stop && !eof) {
            while (!g_stop && SDL_GetQueuedAudioSize(g_audio_dev) > MAX_AQ) SDL_Delay(5);
            if (g_stop) break;
            int ret = av_read_frame(fc, pkt);
            if (ret < 0) { eof = true; break; }
            if (pkt->stream_index == ai) {
                avcodec_send_packet(ac, pkt);
                while (!g_stop) {
                    if (avcodec_receive_frame(ac, frm) < 0) break;
                    int ns = av_rescale_rnd(swr_get_delay(swr, ac->sample_rate) + frm->nb_samples, 44100, ac->sample_rate, AV_ROUND_UP);
                    int16_t* b = (int16_t*)av_malloc(ns * 4);
                    uint8_t* od[1] = { (uint8_t*)b };
                    int cn = swr_convert(swr, od, ns, (const uint8_t**)frm->data, frm->nb_samples);
                    if (cn > 0) {
                        int vol = g_muted ? 0 : g_musicVolume.load();
                        if (vol != 100) {
                            for (int i = 0; i < cn * ac->channels; ++i) {
                                int32_t s = b[i];
                                s = (s * vol) / 100;
                                if (s > 32767) s = 32767;
                                if (s < -32768) s = -32768;
                                b[i] = (int16_t)s;
                            }
                        }
                        SDL_QueueAudio(g_audio_dev, b, cn * 4);
                    }
                    av_free(b);
                    av_frame_unref(frm);
                }
            }
            av_packet_unref(pkt);
        }
        av_frame_free(&frm);
        av_packet_free(&pkt);
        swr_free(&swr);
        avcodec_free_context(&ac);
        avformat_close_input(&fc);
        if (g_stop) break;
    }
}

int main(int argc, char* argv[]) {
    const char* libs[] = {"libavformat.so.58","libavutil.so.56","libswresample.so.3","libswscale.so.5","libavcodec.so.58"};
    for (int i = 0; i < 5; i++) dlopen(libs[i], RTLD_NOW | RTLD_GLOBAL);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) != 0) return -1;
    SDL_JoystickOpen(0);

    SDL_Window* win = SDL_CreateWindow("Manager", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 600, SDL_WINDOW_SHOWN);
    if (!win) win = SDL_CreateWindow("Manager", 0, 0, 1024, 600, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    int win_w, win_h; SDL_GetWindowSize(win, &win_w, &win_h);

    SDL_AudioSpec w = {}, g = {};
    w.freq = 44100; w.format = AUDIO_S16SYS; w.channels = 2; w.samples = 2048;
    g_audio_dev = SDL_OpenAudioDevice(nullptr, 0, &w, &g, 0);
    SDL_PauseAudioDevice(g_audio_dev, 0);
    g_effect_audio = SDL_OpenAudioDevice(nullptr, 0, &w, &g, 0);
    SDL_PauseAudioDevice(g_effect_audio, 0);

    if (!loadSound(CLICK_SOUND_PATH, g_clickSamples)) printf("Warning: click.ogg not loaded\n");

    if (!loadCustomFont(ren)) { printf("FATAL: font not loaded\n"); return 1; }

    SDL_Texture* bgTex = LoadImageFFmpeg(BACKGROUND_IMAGE, ren);
    SDL_Texture* topBarTex = LoadImageFFmpeg(TOPBAR_IMAGE, ren);
    if (!bgTex) printf("Warning: background.png not loaded\n");
    if (!topBarTex) printf("Warning: top.png not loaded\n");

    SDL_Texture* batteryTex[4] = {nullptr};
    for (int i = 1; i <= 4; ++i) {
        std::string path = BATTERY_BASE + std::to_string(i) + ".png";
        batteryTex[i-1] = LoadImageFFmpeg(path, ren);
    }
    SDL_Texture* chargingTex = LoadImageFFmpeg(BATTERY_BASE + "charg.png", ren);

    g_apps = readApps();
    if (g_apps.empty()) {
        App dummy; dummy.title = "No apps installed"; dummy.author = ""; dummy.version = ""; dummy.description = "Install apps via Joystick_Go_Store first.";
        g_apps.push_back(dummy);
    }

    const int HEADER_HEIGHT = (int)(win_h * 0.10f);
    const int FOOTER_HEIGHT = 30;
    const int DETAILS_HEIGHT = 130;
    const int LIST_TOP = HEADER_HEIGHT + 8;
    const int LIST_BOTTOM = win_h - DETAILS_HEIGHT - FOOTER_HEIGHT - 8;
    const int LIST_WIDTH = win_w - 120;
    const int LIST_LEFT = (win_w - LIST_WIDTH) / 2;
    const int LIST_RIGHT = LIST_LEFT + LIST_WIDTH;

    const int ITEMS_PER_PAGE = 10;
    int currentPage = 0;
    int selected = 0;
    bool quit = false;
    int totalPages = ((int)g_apps.size() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (totalPages == 0) totalPages = 1;

    g_stop = false;
    g_bgAudioThread = std::thread(playAudioLoop, MUSIC_PATH);

    bool showConfirm = false;
    int confirmIndex = -1;

    auto clampSelection = [&]() {
        int total = (int)g_apps.size();
        if (total == 0) { selected = -1; currentPage = 0; return; }
        if (selected < 0) selected = 0;
        if (selected >= total) selected = total - 1;
        int pageStart = currentPage * ITEMS_PER_PAGE;
        int pageEnd = std::min(pageStart + ITEMS_PER_PAGE - 1, total - 1);
        if (selected < pageStart || selected > pageEnd) {
            currentPage = selected / ITEMS_PER_PAGE;
        }
        int totalPages = (total + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
        if (currentPage >= totalPages) currentPage = totalPages - 1;
        if (currentPage < 0) currentPage = 0;
    };
    clampSelection();

    Uint32 lastBatteryCheck = 0;
    int lastPercent = -1;
    bool lastCharging = false;
    SDL_Texture* percentTex = nullptr;
    int percentW=0, percentH=0;

    while (!quit) {
        Uint32 now = SDL_GetTicks();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) quit = true;
            if (ev.type == SDL_JOYBUTTONDOWN) {
                int btn = ev.jbutton.button;

                if (!g_clickSamples.empty() && btn != BTN_SELECT) playSound(g_clickSamples);

                if (btn == BTN_SELECT && !showConfirm) {
                    if (g_muted) { g_muted = false; g_musicVolume = g_previousMusicVolume.load(); }
                    else { g_previousMusicVolume = g_musicVolume.load(); g_muted = true; g_musicVolume = 0; }
                    continue;
                }

                if (showConfirm) {
                    if (btn == BTN_A) {
                        if (confirmIndex >= 0 && confirmIndex < (int)g_apps.size()) {
                            if (uninstallAppInternal(g_apps[confirmIndex])) {
                                g_apps.erase(g_apps.begin() + confirmIndex);
                                totalPages = ((int)g_apps.size() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
                                if (totalPages == 0) totalPages = 1;
                                clampSelection();
                            }
                        }
                        showConfirm = false;
                        confirmIndex = -1;
                    } else if (btn == BTN_B) {
                        showConfirm = false;
                        confirmIndex = -1;
                    }
                    continue;
                }

                if (btn == BTN_B) quit = true;
                else if (btn == BTN_DPUP) {
                    if (selected > 0) selected--;
                    else selected = (int)g_apps.size() - 1;
                    clampSelection();
                }
                else if (btn == BTN_DPDOWN) {
                    if (selected < (int)g_apps.size() - 1) selected++;
                    else selected = 0;
                    clampSelection();
                }
                else if (btn == BTN_L1 || btn == BTN_DPLEFT) {
                    if (currentPage > 0) { currentPage--; selected = currentPage * ITEMS_PER_PAGE; clampSelection(); }
                }
                else if (btn == BTN_R1 || btn == BTN_DPRIGHT) {
                    if (currentPage < totalPages - 1) { currentPage++; selected = currentPage * ITEMS_PER_PAGE; clampSelection(); }
                }
                else if (btn == BTN_A) {
                    if (selected >= 0 && selected < (int)g_apps.size() && !g_apps[selected].binary_path.empty()) {
                        g_stop = true;
                        if (g_bgAudioThread.joinable()) g_bgAudioThread.join();
                        SDL_CloseAudioDevice(g_audio_dev);
                        SDL_CloseAudioDevice(g_effect_audio);
                        SDL_DestroyRenderer(ren);
                        SDL_DestroyWindow(win);
                        SDL_Quit();
                        execl(g_apps[selected].binary_path.c_str(), g_apps[selected].binary_path.c_str(), (char*)nullptr);
                        perror("execl failed");
                        exit(1);
                    }
                }
                else if (btn == BTN_X) {
                    if (selected >= 0 && selected < (int)g_apps.size() && g_apps[selected].title != "No apps installed") {
                        showConfirm = true;
                        confirmIndex = selected;
                    }
                }
            }
        }

        if (now - lastBatteryCheck >= 2000) {
            lastBatteryCheck = now;
            int percent = getBatteryPercent();
            bool charging = isCharging();
            if (percent != lastPercent || charging != lastCharging) {
                lastPercent = percent; lastCharging = charging;
                if (percentTex) { SDL_DestroyTexture(percentTex); percentTex = nullptr; }
                if (percent >= 0) {
                    std::string text = std::to_string(percent);
                    percentTex = renderStringToTextureColored(text, ren, 255,255,255, &percentW, &percentH);
                }
            }
        }

        clampSelection();

        SDL_RenderClear(ren);
        if (bgTex) { SDL_Rect full = {0,0,win_w,win_h}; SDL_RenderCopy(ren, bgTex, nullptr, &full); }
        else { SDL_SetRenderDrawColor(ren, 22,22,33,255); SDL_RenderClear(ren); }

        if (topBarTex) { SDL_Rect topRect = {0,0,win_w,HEADER_HEIGHT}; SDL_RenderCopy(ren, topBarTex, nullptr, &topRect); }
        else { SDL_SetRenderDrawColor(ren, 40,40,65,255); SDL_Rect topRect = {0,0,win_w,HEADER_HEIGHT}; SDL_RenderFillRect(ren, &topRect); }

        const float scaleX = (float)win_w / 1024.0f;
        const float scaleY = (float)win_h / 600.0f;
        const float batteryPercentX = 921.7f, batteryPercentY = 21.2f, batteryPercentW = 33.3f, batteryPercentH = 16.1f;
        const float battIconX = 961.7f, battIconY = 13.9f, battIconW = 49.0f, battIconH = 29.0f;
        if (percentTex) { SDL_Rect dst = {(int)(batteryPercentX*scaleX), (int)(batteryPercentY*scaleY), (int)(batteryPercentW*scaleX), (int)(batteryPercentH*scaleY)}; SDL_RenderCopy(ren, percentTex, nullptr, &dst); }
        SDL_Texture* battTex = nullptr;
        if (lastCharging) battTex = chargingTex;
        else if (lastPercent >= 0) { int idx = (lastPercent >= 75) ? 3 : (lastPercent >= 50 ? 2 : (lastPercent >= 25 ? 1 : 0)); battTex = batteryTex[idx]; }
        if (battTex) { SDL_Rect dst = {(int)(battIconX*scaleX), (int)(battIconY*scaleY), (int)(battIconW*scaleX), (int)(battIconH*scaleY)}; SDL_RenderCopy(ren, battTex, nullptr, &dst); }

        std::string headerText = "Joystick Go Manager";
        int headerTextX = (win_w - (int)(headerText.length() * 10 * 1.1f)) / 2;
        drawText(ren, headerText, headerTextX, HEADER_HEIGHT/2 - 12, 1.1f, 220,220,255);

        char pageStr[32];
        snprintf(pageStr, sizeof(pageStr), "Page %d/%d", currentPage+1, totalPages);
        drawText(ren, pageStr, 20, HEADER_HEIGHT/2 - 6, 0.9f, 180,180,220);

        int pageStart = currentPage * ITEMS_PER_PAGE;
        int pageEnd = std::min(pageStart + ITEMS_PER_PAGE, (int)g_apps.size());
        int y = LIST_TOP;
        for (int idx = pageStart; idx < pageEnd; ++idx) {
            std::string display = g_apps[idx].title;
            if (!g_apps[idx].version.empty()) display += " v" + g_apps[idx].version;
            if (!g_apps[idx].author.empty()) display += " | " + g_apps[idx].author;

            if (idx == selected) {
                SDL_SetRenderDrawColor(ren, 70, 70, 140, 200);
                SDL_Rect bar = {LIST_LEFT, y - 2, LIST_RIGHT - LIST_LEFT, 32};
                SDL_RenderFillRect(ren, &bar);
                SDL_SetRenderDrawColor(ren, 120, 120, 200, 220);
                SDL_RenderDrawRect(ren, &bar);
                drawText(ren, display, LIST_LEFT + 10, y + 4, 1.2f, 255,255,255);
            } else {
                drawText(ren, display, LIST_LEFT + 10, y + 4, 1.1f, 220,220,220);
            }
            y += 34;
        }

        int detailY = win_h - DETAILS_HEIGHT - FOOTER_HEIGHT - 4;
        drawRect(ren, 8, detailY, win_w - 16, DETAILS_HEIGHT, 30,30,50,220, true, 90,90,140,180);
        drawText(ren, "Information", 20, detailY + 6, 0.8f, 180,180,220);

        if (selected >= 0 && selected < (int)g_apps.size() && !showConfirm) {
            App& app = g_apps[selected];
            int lineY = detailY + 28;
            drawText(ren, "Title: " + app.title, 20, lineY, 0.9f, 255,255,180);
            lineY += 24;
            if (!app.author.empty()) drawText(ren, "Author: " + app.author, 20, lineY, 0.8f, 180,220,255);
            lineY += 24;
            if (!app.version.empty()) drawText(ren, "Version: " + app.version, 20, lineY, 0.8f, 180,255,180);
            lineY += 24;
            std::string desc = app.description; if (desc.length() > 80) desc = desc.substr(0,77)+"...";
            drawText(ren, "Desc: " + desc, 20, lineY, 0.7f, 200,200,200);
        }

        int footerY = win_h - FOOTER_HEIGHT;
        drawRect(ren, 0, footerY, win_w, FOOTER_HEIGHT, 25,25,45,200, true, 70,70,110,200);
        drawText(ren, "A: Launch    X: Uninstall    B: Quit    L/R: Page    SELECT: Mute", 20, footerY+4, 0.7f, 160,160,180);

        if (showConfirm && confirmIndex >= 0 && confirmIndex < (int)g_apps.size()) {
            SDL_SetRenderDrawColor(ren, 0,0,0,180); SDL_Rect overlay = {0,0,win_w,win_h}; SDL_RenderFillRect(ren, &overlay);
            int boxW=600, boxH=120; int boxX=(win_w-boxW)/2, boxY=(win_h-boxH)/2;
            drawRect(ren, boxX, boxY, boxW, boxH, 50,50,85,255, true, 160,160,220,200);
            drawText(ren, "Uninstall \"" + g_apps[confirmIndex].title + "\"?", boxX+20, boxY+20, 1.1f, 255,255,200);
            drawText(ren, "Press A to confirm, B to cancel", boxX+20, boxY+60, 0.8f, 200,200,200);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    g_stop = true;
    if (g_bgAudioThread.joinable()) g_bgAudioThread.join();
    SDL_CloseAudioDevice(g_audio_dev);
    SDL_CloseAudioDevice(g_effect_audio);
    if (g_fontTexture) SDL_DestroyTexture(g_fontTexture);
    if (bgTex) SDL_DestroyTexture(bgTex);
    if (topBarTex) SDL_DestroyTexture(topBarTex);
    for (int i=0;i<4;i++) if (batteryTex[i]) SDL_DestroyTexture(batteryTex[i]);
    if (chargingTex) SDL_DestroyTexture(chargingTex);
    if (percentTex) SDL_DestroyTexture(percentTex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}