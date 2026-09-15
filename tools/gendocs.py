#!/usr/bin/env python3
"""MultiDore 64 documentation site generator.

Converts docs/*.md into a static, dependency-free HTML site in docs/site/.

Usage:
    python3 tools/gendocs.py            # build the site
    python3 tools/gendocs.py --serve    # build, then serve on http://localhost:8000

Requires: python3, `markdown` (pip install markdown), `pygments` (optional,
enables code highlighting).
"""

import html
import http.server
import json
import os
import re
import shutil
import sys

import markdown

HERE = os.path.dirname(os.path.abspath(__file__))
DOCS = os.path.normpath(os.path.join(HERE, "..", "docs"))
SITE = os.path.join(DOCS, "site")

try:
    from pygments.formatters import HtmlFormatter
    HAVE_PYGMENTS = True
except ImportError:
    HAVE_PYGMENTS = False

# Sidebar order and grouping: (section title, [(slug, label), ...])
NAV = [
    ("Overview", [
        ("index", "Introduction"),
        ("getting-started", "Getting started"),
        ("build", "Building"),
    ]),
    ("Modules", [
        ("renderlib", "Rendering"),
        ("soundlib", "Audio"),
        ("controllerlib", "Input"),
        ("render3d", "3D rendering"),
        ("netlib", "Networking"),
        ("colorlib", "Colors"),
        ("utilslib", "Utilities"),
    ]),
    ("Reference", [
        ("memory", "Memory map"),
        ("examples", "Examples"),
    ]),
]

