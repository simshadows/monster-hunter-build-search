# Sim's MHWI Build Search Tool

**CURRENTLY A WORK IN PROGRESS.** I'm developing this to aid in build optimization for the game Monster Hunter World Iceborne!

As of the time of writing, a significantly more functional version is available at <https://github.com/simshadows/mhwi-build-search-prototype>. However, the goal is to bring everything into this C++ implementation, and continue work from here!

## How do I use this?

This doesn't really have a user interface yet as I'm merely assertion-testing to build the basic components for now.

I'll write a detailed guide on how to use this thing later.

## Issues

- There is a separation between the skills data in the database JSON file `data/database_skills.json` and the implemented skill calculations done by `src/skill_contributions.cpp`. If the JSON data changes, the implemented skill calculations can break and cause array overflows. Please figure out a solution to this!
- Will I need to change to different string types to properly use unicode?
- There's a lot of gnarly code in here, though I tried to keep the issues as localized as possible, such as having all relevant issues be localized into a single `.cpp` file. Particularly problematic areas though include:
    - `core/weapon_augments.cpp`: So many case statements, magic numbers, unsafe state validation, and a generally CPU-inefficient way of doing things.

## License

```
MIT License

Copyright (c) 2020 simshadows

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

