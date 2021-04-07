
/*
 * This file is auto-generated.
 * Do not edit directly!
 */

#include <unordered_map>

#include "../database_miscbuffs.h"

namespace MiscBuffsDatabase
{


static const std::unordered_map<std::string, MiscBuff> g_miscbuffs_map = {
    {
        "FOOD_ATTACK_UP_S",
        {
            "FOOD_ATTACK_UP_S", // id
            "Food: Attack Up (S)", // name
            5, // added_raw
            1.0, // base_raw_multiplier
            {
                "ATTACK_FOOD"
            }
        }
    },
    {
        "FOOD_ATTACK_UP_M",
        {
            "FOOD_ATTACK_UP_M", // id
            "Food: Attack Up (M)", // name
            10, // added_raw
            1.0, // base_raw_multiplier
            {
                "ATTACK_FOOD"
            }
        }
    },
    {
        "FOOD_ATTACK_UP_L",
        {
            "FOOD_ATTACK_UP_L", // id
            "Food: Attack Up (L)", // name
            15, // added_raw
            1.0, // base_raw_multiplier
            {
                "ATTACK_FOOD"
            }
        }
    },
    {
        "MELODY_ATTACK_UP_S",
        {
            "MELODY_ATTACK_UP_S", // id
            "Melody: Attack Up (S)", // name
            0, // added_raw
            1.1, // base_raw_multiplier
            {
                "ATTACK_MELODY"
            }
        }
    },
    {
        "MELODY_ATTACK_UP_L",
        {
            "MELODY_ATTACK_UP_L", // id
            "Melody: Attack Up (L)", // name
            0, // added_raw
            1.15, // base_raw_multiplier
            {
                "ATTACK_MELODY"
            }
        }
    },
    {
        "MELODY_ATTACK_UP_XL",
        {
            "MELODY_ATTACK_UP_XL", // id
            "Melody: Attack Up (XL)", // name
            0, // added_raw
            1.2, // base_raw_multiplier
            {
                "ATTACK_MELODY"
            }
        }
    },
    {
        "POWERCHARM",
        {
            "POWERCHARM", // id
            "Powercharm", // name
            6, // added_raw
            1.0, // base_raw_multiplier
            {

            }
        }
    },
    {
        "POWERTALON",
        {
            "POWERTALON", // id
            "Powertalon", // name
            9, // added_raw
            1.0, // base_raw_multiplier
            {

            }
        }
    },
    {
        "DEMONDRUG",
        {
            "DEMONDRUG", // id
            "Demondrug", // name
            5, // added_raw
            1.0, // base_raw_multiplier
            {
                "DEMONDRUG"
            }
        }
    },
    {
        "MEGA_DEMONDRUG",
        {
            "MEGA_DEMONDRUG", // id
            "Mega Demondrug", // name
            7, // added_raw
            1.0, // base_raw_multiplier
            {
                "DEMONDRUG"
            }
        }
    },
    {
        "MIGHT_SEED",
        {
            "MIGHT_SEED", // id
            "Might Seed", // name
            10, // added_raw
            1.0, // base_raw_multiplier
            {
                "MIGHT_SEED"
            }
        }
    },
    {
        "DEMON_POWDER",
        {
            "DEMON_POWDER", // id
            "Demon Powder", // name
            10, // added_raw
            1.0, // base_raw_multiplier
            {

            }
        }
    },
    {
        "MIGHT_PILL",
        {
            "MIGHT_PILL", // id
            "Might Pill", // name
            25, // added_raw
            1.0, // base_raw_multiplier
            {
                "MIGHT_SEED"
            }
        }
    },
};


const MiscBuff& get_miscbuff(const std::string& miscbuff_id) noexcept {
    return g_miscbuffs_map.at(miscbuff_id);
}


} // namespace
