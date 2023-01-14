#include "Disk.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <dirent.h>

using namespace std;

Disk::Disk( Config *conf ) {
    diskLabel = "DISK BASE";
    opened = { "", "", 0 };
    media_path = "";
    config = conf;
    media_path = conf->get_media();
}

// Disk::~Disk() {}

string Disk::getLabel() { return diskLabel; }

Disk::FileEntry Disk::getOpenedFileEntry() { return opened; }

void Disk::closeFile() {
    opened = { "", "", 0 };
};
