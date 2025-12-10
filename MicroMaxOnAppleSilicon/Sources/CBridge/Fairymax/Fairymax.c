#pragma GCC diagnostic warning "-w"

/***************************************************************************/
/*                               fairy-Max,                                */
/* Version of the sub-2KB (source) micro-Max Chess program, fused to a     */
/* generic WinBoard interface, loading its move-generator tables from file */
/***************************************************************************/

/*****************************************************************/
/*                      LICENCE NOTIFICATION                     */
/* Fairy-Max 4.8 is free software, and you have my permission do */
/* with it whatever you want, whether it is commercial or not.   */
/* Note, however, that Fairy-Max can easily be configured through*/
/* its fmax.ini file to play Chess variants that are legally pro-*/
/* tected by patents, and that to do so would also require per-  */
/* mission of the holders of such patents. No guarantees are     */
/* given that Fairy-Max does anything in particular, or that it  */
/* would not wreck the hardware it runs on, and running it is    */
/* entirely for your own risk.  H.G,Muller, author of Fairy-Max  */
/*****************************************************************/

#define MULTIPATH
#define VERSION "4.8O"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

// Use engine_printf instead of printf for proper pipe communication with Swift
#include "../include/MicroMaxEngine.h"

#ifndef INI_FILE
#define INI_FILE "fmax.ini"
#endif

#ifdef _MSC_VER
#    include <windows.h>
#else
#    include <sys/time.h>
int GetTickCount() // with thanks to Tord
{	struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec*1000 + t.tv_usec/1000;
}
#endif

int StartKey;

#define EMPTY -1
#define WHITE 0
#define BLACK 16

#define STATE 128

/* The following macros indicate the differences between Fairy-Max and its */
/* dedicated Shatranj derivative ShaMax so that these can now be compiled  */
/* from the same unified source file.                                      */
/* Compile with gcc option -DSHATRANJ to build ShaMax.                     */
#ifdef SHATRANJ
#    define FAC 175
#    define EG  13
#    define NAME "ShaMax"
#    define SHAMAX(x) x
#    define FMAX(x)
#else
#    define FAC 128
#    define EG  10
#    define NAME "Fairy-Max"
#    define SHAMAX(x)
#    define FMAX(x) x
#endif

/* make unique integer from engine move representation */
#define PACK_MOVE 256*K + L;

/* convert intger argument back to engine move representation */
#define UNPACK_MOVE(A) K = (A)>>8 & 255; L = (A) & 255;

/* Global variables visible to engine. Normally they */
/* would be replaced by the names under which these  */
/* are known to your engine, so that they can be     */
/* manipulated directly by the interface.            */

int Side;
int Move;
int PromPiece;
int Result;
int TimeLeft;
int MovesLeft;
int MaxDepth;
int Post;
int Fifty;
int UnderProm;
int GameNr;
int Resign;
int Threshold = 800;
int Score;
int makruk;
char piecename[32], piecetype[32], blacktype[32];
char *inifile = INI_FILE;

int Ticks, tlim, Setup, SetupQ;

int GameHistory[1024];
char HistoryBoards[1024][STATE], setupPosition[131];
int GamePtr, HistPtr;

#define W while
#define K(A,B) *(int*)(T+A+S*(B&31))
#define J(A) K(y+A,b[y])-K(x+A,u)-K(H+A,t)

#ifdef CHESS_MINI
int U=(1<<20)-1; // 12 MB
#else
int U=(1<<23)-1; // 96 MB
#endif

struct _ {int K,V;char X,Y,D,F;} *A;           /* hash table, 16M+8 entries*/

int M=136,S=128,I=8e3,Q,O,K,N,j,R,J,Z,LL,      /* M=0x88                   */
BW,BH,sh,
w[16]={0,2,2,-1,7,8,12,23,7,5},                /* relative piece values    */
o[256],
oo[32],                                        /* initial piece setup      */
of[256],
od[16];                                        /* 1st dir. in o[] per piece*/

signed char L,pl[17],
b[513],                                        /* board: 16x8+dummy, + PST */
T[4104],                                       /* hash translation table   */
centr[32],
n[]=".*XKNBRQEWFMACHG?x+knbrqewfmachg";        /* piece symbols on printout*/

int pv[10000],*sp=pv; // triangular array
int margin;

void pboard()
{int i;
    i=-1;W(++i<128)
        //engine_printf(" %c",(i&15)==BW&&(i+=15-BW)?10:n[b[i]&31]);
        fprintf(stderr, " %c",(i&15)==BW&&(i+=15-BW)?10:n[b[i]&31]);
    fprintf(stderr, Side == WHITE ? "White to move" : "Black to move");
}


