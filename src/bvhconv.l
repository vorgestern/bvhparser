
%option never-interactive
%option outfile="lex-bvhconv.cpp"
    // Considerations regarding option outfile:
    //      Using option in lex-File will not allow to specify the directory.
    //      Using commandline '-o filename' will allow full control but embeds the full path in the generated scanner.
    //      Using commandline '-t > filename' will allow full control without embedding the path in the generated scanner.
    // Windows/VS17/MSBuild: Do not specify output file on the command line.
    //                       Specify <Outputs>$(ProjectDir)lex-%(Filename).cpp</Outputs>
    // Linux/make:           Use -t to override this option and fully determine output file on the command line.

  #include <ctype.h>
  #include <parsercontext.h>
  #include "bvhconv.h"
  #define OUTNL {}
  #define ASSTR(s)  { strcpy(yylval.S, yytext); return s; }
  #define ASNONE(s) { return s; }
  void ateof();
  // {INT} { yylval.I=atoi(yytext); return INT; }

WS                [ \t]
ID                [A-Za-z_][A-Za-z_0-9]*
STRING            \"(∞‰ˆ¸ƒ÷‹ﬂ|[^"])*\"
INT               (\+|-)*[0-9]+
FIXED             (\+|-)*([0-9]*\.[0-9]+|[0-9]+\.[0-9]*)
NL                \n

%x animationtable

%%

HIERARCHY { ASNONE(HIERARCHY); }
ROOT{WS}+{ID} { const char*s=yytext+5; while (*s!=0 && isspace(*s)) s++; strcpy(yylval.S, s); return ROOT; }
JOINT{WS}+{ID} { const char*s=yytext+6; while (*s!=0 && isspace(*s)) s++; strcpy(yylval.S, s); return JOINT; }
End{WS}+Site { ASNONE(ENDSITE); }
OFFSET { ASNONE(OFFSET); }
CHANNELS{WS}+{INT} { sscanf(yytext+9, "%d", &yylval.I); return CHANNELS; }
Xposition { yylval.I=XPOSITION; ASNONE(XPOSITION); }
Yposition { yylval.I=YPOSITION; ASNONE(YPOSITION); }
Zposition { yylval.I=ZPOSITION; ASNONE(ZPOSITION); }
Xrotation { yylval.I=XROTATION; ASNONE(XROTATION); }
Yrotation { yylval.I=YROTATION; ASNONE(YROTATION); }
Zrotation { yylval.I=ZROTATION; ASNONE(ZROTATION); }

{INT}|{FIXED}|{FIXED}[Ee](\+|-)?[0-9]+  { sscanf(yytext, "%lf", &yylval.D); return FLOAT; }
  /* Alle numerischen Angaben werden als FLOAT interpretiert. Integerwerte kommen nur vor zusammen mit einem spezifischen Tag, z.B. CHANNELS */

MOTION { BEGIN(animationtable); ASNONE(MOTION); }

<*>{WS}+ {}
<*>{WS}*{NL} { LCX.linenum++; }
\{ { ASNONE(OPENBRACE); }
\} { ASNONE(CLOSEBRACE); }
\* { ASNONE(ASTERISK); }
\( { ASNONE(OPENPAREN); }
\) { ASNONE(CLOSEPAREN); }

{STRING} { strcpy(yylval.S, yytext+1); char*s=strchr(yylval.S, 0); if (s>yylval.S) s[-1]=0; return STRING; }

<animationtable>^Frames:{WS}*{INT}{WS}*{NL} { sscanf(yytext+7, "%d", &yylval.I); return FRAMES; }
<animationtable>^Frame{WS}+Time:{WS}*{FIXED}{WS}*{NL} { const char*s=strchr(yytext, ':'); sscanf(s+1, "%lf", &yylval.D); return FRAMETIME; }
<animationtable>^.*{NL} { yylval.I=scanline(yytext); return TABLELINE; }
<animationtable>^.* { yylval.I=scanline(yytext); return TABLELINE; }

<<EOF>> { ateof(); return 0; }

%%
