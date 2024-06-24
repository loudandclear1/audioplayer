#ifndef AUDIOPLAYER_WLPLAYSTATUS_H
#define AUDIOPLAYER_WLPLAYSTATUS_H

class WlPlaystatus {
public:
    bool exit = false;
    bool load = true;
    bool seek = false;
    bool pause = false;

public:
    WlPlaystatus();

    ~WlPlaystatus();
};

#endif //AUDIOPLAYER_WLPLAYSTATUS_H