CSS = """/* MultiDore 64 docs theme */
:root {
  --bg: #0d1117;
  --bg-panel: #151b23;
  --bg-code: #0a0e14;
  --border: #232b36;
  --text: #c9d1d9;
  --text-dim: #8b949e;
  --accent: #7c6cff;
  --accent-2: #40c4ff;
  --accent-soft: rgba(124, 108, 255, 0.14);
  --mono: ui-monospace, "SFMono-Regular", "JetBrains Mono", Menlo, Consolas, monospace;
  --sans: -apple-system, "Segoe UI", Roboto, Inter, sans-serif;
}
* { box-sizing: border-box; }
html { scroll-behavior: smooth; }
body {
  margin: 0; background: var(--bg); color: var(--text);
  font-family: var(--sans); font-size: 16px; line-height: 1.65;
}

/* layout */
.wrap { display: flex; min-height: 100vh; }
.sidebar {
  width: 264px; flex: 0 0 auto; position: sticky; top: 0;
  height: 100vh; overflow-y: auto; padding: 22px 18px 40px;
  background: var(--bg-panel); border-right: 1px solid var(--border);
}
.main { flex: 1; min-width: 0; }
.content { max-width: 860px; margin: 0 auto; padding: 44px 36px 90px; }

/* brand */
.brand { display: flex; align-items: center; gap: 10px; margin-bottom: 6px; }
.brand .logo {
  width: 30px; height: 30px; border-radius: 8px; flex: 0 0 auto;
  background: linear-gradient(135deg, var(--accent), var(--accent-2));
  display: grid; place-items: center; font-weight: 800; color: #fff; font-size: 13px;
}
.brand a { color: var(--text); text-decoration: none; font-weight: 700; letter-spacing: 0.2px; }
.tagline { color: var(--text-dim); font-size: 12.5px; margin: 0 0 18px; }

/* search */
.search { position: relative; margin: 14px 0 20px; }
.search input {
  width: 100%; padding: 8px 12px; border-radius: 8px; font-size: 13.5px;
  background: var(--bg); border: 1px solid var(--border); color: var(--text);
}
.search input:focus { outline: none; border-color: var(--accent); box-shadow: 0 0 0 3px var(--accent-soft); }

/* nav */
.nav section { margin-bottom: 20px; }
.nav h4 {
  margin: 0 0 6px; font-size: 11px; text-transform: uppercase;
  letter-spacing: 1.2px; color: var(--text-dim);
}
.nav a {
  display: block; padding: 5px 10px; margin: 1px 0; border-radius: 7px;
  color: var(--text-dim); text-decoration: none; font-size: 14px;
  border-left: 2px solid transparent;
}
.nav a:hover { color: var(--text); background: var(--accent-soft); }
.nav a.active {
  color: var(--text); background: var(--accent-soft);
  border-left-color: var(--accent); font-weight: 600;
}
.nav a.hidden { display: none; }
.nav section.hidden { display: none; }

/* content typography */
.content h1 {
  font-size: 2em; line-height: 1.2; margin: 0 0 6px;
  letter-spacing: -0.5px;
}
.content h1 + p { color: var(--text-dim); font-size: 1.06em; margin-top: 4px; }
.content h2 {
  font-size: 1.35em; margin: 40px 0 10px; padding-top: 18px;
  border-top: 1px solid var(--border);
}
.content h3 { font-size: 1.1em; margin: 26px 0 8px; }
.content p { margin: 10px 0; }
.content a { color: var(--accent-2); text-decoration: none; }
.content a:hover { text-decoration: underline; }
.content ul, .content ol { padding-left: 26px; }
.content li { margin: 4px 0; }
.content hr { border: 0; border-top: 1px solid var(--border); margin: 30px 0; }

/* inline code */
.content code {
  font-family: var(--mono); font-size: 0.87em;
  background: var(--accent-soft); color: #dcd6ff;
  padding: 1.5px 5px; border-radius: 5px;
}

/* code blocks */
.codeblock { position: relative; margin: 14px 0; }
.codeblock pre {
  margin: 0; padding: 14px 16px; overflow-x: auto;
  background: var(--bg-code); border: 1px solid var(--border); border-radius: 10px;
}
.codeblock pre code {
  background: none; padding: 0; font-size: 13px; line-height: 1.55; color: var(--text);
}
.copy-btn {
  position: absolute; top: 8px; right: 8px; padding: 3px 9px;
  font-size: 11px; font-family: var(--sans); color: var(--text-dim);
  background: var(--bg-panel); border: 1px solid var(--border);
  border-radius: 6px; cursor: pointer; opacity: 0; transition: opacity .15s;
}
.codeblock:hover .copy-btn { opacity: 1; }
.copy-btn:hover { color: var(--text); border-color: var(--accent); }
.copy-btn.copied { color: #4ade80; }

/* tables */
.content table {
  border-collapse: collapse; width: 100%; margin: 14px 0; font-size: 14.5px;
}
.content th, .content td {
  border: 1px solid var(--border); padding: 7px 12px; text-align: left;
}
.content th { background: var(--bg-panel); font-weight: 600; }
.content tr:nth-child(even) td { background: rgba(255,255,255,0.015); }

/* blockquotes + admonitions */
.content blockquote {
  margin: 14px 0; padding: 8px 16px; border-left: 3px solid var(--accent);
  background: var(--accent-soft); border-radius: 0 8px 8px 0; color: var(--text);
}
.admonition {
  margin: 16px 0; padding: 12px 16px; border-radius: 10px;
  border: 1px solid var(--border); background: var(--bg-panel);
  border-left: 3px solid var(--accent);
}
.admonition .admonition-title {
  font-weight: 700; font-size: 13px; text-transform: uppercase;
  letter-spacing: 0.8px; margin-bottom: 4px; color: var(--accent-2);
}
.admonition.warning { border-left-color: #f0b429; }
.admonition.warning .admonition-title { color: #f0b429; }
.admonition.danger { border-left-color: #f87171; }
.admonition.danger .admonition-title { color: #f87171; }
.admonition.tip { border-left-color: #4ade80; }
.admonition.tip .admonition-title { color: #4ade80; }
.admonition p { margin: 4px 0; }

/* footer */
.footer {
  margin-top: 60px; padding-top: 18px; border-top: 1px solid var(--border);
  color: var(--text-dim); font-size: 13px; display: flex; justify-content: space-between;
}
.footer a { color: var(--text-dim); }

/* prev/next */
.pager { display: flex; gap: 12px; margin-top: 48px; }
.pager a {
  flex: 1; padding: 12px 16px; border: 1px solid var(--border);
  border-radius: 10px; text-decoration: none; color: var(--text);
  background: var(--bg-panel);
}
.pager a:hover { border-color: var(--accent); }
.pager .dir { display: block; font-size: 11px; color: var(--text-dim); text-transform: uppercase; letter-spacing: 1px; }
.pager .next { text-align: right; }

/* palette swatches (colorlib page) */
.swatch {
  display: inline-block; width: 14px; height: 14px; border-radius: 3px;
  vertical-align: middle; margin-right: 2px; border: 1px solid rgba(255,255,255,0.2);
}
.swatch-0 { background:#000; } .swatch-1 { background:#fff; }
.swatch-2 { background:#883932; } .swatch-3 { background:#67b6bd; }
.swatch-4 { background:#8b3f96; } .swatch-5 { background:#55a049; }
.swatch-6 { background:#40318d; } .swatch-7 { background:#bfce72; }
.swatch-8 { background:#8b5425; } .swatch-9 { background:#574200; }
.swatch-10 { background:#b86962; } .swatch-11 { background:#505050; }
.swatch-12 { background:#787878; } .swatch-13 { background:#94e089; }
.swatch-14 { background:#7869c4; } .swatch-15 { background:#9f9f9f; }

/* pygments (github-dark-ish token colors) */
.highlight { background: none !important; }
.highlight .k, .highlight .kn, .highlight .kr { color: #ff7b72; }
.highlight .nf, .highlight .nc { color: #d2a8ff; }
.highlight .s, .highlight .sa, .highlight .sc, .highlight .se { color: #a5d6ff; }
.highlight .c, .highlight .cm, .highlight .c1, .highlight .cs { color: #6e7681; font-style: italic; }
.highlight .mi, .highlight .mh, .highlight .mf { color: #79c0ff; }
.highlight .kt { color: #ffa657; }
.highlight .nb { color: #79c0ff; }
.highlight .o { color: #ff7b72; }
.highlight .n { color: #c9d1d9; }
.highlight .err { background: none; color: inherit; }
.highlight .w { color: inherit; }
.highlight pre { margin: 0; }
.highlight code { color: var(--text); }

/* responsive */
.burger { display: none; }
@media (max-width: 900px) {
  .sidebar {
    position: fixed; left: 0; top: 0; z-index: 50; transform: translateX(-100%);
    transition: transform .2s ease;
  }
  .sidebar.open { transform: none; }
  .burger {
    display: block; position: fixed; z-index: 60; top: 12px; left: 12px;
  }
  .content { padding: 62px 20px 70px; }
}
/* pygments (github-dark-ish token colors) */
.codeblock .k, .codeblock .kn, .codeblock .kr { color: #ff7b72; }
.codeblock .nf, .codeblock .nc { color: #d2a8ff; }
.codeblock .s, .codeblock .sa, .codeblock .sc, .codeblock .se { color: #a5d6ff; }
.codeblock .c, .codeblock .cm, .codeblock .c1, .codeblock .cs { color: #6e7681; font-style: italic; }
.codeblock .mi, .codeblock .mh, .codeblock .mf { color: #79c0ff; }
.codeblock .kt { color: #ffa657; }
.codeblock .nb { color: #79c0ff; }
.codeblock .o { color: #ff7b72; }
.codeblock .n { color: #c9d1d9; }
.codeblock .err { background: none; color: inherit; }
.codeblock .w { color: inherit; }
"""

