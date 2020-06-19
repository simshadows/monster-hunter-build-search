# Sim's MHWI Build Search Tool

**CURRENTLY A WORK IN PROGRESS.** I'm developing this to aid in build optimization for the game Monster Hunter World Iceborne!

## How do I use this?

I'll write a detailed guide on how to use this thing later.

## Issues

### General

Major:

- I don't know the order of operations for Free Element. For now, I take the element/status value of the weapon with augments/customs/Safi included, and I multiply it by the Free Element multiplier.

Minor:

- Will I need to change to different string types to properly use unicode?
- There's a lot of gnarly code in here, though I tried to keep the issues as localized as possible, such as having all relevant issues be localized into a single `.cpp` file. Particularly problematic areas though include:
    - `core/weapon_augments.cpp`: So many case statements, magic numbers, unsafe state validation, and a generally CPU-inefficient way of doing things.
- I will need to figure out a better way to handle weird cases where improving skills will actually reduce EFR. For now, the solution is to have the option to select skills that will be totally prevented from being added to a build.

### Missing Data

- Only greatswords have been added to the database (with a few minor additions from other weapon classes, for debugging purposes).
- All Master Rank armour up until (and including) Guildwork has been added. Anything beyond that is a bit patchy (though I do try to add all of the important pieces of armour).
- In the interest of simplicity, full armour sets will NOT be added to the database for now. At the time of writing, these are:
    - Geralt Alpha
    - Ciri Alpha
    - Leon Alpha+
    - Claire Alpha+
- In the interest of validity, I will only add things that appear in the PC version of Monster Hunter World. All console exclusives will be left out (at least until I can figure out a good way to allow the user to filter things out).
- Other notable sets that should hopefully be added in the near future:
    - Acrobat Earrings
    - Showman Earrings

### Specific plans for the future:

- Elderseal filtering (similar to the health regen filtering)
- The specification of a sequence of attacks (i.e. a *combo*) rather than just a single attack. (You can technically just add the motion values and elemental/status modifiers for now, but there will be no facility to correct for rounding if exact values is ever desired.)

## Data and Educational References

- Directly from in-game UI or minor first-hand in-game testing where possible/needed
- MHWI Damage Formula ([link for v13.50.00](https://docs.google.com/spreadsheets/d/e/2PACX-1vSuFIUfe5Sp9k6sqKvPSPbO2xcClt1WaYMf_xGocWDIkgaSDi0nBLbOKAd8GScLQRjfxbljhPO4Bjf7/pubhtml#))
- MHWI General Data Sheet ([link for v13.50.00](https://docs.google.com/spreadsheets/d/e/2PACX-1vQ5HFkHnEP74gD-SCVta9syb9GaF1_nSmMFgV4hxvZt9iu4HmfhGlP2KbnIbC-cAx5kkvsd8L7oB0Uy/pubhtml#))
- MHWI Weapon Attack Tables ([link for v13.50.00](https://docs.google.com/spreadsheets/d/e/2PACX-1vTEYb4wGpijtIpFVopiYl1V83m48d7g1AHmTwOBKJ5RXdlz1sfxCyEmnhbgHLWQsGiXnodyBsUlPzc3/pubhtml#))
- Honey Hunter World MHWI Builder ([link](https://honeyhunterworld.com/mhwbi/))
- Kiranico ([link](https://mhworld.kiranico.com/))
- JinJinx and Tuna ([link](https://www.youtube.com/channel/UCEU2FbTBYxAETGH4sqqzhPA))

## License

All original source code is licensed under the [*GNU General Public License v3.0*](https://www.gnu.org/licenses/gpl-3.0.en.html). (A copy of the license is attached under `./LICENSE`.)

All original non-source code material (such as everything in `./docs` and its entire tree of subdirectories) are licensed under [*Attribution-ShareAlike 4.0 International*](http://creativecommons.org/licenses/by-sa/4.0/). (A copy of the license is attached under `./docs/LICENSE`.)

All included dependencies (in `./dependencies`) have their own separate licenses.

