@echo off

python GenerateGameplayTags.py ^
  "../Source/MNYS/Public/General/YSEnum.h" ^
  "../Source/MNYS/Private/General/YSInputGameplayTags.cpp" ^
  "../Source/MNYS/Public/General/YSGeneratedGameplayTags.h" ^
  "../Source/MNYS/Private/General/YSGeneratedGameplayTags.cpp"

pause
