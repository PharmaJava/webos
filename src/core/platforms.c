/* Stremio Claude: the streaming platforms. A row of round badges on Home (each platform's own icon, as TMDB lists the
   ones of this country) and a page for each, as if the platform's own app were open: its Top 10 films and series of
   the moment, what is new, the best rated and its genres, every list asked of TMDB for that platform in this country.
   The titles open as any other (their IMDb id first), and the addons play them. */
#include "core/platforms.h"
#include "core/kids.h"
#include "core/tmdb.h"
#include "core/i18n.h"
#include "core/app.h"
#include "platform/plat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static PlatformParams cur;

const PlatformParams *platform_current(void) { return &cur; }

static void place(char *lang, size_t lc, char *reg, size_t rc) {
  const char *l = plat_ui_locale(); if (!l || !*l) l = "en-US";
  snprintf(lang, lc, "%s", l); char *u = strchr(lang, '_'); if (u) *u = '-';
  const char *d = strchr(lang, '-'); snprintf(reg, rc, "%s", d && strlen(d + 1) == 2 ? d + 1 : "US");
}
static void ago(char *out, size_t cap, int days) { time_t t = time(NULL) - (time_t)days * 86400; struct tm tm; gmtime_r(&t, &tm); strftime(out, cap, "%Y-%m-%d", &tm); }

static void def(CatalogDef *d, const char *title, const char *type, const char *path, const char *key, int skip, int top10) {
  memset(d, 0, sizeof *d); snprintf(d->title, sizeof d->title, "%s", title); snprintf(d->type, sizeof d->type, "%s", type);
  snprintf(d->url, sizeof d->url, "tmdbk:%s", path); snprintf(d->path, sizeof d->path, "%s", d->url); snprintf(d->key, sizeof d->key, "%s", key); d->skip = skip; d->top10 = top10; }


/* ---- without a TMDB key: the same platforms from the free Streaming Catalogs addon (no key, by country) ---- */
/* each platform: its TMDB provider id (the same "plat:<id>" cards and page as with TMDB), the addon's code and TMDB's
   logo (TMDB's pictures need no key) */
static const struct { int id; const char *code, *name, *logo; } OFFP[] = {
  { 8, "nfx", "Netflix", "/pbpMk2JmcoNnQwx5JGpXngfoWtp.jpg" }, { 337, "dnp", "Disney Plus", "/7rwgEs15tFwyR9NPQ5vpzxTj19Q.jpg" },
  { 1899, "hbm", "HBO Max", "/jbe4gVSfRlbPTdESXhEKpornsfu.jpg" }, { 119, "amp", "Amazon Prime Video", "/pvske1MyAoymrs5bguRfVqYiM9a.jpg" },
  { 350, "atp", "Apple TV+", "/2E03IAZsX4ZaUqM7tXlctEPMGWS.jpg" }, { 149, "mp9", "Movistar Plus+", "/jse4MOi92Jgetym7nbXFZZBI6LK.jpg" },
  { 283, "cru", "Crunchyroll", "/fzN5Jok5Ig1eJ7gyNGoMhnLSCfh.jpg" }, { 531, "pmp", "Paramount Plus", "/xbhHHa1YgtpwhC8lb1NQ3ACVcLd.jpg" },
  { 15, "hlu", "Hulu", "/zxrVdFjIjLqkfnwyghnfywTn3Lh.jpg" }, { 386, "pcp", "Peacock", "/8VCV78prwd9QzZnEm0ReO6bERDa.jpg" } };
#define NOFFP (int)(sizeof OFFP / sizeof OFFP[0])
/* which of them each country has (the addon's own lists); any other country: the ones nearly everywhere */
static const char *off_codes(const char *reg) {
  static const struct { const char *reg, *codes; } C[] = { { "ES", "nfx,dnp,hbm,amp,atp,mp9,cru" }, { "US", "nfx,dnp,hbm,amp,atp,cru,pmp,hlu,pcp" },
    { "GB", "nfx,dnp,hbm,amp,atp,cru" }, { "BR", "nfx,dnp,hbm,amp,atp,cru,pmp" }, { "IN", "nfx,amp,atp,cru" }, { "TR", "nfx,dnp,hbm,amp,cru" },
    { "NL", "nfx,dnp,hbm,amp,atp,cru" }, { "DE", "nfx,dnp,hbm,amp,atp,cru" }, { "FR", "nfx,dnp,hbm,amp,atp" } };
  for (size_t i = 0; i < sizeof C / sizeof C[0]; i++) if (!strcmp(C[i].reg, reg)) return C[i].codes;
  return "nfx,dnp,hbm,amp,atp,cru"; }
