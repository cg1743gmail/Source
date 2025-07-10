"""
Automation Manager for Interchange Python Extension

This module provides comprehensive automation capabilities for asset import workflows,
including file monitoring, batch processing, and intelligent import management.
"""

import os
import json
import time
import threading
from datetime import datetime
from typing import List, Dict, Any, Optional, Callable
from pathlib import Path

try:
    from watchdog.observers import Observer
    from watchdog.events import FileSystemEventHandler
    WATCHDOG_AVAILABLE = True
except ImportError:
    WATCHDOG_AVAILABLE = False
    print("Warning: watchdog not available. File monitoring will be disabled.")

from ..core.translators import CustomFooTranslator, TranslatorManager
from ..utils.config import ConfigManager
from ..utils.logger import InterchangeLogger

class FileEventHandler(FileSystemEventHandler):
    """File system event handler for automatic import processing"""
    
    def __init__(self, automation_manager, category: str):
        self.automation_manager = automation_manager
        self.category = category
        self.processing_queue = []
        self.processing_lock = threading.Lock()
        self.logger = InterchangeLogger()
    
    def on_created(self, event):
        """Handle file creation events"""
        if not event.is_directory:
            self._queue_file_for_processing(event.src_path, "created")
    
    def on_modified(self, event):
        """Handle file modification events"""
        if not event.is_directory:
            self._queue_file_for_processing(event.src_path, "modified")
    
    def _queue_file_for_processing(self, file_path: str, event_type: str):
        """Queue a file for processing"""
        with self.processing_lock:
            # Avoid duplicate entries
            if file_path not in [item['path'] for item in self.processing_queue]:
                self.processing_queue.append({
                    'path': file_path,
                    'event_type': event_type,
                    'timestamp': datetime.now(),
                    'category': self.category
                })
                
                self.logger.info(f"Queued for processing: {file_path} ({event_type})")
                
                # Start processing thread
                threading.Thread(target=self._process_queue, daemon=True).start()
    
    def _process_queue(self):
        """Process queued files"""
        time.sleep(2)  # Wait for file operations to complete
        
        with self.processing_lock:
            if not self.processing_queue:
                return
            
            items_to_process = self.processing_queue.copy()
            self.processing_queue.clear()
        
        for item in items_to_process:
            try:
                self.automation_manager.process_single_file(
                    item['path'], 
                    item['category'],
                    item['event_type']
                )
            except Exception as e:
                self.logger.error(f"Error processing {item['path']}: {e}")

