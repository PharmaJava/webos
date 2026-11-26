/* Stremio Claude on the phone: "Who's watching?", as on the TV. Each profile has its own history, Continue Watching and
   recommendations; a child's shows only children's titles and can have a daily limit and a bedtime; any profile can be
   locked with the parent PIN. Tap a profile to watch as it; "Edit" to rename it, give it an avatar or change its settings
   (the guarded ones ask the parent PIN, or make it the first time). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "phone/phone.h"
#include "core/keys.h"
#include "core/i18n.h"
#include "core/profiles.h"
#include "core/store.h"
#include "core/notify.h"
#include "core/scrobble.h"
#include "core/traktwatch.h"
#include "platform/plat.h"
#include "screens/pinpad.h"
#include "ui/text.h"
#include "ui/img.h"
#include "ui/icon.h"
#include "ui/widgets.h"
#include <SDL2/SDL.h>

static struct { int edit, mode, id, avatarFor; char name[PROFILE_NAME_CAP]; Scroll v; } P; /* mode: 0 the grid, 1 naming (add or rename), 2 avatars */
static TEd ed;

static const char *label(int id) { const char *n = profiles_name(id); return n && *n ? n : T("Default"); }
static void avatar(int choice, float x, float y, float d) {
  char url[600]; snprintf(url, sizeof url, "file://%s/avatars/%d.webp", plat_asset_dir(), choice);
  GLuint t = img_get(url, 288, 288, NULL); if (t) gfx_image(t, x, y, d, d, d / 2, WHITE_A(255), 0, 0, 1, 1); else gfx_rrect(x, y, d, d, d / 2, WHITE_A(30)); }

/* watching as this profile: what the last one had is let go, the boards made again */
int ph_pick_profile(int id) {
  if (!profiles_name(id)) return 0;
  if (profiles_active() && profiles_active() != id) { extern void board_profile_changed(void); store_flush(); scrobble_leave_profile(); tw_reset(); notify_reset(); lib_reset_sync(); lib_invalidate(); board_profile_changed(); }
  if (profiles_active() != id && !profiles_select(id)) return 0;
  app_go(SCR_HOME, NULL, 1); sb_sync_soon(1); return 1; }
static void on_pick_pin(int ok, void *ud) { if (ok) ph_pick_profile((int)(intptr_t)ud); }
static void pick(int id) { if (profiles_int(id, "lock", 0) && pin_is_set() && id != profiles_active()) { pin_ask(PIN_CHECK, label(id), on_pick_pin, (void *)(intptr_t)id); return; } ph_pick_profile(id); }

/* ---- a profile's settings: a list of choices, the guarded ones behind the parent PIN ---- */
enum { O_RENAME, O_AVATAR, O_KIDS, O_LIMIT, O_BED, O_LOCK, O_DELETE };
static int opts[8], nopts, unlocked;
static const int LIMITS[] = { 0, 30, 60, 90, 120, 180 }, BEDS[] = { 0, 1140, 1170, 1200, 1230, 1260, 1290, 1320 };
static void next_of(const char *key, const int *v, int n) { int cur = profiles_int(P.id, key, 0), i = 0; while (i < n && v[i] != cur) i++; profiles_set_int(P.id, key, v[(i + 1) % n]); }
static void manage(int id);
static void do_opt(int o) {
  if (o == O_KIDS) { if (profiles_set_kids(P.id, !profiles_kids(P.id)) && P.id == profiles_active()) { extern void board_profile_changed(void); board_profile_changed(); } }
  else if (o == O_LIMIT) next_of("limit", LIMITS, 6);
  else if (o == O_BED) next_of("bed", BEDS, 8);
  else if (o == O_LOCK) profiles_set_int(P.id, "lock", !profiles_int(P.id, "lock", 0));
  else if (o == O_DELETE) { if (profiles_count() > 1 && P.id != profiles_active()) { profiles_delete(P.id); toast(T("Profile deleted"), 1600, NULL); } return; }
  manage(P.id); }
static void on_opt_pin(int ok, void *ud) { if (!ok) return; unlocked = 1; do_opt((int)(intptr_t)ud); }
static void opt_pick(int i) {
  if (i < 0 || i >= nopts) return; int o = opts[i];
  if (o == O_RENAME) { P.mode = 1; snprintf(P.name, sizeof P.name, "%s", profiles_name(P.id) ? profiles_name(P.id) : ""); ed.s = P.name; ed.cap = sizeof P.name; ted_reset(&ed); phone_keyboard(1); return; }
  if (o == O_AVATAR) { P.mode = 2; P.avatarFor = P.id; return; }
  if (unlocked) { do_opt(o); return; }
  if (!pin_is_set() && o != O_DELETE) pin_ask(PIN_NEW, T("Parent PIN"), on_opt_pin, (void *)(intptr_t)o);
  else pin_ask(PIN_CHECK, T("Parent PIN"), on_opt_pin, (void *)(intptr_t)o); }
