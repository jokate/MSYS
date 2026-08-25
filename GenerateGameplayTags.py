import re
import sys
from pathlib import Path

enum_file = Path(sys.argv[1])
tag_cpp_file = Path(sys.argv[2])
out_h = Path(sys.argv[3])
out_cpp = Path(sys.argv[4])

enum_text = enum_file.read_text(encoding="utf-8")
tag_cpp_text = tag_cpp_file.read_text(encoding="utf-8")

# enum class EYSInputStatesType 값 추출
enum_match = re.search(
    r"enum\s+class\s+EYSInputStatesType\s*:\s*[^{]+\{([\s\S]*?)\}\s*;",
    enum_text
)

if not enum_match:
    raise RuntimeError("EYSInputStatesType enum을 찾지 못했습니다.")

enum_body = enum_match.group(1)

states = []

for_line = enum_body.splitlines()
for line in for_line:
    line = line.split("//")[0].strip()
    if not line:
        continue

    line = line.rstrip(",")
    name = line.split("=")[0].strip()

    if name and name not in ("None", "End"):
        states.append(name)

# UE_DEFINE_GAMEPLAY_TAG(InputAttack, "Input.Attack") 추출
input_tags = re.findall(
    r"UE_DEFINE_GAMEPLAY_TAG\s*\(\s*(\w+)\s*,\s*\"([^\"]+)\"\s*\)",
    tag_cpp_text
)

input_tags = [
    (var_name, tag_name)
    for var_name, tag_name in input_tags
    if tag_name.startswith("Input.")
]

# 대분류. 여러 상태를 한 태그로 묶어 "어느 상태에서든" 을 표현한다.
# 실제 상태가 아니므로 enum 이 아니라 여기서 관리한다.
CATEGORIES = ["Alive", "Actionable", "Grounded"]

scopes = states + CATEGORIES


def make_var_name(state_name, input_var_name):
    # Attack + InputAttack = AttackInputAttack
    return f"{state_name}{input_var_name}"

header = """#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

namespace YSTags
{
"""

for state in scopes:
    for input_var, input_tag in input_tags:
        var_name = make_var_name(state, input_var)
        header += f"\tUE_DECLARE_GAMEPLAY_TAG_EXTERN({var_name});\n"

header += """}
"""

cpp = """#include "General/YSGeneratedGameplayTags.h"

namespace YSTags
{
"""

for state in scopes:
    for input_var, input_tag in input_tags:
        var_name = make_var_name(state, input_var)
        tag_name = f"{state}.{input_tag}"
        cpp += f'\tUE_DEFINE_GAMEPLAY_TAG({var_name}, "{tag_name}");\n'

cpp += """}
"""

out_h.write_text(header, encoding="utf-8")
out_cpp.write_text(cpp, encoding="utf-8")

print(f"Generated: {out_h}")
print(f"Generated: {out_cpp}")