JS = """// MultiDore 64 docs: search filter, copy buttons, mobile nav
(function () {
  var pages = window.__DOC_PAGES__ || [];


  // search: filter sidebar entries, jump on Enter to best match
  var input = document.getElementById('doc-search');
  if (input) {
    input.addEventListener('input', function () {
      var q = this.value.toLowerCase();
      document.querySelectorAll('.nav a[data-slug]').forEach(function (a) {
        var slug = a.getAttribute('data-slug');
        var hit = !q || slug.indexOf(q) !== -1 ||
                  (a.textContent || '').toLowerCase().indexOf(q) !== -1 ||
                  pages.some(function (p) {
                    return p.slug === slug && p.text.indexOf(q) !== -1;
                  });
        a.classList.toggle('hidden', !hit);
      });
      document.querySelectorAll('.nav section').forEach(function (s) {
        var any = s.querySelectorAll('a[data-slug]:not(.hidden)').length > 0;
        s.classList.toggle('hidden', !any);
      });
    });
    input.addEventListener('keydown', function (e) {
      if (e.key !== 'Enter') return;
      var q = this.value.toLowerCase();
      var best = pages.find(function (p) { return p.slug.indexOf(q) !== -1; }) ||
                 pages.find(function (p) { return p.text.indexOf(q) !== -1; });
      if (best) window.location = best.slug + '.html';
    });
  }

  // copy buttons
  document.querySelectorAll('.codeblock').forEach(function (block) {
    var btn = block.querySelector('.copy-btn');
    var code = block.querySelector('pre code');
    if (!btn || !code) return;
    btn.addEventListener('click', function () {
      var text = code.innerText;
      var done = function () {
        btn.textContent = 'copied'; btn.classList.add('copied');
        setTimeout(function () {
          btn.textContent = 'copy'; btn.classList.remove('copied');
        }, 1200);
      };
      if (navigator.clipboard && navigator.clipboard.writeText)
        navigator.clipboard.writeText(text).then(done, done);
      else {
        var ta = document.createElement('textarea');
        ta.value = text; document.body.appendChild(ta); ta.select();
        try { document.execCommand('copy'); } catch (e) {}
        document.body.removeChild(ta); done();
      }
    });
  });

  // mobile sidebar
  var burger = document.querySelector('.burger');
  var sidebar = document.querySelector('.sidebar');
  if (burger && sidebar)
    burger.addEventListener('click', function () { sidebar.classList.toggle('open'); });
})();
"""