static int off_has(const char *codes, const char *code) { size_t n = strlen(code); for (const char *p = codes; (p = strstr(p, code)); p += n) if ((p == codes || p[-1] == ',') && (p[n] == ',' || !p[n])) return 1; return 0; }
/* the addon's address for this country: its setup ("codes::REG:time:0:0:") in base64 */
static void off_base(char *out, size_t cap) {
  char lang[24], reg[8]; place(lang, sizeof lang, reg, sizeof reg); char cfg[160]; snprintf(cfg, sizeof cfg, "%s::%s:1764160000000:0:0:", off_codes(reg), reg);
  static const char *A = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; char b[256]; size_t n = strlen(cfg), o = 0;
  for (size_t i = 0; i < n && o + 5 < sizeof b; i += 3) { unsigned v = (unsigned char)cfg[i] << 16 | (i + 1 < n ? (unsigned char)cfg[i + 1] << 8 : 0) | (i + 2 < n ? (unsigned char)cfg[i + 2] : 0);
    b[o++] = A[v >> 18 & 63]; b[o++] = A[v >> 12 & 63]; b[o++] = i + 1 < n ? A[v >> 6 & 63] : '='; b[o++] = i + 2 < n ? A[v & 63] : '='; }
  b[o] = 0; char e[300]; size_t k = 0; for (size_t i = 0; b[i] && k + 4 < sizeof e; i++) { if (b[i] == '=') { memcpy(e + k, "%3D", 3); k += 3; } else if (b[i] == '+') { memcpy(e + k, "%2B", 3); k += 3; } else if (b[i] == '/') { memcpy(e + k, "%2F", 3); k += 3; } else e[k++] = b[i]; } e[k] = 0;
  snprintf(out, cap, "https://7a82163c306e-stremio-netflix-catalog-addon.baby-beamup.club/%s", e); }
/* the row of icons: written as TMDB would answer (provider_id, provider_name, logo_path), read back as any other */
int platforms_offline_json(char *out, size_t cap) {
  char lang[24], reg[8]; place(lang, sizeof lang, reg, sizeof reg); const char *codes = off_codes(reg); size_t o = 0; int n = 0;
  o += snprintf(out + o, cap - o, "{\"results\":[");
  for (int i = 0; i < NOFFP && o + 200 < cap; i++) if (off_has(codes, OFFP[i].code))
    o += snprintf(out + o, cap - o, "%s{\"provider_id\":%d,\"provider_name\":\"%s\",\"logo_path\":\"%s\",\"display_priority\":%d}", n++ ? "," : "", OFFP[i].id, OFFP[i].name, OFFP[i].logo, i);
  o += snprintf(out + o, cap - o, "]}"); return n; }
static int off_page(CatalogDef *out, int max) {
  int k = -1; for (int i = 0; i < NOFFP; i++) if (OFFP[i].id == cur.id) k = i; if (k < 0 || max < 2) return 0;
  char base[400], path[600], key[48]; off_base(base, sizeof base); const char *P = cur.name; int n = 0;
  const char *types[2] = { "movie", "series" };
  for (int t = 0; t < 2 && n < max; t++) { CatalogDef *d = &out[n++]; memset(d, 0, sizeof *d);
    snprintf(d->title, sizeof d->title, "%s", t ? T1("Series on {p}", "p", P) : T1("Films on {p}", "p", P)); snprintf(d->type, sizeof d->type, "%s", types[t]);
    snprintf(path, sizeof path, "%s/catalog/%s/%s", base, types[t], OFFP[k].code); snprintf(d->path, sizeof d->path, "%s", path); snprintf(d->url, sizeof d->url, "%s.json", path);
    snprintf(key, sizeof key, "po%d_%c", cur.id, t ? 's' : 'm'); snprintf(d->key, sizeof d->key, "%s", key); d->skip = 1; if (!t) d->wide = 0; }
  return n; }
int platforms_row(CatalogDef *out) {
  if (!tmdb_has_key()) { def(out, T("Platforms"), "platform", "o/", "plat_row", 0, 0); out->round = 1; return 1; } /* (no key: the addon's platforms) */
  char lang[24], reg[8], path[200]; place(lang, sizeof lang, reg, sizeof reg);
  snprintf(path, sizeof path, "p/watch/providers/movie?watch_region=%s&language=%s", reg, lang);
  def(out, T("Platforms"), "platform", path, "plat_row", 0, 0); out->round = 1; return 1;
}

/* the premieres: films reaching streaming in this country in the next weeks, and new series (their day on each card) */
int premieres_rows(CatalogDef *out, int max) {
  if (!tmdb_has_key() || max < 2) return 0;
  char lang[24], reg[8], a[16], b[16], c[16], path[480]; place(lang, sizeof lang, reg, sizeof reg); ago(a, sizeof a, 2); ago(b, sizeof b, -40); ago(c, sizeof c, 60);
  snprintf(path, sizeof path, "u/discover/movie?region=%s&with_release_type=4&release_date.gte=%s&release_date.lte=%s&sort_by=popularity.desc&language=%s", reg, a, b, lang);
  def(&out[0], T("Coming to streaming"), "movie", path, "prem_m", 0, 0); out[0].wide = 1;
  snprintf(path, sizeof path, "u/discover/tv?first_air_date.gte=%s&first_air_date.lte=%s&sort_by=popularity.desc&without_genres=10763,10767,10764&language=%s", a, b, lang);
  def(&out[1], T("New series premieres"), "series", path, "prem_s", 0, 0); out[1].wide = 1;
  (void)c; return 2;
}

