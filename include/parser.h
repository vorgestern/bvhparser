
#include <string>
#include <vector>
#include <glm/glm.hpp>

class hanimjoint;
using MotionLine=std::vector<double>;
using MotionTable=std::vector<MotionLine>;
using Hierarchy=std::vector<hanimjoint>;
enum class jtype {root,joint,endsite};

// Represent a joint (or root) of the model.
// The parser creates a list of objects of this type.
class hanimjoint
{
    std::string name {};
    char channels[6];              //!< XYZ for translation, xyz for rotation
    unsigned channelnum;           //!< Number of channels, 0,3,6
    unsigned level;                //!< Hierarchy level
    unsigned firstchannelindex;    //!< Index of the first associated channel in the MOTION table
    bool haspositionchannels()const;
    bool hasrotationchannels()const;
    void getpositionindexes(int index[])const;
    unsigned getrotationindexes(int index[], int dir[])const; //!< Return columns and axes to which they refer in the specified order.
    glm::dmat4 getrotation(const std::vector<double>&MotionLine)const;
    friend unsigned channelnum(const hanimjoint&X){ return X.channelnum; }
    friend unsigned level(const hanimjoint&X){ return X.level; }
    friend const char*name(const hanimjoint&X){ return X.name.c_str(); }
    friend unsigned firstchannel(const hanimjoint&X){ return X.firstchannelindex; }
    friend unsigned lastchannel(const hanimjoint&X){ return X.firstchannelindex+X.channelnum-1; }
    friend jtype type(const hanimjoint&);
public:
    glm::dvec3 offset;
    hanimjoint(unsigned level1=0): channelnum(0), level(level1), firstchannelindex(0xffffffff), offset({0,0,0}){}
    unsigned char operator[](int n)const{ return n<0?0:n<(int)channelnum?channels[n]:0; }
    void setchannels(unsigned start, unsigned char c0, unsigned char c1, unsigned char c2);
    void setchannels(unsigned start, unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5);
    void setname(const char name[]);
    void dumpmotiontables_x3d(const MotionTable&)const;
    void dumpmotionroutes_x3d()const;
    glm::dmat4 gettransform(const MotionLine&)const;
};

struct BVHScene
{
    Hierarchy H;
    MotionTable M;
    double totaltime {0};
};

void lexdump(std::string_view filename);
BVHScene*parse(std::string_view, bool mixlexeroutput=false);