static void manage(int id) {
  static char ln[8][96]; const char *lines[8]; P.id = id; nopts = 0; char v[24];
#define ADD(o, txt) do { opts[nopts] = (o); snprintf(ln[nopts], sizeof ln[nopts], "%s", (txt)); lines[nopts] = ln[nopts]; nopts++; } while (0)
  ADD(O_RENAME, T("Rename profile")); ADD(O_AVATAR, T("Change avatar"));
  ADD(O_KIDS, T(profiles_kids(id) ? "Kids profile: on" : "Kids profile: off"));
  if (profiles_kids(id)) { int m = profiles_int(id, "limit", 0); if (!m) ADD(O_LIMIT, T("Daily limit: off")); else { if (m % 60) snprintf(v, sizeof v, m >= 60 ? "%d h %d min" : "%d min", m >= 60 ? m / 60 : m, m % 60); else snprintf(v, sizeof v, "%d h", m / 60); ADD(O_LIMIT, T1("Daily limit: {t}", "t", v)); }
    int b = profiles_int(id, "bed", 0); if (!b) ADD(O_BED, T("Bedtime: off")); else { snprintf(v, sizeof v, "%02d:%02d", b / 60, b % 60); ADD(O_BED, T1("Bedtime: {t}", "t", v)); } }
  else ADD(O_LOCK, T(profiles_int(id, "lock", 0) ? "Locked with PIN: on" : "Locked with PIN: off"));
  if (profiles_count() > 1 && id != profiles_active()) ADD(O_DELETE, T("Delete profile"));
#undef ADD
  choices_open(label(id), lines, nopts, -1, opt_pick); }

/* ---- the grid ---- */
static void tile_act(int i, int b) { (void)b; int id = profiles_id(i);
  if (!id) { P.mode = 1; P.id = 0; P.name[0] = 0; ed.s = P.name; ed.cap = sizeof P.name; ted_reset(&ed); phone_keyboard(1); return; }
  if (P.edit) { unlocked = 0; manage(id); } else pick(id); }
static void edit_act(int a, int b) { (void)a; (void)b; P.edit = !P.edit; }
static void set_act(int a, int b) { (void)a; (void)b; app_go(SCR_SET, NULL, 0); }
static void save_name(void) {
  char *s = P.name; while (*s == ' ') s++; if (!*s) { toast(T("Enter a profile name"), 1600, NULL); return; }
  phone_keyboard(0);
  if (P.id) { profiles_rename(P.id, s); P.mode = 0; return; }
  int id = profiles_add(s); if (id > 1) { P.mode = 2; P.avatarFor = id; } else P.mode = 0; } /* (a new one goes on to its avatar) */
static void save_act(int a, int b) { (void)a; (void)b; save_name(); }
static void field_act(int a, int b) { (void)a; (void)b; phone_keyboard(1); }
static void av_act(int a, int b) { (void)b; profiles_set_avatar(P.avatarFor, a); P.mode = 0; }

