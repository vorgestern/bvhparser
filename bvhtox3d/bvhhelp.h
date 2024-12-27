
#include <parser.h>
#include <glm/glm.hpp>

enum class SegmentForms {none,line,cylinder}; // Commandline arguments -segments none|line|cylinder

struct OutputOptions
{
    SegmentForms segmentshape;
    bool has_floor;
    bool has_headlight;
};

struct bbox { glm::dvec3 bmin, bmax; };

void compute_traces(std::vector<glm::dvec3>&, const Hierarchy&, const MotionLine&);
bbox compute_boundingbox(const Hierarchy&, const MotionTable&);

void output_x3d(const Hierarchy&, const MotionTable&, double totaltime, const OutputOptions&opt);
void dumphumanoid_bb(const Hierarchy&);
void dumphumanoid_txt(const Hierarchy&);