int D(k,q,l,e,E,z,n)        /* recursive minimax search, k=moving side, n=depth*/
int k,q,l,e,E,z,n;      /* (q,l)=window, e=current eval. score, E=e.p. sqr.*/
{                       /* e=score, z=prev.dest; J,Z=hashkeys; return score*/
    int j,r,m,v,d,h,i,F,G,P,V,f=J,g=Z,C,s,flag,FF,*ps=sp;
    signed char t,p,u,x,y,X,Y,H,B;
    struct _*a=A+(J+(k+S)*E&U-1);                 /* lookup pos. in hash table*/
    *sp++=0;
    q-=q<e;l-=l<=e;                               /* adj. window: delay bonus */
    d=a->D;m=a->V;X=a->F;Y=a->Y;                  /* resume at stored depth   */
    if(a->K-Z|z&S  |                              /* miss: other pos. or empty*/
       !(m<=q|X&8&&m>=l|X&S))                       /*   or window incompatible */
        d=Y=0;                                       /* start iter. from scratch */
    X=a->X;                                       /* start at best-move hint  */
    W(d++<n||d<3||              /*** min depth = 2   iterative deepening loop */
      z&S&&K==I&&(GetTickCount()-Ticks<tlim&d<=MaxDepth|| /* root: deepen upto time   */
                  (K=X,L=Y&~S,Score=m,d=3)))                  /* time's up: go do best    */
    {x=B=X;                                       /* start scan at prev. best */
        h=Y&S;                                       /* request try noncastl. 1st*/
        P=d>2&&l+I?D(16-k,-l,1-l,-e,2*S,2*S,d-3):I;  /* search null move         */
        m=-P<l|R<5?d-2?-I:e:-P;   /*** prune if > beta  unconsidered:static eval */
        SHAMAX( if(pl[k]<=1&pl[16-k]>1)m=I-1; )      /* bare king loses          */
        N++;                                         /* node count (for timing)  */
        do{u=b[x];                                   /* scan board looking for   */
            if(u&&(u&16)==k)                            /*  own piece (inefficient!)*/
            {r=p=u&15;                                  /* p = piece type (set r>0) */
                j=od[p];                                   /* first step vector f.piece*/
                W(r=o[++j])                                /* loop over directions o[] */
                {A:                                        /* resume normal after best */
                    flag=h?3:of[j];                           /* move modes (for fairies) */
                    y=x;F=FF=G=S;                             /* (x,y)=move, (F,G)=castl.R*/
                    do{                                       /* y traverses ray, or:     */
                        H=y=h?Y^h:y+r;                           /* sneak in prev. best move */
                        if(flag&1<<8)H=y=(y&15)>13?y+BW:(y&15)>=BW?y-BW:y; /* cylinder board */
                        if(y&S|(y&15)>=BW)break;                 /* board edge hit           */
#ifdef MULTIPATH
                        if(flag&1<<9)                            /* if multipath move        */
                        {t=flag>>12;                             /* get dir. stepped twice   */
                            if(b[x+t]){if(b[y-2*t]|b[y-t])break;}else
                                if(b[x+2*t]&&b[y-t])break;              /* test if empty path exists*/
                        }
#endif
                        m=E<16|(E^112)<16&&flag&1&y-E<2&E-y<2?I:m;      /* bad castling  */
                        if(p<3&y==E)H=z&127;                     /* shift capt.sqr. H if e.p.*/
                        t=b[H];
                        if(flag&1+!t)                            /* mode (capt/nonc) allowed?*/
                        {if(t&&(t&16)==k)break;                  /* capture own              */
                            i=w[t&15]+((t&192)>>sh);                /* value of capt. piece t   */
                            if(i<0)m=I,d=98;                        /* K capture                */
                            if(m>=l&d>1)goto C;                     /* abort on fail high       */
                            v=d-1?e:i-p;                            /*** MVV/LVA scoring if d=1**/
                            if(d-!t>1)                              /*** all captures if d=2  ***/
                            {v=centr[p]?b[x+257]-b[y+257]:0;        /* center positional pts.   */
                                if(!(G&S))b[FF]=b[G],v+=50;            /* castling: put R & score  */
                                b[G]=b[H]=b[x]=0;b[y]=u|32;            /* do move, set non-virgin  */
                                v-=w[p]>0|R<EG?0:20;                   /*** freeze K in mid-game ***/
                                if(p<3)                                /* pawns:                   */
                                {v-=9*((x-2&M||b[x-2]-u)+              /* structure, undefended    */
                                       (x+2&M||b[x+2]-u)               /*        squares plus bias */
                                       +(w[b[x^16]&15]<0))              /*** cling to magnetic K ***/
                                    +(R-76>>2);                      /* end-game Pawn-push bonus */
                                    b[y]+=V=y+r+1&S?647-p:2*(u&y+16&32);  /* upgrade P or convert to Q*/
                                    if(V&makruk)b[y]=u|7,V=480;           /* Makruk promotion on 6th  */
                                    V>>=sh;                               /* for Shatranj promo to F  */
                                    i+=V;                                 /* promotion / passer bonus */
                                } if(z&S && GamePtr<6) v+=(rand()>>10&31)-16;
                                J+=J(0);Z+=J(4)+G-S;
                                SHAMAX( pl[k]-=!!t; )                  /* count pieces per side    */
                                v+=e+i;V=m>q?m:q;                      /*** new eval & alpha    ****/
                                if(z&S)V=m-margin>q?m-margin:q;        /* multiPV                  */
                                C=d-1-(d>5&p>2&!t&!h);                 /* nw depth, reduce non-cpt.*/
                                C=R<EG|P-I|d<3||t&&p-3?C:d;            /* extend 1 ply if in-check */
                                do
                                    s=C>2|v>V?-D(16-k,-l,-V,-v,/*** futility, recursive eval. of reply */
                                                 F,y&255,C):v;
                                W(s>q&++C<d); v=s;                     /* no fail:re-srch unreduced*/
                                if(v>V&v<l){int *p=sp;
                                    sp=ps+1;
                                    W(*sp++=*p++);
                                    *ps=256*x+y;
                                }
                                if(z&S&&K-I)                           /* move pending: check legal*/
                                {if(v+I&&x==K&y==L)                    /*   if move found          */
                                {Q=-e-i;O=F;LL=L;
                                    if(b[y]-u&15)b[y]-=PromPiece,        /* under-promotion, correct */
                                        J+=PromPiece;           /*  piece & invalidate hash */
                                    a->D=99;a->V=0;                      /* lock game in hash as draw*/
                                    R-=i/FAC;                            /*** total captd material ***/
                                    Fifty = t|p<3?0:Fifty+1;
                                    sp=ps;
                                    return l;}                /*   & not in check, signal */
                                    v=m;                                  /* (prevent fail-lows on    */
                                }                                      /*   K-capt. replies)       */
                                J=f;Z=g;
                                SHAMAX( pl[k]+=!!t; )
                                b[G]=b[FF];b[FF]=b[y]=0;b[x]=u;b[H]=t; /* undo move,G can be dummy */
                            }                                       /*          if non-castling */
                            if(z&S&&Post&K==I&d>2&v>V&v<l){int *p=ps;char X,Y;
                                engine_printf("%2d ",d-2);
                                engine_printf("%6d ",v);
                                engine_printf("%8d %10d",(GetTickCount()-Ticks)/10,N);
                                while(*p){X=*p>>8;Y=*p++;
                                    engine_printf(" %c%c%c%c",'a'+(X&15),'8'-(X>>4),'a'+(Y&15),'8'-(Y>>4&7));}
                                engine_printf("\n");fflush(stdout);
                            }
                            if(v>m)                                 /* new best, update max,best*/
                                m=v,X=x,Y=y|S&F;                       /* mark non-double with S   */
                            if(h){h=0;goto A;}                      /* redo after doing old best*/
                        }
                        s=t;v=r^flag>>12;                        /* calc. alternated vector  */
                        if(flag&15^4|u&32||                      /* no double or moved before*/
                           p>2&&                                 /* no P & no lateral K move,*/
                           ((b[G=r<0?x&~15:BW-1|x&112]^32)<33    /* no virgin R in corner G, */
                            ||b[G^1]|b[G^2]|b[FF=y+v-r])          /* no 2 empty sq. next to R */
                           )t+=flag&4;                            /* fake capt. for nonsliding*/
                        else F=y;                                /* enable e.p.              */
                        if(s&&flag&8)t=0,flag^=flag>>4&15;       /* hoppers go to next phase */
                        if(!(flag&S))                            /* zig-zag piece?           */
                            r=v,flag^=flag>>4&15;                   /* alternate vector & mode  */
                    }W(!t);                                   /* if not capt. continue ray*/
                }}
            if((++x&15)>=BW)x=x+16&112;                 /* next sqr. of board, wrap */
        }W(x-B);
    C:FMAX( m=m+I|P==I?m:(X=Y=0); )                /* if stalemate, draw-score */
        if(a->D<99)                                  /* protect game history     */
            a->K=Z,a->V=m,a->D=d,a->X=X,                /* always store in hash tab */
            a->F=8*(m>q)|S*(m<l),a->Y=Y;                /* move, type (bound/exact),*/
    }                                            /*    encoded in X S,8 bits */
    if(z&4*S)K=X,L=Y&~S;
    sp=ps;
    return m+=m<e;                                /* delayed-loss bonus       */
}


