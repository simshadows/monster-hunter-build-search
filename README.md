# Sim's MHWI Build Search Tool

**CURRENTLY A WORK IN PROGRESS.** I'm developing this to aid in build optimization for the game Monster Hunter World Iceborne!

## How do I use this?

I'll write a detailed guide on how to use this thing later.

## Issues

- Will I need to change to different string types to properly use unicode?
- There's a lot of gnarly code in here, though I tried to keep the issues as localized as possible, such as having all relevant issues be localized into a single `.cpp` file. Particularly problematic areas though include:
    - `core/weapon_augments.cpp`: So many case statements, magic numbers, unsafe state validation, and a generally CPU-inefficient way of doing things.
- I will need to figure out a better way to handle weird cases where improving skills will actually reduce EFR. For now, the solution is to have the option to select skills that will be totally prevented from being added to a build.

## Data References

- Directly from in-game UI or minor first-hand in-game testing where possible/needed
- MHWI Damage Formula ([link for v13.50.00](https://docs.google.com/spreadsheets/d/e/2PACX-1vSuFIUfe5Sp9k6sqKvPSPbO2xcClt1WaYMf_xGocWDIkgaSDi0nBLbOKAd8GScLQRjfxbljhPO4Bjf7/pubhtml#))
- MHWI General Data Sheet ([link for v13.50.00](https://docs.google.com/spreadsheets/d/e/2PACX-1vQ5HFkHnEP74gD-SCVta9syb9GaF1_nSmMFgV4hxvZt9iu4HmfhGlP2KbnIbC-cAx5kkvsd8L7oB0Uy/pubhtml#))
- MHWI Weapon Attack Tables ([link for v13.50.00](https://docs.google.com/spreadsheets/d/e/2PACX-1vTEYb4wGpijtIpFVopiYl1V83m48d7g1AHmTwOBKJ5RXdlz1sfxCyEmnhbgHLWQsGiXnodyBsUlPzc3/pubhtml#))
- Honey Hunter World MHWI Builder ([link](https://honeyhunterworld.com/mhwbi/))
- Kiranico ([link](https://mhworld.kiranico.com/))

