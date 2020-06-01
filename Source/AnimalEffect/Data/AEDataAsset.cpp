// Copyright Epic Games, Inc. All Rights Reserved.

#include "AEDataAsset.h"

namespace
{

	// this struct was necessary to give IsDefault functionality to the UAEMetaAsset* for the annotation container
	// i tried to generecize this but the UObjectAnnotation wouldn't take it
	struct FPtrAnnotation
	{
		const UAEMetaAsset* AssetPtr;

		FPtrAnnotation()
		{}

		FPtrAnnotation(const UAEMetaAsset* InAssetPtr)
			: AssetPtr(InAssetPtr)
		{}

		bool IsDefault() const { return AssetPtr == nullptr; }

		operator const UAEMetaAsset* () const { return AssetPtr; }
	};

	static FUObjectAnnotationDense<FPtrAnnotation, true> MetaAnnotations;

}

TSubclassOf<AActor> UAEMetaAsset::GetActorClass() const
{
	UClass* LoadedClass = ActorClass.LoadSynchronous();
	if (LoadedClass)
	{
		if (MetaAnnotations.GetAnnotation(LoadedClass) == nullptr)
		{
			MetaAnnotations.AddAnnotation(LoadedClass, this);
		}
	}
	return LoadedClass;
}

const UAEMetaAsset* UAEMetaAsset::GetMetaAssetForClass(UClass* Class)
{
	const UAEMetaAsset* MetaAsset = MetaAnnotations.GetAnnotationRef(Class);
	if (MetaAsset == nullptr)
	{
		// #todo: we shouldn't be asking for a metaasset that hasn't been cached usually
		// if we do we need to find the correct metaasset and cache it
		// MetaAsset = FindMetaAsset(Class);
		// MetaAnnotations.AddAnnotation(Class, MetaAsset);
		check(false);
	}
	return MetaAsset;
}

FText UAEMetaAsset::GetTitle() const
{
	return Title.IsEmpty() ? FText::FromString(GetName()) : Title;
}
