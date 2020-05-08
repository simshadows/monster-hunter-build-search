/*
 * File: database_armour.cpp
 * Author: <contact@simshadows.com>
 */

#include <assert.h>
#include <fstream>

#include "../../dependencies/json-3-7-3/json.hpp"

#include "database.h"


namespace Database {


static constexpr std::size_t k_MAX_DECO_SLOTS_PER_PIECE = 3;


// There must be exactly this many postfixes in an armour set naming scheme.
static constexpr std::size_t k_NUM_POSSIBLE_SLOTS = 5;
static std::size_t armour_slot_to_naming_scheme_index(ArmourSlot x) {
    switch (x) {
        case ArmourSlot::head:  return 0;
        case ArmourSlot::chest: return 1;
        case ArmourSlot::arms:  return 2;
        case ArmourSlot::waist: return 3;
        case ArmourSlot::legs:  return 4;
        default:
            throw std::runtime_error("Invalid armour slot.");
    }
}

// Postfixes in an armour set naming scheme that are intended to be unused must be empty strings.
// This will be used for validation, in case an armour set accidentally attempts to use an
// invalidated postfix.
//static constexpr char k_INVALIDATED_POSTFIX[] = "";


static const std::array<std::pair<std::string, Tier>, 7> variant_keys = {
    std::pair<std::string, Tier>("LR"           , Tier::low_rank   ), // idk how to make a
    std::pair<std::string, Tier>("HR_ALPHA"     , Tier::high_rank  ), // less-verbose
    std::pair<std::string, Tier>("HR_BETA"      , Tier::high_rank  ), // initialization
    std::pair<std::string, Tier>("HR_GAMMA"     , Tier::high_rank  ), // list.
    std::pair<std::string, Tier>("MR_ALPHA_PLUS", Tier::master_rank),
    std::pair<std::string, Tier>("MR_BETA_PLUS" , Tier::master_rank),
    std::pair<std::string, Tier>("MR_GAMMA_PLUS", Tier::master_rank),
};


static const std::unordered_map<std::string, ArmourSlot> upper_case_to_as_map = {
    {"HEAD" , ArmourSlot::head },
    {"CHEST", ArmourSlot::chest},
    {"ARMS" , ArmourSlot::arms },
    {"WAIST", ArmourSlot::waist},
    {"LEGS" , ArmourSlot::legs },
};


static const std::unordered_map<std::string, ArmourVariant> upper_snake_case_to_av_map = {
    {"LR"           , ArmourVariant::low_rank              },
    {"HR_ALPHA"     , ArmourVariant::high_rank_alpha       },
    {"HR_BETA"      , ArmourVariant::high_rank_beta        },
    {"HR_GAMMA"     , ArmourVariant::high_rank_gamma       },
    {"MR_ALPHA_PLUS", ArmourVariant::master_rank_alpha_plus},
    {"MR_BETA_PLUS" , ArmourVariant::master_rank_beta_plus },
    {"MR_GAMMA_PLUS", ArmourVariant::master_rank_gamma_plus},
};


static const std::unordered_map<std::string, Tier> upper_snake_case_to_tier_map = {
    {"LOW_RANK"   , Tier::low_rank   },
    {"HIGH_RANK"  , Tier::high_rank  },
    {"MASTER_RANK", Tier::master_rank},
};


ArmourSlot upper_case_to_armour_slot(const std::string& s) {
    return upper_case_to_as_map.at(s);
}


ArmourVariant upper_snake_case_to_armour_variant(const std::string& s) {
    return upper_snake_case_to_av_map.at(s);
}


Tier upper_snake_case_to_tier(const std::string& s) {
    return upper_snake_case_to_tier_map.at(s);
}


ArmourPiece::ArmourPiece(ArmourSlot                  new_slot,
                         ArmourVariant               new_variant,
                         std::vector<unsigned int>&& new_deco_slots,
                         std::vector<std::pair<const Skill*, unsigned int>>&& new_skills,
                         std::string&&               new_piece_name_postfix,
                         const ArmourSet*            new_set,
                         const SetBonus*             new_set_bonus) noexcept
    : slot               (std::move(new_slot              ))
    , variant            (std::move(new_variant           ))
    , deco_slots         (std::move(new_deco_slots        ))
    , skills             (std::move(new_skills            ))
    , piece_name_postfix (std::move(new_piece_name_postfix))
    , set                (std::move(new_set               ))
    , set_bonus          (std::move(new_set_bonus         ))
{
}


ArmourSet::ArmourSet(std::string&&   new_set_name,
                     Tier            new_tier,
                     std::string&&   new_piece_name_prefix,
                     unsigned int    new_rarity,
                     const SetBonus* new_set_bonus,
                     std::vector<std::shared_ptr<ArmourPiece>>&& new_pieces) noexcept
    : set_name          (std::move(new_set_name         ))
    , tier              (std::move(new_tier             ))
    , piece_name_prefix (std::move(new_piece_name_prefix))
    , rarity            (std::move(new_rarity           ))
    , set_bonus         (std::move(new_set_bonus        ))
    , pieces            (std::move(new_pieces           ))
{
}


const ArmourDatabase ArmourDatabase::read_db_file(const std::string& filename, const SkillsDatabase& skills_db) {
    ArmourDatabase new_db;

    nlohmann::json j;

    {
        std::ifstream f(filename); // open file
        f >> j;
    } // close file

    assert(j.is_object());

    std::unordered_map<std::string, std::vector<std::string>> naming_schemes;

    // TODO: I don't like how I don't know what the type of e is.
    for (auto& e : j["naming_schemes"].items()) {
        nlohmann::json jj(e.value());

        std::string scheme_id = e.key();
        if (!Utils::is_upper_snake_case(scheme_id)) {
            throw std::runtime_error("Armour naming scheme IDs in the armour database must be in UPPER_SNAKE_CASE. "
                                     "Offending ID: '" + scheme_id + "'.");
        }
        if (Utils::map_has_key(naming_schemes, scheme_id)) { // UNIQUENESS CHECK
            throw std::runtime_error("Armour naming scheme IDs must be unique.");
        }

        std::vector<std::string> postfixes = jj;
        if (postfixes.size() != k_NUM_POSSIBLE_SLOTS) {
            throw std::runtime_error("There must be exactly " + std::to_string(k_NUM_POSSIBLE_SLOTS)
                                     + " entries in a naming scheme. Offending ID: '" + scheme_id + "'.");
        }

        // TODO: Is this a guaranteed move?
        naming_schemes.insert({std::move(scheme_id), std::move(postfixes)});
    }

    // TODO: I don't like how I don't know what the type of e is.
    nlohmann::json j_armour = j["armour"];
    if (!j_armour.is_array()) {
        throw std::runtime_error("'armour' key in the armour database must be an array.");
    }
    for (auto& e : j["armour"].items()) {
        nlohmann::json jj(e.value());

        std::string set_name = jj["set"];
        Tier tier = upper_snake_case_to_tier(jj["tier"]);
        std::pair<std::string, Tier> full_key = {set_name, tier};
        if (!Utils::has_ascii_letters(set_name)) { // SANITY CHECK
            throw std::runtime_error("Armour set names must have at least one ASCII letter (A-Z or a-z).");
        }
        if (Utils::map_has_key(new_db.armour_sets, full_key)) { // UNIQUENESS CHECK
            throw std::runtime_error("Armour set names must be unique within their tier. "
                                     "Offending set name: '" + set_name + "'.");
        }

        std::string piece_name_prefix = jj["prefix"];
        if (!Utils::has_ascii_letters(piece_name_prefix)) { // SANITY CHECK
            throw std::runtime_error("Armour set prefixes must have at least one ASCII letter (A-Z or a-z).");
        }

        std::string naming_scheme = jj["naming_scheme"];
        std::vector<std::string>& piece_name_postfixes = naming_schemes.at(naming_scheme);

        unsigned int rarity = jj["rarity"];
        if ((rarity < k_MIN_RARITY) || (rarity > k_MAX_RARITY)) {
            throw std::runtime_error("Armour sets must have valid rarity levels. "
                                     "Offending set: '" + set_name + "'.");
        }

        const SetBonus* set_bonus;
        if (jj["set_bonus"].is_null()) {
            set_bonus = nullptr;
        } else {
            std::string set_bonus_str = jj["set_bonus"];
            set_bonus = skills_db.set_bonus_at(set_bonus_str);
        }

        std::vector<std::shared_ptr<ArmourPiece>> pieces;

        // Oh my god there has got to be an easier way than this for-loop.
        for (const std::pair<std::string, Tier>& t : variant_keys) {
            const std::string variant_key = t.first;
            const Tier variant_tier = t.second;

            // Skip if the variant key isn't present.
            if (jj.count(variant_key) == 0) continue;
            assert(jj.count(variant_key) == 1);

            // Check if the variant key's tier matches the set's tier.
            if (variant_tier != tier) {
                throw std::runtime_error("Armour set variants must match the set's tier. "
                                         "Offending set: '" + set_name + "'.");
            }

            const ArmourVariant variant = upper_snake_case_to_armour_variant(variant_key);

            // Iterate through the variant pieces!
            for (auto& ee : jj[variant_key].items()) {
                nlohmann::json jjj(ee.value());

                const std::string armour_slot_str = ee.key();
                ArmourSlot slot = upper_case_to_armour_slot(armour_slot_str);

                if ((!jjj.is_array()) || (jjj.size() != 2)) {
                    throw std::runtime_error("Armour pieces must be represented by arrays of size 2. "
                                             "Offending set: '" + set_name + "'.");
                }

                std::vector<unsigned int> deco_slots = jjj[0];
                if (deco_slots.size() > k_MAX_DECO_SLOTS_PER_PIECE) {
                    throw std::runtime_error("Armour pieces must have at most 3 decoration slots. "
                                             "Offending set: '" + set_name + "'.");
                }
                for (unsigned int& deco_size : deco_slots) {
                    if ((deco_size < k_MIN_DECO_SIZE) || (deco_size > k_MAX_DECO_SIZE)) {
                        throw std::runtime_error("Armour pieces must have decoration sizes between 1 and 4. "
                                                 "Offending set: '" + set_name + "'.");
                    }
                }

                std::vector<std::pair<const Skill*, unsigned int>> skills;
                for (auto& eee : jjj[1].items()) {
                    const std::string skill_name = eee.key();
                    const Skill* const skill = skills_db.skill_at(skill_name);

                    const unsigned int skill_level = eee.value();
                    if (skill_level > skill->secret_limit) { // SANITY CHECK
                        throw std::runtime_error("Armour pieces must not have a skill level greater than "
                                                 "the maximum possible level for it. "
                                                 "Offending set: '" + set_name + "'.");
                    }
                    
                    skills.emplace_back(skill, skill_level);
                }
                
                std::string piece_name_postfix = piece_name_postfixes[armour_slot_to_naming_scheme_index(slot)];

                pieces.emplace_back(std::make_shared<ArmourPiece>(slot,
                                                                  variant,

                                                                  std::move(deco_slots),
                                                                  std::move(skills),

                                                                  std::move(piece_name_postfix),

                                                                  nullptr, // We adjust this later!
                                                                  set_bonus ));
            }
        }

        new_db.armour_sets.insert({std::move(full_key), std::make_shared<ArmourSet>(std::move(set_name),
                                                                                    std::move(tier),

                                                                                    std::move(piece_name_prefix),

                                                                                    std::move(rarity),
                                                                                    std::move(set_bonus),
                                                                                    
                                                                                    std::move(pieces)) });

    }

    // One final step of setting remaining pointers.
    for (auto& e : new_db.armour_sets) {
        ArmourSet* armour_set = e.second.get();
        for (std::shared_ptr<ArmourPiece>& ee : armour_set->pieces) {
            ee->set = armour_set;
        }
    }

    return new_db;
}


ArmourDatabase::ArmourDatabase() noexcept = default;


} // namespace