/* Generic main() for Winboard-compatible engine     */
/* (Inspired by TSCP)                                */
/* Author: H.G. Muller                               */

/* The engine is invoked through the following       */
/* subroutines, that can draw on the global vaiables */
/* that are maintained by the interface:             */
/* Side         side to move                         */
/* Move         move input to or output from engine  */
/* PromPiece    requested piece on promotion move    */
/* TimeLeft     ms left to next time control         */
/* MovesLeft    nr of moves to play within TimeLeft  */
/* MaxDepth     search-depth limit in ply            */
/* Post         boolean to invite engine babble      */

/* InitEngine() progran start-up initialization      */
/* InitGame()   initialization to start new game     */
/*              (sets Side, but not time control)    */
/* Think()      think up move from current position  */
/*              (leaves move in Move, can be invalid */
/*               if position is check- or stalemate) */
/* DoMove()     perform the move in Move             */
/*              (togglese Side)                      */
/* ReadMove()   convert input move to engine format  */
/* PrintMove()  print Move on standard output        */
/* Legal()      check Move for legality              */
/* ClearBoard() make board empty                     */
/* PutPiece()   put a piece on the board             */

/* define this to the codes used in your engine,     */
/* if the engine hasn't defined it already.          */

int PrintResult(int s)
{
    int i, j, k, cnt=0;
    
    /* search last 50 states with this stm for third repeat */
    for(j=2; j<=100 && j <= HistPtr; j+=2)
    {
        for(k=0; k<STATE; k++)
            if(HistoryBoards[HistPtr][k] !=
               HistoryBoards[HistPtr-j&1023][k] )
            {
                goto differs;}
        /* is the same, count it */
        if(++cnt > 1) /* third repeat */
        {
            engine_printf("1/2-1/2 {Draw by repetition}\n");
            return 1;
        }
    differs: ;
    }
    K=I;
    cnt = D(s,-I,I,Q,O,LL|4*S,3);
#ifdef SHATRANJ
    if(pl[s]==1 && pl[16-s]==1) {
        engine_printf("1/2-1/2 {Insufficient mating material}\n");
        return 4;
    }
    if(pl[s]<=1 && pl[16-s]>1) {
        if (s == BLACK)
            engine_printf("0-1 {Bare King}\n");
        else
            engine_printf("1-0 {Bare King}\n");
        return 5;
    }
#else
    if(cnt>-I+1 && K==0 && L==0) {
        engine_printf("1/2-1/2 {Stalemate}\n");
        return 2;
    }
#endif
    if(cnt==-I+1) {
        if (s == WHITE)
            engine_printf("0-1 {Black mates}\n");
        else
            engine_printf("1-0 {White mates}\n");
        return 3;
    }
    if(Fifty >=100) {
        engine_printf("1/2-1/2 {Draw by fifty move rule}\n");
        return 4;
    }
    return 0;
}


