# Flag resources

Resources are eagerly loaded once by ArdaGen through Flag::loadResources.
Failed initialization publishes nothing and can be retried. Subsequent successful
initialization calls reuse the collection; restarting is required to reload files.

Backgrounds are numbered 0�99, with gaps allowed, and must be 82x52 TGA images.
Symbols use the same numbering scheme and must be 52x52 TGA images.
Each image requires a matching .txt file containing one non-comment data line.

Background metadata has six semicolon-separated fields, with an optional trailing
semicolon:

RGB=palette|RGB=palette;symbolPalette;applySymbol;xOffset;yOffset;scale;

Example:
0,0,0=p1|255,255,255=p2;s1;true;0.35;0.25;0.5;

Every source RGB colour, including transparent pixels, needs an explicit mapping.
RGB channels are integers 0�255. Palette names refer to colour_groups.txt.
Repeated palette names still select independent replacement colours for each RGB
region. The original ordered format is no longer accepted. The shipped migration
retains each region's former effective palette, including last-palette fallback
assignments; it does not redesign the templates.

Offsets are fractions of flag width/height, in [0,1]. Scale is a fraction of the
52-pixel symbol size, in [1/52,1]. Symbols are clipped at the flag boundary.
Booleans must be true or false. Backgrounds are made opaque; symbol alpha is
composited over the background, including partially transparent edges.

Symbol metadata is replaceColour;allowedIdeologies;, for example true;democratic,neutrality;.
Names are none, fascism, democratic, communism, neutrality (case-sensitive).
The comma-separated list is required and nonempty; whitespace is trimmed and duplicates ignored.
none means the literal NONE ideology, not a wildcard. Only matching symbols are selected.
If none match, the background is kept without a symbol.
The first field true recolours all visible pixels using
the background's symbol palette; false preserves original RGB.
The legacy flag_types.txt is retained for reference and is not loaded.

All supplied symbols initially allow all five ideologies. Restrict individual lists
in their sidecars as desired and restart to reload resources. Flag generation requires
an explicit Arda::Utils::Ideology argument; changing a country ideology regenerates
the complete flag during imports, HOI4 politics, and ruling-party editor changes.
