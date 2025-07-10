# Interchange Python Extension API Reference

## Overview

This document provides comprehensive API reference for the Unreal Engine 5.5 Interchange Python Extension. The API is organized into several modules, each providing specific functionality for asset import automation.

## Module Structure

```
python_api/
├── core/                    # Core API components
│   ├── translators.py      # File format translators
│   ├── nodes.py            # Data nodes
│   ├── pipelines.py        # Processing pipelines
│   └── factories.py        # Asset factories
├── automation/             # Automation tools
│   ├── manager.py          # Automation manager
│   ├── monitor.py          # File monitoring
│   ├── batch_processor.py  # Batch processing
│   └── quality_checker.py  # Quality control
└── utils/                  # Utility functions
    ├── config.py           # Configuration management
    ├── logger.py           # Logging utilities
    └── helpers.py          # Helper functions
```

## Core API Classes

### InterchangePythonAPI

Main API class providing unified access to all functionality.

```python
from python_api import InterchangePythonAPI

api = InterchangePythonAPI()
```

#### Methods

##### `get_version() -> str`
Returns the API version string.

##### `get_supported_formats() -> List[str]`
Returns list of supported file formats.

##### `import_file(file_path: str, destination_path: str = "/Game/ImportedAssets/") -> bool`
Import a single file.

**Parameters:**
- `file_path` (str): Path to the source file
- `destination_path` (str): Destination path in UE content browser

**Returns:**
- `bool`: True if import succeeded

##### `batch_import(file_paths: List[str], destination_path: str) -> Dict[str, Any]`
Import multiple files in batch.

**Parameters:**
- `file_paths` (List[str]): List of file paths to import
- `destination_path` (str): Destination path in UE content browser

**Returns:**
- `Dict[str, Any]`: Import results with statistics

##### `start_monitoring(watch_folders: List[str]) -> None`
Start file system monitoring for automatic imports.

**Parameters:**
- `watch_folders` (List[str]): List of folder paths to monitor

##### `stop_monitoring() -> None`
Stop file system monitoring.

##### `get_statistics() -> Dict[str, Any]`
Get comprehensive import statistics.

## Core Module APIs

### CustomFooTranslator

Python wrapper for the custom file format translator.

```python
from python_api.core.translators import CustomFooTranslator

translator = CustomFooTranslator()
```

#### Methods

##### `get_supported_formats() -> List[str]`
Get list of supported file formats.

##### `can_translate_file(file_path: str) -> bool`
Check if a file can be translated.

**Parameters:**
- `file_path` (str): Path to the file to check

**Returns:**
- `bool`: True if file can be translated

##### `import_file(file_path: str, destination_path: str) -> bool`
Import a single file.

**Parameters:**
- `file_path` (str): Path to the source file
- `destination_path` (str): Destination path in UE content browser

**Returns:**
- `bool`: True if import succeeded

##### `validate_file(file_path: str) -> Tuple[bool, str]`
Validate a file for translation.

**Parameters:**
- `file_path` (str): Path to file to validate

**Returns:**
- `Tuple[bool, str]`: (is_valid, error_message)

##### `get_payload_data(payload_key: str) -> Optional[Dict[str, Any]]`
Get payload data for a given key.

**Parameters:**
- `payload_key` (str): The payload key

**Returns:**
- `Optional[Dict[str, Any]]`: Payload data as dictionary, None if failed

##### `set_options(options: Dict[str, str]) -> None`
Set translation options.

**Parameters:**
- `options` (Dict[str, str]): Option key-value pairs

##### `get_options() -> Dict[str, str]`
Get current translation options.

**Returns:**
- `Dict[str, str]`: Current option key-value pairs

##### `reset_options() -> None`
Reset translation options to defaults.

##### `get_statistics() -> Dict[str, Any]`
Get translation statistics.

**Returns:**
- `Dict[str, Any]`: Statistics data

##### `get_translator_info() -> Dict[str, str]`
Get translator information.

**Returns:**
- `Dict[str, str]`: Translator metadata

### TranslatorManager

Static utility class for managing multiple translators.

```python
from python_api.core.translators import TranslatorManager
```

#### Static Methods

##### `get_available_translators() -> List[str]`
Get list of available translator types.

##### `find_best_translator(file_path: str) -> Optional[str]`
Find the best translator for a given file.

**Parameters:**
- `file_path` (str): Path to the file

**Returns:**
- `Optional[str]`: Best translator class name, None if none found

##### `batch_translate(file_paths: List[str], destination_path: str) -> Dict[str, Any]`
Batch translate multiple files.

**Parameters:**
- `file_paths` (List[str]): List of file paths to translate
- `destination_path` (str): Destination path for all files

**Returns:**
- `Dict[str, Any]`: Batch translation results

## Automation Module APIs

### AutomationManager

Comprehensive automation manager for Interchange workflows.

```python
from python_api.automation.manager import AutomationManager

manager = AutomationManager()
```

#### Methods

##### `add_watch_folder(folder_path: str, category: str = "default", enabled: bool = True) -> bool`
Add a folder to the monitoring list.

**Parameters:**
- `folder_path` (str): Path to the folder to monitor
- `category` (str): Category for import rules
- `enabled` (bool): Whether monitoring is enabled for this folder

**Returns:**
- `bool`: True if folder was added successfully

##### `remove_watch_folder(folder_path: str) -> None`
Remove a folder from monitoring.

**Parameters:**
- `folder_path` (str): Path to the folder to remove

##### `start_monitoring() -> bool`
Start file system monitoring for all configured folders.

**Returns:**
- `bool`: True if monitoring started successfully

##### `stop_monitoring() -> None`
Stop all file system monitoring.