void InitEngine()
{
    int i, j;
    
    N=32*S+7;W(N-->S+3)T[N]=rand()>>9;
    srand(GetTickCount());
}

void InitGame()
{
    int i,j;
    
    for(i=0;i<16*BH;i++)b[i]=0;
    K=BW;W(K--)
    {b[K]=oo[K+16]+16;b[K+112]=oo[K];b[K+16]=18;b[K+96]=1; /* initial board setup*/
        L=8;W(L--)b[16*L+K+257]=(K-BW/2)*(K-BW/2)+(L-3.5)*(L-3.5); /* center-pts table   */
    }                                                   /*(in unused half b[])*/
    Side = WHITE; Q=0; O=S;
    Fifty = 0; R = 0;
    for(i=0; i<BW; i++) if(i!=3) R += (w[oo[i]]/FAC) + (w[oo[i+16]]/FAC);
    UnderProm = -1; pl[WHITE] = pl[BLACK] = 2*BW;
}

void CopyBoard(int s)
{
    int i, j, k, cnt=0;
    
    /* copy game representation of engine to HistoryBoard */
    /* don't forget castling rights and e.p. state!       */
    for(i=0; i<BH; i++)
        for(j=0; j<BW; j++)                 /* board squares  */
            HistoryBoards[s][BW*i+j] = b[16*i+j]|64*(16*i+j==O);
}

