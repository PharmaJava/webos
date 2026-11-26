/* Stremio Claude on the phone: a platform's page (Netflix, Disney+…), as on the TV: its badge at the top, then its Top 10 films
   and series, what is new, the best rated and its genres, every list asked of TMDB for that platform in this country. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "phone/phone.h"
#include "core/keys.h"
#include "core/i18n.h"
#include "core/platforms.h"
#include "ui/text.h"
#include "ui/img.h"
#include "ui/icon.h"
#include "ui/widgets.h"

#define MAXP 30
static struct { CatalogDef def[MAXP]; ItemList list[MAXP]; int got[MAXP]; Scroll s[MAXP]; int n; unsigned gen; Scroll v; int fr, fi; } P;

static void got(ItemList *l, int cached, void *ud) { (void)cached; unsigned t = (unsigned)(uintptr_t)ud; int i = t & 255;
  if ((t >> 8) != (P.gen & 0xffffff) || i >= P.n || !l) { if (l) itemlist_free(l); return; }
  if (l->n > 0 || !P.got[i]) { itemlist_clear(&P.list[i]); P.list[i] = *l; free(l); } else itemlist_free(l); P.got[i] = 1; }
static void load(void) {
  for (int i = 0; i < P.n; i++) itemlist_clear(&P.list[i]); memset(P.got, 0, sizeof P.got); memset(P.s, 0, sizeof P.s);
  P.n = platform_defs(P.def, MAXP); P.gen++; P.v.pos = 0; P.v.vel = 0; P.fr = -1;
  for (int i = 0; i < P.n; i++) sb_catalog(&P.def[i], got, (void *)(uintptr_t)(((P.gen & 0xffffff) << 8) | (unsigned)i));
}
static void enter(const void *params) { bg_clear(); if (platform_enter(params) || !P.n) load(); }
static void leave(void) {}
static void focus(int r, int i) { P.fr = r; P.fi = i; }
static const Item *focused(void) { return P.fr >= 0 && P.fr < P.n && P.fi >= 0 && P.fi < P.list[P.fr].n ? &P.list[P.fr].items[P.fi] : NULL; }
static void back_act(int a, int b) { (void)a; (void)b; app_back(); }
static void tick(float dt) { (void)dt; }
static void draw(void) {
  const PlatformParams *pp = platform_current();
  gfx_rect(0, 0, SB_W, SB_H, PAGE);
  ph_drag(0, 0, SB_W, SB_H - INS_B, &P.v, 1); scroll_tick(&P.v);
  float y = INS_T + 120 - P.v.pos;
  for (int i = 0; i < P.n; i++) {
    if (!P.got[i]) { ph_text(PAD, y + 10, 260, 1, F_H2 - 2, 40, ALIGN_START); for (int c = 0; c < 4; c++) ph_box(PAD + c * (POSTER_W + 14), y + 52, POSTER_W, POSTER_H, 14); y += 52 + POSTER_H + 36; continue; }
    char t[200]; snprintf(t, sizeof t, "%s", T(P.def[i].title));
    y += row_draw_kind(t, &P.list[i], P.def[i].top10 ? 3 : 0, y, &P.s[i], i, focus, P.fr == i ? P.fi : -1); }
  P.v.max = y + P.v.pos - SB_H + INS_B + 40;
  /* the top: Back, the platform's own icon and its name, over a shade */
  gfx_rect(0, 0, SB_W, INS_T + 112, col_alpha(PAGE | 0xff000000u, 0.92f));
  float r = 36, cy = INS_T + 56; int pr = ph_pressed(PAD - 8, cy - r, 2 * r, 2 * r);
  gfx_circle(PAD - 8 + r, cy, r, pr ? WHITE_A(40) : WHITE_A(22)); icon_draw("ph_back", PAD - 8 + r - 20, cy - 20, 40, WHITE_A(240)); hit_act(PAD - 8, cy - r, 2 * r + 20, 2 * r, back_act, 0, 0);
  float ix = PAD + 2 * r + 12, s = 64; GLuint t = pp->logo[0] ? img_get(pp->logo, (int)s, (int)s, NULL) : 0;
  if (t) gfx_image(t, ix, cy - s / 2, s, s, 16, WHITE_A(255), 0, 0, 1, 1); else gfx_rrect(ix, cy - s / 2, s, s, 16, WHITE_A(30));
  text_draw(pp->name, ix + s + 16, cy - text_line_height(F_H2) / 2.0f, F_H2, 800, TXT, -0.3f, SB_W - ix - s - 16 - PAD, ALIGN_START);
}
static int key(int k) {
  if (k == K_OK) { const Item *it = focused(); if (it) app_open_item(it); return 1; }
  if (k == K_CH_UP) { scroll_top(&P.v); return 1; }
  return 0; /* (Back: where it was opened from) */
}
static int origin(float *x, float *y) { *x = SB_W / 2; *y = SB_H / 2; return 0; }
Screen phPlatform = { enter, leave, key, tick, draw, focused, NULL, NULL, origin };
