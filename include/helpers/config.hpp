#pragma once

#include <edizon.h>
#include <unistd.h>
#include <stdio.h>
#include <cstring>

#define CONFIG_PATH EDIZON_DIR "/seconfig.dat"

namespace Config {
  typedef struct ConfigData {
    char magic[8];
    bool hideSX;
    char latestCommit[40];
    bool option_once;
    bool options[3];
    char edizon_dir[40];
    u64 lasttitle = 0;
    char version[40];
    bool deletebookmark;
    bool showallsaves;
    bool freeze;
    bool disablerangeonunknown;
    bool enablecheats;
    char dbversion[40];
    bool separatebookmark;
  } config_data_t;

  void readConfig();
  void writeConfig();
  config_data_t* getConfig();
}