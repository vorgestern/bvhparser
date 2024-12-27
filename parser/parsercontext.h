
struct BVHScene;

extern struct parsercontext {
    BVHScene*Scene {nullptr};
    bool mylog {false};
    int jointlevel {0};
    int framenum {0};
    double framesep {0};
    unsigned nextchannel {0};
    int linenum;
    void pushjoint(const char name[]), popjoint();
    void endsite();
    void setcurrentchannels(unsigned, unsigned, unsigned);
    void setcurrentchannels(unsigned, unsigned, unsigned, unsigned, unsigned, unsigned);
    void setcurrentoffset(double, double, double);
    void parserfinished();
    void storetableline(unsigned columns);
    int scanmotionline(const char[]);
    unsigned reservechannels(unsigned numchannels);
} PCX;