static void enter(const void *p) { (void)p; bg_clear(); P.mode = 0; P.edit = 0; P.v.pos = 0; }
static void leave(void) { phone_keyboard(0); }
static void text_in(const char *t) { if (P.mode == 1) ted_insert(&ed, t); }
static void draw(void) {
  gfx_rect(0, 0, SB_W, SB_H, PAGE);
  float y = INS_T + 40;
  if (P.mode == 2) { /* avatars: three a row */
    text_draw(T("Choose an avatar"), SB_W / 2, y, F_H1, 800, TXT, -0.5f, SB_W - 2 * PAD, ALIGN_CENTER); y += 110;
    int cols = SB_W > 900 ? 5 : 3; float gap = 28, d = (SB_W - 2 * PAD - gap * (cols - 1)) / cols;
    ph_drag(0, y, SB_W, SB_H - y, &P.v, 1); scroll_tick(&P.v);
    for (int i = 0; i < PROFILE_AVATAR_COUNT; i++) { float x = PAD + (i % cols) * (d + gap), yy = y + (i / cols) * (d + gap) - P.v.pos; if (yy > SB_H || yy + d < 0) continue;
      avatar(i, x, yy, d); hit_act(x, yy, d, d, av_act, i, 0); }
    P.v.max = ((PROFILE_AVATAR_COUNT + cols - 1) / cols) * (d + gap) - (SB_H - y) + 40; return; }
  if (P.mode == 1) { /* the name */
    text_draw(P.id ? T("Rename profile") : T("Add profile"), PAD, y, F_H1, 800, TXT, -0.5f, SB_W - 2 * PAD, ALIGN_START); y += 120;
    if (ted_field(&ed, PAD, y, SB_W - 2 * PAD, 84, F_BODY + 2, 500, TXT, T("Profile name"), WHITE_A(110), 1, 0)) field_act(0, 0);
    gfx_rect(PAD, y + 82, SB_W - 2 * PAD, 2, 0xffb794ffu); y += 130;
    button(SB_W - PAD - 260, y, 260, 80, "check", T("Save"), 1, save_act, 0, 0); return; }
  text_draw(T("Who's watching?"), SB_W / 2, y + 20, F_H1, 800, TXT, -0.5f, SB_W - 2 * PAD, ALIGN_CENTER); y += 150;
  int total = profiles_count() + (profiles_count() < PROFILE_MAX), cols = SB_W > 900 ? 4 : 3; float gap = 30, d = fminf(190, (SB_W - 2 * PAD - gap * (cols - 1)) / cols), rowW = cols * d + (cols - 1) * gap, x0 = (SB_W - rowW) / 2;
  for (int i = 0; i < total; i++) { float x = x0 + (i % cols) * (d + gap), yy = y + (i / cols) * (d + 96); int id = profiles_id(i); int pr = ph_pressed(x, yy, d, d); float k = pr ? 0.95f : 1;
    gfx_scale_push(k, k, x + d / 2, yy + d / 2);
    if (id) avatar(profiles_avatar(id), x, yy, d); else { gfx_circle(x + d / 2, yy + d / 2, d / 2, WHITE_A(22)); icon_draw("plus", x + d * 0.32f, yy + d * 0.32f, d * 0.36f, WHITE_A(220)); }
    if (id && P.edit) { gfx_circle(x + d / 2, yy + d / 2, d / 2, BLACK_A(110)); icon_draw("set_out", x + d * 0.34f, yy + d * 0.34f, d * 0.32f, WHITE_A(240)); }
    if (id && profiles_int(id, "lock", 0)) icon_draw("lock", x + d - 40, yy + d - 40, 34, WHITE_A(230));
    gfx_scale_pop();
    text_draw(id ? label(id) : T("Add profile"), x + d / 2, yy + d + 12, F_BODY, id == profiles_active() ? 800 : 600, WHITE_A(id ? 240 : 180), 0, d + gap - 6, ALIGN_CENTER);
    if (id && profiles_kids(id)) text_draw(T("Kids"), x + d / 2, yy + d + 52, F_SMALL, 600, WHITE_A(150), 0, d + gap - 6, ALIGN_CENTER);
    hit_act(x, yy, d, d + 80, tile_act, i, 0); }
  float by = SB_H - INS_B - 120, bw = (SB_W - 2 * PAD - 16) / 2;
  button(PAD, by, bw, 80, "set_out", P.edit ? T("Done") : T("Edit profiles"), 0, edit_act, 0, 0);
  button(PAD + bw + 16, by, bw, 80, "nav_set", T("Settings"), 0, set_act, 0, 0);
}
static int key(int k) {
  if (P.mode == 1) { if (ted_key(&ed, k)) return 1; if (k == K_BACK) { if (P.name[0]) ted_backspace(&ed); else { phone_keyboard(0); P.mode = 0; } return 1; } if (k == K_OK) { save_name(); return 1; } return 1; }
  if (P.mode == 2 && k == K_BACK) { P.mode = 0; return 1; }
  if (k == K_BACK && profiles_active()) { app_go(SCR_HOME, NULL, 1); return 1; }
  return 0; }
static void tick(float dt) {
  (void)dt; /* one profile and nobody chosen yet: straight in, as before profiles */
  if (!profiles_active() && profiles_count() == 1 && P.mode == 0) ph_pick_profile(profiles_id(0)); }
static int origin(float *x, float *y) { *x = SB_W / 2; *y = SB_H / 2; return 0; }
Screen phProfiles = { enter, leave, key, tick, draw, NULL, NULL, text_in, origin };
