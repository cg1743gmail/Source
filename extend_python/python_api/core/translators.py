"""
Translator API module for Interchange Python Extension

This module provides Python wrappers for Interchange translators,
enabling file format translation and import functionality.
"""

import os
import json
from typing import List, Dict, Optional, Any

try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    unreal = None

class CustomFooTranslator:
    """
    Python wrapper for UCustomFooTranslatorExtended
    
    Provides Python-friendly access to custom file format translation
    functionality with enhanced error handling and logging.
    """
    
    def __init__(self):
        """Initialize the translator wrapper"""
        self._translator_instance = None
        self._statistics = {
            'total_translations': 0,
            'successful_translations': 0,
            'failed_translations': 0,
            'total_time': 0.0
        }
        
        if UNREAL_AVAILABLE:
            try:
                # Create the extended translator instance
                self._translator_instance = unreal.PythonTranslatorManager.create_translator()
            except Exception as e:
                print(f"Warning: Could not create translator instance: {e}")
                self._translator_instance = None
    
    def get_supported_formats(self) -> List[str]:
        """
        Get list of supported file formats
        
        Returns:
            List[str]: List of supported format strings
        """
        if not self._translator_instance:
            return ['.foo']  # Mock data for development
        
        try:
            return self._translator_instance.get_supported_formats_for_python()
        except Exception as e:
            print(f"Error getting supported formats: {e}")
            return []
    
    def can_translate_file(self, file_path: str) -> bool:
        """
        Check if a file can be translated by this translator
        
        Args:
            file_path (str): Path to the file to check
            
        Returns:
            bool: True if file can be translated
        """
        if not os.path.exists(file_path):
            return False
        
        if not self._translator_instance:
            # Mock check for development
            return file_path.lower().endswith('.foo')
        
        try:
            return self._translator_instance.can_translate_file(file_path)
        except Exception as e:
            print(f"Error checking file compatibility: {e}")
            return False
    
    def import_file(self, file_path: str, destination_path: str = "/Game/ImportedAssets/") -> bool:
        """
        Import a single file
        
        Args:
            file_path (str): Path to the source file
            destination_path (str): Destination path in UE content browser
            
        Returns:
            bool: True if import succeeded
        """
        import time
        start_time = time.time()
        
        # Update statistics
        self._statistics['total_translations'] += 1
        
        try:
            # Validate input
            if not os.path.exists(file_path):
                print(f"Error: File does not exist: {file_path}")
                self._statistics['failed_translations'] += 1
                return False
            
            if not self.can_translate_file(file_path):
                print(f"Error: File format not supported: {file_path}")
                self._statistics['failed_translations'] += 1
                return False
            
            if not self._translator_instance:
                print(f"Mock import: {file_path} -> {destination_path}")
                self._statistics['successful_translations'] += 1
                return True
            
            # Execute translation
            result = self._translator_instance.translate_file_from_python(file_path, destination_path)
            
            if result:
                self._statistics['successful_translations'] += 1
                print(f"Successfully imported: {file_path}")
            else:
                self._statistics['failed_translations'] += 1
                print(f"Failed to import: {file_path}")
            
            return result
            
        except Exception as e:
            print(f"Exception during import: {e}")
            self._statistics['failed_translations'] += 1
            return False
        
        finally:
            # Update timing statistics
            elapsed_time = time.time() - start_time
            self._statistics['total_time'] += elapsed_time
    
    def get_payload_data(self, payload_key: str) -> Optional[Dict[str, Any]]:
        """
        Get payload data for a given key
        
        Args:
            payload_key (str): The payload key
            
        Returns:
            Optional[Dict[str, Any]]: Payload data as dictionary, None if failed
        """
        if not self._translator_instance:
            return {"mock_data": "development_mode", "payload_key": payload_key}
        
        try:
            json_data = self._translator_instance.get_payload_data_as_json(payload_key)
            return json.loads(json_data) if json_data else None
        except Exception as e:
            print(f"Error getting payload data: {e}")
            return None
    
    def validate_file(self, file_path: str) -> tuple[bool, str]:
        """
        Validate a file for translation
        
        Args:
            file_path (str): Path to file to validate
            
        Returns:
            tuple[bool, str]: (is_valid, error_message)
        """
        if not self._translator_instance:
            # Mock validation for development
            if not os.path.exists(file_path):
                return False, f"File does not exist: {file_path}"
            if not file_path.lower().endswith('.foo'):
                return False, f"Unsupported file format: {os.path.splitext(file_path)[1]}"
            return True, ""
        
        try:
            error_message = ""
            is_valid = self._translator_instance.validate_file_for_translation(file_path, error_message)
            return is_valid, error_message
        except Exception as e:
            return False, f"Validation error: {e}"
    
    def get_translator_info(self) -> Dict[str, str]:
        """
        Get translator information
        
        Returns:
            Dict[str, str]: Translator metadata
        """
        if not self._translator_instance:
            return {
                "name": "Custom Foo Translator (Mock)",
                "version": "1.0.0",
                "supported_formats": ".foo",
                "python_enabled": "true"
            }
        
        try:
            return dict(self._translator_instance.get_translator_info())
        except Exception as e:
            print(f"Error getting translator info: {e}")
            return {}
    
    def set_options(self, options: Dict[str, str]) -> None:
        """
        Set translation options
        
        Args:
            options (Dict[str, str]): Option key-value pairs
        """
        if not self._translator_instance:
            print(f"Mock: Setting options: {options}")
            return
        
        try:
            # Convert Python dict to Unreal TMap
            unreal_map = unreal.Map(str, str)
            for key, value in options.items():
                unreal_map[key] = str(value)
            
            self._translator_instance.set_translation_options(unreal_map)
        except Exception as e:
            print(f"Error setting options: {e}")
    
    def get_options(self) -> Dict[str, str]:
        """
        Get current translation options
        
        Returns:
            Dict[str, str]: Current option key-value pairs
        """
        if not self._translator_instance:
            return {"mock_option": "development_mode"}
        
        try:
            unreal_map = self._translator_instance.get_translation_options()
            return dict(unreal_map)
        except Exception as e:
            print(f"Error getting options: {e}")
            return {}
    
    def reset_options(self) -> None:
        """Reset translation options to defaults"""
        if not self._translator_instance:
            print("Mock: Resetting options to defaults")
            return
        
        try:
            self._translator_instance.reset_translation_options()
        except Exception as e:
            print(f"Error resetting options: {e}")
    
    def get_statistics(self) -> Dict[str, Any]:
        """
        Get translation statistics
        
        Returns:
            Dict[str, Any]: Statistics data
        """
        if not self._translator_instance:
            return self._statistics
        
        try:
            unreal_stats = dict(self._translator_instance.get_translation_statistics())
            # Merge with local statistics
            return {**self._statistics, **unreal_stats}
        except Exception as e:
            print(f"Error getting statistics: {e}")
            return self._statistics
    
    def reset_statistics(self) -> None:
        """Reset statistics counters"""
        self._statistics = {
            'total_translations': 0,
            'successful_translations': 0,
            'failed_translations': 0,
            'total_time': 0.0
        }

