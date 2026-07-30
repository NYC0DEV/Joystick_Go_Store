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
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <vector>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <unistd.h>
#include <sstream>
#include <cctype>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define BTN_X        0
#define BTN_A        1
#define BTN_L1       2
#define BTN_B        3
#define BTN_Y        4
#define BTN_R1       5
#define BTN_SELECT   6
#define BTN_START    7
#define BTN_GUIDE    10
#define BTN_VOL_DOWN 14
#define BTN_VOL_UP   16
#define BTN_DPUP     29
#define BTN_DPLEFT   30
#define BTN_DPRIGHT  31
#define BTN_DPDOWN   32

static SDL_AudioDeviceID g_audio = 0;
static SDL_AudioDeviceID g_effect_audio = 0;
static std::atomic<bool> g_stop{false};
static std::atomic<int> g_volume{100};
static std::atomic<int> g_musicVolume{100};
static int g_previousMusicVolume = 100;
static std::atomic<bool> g_launching{false};
static bool g_skipIntro = false;

static bool g_muted = false;
static std::string g_currentMusicPath;

static Uint32 g_lastEraseRepeat = 0;
static const Uint32 REPEAT_INITIAL_DELAY = 300;
static const Uint32 REPEAT_INTERVAL = 150;

static Uint32 g_selectDownTime = 0;
static bool g_selectHeld = false;
static const Uint32 LONG_PRESS_MS = 500;

const std::string SETTINGS_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/settings.txt";
const std::string SPLASH_IMAGE_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/splashscreen3.png";
const std::string SPLASH_AUDIO_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/music/Intro.mp3";
const std::string BG_MUSIC_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/music/Music.mp3";
const std::string BG_MUSIC2_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/music/Music2.mp3";
const std::string CLICK_SOUND_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/sounds/click.ogg";
const std::string BACK_SOUND_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/sounds/back.ogg";
const std::string KEYBOARD_CLICK_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/sounds/keyboard/click.ogg";
const std::string KEYBOARD_ERASE_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/sounds/keyboard/erase.ogg";
const std::string KEYBOARD_MOVE_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/sounds/keyboard/move.ogg";
const std::string BACKGROUND2_IMAGE_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/bg/background2.png";
const std::string TOP_BAR_IMAGE_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/top.png";
const std::string BACKGROUND_FIND_IMAGE_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/bg/background_find.png";

const std::string BATTERY_ICON_BASE = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/battery/";

const std::string DOWN_ICON_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/down.png";
const std::string CHECK_ICON_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/check.png";

const std::string UP_ICON_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/up.png";

const std::string CLOCK_ICON_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/clock.png";

const std::string ICON_BASE = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/";
const std::vector<std::string> ICON_PATHS = {
    ICON_BASE + "find.png",
    ICON_BASE + "apps.png",
    ICON_BASE + "tools.png",
    ICON_BASE + "games.png",
    ICON_BASE + "emu.png"
};

const std::string GAME_ICON_BASE = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/apps/";
const std::string BLANK_ICON_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/apps/blank2.png";

const std::string FIND_ICON_BASE = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/find/";
const std::string FIND_NORMAL_BASE = FIND_ICON_BASE + "normal/";
const std::string FIND_SELECT_BASE = FIND_ICON_BASE + "select/";

const std::string DOWNLOAD_BASE = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/download/";
const std::string DOWNLOAD_BACK_PATH = DOWNLOAD_BASE + "back.png";
const std::string DOWNLOAD_BORDER_PATH = DOWNLOAD_BASE + "border.png";
const std::string DOWNLOAD_BACK1_PATH = DOWNLOAD_BASE + "back1.png";
const std::string DOWNLOAD_DES_PATH = DOWNLOAD_BASE + "des.png";
const std::string DOWNLOAD_TITLE_PATH = DOWNLOAD_BASE + "title.png";
const std::string DOWNLOAD_AUTHOR_PATH = DOWNLOAD_BASE + "author.png";
const std::string DOWNLOAD_SIZE_PATH = DOWNLOAD_BASE + "size.png";
const std::string DOWNLOAD_VER_PATH = DOWNLOAD_BASE + "ver.png";
const std::string DOWNLOAD_DATE_PATH = DOWNLOAD_BASE + "date.png";
const std::string DOWNLOAD_BUTTON_PATH = DOWNLOAD_BASE + "download.png";
const std::string DOWNLOAD_REMOVE_PATH = DOWNLOAD_BASE + "remove.png";
const std::string DOWNLOAD_BACKONE_PATH = DOWNLOAD_BASE + "backone.png";
const std::string DOWNLOAD_INSTALL_PATH = DOWNLOAD_BASE + "install.png";
const std::string DOWNLOAD_INSTALL_TEXT_PATH = DOWNLOAD_BASE + "install_text.png";
const std::string DOWNLOAD_LAUNCH_PATH = DOWNLOAD_BASE + "launch.png";

const std::string DOWNLOAD_INSTALL_TEXT2_PATH = DOWNLOAD_BASE + "install_text2.png";

const std::string DOWNLOAD_UPDATE_PATH = DOWNLOAD_BASE + "update.png";
const std::string DOWNLOAD_UPDATING_TEXT_PATH = DOWNLOAD_BASE + "updating.png";

const std::string CATEGORY_LABEL_PATH = DOWNLOAD_BASE + "category.png";

const std::string PAGES_ICON_PATH = FIND_ICON_BASE + "icon/pages.png";
const std::string BACK1_ICON_PATH = ICON_BASE + "back1.png";
const std::string BACK_RESIZE2_PATH = ICON_BASE + "back_resize2.png";

const std::string SELECT_OVERLAY_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/apps/select.png";

const std::string INSTALLED_LIST_PATH = "/mnt/sdcard/bin/Joystick_Go_Store/installed.txt";

static std::vector<int16_t> g_clickSamples;
static std::vector<int16_t> g_backSamples;
static std::vector<int16_t> g_keyboardClickSamples;
static std::vector<int16_t> g_keyboardEraseSamples;
static std::vector<int16_t> g_keyboardMoveSamples;

static SDL_Window* g_win = nullptr;
static SDL_Renderer* g_ren = nullptr;
static std::thread g_bgAudioThread;

struct StoreGame {
    int id;
    std::string title;
    std::string version;
    std::string description;
    std::string author;
    std::string category;
    std::string image_data;
    std::string created_at;
    std::string file_size;
    std::string install_folder;
    std::string binary_path;
};

static std::shared_mutex g_storeMutex;
static std::vector<StoreGame> g_storeGames;
static std::vector<SDL_Texture*> g_gameTextures;
static std::vector<bool> g_installedGames;
static std::vector<bool> g_needUpdate;
static std::vector<std::string> g_installedVersion;

struct CachedImage {
    std::string title;
    std::vector<unsigned char> pngData;
    SDL_Texture* texture;
    bool loaded;
};

static std::vector<CachedImage> g_imageCache;

static const unsigned char ASCII_FONT[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},
    {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00},
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00},
    {0x18,0x7E,0x60,0x3C,0x06,0x7E,0x18,0x00},
    {0x00,0x66,0x0C,0x18,0x30,0x66,0x00,0x00},
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00},
    {0x18,0x18,0x10,0x00,0x00,0x00,0x00,0x00},
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00},
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},
    {0x00,0x18,0x18,0x7E,0x18,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x18,0x30,0x00},
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
    {0x00,0x06,0x0C,0x18,0x30,0x60,0x00,0x00},
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00},
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    {0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00},
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00},
    {0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00},
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00},
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00},
    {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00},
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00},
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00},
    {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
    {0x00,0x18,0x18,0x00,0x18,0x30,0x00,0x00},
    {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00},
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00},
    {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00},
    {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00},
    {0x3C,0x66,0x6E,0x6E,0x60,0x66,0x3C,0x00},
    {0x3C,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00},
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00},
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00},
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00},
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00},
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00},
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    {0x06,0x06,0x06,0x06,0x06,0x66,0x3C,0x00},
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00},
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00},
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00},
    {0x66,0x76,0x7E,0x6E,0x66,0x66,0x66,0x00},
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00},
    {0x3C,0x66,0x66,0x66,0x66,0x6E,0x3C,0x00},
    {0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x00},
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00},
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00},
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00},
    {0x66,0x66,0x3C,0x18,0x18,0x18,0x18,0x00},
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00},
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00},
    {0x00,0x60,0x30,0x18,0x0C,0x06,0x00,0x00},
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00},
    {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00},
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00},
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00},
    {0x00,0x00,0x3C,0x60,0x60,0x60,0x3C,0x00},
    {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00},
    {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00},
    {0x1C,0x30,0x30,0x7C,0x30,0x30,0x30,0x00},
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x3C},
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00},
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00},
    {0x06,0x00,0x06,0x06,0x06,0x06,0x3C,0x00},
    {0x60,0x60,0x6C,0x78,0x78,0x6C,0x66,0x00},
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    {0x00,0x00,0x6C,0x7E,0x6B,0x63,0x63,0x00},
    {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00},
    {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00},
    {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60},
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06},
    {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00},
    {0x00,0x00,0x3C,0x60,0x3C,0x06,0x3C,0x00},
    {0x30,0x30,0x7C,0x30,0x30,0x30,0x1C,0x00},
    {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00},
    {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00},
    {0x00,0x00,0x63,0x63,0x6B,0x7F,0x3C,0x00},
    {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00},
    {0x00,0x00,0x66,0x66,0x3E,0x06,0x3C,0x00},
    {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00},
    {0x0C,0x18,0x18,0x30,0x18,0x18,0x0C,0x00},
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    {0x30,0x18,0x18,0x0C,0x18,0x18,0x30,0x00},
    {0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00}
};

static void loadSettings() {
    std::ifstream file(SETTINGS_PATH);
    if (!file.is_open()) {
        std::ofstream out(SETTINGS_PATH);
        if (out.is_open()) {
            out << "skip intro false\n";
            out.close();
        }
        g_skipIntro = false;
        printf("Settings: created default file, skip intro = false\n");
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        std::string lowerLine = line;
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);
        if (lowerLine.find("skip intro") != std::string::npos) {
            size_t tokenPos = lowerLine.find("skip intro");
            size_t valueStart = tokenPos + 10;
            while (valueStart < lowerLine.length() && (lowerLine[valueStart] == ' ' || lowerLine[valueStart] == '\t')) {
                valueStart++;
            }
            if (valueStart < lowerLine.length()) {
                size_t valueEnd = lowerLine.find_first_of(" \t", valueStart);
                if (valueEnd == std::string::npos) valueEnd = lowerLine.length();
                std::string value = lowerLine.substr(valueStart, valueEnd - valueStart);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                if (value == "true" || value == "1") {
                    g_skipIntro = true;
                } else if (value == "false" || value == "0") {
                    g_skipIntro = false;
                } else {
                    g_skipIntro = false;
                }
                printf("Settings: skip intro = %s\n", g_skipIntro ? "true" : "false");
            } else {
                g_skipIntro = false;
                printf("Settings: no value after skip intro, default false\n");
            }
        }
    }
    file.close();
}

static int getBatteryPercent() {
    std::ifstream f("/sys/class/power_supply/battery/capacity");
    if (!f.is_open()) return -1;
    int cap;
    f >> cap;
    return cap;
}

