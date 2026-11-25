/* Stremio Claude: kids profiles. A profile marked for a child sees only children's films and series, in every tab:
   rows asked of TMDB (Disney+ news, the big animation studios, children's films and series), with an age limit (US
   rating G or PG for films, TMDB's Kids genre for series) and no horror, thriller, crime or war. Discover and Search
   take the same rule. Titles come as "tmdb:<id>": opening one asks TMDB for its IMDb id first, so the addons find its
   streams. Without a TMDB key, Cinemeta's Family and Animation lists. */
#include "core/kids.h"
#include "core/profiles.h"
#include "core/tmdb.h"
#include "core/net.h"
#include "core/json.h"
#include "core/i18n.h"
#include "core/app.h"
#include "platform/plat.h"
#include "screens/detail.h"
#include "ui/widgets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PFX "tmdbk:"
#define MOVIE_SAFE "&include_adult=false&certification_country=US&certification.lte=PG&without_genres=27,53,80,10752"
#define TV_SAFE "&include_adult=false&without_genres=80,99,10764,10766,10767,10768"

int kids_on(void) { int id = profiles_active(); return id > 0 && profiles_kids(id); }
int kids_url(const char *url) { return url && !strncmp(url, PFX, sizeof PFX - 1); }

/* the TV's language and country for TMDB ("es-ES", "ES"); English and the US when it says nothing */
static void place(char *lang, size_t lc, char *reg, size_t rc) {
  const char *l = plat_ui_locale(); if (!l || !*l) l = "en-US";
  snprintf(lang, lc, "%s", l); char *u = strchr(lang, '_'); if (u) *u = '-';
  const char *d = strchr(lang, '-'); snprintf(reg, rc, "%s", d && strlen(d + 1) == 2 ? d + 1 : "US");
}
static void ago(char *out, size_t cap, int days) { time_t t = time(NULL) - (time_t)days * 86400; struct tm tm; gmtime_r(&t, &tm); strftime(out, cap, "%Y-%m-%d", &tm); }

static void def(CatalogDef *d, const char *title, const char *type, const char *path, const char *key) {
  memset(d, 0, sizeof *d); snprintf(d->title, sizeof d->title, "%s", title); snprintf(d->type, sizeof d->type, "%s", type);
  snprintf(d->url, sizeof d->url, PFX "%s", path); snprintf(d->path, sizeof d->path, "%s", d->url); snprintf(d->key, sizeof d->key, "k_%s", key); d->skip = 1; }

typedef struct { const char *title, *type, *query, *key; } KRow;

