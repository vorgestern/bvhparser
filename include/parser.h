
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
    int parent {-1};
    unsigned level;                //!< Hierarchy level
    unsigned firstchannelindex;    //!< Index of the first associated channel in the MOTION table
    unsigned getrotationindexes(int index[], int dir[])const; //!< Return columns and axes to which they refer in the specified order.
    friend unsigned channelnum(const hanimjoint&X){ return X.channelnum; }
    friend unsigned level(const hanimjoint&X){ return X.level; }
    friend const char*name(const hanimjoint&X){ return X.name.c_str(); }
    friend unsigned firstchannel(const hanimjoint&X){ return X.firstchannelindex; }
    friend unsigned lastchannel(const hanimjoint&X){ return X.firstchannelindex+X.channelnum-1; }
    friend bool haspositionchannels(const hanimjoint&);
    friend bool hasrotationchannels(const hanimjoint&);
    friend jtype type(const hanimjoint&);
    friend int parent(const hanimjoint&X){ return X.parent; }
public:
    glm::dvec3 offset;
    hanimjoint(int p, unsigned level1=0): parent(p), channelnum(0), level(level1), firstchannelindex(0xffffffff), offset({0,0,0}){}
    unsigned char operator[](int n)const{ return n<0?0:n<(int)channelnum?channels[n]:0; }
    void setchannels(unsigned start, unsigned char c0, unsigned char c1, unsigned char c2);
    void setchannels(unsigned start, unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5);
    void setname(const char name[]);
    glm::dmat4 gettransform(const MotionLine&)const;
    glm::dmat4 getrotation(const MotionLine&)const;
    void getpositionindexes(int index[])const;
};

struct BVHScene
{
    Hierarchy H;
    MotionTable M;
    double totaltime {0};
};

void lexdump(std::string_view filename);
BVHScene*parse(std::string_view, bool mixlexeroutput=false);

std::vector<glm::vec3>flatten(const Hierarchy&);
std::vector<glm::vec3>flatten(const Hierarchy&, const MotionLine&);
std::vector<std::pair<unsigned,unsigned>>segments(const Hierarchy&);
std::pair<glm::vec3,glm::vec3>boundingbox(const Hierarchy&);
std::pair<glm::vec3,glm::vec3>boundingbox(const Hierarchy&, const MotionLine&);
std::pair<glm::vec3,glm::vec3>boundingbox(const Hierarchy&, const MotionTable&);
glm::vec3 recenter(const glm::mat4&View, const glm::mat4&Projection, const glm::vec4&viewport, const glm::vec3&eyepoint);