void PrintVariants()
{
    int i, j, count=0; char c, buf[80];
    FILE *f;
    
    f = fopen(INI_FILE, "r");
    if(f==NULL) return;
    
    /* search for game names in definition file */
    do {
        while(fscanf(f, "Game: %s", buf) != 1 && c != EOF)
            while((c = fgetc(f)) != EOF && c != '\n');
        if(c == EOF) break;
        if(count++) engine_printf(",");
        engine_printf("%s", buf);
    } while(c != EOF);
    
    fclose(f);
}

void PrintOptions()
{
    engine_printf("feature option=\"Resign -check %d\"\n", Resign);
    engine_printf("feature option=\"Resign Threshold -spin %d 200 1200\"\n", Threshold);
    engine_printf("feature option=\"Ini File -file %s\"\n", inifile);
    engine_printf("feature option=\"Multi-PV Margin -spin %d 0 1000\"\n", margin);
    engine_printf("feature option=\"Playing Style ;-) -combo Brilliant /// *Brave /// Beautiful\"\n");
    engine_printf("feature option=\"Dummy Slider Example -slider 20 0 100\"\n");
    engine_printf("feature option=\"Dummy String Example -file happy birthday!\"\n");
    engine_printf("feature option=\"Dummy Path Example -path .\"\n");
    engine_printf("feature option=\"Clear Hash -button\"\n");
    engine_printf("feature done=1\n");
}


int LoadGame(char *name)
{
    int i, j, count=0; char c, buf[80];
    static int currentVariant;
    FILE *f;
    
    extern void getMicroMaxIni(char *ini);

    char ini[300];
    getMicroMaxIni(ini);

    f = fopen(ini, "r");
    if(f==NULL)
    {   engine_printf("telluser piece-desription file '%s'  not found\n", inifile);
        exit(0);
    }
    if(fscanf(f, "version 4.8(%c)", &c)!=1 || c != 'w')
    { engine_printf("telluser incompatible fmax.ini file\n"); exit(0); }
    
    if(name != NULL)
    {  /* search for game name in definition file */
        while(fscanf(f, "Game: %s", buf)!=1 || strcmp(name, buf) ) {
            while((c = fgetc(f)) != EOF && c != '\n');
            count++;
            if(c == EOF) {
                engine_printf("telluser variant %s not supported\n", name);
                fclose(f);
                return 1; /* keep old settings */
            }
        }
        currentVariant = count;
    }
    
    /* We have found variant, or if none specified, are at beginning of file */
    if(fscanf(f, "%dx%d", &BW, &BH)!=2 || BW>12 || BH!=8)
    { engine_printf("telluser unsupported board size %dx%d\n",BW,BH); exit(0); }
    
    for(i=0; i<BW; i++) fscanf(f, "%d", oo+i);
    for(i=0; i<BW; i++) fscanf(f, "%d", oo+i+16);
    for(i= 0; i<=U; i++)
        A[i].K = A[i].D = A[i].X = A[i].Y = A[i].F = 0; /* clear hash */
    for(i=0; i<32; i++) piecetype[i] = blacktype[i] = 0;
    
    i=0; j=-1; c=0;
    while(fscanf(f, "%d,%x", o+j, of+j)==2 ||
          fscanf(f,"%c:%d",&c, w+i+1)==2)
    {   if(c)
    { od[++i]=j; centr[i] = c>='a';
        blacktype[c&31]=i; piecename[i]=c&31;
        if(piecetype[c&31]==0) piecetype[c&31]=i; // only first
    }
        j++; o[j]=0;
        /* engine_printf("# c='%c' i=%d od[i]=%d j=%d (%3d,%8x)\n",c?c:' ',i,od[i],j,o[j-1],of[j-1]); /**/
        c=0; if(i>15 || j>255) break;
    }
    
    fclose(f);
    sh = w[7] < 250 ? 3 : 0;
    makruk = w[7]==181 ? 64 : 0; // w[7] is used as kludge to enable makruk promotions
    return 0;
}

extern int readLineForEngine(char buffer[], int size);

