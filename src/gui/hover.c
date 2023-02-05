/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Mouse Hover
 *
 * Displays mouse-over (hover) texts.
 *
 * Add "log_char(cn,LOG_SYSTEM,0,"�c5.");" to the very end of int look_item() in tool.c!
 *
 */

#include <windows.h>
#include <psapi.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "../../src/astonia.h"
#include "../../src/gui.h"
#include "../../src/gui/_gui.h"
#include "../../src/game.h"
#include "../../src/client.h"
#include "../../src/sdl.h"

char hover_bless_text[120];
char hover_freeze_text[120];
char hover_potion_text[120];
char hover_rage_text[120];
char hover_level_text[120];
char hover_rank_text[120];
char hover_time_text[120];

static int display_hover(void);
static void display_hover_update(void);
static int display_hover_skill(void);

void display_mouseover(void) {
    int hide;

    if (mousey>=doty(DOT_BOT)-370+496-60 && mousey<=doty(DOT_BOT)-370+551-60) {
        if (mousex>=dotx(DOT_BOT)+207 && mousex<=dotx(DOT_BOT)+214) dd_drawtext(mousex,mousey-16,0xffff,DD_BIG|DD_FRAME|DD_CENTER,hover_rage_text);
        if (mousex>=dotx(DOT_BOT)+197 && mousex<=dotx(DOT_BOT)+204) dd_drawtext(mousex,mousey-16,0xffff,DD_BIG|DD_FRAME|DD_CENTER,hover_bless_text);
        if (mousex>=dotx(DOT_BOT)+187 && mousex<=dotx(DOT_BOT)+194) dd_drawtext(mousex,mousey-16,0xffff,DD_BIG|DD_FRAME|DD_CENTER,hover_freeze_text);
        if (mousex>=dotx(DOT_BOT)+177 && mousex<=dotx(DOT_BOT)+184) dd_drawtext(mousex,mousey-16,0xffff,DD_BIG|DD_FRAME|DD_CENTER,hover_potion_text);
    }

    if (mousex>=dotx(DOT_BOT)+25 && mousex<=dotx(DOT_BOT)+135) {
        if (mousey>=doty(DOT_TOP)+5 && mousey<=doty(DOT_TOP)+13) dd_drawtext(mousex+16,mousey-4,0xffff,DD_BIG|DD_FRAME,hover_level_text);
        if (mousey>=doty(DOT_TOP)+22 && mousey<=doty(DOT_TOP)+30) dd_drawtext(mousex+16,mousey-4,0xffff,DD_BIG|DD_FRAME,hover_rank_text);
    }

    if (mousex>=dotx(DOT_TOP)+728 && mousex<=dotx(DOT_TOP)+772 && mousey>=doty(DOT_TOP)+7 && mousey<=doty(DOT_TOP)+17)
        dd_drawtext(mousex-16,mousey-4,0xffff,DD_BIG|DD_FRAME|DD_RIGHT,hover_time_text);

    display_hover_update();
    hide=display_hover();
    hide+=display_hover_skill();
    if (hide) sdl_show_cursor(0);
    else if (capbut==-1) sdl_show_cursor(1);
}


#define MAXVALID    (TICKS*60*2)
#define HOVER_DELAY (TICKS/4)
#define MAXDESC 20

struct hover_item {
    int valid_till;
    int cnt;
    int width;
    char *desc[MAXDESC];
};

static struct hover_item hi[INVENTORYSIZE+CONTAINERSIZE];

static int last_look=0,last_invsel=-1,last_line=0,capture=0,last_tick=0;

static int textlength(char *text) {
    int x=0;
    char *c,buf[4];

    for (x=0,c=text; *c; c++) {
        if (c[0]=='�') {
            if (c[1]=='c') {
                if (isdigit(c[2])) {
                    if (isdigit(c[3])) {
                        c+=3; continue;
                    }
                    c+=2; continue;
                }
                c+=1; continue;
            }
            continue;
        }
        buf[0]=*c; buf[1]=0;
        x+=dd_textlength(0,buf);
    }
    return x;
}

int hover_capture_text(char *line) {
    int len;

    while (isspace(*line)) line++;

    if (line[0]=='�' && line[1]=='c' && line[2]=='5' && last_look) {
        capture=1;
    }

    if (line[0]=='�' && line[1]=='c' && line[2]=='5' && line[3]=='.') {
        capture=last_look=0;
        return 1;
    }

    if (capture) {
        len=textlength(line);
        hi[last_invsel].valid_till=tick+MAXVALID;
        hi[last_invsel].desc[last_line++]=xstrdup(line,MEM_TEMP11);
        hi[last_invsel].cnt=last_line;
        hi[last_invsel].width=max(hi[last_invsel].width,len);
    }

    return capture;
}

void hover_capture_tick(void) {
    if (capture) capture=last_look=0;
    if (last_look>0) last_look--;
}

void hover_invalidate_inv(int slot) {
    if (slot<0 || slot>=INVENTORYSIZE) return;
    hi[slot].valid_till=0;
}

