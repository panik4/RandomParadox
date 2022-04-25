# RandomParadox
A project to generate random scenarios for multiple Paradox Games.
As of now, only Hearts of Iron IV is supported.
The Hearts of Iron IV module generates a full mod on a randomly generated worldmap, with random countries, flags, national focus trees, unit templates.
![heightmap](https://github.com/pkramp/RandomParadox/blob/main/images/heightmap.jpg?raw=true)
An example of a generated heightmap. The underlying world generation uses a system of using mutliple configurable noise layers to create heightmaps.
![provinces](https://github.com/pkramp/RandomParadox/blob/main/images/provinces.jpg?raw=true)
An example of a generated province map. A province map contains unique colour combinations for every province.
![climate](https://github.com/pkramp/RandomParadox/blob/main/images/climate.jpg?raw=true)
The generated climate map, which is converted to the colormap_rgb_cityemissivemask_a.dds texture required by the game.
![rivers](https://github.com/pkramp/RandomParadox/blob/main/images/rivers.jpg?raw=true)
Generated river map. Still work in progress.
![terrain](https://github.com/pkramp/RandomParadox/blob/main/images/terrain.jpg?raw=true)
The terrain bitmap required by Hearts of Iron, with the correct colour codes. Note that the game interprets these colours to represent prettier colours in game.
![world_normal](https://github.com/pkramp/RandomParadox/blob/main/images/world_normal.jpg?raw=true)
The world normal map, generated with a Sobel Filter from the heightmap. The game uses this image to display mountains in game.

There are even more files and images being generated, which will not be displayed in the readme.

Furthermore, this tool can be used as a map tool for total conversions, generating all relevant image and text files from a given heightmap. Building upon these files allows a far quicker start for total map conversion mods.
For example, giving the map of a fantasy world as input, and tweaking parameters for latitude and longitude, both the Bitmaps and texture files required for the mod will be generated.
While the resulting climate will not be exactly like the fantasy world, the maps can be altered from there on.

Detailed documentation on its use, with visual guides, is in the RandomParadox_Documentation.pdf.
