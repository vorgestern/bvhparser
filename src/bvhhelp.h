
// Commandline arguments -segments none|line|cylinder  ==> 0|1|2
extern enum SegmentForms {sfnone,sfline,sfcylinder} segmentform;

void dumptoken(int token);
int scanline(const char[]);
void storetableline(unsigned columns);
void getmotiontable(const double**table, unsigned*lines, unsigned*columns);

void setcurrentchannels(unsigned, unsigned, unsigned);
void setcurrentchannels(unsigned, unsigned, unsigned, unsigned, unsigned, unsigned);
void setcurrentoffset(double, double, double);

extern struct lexercontext {
    int linenum;
} LCX;

extern struct parsercontext {
    bool mylog {false};
    int jointlevel;
    int framenum;
    double framesep;
    double totaltime;
    void pushjoint(const char name[]), popjoint();
    void endsite();
} PCX;

// Setup indices for following channels.
unsigned getchannelrange(unsigned numchannels);

// Number of channels required by the hierarchy.
unsigned channelsused();

// Represent a joint (or root) of the model.
// The parser creates a list of objects of this type.
class hanimjoint
{
    const char*name;
    double offset[3];
    unsigned char channels[6];     //!< XYZ for translation, xyz for rotation
    unsigned channelnum;           //!< Number of channels, 0,3,6
    unsigned level;                //!< Hierarchy level
    unsigned firstchannelindex;    //!< Index of the first associated channel in the MOTION table
    bool haspositionchannels()const{ for (unsigned c=0; c<channelnum; c++) switch (channels[c]){ case 'X': case 'Y': case 'Z': return true; } return false; }
    bool hasrotationchannels()const{ for (unsigned c=0; c<channelnum; c++) switch (channels[c]){ case 'x': case 'y': case 'z': return true; } return false; }
    void getpositionindexes(int index[])const;
    unsigned getrotationindexes(int index[], int dir[])const; //!< Return columns and axes to which they refer in the specified order.
    friend unsigned channelnum(const hanimjoint&X){ return X.channelnum; }
    friend unsigned level(const hanimjoint&X){ return X.level; }
    friend const char*name(const hanimjoint&X){ return X.name; }
    friend unsigned firstchannel(const hanimjoint&X){ return X.firstchannelindex; }
    friend unsigned lastchannel(const hanimjoint&X){ return X.firstchannelindex+X.channelnum-1; }
    friend const double*offset(const hanimjoint&X){ return X.offset; }
    friend unsigned type(const hanimjoint&X){ return X.level==0?0:X.name!=0L?1:2; } // type 0=Root, 1=Joint, 2=End Site
public:
    hanimjoint(unsigned level1=0): channelnum(0), level(level1), name(0L), firstchannelindex(0xffffffff){ offset[0]=offset[1]=offset[2]=0; }
   ~hanimjoint(){ if (name!=0L) delete[]name; }
    unsigned char operator[](int n)const{ return n<0?0:n<(int)channelnum?channels[n]:0; }
    void setchannels(unsigned char c0, unsigned char c1, unsigned char c2){ channelnum=3; channels[0]=c0; channels[1]=c1; channels[2]=c2; firstchannelindex=getchannelrange(3); }
    void setchannels(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5){ channelnum=6; channels[0]=c0; channels[1]=c1; channels[2]=c2; channels[3]=c3; channels[4]=c4; channels[5]=c5; firstchannelindex=getchannelrange(6); }
    void setoffset(double x, double y, double z){ offset[0]=x; offset[1]=y; offset[2]=z;  }
    void setname(const char name[]);
    void dumpmotiontables_x3d(const double table[], unsigned lines, unsigned columns)const;
    void dumpmotionroutes_x3d()const;
};

extern hanimjoint HUMANOID[];
extern unsigned HLEN;

void dumphumanoid();
void dumphumanoid_txt(const hanimjoint[], unsigned nj),
    dumphumanoid_x3d(const hanimjoint[], unsigned nj),
    dumpmotiontable_x3d(const hanimjoint[], unsigned nj),
    dumpmotionroutes_x3d(const hanimjoint[], unsigned nj);

// Compute the axis/angle-representation from num rotations (1 to 3).
// The first rotation rotates around DIRS[0] by ANGLES[0],
// the second around DIRS[1] by ANGLES[1], the third around DIRS[2] by ANGLES[2].
// DIRS[n]: 0=>X-axis, 1=>Y-axis, 2=>Z-axis.
void AnglesToAxisAngle(double axis[], double*angle, const double ANGLES[], const int DIRS[], unsigned num);
