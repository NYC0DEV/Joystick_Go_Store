#!/bin/sh
#clean up some system memory
echo 1 > /proc/sys/vm/drop_caches
#Checks to see if /userdata/start_local_sd.sh is the latest one with wireless gamepad event check
if ! grep -qw "event2" /userdata/start_local_sd.sh; then
  cp -f /mnt/sdcard/start_local_sd_mod.sh /userdata/start_local_sd.sh
fi

APPEND_CFG=/sdcard/retroarch/config/handheld_mode.cfg
BOS_OPTION="-dedicated_landscape"
#Configure some settings if the GSG is connected to an HDMI device
if [ `cat /sys/class/drm/card0-HDMI-A-1/status` == "connected" ]; then
  export AUDIODEV=hdmi
  alsactl restore -f /sdcard/asound.state
  aplay -D hw:0,1 /etc/open.wav
  export ALSA_CARD=0
  export ALSA_DEVICE=1
  echo off > /sys/class/drm/card0-HDMI-A-1/status
  echo on > /sys/class/drm/card0-HDMI-A-1/status
  echo detect > /sys/class/drm/card0-HDMI-A-1/status
  APPEND_CFG=/sdcard/retroarch/config/HDMI_mode.cfg
  BOS_OPTION="-hdmi_dedicated_landscape"
fi
cp /sdcard/bin/retroarch-1.22.2 /tmp/
chmod +x /tmp/retroarch-1.22.2
export HOME=/sdcard
export XDG_CONFIG_HOME=/sdcard
export XDG_RUNTIME_DIR=/sdcard
RETRO_BINARY=/tmp/retroarch-1.22.2
CFG=/sdcard/retroarch/config/retroarch.cfg

GAME_LIB=

#Hold down on A button to set wireless gamepad's trackball as the one to use
evtest --query /dev/input/event2 EV_KEY 305

if [ $? -eq 10 ]; then
  if [ `cat /sys/class/drm/card0-HDMI-A-1/status` == "connected" ]; then
    APPEND_CFG=/sdcard/retroarch/config/HDMI_mode_mouse_index_0.cfg
  else
    APPEND_CFG=/sdcard/retroarch/config/handheld_mode_mouse_index_0.cfg
  fi
fi

