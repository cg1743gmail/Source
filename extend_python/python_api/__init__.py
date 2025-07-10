"""
Unreal Engine 5.5 Interchange Plugin Python Extension

This package provides Python API access to the custom Interchange plugin,
enabling automated asset import workflows and advanced customization.

Main Components:
- core: Core API wrappers for translators, nodes, pipelines, and factories
- automation: Automated workflow tools and managers
- utils: Utility functions and helpers
- examples: Usage examples and sample scripts

Version: 1.0.0
Compatible with: Unreal Engine 5.5+
"""

__version__ = "1.0.0"
__author__ = "Interchange Plugin Python Extension Team"
__license__ = "Same as Unreal Engine"

# Import main classes for easy access
try:
    import unreal
    
    # Check if we're running in Unreal Engine
    if hasattr(unreal, 'InterchangeManager'):
        from .core.translators import CustomFooTranslator
        from .core.nodes import CustomDemoObjectNode
        from .core.pipelines import CustomDemoObjectPipeline
        from .core.factories import CustomDemoObjectFactory
        from .automation.manager import AutomationManager
        from .automation.monitor import FileMonitor
        from .automation.batch_processor import BatchProcessor
        from .utils.config import ConfigManager
        from .utils.logger import InterchangeLogger
        
        # Main API class
        class InterchangePythonAPI:
            """
            Main API class providing access to all Interchange Python functionality
            """
            
            def __init__(self):
                self.translator = CustomFooTranslator()
                self.node_manager = CustomDemoObjectNode
                self.pipeline = CustomDemoObjectPipeline()
                self.factory = CustomDemoObjectFactory()
                self.automation = AutomationManager()
                self.config = ConfigManager()
                self.logger = InterchangeLogger()
            
            def get_version(self):
                """Get the API version"""
                return __version__
            
            def get_supported_formats(self):
                """Get all supported file formats"""
                return self.translator.get_supported_formats()
            
            def import_file(self, file_path, destination_path="/Game/ImportedAssets/"):
                """
                Import a single file using the default translator
                
                Args:
                    file_path (str): Path to the source file
                    destination_path (str): Destination path in UE content browser
                    
                Returns:
                    bool: True if import succeeded
                """
                return self.translator.import_file(file_path, destination_path)
            
            def batch_import(self, file_paths, destination_path="/Game/ImportedAssets/"):
                """
                Import multiple files
                
                Args:
                    file_paths (list): List of file paths to import
                    destination_path (str): Destination path in UE content browser
                    
                Returns:
                    dict: Import results with statistics
                """
                return self.automation.batch_import_files(file_paths, destination_path)
            
            def start_monitoring(self, watch_folders):
                """
                Start file system monitoring for automatic imports
                
                Args:
                    watch_folders (list): List of folder paths to monitor
                """
                for folder in watch_folders:
                    self.automation.add_watch_folder(folder)
                self.automation.start_monitoring()
            
            def stop_monitoring(self):
                """Stop file system monitoring"""
                self.automation.stop_monitoring()
            
            def get_statistics(self):
                """Get import statistics"""
                return self.automation.get_statistics()
        
        # Export main classes
        __all__ = [
            'InterchangePythonAPI',
            'CustomFooTranslator',
            'CustomDemoObjectNode', 
            'CustomDemoObjectPipeline',
            'CustomDemoObjectFactory',
            'AutomationManager',
            'FileMonitor',
            'BatchProcessor',
            'ConfigManager',
            'InterchangeLogger'
        ]
        
    else:
        # Not running in Unreal Engine - provide mock classes for development
        print("Warning: Not running in Unreal Engine. Mock classes will be used.")
        
        class MockAPI:
            def __init__(self):
                print("Mock Interchange Python API initialized")
            
            def get_version(self):
                return __version__
        
        InterchangePythonAPI = MockAPI
        __all__ = ['InterchangePythonAPI']