void hover_invalidate_inv_delayed(int slot) {
    if (slot<0 || slot>=INVENTORYSIZE) return;
    hi[slot].valid_till=tick+TICKS/2;
}

void hover_invalidate_con(int slot) {
    if (slot<0 || slot>=INVENTORYSIZE) return;
    hi[slot+INVENTORYSIZE].valid_till=0;
}

static int display_hover(void) {
    int n,i,col,x,sx,sy,slot;
    char buf[4];

    if (invsel==-1) {
        if (weasel==-1) {
            if (consel==-1) return 0;
            else slot=consel+INVENTORYSIZE;
        } else slot=weatab[weasel];
    } else slot=invsel;

    if ((slot<INVENTORYSIZE && !item[slot]) || (slot>=INVENTORYSIZE && !container[slot-INVENTORYSIZE])) return 0;

    if (hi[slot].valid_till>=tick && tick-last_tick>HOVER_DELAY) {
        sx=mousex+8;
        if (sx<dotx(DOT_TL)) sx=dotx(DOT_TL);
        if (sx>dotx(DOT_BR)-hi[slot].width-8) sx=dotx(DOT_BR)-hi[slot].width-8;

        sy=mousey-hi[slot].cnt*5;
        if (sy<doty(DOT_TL)) sy=doty(DOT_TL);
        if (sy>doty(DOT_BR)-hi[slot].cnt*10-8) sy=doty(DOT_BR)-hi[slot].cnt*10-8;

        dd_shaded_rect(sx,sy,sx+hi[slot].width+8,sy+hi[slot].cnt*10+8,0x0000);

        for (n=0; n<MAXDESC; n++) {
            if (!hi[slot].desc[n]) break;

            x=sx+4;
            col=IRGB(24,24,24);

            for (i=0; hi[slot].desc[n][i]; i++) {

                if (hi[slot].desc[n][i]=='�') {
                    if (hi[slot].desc[n][i+1]=='c') {
                        if (isdigit(hi[slot].desc[n][i+2])) {
                            if (hi[slot].desc[n][i+2]=='5') col=IRGB(31,31,31);
                            else col=IRGB(16,16,16);
                            if (isdigit(hi[slot].desc[n][i+3])) {
                                i+=3; continue;
                            }
                            i+=2; continue;
                        }
                        i+=1; continue;
                    }
                    continue;
                }
                buf[0]=hi[slot].desc[n][i]; buf[1]=0;
                x=dd_drawtext(x,sy+n*10+4,col,0,buf);
            }
        }
        return 1;
    } else {
        if (!last_look && hi[slot].valid_till<tick) {
            if (slot<INVENTORYSIZE) cmd_look_inv(slot);
            else cmd_look_con(slot-INVENTORYSIZE);
            last_line=0;
            last_look=20;
            last_invsel=slot;
        }
        return 0;
    }
}

static int sklsel2=-1;
static void display_hover_update(void) {
    static int ivsel=-1,wsel=-1,csel=-1,ssel=-1,soff=0,ioff=0,coff=0;
    int x,y,i,v;

    sklsel2=-1;
    for (i=0; i<skltab_cnt; i++) {
        x=butx(i+BUT_SKL_BEG);
        y=buty(i+BUT_SKL_BEG);
        if (mousex>x+16 && mousex<x+SKLWIDTH && mousey>y-5 && mousey<y+5) {
            sklsel2=i;
            break;
        }
    }

    if (ivsel!=invsel || wsel!=weasel || csel!=consel || ssel!=sklsel2 ||
        soff!=skloff  || ioff!=invoff || coff!=conoff) {
        ivsel=invsel;
        wsel=weasel;
        csel=consel;
        ssel=sklsel2;
        soff=skloff;
        ioff=invoff;
        coff=conoff;
        last_tick=tick;
    }
}

int tactics2melee(int val) {
    return val*0.375;
}

int tactics2spell(int val) {
    return val*0.125;
}


static int display_hover_skill(void) {
    int sx,sy,height,width=200,v;

    if (sklsel2!=-1 && tick-last_tick>HOVER_DELAY) {
        v=skltab[sklsel2+skloff].v;
        if (v<0 || v>=*game_v_max) return 0;

        height=dd_drawtext_break_length(0,0,width-8,0xffff,0,game_skilldesc[v]);

        sx=mousex+8;
        if (sx<dotx(DOT_TL)) sx=dotx(DOT_TL);
        if (sx>dotx(DOT_BR)-width-8) sx=dotx(DOT_BR)-width-8;

        sy=mousey-height/2-4;
        if (sy<doty(DOT_TL)) sy=doty(DOT_TL);
        if (sy>doty(DOT_BR)-height-8) sy=doty(DOT_BR)-height-8;

        dd_shaded_rect(sx,sy,sx+width+8,sy+height+8,0x0000);

        sy=dd_drawtext_break(sx+4,sy+4,sx+width-8,0xffff,0,game_skilldesc[v]);
        sy+=10;

        return 1;
    }

    return 0;
}

