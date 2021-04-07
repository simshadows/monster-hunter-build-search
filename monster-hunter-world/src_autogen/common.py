"""
Filename: common.py
Author:   <contact@simshadows.com>

Common components for the source generator scripts.
"""

import os
import re
import json

ENCODING = "utf-8"

def json_read(relfilepath):
    with open(relfilepath, encoding=ENCODING, mode="r") as f:
        return json.loads(f.read())

def file_write(relfilepath, *, data):
    with open(relfilepath, encoding=ENCODING, mode="w") as f:
        f.write(data)
    return

def is_upper_snake_case(s):
    return bool(re.match("^[A-Z0-9_]+$", s))

def is_safe_name(s):
    return bool(re.match("^[():a-zA-Z0-9 '+/_-]+$", s))

###

def skill_id_to_source_identifier(skill_id):
    if not is_upper_snake_case(skill_id):
        raise ValueError("Skill IDs must be upper case letters, numbers, and underscores.")
    return "g_skill_" + skill_id.lower()

def skill_id_to_source_identifier_fromforeign(skill_id):
    if not is_upper_snake_case(skill_id):
        raise ValueError("Skill IDs must be upper case letters, numbers, and underscores.")
    return "SkillsDatabase::g_skill_" + skill_id.lower()