class TranslatorManager:
    """
    Manager class for handling multiple translators
    """
    
    @staticmethod
    def get_available_translators() -> List[str]:
        """
        Get list of available translator types
        
        Returns:
            List[str]: List of translator class names
        """
        if not UNREAL_AVAILABLE:
            return ["CustomFooTranslator"]
        
        try:
            return unreal.PythonTranslatorManager.get_available_translator_types()
        except Exception as e:
            print(f"Error getting available translators: {e}")
            return []
    
    @staticmethod
    def find_best_translator(file_path: str) -> Optional[str]:
        """
        Find the best translator for a given file
        
        Args:
            file_path (str): Path to the file
            
        Returns:
            Optional[str]: Best translator class name, None if none found
        """
        if not UNREAL_AVAILABLE:
            return "CustomFooTranslator" if file_path.lower().endswith('.foo') else None
        
        try:
            result = unreal.PythonTranslatorManager.find_best_translator_for_file(file_path)
            return result if result else None
        except Exception as e:
            print(f"Error finding best translator: {e}")
            return None
    
    @staticmethod
    def batch_translate(file_paths: List[str], destination_path: str) -> Dict[str, Any]:
        """
        Batch translate multiple files
        
        Args:
            file_paths (List[str]): List of file paths to translate
            destination_path (str): Destination path for all files
            
        Returns:
            Dict[str, Any]: Batch translation results
        """
        if not UNREAL_AVAILABLE:
            return {
                "total_files": len(file_paths),
                "successful": len(file_paths),
                "failed": 0,
                "results": ["Success"] * len(file_paths)
            }
        
        try:
            results = []
            success_count = unreal.PythonTranslatorManager.batch_translate_files(
                file_paths, destination_path, results
            )
            
            return {
                "total_files": len(file_paths),
                "successful": success_count,
                "failed": len(file_paths) - success_count,
                "results": results
            }
        except Exception as e:
            print(f"Error in batch translation: {e}")
            return {
                "total_files": len(file_paths),
                "successful": 0,
                "failed": len(file_paths),
                "error": str(e)
            }