#Assign a library core based off of folder name
#gsgparm changes the button and control LEDs and some controls
case $(echo "$4" | tr '[:upper:]' '[:lower:]' | tr -d ' ') in
  /sdcard/games/amiga/*)
    GAME_LIB=uae4arm_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,dpad,L1,R1,L2,R2,1,3,4,5,tball -k 18
    ;;
  /sdcard/games/atari2600/*)
    GAME_LIB=stella2014_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/atari2600homebrew/*)
    GAME_LIB=stellahb
    /mnt/sdcard/bin/gsgparm -0 all -1 dpad,A,B,X,Y -k 27
    ;;
  /sdcard/games/atari2600paddle/*)
    GAME_LIB=stella
    /mnt/sdcard/bin/gsgparm -0 all -1 A,spin -k 11
    ;;
  /sdcard/games/atari5200/*)
    GAME_LIB=a5200_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,dpad,kpad -k 18
    ;;
  /sdcard/games/atari7800/*)
    GAME_LIB=prosystem_nds_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/atari800/*)
    GAME_LIB=atari800_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad -k 18
    ;;
  /sdcard/games/atarilynx/*)
    GAME_LIB=handy_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/atarirecharged/*)
    GAME_LIB=recharged
    ;;
  /sdcard/games/atarist/*)
    GAME_LIB=hatari_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,1,3,4,5,tball,L1,L2,R1,R2 -k 18
    ;;
  /sdcard/games/ballsofsteel/*)
    GAME_LIB=balls
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,L1,L2,R1,R2 -k 0
    ;;
  /sdcard/games/colecovision/*)
    GAME_LIB=gearcoleco_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,kpad -k 18
    ;;  
  /sdcard/games/commodore64/*)
    GAME_LIB=vice_x64_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad -k 18
    ;;
  /sdcard/games/doom/*)
    GAME_LIB=prboom_libretro.so
    /mnt/sdcard/bin/gsgparm -1 A,B,X,Y,dpad,1,2,3,4,5,6,7,8,9,0,L1,L2,R1,R2 -k 18
    ;;
  /sdcard/games/gameandwatch/*)
    GAME_LIB=gw_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,1,3,4,5,L1,R1 -k 18
    ;;
  /sdcard/games/gameboy/*)
    GAME_LIB=gambatte_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,1,3,4,5 -k 18
    ;;  
  /sdcard/games/gameboyadvance/*)
    GAME_LIB=mgba_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,1,3,4,5,L1,R1 -k 18
    ;;
  /sdcard/games/gameboycolor/*)
    GAME_LIB=gambatte_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,1,3,4,5 -k 18
    ;;  
  /sdcard/games/intellivision/*)
    GAME_LIB=freeintv_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,kpad -k 18
    ;;  
  /sdcard/games/mame/*)
    GAME_LIB=mame2003_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,tball,spin -k 22
    ;;
  /sdcard/games/microvision/*)
    GAME_LIB=mvem_libretro.so
    /sdcard/bin/gsgparm -0 all -1 spin,dpad,A,B,X,Y,L1,R1,L2,R2 -k 26
    ;;
  /sdcard/games/misc/* | /sdcard/games/Misc/*)
    GAME_LIB=MISC
    ;;
  /sdcard/games/msdos/*)
    GAME_LIB=dosbox_pure_libretro.so
    /sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,tball,L1,R1,L2,R2 -k 18
    ;;
  /sdcard/games/msx/*)
    GAME_LIB=fmsx_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/neogeo/*)
    GAME_LIB=fbneo_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/neogeopocketcolor/*)
    GAME_LIB=mednafen_ngp_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/nes/*)
    GAME_LIB=fceumm_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/odyssey2/*)
    GAME_LIB=o2em_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/pcengine/*)
    GAME_LIB=mednafen_pce_fast_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/pcenginecd/*)
    GAME_LIB=mednafen_pce_fast_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/playstation/*)
    GAME_LIB=pcsx_rearmed_libretro.so
    /mnt/sdcard/bin/gsgparm -1 A,B,X,Y,dpad,1,3,4,5,7,8,9,L1,L2,R1,R2 -k 18
    ;;
  /sdcard/games/scummvm/*)
    GAME_LIB=scummvm_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,kpad,tball,L1,R1,R2 -k 18
    ;;
  /sdcard/games/sega32x/*)
    GAME_LIB=picodrive_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/segacd/*)
    GAME_LIB=genesis_plus_gx_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/segagamegear/*)
    GAME_LIB=picodrive_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all  -1 A,B,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/segagenesis/*)
    GAME_LIB=picodrive_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/segamastersystem/*)
    GAME_LIB=picodrive_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all  -1 A,B,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/snes/*)
    GAME_LIB=snes9x2005_plus_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,1,3,4,5,L1,R1 -k 18
    ;;
  /sdcard/games/ti99sim/*)
    GAME_LIB=ti99sim
    /sdcard/bin/gsgparm -0 all -1 dpad,A,Y,1,2,3,4,5,6,7,8,9,0,L1,R1 -k 18
    ;;
  /sdcard/games/vectrex/*)
    GAME_LIB=vecx_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,1,3,4,5 -k 18
    ;;
  /sdcard/games/vic20/*)
    GAME_LIB=vice_xvic_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad -k 18
    ;;
  /sdcard/games/wasm-4/*)
    GAME_LIB=wasm4_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,dpad,1,3,4,5,tball -k 18
    ;;    
  /sdcard/games/wolfenstein_3d/*)
    GAME_LIB=ecwolf_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,1,3,4,5,L1,L2,R1,R2 -k 18
    ;;
  /sdcard/games/wonderswancolor/*)
    GAME_LIB=mednafen_wswan_libretro.so
    #Turn on keypad buttons to be the Y buttons for games in the Vertical folder, otherwise
    #Turn on the shoulders and triggers to be the Y buttons
    case $(echo "$4" | tr '[:upper:]' '[:lower:]' | tr -d ' ') in
      /sdcard/games/wonderswancolor/vertical/*)
        /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,5,7,9,0 -k 18
        ;;
      *)
        /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad,L1,L2,R1,R2 -k 0
        ;;
    esac
    ;;
  /sdcard/games/zxspectrum/*)
    GAME_LIB=fuse_libretro.so
    /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,L1,R1,1,3,4,5 -k 18
    ;;
  *)
    GAME_LIB=NONE
    ;;
esac

#Change the button LEDs if the user set them up for a specific game
folder_path=$(dirname "$4")
game_filename="${4##*/}"
game_name="${game_filename%.*}"
if [ -f "$folder_path/$game_name.sh" ]; then
  sh "$folder_path/$game_name.sh"
fi

