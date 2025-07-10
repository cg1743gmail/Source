"""
Basic Import Example for Interchange Python Extension

This example demonstrates the basic usage of the Interchange Python API
for importing individual files and performing simple operations.
"""

import os
import sys

# Add the Python API to the path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))

from python_api import InterchangePythonAPI
from python_api.core.translators import CustomFooTranslator

def basic_file_import():
    """Demonstrate basic file import functionality"""
    print("=== Basic File Import Example ===")
    
    # Initialize the API
    api = InterchangePythonAPI()
    print(f"Interchange Python API Version: {api.get_version()}")
    
    # Get supported formats
    formats = api.get_supported_formats()
    print(f"Supported formats: {formats}")
    
    # Example file path (replace with your actual file)
    example_file = "C:/GameAssets/Characters/Hero.foo"
    destination_path = "/Game/Characters/"
    
    # Check if we can translate the file
    translator = CustomFooTranslator()
    
    if os.path.exists(example_file):
        can_translate = translator.can_translate_file(example_file)
        print(f"Can translate {example_file}: {can_translate}")
        
        if can_translate:
            # Validate the file first
            is_valid, error_message = translator.validate_file(example_file)
            print(f"File validation: {'Valid' if is_valid else 'Invalid'}")
            if not is_valid:
                print(f"Validation error: {error_message}")
                return False
            
            # Import the file
            print(f"Importing {example_file} to {destination_path}...")
            success = api.import_file(example_file, destination_path)
            
            if success:
                print("✅ Import successful!")
                
                # Get import statistics
                stats = translator.get_statistics()
                print(f"Translation statistics: {stats}")
                
            else:
                print("❌ Import failed!")
                return False
        else:
            print(f"❌ Cannot translate file: {example_file}")
            return False
    else:
        print(f"⚠️ Example file not found: {example_file}")
        print("Please update the example_file path to point to a valid .foo file")
        
        # Show mock import instead
        print("\n--- Mock Import Example ---")
        success = api.import_file("mock_file.foo", destination_path)
        print(f"Mock import result: {'Success' if success else 'Failed'}")
    
    return True

def translator_configuration_example():
    """Demonstrate translator configuration"""
    print("\n=== Translator Configuration Example ===")
    
    translator = CustomFooTranslator()
    
    # Get current options
    current_options = translator.get_options()
    print(f"Current options: {current_options}")
    
    # Set custom options
    custom_options = {
        "EnableDetailedLogging": "true",
        "ValidateInputFiles": "true",
        "CreateBackups": "false",
        "CustomOption": "example_value"
    }
    
    print(f"Setting custom options: {custom_options}")
    translator.set_options(custom_options)
    
    # Verify options were set
    updated_options = translator.get_options()
    print(f"Updated options: {updated_options}")
    
    # Get translator information
    info = translator.get_translator_info()
    print(f"Translator info: {info}")
    
    # Reset to defaults
    print("Resetting options to defaults...")
    translator.reset_options()
    
    default_options = translator.get_options()
    print(f"Default options: {default_options}")

def payload_data_example():
    """Demonstrate payload data handling"""
    print("\n=== Payload Data Example ===")
    
    translator = CustomFooTranslator()
    
    # Example payload key (in real usage, this would come from a translated file)
    payload_key = "example_payload_key"
    
    # Get payload data
    payload_data = translator.get_payload_data(payload_key)
    print(f"Payload data for '{payload_key}': {payload_data}")
    
    if payload_data:
        # Process payload data
        print("Processing payload data...")
        for key, value in payload_data.items():
            print(f"  {key}: {value}")

def error_handling_example():
    """Demonstrate error handling and validation"""
    print("\n=== Error Handling Example ===")
    
    translator = CustomFooTranslator()
    
    # Test with non-existent file
    non_existent_file = "C:/NonExistent/File.foo"
    print(f"Testing with non-existent file: {non_existent_file}")
    
    can_translate = translator.can_translate_file(non_existent_file)
    print(f"Can translate: {can_translate}")
    
    is_valid, error_message = translator.validate_file(non_existent_file)
    print(f"Validation result: {'Valid' if is_valid else 'Invalid'}")
    if not is_valid:
        print(f"Error: {error_message}")
    
    # Test with unsupported file format
    unsupported_file = "C:/SomeFile.txt"
    print(f"\nTesting with unsupported format: {unsupported_file}")
    
    can_translate = translator.can_translate_file(unsupported_file)
    print(f"Can translate: {can_translate}")
    
    # Try to import anyway (should fail gracefully)
    success = translator.import_file(unsupported_file, "/Game/Test/")
    print(f"Import result: {'Success' if success else 'Failed (as expected)'}")

def statistics_example():
    """Demonstrate statistics tracking"""
    print("\n=== Statistics Example ===")
    
    translator = CustomFooTranslator()
    
    # Reset statistics
    translator.reset_statistics()
    print("Statistics reset")
    
    # Perform some mock operations
    test_files = [
        "mock_file1.foo",
        "mock_file2.foo", 
        "invalid_file.txt",
        "mock_file3.foo"
    ]
    
    for file_path in test_files:
        print(f"Processing: {file_path}")
        translator.import_file(file_path, "/Game/Test/")
    
    # Get final statistics
    final_stats = translator.get_statistics()
    print(f"\nFinal statistics: {final_stats}")
    
    # Calculate success rate
    total = final_stats.get('total_translations', 0)
    successful = final_stats.get('successful_translations', 0)
    if total > 0:
        success_rate = (successful / total) * 100
        print(f"Success rate: {success_rate:.1f}%")

def main():
    """Main function to run all examples"""
    print("Interchange Python Extension - Basic Import Examples")
    print("=" * 60)
    
    try:
        # Run all examples
        basic_file_import()
        translator_configuration_example()
        payload_data_example()
        error_handling_example()
        statistics_example()
        
        print("\n" + "=" * 60)
        print("✅ All examples completed successfully!")
        
    except Exception as e:
        print(f"\n❌ Error running examples: {e}")
        import traceback
        traceback.print_exc()
        return False
    
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