/* each tab's rows: %R is the country, %L the language, %D the date a year and a half ago */
static const KRow HOME[] = {
  { "New on Disney+", "movie", "/discover/movie?with_watch_providers=337&watch_region=%R&sort_by=popularity.desc&primary_release_date.gte=%D" MOVIE_SAFE, "dnew" },
  { "Disney+ series", "series", "/discover/tv?with_watch_providers=337&watch_region=%R&with_genres=10762&sort_by=popularity.desc" TV_SAFE, "dser" },
  { "Popular children's films", "movie", "/discover/movie?with_genres=10751&sort_by=popularity.desc&vote_count.gte=50" MOVIE_SAFE, "mpop" },
  { "Children's series", "series", "/discover/tv?with_genres=10762&sort_by=popularity.desc&vote_count.gte=10" TV_SAFE, "spop" },
  { "Pixar", "movie", "/discover/movie?with_companies=3&sort_by=popularity.desc" MOVIE_SAFE, "pixar" },
  { "Walt Disney Animation", "movie", "/discover/movie?with_companies=6125&sort_by=popularity.desc" MOVIE_SAFE, "wdas" },
  { "DreamWorks Animation", "movie", "/discover/movie?with_companies=521&sort_by=popularity.desc" MOVIE_SAFE, "dwa" },
  { "Illumination", "movie", "/discover/movie?with_companies=6704&sort_by=popularity.desc" MOVIE_SAFE, "illu" },
  { "New children's films", "movie", "/discover/movie?with_genres=10751&sort_by=popularity.desc&primary_release_date.gte=%D" MOVIE_SAFE, "mnew" },
  { "Best-rated animated films", "movie", "/discover/movie?with_genres=16&sort_by=vote_average.desc&vote_count.gte=1500" MOVIE_SAFE, "anitop" },
  { "Disney classics", "movie", "/discover/movie?with_companies=2&with_genres=16&primary_release_date.lte=2005-12-31&sort_by=vote_count.desc" MOVIE_SAFE, "dclas" },
  { "Studio Ghibli", "movie", "/discover/movie?with_companies=10342&sort_by=popularity.desc" MOVIE_SAFE, "ghibli" },
};
static const KRow FILMS[] = {
  { "New on Disney+", "movie", "/discover/movie?with_watch_providers=337&watch_region=%R&sort_by=popularity.desc&primary_release_date.gte=%D" MOVIE_SAFE, "dnew" },
  { "Popular on Disney+", "movie", "/discover/movie?with_watch_providers=337&watch_region=%R&sort_by=popularity.desc" MOVIE_SAFE, "dpop" },
  { "Popular children's films", "movie", "/discover/movie?with_genres=10751&sort_by=popularity.desc&vote_count.gte=50" MOVIE_SAFE, "mpop" },
  { "New children's films", "movie", "/discover/movie?with_genres=10751&sort_by=popularity.desc&primary_release_date.gte=%D" MOVIE_SAFE, "mnew" },
  { "Best-rated animated films", "movie", "/discover/movie?with_genres=16&sort_by=vote_average.desc&vote_count.gte=1500" MOVIE_SAFE, "anitop" },
  { "Pixar", "movie", "/discover/movie?with_companies=3&sort_by=popularity.desc" MOVIE_SAFE, "pixar" },
  { "Walt Disney Animation", "movie", "/discover/movie?with_companies=6125&sort_by=popularity.desc" MOVIE_SAFE, "wdas" },
  { "DreamWorks Animation", "movie", "/discover/movie?with_companies=521&sort_by=popularity.desc" MOVIE_SAFE, "dwa" },
  { "Illumination", "movie", "/discover/movie?with_companies=6704&sort_by=popularity.desc" MOVIE_SAFE, "illu" },
  { "Disney classics", "movie", "/discover/movie?with_companies=2&with_genres=16&primary_release_date.lte=2005-12-31&sort_by=vote_count.desc" MOVIE_SAFE, "dclas" },
  { "Adventures", "movie", "/discover/movie?with_genres=10751,12&sort_by=popularity.desc&vote_count.gte=50" MOVIE_SAFE, "madv" },
  { "Comedies", "movie", "/discover/movie?with_genres=10751,35&sort_by=popularity.desc&vote_count.gte=50" MOVIE_SAFE, "mcom" },
  { "Fantasy", "movie", "/discover/movie?with_genres=10751,14&sort_by=popularity.desc&vote_count.gte=50" MOVIE_SAFE, "mfan" },
};
static const KRow SERIES[] = {
  { "Disney+ series", "series", "/discover/tv?with_watch_providers=337&watch_region=%R&with_genres=10762&sort_by=popularity.desc" TV_SAFE, "dser" },
  { "Children's series", "series", "/discover/tv?with_genres=10762&sort_by=popularity.desc&vote_count.gte=10" TV_SAFE, "spop" },
  { "New children's series", "series", "/discover/tv?with_genres=10762&sort_by=popularity.desc&first_air_date.gte=%D" TV_SAFE, "snew" },
  { "Best-rated children's series", "series", "/discover/tv?with_genres=10762&sort_by=vote_average.desc&vote_count.gte=150" TV_SAFE, "stop" },
  { "Animated series for children", "series", "/discover/tv?with_genres=16,10762&sort_by=popularity.desc" TV_SAFE, "sani" },
  { "Children's series on Netflix", "series", "/discover/tv?with_watch_providers=8&watch_region=%R&with_genres=10762&sort_by=popularity.desc" TV_SAFE, "snfx" },
  { "Children's series on Prime Video", "series", "/discover/tv?with_watch_providers=119&watch_region=%R&with_genres=10762&sort_by=popularity.desc" TV_SAFE, "sprime" },
};
static const KRow ANIME[] = {
  { "Studio Ghibli", "movie", "/discover/movie?with_companies=10342&sort_by=popularity.desc" MOVIE_SAFE, "ghibli" },
  { "Japanese animated films", "movie", "/discover/movie?with_origin_country=JP&with_genres=16&sort_by=popularity.desc" MOVIE_SAFE, "jpmov" },
  { "Japanese series for children", "series", "/discover/tv?with_origin_country=JP&with_genres=16,10762&sort_by=popularity.desc" TV_SAFE, "jpser" },
};

