#include "Data/YSDataStruct.h"

#include "Ability/YSGameplayEffectHandler.h"

#if WITH_EDITOR
void FYSTableRowBase::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	FTableRowBase::OnDataTableChanged(InDataTable, InRowName);
	FYSGameplayEffectHandler::SyncAllIn(StaticStruct(), this);
}
#endif