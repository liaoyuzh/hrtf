#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <bass.h>
#include "hrtf.hpp"

#define X cos(theta)
#define Y sin(theta)
#define Z -1.0

void CALLBACK hrtfDSP(HDSP, DWORD, void *buffer, DWORD length, void *user) {
    HRTF &h = *(HRTF*)(user);
    std :: vector<int16_t> l, r;
    int16_t *ptr = (int16_t*)buffer;
    for (DWORD i = 0; i < length >> 1; i += 2) {
        l.push_back(ptr[i]);
        r.push_back(ptr[i + 1]);
    }
    h.DSP(l, r);
    for (DWORD i = 0; i < length >> 1; i += 2) {
        ptr[i] = l[i >> 1];
        ptr[i + 1] = r[i >> 1];
    }
}

int main() {
    BASS_Init(-1, 44100, 0, 0, NULL);
    HRTF h("full");
    printf("Enter audio filename: ");
    char filename[256];
    fgets(filename, 256, stdin);
    filename[strlen(filename) - 1] = '\0';
    HSTREAM handle = BASS_StreamCreateFile(FALSE, filename, 0, 0, 0);
    BASS_ChannelSetDSP(handle, hrtfDSP, &h, 1);
    BASS_ChannelPlay(handle, FALSE);
    for (float theta = 0;; theta += M_PI / 1000) {
        if (BASS_ChannelIsActive(handle) == BASS_ACTIVE_STOPPED) {
            break;
        }
        float x = (X), y = (Y), z = (Z);
        h.setSpeakerPosition(x, y, z);
        printf("(%f, %f ,%f)\n", x, y, z);
        usleep(10000);
    }
    BASS_StreamFree(handle);
    BASS_Free();
    return 0;
}
