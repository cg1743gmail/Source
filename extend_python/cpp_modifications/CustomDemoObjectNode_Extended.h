// Copyright Epic Games, Inc. All Rights Reserved.
// Extended version with Python API support

#pragma once

#include "CoreMinimal.h"
#include "Nodes/InterchangeBaseNode.h"
#include "CustomDemoObjectNode_Extended.generated.h"

/**
 * Extended Custom Demo Object Node with Python API support
 * This class extends the original UCustomDemoObjectNode to provide Python-accessible methods
 * while maintaining full backward compatibility with the original implementation.
 */
UCLASS(BlueprintType, Category = "Interchange|Node|Custom")
class CUSTOMNODES_API UCustomDemoObjectNodeExtended : public UInterchangeBaseNode
{
    GENERATED_BODY()

public:
    UCustomDemoObjectNodeExtended();

    /**
     * Initialize the custom demo object node
     * @param UniqueID Unique identifier for the node
     * @param DisplayLabel Display label for the node
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom")
    void InitializeCustomDemoObjectNode(const FString& UniqueID, const FString& DisplayLabel);

    /**
     * Get the type name of this node
     * @return Type name string
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom")
    virtual FString GetTypeName() const override { return TEXT("CustomDemoObjectNodeExtended"); }

    /**
     * Get the payload key (Python-friendly version)
     * @return Payload key string, empty if not set
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    FString GetPayLoadKeyForPython() const;

    /**
     * Set the payload key
     * @param PayLoadKey The payload key to set
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom")
    virtual void SetPayLoadKey(const FString& PayLoadKey);

    /**
     * Get the original payload key (for compatibility)
     * @return Optional payload key
     */
    virtual const TOptional<FString> GetPayLoadKey() const;

    // Python-specific attribute management