case "$GAME_LIB" in
  MISC)
    app_filename="${4##*/}"
    app_name="${app_filename%.*}"
    cd /sdcard/bin/$app_name
    /mnt/sdcard/bin/gsgparm -0 all -1 dpad,Y,X,B,A -k 18
    ./$app_name
    /mnt/sdcard/bin/gsgparm -0 all -1 dpad,Y,X,B,A -k 18
    ;;
  balls)
    game_filename="${4##*/}"
    game_name="${game_filename%.*}"
    cd /data/Games/Balls_of_Steel/$game_name
    case "$game_name" in
      Barbarian)
        game_name=barbarian_ARM32
        ;;
      Centipede)
        game_name=centipede_ARM32
        ;;
      Darkside)
        game_name=darkside_ARM32
        ;;
      Devils-Island)
        game_name=devils_ARM32
        ;;
      Firestorm)
        game_name=firestorm_ARM32
        ;;
      Missile-Command)
        game_name=missile_ARM32
        ;;
      Mutation)
        game_name=mutation_ARM32
        ;;
    esac
    ./$game_name $BOS_OPTION
    ;;
  dosbox_pure_libretro.so)
    sed -i 's/dosbox_pure_sblaster_adlib_emu = "default"/dosbox_pure_sblaster_adlib_emu = "fast"/' /mnt/sdcard/retroarch/config/DOSBox-pure/DOSBox-pure.opt
    sed -i 's|dosbox_pure_midi = "GeneralUser-GS.sf2"|dosbox_pure_midi = "/sdcard/retroarch/system/GeneralUser-GS.sf2"|' /mnt/sdcard/retroarch/config/DOSBox-pure/DOSBox-pure.opt
    $RETRO_BINARY -c $CFG -L /sdcard/retroarch/cores/$GAME_LIB "$4" --appendconfig $APPEND_CFG
    sed -i 's/dosbox_pure_sblaster_adlib_emu = "default"/dosbox_pure_sblaster_adlib_emu = "fast"/' /mnt/sdcard/retroarch/config/DOSBox-pure/DOSBox-pure.opt
    sed -i 's|dosbox_pure_midi = "GeneralUser-GS.sf2"|dosbox_pure_midi = "/sdcard/retroarch/system/GeneralUser-GS.sf2"|' /mnt/sdcard/retroarch/config/DOSBox-pure/DOSBox-pure.opt
    ;;
  recharged)
    game_filename="${4##*/}"
    game_name="${game_filename%.*}"
    options=""
    cd /data/Games/Recharged/$game_name
    case "$game_name" in
      Asteroids)
        game_name=Asteroids_ARM32
        /mnt/sdcard/bin/gsgparm -0 all -1 A,B,dpad -k 0
        ;;
      Berzerk)
        game_name=Berzerk_ARM32
        options="--megadevice"
        /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,L1,L2,R1,R2 -k 0
        ;;
      Breakout)
        game_name=Breakout_ARM32
        /mnt/sdcard/bin/gsgparm -0 all -1 spin -k 10
        ;;
      Centipede)
        game_name=Centipede_ARM32
        /mnt/sdcard/bin/gsgparm -0 all -1 A,dpad,tball -k 0
        ;;
      MissileCommand)
        game_name=MissileCommand_ARM32
        /mnt/sdcard/bin/gsgparm -0 all -1 A,dpad,tball -k 0
        ;;
    esac
    if [ `cat /sys/class/drm/card0-HDMI-A-1/status` == "connected" ]; then
      ./$game_name
    else
      ./$game_name --handheld
    fi
    ;;
  scummvm_libretro.so)
    #Launch ScummVM directly, no game specified
    $RETRO_BINARY -c $CFG -L /sdcard/retroarch/cores/$GAME_LIB  --appendconfig $APPEND_CFG
    ;;
  stella)
    #Run 2600 paddle games
    /usr/bin/stella $3 "$4"
    ;;
  stellahb)
    #Run 2600 games, it should automatically change the controls based on the game
    /sdcard/bin/stella -fullscreen 1 "$4"
   ;;
  ti99sim)
    filename=$(basename "$4")
    base="${filename%.zip}"
    cart="${base}.ctg"
    cp -r /sdcard/bin/ti99sim /tmp/
    chmod +x /tmp/ti99sim/*
    /tmp/ti99sim/bin/ti99sim-sdl "$cart"
    ;;
  *)
    #Run any other game
    $RETRO_BINARY -c $CFG -L /sdcard/retroarch/cores/$GAME_LIB "$4" --appendconfig $APPEND_CFG
    ;;
esac

if [ "$4" == /sdcard/Games/pico8/pico8.zip ]; then
  #Check to see if pico-8 is installed and launch it
  if [ -f "/sdcard/bin/pico-8/pico8_dyn" ]; then
    /mnt/sdcard/bin/gsgparm -0 all -1 A,X,dpad -k 0
    cp -r /sdcard/bin/pico-8 /tmp/
    chmod +x /tmp/pico-8/*
    /tmp/pico-8/pico8_dyn -home /sdcard/Games/pico8 -splore
  fi
fi
if [ "$4" == "/sdcard/Games/Launcher/Launcher.zip" ]; then
  /mnt/sdcard/bin/gsgparm -0 all -1 A,B,X,Y,dpad,L1,R1,1,3,7,9 -k 18
  sh /sdcard/bin/AGSG_CCF_FE/launcher.sh
  exit 0
fi
#Exiting game, resetting button LEDs and freeing up memory
if [ "$GAME_LIB" != "MISC" ]; then
  /mnt/sdcard/bin/gsgparm -0 all -1 dpad,A,B,X,Y,L1,R1,1,3,7,9 -k 18
fi
sync && echo 3 > /proc/sys/vm/drop_caches