/***************************************************

					AXIA|Trace4

		(C) Copyright  www.thecodeway.com 2023
***************************************************/
#include "AxTraceEditorK2Node_Value.h"
#include "EdGraphSchema_K2.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_MakeStruct.h"
#include "AxTraceFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "K2Node_AxTraceValueNode"

struct FK2Node_AxTraceValueHelper
{
	static FName NamePin;
	static FName ValuePin;
};

FName FK2Node_AxTraceValueHelper::NamePin(TEXT("Name"));
FName FK2Node_AxTraceValueHelper::ValuePin(TEXT("Value"));

void UAxTraceValueNode::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UAxTraceValueNode::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "AxTrace");
}

FText UAxTraceValueNode::GetNodeTitle(ENodeTitleType::Type Title) const
{
	return LOCTEXT("DefaultTitle", "AxValue");
}

FText UAxTraceValueNode::GetTooltipText() const
{
	return FText::FromString(TEXT("Watch value"));
}

void UAxTraceValueNode::AllocateDefaultPins()
{
	// Add execution pins
	UEdGraphPin* execPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	UEdGraphPin* thenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// Add Name  pin
	UEdGraphPin* namePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, FK2Node_AxTraceValueHelper::NamePin);
	// Add Message pin
	UEdGraphPin* valuePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, FK2Node_AxTraceValueHelper::ValuePin);

	Super::AllocateDefaultPins();
}

void UAxTraceValueNode::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);
	
	UEdGraphPin* execPin = GetExecPin();
	UEdGraphPin* namePin = GetInputPin(FK2Node_AxTraceValueHelper::NamePin);
	UEdGraphPin* valuePin = GetInputPin(FK2Node_AxTraceValueHelper::ValuePin);
	UEdGraphPin* thenPin = GetThenPin();

	// not working yet!
	if (execPin == nullptr || thenPin == nullptr)
	{
		BreakAllNodeLinks();
		return;
	}

	//is linked to a valid make grpc message struct node?
	if (valuePin == nullptr || valuePin->LinkedTo.Num() == 0)
	{
		BreakAllNodeLinks();
		return;
	}

	UEdGraphPin* valuePinLink = valuePin->LinkedTo[0];
	if (valuePinLink == nullptr)
	{
		BreakAllNodeLinks();
		return;
	}

	FName InternalFunctionName;
	if (valuePinLink->PinType.PinCategory == UEdGraphSchema_K2::PC_Boolean)
	{
		static const FName AxTrace_Internal_FunctionName = GET_FUNCTION_NAME_CHECKED(UAxTrace, Value_Boolean);
		InternalFunctionName = AxTrace_Internal_FunctionName;
	}
	else if (valuePinLink->PinType.PinCategory == UEdGraphSchema_K2::PC_Byte)
	{
		static const FName AxTrace_Internal_FunctionName = GET_FUNCTION_NAME_CHECKED(UAxTrace, Value_UInt8);
		InternalFunctionName = AxTrace_Internal_FunctionName;
	}
	else if (valuePinLink->PinType.PinCategory == UEdGraphSchema_K2::PC_Int)
	{
		static const FName AxTrace_Internal_FunctionName = GET_FUNCTION_NAME_CHECKED(UAxTrace, Value_Int32);
		InternalFunctionName = AxTrace_Internal_FunctionName;
	}
	else if (valuePinLink->PinType.PinCategory == UEdGraphSchema_K2::PC_Int64)
	{
		static const FName AxTrace_Internal_FunctionName = GET_FUNCTION_NAME_CHECKED(UAxTrace, Value_Int64);
		InternalFunctionName = AxTrace_Internal_FunctionName;
	}
	else if (valuePinLink->PinType.PinCategory == UEdGraphSchema_K2::PC_Float)
	{
		static const FName AxTrace_Internal_FunctionName = GET_FUNCTION_NAME_CHECKED(UAxTrace, Value_Float);
		InternalFunctionName = AxTrace_Internal_FunctionName;
	}
	else if (valuePinLink->PinType.PinCategory == UEdGraphSchema_K2::PC_Name)
	{
		static const FName AxTrace_Internal_FunctionName = GET_FUNCTION_NAME_CHECKED(UAxTrace, Value_Name);
		InternalFunctionName = AxTrace_Internal_FunctionName;
	}
	else if (valuePinLink->PinType.PinCategory == UEdGraphSchema_K2::PC_String)
	{
		static const FName AxTrace_Internal_FunctionName = GET_FUNCTION_NAME_CHECKED(UAxTrace, Value_String);
		InternalFunctionName = AxTrace_Internal_FunctionName;
	}
	else
	{
		//compile error
		CompilerContext.MessageLog.Error(TEXT("Only accept these types: Boolean, Byte, Int, Int64, Float, Name and String. @@"), this);
		BreakAllNodeLinks();
		return;
	}

	// here we adapt/bind our pins to the static function pins that we are calling.
	UK2Node_CallFunction* callFunctionNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	callFunctionNode->FunctionReference.SetExternalMember(InternalFunctionName, UAxTrace::StaticClass());
	callFunctionNode->AllocateDefaultPins();

	// get pins from call function node
	UEdGraphPin* callFunction_ExecPin = callFunctionNode->GetExecPin();
	UEdGraphPin* callFunction_NamePin = callFunctionNode->FindPinChecked(FK2Node_AxTraceValueHelper::NamePin);
	UEdGraphPin* callFunction_ValuePin = callFunctionNode->FindPinChecked(FK2Node_AxTraceValueHelper::ValuePin);
	UEdGraphPin* callFunction_ThenPin = callFunctionNode->GetThenPin();

	//exec pin 
	CompilerContext.MovePinLinksToIntermediate(*execPin, *callFunction_ExecPin);
	// pretty mode pin
	CompilerContext.MovePinLinksToIntermediate(*namePin, *callFunction_NamePin);
	// return value pin
	CompilerContext.MovePinLinksToIntermediate(*valuePin, *callFunction_ValuePin);
	//then pin
	CompilerContext.MovePinLinksToIntermediate(*thenPin, *callFunction_ThenPin);

	// break any links to the expanded node
	BreakAllNodeLinks();
}

UEdGraphPin* UAxTraceValueNode::GetThenPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_Then);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UAxTraceValueNode::GetInputPin(const FName& PinName) const
{
	UEdGraphPin* Pin = nullptr;
	for (UEdGraphPin* TestPin : Pins)
	{
		if (TestPin && TestPin->PinName == PinName)
		{
			Pin = TestPin;
			break;
		}
	}
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}


void UAxTraceValueNode::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	if (Pin && (Pin->PinName == FK2Node_AxTraceValueHelper::ValuePin))
	{
		OnMessagePinChanged();
	}
}

void UAxTraceValueNode::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin && (ChangedPin->PinName == FK2Node_AxTraceValueHelper::ValuePin))
	{
		OnMessagePinChanged();
	}
}

void UAxTraceValueNode::OnMessagePinChanged()
{
	UEdGraphPin* valuePin = GetInputPin(FK2Node_AxTraceValueHelper::ValuePin);

	// Mark dirty
	FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
}

#undef LOCTEXT_NAMESPACE