except ImportError as e:
    print(f"Warning: Could not import Unreal Engine modules: {e}")
    print("This package requires Unreal Engine 5.5+ with Python support enabled.")
    
    # Provide a minimal interface for development outside UE
    class DevelopmentAPI:
        """Development-only API for testing outside Unreal Engine"""
        
        def __init__(self):
            self.version = __version__
            print(f"Interchange Python API {self.version} - Development Mode")
        
        def get_version(self):
            return self.version
        
        def import_file(self, file_path, destination_path="/Game/ImportedAssets/"):
            print(f"Mock import: {file_path} -> {destination_path}")
            return True
    
    InterchangePythonAPI = DevelopmentAPI
    __all__ = ['InterchangePythonAPI']

# Utility functions
def check_unreal_environment():
    """
    Check if we're running in a proper Unreal Engine environment
    
    Returns:
        bool: True if running in UE with proper Interchange support
    """
    try:
        import unreal
        return hasattr(unreal, 'InterchangeManager')
    except ImportError:
        return False

def get_api_info():
    """
    Get information about the API
    
    Returns:
        dict: API information
    """
    return {
        'version': __version__,
        'author': __author__,
        'license': __license__,
        'unreal_environment': check_unreal_environment(),
        'supported_features': [
            'File Import/Export',
            'Batch Processing', 
            'File System Monitoring',
            'Custom Pipelines',
            'Quality Checking',
            'Automation Workflows'
        ]
    }

def print_welcome():
    """Print welcome message with API information"""
    info = get_api_info()
    print("=" * 60)
    print("Unreal Engine 5.5 Interchange Python Extension")
    print("=" * 60)
    print(f"Version: {info['version']}")
    print(f"Author: {info['author']}")
    print(f"Unreal Environment: {'✓' if info['unreal_environment'] else '✗'}")
    print("\nSupported Features:")
    for feature in info['supported_features']:
        print(f"  • {feature}")
    print("=" * 60)
    
    if not info['unreal_environment']:
        print("⚠️  Warning: Not running in Unreal Engine environment")
        print("   Some features may not be available")
    else:
        print("✅ Ready to use!")
    print()

# Auto-print welcome message when imported
if __name__ != "__main__":
    import os
    if os.environ.get('INTERCHANGE_PYTHON_QUIET', '').lower() not in ('1', 'true', 'yes'):
        print_welcome()

# Version check function
def require_version(min_version):
    """
    Check if the current version meets minimum requirements
    
    Args:
        min_version (str): Minimum required version (e.g., "1.0.0")
        
    Raises:
        RuntimeError: If version requirement is not met
    """
    from packaging import version
    
    if version.parse(__version__) < version.parse(min_version):
        raise RuntimeError(f"Interchange Python API version {__version__} is below required minimum {min_version}")

# Configuration
class APIConfig:
    """Global API configuration"""
    
    # Default settings
    DEFAULT_DESTINATION_PATH = "/Game/ImportedAssets/"
    DEFAULT_BATCH_SIZE = 10
    DEFAULT_TIMEOUT = 30.0
    ENABLE_DETAILED_LOGGING = True
    ENABLE_STATISTICS = True
    
    # File size limits (in MB)
    MAX_FILE_SIZE_MB = 500
    WARN_FILE_SIZE_MB = 100
    
    # Supported file extensions
    SUPPORTED_EXTENSIONS = ['.foo', '.bar', '.custom']
    
    @classmethod
    def get_config(cls):
        """Get current configuration as dictionary"""
        return {
            'default_destination_path': cls.DEFAULT_DESTINATION_PATH,
            'default_batch_size': cls.DEFAULT_BATCH_SIZE,
            'default_timeout': cls.DEFAULT_TIMEOUT,
            'enable_detailed_logging': cls.ENABLE_DETAILED_LOGGING,
            'enable_statistics': cls.ENABLE_STATISTICS,
            'max_file_size_mb': cls.MAX_FILE_SIZE_MB,
            'warn_file_size_mb': cls.WARN_FILE_SIZE_MB,
            'supported_extensions': cls.SUPPORTED_EXTENSIONS
        }
    
    @classmethod
    def update_config(cls, **kwargs):
        """Update configuration values"""
        for key, value in kwargs.items():
            if hasattr(cls, key.upper()):
                setattr(cls, key.upper(), value)

# Export configuration
__all__.append('APIConfig')