static void expand(const char *q, char *out, size_t cap) {
  char lang[24], reg[8], d18[16]; place(lang, sizeof lang, reg, sizeof reg); ago(d18, sizeof d18, 540);
  size_t n = 0; for (const char *p = q; *p && n + 24 < cap; p++) {
    if (p[0] == '%' && (p[1] == 'R' || p[1] == 'D')) { n += (size_t)snprintf(out + n, cap - n, "%s", p[1] == 'R' ? reg : d18); p++; }
    else out[n++] = *p; }
  out[n] = 0; snprintf(out + n, cap - n, "&language=%s", lang);
}
static int rows(const KRow *r, int nr, CatalogDef *out, int max, const char *only) {
  int n = 0; for (int i = 0; i < nr && n < max; i++) { if (only && strcmp(r[i].type, only)) continue; char path[480]; expand(r[i].query, path, sizeof path); def(&out[n++], T(r[i].title), r[i].type, path, r[i].key); }
  return n; }

/* no TMDB key: Cinemeta's family and animation lists (genre names as Cinemeta has them) */
static int cinemeta(int kind, CatalogDef *out, int max) {
  static const struct { const char *title, *type, *genre; int kind; } C[] = {
    { "Children's films", "movie", "Family", 0 }, { "Animated films", "movie", "Animation", 0 }, { "Animated series", "series", "Animation", 0 },
    { "Children's films", "movie", "Family", 2 }, { "Animated films", "movie", "Animation", 2 }, { "Animated series", "series", "Animation", 3 }, { "Children's series", "series", "Family", 3 } };
  int n = 0; for (size_t i = 0; i < sizeof C / sizeof C[0] && n < max; i++) { if (C[i].kind != (kind == 1 || kind == 4 ? 0 : kind)) continue;
    CatalogDef *d = &out[n++]; memset(d, 0, sizeof *d); snprintf(d->title, sizeof d->title, "%s", T(C[i].title)); snprintf(d->type, sizeof d->type, "%s", C[i].type);
    snprintf(d->path, sizeof d->path, CINEMETA "/catalog/%s/top", C[i].type); snprintf(d->extra, sizeof d->extra, "genre=%s", C[i].genre);
    snprintf(d->url, sizeof d->url, "%s/%s.json", d->path, d->extra); snprintf(d->key, sizeof d->key, "kc_%d", (int)i); d->skip = 1; }
  return n; }

