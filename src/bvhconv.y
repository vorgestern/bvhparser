
%{
    #include "parsercontext.h"
    int yylex();
    void yyerror(char const*);
%}

%union {
    int I;
    double D;
    char S[1000];
}

%token <I> OPENBRACE CLOSEBRACE OPENPAREN CLOSEPAREN
%token <S> STRING
%token <I> ASTERISK INT
%token <D> FLOAT

%token HIERARCHY OFFSET ENDSITE MOTION
%token XPOSITION YPOSITION ZPOSITION XROTATION YROTATION ZROTATION
%token <I> CHANNELS FRAMES TABLELINE
%token <S> ROOT JOINT
%token <D> FRAMETIME

%type <I> channelspec
%type <D> number
%%

input: HIERARCHY startroot root MOTION framenumspec frametimespec motiontable { dumphumanoid(); }

startroot: ROOT { PCX.pushjoint($1); }

root: OPENBRACE offsetspec channels joints CLOSEBRACE { PCX.popjoint(); }
    | OPENBRACE offsetspec channels endsite CLOSEBRACE { PCX.popjoint(); }

framenumspec: FRAMES { PCX.framenum=$1; }
frametimespec: FRAMETIME { PCX.framesep=$1; }

joints: joint {}
      | endsite {}
      | joint joints {}

joint: startjoint OPENBRACE offsetspec channels joints CLOSEBRACE { PCX.popjoint(); }
     | startjoint OPENBRACE offsetspec channels endsite CLOSEBRACE { PCX.popjoint(); }

startjoint: JOINT { PCX.pushjoint($1); }

endsite: endsitespec { PCX.popjoint(); PCX.endsite(); }
       | endsitespec OPENBRACE CLOSEBRACE { PCX.popjoint(); PCX.endsite(); }
       | endsitespec OPENBRACE offsetspec CLOSEBRACE { PCX.popjoint(); PCX.endsite(); }

endsitespec: ENDSITE { PCX.pushjoint(nullptr); }

offsetspec: OFFSET number number number { setcurrentoffset($2,$3,$4); }

channels: CHANNELS channelspec channelspec channelspec { setcurrentchannels($2,$3,$4); }
        | CHANNELS channelspec channelspec channelspec channelspec channelspec channelspec { setcurrentchannels($2,$3,$4,$5,$6,$7); }
channelspec: XPOSITION {}
           | YPOSITION {}
           | ZPOSITION {}
           | XROTATION {}
           | YROTATION {}
           | ZROTATION {}

number: INT {}
      | FLOAT {}

motiontable: /* */ {}
           | tableline motiontable {}

tableline: TABLELINE { storetableline($1); }

%%

// offsetspec: OFFSET FLOAT FLOAT FLOAT { setcurrentoffset($2,%$3,$4); }
//           | OFFSET INT INT INT { setcurrentoffset($2,$3,$4); }
