
extern struct lexercontext {
    int linenum;
} LCX;

extern struct parsercontext {
    bool mylog {false};
    int jointlevel;
    int framenum;
    double framesep;
    void pushjoint(const char name[]), popjoint();
    void endsite();
} PCX;

int scanmotionline(const char[]);
void setcurrentoffset(double, double, double);
void setcurrentchannels(unsigned, unsigned, unsigned);
void setcurrentchannels(unsigned, unsigned, unsigned, unsigned, unsigned, unsigned);
void storetableline(unsigned columns);
void parserfinished();
