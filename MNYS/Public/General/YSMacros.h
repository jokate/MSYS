#pragma once

/**
 * YSMacros.h
 *
 * 프로젝트 공통 Accessor 매크로 모음.
 * USTRUCT 멤버 변수에 대한 Getter / Setter 보일러플레이트를 제거합니다.
 *
 * ───────────────────────────────────────────────────
 *  주의: 템플릿 타입에 쉼표가 포함된 경우 (예: TMap<FName, int32>)
 *        YS_ACCESSOR 한 줄로 쓰면 컴파일 오류가 납니다.
 *        그런 경우 YS_GETTER / YS_SETTER 를 분리해서 사용하세요.
 * ───────────────────────────────────────────────────
 */


// ─── Value / Pointer Type ────────────────────────────────────────────────────

/** Getter + Setter (값 or 포인터 타입)
 *  YS_ACCESSOR(float, Health)
 *  => GetHealth() const  /  SetHealth(float)
 */
#define YS_ACCESSOR(Type, MemberName)                                          \
	FORCEINLINE Type Get##MemberName() const { return MemberName; }            \
	FORCEINLINE void Set##MemberName(Type In##MemberName) { MemberName = In##MemberName; }

/** Getter + Setter (대형 값 타입 — 복사 비용 절감용 const ref)
 *  YS_ACCESSOR_REF(FString, PlayerName)
 *  => GetPlayerName() const&  /  SetPlayerName(const FString&)
 */
#define YS_ACCESSOR_REF(Type, MemberName)                                                   \
	FORCEINLINE const Type& Get##MemberName() const { return MemberName; }                  \
	FORCEINLINE void Set##MemberName(const Type& In##MemberName) { MemberName = In##MemberName; }

/** Getter only (값 반환)
 *  YS_GETTER(float, Health)
 */
#define YS_GETTER(Type, MemberName)                                            \
	FORCEINLINE Type Get##MemberName() const { return MemberName; }

/** Getter only (const ref 반환 — 대형 값 타입)
 *  YS_GETTER_REF(TArray<FName>, Tags)
 */
#define YS_GETTER_REF(Type, MemberName)                                        \
	FORCEINLINE const Type& Get##MemberName() const { return MemberName; }

/** Setter only (값 전달)
 *  YS_SETTER(float, Health)
 */
#define YS_SETTER(Type, MemberName)                                            \
	FORCEINLINE void Set##MemberName(Type In##MemberName) { MemberName = In##MemberName; }

/** Setter only (const ref 전달 — 대형 값 타입)
 *  YS_SETTER_REF(TArray<FName>, Tags)
 */
#define YS_SETTER_REF(Type, MemberName)                                                     \
	FORCEINLINE void Set##MemberName(const Type& In##MemberName) { MemberName = In##MemberName; }


// ─── Bool Type ───────────────────────────────────────────────────────────────
//
//  UE 컨벤션: 변수명 = bIs<PropertyName>  →  Getter = Is<PropertyName>()
//
//  MemberName   : 실제 변수 이름  (예: bIsInputAcceptable)
//  PropertyName : b 없는 의미 이름 (예: InputAcceptable)

/** Bool Getter + Setter
 *  YS_BOOL_ACCESSOR(bIsInputAcceptable, InputAcceptable)
 *  => IsInputAcceptable() const  /  SetInputAcceptable(bool)
 */
#define YS_BOOL_ACCESSOR(MemberName, PropertyName)                             \
	FORCEINLINE bool Is##PropertyName() const { return MemberName; }           \
	FORCEINLINE void Set##PropertyName(bool bNewValue) { MemberName = bNewValue; }

/** Bool Getter only
 *  YS_BOOL_GETTER(bIsInputAcceptable, InputAcceptable)
 *  => IsInputAcceptable() const
 */
#define YS_BOOL_GETTER(MemberName, PropertyName)                               \
	FORCEINLINE bool Is##PropertyName() const { return MemberName; }

/** Bool Setter only
 *  YS_BOOL_SETTER(bIsInputAcceptable, InputAcceptable)
 *  => SetInputAcceptable(bool)
 */
#define YS_BOOL_SETTER(MemberName, PropertyName)                               \
	FORCEINLINE void Set##PropertyName(bool bNewValue) { MemberName = bNewValue; }


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