##### `process_single_file(file_path: str, category: str, event_type: str = "manual") -> bool`
Process a single file according to automation rules.

**Parameters:**
- `file_path` (str): Path to the file to process
- `category` (str): Category for determining import rules
- `event_type` (str): Type of event that triggered processing

**Returns:**
- `bool`: True if processing succeeded

##### `batch_import_files(file_paths: List[str], destination_path: str, category: str = "batch") -> Dict[str, Any]`
Import multiple files in batch.

**Parameters:**
- `file_paths` (List[str]): List of file paths to import
- `destination_path` (str): Destination path for all files
- `category` (str): Category for processing rules

**Returns:**
- `Dict[str, Any]`: Batch import results

##### `get_statistics() -> Dict[str, Any]`
Get comprehensive automation statistics.

**Returns:**
- `Dict[str, Any]`: Statistics data including:
  - `total_processed`: Total files processed
  - `successful_imports`: Number of successful imports
  - `failed_imports`: Number of failed imports
  - `success_rate`: Success rate percentage
  - `average_processing_time`: Average processing time
  - `uptime_seconds`: System uptime in seconds
  - `files_per_category`: Files processed by category

##### `reset_statistics() -> None`
Reset all statistics counters.

##### `generate_report(output_file: Optional[str] = None) -> str`
Generate a comprehensive automation report.

**Parameters:**
- `output_file` (Optional[str]): File path to save the report

**Returns:**
- `str`: Report content

##### `load_configuration() -> None`
Load automation configuration from file.

##### `save_configuration() -> None`
Save current configuration to file.

## Configuration Structure

### Automation Configuration

The automation manager uses a JSON configuration file with the following structure:

```json
{
  "watch_folders": [
    {
      "path": "/path/to/watch",
      "category": "characters",
      "enabled": true,
      "added_time": "2024-01-01T00:00:00"
    }
  ],
  "import_rules": {
    "characters": {
      "destination": "/Game/Characters/",
      "translator_options": {
        "EnableDetailedLogging": "true",
        "ValidateInputFiles": "true"
      },
      "file_patterns": ["*character*", "*char*"],
      "auto_import": true
    }
  },
  "quality_checks": {
    "enabled": true,
    "max_file_size_mb": 100,
    "min_file_size_bytes": 1024,
    "allowed_extensions": [".foo", ".bar"]
  },
  "performance": {
    "max_concurrent_imports": 3,
    "processing_delay_seconds": 2,
    "batch_size": 10,
    "timeout_seconds": 300
  }
}
```

### Import Rules

Each category in `import_rules` can have the following properties:

- `destination` (str): Target path in UE content browser
- `translator_options` (Dict[str, str]): Translator-specific options
- `file_patterns` (List[str]): File name patterns to match
- `auto_import` (bool): Whether to automatically import matching files
- `quality_checks` (Dict): Category-specific quality check overrides

### Quality Checks

Quality check configuration includes:

- `enabled` (bool): Whether quality checks are enabled
- `max_file_size_mb` (int): Maximum file size in megabytes
- `min_file_size_bytes` (int): Minimum file size in bytes
- `allowed_extensions` (List[str]): List of allowed file extensions
- `scan_for_viruses` (bool): Whether to scan for viruses (if available)

### Performance Settings

Performance configuration includes:

- `max_concurrent_imports` (int): Maximum number of concurrent import operations
- `processing_delay_seconds` (float): Delay between processing operations
- `batch_size` (int): Number of files to process in each batch
- `timeout_seconds` (int): Timeout for individual import operations

## Error Handling

All API methods include comprehensive error handling and return appropriate error information:

### Common Return Patterns

- **Boolean methods**: Return `False` on failure, with error details logged
- **Data methods**: Return `None` or empty collections on failure
- **Validation methods**: Return tuple `(False, error_message)` on failure

### Exception Handling

The API catches and handles common exceptions:

- `FileNotFoundError`: When source files don't exist
- `PermissionError`: When file access is denied
- `ValueError`: When invalid parameters are provided
- `RuntimeError`: When Unreal Engine operations fail

### Logging

All operations are logged using the integrated logging system:

```python
from python_api.utils.logger import InterchangeLogger

logger = InterchangeLogger()
logger.info("Operation completed successfully")
logger.warning("Potential issue detected")
logger.error("Operation failed")
```

## Usage Examples

### Basic Import

```python
from python_api import InterchangePythonAPI

api = InterchangePythonAPI()
success = api.import_file("C:/Assets/MyFile.foo", "/Game/ImportedAssets/")
```

### Batch Processing

```python
from python_api.automation.manager import AutomationManager

manager = AutomationManager()
files = ["file1.foo", "file2.foo", "file3.foo"]
results = manager.batch_import_files(files, "/Game/BatchImported/")
```

### File Monitoring

```python
from python_api.automation.manager import AutomationManager

manager = AutomationManager()
manager.add_watch_folder("D:/GameAssets/Characters/", "characters")
manager.start_monitoring()
```

### Custom Configuration

```python
from python_api.core.translators import CustomFooTranslator

translator = CustomFooTranslator()
translator.set_options({
    "EnableDetailedLogging": "true",
    "ValidateInputFiles": "true",
    "CreateBackups": "false"
})
```

## Version Compatibility

- **Unreal Engine**: 5.5+
- **Python**: 3.7+
- **Required Packages**: 
  - `watchdog` (for file monitoring)
  - `packaging` (for version checking)

## Support and Troubleshooting

For detailed troubleshooting information, see the [Troubleshooting Guide](Troubleshooting.md).

For advanced usage patterns, see the [Advanced Usage Guide](Advanced_Usage.md).