int kids_defs(int kind, CatalogDef *out, int max) {
  if (!tmdb_has_key()) return cinemeta(kind, out, max);
  if (kind == 2) return rows(FILMS, sizeof FILMS / sizeof FILMS[0], out, max, NULL);
  if (kind == 3) return rows(SERIES, sizeof SERIES / sizeof SERIES[0], out, max, NULL);
  if (kind == 4) return rows(ANIME, sizeof ANIME / sizeof ANIME[0], out, max, NULL);
  return rows(HOME, sizeof HOME / sizeof HOME[0], out, max, NULL);
}
int kids_disc_cats(const char *type, CatalogDef *out, int max) {
  if (!tmdb_has_key()) { CatalogDef all[8]; int n = cinemeta(0, all, 8), k = 0; for (int i = 0; i < n && k < max; i++) if (!strcmp(all[i].type, type)) out[k++] = all[i]; return k; }
  return !strcmp(type, "series") ? rows(SERIES, sizeof SERIES / sizeof SERIES[0], out, max, NULL) : rows(FILMS, sizeof FILMS / sizeof FILMS[0], out, max, "movie");
}
int kids_search_defs(const char *q, CatalogDef *out, int max) {
  if (!tmdb_has_key() || max < 2) return 0; char enc[300]; size_t n = 0;
  for (const unsigned char *p = (const unsigned char *)q; *p && n + 4 < sizeof enc; p++) { if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '-' || *p == '.') enc[n++] = (char)*p; else n += (size_t)snprintf(enc + n, sizeof enc - n, "%%%02X", *p); }
  enc[n] = 0; char lang[24], reg[8], path[480]; place(lang, sizeof lang, reg, sizeof reg);
  snprintf(path, sizeof path, "f/search/movie?query=%s&include_adult=false&language=%s", enc, lang); def(&out[0], T("Films"), "movie", path, "qm");
  snprintf(path, sizeof path, "f/search/tv?query=%s&include_adult=false&language=%s", enc, lang); def(&out[1], T("Series"), "series", path, "qs");
  out[0].skip = out[1].skip = 0; return 2;
}
void kids_feat(char *movieUrl, size_t mc, char *seriesUrl, size_t sc) {
  CatalogDef d; char path[480];
  expand(HOME[0].query, path, sizeof path); def(&d, "", "movie", path, "f1"); snprintf(movieUrl, mc, "%s", d.url);
  expand(HOME[3].query, path, sizeof path); def(&d, "", "series", path, "f2"); snprintf(seriesUrl, sc, "%s", d.url);
}

/* a search answer keeps only what is for children: Family, Animation or Kids, and none of horror, thriller, crime, war */
static int child_safe(const cJSON *r) {
  const cJSON *g = js_arr(r, "genre_ids"), *x; int ok = 0; if (cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(r, "adult"))) return 0;
  if (g) cJSON_ArrayForEach(x, g) { int v = x->valueint; if (v == 27 || v == 53 || v == 80 || v == 10752) return 0; if (v == 16 || v == 10751 || v == 10762) ok = 1; }
  return ok; }

typedef struct { char type[16]; int filter, max; void (*cb)(ItemList *, int, void *); void (*pcb)(ItemList *, int, void *); void *ud; } KReq;
static ItemList *parse(const unsigned char *body, const KReq *r, int *more) {
  cJSON *j = body ? net_json(body) : NULL; const cJSON *res = js_arr(j, "results"), *x; ItemList *l = calloc(1, sizeof *l);
  int n = res ? cJSON_GetArraySize(res) : 0; l->items = calloc(n ? (size_t)n : 1, sizeof(Item)); l->total = j ? n : -1;
  if (more) { int pg = (int)js_num(j, "page", 1), tp = (int)js_num(j, "total_pages", 1); *more = pg < tp && pg < 25; }
  if (res) cJSON_ArrayForEach(x, res) { if (l->n >= r->max) break; if (r->filter && !child_safe(x)) continue;
    int id = (int)js_num(x, "id", 0); const char *pp = js_str(x, "poster_path", ""), *bp = js_str(x, "backdrop_path", "");
    const char *nm = js_str(x, "title", js_str(x, "name", "")), *dt = js_str(x, "release_date", js_str(x, "first_air_date", ""));
    if (id <= 0 || !*nm || !*pp) continue;
    Item *it = &l->items[l->n++]; snprintf(it->id, sizeof it->id, "tmdb:%d", id); snprintf(it->type, sizeof it->type, "%s", r->type);
    snprintf(it->name, sizeof it->name, "%s", nm); snprintf(it->poster, sizeof it->poster, "https://image.tmdb.org/t/p/w342%s", pp);
    if (*bp) snprintf(it->background, sizeof it->background, "https://image.tmdb.org/t/p/w1280%s", bp);
    snprintf(it->description, sizeof it->description, "%s", js_str(x, "overview", "")); if (strlen(dt) >= 4) snprintf(it->metaLine, sizeof it->metaLine, "%.4s", dt); }
  net_json_free(j); return l; }
