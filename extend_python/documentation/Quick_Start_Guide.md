# Quick Start Guide - Interchange Python Extension

## Overview

This guide will help you get started with the Unreal Engine 5.5 Interchange Python Extension in just a few minutes. You'll learn how to set up the extension, perform basic imports, and configure automation.

## Prerequisites

### System Requirements
- Unreal Engine 5.5 or later
- Python 3.7 or later
- Custom Interchange Plugin installed and enabled

### Optional Dependencies
```bash
# For file monitoring (recommended)
pip install watchdog

# For advanced configuration management
pip install packaging
```

## Installation

### Step 1: Install the C++ Extensions

1. Copy the extended C++ files to your plugin directory:
   ```
   YourProject/Plugins/CustomInterchange/Source/
   ├── CustomImport/Public/CustomFooTranslator_Extended.h
   ├── CustomImport/Private/CustomFooTranslator_Extended.cpp
   ├── CustomNodes/Public/CustomDemoObjectNode_Extended.h
   ├── CustomNodes/Private/CustomDemoObjectNode_Extended.cpp
   └── ... (other extended files)
   ```

2. Regenerate project files and rebuild your project.

### Step 2: Set Up Python API

1. Copy the `extend_python` folder to your project:
   ```
   YourProject/Content/Python/extend_python/
   ```

2. Add the Python path to your Unreal Engine Python environment:
   ```python
   import sys
   sys.path.append('path/to/your/project/Content/Python')
   ```

## Basic Usage

### 1. Simple File Import

```python
# Import the API
from extend_python.python_api import InterchangePythonAPI

# Initialize
api = InterchangePythonAPI()

# Import a single file
success = api.import_file(
    file_path="C:/GameAssets/MyCharacter.foo",
    destination_path="/Game/Characters/"
)

if success:
    print("✅ Import successful!")
else:
    print("❌ Import failed!")
```

### 2. Check Supported Formats

```python
from extend_python.python_api.core.translators import CustomFooTranslator

translator = CustomFooTranslator()

# Get supported formats
formats = translator.get_supported_formats()
print(f"Supported formats: {formats}")

# Check if a specific file can be translated
can_translate = translator.can_translate_file("C:/Assets/MyFile.foo")
print(f"Can translate: {can_translate}")
```

### 3. Batch Import Multiple Files

```python
from extend_python.python_api.automation.manager import AutomationManager

manager = AutomationManager()

# List of files to import
files_to_import = [
    "C:/GameAssets/Character1.foo",
    "C:/GameAssets/Character2.foo",
    "C:/GameAssets/Environment1.foo"
]

# Batch import
results = manager.batch_import_files(
    file_paths=files_to_import,
    destination_path="/Game/BatchImported/"
)

print(f"Imported {results['successful']}/{results['total_files']} files")
```

## Setting Up Automation

### 1. Basic File Monitoring

```python
from extend_python.python_api.automation.manager import AutomationManager

# Create automation manager
manager = AutomationManager()

# Add folders to monitor
manager.add_watch_folder("D:/GameAssets/Characters/", "characters")
manager.add_watch_folder("D:/GameAssets/Environments/", "environments")

# Start monitoring
manager.start_monitoring()
print("🔄 File monitoring started!")

# Files added to monitored folders will be automatically imported
# Keep the script running or integrate into your workflow
```

### 2. Configure Import Rules

```python
from extend_python.python_api.automation.manager import AutomationManager

manager = AutomationManager()

# The manager automatically loads configuration from JSON
# You can modify the configuration programmatically:

# Add custom import rule for weapons
manager.config["import_rules"]["weapons"] = {
    "destination": "/Game/Weapons/",
    "translator_options": {
        "EnableDetailedLogging": "true",
        "ValidateInputFiles": "true"
    },
    "file_patterns": ["*weapon*", "*gun*", "*sword*"],
    "auto_import": True
}

# Save the updated configuration
manager.save_configuration()
print("✅ Configuration updated!")
```

## Common Workflows

### Workflow 1: Artist Asset Drop Folder

Set up a drop folder where artists can place assets for automatic import:

```python
from extend_python.python_api.automation.manager import AutomationManager

def setup_artist_drop_folder():
    manager = AutomationManager()
    
    # Configure drop folder
    drop_folder = "D:/ArtistDropFolder/"
    manager.add_watch_folder(drop_folder, "artist_assets")
    
    # Configure import rules for artist assets
    manager.config["import_rules"]["artist_assets"] = {
        "destination": "/Game/ArtistAssets/",
        "auto_import": True,
        "translator_options": {
            "EnableDetailedLogging": "true",
            "ValidateInputFiles": "true",
            "CreateBackups": "true"
        }
    }
    
    manager.save_configuration()
    manager.start_monitoring()
    
    print(f"🎨 Artist drop folder monitoring started: {drop_folder}")
    print("   Artists can now drop .foo files here for automatic import!")

setup_artist_drop_folder()
```

### Workflow 2: Daily Asset Sync

Set up a daily sync process for external asset repositories:

