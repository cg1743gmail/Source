// Copyright Epic Games, Inc. All Rights Reserved.
// Extended version with Python API support

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "InterchangeTranslatorBase.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "CustomDemoObjectPayloadData.h"
#include "CustomDemoObjectPayloadInterface.h"

#include "CustomFooTranslator_Extended.generated.h"

/**
 * Extended Custom Foo Translator with Python API support
 * This class extends the original UCustomFooTranslator to provide Python-accessible methods
 * while maintaining full backward compatibility with the original implementation.
 */
UCLASS(BlueprintType, Category = "Interchange|Translator|Custom")
class CUSTOMIMPORT_API UCustomFooTranslatorExtended : public UInterchangeTranslatorBase, 
                                                     public ICustomDemoObjectPayloadInterface
{
    GENERATED_BODY()

public:
    UCustomFooTranslatorExtended();

    //~ Begin UInterchangeTranslatorBase Interface
    virtual TArray<FString> GetSupportedFormats() const override;
    virtual EInterchangeTranslatorAssetType GetSupportedAssetTypes() const override;
    virtual bool Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const override;
    //~ End UInterchangeTranslatorBase Interface

    //~ Begin ICustomDemoObjectPayloadInterface Interface
    virtual TOptional<UE::Interchange::FCustomDemoObjectData> GetDemoObjectPayloadData(const FString& PayloadKey) const override;
    //~ End ICustomDemoObjectPayloadInterface Interface

    // Python-accessible methods
    
    /**
     * Get supported file formats for Python
     * @return Array of supported format strings
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    TArray<FString> GetSupportedFormatsForPython() const;

    /**
     * Translate a file from Python with simplified interface
     * @param FilePath Source file path
     * @param DestinationPath Destination path in UE content browser
     * @return True if translation succeeded
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    bool TranslateFileFromPython(const FString& FilePath, const FString& DestinationPath);

    /**
     * Get payload data as JSON string for Python
     * @param PayloadKey The payload key
     * @return JSON string representation of payload data
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    FString GetPayloadDataAsJson(const FString& PayloadKey) const;

    /**
     * Check if a file can be translated by this translator
     * @param FilePath Path to the file to check
     * @return True if file can be translated
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    bool CanTranslateFile(const FString& FilePath) const;

    /**
     * Get translator information for Python
     * @return Map containing translator metadata
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    TMap<FString, FString> GetTranslatorInfo() const;

    /**
     * Validate file before translation
     * @param FilePath Path to file to validate
     * @param OutErrorMessage Error message if validation fails
     * @return True if file is valid for translation
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    bool ValidateFileForTranslation(const FString& FilePath, FString& OutErrorMessage) const;

    /**
     * Set custom translation options from Python
     * @param Options Map of option key-value pairs
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    void SetTranslationOptions(const TMap<FString, FString>& Options);

    /**
     * Get current translation options
     * @return Map of current option key-value pairs
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    TMap<FString, FString> GetTranslationOptions() const;

    /**
     * Reset translation options to defaults
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    void ResetTranslationOptions();

    /**
     * Get translation statistics
     * @return Map containing translation statistics
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    TMap<FString, FString> GetTranslationStatistics() const;

protected:
    // Internal helper methods for Python integration
    
    /**
     * Create a node container for Python translation
     * @return New node container instance
     */
    UInterchangeBaseNodeContainer* CreateNodeContainerForPython() const;

    /**
     * Execute the actual translation with error handling
     * @param Container Node container to populate
     * @param FilePath Source file path
     * @return True if translation succeeded
     */
    bool ExecuteTranslationInternal(UInterchangeBaseNodeContainer& Container, const FString& FilePath) const;

    /**
     * Log translation event for Python
     * @param EventType Type of event (Info, Warning, Error)
     * @param Message Event message
     * @param FilePath Associated file path
     */
    void LogTranslationEvent(const FString& EventType, const FString& Message, const FString& FilePath = TEXT("")) const;

private:
    // Python-specific configuration
    UPROPERTY()
    TMap<FString, FString> TranslationOptions;

    // Statistics tracking
    mutable int32 TranslationCount;
    mutable int32 SuccessfulTranslations;
    mutable int32 FailedTranslations;
    mutable double TotalTranslationTime;

    // Initialize default options
    void InitializeDefaultOptions();

    // Update statistics
    void UpdateStatistics(bool bSuccess, double TranslationTime) const;
};

/**
 * Python helper class for managing multiple translators
 */
UCLASS(BlueprintType, Category = "Interchange|Translator|Custom|Python")
class CUSTOMIMPORT_API UPythonTranslatorManager : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Create a new translator instance
     * @return New translator instance
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    static UCustomFooTranslatorExtended* CreateTranslator();

    /**
     * Get all available translator types
     * @return Array of translator class names
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    static TArray<FString> GetAvailableTranslatorTypes();

    /**
     * Find best translator for file
     * @param FilePath Path to file
     * @return Best translator class name, or empty if none found
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    static FString FindBestTranslatorForFile(const FString& FilePath);

    /**
     * Batch translate multiple files
     * @param FilePaths Array of file paths to translate
     * @param DestinationPath Destination path for all files
     * @param OutResults Array of translation results
     * @return Number of successful translations
     */
    UFUNCTION(BlueprintCallable, Category = "Interchange|Translator|Custom|Python")
    static int32 BatchTranslateFiles(const TArray<FString>& FilePaths, 
                                    const FString& DestinationPath, 
                                    TArray<FString>& OutResults);
};