static void on_list(int st, const unsigned char *body, size_t len, void *ud) { (void)len; KReq *r = ud; ItemList *l = parse(st == 200 ? body : NULL, r, NULL); r->cb(l, 0, r->ud); free(r); }
static void on_page(int st, const unsigned char *body, size_t len, void *ud) { (void)len; KReq *r = ud; int more = 0; ItemList *l = st == 200 ? parse(body, r, &more) : NULL; r->pcb(l, more, r->ud); free(r); }

static const char *tmdb_path(const char *url, int *filter) { const char *p = url + sizeof PFX - 1; *filter = !strncmp(p, "f/", 2); return *filter ? p + 1 : p; }
void kids_fetch(const char *url, const char *type, int n, void (*cb)(ItemList *, int, void *), void *ud) {
  KReq *r = calloc(1, sizeof *r); snprintf(r->type, sizeof r->type, "%s", type); r->max = n > 0 ? n : 20; r->cb = cb; r->ud = ud;
  const char *p = tmdb_path(url, &r->filter);
  if (!tmdb_request(p, on_list, r)) { ItemList *l = calloc(1, sizeof *l); l->items = calloc(1, sizeof(Item)); l->total = -1; free(r); cb(l, 0, ud); }
}
void kids_page(const char *url, const char *type, int skip, void (*cb)(ItemList *, int, void *), void *ud) {
  KReq *r = calloc(1, sizeof *r); snprintf(r->type, sizeof r->type, "%s", type); r->max = 40; r->pcb = cb; r->ud = ud;
  char path[600]; const char *p = tmdb_path(url, &r->filter); snprintf(path, sizeof path, "%s&page=%d", p, skip / 20 + 1);
  if (!tmdb_request(path, on_page, r)) { free(r); cb(NULL, 0, ud); }
}

/* a TMDB title opened: its IMDb id first (the addons find streams by it), then its page */
static unsigned openGen;
static void on_ext(int st, const unsigned char *body, size_t len, void *ud) {
  (void)len; Item *it = ud; unsigned g = (unsigned)(uintptr_t)0; (void)g;
  cJSON *j = st == 200 ? net_json(body) : NULL; const char *imdb = js_str(j, "imdb_id", "");
  if (it->ep == (int)openGen && !strncmp(imdb, "tt", 2)) { DetailParams p; memset(&p, 0, sizeof p); p.it = *it; p.it.ep = 0; snprintf(p.it.id, sizeof p.it.id, "%s", imdb); app_go(SCR_DETAIL, &p, 0); }
  else if (it->ep == (int)openGen) toast(T("Nothing found."), 2000, NULL);
  net_json_free(j); free(it);
}
int kids_open(const Item *it) {
  if (strncmp(it->id, "tmdb:", 5)) return 0;
  Item *c = malloc(sizeof *c); if (!c) return 1; *c = *it; c->ep = (int)++openGen; /* (ep carries which press this is: only the last one opens) */
  char path[64]; snprintf(path, sizeof path, "/%s/%d/external_ids", !strcmp(it->type, "series") ? "tv" : "movie", atoi(it->id + 5));
  if (!tmdb_request(path, on_ext, c)) { free(c); toast(T("Nothing found."), 2000, NULL); }
  return 1;
}
