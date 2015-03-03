// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "BlueprintGraphPrivatePCH.h"
#include "KismetCompilerMisc.h"
#include "K2Node_EnumLiteral.h"
#include "BlueprintFieldNodeSpawner.h"
#include "EditorCategoryUtils.h"
#include "BlueprintActionDatabaseRegistrar.h"

const FString& UK2Node_EnumLiteral::GetEnumInputPinName()
{
	static const FString Name(TEXT("Enum"));
	return Name;
}

UK2Node_EnumLiteral::UK2Node_EnumLiteral(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UK2Node_EnumLiteral::ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);
	if (!Enum)
	{
		MessageLog.Error(*FString::Printf(*NSLOCTEXT("K2Node", "EnumLiteral_NullEnumError", "Undefined Enum in @@").ToString()), this);
	}
}

void UK2Node_EnumLiteral::AllocateDefaultPins()
{
	const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

	auto InputPin = CreatePin(EGPD_Input, Schema->PC_Byte, TEXT(""), Enum, false, false, GetEnumInputPinName());
	Schema->SetPinDefaultValueBasedOnType(InputPin);

	CreatePin(EGPD_Output, Schema->PC_Byte, TEXT(""), Enum, false, false, Schema->PN_ReturnValue);

	Super::AllocateDefaultPins();
}

FName UK2Node_EnumLiteral::GetPaletteIcon(FLinearColor& OutColor) const
{ 
	static const FName PaletteIconName(TEXT("GraphEditor.Enum_16x"));
	return PaletteIconName;
}

FText UK2Node_EnumLiteral::GetTooltipText() const
{
	if (Enum == nullptr)
	{
		return NSLOCTEXT("K2Node", "BadEnumLiteral_Tooltip", "Literal enum (bad enum)");
	}
	else if (CachedTooltip.IsOutOfDate())
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("EnumName"), FText::FromName(Enum->GetFName()));
		CachedTooltip = FText::Format(NSLOCTEXT("K2Node", "EnumLiteral_Tooltip", "Literal enum {EnumName}"), Args);
	}
	return CachedTooltip;	
}

FText UK2Node_EnumLiteral::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (CachedTooltip.IsOutOfDate())
	{
		return GetTooltipText();
	}
	return CachedTooltip;
}

class FKCHandler_EnumLiteral : public FNodeHandlingFunctor
{
public:
	FKCHandler_EnumLiteral(FKismetCompilerContext& InCompilerContext)
		: FNodeHandlingFunctor(InCompilerContext)
	{
	}

	virtual void RegisterNets(FKismetFunctionContext& Context, UEdGraphNode* Node) override
	{
		check(Context.Schema && Cast<UK2Node_EnumLiteral>(Node));
		FNodeHandlingFunctor::RegisterNets(Context, Node);

		UEdGraphPin* InPin = Node->FindPinChecked(UK2Node_EnumLiteral::GetEnumInputPinName());
		UEdGraphPin* Net = FEdGraphUtilities::GetNetFromPin(InPin);
		if (Context.NetMap.Find(Net) == NULL)
		{
			FBPTerminal* Term = Context.CreateLocalTerminalFromPinAutoChooseScope(Net, Context.NetNameMap->MakeValidName(Net));
			Context.NetMap.Add(Net, Term);
		}

		FBPTerminal** ValueSource = Context.NetMap.Find(Net);
		check(ValueSource && *ValueSource);
		UEdGraphPin* OutPin = Node->FindPinChecked(Context.Schema->PN_ReturnValue);
		if (ensure(Context.NetMap.Find(OutPin) == NULL))
		{
			FBPTerminal* TerminalPtr = *ValueSource; //necessary because of CheckAddress in Map::Add
			Context.NetMap.Add(OutPin, TerminalPtr);
		}
	}
};

FNodeHandlingFunctor* UK2Node_EnumLiteral::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return new FKCHandler_EnumLiteral(CompilerContext);
}

void UK2Node_EnumLiteral::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	auto SetNodeEnumLambda = [](UEdGraphNode* NewNode, UField const* /*EnumField*/, TWeakObjectPtr<UEnum> NonConstEnumPtr)
	{
		UK2Node_EnumLiteral* EnumNode = CastChecked<UK2Node_EnumLiteral>(NewNode);
		EnumNode->Enum = NonConstEnumPtr.Get();
	};

	for (TObjectIterator<UEnum> EnumIt; EnumIt; ++EnumIt)
	{
		UEnum const* EnumToConsider = (*EnumIt);
		if (!UEdGraphSchema_K2::IsAllowableBlueprintVariableType(EnumToConsider))
		{
			continue;
		}

		// to keep from needlessly instantiating a UBlueprintNodeSpawners, first   
		// check to make sure that the registrar is looking for actions of this type
		// (could be regenerating actions for a specific asset, and therefore the 
		// registrar would only accept actions corresponding to that asset)
		if (!ActionRegistrar.IsOpenForRegistration(EnumToConsider))
		{
			continue;
		}

		UBlueprintFieldNodeSpawner* NodeSpawner = UBlueprintFieldNodeSpawner::Create(GetClass(), EnumToConsider);
		check(NodeSpawner != nullptr);
		TWeakObjectPtr<UEnum> NonConstEnumPtr = EnumToConsider;
		NodeSpawner->SetNodeFieldDelegate = UBlueprintFieldNodeSpawner::FSetNodeFieldDelegate::CreateStatic(SetNodeEnumLambda, NonConstEnumPtr);

		// this enum could belong to a class, or is a user defined enum (asset), 
		// that's why we want to make sure to register it along with the action 
		// (so the action can be refreshed when the class/asset is).
		ActionRegistrar.AddBlueprintAction(EnumToConsider, NodeSpawner);
	}
}

FText UK2Node_EnumLiteral::GetMenuCategory() const
{
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Enum);
}