static bool isCharging() {
    std::ifstream f("/sys/class/power_supply/battery/status");
    if (!f.is_open()) return false;
    std::string status;
    f >> status;
    return (status == "Charging" || status == "Full");
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
                if (cn > 0) {
                    buffer.insert(buffer.end(), b, b + cn * ac->channels);
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
    return !buffer.empty();
}

static void playSound(const std::vector<int16_t>& buffer) {
    if (buffer.empty()) return;
    const size_t maxSamples = 1764;
    size_t samplesToUse = std::min(buffer.size(), maxSamples);
    int vol = g_volume.load();
    if (vol == 100) {
        SDL_QueueAudio(g_effect_audio, buffer.data(), samplesToUse * sizeof(int16_t));
    } else {
        static std::vector<int16_t> scaled;
        scaled.resize(samplesToUse);
        for (size_t i = 0; i < samplesToUse; ++i) {
            int32_t s = buffer[i];
            s = (s * vol) / 100;
            if (s > 32767) s = 32767;
            else if (s < -32768) s = -32768;
            scaled[i] = (int16_t)s;
        }
        SDL_QueueAudio(g_effect_audio, scaled.data(), samplesToUse * sizeof(int16_t));
    }
}

static void playIntroAudioThread(std::string path) {
    AVFormatContext* fc = avformat_alloc_context();
    fc->interrupt_callback.callback = [](void*) -> int { return g_stop.load() ? 1 : 0; };
    if (avformat_open_input(&fc, path.c_str(), nullptr, nullptr) != 0 || !fc) return;
    avformat_find_stream_info(fc, nullptr);
    int ai = av_find_best_stream(fc, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (ai < 0) { avformat_close_input(&fc); return; }
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
    while (!g_stop) {
        while (!g_stop && SDL_GetQueuedAudioSize(g_audio) > MAX_AQ) SDL_Delay(5);
        if (g_stop) break;
        if (av_read_frame(fc, pkt) < 0) break;
        if (pkt->stream_index == ai) {
            avcodec_send_packet(ac, pkt);
            while (!g_stop) {
                if (avcodec_receive_frame(ac, frm) < 0) break;
                int ns = av_rescale_rnd(swr_get_delay(swr, ac->sample_rate) + frm->nb_samples, 44100, ac->sample_rate, AV_ROUND_UP);
                int16_t* b = (int16_t*)av_malloc(ns * 4);
                uint8_t* od[1] = { (uint8_t*)b };
                int cn = swr_convert(swr, od, ns, (const uint8_t**)frm->data, frm->nb_samples);
                if (cn > 0) {
                    int current_vol = g_musicVolume.load();
                    if (current_vol != 100) {
                        for (int i = 0; i < cn * ac->channels; ++i) {
                            int32_t s = b[i];
                            s = (s * current_vol) / 100;
                            if (s > 32767) s = 32767;
                            else if (s < -32768) s = -32768;
                            b[i] = (int16_t)s;
                        }
                    }
                    SDL_QueueAudio(g_audio, b, cn * 4);
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
}

static void playAudioLoop(const std::string& path) {
    while (!g_stop) {
        AVFormatContext* fc = avformat_alloc_context();
        fc->interrupt_callback.callback = [](void*) -> int { return g_stop.load() ? 1 : 0; };
        if (avformat_open_input(&fc, path.c_str(), nullptr, nullptr) != 0 || !fc) {
            SDL_Delay(100);
            continue;
        }
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
            while (!g_stop && SDL_GetQueuedAudioSize(g_audio) > MAX_AQ) SDL_Delay(5);
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
                        int current_vol = g_musicVolume.load();
                        if (current_vol != 100) {
                            for (int i = 0; i < cn * ac->channels; ++i) {
                                int32_t s = b[i];
                                s = (s * current_vol) / 100;
                                if (s > 32767) s = 32767;
                                else if (s < -32768) s = -32768;
                                b[i] = (int16_t)s;
                            }
                        }
                        SDL_QueueAudio(g_audio, b, cn * 4);
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

static SDL_Texture* renderStringToTexture(const std::string& text, SDL_Renderer* ren,
                                          int* outW, int* outH) {
    if (text.empty()) return nullptr;
    const int CHAR_W = 8, CHAR_H = 8, SPACING = 1;
    int totalW = text.size() * (CHAR_W + SPACING) - SPACING;
    int totalH = CHAR_H;
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, totalW, totalH, 32, SDL_PIXELFORMAT_ABGR8888);
    if (!surf) return nullptr;
    SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 0,0,0,0));
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char ch = (unsigned char)text[i];
        if (ch < 32 || ch > 126) continue;
        const unsigned char* glyph = ASCII_FONT[ch - 32];
        int x = i * (CHAR_W + SPACING);
        for (int row = 0; row < CHAR_H; ++row) {
            unsigned char byte = glyph[row];
            for (int col = 0; col < CHAR_W; ++col) {
                if (byte & (0x80 >> col)) {
                    Uint32* pixels = (Uint32*)surf->pixels;
                    int pitch = surf->pitch / 4;
                    pixels[row * pitch + x + col] = 0xFFFFFFFF;
                }
            }
        }
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    if (outW) *outW = surf->w;
    if (outH) *outH = surf->h;
    SDL_FreeSurface(surf);
    return tex;
}

static SDL_Texture* renderStringToTextureColored(const std::string& text, SDL_Renderer* ren,
                                                 Uint8 r, Uint8 g, Uint8 b,
                                                 int* outW, int* outH) {
    if (text.empty()) return nullptr;
    const int CHAR_W = 8, CHAR_H = 8, SPACING = 1;
    int totalW = text.size() * (CHAR_W + SPACING) - SPACING;
    int totalH = CHAR_H;
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, totalW, totalH, 32, SDL_PIXELFORMAT_ABGR8888);
    if (!surf) return nullptr;
    SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 0,0,0,0));
    Uint32 color = SDL_MapRGBA(surf->format, r, g, b, 255);
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char ch = (unsigned char)text[i];
        if (ch < 32 || ch > 126) continue;
        const unsigned char* glyph = ASCII_FONT[ch - 32];
        int x = i * (CHAR_W + SPACING);
        for (int row = 0; row < CHAR_H; ++row) {
            unsigned char byte = glyph[row];
            for (int col = 0; col < CHAR_W; ++col) {
                if (byte & (0x80 >> col)) {
                    Uint32* pixels = (Uint32*)surf->pixels;
                    int pitch = surf->pitch / 4;
                    pixels[row * pitch + x + col] = color;
                }
            }
        }
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    if (outW) *outW = surf->w;
    if (outH) *outH = surf->h;
    SDL_FreeSurface(surf);
    return tex;
}

static void drawTextWithAspect(SDL_Renderer* ren, const std::string& text,
                               int x, int y, int w, int h,
                               Uint8 r = 255, Uint8 g = 255, Uint8 b = 255) {
    if (text.empty() || w <= 0 || h <= 0) return;
    SDL_Texture* tex = renderStringToTextureColored(text, ren, r, g, b, nullptr, nullptr);
    if (!tex) return;
    int tw, th;
    SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
    float aspect = (float)tw / th;
    float targetAspect = (float)w / h;
    float scale;
    if (aspect > targetAspect) {
        scale = (float)w / tw;
    } else {
        scale = (float)h / th;
    }
    int newW = (int)(tw * scale);
    int newH = (int)(th * scale);
    int offX = (w - newW) / 2;
    int offY = (h - newH) / 2;
    SDL_Rect dst = { x + offX, y + offY, newW, newH };
    SDL_RenderCopy(ren, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

static int drawWrappedText(SDL_Renderer* ren, const std::string& text,
                           int startX, int startY, int maxWidth, int maxHeight,
                           float scale, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255) {
    if (text.empty()) return 0;
    std::vector<std::string> words;
    size_t pos = 0;
    while (pos < text.length()) {
        size_t end = text.find(' ', pos);
        if (end == std::string::npos) end = text.length();
        std::string word = text.substr(pos, end - pos);
        if (!word.empty()) words.push_back(word);
        pos = end + 1;
    }

    const int CHAR_W = 8, CHAR_H = 8, SPACING = 1;
    int lineHeight = (int)((CHAR_H + SPACING) * scale);
    int charWidth = (int)((CHAR_W + SPACING) * scale);

    std::vector<std::string> lines;
    std::string currentLine;
    for (const auto& word : words) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        int testWidth = (int)(testLine.length() * charWidth);
        if (testWidth > maxWidth && !currentLine.empty()) {
            lines.push_back(currentLine);
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);

    int totalLines = lines.size();
    int totalHeight = totalLines * lineHeight;
    if (totalHeight > maxHeight) {
        totalLines = maxHeight / lineHeight;
        if (totalLines < 1) totalLines = 1;
        totalHeight = totalLines * lineHeight;
    }

    int yOffset = 0;
    for (int i = 0; i < totalLines; ++i) {
        const std::string& line = lines[i];
        SDL_Texture* tex = renderStringToTextureColored(line, ren, r, g, b, nullptr, nullptr);
        if (tex) {
            int tw = (int)(line.length() * charWidth);
            int th = (int)(CHAR_H * scale);
            if (tw > maxWidth) tw = maxWidth;
            SDL_Rect dst = { startX, startY + yOffset, tw, th };
            SDL_RenderCopy(ren, tex, nullptr, &dst);
            SDL_DestroyTexture(tex);
        }
        yOffset += lineHeight;
    }
    return totalHeight;
}

static void renderTextureWithAspectAndClip(SDL_Renderer* ren, SDL_Texture* tex, const SDL_Rect* dst) {
    if (!tex || !dst) return;
    int texW, texH;
    SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
    if (texW <= 0 || texH <= 0) return;
    float aspect = (float)texW / texH;
    float dstAspect = (float)dst->w / dst->h;
    SDL_Rect renderRect = *dst;
    if (aspect > dstAspect) {
        renderRect.w = (int)(dst->h * aspect);
        renderRect.x = dst->x + (dst->w - renderRect.w) / 2;
    } else {
        renderRect.h = (int)(dst->w / aspect);
        renderRect.y = dst->y + (dst->h - renderRect.h) / 2;
    }
    SDL_Rect clipRect = *dst;
    SDL_RenderSetClipRect(ren, &clipRect);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(ren, tex, NULL, &renderRect);
    SDL_RenderSetClipRect(ren, NULL);
}

enum Category { CAT_GAME, CAT_EMULATOR, CAT_TOOL };

static Category getCategory(const std::string& catStr) {
    if (catStr == "game") return CAT_GAME;
    if (catStr == "emulator") return CAT_EMULATOR;
    if (catStr == "tool") return CAT_TOOL;
    return CAT_GAME;
}

static std::string getCategoryName(Category cat) {
    switch(cat) {
        case CAT_GAME: return "Game";
        case CAT_EMULATOR: return "Emulator";
        case CAT_TOOL: return "Tool";
        default: return "";
    }
}

static void updateInstalledStatus() {
    std::unique_lock<std::shared_mutex> lock(g_storeMutex);
    g_installedGames.resize(g_storeGames.size());
    g_needUpdate.resize(g_storeGames.size());
    g_installedVersion.resize(g_storeGames.size());
    for (size_t i = 0; i < g_storeGames.size(); ++i) {
        std::string installFolder = g_storeGames[i].install_folder;
        if (installFolder.empty()) {
            installFolder = "/mnt/sdcard/bin/Joystick_Go_Store/games/" + g_storeGames[i].title + "/";
        }
        std::string versionFile = installFolder + ".version";
        struct stat st;
        if (stat(versionFile.c_str(), &st) == 0 && (st.st_mode & S_IFREG)) {
            g_installedGames[i] = true;
            std::ifstream vf(versionFile);
            std::string installedVer;
            if (vf.is_open()) {
                std::getline(vf, installedVer);
                vf.close();
            }
            g_installedVersion[i] = installedVer;
            if (installedVer != g_storeGames[i].version) {
                g_needUpdate[i] = true;
            } else {
                g_needUpdate[i] = false;
            }
        } else {
            g_installedGames[i] = false;
            g_needUpdate[i] = false;
            g_installedVersion[i].clear();
        }
    }
}

static void writeInstalledList() {
    std::unique_lock<std::shared_mutex> lock(g_storeMutex);
    std::ofstream out(INSTALLED_LIST_PATH);
    if (!out.is_open()) return;
    for (size_t i = 0; i < g_storeGames.size(); ++i) {
        if (g_installedGames[i]) {
            out << g_storeGames[i].title << "|"
                << g_storeGames[i].install_folder << "|"
                << g_storeGames[i].binary_path << "|"
                << g_storeGames[i].description << "|"
                << g_storeGames[i].author << "|"
                << g_storeGames[i].category << "|"
                << g_storeGames[i].file_size << "|"
                << g_storeGames[i].created_at << "|"
                << g_storeGames[i].version << "\n";
        }
    }
    out.close();
}

static void loadInstalledAppsFromFile(SDL_Texture* blankTex) {
    std::ifstream in(INSTALLED_LIST_PATH);
    if (!in.is_open()) return;
    std::string line;
    std::vector<StoreGame> installedGames;
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
        StoreGame game;
        game.id = -1;
        game.title = fields[0];
        game.install_folder = fields[1];
        game.binary_path = fields[2];
        if (fields.size() >= 4) game.description = fields[3];
        if (fields.size() >= 5) game.author = fields[4];
        if (fields.size() >= 6) game.category = fields[5];
        if (fields.size() >= 7) game.file_size = fields[6];
        if (fields.size() >= 8) game.created_at = fields[7];
        if (fields.size() >= 9) game.version = fields[8];
        else {
            std::string installFolder = game.install_folder;
            if (installFolder.empty()) {
                installFolder = "/mnt/sdcard/bin/Joystick_Go_Store/games/" + game.title + "/";
            }
            std::string versionFile = installFolder + ".version";
            std::ifstream vf(versionFile);
            if (vf.is_open()) {
                std::getline(vf, game.version);
                vf.close();
            }
        }
        game.image_data = "";
        installedGames.push_back(game);
    }
    in.close();
    if (!installedGames.empty()) {
        std::unique_lock<std::shared_mutex> lock(g_storeMutex);
        g_storeGames = std::move(installedGames);
        for (auto tex : g_gameTextures) {
            if (tex && tex != blankTex) {
                SDL_DestroyTexture(tex);
            }
        }
        g_gameTextures.clear();
        g_gameTextures.resize(g_storeGames.size(), blankTex);
        g_installedGames.assign(g_storeGames.size(), true);
        g_needUpdate.assign(g_storeGames.size(), false);
        g_installedVersion.assign(g_storeGames.size(), "");
        for (size_t i = 0; i < g_storeGames.size(); ++i) {
            if (g_storeGames[i].version.empty()) {
                std::string installFolder = g_storeGames[i].install_folder;
                if (installFolder.empty()) {
                    installFolder = "/mnt/sdcard/bin/Joystick_Go_Store/games/" + g_storeGames[i].title + "/";
                }
                std::string versionFile = installFolder + ".version";
                std::ifstream vf(versionFile);
                if (vf.is_open()) {
                    std::string ver;
                    std::getline(vf, ver);
                    g_installedVersion[i] = ver;
                    g_storeGames[i].version = ver;
                    vf.close();
                }
            } else {
                g_installedVersion[i] = g_storeGames[i].version;
            }
        }
    }
}

static void launchGame(int gameIndex) {
    if (g_launching.exchange(true)) return;

    std::string binaryPath;
    {
        std::shared_lock<std::shared_mutex> lock(g_storeMutex);
        if (gameIndex >= 0 && gameIndex < (int)g_storeGames.size()) {
            binaryPath = g_storeGames[gameIndex].binary_path;
        }
    }
    if (binaryPath.empty()) {
        printf("No binary path for game %d\n", gameIndex);
        g_launching = false;
        return;
    }

    g_stop = true;
    if (g_bgAudioThread.joinable()) {
        g_bgAudioThread.join();
    }

    SDL_CloseAudioDevice(g_effect_audio);
    SDL_CloseAudioDevice(g_audio);
    SDL_DestroyRenderer(g_ren);
    SDL_DestroyWindow(g_win);
    SDL_Quit();

    execl(binaryPath.c_str(), binaryPath.c_str(), (char*)nullptr);

    perror("execl failed");
    exit(1);
}

static std::string base64_decode(const std::string& in) {
    static const std::string b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[(unsigned char)b64[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

static bool parseStoreResponse(const std::string& json, std::vector<StoreGame>& outGames) {
    printf("Parsing JSON response (first 200 chars):\n%.200s...\n", json.c_str());
    outGames.clear();

    size_t firstNonSpace = json.find_first_not_of(" \t\n\r");
    if (firstNonSpace != std::string::npos && json[firstNonSpace] == '[') {
        int depth = 0;
        bool emptyArray = true;
        for (size_t i = firstNonSpace; i < json.size(); ++i) {
            if (json[i] == '[') depth++;
            else if (json[i] == ']') {
                depth--;
                if (depth == 0) {
                    if (json.find_first_not_of(" \t\n\r[]", firstNonSpace+1) != std::string::npos) {
                        emptyArray = false;
                    }
                    break;
                }
            }
        }
        if (emptyArray) {
            printf("Store is empty ([]).\n");
            return true;
        }
    }

    size_t pos = 0;
    int parsedCount = 0;
    while ((pos = json.find('{', pos)) != std::string::npos) {
        size_t end = json.find('}', pos);
        if (end == std::string::npos) break;
        std::string obj = json.substr(pos, end - pos + 1);
        pos = end + 1;

        StoreGame game;
        auto getField = [&](const std::string& key) -> std::string {
            size_t k = obj.find("\"" + key + "\"");
            if (k == std::string::npos) return "";
            size_t colon = obj.find(':', k);
            if (colon == std::string::npos) return "";
            size_t start = obj.find_first_not_of(" \t\n\r", colon + 1);
            if (start == std::string::npos) return "";
            if (obj[start] == '"') {
                size_t endq = obj.find('"', start + 1);
                if (endq == std::string::npos) return "";
                return obj.substr(start + 1, endq - start - 1);
            } else {
                size_t endv = obj.find_first_of(",}", start);
                if (endv == std::string::npos) endv = obj.length();
                return obj.substr(start, endv - start);
            }
        };

        try {
            game.id = std::stoi(getField("id"));
        } catch (...) { continue; }
        game.title = getField("title");
        game.version = getField("version");
        game.description = getField("description");
        game.author = getField("display_name");
        game.category = getField("category");
        game.image_data = getField("image_data");
        game.created_at = getField("approved_at");
        game.install_folder = getField("install_folder");
        game.binary_path = getField("binary_path");
        std::string fs = getField("file_size");
        if (!fs.empty()) {
            long long bytes = std::stoll(fs);
            if (bytes < 1024) game.file_size = std::to_string(bytes) + " B";
            else if (bytes < 1024*1024) game.file_size = std::to_string(bytes/1024) + " KB";
            else game.file_size = std::to_string(bytes/(1024*1024)) + " MB";
        } else {
            game.file_size = "—";
        }

        if (!game.title.empty()) {
            outGames.push_back(game);
            parsedCount++;
        }
    }
    printf("Parsed %d games.\n", parsedCount);
    return true;
}

static bool fetchStoreData(std::vector<StoreGame>& outGames) {
    const char* url = "https://joystick-store-proxy.joystick-go-store.workers.dev/api/store";
    std::string result;

    const char* commands[] = {
        "/usr/bin/curl -s -k --max-time 10",
        "curl -s -k --max-time 10",
        "/usr/bin/wget -qO- --no-check-certificate --timeout=10",
        "wget -qO- --no-check-certificate --timeout=10"
    };

    for (const char* cmdBase : commands) {
        std::string cmd = std::string(cmdBase) + " \"" + url + "\"";
        printf("Trying: %s\n", cmd.c_str());
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            printf("Failed to run command.\n");
            continue;
        }
        result.clear();
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        int status = pclose(pipe);
        if (status == 0 && !result.empty()) {
            printf("Command succeeded (%zu bytes).\n", result.size());
            if (parseStoreResponse(result, outGames)) {
                printf("Parsed %zu games.\n", outGames.size());
                return true;
            } else {
                printf("JSON parse failed.\n");
                return false;
            }
        } else {
            printf("Command failed with status %d or empty result.\n", status);
        }
    }

    printf("All fetch attempts failed. Store is empty.\n");
    return false;
}

static bool decodeBase64ToPNG(const std::string& dataUrl, std::vector<unsigned char>& outData) {
    if (dataUrl.empty()) return false;

    size_t comma = dataUrl.find(',');
    if (comma == std::string::npos) return false;
    std::string base64 = dataUrl.substr(comma + 1);

    std::string decoded = base64_decode(base64);
    if (decoded.empty()) return false;

    outData.resize(decoded.size());
    memcpy(outData.data(), decoded.data(), decoded.size());
    return true;
}

static SDL_Texture* createTextureFromPNGData(const std::vector<unsigned char>& pngData, SDL_Renderer* ren) {
    if (pngData.empty()) return nullptr;

    SDL_RWops* rw = SDL_RWFromConstMem(pngData.data(), pngData.size());
    if (!rw) return nullptr;

    SDL_Surface* surf = SDL_LoadBMP_RW(rw, 1);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
        if (tex) {
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        }
        SDL_FreeSurface(surf);
        return tex;
    }

    const char* tempDir = "/dev/shm";
    struct stat st;
    if (stat(tempDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        tempDir = "/tmp";
    }
    char tempPath[256];
    snprintf(tempPath, sizeof(tempPath), "%s/store_img_XXXXXX", tempDir);
    int fd = mkstemp(tempPath);
    if (fd == -1) return nullptr;

    ssize_t written = write(fd, pngData.data(), pngData.size());
    close(fd);
    if (written != (ssize_t)pngData.size()) {
        unlink(tempPath);
        return nullptr;
    }

    SDL_Texture* tex = LoadImageFFmpeg(tempPath, ren);
    unlink(tempPath);
    return tex;
}

static SDL_Texture* loadTextureFromCache(const std::string& title, SDL_Renderer* ren, SDL_Texture* fallbackTex) {
    for (auto& cached : g_imageCache) {
        if (cached.title == title) {
            if (cached.loaded && cached.texture) {
                return cached.texture;
            }
            if (!cached.pngData.empty()) {
                cached.texture = createTextureFromPNGData(cached.pngData, ren);
                if (cached.texture) {
                    cached.loaded = true;
                    return cached.texture;
                }
            }
            break;
        }
    }
    return fallbackTex;
}

static void cacheGameImages(const std::vector<StoreGame>& games) {
    printf("Caching game images in RAM...\n");
    for (auto& cached : g_imageCache) {
        if (cached.texture) {
            cached.texture = nullptr;
        }
    }
    g_imageCache.clear();
    g_imageCache.reserve(games.size());

    int cachedCount = 0;
    for (size_t i = 0; i < games.size(); ++i) {
        const auto& game = games[i];
        CachedImage cached;
        cached.title = game.title;
        cached.loaded = false;
        cached.texture = nullptr;

        if (!game.image_data.empty()) {
            if (decodeBase64ToPNG(game.image_data, cached.pngData)) {
                cachedCount++;
                printf("Cached image for: %s (%zu bytes)\n", game.title.c_str(), cached.pngData.size());
            } else {
                printf("Failed to decode image for: %s\n", game.title.c_str());
            }
        }
        g_imageCache.push_back(std::move(cached));
    }
    printf("Cached %d/%zu images in RAM\n", cachedCount, games.size());
}

static std::vector<SDL_Texture*> loadGameTexturesFromCache(SDL_Renderer* ren, SDL_Texture* fallbackTex) {
    std::vector<SDL_Texture*> textures;
    textures.reserve(g_imageCache.size());
    for (auto& cached : g_imageCache) {
        textures.push_back(loadTextureFromCache(cached.title, ren, fallbackTex));
    }
    return textures;
}

static std::thread g_refreshThread;
static std::atomic<bool> g_refreshStop{false};
static std::atomic<bool> g_needRefresh{true};

static void refreshStoreData(SDL_Renderer* ren, SDL_Texture* blankTex) {
    std::vector<StoreGame> newGames;
    bool ok = fetchStoreData(newGames);
    if (!ok || newGames.empty()) {
        return;
    }

    std::unique_lock<std::shared_mutex> lock(g_storeMutex);
    g_storeGames = std::move(newGames);

    cacheGameImages(g_storeGames);
    for (auto tex : g_gameTextures) {
        if (tex && tex != blankTex) {
            SDL_DestroyTexture(tex);
        }
    }
    g_gameTextures = loadGameTexturesFromCache(g_ren, blankTex);

    g_installedGames.assign(g_storeGames.size(), false);
    g_needUpdate.assign(g_storeGames.size(), false);
    g_installedVersion.assign(g_storeGames.size(), "");
    lock.unlock();
    updateInstalledStatus();

    printf("Store data refreshed: %zu games\n", g_storeGames.size());
}

static void storeRefreshLoop(SDL_Renderer* ren, SDL_Texture* blankTex) {
    while (!g_refreshStop) {
        for (int i = 0; i < 1800 && !g_refreshStop; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (g_refreshStop) break;
        refreshStoreData(ren, blankTex);
    }
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
            if (S_ISDIR(st.st_mode)) {
                removeDirectory(fullPath);
            } else {
                unlink(fullPath.c_str());
            }
        }
    }
    closedir(dir);
    rmdir(path.c_str());
    return true;
}

static std::atomic<int> g_downloadStatus{0};
static std::atomic<bool> g_isUpdate{false};
static std::atomic<int> g_downloadProgress{0};
static std::thread g_downloadThread;
static Uint32 g_lastProgressUpdate = 0;

static bool downloadFile(const std::string& url, const std::string& destPath) {
    long long totalSize = 0;
    std::string headCmd = "curl -sI -k \"" + url + "\" | grep -i content-length | awk '{print $2}'";
    FILE* headPipe = popen(headCmd.c_str(), "r");
    if (headPipe) {
        char line[256];
        if (fgets(line, sizeof(line), headPipe)) {
            totalSize = std::stoll(line);
        }
        pclose(headPipe);
    }

    if (totalSize <= 0) {
        std::string cmd = "wget -q --no-check-certificate -O \"" + destPath + "\" \"" + url + "\"";
        int ret = system(cmd.c_str());
        if (ret == 0) {
            g_downloadProgress = 100;
            return true;
        }
        cmd = "curl -s -k -L -o \"" + destPath + "\" \"" + url + "\"";
        ret = system(cmd.c_str());
        if (ret == 0) {
            g_downloadProgress = 100;
            return true;
        }
        return false;
    }

    std::string cmd = "curl -s -k -L -o \"" + destPath + "\" \"" + url + "\" &";
    int ret = system(cmd.c_str());
    if (ret != 0) {
        cmd = "wget -q --no-check-certificate -O \"" + destPath + "\" \"" + url + "\" &";
        ret = system(cmd.c_str());
        if (ret != 0) {
            return false;
        }
    }

    while (true) {
        struct stat st;
        if (stat(destPath.c_str(), &st) == 0) {
            long long currentSize = st.st_size;
            int percent = (int)((currentSize * 100) / totalSize);
            if (percent > 100) percent = 100;
            g_downloadProgress = percent;
        }
        if (stat(destPath.c_str(), &st) == 0 && st.st_size >= totalSize) {
            g_downloadProgress = 100;
            break;
        }
        SDL_Delay(200);
        static int stuckCounter = 0;
        static long long lastSize = 0;
        if (st.st_size == lastSize) {
            stuckCounter++;
            if (stuckCounter > 50) {
                if (st.st_size > 0) {
                    g_downloadProgress = 100;
                } else {
                    g_downloadProgress = 0;
                }
                break;
            }
        } else {
            stuckCounter = 0;
            lastSize = st.st_size;
        }
    }

    return true;
}

static bool extractZip(const std::string& zipPath, const std::string& destDir) {
    std::string cmd = "unzip -o \"" + zipPath + "\" -d \"" + destDir + "\"";
    int result = system(cmd.c_str());
    return (result == 0);
}

static bool createDummyZip(const std::string& path) {
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return false;
    fclose(f);
    return true;
}

static void downloadAndInstallThread(int gameIndex) {
    StoreGame game;
    {
        std::shared_lock<std::shared_mutex> lock(g_storeMutex);
        if (gameIndex < 0 || gameIndex >= (int)g_storeGames.size()) {
            g_downloadStatus = 0;
            return;
        }
        game = g_storeGames[gameIndex];
    }

    std::string url = "https://joystick-store-proxy.joystick-go-store.workers.dev/api/store/download?id=" + std::to_string(game.id);
    std::string tempZip = "/tmp/joystick_store_" + std::to_string(game.id) + ".zip";

    if (!downloadFile(url, tempZip)) {
        printf("Download failed for game %d\n", gameIndex);
        g_downloadStatus = 0;
        g_downloadProgress = 0;
        return;
    }

    std::string installFolder = game.install_folder;
    if (installFolder.empty()) {
        installFolder = "/mnt/sdcard/bin/Joystick_Go_Store/games/" + game.title + "/";
    }
    std::string mkdirCmd = "mkdir -p \"" + installFolder + "\"";
    system(mkdirCmd.c_str());

    if (!extractZip(tempZip, installFolder)) {
        printf("Extraction failed for game %d\n", gameIndex);
        g_downloadStatus = 0;
        g_downloadProgress = 0;
        unlink(tempZip.c_str());
        return;
    }

    std::string versionFile = installFolder + ".version";
    std::ofstream vf(versionFile);
    if (vf.is_open()) {
        vf << game.version;
        vf.close();
    }

    unlink(tempZip.c_str());

    {
        std::unique_lock<std::shared_mutex> lock(g_storeMutex);
        if (gameIndex < (int)g_installedGames.size()) {
            g_installedGames[gameIndex] = true;
            g_needUpdate[gameIndex] = false;
            g_installedVersion[gameIndex] = game.version;
        }
    }
    writeInstalledList();

    g_downloadStatus = 3;
    g_isUpdate = false;
    g_downloadProgress = 100;
    printf("Install finished for game %d\n", gameIndex);
}

static bool removeGameFiles(int gameIndex) {
    StoreGame game;
    {
        std::shared_lock<std::shared_mutex> lock(g_storeMutex);
        if (gameIndex < 0 || gameIndex >= (int)g_storeGames.size()) return false;
        game = g_storeGames[gameIndex];
    }
    std::string installFolder = game.install_folder;
    if (installFolder.empty()) {
        installFolder = "/mnt/sdcard/bin/Joystick_Go_Store/games/" + game.title + "/";
    }
    removeDirectory(installFolder);

    {
        std::unique_lock<std::shared_mutex> lock(g_storeMutex);
        if (gameIndex < (int)g_installedGames.size()) {
            g_installedGames[gameIndex] = false;
            g_needUpdate[gameIndex] = false;
            g_installedVersion[gameIndex].clear();
        }
    }
    writeInstalledList();

    {
        std::unique_lock<std::shared_mutex> lock(g_storeMutex);
        if (!g_storeGames.empty() && g_storeGames[0].id == -1 && gameIndex < (int)g_storeGames.size()) {
            if (gameIndex < (int)g_gameTextures.size()) {
                g_gameTextures.erase(g_gameTextures.begin() + gameIndex);
            }
            if (gameIndex < (int)g_installedGames.size()) {
                g_installedGames.erase(g_installedGames.begin() + gameIndex);
                g_needUpdate.erase(g_needUpdate.begin() + gameIndex);
                g_installedVersion.erase(g_installedVersion.begin() + gameIndex);
            }
            g_storeGames.erase(g_storeGames.begin() + gameIndex);
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    loadSettings();

    const char* libs[] = {"libavformat.so.58","libavutil.so.56","libswresample.so.3","libswscale.so.5","libavcodec.so.58"};
    for (int i = 0; i < 5; i++) dlopen(libs[i], RTLD_NOW | RTLD_GLOBAL);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) != 0) return -1;
    SDL_JoystickOpen(0);

    g_win = SDL_CreateWindow("Store", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 600, SDL_WINDOW_SHOWN);
    if (!g_win) g_win = SDL_CreateWindow("Store", 0, 0, 1024, 600, SDL_WINDOW_FULLSCREEN);
    g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_ren) g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_SOFTWARE);
    int win_w, win_h; SDL_GetWindowSize(g_win, &win_w, &win_h);

    SDL_AudioSpec w = {}, g = {};
    w.freq = 44100; w.format = AUDIO_S16SYS; w.channels = 2; w.samples = 2048;
    g_audio = SDL_OpenAudioDevice(nullptr, 0, &w, &g, 0);
    SDL_PauseAudioDevice(g_audio, 0);
    g_effect_audio = SDL_OpenAudioDevice(nullptr, 0, &w, &g, 0);
    SDL_PauseAudioDevice(g_effect_audio, 0);

    SDL_Texture* blankTex = LoadImageFFmpeg(BLANK_ICON_PATH, g_ren);
    if (!blankTex) {
        blankTex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, 111, 111);
        SDL_SetRenderTarget(g_ren, blankTex);
        SDL_SetRenderDrawColor(g_ren, 80, 80, 80, 255);
        SDL_RenderFillRect(g_ren, nullptr);
        SDL_SetRenderTarget(g_ren, nullptr);
        SDL_SetTextureBlendMode(blankTex, SDL_BLENDMODE_BLEND);
    }

    std::vector<StoreGame> initialGames;
    bool dataLoaded = fetchStoreData(initialGames);
    if (!dataLoaded || initialGames.empty()) {
        printf("No games from server. Trying offline installed apps.\n");
        loadInstalledAppsFromFile(blankTex);
        updateInstalledStatus();
    } else {
        cacheGameImages(initialGames);
        {
            std::unique_lock<std::shared_mutex> lock(g_storeMutex);
            g_storeGames = std::move(initialGames);
            g_gameTextures = loadGameTexturesFromCache(g_ren, blankTex);
            g_installedGames.assign(g_storeGames.size(), false);
            g_needUpdate.assign(g_storeGames.size(), false);
            g_installedVersion.assign(g_storeGames.size(), "");
        }
        updateInstalledStatus();
    }

    SDL_Texture* splashTex = nullptr;
    if (!g_skipIntro) {
        splashTex = LoadImageFFmpeg(SPLASH_IMAGE_PATH, g_ren);
        if (splashTex) {
            SDL_RenderClear(g_ren);
            SDL_Rect fullRect = {0, 0, win_w, win_h};
            SDL_RenderCopy(g_ren, splashTex, nullptr, &fullRect);
            SDL_RenderPresent(g_ren);
        }
    }

    g_stop = false;
    std::thread audioThread;
    if (!g_skipIntro) {
        audioThread = std::thread(playIntroAudioThread, SPLASH_AUDIO_PATH);
    }

    Uint32 splashEndTime = SDL_GetTicks() + 4920;
    SDL_Event ev;
    bool splashSkipped = false;
    if (!g_skipIntro) {
        while (SDL_GetTicks() < splashEndTime && !splashSkipped) {
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_QUIT) { splashSkipped = true; break; }
                if (ev.type == SDL_JOYBUTTONDOWN || ev.type == SDL_KEYDOWN) { splashSkipped = true; break; }
            }
            SDL_Delay(10);
        }
    }

    g_stop = true;
    if (!g_skipIntro && audioThread.joinable()) audioThread.join();
    SDL_ClearQueuedAudio(g_audio);
    if (splashTex) SDL_DestroyTexture(splashTex);

    bool clickLoaded = loadSound(CLICK_SOUND_PATH, g_clickSamples);
    bool backLoaded = loadSound(BACK_SOUND_PATH, g_backSamples);
    bool keyboardClickLoaded = loadSound(KEYBOARD_CLICK_PATH, g_keyboardClickSamples);
    bool keyboardEraseLoaded = loadSound(KEYBOARD_ERASE_PATH, g_keyboardEraseSamples);
    bool keyboardMoveLoaded = loadSound(KEYBOARD_MOVE_PATH, g_keyboardMoveSamples);
    if (!keyboardMoveLoaded) printf("Warning: keyboard move.ogg not loaded\n");

    SDL_Texture* bg2Tex = LoadImageFFmpeg(BACKGROUND2_IMAGE_PATH, g_ren);
    SDL_Texture* topBarTex = LoadImageFFmpeg(TOP_BAR_IMAGE_PATH, g_ren);
    SDL_Texture* bgFindTex = LoadImageFFmpeg(BACKGROUND_FIND_IMAGE_PATH, g_ren);
    std::vector<SDL_Texture*> iconTextures(ICON_PATHS.size());
    for (size_t i = 0; i < ICON_PATHS.size(); ++i) iconTextures[i] = LoadImageFFmpeg(ICON_PATHS[i], g_ren);

    SDL_Texture* selectOverlayTex = LoadImageFFmpeg(SELECT_OVERLAY_PATH, g_ren);
    if (!selectOverlayTex) printf("Warning: select.png not loaded\n");

    std::vector<SDL_Texture*> batteryTextures(4);
    for (int i = 1; i <= 4; ++i) {
        batteryTextures[i-1] = LoadImageFFmpeg(BATTERY_ICON_BASE + std::to_string(i) + ".png", g_ren);
    }
    SDL_Texture* chargingTex = LoadImageFFmpeg(BATTERY_ICON_BASE + "charg.png", g_ren);

    SDL_Texture* downTex = LoadImageFFmpeg(DOWN_ICON_PATH, g_ren);
    if (!downTex) printf("Warning: down.png not loaded\n");
    SDL_Texture* checkTex = LoadImageFFmpeg(CHECK_ICON_PATH, g_ren);
    if (!checkTex) printf("Warning: check.png not loaded\n");
    SDL_Texture* upTex = LoadImageFFmpeg(UP_ICON_PATH, g_ren);
    if (!upTex) printf("Warning: up.png not loaded\n");

    SDL_Texture* clockTex = LoadImageFFmpeg(CLOCK_ICON_PATH, g_ren);
    if (!clockTex) printf("Warning: clock.png not loaded\n");

    SDL_Texture* findBackTex = LoadImageFFmpeg(FIND_ICON_BASE + "icon/back.png", g_ren);
    SDL_Texture* findBack2Tex = LoadImageFFmpeg(FIND_ICON_BASE + "icon/back2.png", g_ren);
    SDL_Texture* find2Tex = LoadImageFFmpeg(FIND_ICON_BASE + "icon/find2.png", g_ren);
    SDL_Texture* findYkeyTex = LoadImageFFmpeg(FIND_ICON_BASE + "icon/Ykey.png", g_ren);
    SDL_Texture* findKeybackTex = LoadImageFFmpeg(FIND_ICON_BASE + "icon/keyback.png", g_ren);
    SDL_Texture* findKeyback2Tex = LoadImageFFmpeg(FIND_ICON_BASE + "icon/keyback2.png", g_ren);

    SDL_Texture* downloadBackTex = LoadImageFFmpeg(DOWNLOAD_BACK_PATH, g_ren);
    SDL_Texture* downloadBorderTex = LoadImageFFmpeg(DOWNLOAD_BORDER_PATH, g_ren);
    SDL_Texture* downloadBack1Tex = LoadImageFFmpeg(DOWNLOAD_BACK1_PATH, g_ren);
    SDL_Texture* downloadDesTex = LoadImageFFmpeg(DOWNLOAD_DES_PATH, g_ren);
    SDL_Texture* downloadTitleTex = LoadImageFFmpeg(DOWNLOAD_TITLE_PATH, g_ren);
    SDL_Texture* downloadAuthorTex = LoadImageFFmpeg(DOWNLOAD_AUTHOR_PATH, g_ren);
    SDL_Texture* downloadSizeTex = LoadImageFFmpeg(DOWNLOAD_SIZE_PATH, g_ren);
    SDL_Texture* downloadVerTex = LoadImageFFmpeg(DOWNLOAD_VER_PATH, g_ren);
    SDL_Texture* downloadDateTex = LoadImageFFmpeg(DOWNLOAD_DATE_PATH, g_ren);
    SDL_Texture* downloadBtnTex = LoadImageFFmpeg(DOWNLOAD_BUTTON_PATH, g_ren);
    SDL_Texture* downloadRemoveBtnTex = LoadImageFFmpeg(DOWNLOAD_REMOVE_PATH, g_ren);
    SDL_Texture* downloadBackBtnTex = LoadImageFFmpeg(DOWNLOAD_BACKONE_PATH, g_ren);
    SDL_Texture* downloadInstallTex = LoadImageFFmpeg(DOWNLOAD_INSTALL_PATH, g_ren);
    SDL_Texture* downloadInstallTextTex = LoadImageFFmpeg(DOWNLOAD_INSTALL_TEXT_PATH, g_ren);
    SDL_Texture* downloadLaunchTex = LoadImageFFmpeg(DOWNLOAD_LAUNCH_PATH, g_ren);

    SDL_Texture* downloadUpdateTex = LoadImageFFmpeg(DOWNLOAD_UPDATE_PATH, g_ren);
    if (!downloadUpdateTex) printf("Warning: update.png not loaded\n");
    SDL_Texture* downloadUpdatingTextTex = LoadImageFFmpeg(DOWNLOAD_UPDATING_TEXT_PATH, g_ren);
    if (!downloadUpdatingTextTex) printf("Warning: updating.png not loaded\n");

    SDL_Texture* downloadInstallText2Tex = LoadImageFFmpeg(DOWNLOAD_INSTALL_TEXT2_PATH, g_ren);
    if (!downloadInstallText2Tex) printf("Warning: install_text2.png not loaded\n");

    SDL_Texture* categoryLabelTex = LoadImageFFmpeg(CATEGORY_LABEL_PATH, g_ren);

    SDL_Texture* pagesTex = LoadImageFFmpeg(PAGES_ICON_PATH, g_ren);
    SDL_Texture* back1Tex = LoadImageFFmpeg(BACK1_ICON_PATH, g_ren);
    SDL_Texture* backResize2Tex = LoadImageFFmpeg(BACK_RESIZE2_PATH, g_ren);
    if (!pagesTex) printf("Warning: pages.png not loaded\n");
    if (!back1Tex) printf("Warning: back1.png not loaded\n");
    if (!backResize2Tex) printf("Warning: back_resize2.png not loaded\n");

    SDL_Texture* boxTex = LoadImageFFmpeg("/mnt/sdcard/bin/Joystick_Go_Store/content/images/menu/icons/box.png", g_ren);
    if (!boxTex) printf("Warning: box.png not loaded\n");

    struct KeyDef {
        int x, y, w, h;
        std::string ch;
    };
    std::vector<KeyDef> keys;
    keys.push_back({371, 441, 49, 45, "Q"});
    keys.push_back({427, 441, 49, 45, "W"});
    keys.push_back({483, 441, 49, 45, "E"});
    keys.push_back({539, 441, 49, 45, "R"});
    keys.push_back({595, 441, 49, 45, "T"});
    keys.push_back({651, 441, 49, 45, "Y"});
    keys.push_back({707, 441, 49, 45, "U"});
    keys.push_back({763, 441, 49, 45, "I"});
    keys.push_back({819, 441, 49, 45, "O"});
    keys.push_back({875, 441, 49, 45, "P"});
    keys.push_back({399, 492, 49, 45, "A"});
    keys.push_back({455, 492, 49, 45, "S"});
    keys.push_back({511, 492, 49, 45, "D"});
    keys.push_back({567, 492, 49, 45, "F"});
    keys.push_back({623, 492, 49, 45, "G"});
    keys.push_back({679, 492, 49, 45, "H"});
    keys.push_back({735, 492, 49, 45, "J"});
    keys.push_back({791, 492, 49, 45, "K"});
    keys.push_back({847, 492, 49, 45, "L"});
    keys.push_back({379, 543, 69, 45, "DEL"});
    keys.push_back({456, 543, 49, 45, "Z"});
    keys.push_back({512, 543, 49, 45, "X"});
    keys.push_back({568, 543, 49, 45, "C"});
    keys.push_back({624, 543, 49, 45, "V"});
    keys.push_back({679, 543, 49, 45, "B"});
    keys.push_back({735, 543, 49, 45, "N"});
    keys.push_back({791, 543, 49, 45, "M"});
    keys.push_back({847, 543, 69, 45, "SPACE"});

    std::vector<SDL_Texture*> keyNormalTex(keys.size());
    std::vector<SDL_Texture*> keySelectTex(keys.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        std::string fname = keys[i].ch;
        std::string normalPath = FIND_NORMAL_BASE + fname + ".png";
        std::string selectPath = FIND_SELECT_BASE + fname + ".png";
        keyNormalTex[i] = LoadImageFFmpeg(normalPath, g_ren);
        keySelectTex[i] = LoadImageFFmpeg(selectPath, g_ren);
        if (!keyNormalTex[i] || !keySelectTex[i]) {
            printf("Warning: missing key textures for %s\n", fname.c_str());
        }
    }

    int lastPercent = -1;
    bool lastCharging = false;
    SDL_Texture* percentTex = nullptr;
    int percentW = 0, percentH = 0;

    SDL_Texture* dateTex = nullptr;
    SDL_Texture* timeTex = nullptr;
    SDL_Texture* ampmTex = nullptr;
    int dateW, dateH, timeW, timeH, ampmW, ampmH;
    std::string lastDateStr, lastTimeStr, lastAmpmStr;

    enum MenuState { STATE_MAIN, STATE_APPS, STATE_FIND, STATE_DOWNLOAD };
    MenuState currentState = STATE_MAIN;
    int selectedMain = 0;
    int selectedGame = -1;
    bool quitApp = false;

    int selectedKey = 0;
    std::string searchString = "";
    bool keyboardVisible = false;
    int selectedGameFind = -1;
    const int MAX_SEARCH_LEN = 15;
    const float FIND_TEXT_SCALE = 1.3f;
    int findCurrentPage = 0;

    int selectedDownloadGame = -1;
    MenuState prevState = STATE_MAIN;
    Uint32 downloadStartTime = 0;
    Uint32 installStartTime = 0;

    int currentPage = 0;
    int itemsPerPage = 12;

    g_stop = false;
    g_currentMusicPath = BG_MUSIC_PATH;
    g_bgAudioThread = std::thread(playAudioLoop, g_currentMusicPath);

    g_refreshStop = false;
    g_refreshThread = std::thread(storeRefreshLoop, g_ren, blankTex);

    Uint32 lastBatteryCheck = 0;
    const Uint32 BATTERY_CHECK_INTERVAL = 2000;

    const float clockX = 259.1f, clockY = 3.8f;
    const float clockW = 54.3f, clockH = 50.4f;

    const float dateX = 307.8f, dateY = 20.9f;
    const float dateW_base = 111.0f, dateH_base = 16.1f;

    const float timeX = 436.5f, timeY = 20.9f;
    const float timeW_base = 40.9f, timeH_base = 16.1f;

    const float ampmX = 482.3f, ampmY = 21.1f;
    const float ampmW_base = 29.7f, ampmH_base = 15.8f;

    const float batteryPercentX = 921.7f, batteryPercentY = 21.2f;
    const float batteryPercentW = 33.3f, batteryPercentH = 16.1f;

    const float backX = 272.5f, backY = 69.4f, backW = 268.0f, backH = 44.5f;
    const float find2X = 284.1f, find2Y = 78.9f, find2W = 31.9f, find2H = 27.9f;
    const float searchTextX = 326.0f;
    const float searchTextY = 83.6f;
    const float searchTextW = 274.3f;
    const float searchTextH = 16.1f;

    const float findYkeyX = 875.6f, findYkeyY = 69.4f, findYkeyW = 134.8f, findYkeyH = 44.5f;
    const float keyback2X = 345.5f, keyback2Y = 430.5f, keyback2W = 605.4f, keyback2H = 169.5f;

    const float pagesX = 875.6f, pagesY = 69.1f, pagesW = 134.8f, pagesH = 44.5f;
    const float back1X = 738.9f, back1Y = 69.4f, back1W = 134.8f, back1H = 44.5f;
    const float backResize2X = 773.2f, backResize2Y = 69.4f;
    const float pageTextX = 781.5f, pageTextY = 78.3f;

    const float categoryLabelX = 323.5f, categoryLabelY = 82.3f, categoryLabelW = 76.9f, categoryLabelH = 17.2f;
    const float categoryTextX = 411.5f, categoryTextY = 81.0f;

    const float downX_find_default[4] = {342.5f, 496.9f, 649.4f, 801.3f};
    const float downY_find_default[4] = {141.8f, 141.8f, 141.8f, 141.8f};
    const float downW_find = 33.3f, downH_find = 32.5f;

    const float gameX_find_default[4] = {344.6f, 496.8f, 649.2f, 801.2f};
    const float gameY_find_default[4] = {151.8f, 151.8f, 151.8f, 151.8f};
    const float gameW_find_default[4] = {152.2f, 152.2f, 152.2f, 152.2f};
    const float gameH_find_default[4] = {114.2f, 114.2f, 114.2f, 114.2f};

    const float titleX_find_default[4] = {358.0f, 512.4f, 664.9f, 816.9f};
    const float titleY_find_default[4] = {266.6f, 266.6f, 266.6f, 266.6f};
    const float titleW_find = 121.1f, titleH_find = 11.0f;

    const float creatorX_find_default[4] = {387.0f, 541.4f, 693.9f, 845.9f};
    const float creatorY_find_default[4] = {279.6f, 279.6f, 279.6f, 279.6f};
    const float creatorW_find = 63.1f, creatorH_find = 12.3f;

    const float rowSpacing = 159.8f;
    const int visibleRows = 3;

    const float dwBackX = 72.1f, dwBackY = 62.5f, dwBackW = 949.4f, dwBackH = 537.5f;
    const float dwBorderX = 284.3f, dwBorderY = 57.4f, dwBorderW = 715.6f, dwBorderH = 542.6f;
    const float dwBack1X = 290.3f, dwBack1Y = 72.8f, dwBack1W = 699.7f, dwBack1H = 512.1f;
    const float dwIconX = 325.1f, dwIconY = 143.9f, dwIconW = 152.4f, dwIconH = 113.2f;
    const float dwDownX = 325.2f, dwDownY = 131.9f, dwDownW = 33.3f, dwDownH = 32.5f;
    const float dwDesX = 491.7f, dwDesY = 122.3f, dwDesW = 86.3f, dwDesH = 15.4f;
    const float dwDescTextX = 487.7f, dwDescTextY = 143.9f, dwDescTextW = 415.1f, dwDescTextH = 127.4f;
    const float dwTitleIconX = 322.9f, dwTitleIconY = 294.8f, dwTitleIconW = 52.5f, dwTitleIconH = 18.9f;
    const float dwTitleTextX = 378.5f, dwTitleTextY = 294.9f, dwTitleTextW = 222.1f, dwTitleTextH = 18.6f;
    const float dwAuthorIconX = 322.9f, dwAuthorIconY = 319.8f, dwAuthorIconW = 81.6f, dwAuthorIconH = 18.9f;
    const float dwAuthorTextX = 407.7f, dwAuthorTextY = 319.8f, dwAuthorTextW = 204.8f, dwAuthorTextH = 18.6f;
    const float dwSizeIconX = 323.1f, dwSizeIconY = 344.9f, dwSizeIconW = 172.1f, dwSizeIconH = 18.9f;
    const float dwSizeTextX = 498.3f, dwSizeTextY = 344.9f, dwSizeTextW = 118.0f, dwSizeTextH = 18.6f;
    const float dwVerIconX = 323.6f, dwVerIconY = 369.8f, dwVerIconW = 91.6f, dwVerIconH = 18.9f;
    const float dwVerTextX = 418.3f, dwVerTextY = 369.9f, dwVerTextW = 187.4f, dwVerTextH = 18.6f;
    const float dwDateIconX = 325.5f, dwDateIconY = 395.1f, dwDateIconW = 57.9f, dwDateIconH = 18.9f;
    const float dwDateTextX = 386.5f, dwDateTextY = 395.0f, dwDateTextW = 222.1f, dwDateTextH = 18.6f;
    const float dwDlBtnX = 506.7f, dwDlBtnY = 489.2f, dwDlBtnW = 134.8f, dwDlBtnH = 44.2f;
    const float dwBackBtnX = 648.7f, dwBackBtnY = 489.2f, dwBackBtnW = 134.8f, dwBackBtnH = 44.5f;
    const float dwInstallX = 466.8f, dwInstallY = 282.5f, dwInstallW = 350.5f, dwInstallH = 88.5f;
    const float dwInstallTextX = 558.2f, dwInstallTextY = 314.7f, dwInstallTextW = 167.8f, dwInstallTextH = 24.1f;
    const float dwLaunchX = 814.4f, dwLaunchY = 559.5f, dwLaunchW = 136.4f, dwLaunchH = 17.0f;

    const float INSTALL_TEXT2_X = 575.6f;
    const float INSTALL_TEXT2_Y = 314.7f;
    const float INSTALL_TEXT2_W = 119.7f;
    const float INSTALL_TEXT2_H = 24.1f;

    const int SELECT_OVERLAY_W = 111;
    const int SELECT_OVERLAY_H = 111;

    const float downloadPercentX = 582.6f;
    const float downloadPercentY = 256.9f;
    const float downloadPercentW = 112.7f;
    const float downloadPercentH = 19.6f;

    while (!quitApp) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { quitApp = true; break; }
            if (ev.type == SDL_JOYBUTTONDOWN) {
                int btn = ev.jbutton.button;

                if (btn == BTN_SELECT) {
                    g_selectDownTime = SDL_GetTicks();
                    g_selectHeld = true;
                    continue;
                }

                bool isKeyboardKeyPress = (currentState == STATE_FIND && keyboardVisible && btn == BTN_A);
                if (clickLoaded && !isKeyboardKeyPress &&
                    (btn == BTN_DPUP || btn == BTN_DPDOWN || btn == BTN_DPLEFT || btn == BTN_DPRIGHT ||
                     btn == BTN_START || btn == BTN_A || btn == BTN_B ||
                     btn == BTN_L1 || btn == BTN_R1)) {
                    playSound(g_clickSamples);
                }

                if (currentState == STATE_MAIN) {
                    int prevSelected = selectedMain;
                    if (btn == BTN_DPUP) {
                        selectedMain--;
                        if (selectedMain < 0) selectedMain = ICON_PATHS.size() - 1;
                    } else if (btn == BTN_DPDOWN) {
                        selectedMain++;
                        if (selectedMain >= (int)ICON_PATHS.size()) selectedMain = 0;
                    }
                    if (selectedMain != prevSelected) {
                        currentPage = 0;
                    }
                    else if (btn == BTN_A) {
                        bool canEnter = true;
                        if (selectedMain >= 2 && selectedMain <= 4) {
                            Category targetCat;
                            if (selectedMain == 2) targetCat = CAT_TOOL;
                            else if (selectedMain == 3) targetCat = CAT_GAME;
                            else if (selectedMain == 4) targetCat = CAT_EMULATOR;
                            else targetCat = CAT_GAME;
                            int count = 0;
                            {
                                std::shared_lock<std::shared_mutex> lock(g_storeMutex);
                                for (size_t i = 0; i < g_storeGames.size(); ++i) {
                                    if (getCategory(g_storeGames[i].category) == targetCat) {
                                        count++;
                                        break;
                                    }
                                }
                            }
                            if (count == 0) canEnter = false;
                        }
                        if (selectedMain == 0) {
                            currentState = STATE_FIND;
                            selectedKey = 0;
                            keyboardVisible = false;
                            findCurrentPage = 0;
                        } else if (selectedMain >= 1 && selectedMain <= 4 && canEnter) {
                            currentState = STATE_APPS;
                            selectedGame = -1;
                            currentPage = 0;
                        } else {
                            if (clickLoaded) playSound(g_clickSamples);
                        }
                    }
                    else if (btn == BTN_B) {
                        quitApp = true;
                    }
                }
                else if (currentState == STATE_APPS) {
                    std::shared_lock<std::shared_mutex> lock(g_storeMutex);

                    std::vector<int> filteredGames;
                    if (selectedMain == 1) {
                        for (size_t i = 0; i < g_storeGames.size(); ++i) filteredGames.push_back(i);
                    } else {
                        Category targetCat;
                        if (selectedMain == 2) targetCat = CAT_TOOL;
                        else if (selectedMain == 3) targetCat = CAT_GAME;
                        else if (selectedMain == 4) targetCat = CAT_EMULATOR;
                        else targetCat = CAT_GAME;
                        for (size_t i = 0; i < g_storeGames.size(); ++i) {
                            if (getCategory(g_storeGames[i].category) == targetCat) filteredGames.push_back(i);
                        }
                    }
                    int totalFiltered = filteredGames.size();
                    int totalPagesFiltered = (totalFiltered + itemsPerPage - 1) / itemsPerPage;
                    if (totalPagesFiltered == 0) totalPagesFiltered = 1;
                    if (currentPage >= totalPagesFiltered) currentPage = totalPagesFiltered - 1;

                    if (btn == BTN_START) {
                        if (selectedGame != -1 && selectedGame < (int)g_installedGames.size() && g_installedGames[selectedGame]) {
                            launchGame(selectedGame);
                        } else {
                            printf("Game not installed or not selected\n");
                        }
                    }

                    if (btn == BTN_L1) {
                        currentPage--;
                        if (currentPage < 0) currentPage = totalPagesFiltered - 1;
                        if (!filteredGames.empty()) {
                            int startIdx = currentPage * itemsPerPage;
                            if (startIdx < (int)filteredGames.size()) {
                                selectedGame = filteredGames[startIdx];
                            } else {
                                selectedGame = filteredGames[0];
                            }
                        }
                    } else if (btn == BTN_R1) {
                        currentPage++;
                        if (currentPage >= totalPagesFiltered) currentPage = 0;
                        if (!filteredGames.empty()) {
                            int startIdx = currentPage * itemsPerPage;
                            if (startIdx < (int)filteredGames.size()) {
                                selectedGame = filteredGames[startIdx];
                            } else {
                                selectedGame = filteredGames[0];
                            }
                        }
                    } else if (btn == BTN_DPUP || btn == BTN_DPDOWN || btn == BTN_DPLEFT || btn == BTN_DPRIGHT) {
                        if (filteredGames.empty()) {
                            selectedGame = -1;
                            continue;
                        }
                        int curPos = 0;
                        if (selectedGame != -1) {
                            auto it = std::find(filteredGames.begin(), filteredGames.end(), selectedGame);
                            if (it != filteredGames.end()) curPos = it - filteredGames.begin();
                            else curPos = 0;
                        } else {
                            curPos = 0;
                        }
                        int maxPos = (int)filteredGames.size() - 1;
                        int pageStart = currentPage * itemsPerPage;
                        int pageEnd = std::min((currentPage+1)*itemsPerPage, (int)filteredGames.size()) - 1;
                        if (curPos < pageStart || curPos > pageEnd) curPos = pageStart;

                        int row = (curPos - pageStart) / 4;
                        int col = (curPos - pageStart) % 4;

                        if (btn == BTN_DPUP) { row--; if (row < 0) row = (pageEnd - pageStart) / 4; }
                        else if (btn == BTN_DPDOWN) { row++; if (row > (pageEnd - pageStart) / 4) row = 0; }
                        else if (btn == BTN_DPLEFT) { col--; if (col < 0) col = 3; }
                        else if (btn == BTN_DPRIGHT) { col++; if (col > 3) col = 0; }

                        int newPos = pageStart + row * 4 + col;
                        if (newPos < pageStart) newPos = pageStart;
                        if (newPos > pageEnd) newPos = pageEnd;
                        if (newPos >= 0 && newPos < (int)filteredGames.size()) {
                            selectedGame = filteredGames[newPos];
                        }
                    }
                    else if (btn == BTN_A) {
                        if (selectedGame != -1) {
                            prevState = STATE_APPS;
                            selectedDownloadGame = selectedGame;
                            currentState = STATE_DOWNLOAD;
                            g_downloadStatus = 0;
                            g_isUpdate = false;
                            g_downloadProgress = 0;
                            downloadStartTime = 0;
                            installStartTime = 0;
                        }
                    }
                    else if (btn == BTN_B) {
                        if (backLoaded) playSound(g_backSamples);
                        currentState = STATE_MAIN;
                        selectedGame = -1;
                        currentPage = 0;
                    }
                }
                else if (currentState == STATE_FIND) {
                    if (btn == BTN_START) {
                        if (selectedGameFind != -1 && selectedGameFind < (int)g_installedGames.size() && g_installedGames[selectedGameFind]) {
                            launchGame(selectedGameFind);
                        } else {
                            printf("Game not installed or not selected\n");
                        }
                    }
                    if (btn == BTN_Y) {
                        keyboardVisible = !keyboardVisible;
                        playSound(g_clickSamples);
                        if (!keyboardVisible && selectedGameFind == -1) {
                            selectedGameFind = -1;
                        }
                    }
                    else if (keyboardVisible) {
                        if (btn == BTN_DPUP || btn == BTN_DPDOWN || btn == BTN_DPLEFT || btn == BTN_DPRIGHT) {
                            if (keyboardMoveLoaded) playSound(g_keyboardMoveSamples);
                            std::vector<int> rowStarts = {0, 10, 19};
                            std::vector<int> rowEnds   = {10, 19, 28};
                            int curY = keys[selectedKey].y;
                            int rowIndex = -1;
                            for (int r = 0; r < 3; ++r) {
                                int rowY = (r == 0) ? 441 : (r == 1 ? 492 : 543);
                                if (curY == rowY) { rowIndex = r; break; }
                            }
                            if (rowIndex == -1) rowIndex = 0;
                            int start = rowStarts[rowIndex];
                            int end   = rowEnds[rowIndex];
                            int colIndex = selectedKey - start;
                            int cols = end - start;

                            if (btn == BTN_DPLEFT) { colIndex--; if (colIndex < 0) colIndex = cols - 1; }
                            else if (btn == BTN_DPRIGHT) { colIndex++; if (colIndex >= cols) colIndex = 0; }
                            else if (btn == BTN_DPUP) { rowIndex--; if (rowIndex < 0) rowIndex = 2; }
                            else if (btn == BTN_DPDOWN) { rowIndex++; if (rowIndex > 2) rowIndex = 0; }

                            int newStart = rowStarts[rowIndex];
                            int newEnd   = rowEnds[rowIndex];
                            int newCols  = newEnd - newStart;
                            if (colIndex >= newCols) colIndex = newCols - 1;
                            if (colIndex < 0) colIndex = 0;
                            int newIndex = newStart + colIndex;
                            if (newIndex >= 0 && newIndex < (int)keys.size()) {
                                selectedKey = newIndex;
                            }
                        } else if (btn == BTN_A) {
                            std::string ch = keys[selectedKey].ch;
                            if (ch == "DEL") {
                                if (!searchString.empty()) searchString.pop_back();
                                if (keyboardEraseLoaded) playSound(g_keyboardEraseSamples);
                                else if (clickLoaded) playSound(g_clickSamples);
                            } else if (ch == "SPACE") {
                                if (searchString.length() < MAX_SEARCH_LEN) searchString += '_';
                                if (keyboardClickLoaded) playSound(g_keyboardClickSamples);
                                else if (clickLoaded) playSound(g_clickSamples);
                            } else {
                                if (searchString.length() < MAX_SEARCH_LEN) searchString += ch;
                                if (keyboardClickLoaded) playSound(g_keyboardClickSamples);
                                else if (clickLoaded) playSound(g_clickSamples);
                            }
                            printf("Search: %s\n", searchString.c_str());
                            findCurrentPage = 0;
                        } else if (btn == BTN_B) {
                            if (keyboardVisible) {
                                keyboardVisible = false;
                                if (backLoaded) playSound(g_backSamples);
                            } else {
                                if (backLoaded) playSound(g_backSamples);
                                currentState = STATE_MAIN;
                                selectedGame = -1;
                            }
                        }
                    } else {
                        std::shared_lock<std::shared_mutex> lock(g_storeMutex);
                        std::vector<int> filteredIndices;
                        for (size_t i = 0; i < g_storeGames.size(); ++i) {
                            std::string title = g_storeGames[i].title;
                            std::string lowerTitle = title;
                            std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
                            std::string lowerSearch = searchString;
                            std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
                            if (lowerTitle.find(lowerSearch) != std::string::npos) {
                                filteredIndices.push_back(i);
                            }
                        }
                        int totalFiltered = filteredIndices.size();
                        int totalPages = (totalFiltered + itemsPerPage - 1) / itemsPerPage;
                        if (totalPages == 0) totalPages = 1;
                        if (findCurrentPage >= totalPages) findCurrentPage = totalPages - 1;

                        if (btn == BTN_L1) {
                            findCurrentPage--;
                            if (findCurrentPage < 0) findCurrentPage = totalPages - 1;
                            if (!filteredIndices.empty()) {
                                int startIdx = findCurrentPage * itemsPerPage;
                                if (startIdx < totalFiltered) selectedGameFind = filteredIndices[startIdx];
                                else selectedGameFind = filteredIndices[0];
                            }
                        } else if (btn == BTN_R1) {
                            findCurrentPage++;
                            if (findCurrentPage >= totalPages) findCurrentPage = 0;
                            if (!filteredIndices.empty()) {
                                int startIdx = findCurrentPage * itemsPerPage;
                                if (startIdx < totalFiltered) selectedGameFind = filteredIndices[startIdx];
                                else selectedGameFind = filteredIndices[0];
                            }
                        } else if (btn == BTN_DPUP || btn == BTN_DPDOWN || btn == BTN_DPLEFT || btn == BTN_DPRIGHT) {
                            if (filteredIndices.empty()) {
                                selectedGameFind = -1;
                                continue;
                            }
                            int curPos = 0;
                            if (selectedGameFind != -1) {
                                auto it = std::find(filteredIndices.begin(), filteredIndices.end(), selectedGameFind);
                                if (it != filteredIndices.end()) curPos = it - filteredIndices.begin();
                                else curPos = 0;
                            } else {
                                curPos = 0;
                            }
                            int maxPos = totalFiltered - 1;
                            int pageStart = findCurrentPage * itemsPerPage;
                            int pageEnd = std::min((findCurrentPage+1)*itemsPerPage, totalFiltered) - 1;
                            if (curPos < pageStart || curPos > pageEnd) curPos = pageStart;

                            int row = (curPos - pageStart) / 4;
                            int col = (curPos - pageStart) % 4;

                            if (btn == BTN_DPUP) { row--; if (row < 0) row = (pageEnd - pageStart) / 4; }
                            else if (btn == BTN_DPDOWN) { row++; if (row > (pageEnd - pageStart) / 4) row = 0; }
                            else if (btn == BTN_DPLEFT) { col--; if (col < 0) col = 3; }
                            else if (btn == BTN_DPRIGHT) { col++; if (col > 3) col = 0; }

                            int newPos = pageStart + row * 4 + col;
                            if (newPos < pageStart) newPos = pageStart;
                            if (newPos > pageEnd) newPos = pageEnd;
                            if (newPos >= 0 && newPos < totalFiltered) {
                                selectedGameFind = filteredIndices[newPos];
                            }
                        } else if (btn == BTN_A) {
                            if (filteredIndices.empty()) continue;
                            if (selectedGameFind != -1) {
                                prevState = STATE_FIND;
                                selectedDownloadGame = selectedGameFind;
                                currentState = STATE_DOWNLOAD;
                                g_downloadStatus = 0;
                                g_isUpdate = false;
                                g_downloadProgress = 0;
                                downloadStartTime = 0;
                                installStartTime = 0;
                            }
                        } else if (btn == BTN_B) {
                            if (backLoaded) playSound(g_backSamples);
                            currentState = STATE_MAIN;
                            selectedGame = -1;
                        }
                    }
                }
                else if (currentState == STATE_DOWNLOAD) {
                    if (btn == BTN_START) {
                        if (selectedDownloadGame != -1 && selectedDownloadGame < (int)g_installedGames.size() && g_installedGames[selectedDownloadGame]) {
                            if (g_needUpdate[selectedDownloadGame]) {
                                if (g_downloadStatus == 0) {
                                    g_isUpdate = true;
                                    g_downloadStatus = 1;
                                    g_downloadProgress = 0;
                                    downloadStartTime = SDL_GetTicks();
                                    installStartTime = 0;
                                    g_lastProgressUpdate = SDL_GetTicks();
                                    printf("Update started for game %d\n", selectedDownloadGame+1);
                                    if (g_downloadThread.joinable()) g_downloadThread.join();
                                    g_downloadThread = std::thread(downloadAndInstallThread, selectedDownloadGame);
                                }
                            } else {
                                launchGame(selectedDownloadGame);
                            }
                        }
                        continue;
                    }
                    if ((btn == BTN_B || btn == BTN_START) && (g_downloadStatus == 1 || g_downloadStatus == 2)) {
                        continue;
                    }
                    if (btn == BTN_B) {
                        if (backLoaded) playSound(g_backSamples);
                        currentState = prevState;
                        if (prevState == STATE_APPS) selectedGame = selectedDownloadGame;
                        else if (prevState == STATE_FIND) selectedGameFind = selectedDownloadGame;
                        selectedDownloadGame = -1;
                        g_downloadStatus = 0;
                        g_isUpdate = false;
                        g_downloadProgress = 0;
                        downloadStartTime = 0;
                        installStartTime = 0;
                    } else if (btn == BTN_A) {
                        if (selectedDownloadGame >= 0 && selectedDownloadGame < (int)g_installedGames.size()) {
                            bool isInstalled = g_installedGames[selectedDownloadGame];
                            if (isInstalled) {
                                removeGameFiles(selectedDownloadGame);
                                g_installedGames[selectedDownloadGame] = false;
                                g_needUpdate[selectedDownloadGame] = false;
                                g_downloadStatus = 0;
                                g_downloadProgress = 0;
                                printf("Game %d uninstalled\n", selectedDownloadGame+1);
                                {
                                    std::shared_lock<std::shared_mutex> lock(g_storeMutex);
                                    bool offline = (!g_storeGames.empty() && g_storeGames[0].id == -1);
                                    if (offline) {
                                        currentState = STATE_MAIN;
                                        selectedDownloadGame = -1;
                                        printf("Offline: uninstalled, returning to main menu\n");
                                    }
                                }
                            } else {
                                if (g_downloadStatus == 0) {
                                    g_isUpdate = false;
                                    g_downloadStatus = 1;
                                    g_downloadProgress = 0;
                                    downloadStartTime = SDL_GetTicks();
                                    installStartTime = 0;
                                    g_lastProgressUpdate = SDL_GetTicks();
                                    printf("Download started for game %d\n", selectedDownloadGame+1);
                                    if (g_downloadThread.joinable()) g_downloadThread.join();
                                    g_downloadThread = std::thread(downloadAndInstallThread, selectedDownloadGame);
                                }
                            }
                        }
                    }
                }
            }

            if (ev.type == SDL_JOYBUTTONUP) {
                int btn = ev.jbutton.button;
                if (btn == BTN_SELECT) {
                    Uint32 held = SDL_GetTicks() - g_selectDownTime;
                    g_selectHeld = false;
                    if (held < LONG_PRESS_MS) {
                        if (!g_muted) {
                            g_previousMusicVolume = g_musicVolume.load();
                            g_musicVolume = 0;
                            g_muted = true;
                        } else {
                            g_musicVolume = g_previousMusicVolume;
                            g_muted = false;
                        }
                    } else {
                        if (g_currentMusicPath == BG_MUSIC_PATH) {
                            g_currentMusicPath = BG_MUSIC2_PATH;
                        } else {
                            g_currentMusicPath = BG_MUSIC_PATH;
                        }
                        g_stop = true;
                        if (g_bgAudioThread.joinable()) {
                            g_bgAudioThread.join();
                        }
                        g_stop = false;
                        g_bgAudioThread = std::thread(playAudioLoop, g_currentMusicPath);
                    }
                }
            }
        }

        Uint32 now = SDL_GetTicks();

        if (currentState == STATE_FIND && keyboardVisible) {
            if (selectedKey >= 0 && selectedKey < (int)keys.size() && keys[selectedKey].ch == "DEL") {
                SDL_Joystick* joy2 = SDL_JoystickOpen(0);
                bool erasePressed = false;
                if (joy2) {
                    if (SDL_JoystickGetButton(joy2, BTN_A)) {
                        erasePressed = true;
                    }
                }
                if (erasePressed) {
                    if (g_lastEraseRepeat == 0) {
                        g_lastEraseRepeat = now;
                    } else {
                        Uint32 elapsed = now - g_lastEraseRepeat;
                        if (elapsed >= REPEAT_INITIAL_DELAY && (elapsed - REPEAT_INITIAL_DELAY) % REPEAT_INTERVAL < 16) {
                            SDL_Event repeatEv;
                            repeatEv.type = SDL_JOYBUTTONDOWN;
                            repeatEv.jbutton.which = 0;
                            repeatEv.jbutton.button = BTN_A;
                            repeatEv.jbutton.state = SDL_PRESSED;
                            SDL_PushEvent(&repeatEv);
                        }
                    }
                } else {
                    g_lastEraseRepeat = 0;
                }
            } else {
                g_lastEraseRepeat = 0;
            }
        } else {
            g_lastEraseRepeat = 0;
        }

        if (now - lastBatteryCheck >= BATTERY_CHECK_INTERVAL) {
            lastBatteryCheck = now;
            int percent = getBatteryPercent();
            bool charging = isCharging();
            if (percent != lastPercent || charging != lastCharging) {
                lastPercent = percent;
                lastCharging = charging;
                if (percentTex) {
                    SDL_DestroyTexture(percentTex);
                    percentTex = nullptr;
                }
                if (percent >= 0) {
                    std::string text = std::to_string(percent);
                    percentTex = renderStringToTexture(text, g_ren, &percentW, &percentH);
                }
            }
        }

        static Uint32 lastTimeUpdate = 0;
        if (now - lastTimeUpdate >= 1000) {
            lastTimeUpdate = now;
            std::time_t t = std::time(nullptr);
            std::tm* tm = std::localtime(&t);
            char dateBuf[20], timeBuf[10], ampmBuf[5];
            std::strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", tm);
            std::strftime(timeBuf, sizeof(timeBuf), "%I:%M", tm);
            std::strftime(ampmBuf, sizeof(ampmBuf), "%p", tm);
            std::string newDate(dateBuf);
            std::string newTime(timeBuf);
            std::string newAmpm(ampmBuf);
            if (newDate != lastDateStr) {
                lastDateStr = newDate;
                if (dateTex) { SDL_DestroyTexture(dateTex); dateTex = nullptr; }
                dateTex = renderStringToTexture(newDate, g_ren, &dateW, &dateH);
            }
            if (newTime != lastTimeStr) {
                lastTimeStr = newTime;
                if (timeTex) { SDL_DestroyTexture(timeTex); timeTex = nullptr; }
                timeTex = renderStringToTexture(newTime, g_ren, &timeW, &timeH);
            }
            if (newAmpm != lastAmpmStr) {
                lastAmpmStr = newAmpm;
                if (ampmTex) { SDL_DestroyTexture(ampmTex); ampmTex = nullptr; }
                ampmTex = renderStringToTexture(newAmpm, g_ren, &ampmW, &ampmH);
            }
        }

        if (g_downloadStatus == 3) {
            if (selectedDownloadGame >= 0 && selectedDownloadGame < (int)g_installedGames.size()) {
                g_installedGames[selectedDownloadGame] = true;
                g_needUpdate[selectedDownloadGame] = false;
            }
            updateInstalledStatus();
            g_downloadStatus = 0;
            g_isUpdate = false;
            g_downloadProgress = 0;
            downloadStartTime = 0;
            installStartTime = 0;
            printf("Install/Update finished for game %d\n", selectedDownloadGame+1);
        }

        if (currentState == STATE_FIND) {
            std::shared_lock<std::shared_mutex> lock(g_storeMutex);
            std::vector<int> filteredIndices;
            for (size_t i = 0; i < g_storeGames.size(); ++i) {
                std::string title = g_storeGames[i].title;
                std::string lowerTitle = title;
                std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
                std::string lowerSearch = searchString;
                std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
                if (lowerTitle.find(lowerSearch) != std::string::npos) {
                    filteredIndices.push_back(i);
                }
            }
            int totalFiltered = filteredIndices.size();
            int totalPages = (totalFiltered + itemsPerPage - 1) / itemsPerPage;
            if (totalPages == 0) totalPages = 1;
            if (findCurrentPage >= totalPages) findCurrentPage = totalPages - 1;
            if (!filteredIndices.empty()) {
                int startIdx = findCurrentPage * itemsPerPage;
                if (selectedGameFind == -1 || std::find(filteredIndices.begin()+startIdx,
                                                        filteredIndices.begin()+std::min(startIdx+itemsPerPage, totalFiltered),
                                                        selectedGameFind) == filteredIndices.begin()+std::min(startIdx+itemsPerPage, totalFiltered)) {
                    if (startIdx < totalFiltered) selectedGameFind = filteredIndices[startIdx];
                    else selectedGameFind = filteredIndices[0];
                }
            } else {
                selectedGameFind = -1;
            }
        }

        SDL_RenderClear(g_ren);
        SDL_Rect fullRect = {0, 0, win_w, win_h};

        if (bg2Tex && currentState != STATE_DOWNLOAD) {
            SDL_RenderCopy(g_ren, bg2Tex, nullptr, &fullRect);
        } else if (currentState == STATE_DOWNLOAD) {
            if (bgFindTex) SDL_RenderCopy(g_ren, bgFindTex, nullptr, &fullRect);
            else { SDL_SetRenderDrawColor(g_ren, 30,30,35,255); SDL_RenderFillRect(g_ren, &fullRect); }
        } else {
            SDL_SetRenderDrawColor(g_ren, 30,30,35,255); SDL_RenderFillRect(g_ren, &fullRect);
        }

        if (topBarTex) {
            int topBarH = (int)(win_h * 0.10f);
            SDL_Rect topBarRect = {0, 0, win_w, topBarH};
            SDL_RenderCopy(g_ren, topBarTex, nullptr, &topBarRect);
        }

        std::shared_lock<std::shared_mutex> storeLock(g_storeMutex);

        if (currentState == STATE_MAIN) {
            float scaleX = (float)win_w / 1024.0f;
            float scaleY = (float)win_h / 600.0f;

            if (selectedMain == 0) {
                if (bgFindTex) SDL_RenderCopy(g_ren, bgFindTex, nullptr, &fullRect);
                std::vector<int> filteredIndices;
                for (size_t i = 0; i < g_storeGames.size(); ++i) {
                    std::string title = g_storeGames[i].title;
                    std::string lowerTitle = title;
                    std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
                    std::string lowerSearch = searchString;
                    std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
                    if (lowerTitle.find(lowerSearch) != std::string::npos) {
                        filteredIndices.push_back(i);
                    }
                }
                int totalFiltered = filteredIndices.size();
                int totalPages = (totalFiltered + itemsPerPage - 1) / itemsPerPage;
                if (totalPages == 0) totalPages = 1;
                if (findCurrentPage >= totalPages) findCurrentPage = totalPages - 1;
                int startIdx = findCurrentPage * itemsPerPage;
                int endIdx = std::min(startIdx + itemsPerPage, totalFiltered);

                for (int pos = startIdx; pos < endIdx; ++pos) {
                    int i = filteredIndices[pos];
                    int row = (pos - startIdx) / 4;
                    int col = (pos - startIdx) % 4;

                    float baseGameY = gameY_find_default[col];
                    float gameX = gameX_find_default[col];
                    float gameY = baseGameY + row * rowSpacing - 20;
                    float gameW = gameW_find_default[col];
                    float gameH = gameH_find_default[col];

                    int dstX = (int)(gameX * scaleX);
                    int dstY = (int)(gameY * scaleY);
                    int dstW = (int)(gameW * scaleX);
                    int dstH = (int)(gameH * scaleY);

                    if (blankTex) {
                        SDL_Rect dst = {dstX, dstY, dstW, dstH};
                        SDL_RenderCopy(g_ren, blankTex, nullptr, &dst);
                    }
                    SDL_Texture* tex = (i < (int)g_gameTextures.size()) ? g_gameTextures[i] : nullptr;
                    if (tex && tex != blankTex) {
                        SDL_Rect dst = {dstX, dstY, dstW, dstH};
                        renderTextureWithAspectAndClip(g_ren, tex, &dst);
                    } else if (!tex && !blankTex) {
                        SDL_SetRenderDrawColor(g_ren, 100,100,100,255);
                        SDL_Rect dst = {dstX, dstY, dstW, dstH};
                        SDL_RenderFillRect(g_ren, &dst);
                    }

                    SDL_Texture* arrowTex = downTex;
                    if (i < (int)g_installedGames.size() && g_installedGames[i]) {
                        if (i < (int)g_needUpdate.size() && g_needUpdate[i]) {
                            arrowTex = upTex;
                        } else {
                            arrowTex = checkTex;
                        }
                    }
                    if (arrowTex) {
                        float baseDownY = downY_find_default[col];
                        float downX = downX_find_default[col];
                        float downY = baseDownY + row * rowSpacing - 20;
                        int drawX = (int)(downX * scaleX);
                        int drawY = (int)(downY * scaleY);
                        int drawW = (int)(downW_find * scaleX);
                        int drawH = (int)(downH_find * scaleY);
                        SDL_Rect dst = {drawX, drawY, drawW, drawH};
                        SDL_RenderCopy(g_ren, arrowTex, nullptr, &dst);
                    }
                    {
                        std::string title = (i < (int)g_storeGames.size()) ? g_storeGames[i].title : "Unknown";
                        float baseTitleY = titleY_find_default[col];
                        float titleX = titleX_find_default[col];
                        float titleY = baseTitleY + row * rowSpacing - 20;
                        int drawX = (int)(titleX * scaleX);
                        int drawY = (int)(titleY * scaleY);
                        int drawW = (int)(titleW_find * scaleX);
                        int drawH = (int)(titleH_find * scaleY);
                        float scale = 1.5f;
                        int charWidth = (int)((8 + 1) * scale);
                        int textWidth = (int)(title.length() * charWidth);
                        if (textWidth > drawW) {
                            scale = (float)drawW / (title.length() * (8 + 1));
                            scale *= 0.9f;
                            if (scale < 0.5f) scale = 0.5f;
                            charWidth = (int)((8 + 1) * scale);
                            textWidth = (int)(title.length() * charWidth);
                        }
                        int centeredX = drawX + (drawW - textWidth) / 2;
                        drawWrappedText(g_ren, title, centeredX, drawY, drawW, drawH, scale);
                    }
                    {
                        std::string creator = (i < (int)g_storeGames.size()) ? g_storeGames[i].author : "Unknown";
                        std::string colorStr = (i < (int)g_storeGames.size()) ? g_storeGames[i].author : "";
                        Uint8 r=255, g=255, b=255;
                        if (!colorStr.empty() && colorStr[0] == '#') {
                            unsigned int hex;
                            sscanf(colorStr.c_str()+1, "%06x", &hex);
                            r = (hex >> 16) & 0xFF;
                            g = (hex >> 8) & 0xFF;
                            b = hex & 0xFF;
                        }
                        float baseCreatorY = creatorY_find_default[col];
                        float creatorX = creatorX_find_default[col];
                        float creatorY = baseCreatorY + row * rowSpacing - 20;
                        int drawX = (int)(creatorX * scaleX);
                        int drawY = (int)(creatorY * scaleY);
                        int drawW = (int)(creatorW_find * scaleX);
                        int drawH = (int)(creatorH_find * scaleY);
                        float scale = 1.5f;
                        int charWidth = (int)((8 + 1) * scale);
                        int textWidth = (int)(creator.length() * charWidth);
                        if (textWidth > drawW) {
                            scale = (float)drawW / (creator.length() * (8 + 1));
                            scale *= 0.9f;
                            if (scale < 0.5f) scale = 0.5f;
                            charWidth = (int)((8 + 1) * scale);
                            textWidth = (int)(creator.length() * charWidth);
                        }
                        int centeredX = drawX + (drawW - textWidth) / 2;
                        drawWrappedText(g_ren, creator, centeredX, drawY, drawW, drawH, scale, r, g, b);
                    }
                }

                if (findBackTex) {
                    int dstX = (int)(backX * scaleX);
                    int dstY = (int)(backY * scaleY);
                    int dstW = (int)(backW * scaleX);
                    int dstH = (int)(backH * scaleY);
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    SDL_RenderCopy(g_ren, findBackTex, nullptr, &dst);
                }
                if (find2Tex) {
                    int dstX = (int)(find2X * scaleX);
                    int dstY = (int)(find2Y * scaleY);
                    int dstW = (int)(find2W * scaleX);
                    int dstH = (int)(find2H * scaleY);
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    SDL_RenderCopy(g_ren, find2Tex, nullptr, &dst);
                }
                if (!searchString.empty()) {
                    int tw, th;
                    SDL_Texture* txt = renderStringToTexture(searchString, g_ren, &tw, &th);
                    if (txt) {
                        int drawW = (int)(tw * FIND_TEXT_SCALE);
                        int drawH = (int)(th * FIND_TEXT_SCALE);
                        int dstX = (int)(searchTextX * scaleX);
                        int dstY = (int)(searchTextY * scaleY + (searchTextH * scaleY - drawH) / 2);
                        SDL_Rect dst = {dstX, dstY, drawW, drawH};
                        SDL_RenderCopy(g_ren, txt, nullptr, &dst);
                        SDL_DestroyTexture(txt);
                    }
                }
                if (findYkeyTex) {
                    int dstX = (int)(findYkeyX * scaleX);
                    int dstY = (int)(findYkeyY * scaleY);
                    int dstW = (int)(findYkeyW * scaleX);
                    int dstH = (int)(findYkeyH * scaleY);
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    SDL_RenderCopy(g_ren, findYkeyTex, nullptr, &dst);
                }

                if (pagesTex) {
                    int dstX = (int)(pagesX * scaleX);
                    int dstY = (int)(pagesY * scaleY);
                    int dstW = (int)(pagesW * scaleX);
                    int dstH = (int)(pagesH * scaleY);
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    SDL_RenderCopy(g_ren, pagesTex, nullptr, &dst);
                }
                if (back1Tex) {
                    int dstX = (int)(back1X * scaleX);
                    int dstY = (int)(back1Y * scaleY);
                    int dstW = (int)(back1W * scaleX);
                    int dstH = (int)(back1H * scaleY);
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    SDL_RenderCopy(g_ren, back1Tex, nullptr, &dst);
                }
                int totalFiltered2 = filteredIndices.size();
                int totalPages2 = (totalFiltered2 + itemsPerPage - 1) / itemsPerPage;
                if (totalPages2 == 0) totalPages2 = 1;
                std::string pageText = std::to_string(findCurrentPage+1) + "/" + std::to_string(totalPages2);
                int tw, th;
                SDL_Texture* pageTxtTex = renderStringToTexture(pageText, g_ren, &tw, &th);
                if (pageTxtTex) {
                    float textScale = 1.5f;
                    int textW = (int)(tw * textScale);
                    int textH = (int)(th * textScale);
                    int padW = 20, padH = 10;
                    int bgW = textW + padW;
                    int bgH = textH + padH;
                    if (bgW < 66) bgW = 66;
                    if (bgH < 44) bgH = 44;
                    if (backResize2Tex) {
                        SDL_Rect bgDst = {
                            (int)(backResize2X * scaleX),
                            (int)(backResize2Y * scaleY),
                            (int)(bgW * scaleX),
                            (int)(bgH * scaleY)
                        };
                        SDL_RenderCopy(g_ren, backResize2Tex, nullptr, &bgDst);
                    }
                    int textDstX = (int)((backResize2X + (bgW - textW)/2) * scaleX);
                    int textDstY = (int)((backResize2Y + (bgH - textH)/2) * scaleY);
                    SDL_Rect textDst = {textDstX, textDstY, textW, textH};
                    SDL_RenderCopy(g_ren, pageTxtTex, nullptr, &textDst);
                    SDL_DestroyTexture(pageTxtTex);
                }
            } else if (selectedMain >= 1 && selectedMain <= 4) {
                std::vector<int> filteredGames;
                if (selectedMain == 1) {
                    for (size_t i = 0; i < g_storeGames.size(); ++i) filteredGames.push_back(i);
                } else {
                    Category targetCat;
                    if (selectedMain == 2) targetCat = CAT_TOOL;
                    else if (selectedMain == 3) targetCat = CAT_GAME;
                    else if (selectedMain == 4) targetCat = CAT_EMULATOR;
                    else targetCat = CAT_GAME;
                    for (size_t i = 0; i < g_storeGames.size(); ++i) {
                        if (getCategory(g_storeGames[i].category) == targetCat) filteredGames.push_back(i);
                    }
                }
                if (filteredGames.empty()) {
                    if (boxTex) {
                        SDL_Rect boxDst = {
                            (int)(593.4f * scaleX),
                            (int)(274.6f * scaleY),
                            (int)(125.4f * scaleX),
                            (int)(118.4f * scaleY)
                        };
                        SDL_RenderCopy(g_ren, boxTex, nullptr, &boxDst);
                    }
                } else {
                    int totalFiltered = filteredGames.size();
                    int totalPagesFiltered = (totalFiltered + itemsPerPage - 1) / itemsPerPage;
                    if (totalPagesFiltered == 0) totalPagesFiltered = 1;
                    if (currentPage >= totalPagesFiltered) currentPage = totalPagesFiltered - 1;

                    int startIdx = currentPage * itemsPerPage;
                    int endIdx = std::min(startIdx + itemsPerPage, (int)filteredGames.size());
                    for (int p = startIdx; p < endIdx; ++p) {
                        int i = filteredGames[p];
                        int pos = p - startIdx;
                        int row = pos / 4;
                        int col = pos % 4;

                        float gameX = gameX_find_default[col];
                        float gameY = gameY_find_default[col] + row * rowSpacing - 20;
                        float gameW = gameW_find_default[col];
                        float gameH = gameH_find_default[col];

                        int dstX = (int)(gameX * scaleX);
                        int dstY = (int)(gameY * scaleY);
                        int dstW = (int)(gameW * scaleX);
                        int dstH = (int)(gameH * scaleY);

                        if (blankTex) {
                            SDL_Rect dst = {dstX, dstY, dstW, dstH};
                            SDL_RenderCopy(g_ren, blankTex, nullptr, &dst);
                        }
                        SDL_Texture* tex = (i < (int)g_gameTextures.size()) ? g_gameTextures[i] : nullptr;
                        if (tex && tex != blankTex) {
                            SDL_Rect dst = {dstX, dstY, dstW, dstH};
                            renderTextureWithAspectAndClip(g_ren, tex, &dst);
                        } else if (!tex && !blankTex) {
                            SDL_SetRenderDrawColor(g_ren, 100,100,100,255);
                            SDL_Rect dst = {dstX, dstY, dstW, dstH};
                            SDL_RenderFillRect(g_ren, &dst);
                        }

                        SDL_Texture* arrowTex = downTex;
                        if (i < (int)g_installedGames.size() && g_installedGames[i]) {
                            if (i < (int)g_needUpdate.size() && g_needUpdate[i]) {
                                arrowTex = upTex;
                            } else {
                                arrowTex = checkTex;
                            }
                        }
                        if (arrowTex) {
                            float downX = downX_find_default[col];
                            float downY = downY_find_default[col] + row * rowSpacing - 20;
                            int drawX = (int)(downX * scaleX);
                            int drawY = (int)(downY * scaleY);
                            int drawW = (int)(downW_find * scaleX);
                            int drawH = (int)(downH_find * scaleY);
                            SDL_Rect dst = {drawX, drawY, drawW, drawH};
                            SDL_RenderCopy(g_ren, arrowTex, nullptr, &dst);
                        }
                        {
                            std::string title = (i < (int)g_storeGames.size()) ? g_storeGames[i].title : "Unknown";
                            float titleX = titleX_find_default[col];
                            float titleY = titleY_find_default[col] + row * rowSpacing - 20;
                            int drawX = (int)(titleX * scaleX);
                            int drawY = (int)(titleY * scaleY);
                            int drawW = (int)(titleW_find * scaleX);
                            int drawH = (int)(titleH_find * scaleY);
                            float scale = 1.5f;
                            int charWidth = (int)((8 + 1) * scale);
                            int textWidth = (int)(title.length() * charWidth);
                            if (textWidth > drawW) {
                                scale = (float)drawW / (title.length() * (8 + 1));
                                scale *= 0.9f;
                                if (scale < 0.5f) scale = 0.5f;
                                charWidth = (int)((8 + 1) * scale);
                                textWidth = (int)(title.length() * charWidth);
                            }
                            int centeredX = drawX + (drawW - textWidth) / 2;
                            drawWrappedText(g_ren, title, centeredX, drawY, drawW, drawH, scale);
                        }
                        {
                            std::string creator = (i < (int)g_storeGames.size()) ? g_storeGames[i].author : "Unknown";
                            std::string colorStr = (i < (int)g_storeGames.size()) ? g_storeGames[i].author : "";
                            Uint8 r=255, g=255, b=255;
                            if (!colorStr.empty() && colorStr[0] == '#') {
                                unsigned int hex;
                                sscanf(colorStr.c_str()+1, "%06x", &hex);
                                r = (hex >> 16) & 0xFF;
                                g = (hex >> 8) & 0xFF;
                                b = hex & 0xFF;
                            }
                            float creatorX = creatorX_find_default[col];
                            float creatorY = creatorY_find_default[col] + row * rowSpacing - 20;
                            int drawX = (int)(creatorX * scaleX);
                            int drawY = (int)(creatorY * scaleY);
                            int drawW = (int)(creatorW_find * scaleX);
                            int drawH = (int)(creatorH_find * scaleY);
                            float scale = 1.5f;
                            int charWidth = (int)((8 + 1) * scale);
                            int textWidth = (int)(creator.length() * charWidth);
                            if (textWidth > drawW) {
                                scale = (float)drawW / (creator.length() * (8 + 1));
                                scale *= 0.9f;
                                if (scale < 0.5f) scale = 0.5f;
                                charWidth = (int)((8 + 1) * scale);
                                textWidth = (int)(creator.length() * charWidth);
                            }
                            int centeredX = drawX + (drawW - textWidth) / 2;
                            drawWrappedText(g_ren, creator, centeredX, drawY, drawW, drawH, scale, r, g, b);
                        }
                    }

                    if (pagesTex) {
                        int dstX = (int)(pagesX * scaleX);
                        int dstY = (int)(pagesY * scaleY);
                        int dstW = (int)(pagesW * scaleX);
                        int dstH = (int)(pagesH * scaleY);
                        SDL_Rect dst = {dstX, dstY, dstW, dstH};
                        SDL_RenderCopy(g_ren, pagesTex, nullptr, &dst);
                    }
                    if (back1Tex) {
                        int dstX = (int)(back1X * scaleX);
                        int dstY = (int)(back1Y * scaleY);
                        int dstW = (int)(back1W * scaleX);
                        int dstH = (int)(back1H * scaleY);
                        SDL_Rect dst = {dstX, dstY, dstW, dstH};
                        SDL_RenderCopy(g_ren, back1Tex, nullptr, &dst);
                    }
                    int totalFiltered2 = filteredGames.size();
                    int totalPagesFiltered2 = (totalFiltered2 + itemsPerPage - 1) / itemsPerPage;
                    if (totalPagesFiltered2 == 0) totalPagesFiltered2 = 1;
                    std::string pageText = std::to_string(currentPage+1) + "/" + std::to_string(totalPagesFiltered2);
                    int tw, th;
                    SDL_Texture* pageTxtTex = renderStringToTexture(pageText, g_ren, &tw, &th);
                    if (pageTxtTex) {
                        float textScale = 1.5f;
                        int textW = (int)(tw * textScale);
                        int textH = (int)(th * textScale);
                        int padW = 20, padH = 10;
                        int bgW = textW + padW;
                        int bgH = textH + padH;
                        if (bgW < 66) bgW = 66;
                        if (bgH < 44) bgH = 44;
                        if (backResize2Tex) {
                            SDL_Rect bgDst = {
                                (int)(backResize2X * scaleX),
                                (int)(backResize2Y * scaleY),
                                (int)(bgW * scaleX),
                                (int)(bgH * scaleY)
                            };
                            SDL_RenderCopy(g_ren, backResize2Tex, nullptr, &bgDst);
                        }
                        int textDstX = (int)((backResize2X + (bgW - textW)/2) * scaleX);
                        int textDstY = (int)((backResize2Y + (bgH - textH)/2) * scaleY);
                        SDL_Rect textDst = {textDstX, textDstY, textW, textH};
                        SDL_RenderCopy(g_ren, pageTxtTex, nullptr, &textDst);
                        SDL_DestroyTexture(pageTxtTex);
                    }
                }
            }
        }

        if (currentState == STATE_APPS) {
            float scaleX = (float)win_w / 1024.0f;
            float scaleY = (float)win_h / 600.0f;

            std::vector<int> filteredGames;
            if (selectedMain == 1) {
                for (size_t i = 0; i < g_storeGames.size(); ++i) filteredGames.push_back(i);
            } else {
                Category targetCat;
                if (selectedMain == 2) targetCat = CAT_TOOL;
                else if (selectedMain == 3) targetCat = CAT_GAME;
                else if (selectedMain == 4) targetCat = CAT_EMULATOR;
                else targetCat = CAT_GAME;
                for (size_t i = 0; i < g_storeGames.size(); ++i) {
                    if (getCategory(g_storeGames[i].category) == targetCat) filteredGames.push_back(i);
                }
            }
            int totalFiltered = filteredGames.size();
            int totalPagesFiltered = (totalFiltered + itemsPerPage - 1) / itemsPerPage;
            if (totalPagesFiltered == 0) totalPagesFiltered = 1;
            if (currentPage >= totalPagesFiltered) currentPage = totalPagesFiltered - 1;

            int startIdx = currentPage * itemsPerPage;
            int endIdx = std::min(startIdx + itemsPerPage, totalFiltered);

            if (selectedGame == -1 || std::find(filteredGames.begin()+startIdx, filteredGames.begin()+endIdx, selectedGame) == filteredGames.begin()+endIdx) {
                if (startIdx < totalFiltered) selectedGame = filteredGames[startIdx];
                else if (!filteredGames.empty()) selectedGame = filteredGames[0];
                else selectedGame = -1;
            }

            for (int p = startIdx; p < endIdx; ++p) {
                int i = filteredGames[p];
                int pos = p - startIdx;
                int row = pos / 4;
                int col = pos % 4;

                float gameX = gameX_find_default[col];
                float gameY = gameY_find_default[col] + row * rowSpacing - 20;
                float gameW = gameW_find_default[col];
                float gameH = gameH_find_default[col];

                int dstX = (int)(gameX * scaleX);
                int dstY = (int)(gameY * scaleY);
                int dstW = (int)(gameW * scaleX);
                int dstH = (int)(gameH * scaleY);

                if (blankTex) {
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    SDL_RenderCopy(g_ren, blankTex, nullptr, &dst);
                }
                SDL_Texture* tex = (i < (int)g_gameTextures.size()) ? g_gameTextures[i] : nullptr;
                if (tex && tex != blankTex) {
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    renderTextureWithAspectAndClip(g_ren, tex, &dst);
                } else if (!tex && !blankTex) {
                    SDL_SetRenderDrawColor(g_ren, 100,100,100,255);
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    SDL_RenderFillRect(g_ren, &dst);
                }

                if (selectedGame != -1 && i == selectedGame && selectOverlayTex) {
                    int overlayX = dstX + (dstW - (int)(SELECT_OVERLAY_W * scaleX)) / 2;
                    int overlayY = dstY + (dstH - (int)(SELECT_OVERLAY_H * scaleY)) / 2;
                    SDL_Rect overlayDst = {
                        overlayX,
                        overlayY,
                        (int)(SELECT_OVERLAY_W * scaleX),
                        (int)(SELECT_OVERLAY_H * scaleY)
                    };
                    SDL_SetTextureBlendMode(selectOverlayTex, SDL_BLENDMODE_BLEND);
                    SDL_RenderCopy(g_ren, selectOverlayTex, nullptr, &overlayDst);
                }

                SDL_Texture* arrowTex = downTex;
                if (i < (int)g_installedGames.size() && g_installedGames[i]) {
                    if (i < (int)g_needUpdate.size() && g_needUpdate[i]) {
                        arrowTex = upTex;
                    } else {
                        arrowTex = checkTex;
                    }
                }
                if (arrowTex) {
                    float downX = downX_find_default[col];
                    float downY = downY_find_default[col] + row * rowSpacing - 20;
                    int drawX = (int)(downX * scaleX);
                    int drawY = (int)(downY * scaleY);
                    int drawW = (int)(downW_find * scaleX);
                    int drawH = (int)(downH_find * scaleY);
                    SDL_Rect dst = {drawX, drawY, drawW, drawH};
                    SDL_RenderCopy(g_ren, arrowTex, nullptr, &dst);
                }
                {
                    std::string title = (i < (int)g_storeGames.size()) ? g_storeGames[i].title : "Unknown";
                    float titleX = titleX_find_default[col];
                    float titleY = titleY_find_default[col] + row * rowSpacing - 20;
                    int drawX = (int)(titleX * scaleX);
                    int drawY = (int)(titleY * scaleY);
                    int drawW = (int)(titleW_find * scaleX);
                    int drawH = (int)(titleH_find * scaleY);
                    float scale = 1.5f;
                    int charWidth = (int)((8 + 1) * scale);
                    int textWidth = (int)(title.length() * charWidth);
                    if (textWidth > drawW) {
                        scale = (float)drawW / (title.length() * (8 + 1));
                        scale *= 0.9f;
                        if (scale < 0.5f) scale = 0.5f;
                        charWidth = (int)((8 + 1) * scale);
                        textWidth = (int)(title.length() * charWidth);
                    }
                    int centeredX = drawX + (drawW - textWidth) / 2;
                    drawWrappedText(g_ren, title, centeredX, drawY, drawW, drawH, scale);
                }
                {
                    std::string creator = (i < (int)g_storeGames.size()) ? g_storeGames[i].author : "Unknown";
                    std::string colorStr = (i < (int)g_storeGames.size()) ? g_storeGames[i].author : "";
                    Uint8 r=255, g=255, b=255;
                    if (!colorStr.empty() && colorStr[0] == '#') {
                        unsigned int hex;
                        sscanf(colorStr.c_str()+1, "%06x", &hex);
                        r = (hex >> 16) & 0xFF;
                        g = (hex >> 8) & 0xFF;
                        b = hex & 0xFF;
                    }
                    float creatorX = creatorX_find_default[col];
                    float creatorY = creatorY_find_default[col] + row * rowSpacing - 20;
                    int drawX = (int)(creatorX * scaleX);
                    int drawY = (int)(creatorY * scaleY);
                    int drawW = (int)(creatorW_find * scaleX);
                    int drawH = (int)(creatorH_find * scaleY);
                    float scale = 1.5f;
                    int charWidth = (int)((8 + 1) * scale);
                    int textWidth = (int)(creator.length() * charWidth);
                    if (textWidth > drawW) {
                        scale = (float)drawW / (creator.length() * (8 + 1));
                        scale *= 0.9f;
                        if (scale < 0.5f) scale = 0.5f;
                        charWidth = (int)((8 + 1) * scale);
                        textWidth = (int)(creator.length() * charWidth);
                    }
                    int centeredX = drawX + (drawW - textWidth) / 2;
                    drawWrappedText(g_ren, creator, centeredX, drawY, drawW, drawH, scale, r, g, b);
                }
            }

            if (pagesTex) {
                int dstX = (int)(pagesX * scaleX);
                int dstY = (int)(pagesY * scaleY);
                int dstW = (int)(pagesW * scaleX);
                int dstH = (int)(pagesH * scaleY);
                SDL_Rect dst = {dstX, dstY, dstW, dstH};
                SDL_RenderCopy(g_ren, pagesTex, nullptr, &dst);
            }
            if (back1Tex) {
                int dstX = (int)(back1X * scaleX);
                int dstY = (int)(back1Y * scaleY);
                int dstW = (int)(back1W * scaleX);
                int dstH = (int)(back1H * scaleY);
                SDL_Rect dst = {dstX, dstY, dstW, dstH};
                SDL_RenderCopy(g_ren, back1Tex, nullptr, &dst);
            }
            std::string pageText = std::to_string(currentPage+1) + "/" + std::to_string(totalPagesFiltered);
            int tw, th;
            SDL_Texture* pageTxtTex = renderStringToTexture(pageText, g_ren, &tw, &th);
            if (pageTxtTex) {
                float textScale = 1.5f;
                int textW = (int)(tw * textScale);
                int textH = (int)(th * textScale);
                int padW = 20, padH = 10;
                int bgW = textW + padW;
                int bgH = textH + padH;
                if (bgW < 66) bgW = 66;
                if (bgH < 44) bgH = 44;
                if (backResize2Tex) {
                    SDL_Rect bgDst = {
                        (int)(backResize2X * scaleX),
                        (int)(backResize2Y * scaleY),
                        (int)(bgW * scaleX),
                        (int)(bgH * scaleY)
                    };
                    SDL_RenderCopy(g_ren, backResize2Tex, nullptr, &bgDst);
                }
                int textDstX = (int)((backResize2X + (bgW - textW)/2) * scaleX);
                int textDstY = (int)((backResize2Y + (bgH - textH)/2) * scaleY);
                SDL_Rect textDst = {textDstX, textDstY, textW, textH};
                SDL_RenderCopy(g_ren, pageTxtTex, nullptr, &textDst);
                SDL_DestroyTexture(pageTxtTex);
            }
        }

        if (currentState == STATE_FIND) {
            float scaleX = (float)win_w / 1024.0f;
            float scaleY = (float)win_h / 600.0f;

            std::vector<int> filteredIndices;
            for (size_t i = 0; i < g_storeGames.size(); ++i) {
                std::string title = g_storeGames[i].title;
                std::string lowerTitle = title;
                std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
                std::string lowerSearch = searchString;
                std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
                if (lowerTitle.find(lowerSearch) != std::string::npos) {
                    filteredIndices.push_back(i);
                }
            }
            int totalFiltered = filteredIndices.size();
            int totalPages = (totalFiltered + itemsPerPage - 1) / itemsPerPage;
            if (totalPages == 0) totalPages = 1;
            if (findCurrentPage >= totalPages) findCurrentPage = totalPages - 1;
            int startIdx = findCurrentPage * itemsPerPage;
            int endIdx = std::min(startIdx + itemsPerPage, totalFiltered);

            for (int pos = startIdx; pos < endIdx; ++pos) {
                int i = filteredIndices[pos];
                int row = (pos - startIdx) / 4;
                int col = (pos - startIdx) % 4;

                float gameX = gameX_find_default[col];
                float gameY = gameY_find_default[col] + row * rowSpacing - 20;
                float gameW = gameW_find_default[col];
                float gameH = gameH_find_default[col];

                int dstX = (int)(gameX * scaleX);
                int dstY = (int)(gameY * scaleY);
                int dstW = (int)(gameW * scaleX);
                int dstH = (int)(gameH * scaleY);

                if (blankTex) {
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    SDL_RenderCopy(g_ren, blankTex, nullptr, &dst);
                }
                SDL_Texture* tex = (i < (int)g_gameTextures.size()) ? g_gameTextures[i] : nullptr;
                if (tex && tex != blankTex) {
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    renderTextureWithAspectAndClip(g_ren, tex, &dst);
                } else if (!tex && !blankTex) {
                    SDL_SetRenderDrawColor(g_ren, 100,100,100,255);
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    SDL_RenderFillRect(g_ren, &dst);
                }

                if (!keyboardVisible && selectedGameFind != -1 && selectedGameFind == i && selectOverlayTex) {
                    int overlayX = dstX + (dstW - (int)(SELECT_OVERLAY_W * scaleX)) / 2;
                    int overlayY = dstY + (dstH - (int)(SELECT_OVERLAY_H * scaleY)) / 2;
                    SDL_Rect overlayDst = {
                        overlayX,
                        overlayY,
                        (int)(SELECT_OVERLAY_W * scaleX),
                        (int)(SELECT_OVERLAY_H * scaleY)
                    };
                    SDL_SetTextureBlendMode(selectOverlayTex, SDL_BLENDMODE_BLEND);
                    SDL_RenderCopy(g_ren, selectOverlayTex, nullptr, &overlayDst);
                }

                SDL_Texture* arrowTex = downTex;
                if (i < (int)g_installedGames.size() && g_installedGames[i]) {
                    if (i < (int)g_needUpdate.size() && g_needUpdate[i]) {
                        arrowTex = upTex;
                    } else {
                        arrowTex = checkTex;
                    }
                }
                if (arrowTex) {
                    float downX = downX_find_default[col];
                    float downY = downY_find_default[col] + row * rowSpacing - 20;
                    int drawX = (int)(downX * scaleX);
                    int drawY = (int)(downY * scaleY);
                    int drawW = (int)(downW_find * scaleX);
                    int drawH = (int)(downH_find * scaleY);
                    SDL_Rect dst = {drawX, drawY, drawW, drawH};
                    SDL_RenderCopy(g_ren, arrowTex, nullptr, &dst);
                }
                {
                    std::string title = (i < (int)g_storeGames.size()) ? g_storeGames[i].title : "Unknown";
                    float titleX = titleX_find_default[col];
                    float titleY = titleY_find_default[col] + row * rowSpacing - 20;
                    int drawX = (int)(titleX * scaleX);
                    int drawY = (int)(titleY * scaleY);
                    int drawW = (int)(titleW_find * scaleX);
                    int drawH = (int)(titleH_find * scaleY);
                    float scale = 1.5f;
                    int charWidth = (int)((8 + 1) * scale);
                    int textWidth = (int)(title.length() * charWidth);
                    if (textWidth > drawW) {
                        scale = (float)drawW / (title.length() * (8 + 1));
                        scale *= 0.9f;
                        if (scale < 0.5f) scale = 0.5f;
                        charWidth = (int)((8 + 1) * scale);
                        textWidth = (int)(title.length() * charWidth);
                    }
                    int centeredX = drawX + (drawW - textWidth) / 2;
                    drawWrappedText(g_ren, title, centeredX, drawY, drawW, drawH, scale);
                }
                {
                    std::string creator = (i < (int)g_storeGames.size()) ? g_storeGames[i].author : "Unknown";
                    std::string colorStr = (i < (int)g_storeGames.size()) ? g_storeGames[i].author : "";
                    Uint8 r=255, g=255, b=255;
                    if (!colorStr.empty() && colorStr[0] == '#') {
                        unsigned int hex;
                        sscanf(colorStr.c_str()+1, "%06x", &hex);
                        r = (hex >> 16) & 0xFF;
                        g = (hex >> 8) & 0xFF;
                        b = hex & 0xFF;
                    }
                    float creatorX = creatorX_find_default[col];
                    float creatorY = creatorY_find_default[col] + row * rowSpacing - 20;
                    int drawX = (int)(creatorX * scaleX);
                    int drawY = (int)(creatorY * scaleY);
                    int drawW = (int)(creatorW_find * scaleX);
                    int drawH = (int)(creatorH_find * scaleY);
                    float scale = 1.5f;
                    int charWidth = (int)((8 + 1) * scale);
                    int textWidth = (int)(creator.length() * charWidth);
                    if (textWidth > drawW) {
                        scale = (float)drawW / (creator.length() * (8 + 1));
                        scale *= 0.9f;
                        if (scale < 0.5f) scale = 0.5f;
                        charWidth = (int)((8 + 1) * scale);
                        textWidth = (int)(creator.length() * charWidth);
                    }
                    int centeredX = drawX + (drawW - textWidth) / 2;
                    drawWrappedText(g_ren, creator, centeredX, drawY, drawW, drawH, scale, r, g, b);
                }
            }

            if (keyboardVisible && findKeyback2Tex) {
                int dstX = (int)(keyback2X * scaleX);
                int dstY = (int)(keyback2Y * scaleY);
                int dstW = (int)(keyback2W * scaleX);
                int dstH = (int)(keyback2H * scaleY);
                SDL_Rect dst = {dstX, dstY, dstW, dstH};
                SDL_RenderCopy(g_ren, findKeyback2Tex, nullptr, &dst);
            }
            if (keyboardVisible && findKeybackTex) {
                SDL_Rect kbRect = {0, (int)(400*scaleY), win_w, (int)(200*scaleY)};
                SDL_RenderCopy(g_ren, findKeybackTex, nullptr, &kbRect);
            }

            if (keyboardVisible) {
                for (size_t i = 0; i < keys.size(); ++i) {
                    SDL_Texture* tex = (i == selectedKey) ? keySelectTex[i] : keyNormalTex[i];
                    if (!tex) continue;
                    int dstX = (int)(keys[i].x * scaleX);
                    int dstY = (int)(keys[i].y * scaleY);
                    int dstW = (int)(keys[i].w * scaleX);
                    int dstH = (int)(keys[i].h * scaleY);
                    SDL_Rect dst = {dstX, dstY, dstW, dstH};
                    SDL_RenderCopy(g_ren, tex, nullptr, &dst);
                }
            }

            if (findBackTex) {
                int dstX = (int)(backX * scaleX);
                int dstY = (int)(backY * scaleY);
                int dstW = (int)(backW * scaleX);
                int dstH = (int)(backH * scaleY);
                SDL_Rect dst = {dstX, dstY, dstW, dstH};
                SDL_RenderCopy(g_ren, findBackTex, nullptr, &dst);
            }
            if (find2Tex) {
                int dstX = (int)(find2X * scaleX);
                int dstY = (int)(find2Y * scaleY);
                int dstW = (int)(find2W * scaleX);
                int dstH = (int)(find2H * scaleY);
                SDL_Rect dst = {dstX, dstY, dstW, dstH};
                SDL_RenderCopy(g_ren, find2Tex, nullptr, &dst);
            }

            if (!searchString.empty()) {
                int tw, th;
                SDL_Texture* txt = renderStringToTexture(searchString, g_ren, &tw, &th);
                if (txt) {
                    int drawW = (int)(tw * FIND_TEXT_SCALE);
                    int drawH = (int)(th * FIND_TEXT_SCALE);
                    int dstX = (int)(searchTextX * scaleX);
                    int dstY = (int)(searchTextY * scaleY + (searchTextH * scaleY - drawH) / 2);
                    SDL_Rect dst = {dstX, dstY, drawW, drawH};
                    SDL_RenderCopy(g_ren, txt, nullptr, &dst);
                    SDL_DestroyTexture(txt);
                }
            }

            if (findYkeyTex) {
                int dstX = (int)(findYkeyX * scaleX);
                int dstY = (int)(findYkeyY * scaleY);
                int dstW = (int)(findYkeyW * scaleX);
                int dstH = (int)(findYkeyH * scaleY);
                SDL_Rect dst = {dstX, dstY, dstW, dstH};
                SDL_RenderCopy(g_ren, findYkeyTex, nullptr, &dst);
            }

            if (pagesTex) {
                int dstX = (int)(pagesX * scaleX);
                int dstY = (int)(pagesY * scaleY);
                int dstW = (int)(pagesW * scaleX);
                int dstH = (int)(pagesH * scaleY);
                SDL_Rect dst = {dstX, dstY, dstW, dstH};
                SDL_RenderCopy(g_ren, pagesTex, nullptr, &dst);
            }
            if (back1Tex) {
                int dstX = (int)(back1X * scaleX);
                int dstY = (int)(back1Y * scaleY);
                int dstW = (int)(back1W * scaleX);
                int dstH = (int)(back1H * scaleY);
                SDL_Rect dst = {dstX, dstY, dstW, dstH};
                SDL_RenderCopy(g_ren, back1Tex, nullptr, &dst);
            }
            std::string pageText = std::to_string(findCurrentPage+1) + "/" + std::to_string(totalPages);
            int tw, th;
            SDL_Texture* pageTxtTex = renderStringToTexture(pageText, g_ren, &tw, &th);
            if (pageTxtTex) {
                float textScale = 1.5f;
                int textW = (int)(tw * textScale);
                int textH = (int)(th * textScale);
                int padW = 20, padH = 10;
                int bgW = textW + padW;
                int bgH = textH + padH;
                if (bgW < 66) bgW = 66;
                if (bgH < 44) bgH = 44;
                if (backResize2Tex) {
                    SDL_Rect bgDst = {
                        (int)(backResize2X * scaleX),
                        (int)(backResize2Y * scaleY),
                        (int)(bgW * scaleX),
                        (int)(bgH * scaleY)
                    };
                    SDL_RenderCopy(g_ren, backResize2Tex, nullptr, &bgDst);
                }
                int textDstX = (int)((backResize2X + (bgW - textW)/2) * scaleX);
                int textDstY = (int)((backResize2Y + (bgH - textH)/2) * scaleY);
                SDL_Rect textDst = {textDstX, textDstY, textW, textH};
                SDL_RenderCopy(g_ren, pageTxtTex, nullptr, &textDst);
                SDL_DestroyTexture(pageTxtTex);
            }
        }

        if (currentState == STATE_DOWNLOAD && selectedDownloadGame >= 0) {
            float scaleX = (float)win_w / 1024.0f;
            float scaleY = (float)win_h / 600.0f;

            if (downloadBackTex) {
                SDL_Rect dst = {
                    (int)(dwBackX * scaleX), (int)(dwBackY * scaleY),
                    (int)(dwBackW * scaleX), (int)(dwBackH * scaleY)
                };
                SDL_RenderCopy(g_ren, downloadBackTex, nullptr, &dst);
            }
            if (downloadBack1Tex) {
                SDL_Rect dst = {
                    (int)(dwBack1X * scaleX), (int)(dwBack1Y * scaleY),
                    (int)(dwBack1W * scaleX), (int)(dwBack1H * scaleY)
                };
                SDL_RenderCopy(g_ren, downloadBack1Tex, nullptr, &dst);
            }
            if (downloadBorderTex) {
                SDL_Rect dst = {
                    (int)(dwBorderX * scaleX), (int)(dwBorderY * scaleY),
                    (int)(dwBorderW * scaleX), (int)(dwBorderH * scaleY)
                };
                SDL_RenderCopy(g_ren, downloadBorderTex, nullptr, &dst);
            }

            if (selectedDownloadGame >= 0 && selectedDownloadGame < (int)g_gameTextures.size()) {
                SDL_Texture* iconTex = g_gameTextures[selectedDownloadGame];
                if (iconTex) {
                    SDL_Rect dst = {
                        (int)(dwIconX * scaleX), (int)(dwIconY * scaleY),
                        (int)(dwIconW * scaleX), (int)(dwIconH * scaleY)
                    };
                    if (iconTex != blankTex) {
                        renderTextureWithAspectAndClip(g_ren, iconTex, &dst);
                    } else {
                        SDL_RenderCopy(g_ren, iconTex, nullptr, &dst);
                    }
                }
            }

            bool isInstalled = (selectedDownloadGame < (int)g_installedGames.size() && g_installedGames[selectedDownloadGame]);
            bool needUpdate = (selectedDownloadGame < (int)g_needUpdate.size() && g_needUpdate[selectedDownloadGame]);

            SDL_Texture* arrowOverlay = downTex;
            if (isInstalled) {
                if (needUpdate) {
                    arrowOverlay = upTex;
                } else {
                    arrowOverlay = checkTex;
                }
            }
            if (arrowOverlay) {
                SDL_Rect dst = {
                    (int)(dwDownX * scaleX), (int)(dwDownY * scaleY),
                    (int)(dwDownW * scaleX), (int)(dwDownH * scaleY)
                };
                SDL_RenderCopy(g_ren, arrowOverlay, nullptr, &dst);
            }

            if (downloadDesTex) {
                SDL_Rect dst = {
                    (int)(dwDesX * scaleX), (int)(dwDesY * scaleY),
                    (int)(dwDesW * scaleX), (int)(dwDesH * scaleY)
                };
                SDL_RenderCopy(g_ren, downloadDesTex, nullptr, &dst);
            }

            {
                std::string desc = (selectedDownloadGame < (int)g_storeGames.size()) ? g_storeGames[selectedDownloadGame].description : "No description available.";
                int maxW = (int)(dwDescTextW * scaleX);
                int maxH = (int)(dwDescTextH * scaleY);
                int startX = (int)(dwDescTextX * scaleX);
                int startY = (int)(dwDescTextY * scaleY);
                float textScale = 2.0f;
                drawWrappedText(g_ren, desc, startX, startY, maxW, maxH, textScale);
            }

            if (downloadTitleTex) {
                SDL_Rect dst = {
                    (int)(dwTitleIconX * scaleX), (int)(dwTitleIconY * scaleY),
                    (int)(dwTitleIconW * scaleX), (int)(dwTitleIconH * scaleY)
                };
                SDL_RenderCopy(g_ren, downloadTitleTex, nullptr, &dst);
            }
            {
                std::string title = (selectedDownloadGame < (int)g_storeGames.size()) ? g_storeGames[selectedDownloadGame].title : "Unknown";
                int dw = (int)(dwTitleTextW * scaleX);
                int dh = (int)(dwTitleTextH * scaleY);
                int dx = (int)(dwTitleTextX * scaleX);
                int dy = (int)(dwTitleTextY * scaleY);
                float textScale = 2.2f;
                int textHeight = (int)(8 * textScale);
                int startY = dy + (dh - textHeight) / 2;
                drawWrappedText(g_ren, title, dx, startY, dw, dh, textScale);
            }

            if (downloadAuthorTex) {
                SDL_Rect dst = {
                    (int)(dwAuthorIconX * scaleX), (int)(dwAuthorIconY * scaleY),
                    (int)(dwAuthorIconW * scaleX), (int)(dwAuthorIconH * scaleY)
                };
                SDL_RenderCopy(g_ren, downloadAuthorTex, nullptr, &dst);
            }
            {
                std::string author = (selectedDownloadGame < (int)g_storeGames.size()) ? g_storeGames[selectedDownloadGame].author : "Unknown";
                std::string colorStr = (selectedDownloadGame < (int)g_storeGames.size()) ? g_storeGames[selectedDownloadGame].author : "";
                Uint8 r=255, g=255, b=255;
                if (!colorStr.empty() && colorStr[0] == '#') {
                    unsigned int hex;
                    sscanf(colorStr.c_str()+1, "%06x", &hex);
                    r = (hex >> 16) & 0xFF;
                    g = (hex >> 8) & 0xFF;
                    b = hex & 0xFF;
                }
                int dw = (int)(dwAuthorTextW * scaleX);
                int dh = (int)(dwAuthorTextH * scaleY);
                int dx = (int)(dwAuthorTextX * scaleX);
                int dy = (int)(dwAuthorTextY * scaleY);
                float textScale = 2.2f;
                int textHeight = (int)(8 * textScale);
                int startY = dy + (dh - textHeight) / 2;
                drawWrappedText(g_ren, author, dx, startY, dw, dh, textScale, r, g, b);
            }

            if (downloadSizeTex) {
                SDL_Rect dst = {
                    (int)(dwSizeIconX * scaleX), (int)(dwSizeIconY * scaleY),
                    (int)(dwSizeIconW * scaleX), (int)(dwSizeIconH * scaleY)
                };
                SDL_RenderCopy(g_ren, downloadSizeTex, nullptr, &dst);
            }
            {
                std::string size = (selectedDownloadGame < (int)g_storeGames.size()) ? g_storeGames[selectedDownloadGame].file_size : "—";
                int dw = (int)(dwSizeTextW * scaleX);
                int dh = (int)(dwSizeTextH * scaleY);
                int dx = (int)(dwSizeTextX * scaleX);
                int dy = (int)(dwSizeTextY * scaleY);
                float textScale = 2.2f;
                int textHeight = (int)(8 * textScale);
                int startY = dy + (dh - textHeight) / 2;
                drawWrappedText(g_ren, size, dx, startY, dw, dh, textScale);
            }

            if (downloadVerTex) {
                SDL_Rect dst = {
                    (int)(dwVerIconX * scaleX), (int)(dwVerIconY * scaleY),
                    (int)(dwVerIconW * scaleX), (int)(dwVerIconH * scaleY)
                };
                SDL_RenderCopy(g_ren, downloadVerTex, nullptr, &dst);
            }
            {
                std::string ver = (selectedDownloadGame < (int)g_storeGames.size()) ? g_storeGames[selectedDownloadGame].version : "1.0";
                int dw = (int)(dwVerTextW * scaleX);
                int dh = (int)(dwVerTextH * scaleY);
                int dx = (int)(dwVerTextX * scaleX);
                int dy = (int)(dwVerTextY * scaleY);
                float textScale = 2.2f;
                int textHeight = (int)(8 * textScale);
                int startY = dy + (dh - textHeight) / 2;
                drawWrappedText(g_ren, ver, dx, startY, dw, dh, textScale);
            }

            if (downloadDateTex) {
                SDL_Rect dst = {
                    (int)(dwDateIconX * scaleX), (int)(dwDateIconY * scaleY),
                    (int)(dwDateIconW * scaleX), (int)(dwDateIconH * scaleY)
                };
                SDL_RenderCopy(g_ren, downloadDateTex, nullptr, &dst);
            }
            {
                std::string date = (selectedDownloadGame < (int)g_storeGames.size()) ? g_storeGames[selectedDownloadGame].created_at : "2024-01-01";
                size_t pos = date.find('T');
                if (pos != std::string::npos) date = date.substr(0, pos);
                int dw = (int)(dwDateTextW * scaleX);
                int dh = (int)(dwDateTextH * scaleY);
                int dx = (int)(dwDateTextX * scaleX);
                int dy = (int)(dwDateTextY * scaleY);
                float textScale = 2.2f;
                int textHeight = (int)(8 * textScale);
                int startY = dy + (dh - textHeight) / 2;
                drawWrappedText(g_ren, date, dx, startY, dw, dh, textScale);
            }

            if (categoryLabelTex) {
                SDL_Rect dst = {
                    (int)(categoryLabelX * scaleX),
                    (int)(categoryLabelY * scaleY),
                    (int)(categoryLabelW * scaleX),
                    (int)(categoryLabelH * scaleY)
                };
                SDL_RenderCopy(g_ren, categoryLabelTex, nullptr, &dst);
            }
            std::string catName = (selectedDownloadGame < (int)g_storeGames.size()) ? getCategoryName(getCategory(g_storeGames[selectedDownloadGame].category)) : "Game";
            int tw, th;
            SDL_Texture* catNameTex = renderStringToTexture(catName, g_ren, &tw, &th);
            if (catNameTex) {
                float textScale = 1.4f;
                int textW = (int)(tw * textScale);
                int textH = (int)(th * textScale);
                int x = (int)(categoryTextX * scaleX);
                int y = (int)(categoryTextY * scaleY + (categoryLabelH * scaleY - textH) / 2);
                SDL_Rect dst = {x, y, textW, textH};
                SDL_RenderCopy(g_ren, catNameTex, nullptr, &dst);
                SDL_DestroyTexture(catNameTex);
            }

            if (g_downloadStatus == 0 || g_downloadStatus == 3) {
                if (isInstalled) {
                    if (downloadRemoveBtnTex) {
                        SDL_Rect dst = {
                            (int)(dwDlBtnX * scaleX), (int)(dwDlBtnY * scaleY),
                            (int)(dwDlBtnW * scaleX), (int)(dwDlBtnH * scaleY)
                        };
                        SDL_RenderCopy(g_ren, downloadRemoveBtnTex, nullptr, &dst);
                    }
                } else {
                    if (downloadBtnTex) {
                        SDL_Rect dst = {
                            (int)(dwDlBtnX * scaleX), (int)(dwDlBtnY * scaleY),
                            (int)(dwDlBtnW * scaleX), (int)(dwDlBtnH * scaleY)
                        };
                        SDL_RenderCopy(g_ren, downloadBtnTex, nullptr, &dst);
                    }
                }
            }

            if (downloadBackBtnTex) {
                SDL_Rect dst = {
                    (int)(dwBackBtnX * scaleX), (int)(dwBackBtnY * scaleY),
                    (int)(dwBackBtnW * scaleX), (int)(dwBackBtnH * scaleY)
                };
                SDL_RenderCopy(g_ren, downloadBackBtnTex, nullptr, &dst);
            }

            if (g_downloadStatus == 1 || g_downloadStatus == 2) {
                if (downloadInstallTex) {
                    SDL_Rect bgDst = {
                        (int)(dwInstallX * scaleX), (int)(dwInstallY * scaleY),
                        (int)(dwInstallW * scaleX), (int)(dwInstallH * scaleY)
                    };
                    SDL_RenderCopy(g_ren, downloadInstallTex, nullptr, &bgDst);
                }

                if (g_downloadStatus == 1) {
                    if (g_isUpdate && downloadUpdatingTextTex) {
                        float pulse = 0.5f + 0.5f * sin((now - downloadStartTime) / 200.0f);
                        Uint8 alpha = (Uint8)(128 + 127 * pulse);
                        SDL_SetTextureAlphaMod(downloadUpdatingTextTex, alpha);
                        SDL_Rect dst = {
                            (int)(dwInstallTextX * scaleX), (int)(dwInstallTextY * scaleY),
                            (int)(dwInstallTextW * scaleX), (int)(dwInstallTextH * scaleY)
                        };
                        SDL_RenderCopy(g_ren, downloadUpdatingTextTex, nullptr, &dst);
                        SDL_SetTextureAlphaMod(downloadUpdatingTextTex, 255);
                    } else if (downloadInstallTextTex) {
                        float pulse = 0.5f + 0.5f * sin((now - downloadStartTime) / 200.0f);
                        Uint8 alpha = (Uint8)(128 + 127 * pulse);
                        SDL_SetTextureAlphaMod(downloadInstallTextTex, alpha);
                        SDL_Rect dst = {
                            (int)(dwInstallTextX * scaleX), (int)(dwInstallTextY * scaleY),
                            (int)(dwInstallTextW * scaleX), (int)(dwInstallTextH * scaleY)
                        };
                        SDL_RenderCopy(g_ren, downloadInstallTextTex, nullptr, &dst);
                        SDL_SetTextureAlphaMod(downloadInstallTextTex, 255);
                    }
                } else if (g_downloadStatus == 2) {
                    if (downloadInstallText2Tex) {
                        float pulse = 0.5f + 0.5f * sin((now - installStartTime) / 200.0f);
                        Uint8 alpha = (Uint8)(128 + 127 * pulse);
                        SDL_SetTextureAlphaMod(downloadInstallText2Tex, alpha);
                        SDL_Rect dst = {
                            (int)(INSTALL_TEXT2_X * scaleX), (int)(INSTALL_TEXT2_Y * scaleY),
                            (int)(INSTALL_TEXT2_W * scaleX), (int)(INSTALL_TEXT2_H * scaleY)
                        };
                        SDL_RenderCopy(g_ren, downloadInstallText2Tex, nullptr, &dst);
                        SDL_SetTextureAlphaMod(downloadInstallText2Tex, 255);
                    }
                }
            }

            if (isInstalled && g_downloadStatus != 2) {
                if (needUpdate) {
                    if (downloadUpdateTex) {
                        SDL_Rect dst = {
                            (int)(dwLaunchX * scaleX), (int)(dwLaunchY * scaleY),
                            (int)(dwLaunchW * scaleX), (int)(dwLaunchH * scaleY)
                        };
                        SDL_RenderCopy(g_ren, downloadUpdateTex, nullptr, &dst);
                    }
                } else {
                    if (downloadLaunchTex) {
                        SDL_Rect dst = {
                            (int)(dwLaunchX * scaleX), (int)(dwLaunchY * scaleY),
                            (int)(dwLaunchW * scaleX), (int)(dwLaunchH * scaleY)
                        };
                        SDL_RenderCopy(g_ren, downloadLaunchTex, nullptr, &dst);
                    }
                }
            }

            if (g_downloadStatus == 1) {
                int progress = g_downloadProgress.load();
                std::string progressText = std::to_string(progress) + "%";
                int px = (int)(downloadPercentX * scaleX);
                int py = (int)(downloadPercentY * scaleY);
                int pw = (int)(downloadPercentW * scaleX);
                int ph = (int)(downloadPercentH * scaleY);
                drawTextWithAspect(g_ren, progressText, px, py, pw, ph, 255, 255, 255);
            }
        }

        int sidebarIconIdx;
        if (currentState == STATE_MAIN) {
            sidebarIconIdx = selectedMain;
        } else if (currentState == STATE_APPS || currentState == STATE_DOWNLOAD) {
            sidebarIconIdx = (currentState == STATE_APPS) ? selectedMain : 1;
        } else {
            sidebarIconIdx = 0;
        }
        if (currentState == STATE_DOWNLOAD) {
            sidebarIconIdx = (prevState == STATE_FIND) ? 0 : selectedMain;
        }

        if (sidebarIconIdx >= 0 && sidebarIconIdx < (int)iconTextures.size() && iconTextures[sidebarIconIdx]) {
            int iconW = (int)(win_w * (261.6f / 1024.0f));
            SDL_Rect iconRect = {0, 0, iconW, win_h};
            SDL_RenderCopy(g_ren, iconTextures[sidebarIconIdx], nullptr, &iconRect);
        } else {
            SDL_SetRenderDrawColor(g_ren, 255,0,0,128);
            SDL_Rect fallback = {0, 0, (int)(win_w*0.2555f), win_h};
            SDL_RenderFillRect(g_ren, &fallback);
        }

        float scaleX = (float)win_w / 1024.0f;
        float scaleY = (float)win_h / 600.0f;

        if (clockTex) {
            int dstX = (int)(clockX * scaleX);
            int dstY = (int)(clockY * scaleY);
            int dstW = (int)(clockW * scaleX);
            int dstH = (int)(clockH * scaleY);
            SDL_Rect dst = {dstX, dstY, dstW, dstH};
            SDL_RenderCopy(g_ren, clockTex, nullptr, &dst);
        }
        if (dateTex) {
            int dstX = (int)(dateX * scaleX);
            int dstY = (int)(dateY * scaleY);
            int dstW = (int)(dateW_base * scaleX);
            int dstH = (int)(dateH_base * scaleY);
            SDL_Rect dst = {dstX, dstY, dstW, dstH};
            SDL_RenderCopy(g_ren, dateTex, nullptr, &dst);
        }
        if (timeTex) {
            int dstX = (int)(timeX * scaleX);
            int dstY = (int)(timeY * scaleY);
            int dstW = (int)(timeW_base * scaleX);
            int dstH = (int)(timeH_base * scaleY);
            SDL_Rect dst = {dstX, dstY, dstW, dstH};
            SDL_RenderCopy(g_ren, timeTex, nullptr, &dst);
        }
        if (ampmTex) {
            int dstX = (int)(ampmX * scaleX);
            int dstY = (int)(ampmY * scaleY);
            int dstW = (int)(ampmW_base * scaleX);
            int dstH = (int)(ampmH_base * scaleY);
            SDL_Rect dst = {dstX, dstY, dstW, dstH};
            SDL_RenderCopy(g_ren, ampmTex, nullptr, &dst);
        }

        if (percentTex) {
            int dstX = (int)(batteryPercentX * scaleX);
            int dstY = (int)(batteryPercentY * scaleY);
            int dstW = (int)(batteryPercentW * scaleX);
            int dstH = (int)(batteryPercentH * scaleY);
            SDL_Rect dst = {dstX, dstY, dstW, dstH};
            SDL_RenderCopy(g_ren, percentTex, nullptr, &dst);
        }

        SDL_Texture* battTex = nullptr;
        if (lastCharging) {
            battTex = chargingTex;
        } else if (lastPercent >= 0) {
            int idx = 0;
            if (lastPercent >= 75) idx = 3;
            else if (lastPercent >= 50) idx = 2;
            else if (lastPercent >= 25) idx = 1;
            else idx = 0;
            battTex = batteryTextures[idx];
        }
        if (battTex) {
            int dstX = (int)(961.7f * scaleX);
            int dstY = (int)(13.9f * scaleY);
            int dstW = (int)(49.0f * scaleX);
            int dstH = (int)(29.0f * scaleY);
            SDL_Rect dst = {dstX, dstY, dstW, dstH};
            SDL_RenderCopy(g_ren, battTex, nullptr, &dst);
        }

        SDL_RenderPresent(g_ren);
        SDL_Delay(16);
    }

    g_refreshStop = true;
    if (g_refreshThread.joinable()) g_refreshThread.join();

    g_stop = true;
    if (g_bgAudioThread.joinable()) g_bgAudioThread.join();
    if (g_downloadThread.joinable()) g_downloadThread.join();
    SDL_ClearQueuedAudio(g_audio);

    for (auto t : batteryTextures) if (t) SDL_DestroyTexture(t);
    if (chargingTex) SDL_DestroyTexture(chargingTex);
    if (percentTex) SDL_DestroyTexture(percentTex);
    if (downTex) SDL_DestroyTexture(downTex);
    if (checkTex) SDL_DestroyTexture(checkTex);
    if (upTex) SDL_DestroyTexture(upTex);
    if (clockTex) SDL_DestroyTexture(clockTex);
    if (dateTex) SDL_DestroyTexture(dateTex);
    if (timeTex) SDL_DestroyTexture(timeTex);
    if (ampmTex) SDL_DestroyTexture(ampmTex);
    if (pagesTex) SDL_DestroyTexture(pagesTex);
    if (back1Tex) SDL_DestroyTexture(back1Tex);
    if (backResize2Tex) SDL_DestroyTexture(backResize2Tex);
    if (categoryLabelTex) SDL_DestroyTexture(categoryLabelTex);
    if (findBackTex) SDL_DestroyTexture(findBackTex);
    if (findBack2Tex) SDL_DestroyTexture(findBack2Tex);
    if (find2Tex) SDL_DestroyTexture(find2Tex);
    if (findYkeyTex) SDL_DestroyTexture(findYkeyTex);
    if (findKeybackTex) SDL_DestroyTexture(findKeybackTex);
    if (findKeyback2Tex) SDL_DestroyTexture(findKeyback2Tex);
    for (auto t : keyNormalTex) if (t) SDL_DestroyTexture(t);
    for (auto t : keySelectTex) if (t) SDL_DestroyTexture(t);

    SDL_DestroyTexture(downloadBackTex);
    SDL_DestroyTexture(downloadBorderTex);
    SDL_DestroyTexture(downloadBack1Tex);
    SDL_DestroyTexture(downloadDesTex);
    SDL_DestroyTexture(downloadTitleTex);
    SDL_DestroyTexture(downloadAuthorTex);
    SDL_DestroyTexture(downloadSizeTex);
    SDL_DestroyTexture(downloadVerTex);
    SDL_DestroyTexture(downloadDateTex);
    SDL_DestroyTexture(downloadBtnTex);
    SDL_DestroyTexture(downloadRemoveBtnTex);
    SDL_DestroyTexture(downloadBackBtnTex);
    SDL_DestroyTexture(downloadInstallTex);
    SDL_DestroyTexture(downloadInstallTextTex);
    SDL_DestroyTexture(downloadInstallText2Tex);
    SDL_DestroyTexture(downloadLaunchTex);
    SDL_DestroyTexture(downloadUpdateTex);
    SDL_DestroyTexture(downloadUpdatingTextTex);
    SDL_DestroyTexture(selectOverlayTex);
    for (auto t : g_gameTextures) {
        if (t) {
            SDL_DestroyTexture(t);
        }
    }
    SDL_DestroyTexture(blankTex);
    SDL_DestroyTexture(boxTex);

    SDL_DestroyTexture(bg2Tex);
    SDL_DestroyTexture(topBarTex);
    SDL_DestroyTexture(bgFindTex);
    for (auto t : iconTextures) if (t) SDL_DestroyTexture(t);

    SDL_CloseAudioDevice(g_effect_audio);
    SDL_CloseAudioDevice(g_audio);
    SDL_DestroyRenderer(g_ren);
    SDL_DestroyWindow(g_win);
    SDL_Quit();
    return 0;
}