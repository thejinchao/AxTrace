/***************************************************

					AXIA|Trace4

		(C) Copyright  www.thecodeway.com 2023
***************************************************/
#pragma once
#include "CoreMinimal.h"
#include "K2Node.h"
#include "AxTraceEditorK2Node_Value.generated.h"

UCLASS()
class AXTRACEEDITOR_API UAxTraceValueNode : public UK2Node
{
	GENERATED_BODY()

public:
	// Override UK2Node Interface
	virtual bool IsNodeSafeToIgnore() const override { return true; }

	// Override UEdGraphNode Interface.
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type Title) const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin);

	// Get the then output pin
	UEdGraphPin* GetThenPin() const;
	// Get the input pin
	UEdGraphPin* GetInputPin(const FName& PinName) const;

protected:
	// Refresh pins when input message was changed
	void OnMessagePinChanged();
};
