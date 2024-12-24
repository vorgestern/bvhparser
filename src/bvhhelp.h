
// Commandline arguments -segments none|line|cylinder
enum class SegmentForms {none,line,cylinder};

void dumptoken(int token);

// Setup indices for following channels.
unsigned getchannelrange(unsigned numchannels);

enum class jtype {root,joint,endsite};

// Represent a joint (or root) of the model.
// The parser creates a list of objects of this type.
class hanimjoint
{
    std::string name {};
    double offset[3];
    char channels[6];              //!< XYZ for translation, xyz for rotation
    unsigned channelnum;           //!< Number of channels, 0,3,6
    unsigned level;                //!< Hierarchy level
    unsigned firstchannelindex;    //!< Index of the first associated channel in the MOTION table
    bool haspositionchannels()const;
    bool hasrotationchannels()const;
    void getpositionindexes(int index[])const;
    unsigned getrotationindexes(int index[], int dir[])const; //!< Return columns and axes to which they refer in the specified order.
    friend unsigned channelnum(const hanimjoint&X){ return X.channelnum; }
    friend unsigned level(const hanimjoint&X){ return X.level; }
    friend const char*name(const hanimjoint&X){ return X.name.c_str(); }
    friend unsigned firstchannel(const hanimjoint&X){ return X.firstchannelindex; }
    friend unsigned lastchannel(const hanimjoint&X){ return X.firstchannelindex+X.channelnum-1; }
    friend const double*offset(const hanimjoint&X){ return X.offset; }
    friend jtype type(const hanimjoint&);
public:
    hanimjoint(unsigned level1=0): channelnum(0), level(level1), firstchannelindex(0xffffffff)
    {
        offset[0]=offset[1]=offset[2]=0;
    }
    unsigned char operator[](int n)const{ return n<0?0:n<(int)channelnum?channels[n]:0; }
    void setchannels(unsigned char c0, unsigned char c1, unsigned char c2);
    void setchannels(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5);
    void setoffset(double x, double y, double z){ offset[0]=x; offset[1]=y; offset[2]=z;  }
    void setname(const char name[]);
    void dumpmotiontables_x3d(const std::vector<std::vector<double>>&)const;
    void dumpmotionroutes_x3d()const;
};

extern std::vector<hanimjoint>HUMANOID;
extern std::vector<std::vector<double>>MotionTable;

void dumphumanoid_x3d(const std::vector<hanimjoint>&, SegmentForms),
    dumpmotiontable_x3d(const std::vector<hanimjoint>&, const std::vector<std::vector<double>>&),
    dumpmotionroutes_x3d(const std::vector<hanimjoint>&);

// Compute the axis/angle-representation from num rotations (1 to 3).
// The first rotation rotates around DIRS[0] by ANGLES[0],
// the second around DIRS[1] by ANGLES[1], the third around DIRS[2] by ANGLES[2].
// DIRS[n]: 0=>X-axis, 1=>Y-axis, 2=>Z-axis.
void AnglesToAxisAngle(double axis[], double*angle, const double ANGLES[], const int DIRS[], unsigned num);