    /**
     * Set a custom attribute with string value
     * @param AttributeName Name of the attribute
     * @param AttributeValue String value of the attribute
     * @return True if attribute was set successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool SetCustomStringAttribute(const FString& AttributeName, const FString& AttributeValue);

    /**
     * Get a custom attribute as string value
     * @param AttributeName Name of the attribute
     * @param OutAttributeValue Output string value
     * @return True if attribute was found and retrieved
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool GetCustomStringAttribute(const FString& AttributeName, FString& OutAttributeValue) const;

    /**
     * Set a custom attribute with integer value
     * @param AttributeName Name of the attribute
     * @param AttributeValue Integer value of the attribute
     * @return True if attribute was set successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool SetCustomIntAttribute(const FString& AttributeName, int32 AttributeValue);

    /**
     * Get a custom attribute as integer value
     * @param AttributeName Name of the attribute
     * @param OutAttributeValue Output integer value
     * @return True if attribute was found and retrieved
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool GetCustomIntAttribute(const FString& AttributeName, int32& OutAttributeValue) const;

    /**
     * Set a custom attribute with float value
     * @param AttributeName Name of the attribute
     * @param AttributeValue Float value of the attribute
     * @return True if attribute was set successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool SetCustomFloatAttribute(const FString& AttributeName, float AttributeValue);

    /**
     * Get a custom attribute as float value
     * @param AttributeName Name of the attribute
     * @param OutAttributeValue Output float value
     * @return True if attribute was found and retrieved
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool GetCustomFloatAttribute(const FString& AttributeName, float& OutAttributeValue) const;

    /**
     * Set a custom attribute with boolean value
     * @param AttributeName Name of the attribute
     * @param AttributeValue Boolean value of the attribute
     * @return True if attribute was set successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool SetCustomBoolAttribute(const FString& AttributeName, bool AttributeValue);

    /**
     * Get a custom attribute as boolean value
     * @param AttributeName Name of the attribute
     * @param OutAttributeValue Output boolean value
     * @return True if attribute was found and retrieved
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool GetCustomBoolAttribute(const FString& AttributeName, bool& OutAttributeValue) const;

    /**
     * Remove a custom attribute
     * @param AttributeName Name of the attribute to remove
     * @return True if attribute was removed successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool RemoveCustomAttribute(const FString& AttributeName);

    /**
     * Get all custom attribute names
     * @return Array of attribute names
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    TArray<FString> GetAllCustomAttributeNames() const;

    /**
     * Check if a custom attribute exists
     * @param AttributeName Name of the attribute to check
     * @return True if attribute exists
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool HasCustomAttribute(const FString& AttributeName) const;

    /**
     * Get all attributes as a map (Python-friendly)
     * @return Map of attribute name to string value
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    TMap<FString, FString> GetAllAttributesAsMap() const;

    /**
     * Set multiple attributes from a map
     * @param AttributeMap Map of attribute name to string value
     * @return Number of attributes successfully set
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    int32 SetAttributesFromMap(const TMap<FString, FString>& AttributeMap);

    /**
     * Clear all custom attributes
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    void ClearAllCustomAttributes();

    // Node relationship management for Python

    /**
     * Add a target node UID
     * @param TargetNodeUid UID of the target node
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    void AddTargetNodeUidForPython(const FString& TargetNodeUid);

    /**
     * Remove a target node UID
     * @param TargetNodeUid UID of the target node to remove
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    void RemoveTargetNodeUidForPython(const FString& TargetNodeUid);

    /**
     * Get all target node UIDs
     * @return Array of target node UIDs
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    TArray<FString> GetTargetNodeUidsForPython() const;

    /**
     * Check if a target node UID exists
     * @param TargetNodeUid UID to check
     * @return True if target node UID exists
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool HasTargetNodeUid(const FString& TargetNodeUid) const;

    /**
     * Get node information as a map
     * @return Map containing node information
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    TMap<FString, FString> GetNodeInfo() const;

    /**
     * Validate node data
     * @param OutErrorMessages Array to store validation error messages
     * @return True if node data is valid
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool ValidateNodeData(TArray<FString>& OutErrorMessages) const;

    /**
     * Clone this node
     * @param NewUniqueID New unique ID for the cloned node
     * @return Cloned node instance
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    UCustomDemoObjectNodeExtended* CloneNode(const FString& NewUniqueID) const;

    /**
     * Export node data to JSON string
     * @return JSON string representation of node data
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    FString ExportToJson() const;

    /**
     * Import node data from JSON string
     * @param JsonString JSON string containing node data
     * @return True if import was successful
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    bool ImportFromJson(const FString& JsonString);

private:
    // Attribute key definitions
    const UE::Interchange::FAttributeKey Macro_CustomPayLoadKey = UE::Interchange::FAttributeKey(TEXT("PayLoadKey"));

    // Helper methods for attribute management
    FString GenerateCustomAttributeKey(const FString& AttributeName) const;
    bool SetAttributeInternal(const FString& AttributeName, const FString& AttributeValue);
    bool GetAttributeInternal(const FString& AttributeName, FString& OutAttributeValue) const;
};

/**
 * Python helper class for managing multiple nodes
 */
UCLASS(BlueprintType, Category = "Interchange|Node|Custom|Python")
class CUSTOMNODES_API UPythonNodeManager : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Create a new custom demo object node
     * @param UniqueID Unique identifier for the node
     * @param DisplayLabel Display label for the node
     * @return New node instance
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    static UCustomDemoObjectNodeExtended* CreateCustomDemoObjectNode(const FString& UniqueID, const FString& DisplayLabel);

    /**
     * Find nodes by attribute value
     * @param Nodes Array of nodes to search
     * @param AttributeName Name of the attribute to search
     * @param AttributeValue Value to search for
     * @return Array of matching nodes
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    static TArray<UCustomDemoObjectNodeExtended*> FindNodesByAttribute(const TArray<UCustomDemoObjectNodeExtended*>& Nodes,
                                                                       const FString& AttributeName,
                                                                       const FString& AttributeValue);

    /**
     * Batch set attribute on multiple nodes
     * @param Nodes Array of nodes to modify
     * @param AttributeName Name of the attribute to set
     * @param AttributeValue Value to set
     * @return Number of nodes successfully modified
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    static int32 BatchSetAttribute(const TArray<UCustomDemoObjectNodeExtended*>& Nodes,
                                  const FString& AttributeName,
                                  const FString& AttributeValue);

    /**
     * Validate multiple nodes
     * @param Nodes Array of nodes to validate
     * @param OutValidationResults Array of validation results
     * @return Number of valid nodes
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Node|Custom|Python")
    static int32 BatchValidateNodes(const TArray<UCustomDemoObjectNodeExtended*>& Nodes,
                                   TArray<FString>& OutValidationResults);
};
