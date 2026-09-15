# Colors - colorlib

`colorlib.h` defines the sixteen VIC-II palette entries as named constants. Every function in the engine that takes a `color` parameter expects one of these indices (or any value 0-15).

```c
#include "multidore64/colorlib.h"
```

## The palette

| Index | Constant | Sample |
|---:|---|---|
| 0 | `color_black` | ◆0 |
| 1 | `color_white` | ◆1 |
| 2 | `color_red` | ◆2 |
| 3 | `color_cyan` | ◆3 |
| 4 | `color_purple` | ◆4 |
| 5 | `color_green` | ◆5 |
| 6 | `color_blue` | ◆6 |
| 7 | `color_yellow` | ◆7 |
| 8 | `color_orange` | ◆8 |
| 9 | `color_brown` | ◆9 |
| 10 | `color_light_red` | ◆10 |
| 11 | `color_dark_grey` | ◆11 |
| 12 | `color_grey` | ◆12 |
| 13 | `color_light_green` | ◆13 |
| 14 | `color_light_blue` | ◆14 |
| 15 | `color_light_grey` | ◆15 |

The swatches above are rendered with the authentic VIC-II colors by the documentation theme (see `docs/assets/style.css`, `.swatch-*`).

## Usage

```c
renderlib_setbg(color_blue);
renderlib_setborder(color_black);
renderlib_drawstring(0, 0, color_yellow, "GOLD!");
renderlib_fillcircle(160, 100, 30, color_red);
```

## Notes

- The constants are plain `0x00`-`0x0F` values - passing them to `$D020/$D021` directly is fine.
- In multicolor modes only three colors plus background are visible per cell; see any VIC-II reference for the exact rules.
- `renderlib_setpalette(index, color)` reprograms one hardware color register, enabling palette-cycling effects.