int platform_defs(CatalogDef *out, int max) {
  if (!cur.id) return 0; if (!tmdb_has_key()) return off_page(out, max); /* (no key: its films and series from the addon) */
  char lang[24], reg[8], now[16], d90[16], d30[16]; place(lang, sizeof lang, reg, sizeof reg); ago(now, sizeof now, 0); ago(d90, sizeof d90, 120); ago(d30, sizeof d30, 30);
  char base[96]; snprintf(base, sizeof base, "with_watch_providers=%d&watch_region=%s&language=%s", cur.id, reg, lang);
  const char *P = cur.name; int n = 0; char path[480], key[48];
#define ROW(title, type, k, skip, top) do { if (n < max) { snprintf(key, sizeof key, "p%d_%s", cur.id, k); def(&out[n++], title, type, path, key, skip, top); } } while (0)
  snprintf(path, sizeof path, "/discover/movie?%s&sort_by=popularity.desc&vote_count.gte=20", base); ROW(T1("Top 10 films on {p}", "p", P), "movie", "tm", 0, 1);
  snprintf(path, sizeof path, "/discover/tv?%s&sort_by=popularity.desc&vote_count.gte=20&without_genres=10763,10767", base); ROW(T1("Top 10 series on {p}", "p", P), "series", "ts", 0, 1);
  snprintf(path, sizeof path, "/discover/movie?%s&sort_by=popularity.desc&primary_release_date.gte=%s&primary_release_date.lte=%s", base, d90, now); ROW(T("New films"), "movie", "nm", 1, 0);
  snprintf(path, sizeof path, "/discover/tv?%s&sort_by=popularity.desc&air_date.gte=%s&air_date.lte=%s&without_genres=10763,10767", base, d30, now); ROW(T("New episodes"), "series", "ns", 1, 0);
  snprintf(path, sizeof path, "/discover/movie?%s&sort_by=vote_average.desc&vote_count.gte=500", base); ROW(T("Best-rated films"), "movie", "rm", 1, 0);
  snprintf(path, sizeof path, "/discover/tv?%s&sort_by=vote_average.desc&vote_count.gte=300", base); ROW(T("Best-rated series"), "series", "rs", 1, 0);
  static const struct { const char *name; int id; } MG[] = { { "Action", 28 }, { "Comedy", 35 }, { "Drama", 18 }, { "Thriller", 53 }, { "Sci-Fi", 878 }, { "Horror", 27 },
    { "Romance", 10749 }, { "Animation", 16 }, { "Family", 10751 }, { "Documentary", 99 } };
  static const struct { const char *name; int id; } SG[] = { { "Crime", 80 }, { "Comedy", 35 }, { "Drama", 18 }, { "Sci-Fi", 10765 }, { "Action", 10759 }, { "Animation", 16 }, { "Documentary", 99 } };
  for (size_t i = 0; i < sizeof MG / sizeof MG[0] && n < max; i++) { char t[96]; snprintf(t, sizeof t, "%s · %s", T(MG[i].name), T("Movies"));
    snprintf(path, sizeof path, "/discover/movie?%s&with_genres=%d&sort_by=popularity.desc&vote_count.gte=30", base, MG[i].id); snprintf(key, sizeof key, "p%d_gm%d", cur.id, MG[i].id); def(&out[n++], t, "movie", path, key, 1, 0); }
  for (size_t i = 0; i < sizeof SG / sizeof SG[0] && n < max; i++) { char t[96]; snprintf(t, sizeof t, "%s · %s", T(SG[i].name), T("Series"));
    snprintf(path, sizeof path, "/discover/tv?%s&with_genres=%d&sort_by=popularity.desc&vote_count.gte=20", base, SG[i].id); snprintf(key, sizeof key, "p%d_gs%d", cur.id, SG[i].id); def(&out[n++], t, "series", path, key, 1, 0); }
  return n;
#undef ROW
}

int platform_open(const Item *it) {
  if (strncmp(it->id, "plat:", 5)) return 0;
  PlatformParams p; memset(&p, 0, sizeof p); p.id = atoi(it->id + 5); snprintf(p.name, sizeof p.name, "%s", it->name); snprintf(p.logo, sizeof p.logo, "%s", it->logo[0] ? it->logo : it->poster);
  app_go(SCR_PLATFORM, &p, 0); return 1;
}
int platform_enter(const PlatformParams *p) { /* 1 when it is another platform than last time (its rows made anew) */
  if (!p) return 0; int other = p->id != cur.id; cur = *p; return other; }