class AutomationManager:
    """
    Main automation manager for Interchange workflows
    
    Provides comprehensive automation capabilities including:
    - File system monitoring
    - Batch processing
    - Rule-based import management
    - Statistics and reporting
    """
    
    def __init__(self, config_file: str = "interchange_automation_config.json"):
        self.config_file = config_file
        self.config_manager = ConfigManager(config_file)
        self.logger = InterchangeLogger()
        self.translator = CustomFooTranslator()
        
        # Monitoring state
        self.observers = {}
        self.is_monitoring = False
        self.monitoring_lock = threading.Lock()
        
        # Statistics
        self.statistics = {
            'total_processed': 0,
            'successful_imports': 0,
            'failed_imports': 0,
            'start_time': None,
            'last_activity': None,
            'files_per_category': {},
            'processing_times': []
        }
        
        # Load configuration
        self.load_configuration()
    
    def load_configuration(self):
        """Load automation configuration"""
        try:
            self.config = self.config_manager.load_config()
            self.logger.info("Automation configuration loaded successfully")
        except Exception as e:
            self.logger.error(f"Error loading configuration: {e}")
            self.config = self._get_default_config()
            self.save_configuration()
    
    def save_configuration(self):
        """Save current configuration"""
        try:
            self.config_manager.save_config(self.config)
            self.logger.info("Automation configuration saved")
        except Exception as e:
            self.logger.error(f"Error saving configuration: {e}")
    
    def _get_default_config(self) -> Dict[str, Any]:
        """Get default automation configuration"""
        return {
            "watch_folders": [],
            "import_rules": {
                "characters": {
                    "destination": "/Game/Characters/",
                    "translator_options": {
                        "EnableDetailedLogging": "true",
                        "ValidateInputFiles": "true"
                    },
                    "file_patterns": ["*character*", "*char*"],
                    "auto_import": True
                },
                "environments": {
                    "destination": "/Game/Environments/",
                    "translator_options": {
                        "EnableDetailedLogging": "true",
                        "ValidateInputFiles": "true"
                    },
                    "file_patterns": ["*env*", "*environment*", "*level*"],
                    "auto_import": True
                },
                "props": {
                    "destination": "/Game/Props/",
                    "translator_options": {
                        "EnableDetailedLogging": "true"
                    },
                    "file_patterns": ["*prop*", "*object*"],
                    "auto_import": True
                }
            },
            "quality_checks": {
                "enabled": True,
                "max_file_size_mb": 100,
                "min_file_size_bytes": 1024,
                "allowed_extensions": [".foo", ".bar"],
                "scan_for_viruses": False
            },
            "performance": {
                "max_concurrent_imports": 3,
                "processing_delay_seconds": 2,
                "batch_size": 10,
                "timeout_seconds": 300
            }
        }
    
    def add_watch_folder(self, folder_path: str, category: str = "default", enabled: bool = True):
        """
        Add a folder to the monitoring list
        
        Args:
            folder_path (str): Path to the folder to monitor
            category (str): Category for import rules
            enabled (bool): Whether monitoring is enabled for this folder
        """
        if not os.path.exists(folder_path):
            self.logger.warning(f"Watch folder does not exist: {folder_path}")
            return False
        
        watch_entry = {
            "path": os.path.abspath(folder_path),
            "category": category,
            "enabled": enabled,
            "added_time": datetime.now().isoformat()
        }
        
        # Check if already exists
        existing_folders = [w['path'] for w in self.config.get('watch_folders', [])]
        if watch_entry['path'] not in existing_folders:
            self.config.setdefault('watch_folders', []).append(watch_entry)
            self.save_configuration()
            self.logger.info(f"Added watch folder: {folder_path} (category: {category})")
            
            # Start monitoring if already running
            if self.is_monitoring:
                self._start_folder_monitoring(watch_entry)
            
            return True
        else:
            self.logger.warning(f"Folder already being monitored: {folder_path}")
            return False
    
    def remove_watch_folder(self, folder_path: str):
        """
        Remove a folder from monitoring
        
        Args:
            folder_path (str): Path to the folder to remove
        """
        abs_path = os.path.abspath(folder_path)
        
        # Remove from configuration
        watch_folders = self.config.get('watch_folders', [])
        self.config['watch_folders'] = [w for w in watch_folders if w['path'] != abs_path]
        self.save_configuration()
        
        # Stop monitoring
        if abs_path in self.observers:
            self.observers[abs_path].stop()
            self.observers[abs_path].join()
            del self.observers[abs_path]
            self.logger.info(f"Removed watch folder: {folder_path}")
    
    def start_monitoring(self):
        """Start file system monitoring for all configured folders"""
        if not WATCHDOG_AVAILABLE:
            self.logger.error("File monitoring not available - watchdog package not installed")
            return False
        
        with self.monitoring_lock:
            if self.is_monitoring:
                self.logger.warning("Monitoring is already running")
                return True
            
            self.statistics['start_time'] = datetime.now()
            
            # Start monitoring each configured folder
            for watch_entry in self.config.get('watch_folders', []):
                if watch_entry.get('enabled', True):
                    self._start_folder_monitoring(watch_entry)
            
            self.is_monitoring = True
            self.logger.info(f"Started monitoring {len(self.observers)} folders")
            return True
    
    def stop_monitoring(self):
        """Stop all file system monitoring"""
        with self.monitoring_lock:
            if not self.is_monitoring:
                self.logger.warning("Monitoring is not running")
                return
            
            # Stop all observers
            for observer in self.observers.values():
                observer.stop()
                observer.join()
            
            self.observers.clear()
            self.is_monitoring = False
            self.logger.info("Stopped all monitoring")
    
    def _start_folder_monitoring(self, watch_entry: Dict[str, Any]):
        """Start monitoring a specific folder"""
        folder_path = watch_entry['path']
        category = watch_entry['category']
        
        if not os.path.exists(folder_path):
            self.logger.error(f"Cannot monitor non-existent folder: {folder_path}")
            return
        
        try:
            handler = FileEventHandler(self, category)
            observer = Observer()
            observer.schedule(handler, folder_path, recursive=True)
            observer.start()
            
            self.observers[folder_path] = observer
            self.logger.info(f"Started monitoring: {folder_path} (category: {category})")
            
        except Exception as e:
            self.logger.error(f"Error starting monitoring for {folder_path}: {e}")
    
    def process_single_file(self, file_path: str, category: str, event_type: str = "manual"):
        """
        Process a single file according to automation rules
        
        Args:
            file_path (str): Path to the file to process
            category (str): Category for determining import rules
            event_type (str): Type of event that triggered processing
        """
        start_time = time.time()
        
        try:
            # Update statistics
            self.statistics['total_processed'] += 1
            self.statistics['last_activity'] = datetime.now()
            self.statistics['files_per_category'][category] = self.statistics['files_per_category'].get(category, 0) + 1
            
            # Get import rules for category
            import_rules = self.config.get('import_rules', {}).get(category)
            if not import_rules:
                self.logger.warning(f"No import rules found for category: {category}")
                return False
            
            # Check if auto-import is enabled
            if not import_rules.get('auto_import', True):
                self.logger.info(f"Auto-import disabled for category: {category}")
                return False
            
            # Perform quality checks
            if not self._perform_quality_checks(file_path):
                self.statistics['failed_imports'] += 1
                return False
            
            # Check file patterns
            if not self._matches_file_patterns(file_path, import_rules.get('file_patterns', [])):
                self.logger.info(f"File does not match patterns for category {category}: {file_path}")
                return False
            
            # Configure translator
            translator_options = import_rules.get('translator_options', {})
            if translator_options:
                self.translator.set_options(translator_options)
            
            # Execute import
            destination = import_rules.get('destination', '/Game/ImportedAssets/')
            success = self.translator.import_file(file_path, destination)
            
            if success:
                self.statistics['successful_imports'] += 1
                self.logger.info(f"Successfully processed: {file_path} -> {destination}")
            else:
                self.statistics['failed_imports'] += 1
                self.logger.error(f"Failed to process: {file_path}")
            
            return success
            
        except Exception as e:
            self.statistics['failed_imports'] += 1
            self.logger.error(f"Exception processing {file_path}: {e}")
            return False
        
        finally:
            # Record processing time
            processing_time = time.time() - start_time
            self.statistics['processing_times'].append(processing_time)
            
            # Keep only last 100 processing times
            if len(self.statistics['processing_times']) > 100:
                self.statistics['processing_times'] = self.statistics['processing_times'][-100:]
    
    def _perform_quality_checks(self, file_path: str) -> bool:
        """Perform quality checks on a file"""
        quality_config = self.config.get('quality_checks', {})
        
        if not quality_config.get('enabled', True):
            return True
        
        # Check file existence
        if not os.path.exists(file_path):
            self.logger.error(f"File does not exist: {file_path}")
            return False
        
        # Check file size
        file_size = os.path.getsize(file_path)
        max_size = quality_config.get('max_file_size_mb', 100) * 1024 * 1024
        min_size = quality_config.get('min_file_size_bytes', 1024)
        
        if file_size > max_size:
            self.logger.error(f"File too large: {file_path} ({file_size} bytes > {max_size} bytes)")
            return False
        
        if file_size < min_size:
            self.logger.error(f"File too small: {file_path} ({file_size} bytes < {min_size} bytes)")
            return False
        
        # Check file extension
        allowed_extensions = quality_config.get('allowed_extensions', [])
        if allowed_extensions:
            file_ext = os.path.splitext(file_path)[1].lower()
            if file_ext not in allowed_extensions:
                self.logger.warning(f"File extension not in allowed list: {file_ext}")
                return False
        
        # Validate with translator
        is_valid, error_message = self.translator.validate_file(file_path)
        if not is_valid:
            self.logger.error(f"File validation failed: {error_message}")
            return False
        
        return True
    
    def _matches_file_patterns(self, file_path: str, patterns: List[str]) -> bool:
        """Check if file matches any of the specified patterns"""
        if not patterns:
            return True  # No patterns means accept all
        
        import fnmatch
        file_name = os.path.basename(file_path).lower()
        
        for pattern in patterns:
            if fnmatch.fnmatch(file_name, pattern.lower()):
                return True
        
        return False
    
    def batch_import_files(self, file_paths: List[str], destination_path: str, category: str = "batch") -> Dict[str, Any]:
        """
        Import multiple files in batch
        
        Args:
            file_paths (List[str]): List of file paths to import
            destination_path (str): Destination path for all files
            category (str): Category for processing rules
            
        Returns:
            Dict[str, Any]: Batch import results
        """
        start_time = time.time()
        results = {
            'total_files': len(file_paths),
            'successful': 0,
            'failed': 0,
            'start_time': datetime.now().isoformat(),
            'details': []
        }
        
        self.logger.info(f"Starting batch import of {len(file_paths)} files")
        
        for i, file_path in enumerate(file_paths):
            try:
                success = self.translator.import_file(file_path, destination_path)
                
                if success:
                    results['successful'] += 1
                    results['details'].append({
                        'file': file_path,
                        'status': 'success',
                        'destination': destination_path
                    })
                else:
                    results['failed'] += 1
                    results['details'].append({
                        'file': file_path,
                        'status': 'failed',
                        'error': 'Import failed'
                    })
                
                # Progress logging
                if (i + 1) % 10 == 0 or (i + 1) == len(file_paths):
                    progress = ((i + 1) / len(file_paths)) * 100
                    self.logger.info(f"Batch import progress: {progress:.1f}% ({i + 1}/{len(file_paths)})")
                
            except Exception as e:
                results['failed'] += 1
                results['details'].append({
                    'file': file_path,
                    'status': 'error',
                    'error': str(e)
                })
        
        # Finalize results
        results['end_time'] = datetime.now().isoformat()
        results['duration_seconds'] = time.time() - start_time
        results['success_rate'] = (results['successful'] / results['total_files']) * 100 if results['total_files'] > 0 else 0
        
        self.logger.info(f"Batch import completed: {results['successful']}/{results['total_files']} successful ({results['success_rate']:.1f}%)")
        
        return results
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get comprehensive automation statistics"""
        stats = self.statistics.copy()
        
        # Calculate additional metrics
        if stats['processing_times']:
            stats['average_processing_time'] = sum(stats['processing_times']) / len(stats['processing_times'])
            stats['min_processing_time'] = min(stats['processing_times'])
            stats['max_processing_time'] = max(stats['processing_times'])
        
        if stats['total_processed'] > 0:
            stats['success_rate'] = (stats['successful_imports'] / stats['total_processed']) * 100
        else:
            stats['success_rate'] = 0
        
        if stats['start_time']:
            uptime = datetime.now() - stats['start_time']
            stats['uptime_seconds'] = uptime.total_seconds()
            stats['uptime_formatted'] = str(uptime)
        
        # Add configuration info
        stats['monitoring_enabled'] = self.is_monitoring
        stats['watched_folders'] = len(self.config.get('watch_folders', []))
        stats['import_rules'] = len(self.config.get('import_rules', {}))
        
        return stats
    
    def reset_statistics(self):
        """Reset all statistics counters"""
        self.statistics = {
            'total_processed': 0,
            'successful_imports': 0,
            'failed_imports': 0,
            'start_time': datetime.now() if self.is_monitoring else None,
            'last_activity': None,
            'files_per_category': {},
            'processing_times': []
        }
        self.logger.info("Statistics reset")
    
    def generate_report(self, output_file: Optional[str] = None) -> str:
        """
        Generate a comprehensive automation report
        
        Args:
            output_file (Optional[str]): File path to save the report
            
        Returns:
            str: Report content
        """
        stats = self.get_statistics()
        
        report_lines = []
        report_lines.append("=" * 60)
        report_lines.append("INTERCHANGE AUTOMATION REPORT")
        report_lines.append("=" * 60)
        report_lines.append(f"Generated: {datetime.now().isoformat()}")
        report_lines.append("")
        
        # System status
        report_lines.append("SYSTEM STATUS:")
        report_lines.append(f"  Monitoring Active: {'Yes' if stats['monitoring_enabled'] else 'No'}")
        report_lines.append(f"  Watched Folders: {stats['watched_folders']}")
        report_lines.append(f"  Import Rules: {stats['import_rules']}")
        if stats.get('uptime_formatted'):
            report_lines.append(f"  Uptime: {stats['uptime_formatted']}")
        report_lines.append("")
        
        # Processing statistics
        report_lines.append("PROCESSING STATISTICS:")
        report_lines.append(f"  Total Processed: {stats['total_processed']}")
        report_lines.append(f"  Successful: {stats['successful_imports']}")
        report_lines.append(f"  Failed: {stats['failed_imports']}")
        report_lines.append(f"  Success Rate: {stats['success_rate']:.1f}%")
        report_lines.append("")
        
        # Performance metrics
        if stats.get('average_processing_time'):
            report_lines.append("PERFORMANCE METRICS:")
            report_lines.append(f"  Average Processing Time: {stats['average_processing_time']:.3f}s")
            report_lines.append(f"  Min Processing Time: {stats['min_processing_time']:.3f}s")
            report_lines.append(f"  Max Processing Time: {stats['max_processing_time']:.3f}s")
            report_lines.append("")
        
        # Category breakdown
        if stats['files_per_category']:
            report_lines.append("FILES BY CATEGORY:")
            for category, count in stats['files_per_category'].items():
                report_lines.append(f"  {category}: {count}")
            report_lines.append("")
        
        # Configuration summary
        report_lines.append("CONFIGURATION:")
        for folder in self.config.get('watch_folders', []):
            status = "Enabled" if folder.get('enabled', True) else "Disabled"
            report_lines.append(f"  {folder['path']} ({folder['category']}) - {status}")
        
        report_lines.append("=" * 60)
        
        report_content = "\n".join(report_lines)
        
        # Save to file if requested
        if output_file:
            try:
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(report_content)
                self.logger.info(f"Report saved to: {output_file}")
            except Exception as e:
                self.logger.error(f"Error saving report: {e}")
        
        return report_content