```python
import os
import glob
from extend_python.python_api.automation.manager import AutomationManager

def daily_asset_sync():
    manager = AutomationManager()
    
    # Define asset categories and their source folders
    asset_sources = {
        "characters": "//NetworkDrive/Assets/Characters/",
        "environments": "//NetworkDrive/Assets/Environments/",
        "props": "//NetworkDrive/Assets/Props/"
    }
    
    total_imported = 0
    
    for category, source_folder in asset_sources.items():
        if os.path.exists(source_folder):
            # Find all .foo files
            files = glob.glob(os.path.join(source_folder, "**/*.foo"), recursive=True)
            
            if files:
                print(f"📦 Syncing {len(files)} {category} files...")
                
                # Get destination from import rules
                destination = manager.config["import_rules"].get(category, {}).get("destination", f"/Game/{category.title()}/")
                
                # Batch import
                results = manager.batch_import_files(files, destination, category)
                total_imported += results['successful']
                
                print(f"   ✅ {results['successful']}/{results['total_files']} files imported")
    
    print(f"🎉 Daily sync completed! Total imported: {total_imported} files")

# Run daily sync
daily_asset_sync()
```

### Workflow 3: Quality Control Pipeline

Set up automated quality control for imported assets:

```python
from extend_python.python_api.automation.manager import AutomationManager

def setup_quality_control():
    manager = AutomationManager()
    
    # Configure strict quality checks
    manager.config["quality_checks"] = {
        "enabled": True,
        "max_file_size_mb": 50,  # Limit file size
        "min_file_size_bytes": 1024,  # Minimum file size
        "allowed_extensions": [".foo"],  # Only allow .foo files
        "validate_file_integrity": True
    }
    
    # Configure performance limits
    manager.config["performance"] = {
        "max_concurrent_imports": 2,  # Limit concurrent operations
        "processing_delay_seconds": 1,  # Add delay between operations
        "timeout_seconds": 120  # Timeout for long operations
    }
    
    manager.save_configuration()
    print("🔍 Quality control pipeline configured!")

setup_quality_control()
```

## Configuration Management

### Default Configuration File

The automation manager creates a default configuration file `interchange_automation_config.json`:

```json
{
  "watch_folders": [],
  "import_rules": {
    "characters": {
      "destination": "/Game/Characters/",
      "auto_import": true,
      "file_patterns": ["*character*", "*char*"]
    }
  },
  "quality_checks": {
    "enabled": true,
    "max_file_size_mb": 100
  }
}
```

### Customizing Configuration

```python
from extend_python.python_api.automation.manager import AutomationManager

# Load existing configuration
manager = AutomationManager()

# Modify configuration
manager.config["import_rules"]["vehicles"] = {
    "destination": "/Game/Vehicles/",
    "auto_import": True,
    "file_patterns": ["*car*", "*truck*", "*vehicle*"],
    "translator_options": {
        "EnableDetailedLogging": "true"
    }
}

# Save changes
manager.save_configuration()
```

## Monitoring and Statistics

### View Import Statistics

```python
from extend_python.python_api.automation.manager import AutomationManager

manager = AutomationManager()

# Get current statistics
stats = manager.get_statistics()

print("📊 Import Statistics:")
print(f"   Total processed: {stats['total_processed']}")
print(f"   Successful: {stats['successful_imports']}")
print(f"   Failed: {stats['failed_imports']}")
print(f"   Success rate: {stats['success_rate']:.1f}%")

if 'average_processing_time' in stats:
    print(f"   Average time: {stats['average_processing_time']:.2f}s")
```

### Generate Reports

```python
from extend_python.python_api.automation.manager import AutomationManager

manager = AutomationManager()

# Generate and save report
report = manager.generate_report("daily_report.txt")
print("📄 Report generated and saved!")
```

## Troubleshooting

### Common Issues

1. **Import fails silently**
   ```python
   # Enable detailed logging
   from extend_python.python_api.core.translators import CustomFooTranslator
   
   translator = CustomFooTranslator()
   translator.set_options({"EnableDetailedLogging": "true"})
   ```

2. **File monitoring not working**
   ```bash
   # Install watchdog
   pip install watchdog
   ```

3. **Configuration not saving**
   ```python
   # Check file permissions and path
   import os
   config_file = "interchange_automation_config.json"
   print(f"Config file exists: {os.path.exists(config_file)}")
   print(f"Config file writable: {os.access(config_file, os.W_OK)}")
   ```

### Getting Help

- Check the [API Reference](API_Reference.md) for detailed method documentation
- See [Advanced Usage](Advanced_Usage.md) for complex scenarios
- Review [Troubleshooting](Troubleshooting.md) for common problems

## Next Steps

1. **Explore Examples**: Check the `examples/` folder for more detailed usage patterns
2. **Customize Rules**: Modify import rules to match your project's asset organization
3. **Set Up Monitoring**: Configure file monitoring for your asset directories
4. **Integrate with Pipeline**: Incorporate the API into your existing asset pipeline

## Quick Reference

### Essential Commands

```python
# Basic import
from extend_python.python_api import InterchangePythonAPI
api = InterchangePythonAPI()
api.import_file("file.foo", "/Game/Assets/")

# Start monitoring
from extend_python.python_api.automation.manager import AutomationManager
manager = AutomationManager()
manager.add_watch_folder("D:/Assets/", "category")
manager.start_monitoring()

# Batch import
files = ["file1.foo", "file2.foo"]
results = manager.batch_import_files(files, "/Game/Batch/")

# Get statistics
stats = manager.get_statistics()
```

You're now ready to use the Interchange Python Extension! Start with simple imports and gradually add automation as needed.