int main_fairymax(int argc, char **argv)
{    
    int Computer, MaxTime, MaxMoves, TimeInc, sec, i, j;
    char line[256], command[256], c, cc;
    int m, nr, fd;
    FILE *f;
    
    if(argc>1 && sscanf(argv[1], "%d", &m)==1)
    { U = (1<<m)-1; argc--; argv++; }
    A = (struct _ *) calloc(U+1, sizeof(struct _));
    if(argc>1) inifile = argv[1];
    
    signal(SIGINT, SIG_IGN);
    engine_printf("tellics say     " NAME " " VERSION "\n");
    engine_printf("tellics say     by H.G. Muller\n");
    engine_printf("tellics say Gothic Chess is protected by U.S. patent #6,481,716 by Ed Trice.\n");
    engine_printf("tellics say Falcon Chess is protected by U.S. patent #5,690,334 by George W. Duke\n");
    InitEngine();
    LoadGame(NULL);
    InitGame();
    Computer = EMPTY;
    MaxTime  = 10000;  /* 10 sec */
    MaxDepth = 30;     /* maximum depth of your search */
    
    fd = atoi(argv[0]);
    
    for (;;) {
        fflush(stdout);
        if (Side == Computer) {
            /* think up & do move, measure time used  */
            /* it is the responsibility of the engine */
            /* to control its search time based on    */
            /* MovesLeft, TimeLeft, MaxMoves, TimeInc */
            /* Next 'MovesLeft' moves have to be done */
            /* within TimeLeft+(MovesLeft-1)*TimeInc  */
            /* If MovesLeft<0 all remaining moves of  */
            /* the game have to be done in this time. */
            /* If MaxMoves=1 any leftover time is lost*/
            Ticks = GetTickCount();
            m = MovesLeft<=0 ? 40 : MovesLeft;
            tlim = (0.6-0.06*(BW-8))*(TimeLeft+(m-1)*TimeInc)/(m+7);
            if(tlim>TimeLeft/15) tlim = TimeLeft/15;
            PromPiece = 0; /* Always promote to Queen ourselves */
            N=0;K=I;
            if (D(Side,-I,I,Q,O,LL|S,3)==I) {
                Side ^= BLACK^WHITE;
                if(UnderProm>=0 && UnderProm != L)
                {    engine_printf("tellics I hate under-promotions!\n");
                    engine_printf("resign { underpromotion } \n");
                    Computer = EMPTY;
                    continue;
                } else UnderProm = -1;
                engine_printf("move ");
                engine_printf("%c%c%c%c",'a'+(K&15),'0'+BH-(K>>4),
                       'a'+(L&15),'0'+BH-(L>>4));
                engine_printf("\n");
                m = GetTickCount() - Ticks;
                
                /* time-control accounting */
                TimeLeft -= m;
                TimeLeft += TimeInc;
                if(--MovesLeft == 0) {
                    MovesLeft = MaxMoves;
                    if(MaxMoves == 1)
                        TimeLeft  = MaxTime;
                    else TimeLeft += MaxTime;
                }
                
                GameHistory[GamePtr++] = PACK_MOVE;
                CopyBoard(HistPtr=HistPtr+1&1023);
                if(Resign && Score <= -Threshold) {
                    engine_printf("resign\n"); Computer=EMPTY;
                } else if(PrintResult(Side))
                    Computer = EMPTY;
            } else {
                if(!PrintResult(Side))
                    engine_printf("resign { refuses own move }\n");
                Computer = EMPTY;
            }
            continue;
        }
        
        readLineForEngine(line, 256);
        
        /*
         if (!fgets(line, 256, stdin))
         return;
         */
        
        if (line[0] == '\n')
            continue;
        sscanf(line, "%s", command);
        
        if (!strcmp(command, "xboard"))
            continue;
        if (!strcmp(command, "protover")) {
            engine_printf("feature myname=\"" NAME " " VERSION "\"\n");
            engine_printf("feature memory=1\n");
            engine_printf("feature setboard=0 ping=1 done=0\n");
            engine_printf("feature variants=\"");
            PrintVariants();
            engine_printf("\"\n");
            PrintOptions();
            continue;
        }
        if (!strcmp(command, "ping")) { int nr=0;
            sscanf(line, "ping %d", &nr);
            engine_printf("pong %d\n", nr);
            continue;
        }
        if (!strcmp(command, "p")) {
            pboard();
            continue;
        }
        if (!strcmp(command, "memory")) {
            int mem, mask;
            sscanf(line+6, "%d", &mem); mem = (mem*1024*1024)/12; // max nr of hash entries
            mask = 0x7FFFFFFF; while(mask > mem) mask >>= 1;
            if(mask != U) {
                free(A); U = mask;
                A = (struct _ *) calloc(U+1, sizeof(struct _));
            }
            continue;
        }
        if (!strcmp(command, "new")) {
            /* start new game */
            LoadGame("normal");
            InitGame();
            GamePtr   = Setup = 0;
            GameNr++;
            HistPtr   = 0;
            Computer  = BLACK;
            TimeLeft  = MaxTime;
            MovesLeft = MaxMoves;
            for(nr=0; nr<1024; nr++)
                for(m=0; m<STATE; m++)
                    HistoryBoards[nr][m] = 0;
            continue;
        }
        if (!strcmp(command, "quit"))
        /* exit engine */
            return 0;
        if (!strcmp(command, "force")) {
            /* computer plays neither */
            Computer = EMPTY;
            continue;
        }
        if (!strcmp(command, "white")) {
            /* set white to move in current position */
            Side     = WHITE;
            Computer = BLACK;
            continue;
        }
        if (!strcmp(command, "black")) {
            /* set blck to move in current position */
            Side     = BLACK;
            Computer = WHITE;
            continue;
        }
        if (!strcmp(command, "st")) {
            /* move-on-the-bell mode     */
            /* indicated by MaxMoves = 1 */
            sscanf(line, "st %d", &MaxTime);
            MovesLeft = MaxMoves = 1;
            TimeLeft  = MaxTime *= 1000;
            TimeInc   = 0;
            continue;
        }
        if (!strcmp(command, "sd")) {
            /* set depth limit (remains in force */
            /* until next 'sd n' command)        */
            sscanf(line, "sd %d", &MaxDepth);
            MaxDepth += 2; /* QS depth */
            continue;
        }
        if (!strcmp(command, "level")) {
            /* normal or blitz time control */
            sec = 0;
            if(sscanf(line, "level %d %d %d",
                      &MaxMoves, &MaxTime, &TimeInc)!=3 &&
               sscanf(line, "level %d %d:%d %d",
                      &MaxMoves, &MaxTime, &sec, &TimeInc)!=4)
                continue;
            MovesLeft = MaxMoves;
            TimeLeft  = MaxTime = 60000*MaxTime + 1000*sec;
            TimeInc  *= 1000;
            continue;
        }
        if (!strcmp(command, "time")) {
            /* set time left on clock */
            sscanf(line, "time %d", &TimeLeft);
            TimeLeft  *= 10; /* centi-sec to ms */
            continue;
        }
        if (!strcmp(command, "otim")) {
            /* opponent's time (not kept, so ignore) */
            continue;
        }
        if (!strcmp(command, "easy")) {
            continue;
        }
        if (!strcmp(command, "hard")) {
            continue;
        }
        if (!strcmp(command, "accepted")) {
            continue;
        }
        if (!strcmp(command, "rejected")) {
            continue;
        }
        if (!strcmp(command, "random")) {
            continue;
        }
        if (!strcmp(command, "option")) {
            int i; static char filename[80];
            if(sscanf(line+7, "Resign=%d", &Resign) == 1) continue;
            if(sscanf(line+7, "Resign Threshold=%d", &Threshold) == 1) continue;
            if(sscanf(line+7, "Ini File=%s", filename) == 1) {
                inifile = filename; continue;
            }
            if(sscanf(line+7, "Clear Hash") == 1) for(i=0; i<U; i++) A->K = 0;
            if(sscanf(line+7, "MultiVariation Margin=%d", &margin) == 1) continue;
            continue;
        }
        if (!strcmp(command, "go")) {
            /* set computer to play current side to move */
            Computer = Side;
            MovesLeft = -(GamePtr+(Side==WHITE)>>1);
            while(MaxMoves>0 && MovesLeft<=0)
                MovesLeft += MaxMoves;
            continue;
        }
        if (!strcmp(command, "hint")) {
            Ticks = GetTickCount(); tlim = 1000;
            D(Side,-I,I,Q,O,LL|4*S,6);
            if (K==0 && L==0)
                continue;
            engine_printf("Hint: ");
            engine_printf("%c%c%c%c",'a'+(K&7),'8'-(K>>4),
                   'a'+(L&7),'8'-(L>>4));
            engine_printf("\n");
            continue;
        }
        if (!strcmp(command, "undo")   && (nr=1) ||
            !strcmp(command, "remove") && (nr=2)   ) {
            /* 'take back' moves by replaying game */
            /* from history until desired ply      */
            if (GamePtr - nr < 0)
                continue;
            GamePtr -= nr;
            HistPtr -= nr;   /* erase history boards */
            while(nr-- > 0)
                for(m=0; m<STATE; m++)
                    HistoryBoards[HistPtr+nr+1&1023][m] = 0;
            InitGame();
            if(Setup) {
                for(i=0; i<128; i++) b[i] = setupPosition[i];
                Side = setupPosition[128]; Q = SetupQ;
                pl[WHITE] = setupPosition[129];
                pl[BLACK] = setupPosition[130];
            }
            for(i=0; i<=U; i++) A[i].D = A[i].K = 0; // clear hash table
            for(nr=0; nr<GamePtr; nr++) {
                UNPACK_MOVE(GameHistory[nr]);
                D(Side,-I,I,Q,O,LL|S,3);
                Side ^= BLACK^WHITE;
            }
            continue;
        }
        if (!strcmp(command, "post")) {
            Post = 1;
            continue;
        }
        if (!strcmp(command, "nopost")) {
            Post = 0;
            continue;
        }
        if (!strcmp(command, "variant")) {
            sscanf(line, "variant %s", command);
            LoadGame(command);
            InitGame(); Setup = 0;
            continue;
        }
        if (!strcmp(command, "edit")) {
            int color = WHITE, p;
            
            //                        while(fgets(line, 256, stdin)) {
            while (readLineForEngine(line, 256)) {
                
                m = line[0];
                if(m=='.') break;
                if(m=='#') {
                    for(i=0; i<128; i++) b[i]=0;
                    Q=0; R=0; O=S;
                    pl[WHITE]=pl[BLACK]=0;
                    continue;
                }
                if(m=='c') {
                    color = WHITE+BLACK - color;
                    Q = -Q;
                    continue;
                }
                if( m >= 'A' && m <= 'Z' && piecetype[m&31]
                   && line[1] >= 'a' && line[1] <= 'a'+BW-1
                   && line[2] >= '1' && line[2] <= '0'+BH) {
                    m = line[1]-16*line[2]+799;
                    switch(p = (color == WHITE ? piecetype : blacktype)[line[0]&31])
                    {
                        case 1:
                        case 2:
                            if(color==WHITE)
                                b[m]=(m&0x70)==0x60?1:33,
                                Q+=w[1];
                            else b[m]=(m&0x70)==0x10?18:50,
                                Q+=w[2];
                            break;
                        default:
                            b[m]=p+color+32; // assume non-virgin
                            if(w[p]<0) { // Royal piece on original square: virgin
                                
                                if(color==BLACK && m<0x10 && p==oo[m+16] ||
                                   color==WHITE && m>0x6F && p==oo[m-0x70]) b[m] -= 32;
                            } else { Q+=w[p]; R+=w[p]/FAC; }
                            if((m==0x00 || m==BW-1   ) && color==BLACK && p==oo[m+16] ||
                               (m==0x70 || m==0x6F+BW) && color==WHITE && p==oo[m-0x70])
                                b[m] &= ~32; // corner piece as in original setup: virgin
                        case 0: // undefined piece, ignore
                            break;
                    }
                    pl[BLACK+WHITE-color]++;
                    continue;
                }
            }
            if(Side != color) Q = -Q;
            GamePtr = HistPtr = 0; Setup = 1; SetupQ = Q; // start anew
            for(i=0; i<128; i++) setupPosition[i] = b[i]; // remember position
            setupPosition[128] = Side;
            setupPosition[129] = pl[WHITE];
            setupPosition[130] = pl[BLACK];
            continue;
        }
        /* command not recognized, assume input move */
        m = line[0]<'a' | line[0]>='a'+BW | line[1]<'1' | line[1]>='1'+BH |
        line[2]<'a' | line[2]>='a'+BW | line[3]<'1' | line[3]>='1'+BH;
        if(line[4] == '\n') line[4] = piecename[7];
        PromPiece = 7 - (Side == WHITE ? piecetype : blacktype)[line[4]&31];
        if(PromPiece == 7) PromPiece = 0;
        {char *c=line; K=c[0]-16*c[1]+799;L=c[2]-16*c[3]+799; }
        if (m)
        /* doesn't have move syntax */
            engine_printf("Error (unknown command): %s\n", command);
        else if(D(Side,-I,I,Q,O,LL|S,3)!=I) {
            /* did have move syntax, but illegal move */
            fprintf(stderr, "Illegal move:%s\n", line);
        } else {  /* legal move, perform it */
            GameHistory[GamePtr++] = PACK_MOVE;
            Side ^= BLACK^WHITE;
            CopyBoard(HistPtr=HistPtr+1&1023);
            if(PrintResult(Side)) Computer = EMPTY;
        }
    }
}

