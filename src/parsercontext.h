
struct BVHScene;

extern struct parsercontext {
    BVHScene*Scene {nullptr};
    bool mylog {false};
    int jointlevel;
    int framenum;
    double framesep;
    int linenum;
    void pushjoint(const char name[]), popjoint();
    void endsite();
    void setcurrentchannels(unsigned, unsigned, unsigned);
    void setcurrentchannels(unsigned, unsigned, unsigned, unsigned, unsigned, unsigned);
    void setcurrentoffset(double, double, double);
    void parserfinished();
    void storetableline(unsigned columns);
    int scanmotionline(const char[]);
} PCX;