PAGE_TMPL = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{title} · MultiDore 64</title>
<meta name="description" content="{description}">
<style>
{css}
</style>
</head>
<body>
<button class="burger" aria-label="Toggle navigation">&#9776;</button>
<div class="wrap">
<nav class="sidebar">
  <div class="brand"><span class="logo">M64</span><a href="index.html">MultiDore 64</a></div>
  <p class="tagline">Game engine for the Commodore 64</p>
  <div class="search"><input id="doc-search" type="search" placeholder="Search docs&hellip;" autocomplete="off"></div>
  <div class="nav">
{nav}
  </div>
</nav>
<div class="main"><div class="content">
{body}
<div class="pager">{prev}{next}</div>
<div class="footer">
  <span>&copy; 2023-2026 Malte0621 &middot; built with <code>tools/gendocs.py</code></span>
  <a href="https://github.com/drmortalwombat/oscar64">oscar64</a>
</div>
</div></div>
</div>
<script>window.__DOC_PAGES__ = {pages_json};</script>
<script>
{js}
</script>
</body>
</html>
"""


def slug_of(path):
    return os.path.splitext(os.path.basename(path))[0]


def md_to_html(text):
    """Markdown -> HTML with admonitions, tables, fenced code (+pygments)."""
    exts = ["fenced_code", "tables", "admonition", "attr_list", "sane_lists"]
    ext_cfg = {}

    if HAVE_PYGMENTS:
        exts.append("codehilite")
        ext_cfg["codehilite"] = {"guess_lang": False, "pygments_style": "default"}

    md = markdown.Markdown(extensions=exts, extension_configs=ext_cfg)
    body = md.convert(text)

    # normalize codehilite output into our .codeblock wrapper with a copy button
    body = body.replace(
        '<div class="codehilite"><pre>',
        '<div class="codeblock"><button class="copy-btn">copy</button><pre>'
    )

    # wrap any plain <pre><code> (unhighlighted) the same way
    def wrap_plain(m):
        return ('<div class="codeblock"><button class="copy-btn">copy</button>'
                f'<pre><code>{m.group(1)}</code></pre></div>')

    body = re.sub(r'<pre><code>(.*?)</code></pre>', wrap_plain, body, flags=re.S)
    return body


def add_swatches(html_text, slug):
    """Replace swatch placeholders on the colors page with colored chips."""
    if slug != "colorlib":
        return html_text

    def repl(m):
        return f'<span class="swatch swatch-{int(m.group(1))}"></span>'

    return re.sub(r"\u25c6(\d+)", repl, html_text)


def extract_title(text, slug):
    m = re.search(r"^#\s+(.+)$", text, re.M)
    return m.group(1).strip() if m else slug.replace("-", " ").title()


def first_paragraph(text):
    for line in text.splitlines():
        line = line.strip()
        if line and not line.startswith(("#", "|", "```", "!", "-", "*")):
            return re.sub(r"[*`_\[]", "", line)[:180]
    return ""


def strip_markup(text):
    text = re.sub(r"```.*?```", " ", text, flags=re.S)
    text = re.sub(r"[#>*`|\-_\[\]()!]", " ", text)
    return re.sub(r"\s+", " ", text).lower()


def render_nav(pages, current):
    rows = []
    listed = set()
    for section, items in NAV:
        rows.append(f"  <section><h4>{html.escape(section)}</h4>")
        for slug, label in items:
            if slug not in pages:
                print(f"warning: nav item '{slug}' has no docs/{slug}.md",
                      file=sys.stderr)
                continue
            listed.add(slug)
            cls = ' class="active"' if slug == current else ""
            rows.append(
                f'  <a{cls} data-slug="{slug}" href="{slug}.html">'
                f"{html.escape(label)}</a>"
            )
        rows.append("  </section>")
    extra = [s for s in pages if s not in listed]
    if extra:
        rows.append("  <section><h4>More</h4>")
        for slug in extra:
            cls = ' class="active"' if slug == current else ""
            rows.append(
                f'  <a{cls} data-slug="{slug}" href="{slug}.html">'
                f'{html.escape(pages[slug]["title"])}</a>'
            )
        rows.append("  </section>")
    return "\n".join(rows)


def build():
    if not os.path.isdir(DOCS):
        sys.exit(f"docs directory not found: {DOCS}")

    md_files = sorted(f for f in os.listdir(DOCS) if f.endswith(".md"))
    if not md_files:
        sys.exit("no markdown files found in docs/")

    shutil.rmtree(SITE, ignore_errors=True)
    os.makedirs(os.path.join(SITE, "assets"))

    pages = {}
    for f in md_files:
        slug = slug_of(f)
        raw = open(os.path.join(DOCS, f), encoding="utf-8").read()
        pages[slug] = {
            "slug": slug,
            "title": extract_title(raw, slug),
            "desc": first_paragraph(raw),
            "text": strip_markup(raw),
            "html": add_swatches(md_to_html(raw), slug),
        }

    order = [slug for _, items in NAV for slug, _ in items if slug in pages]
    order += [s for s in pages if s not in order]
    pages_json = json.dumps([{"slug": s, "text": pages[s]["text"]} for s in order])

    for slug in order:
        p = pages[slug]
        pos = order.index(slug)
        prev = order[pos - 1] if pos > 0 else None
        nxt = order[pos + 1] if pos + 1 < len(order) else None

        prev_html = ""
        if prev:
            prev_html = (
                f'<a class="prev" href="{prev}.html">'
                f'<span class="dir">&larr; previous</span>'
                f'{html.escape(pages[prev]["title"])}</a>'
            )
        next_html = ""
        if nxt:
            next_html = (
                f'<a class="next" href="{nxt}.html">'
                f'<span class="dir">next &rarr;</span>'
                f'{html.escape(pages[nxt]["title"])}</a>'
            )

        page = PAGE_TMPL.format(
            title=html.escape(p["title"]),
            description=html.escape(p["desc"]),
            nav=render_nav(pages, slug),
            body=p["html"],
            pages_json=pages_json,
            prev=prev_html,
            next=next_html,
            css=CSS,
            js=JS,
        )
        open(os.path.join(SITE, f"{slug}.html"), "w", encoding="utf-8").write(page)

    open(os.path.join(SITE, "assets", "style.css"), "w").write(CSS)
    open(os.path.join(SITE, "assets", "app.js"), "w").write(JS)

    print(f"built {len(order)} pages -> {SITE}")
    for slug in order:
        print(f"  {slug}.html")


def serve(port=8000):
    os.chdir(SITE)
    print(f"serving {SITE} at http://localhost:{port}")
    http.server.HTTPServer(
        ("127.0.0.1", port), http.server.SimpleHTTPRequestHandler
    ).serve_forever()


if __name__ == "__main__":
    if "--serve" in sys.argv:
        build()
        serve()
    else:
        build()
