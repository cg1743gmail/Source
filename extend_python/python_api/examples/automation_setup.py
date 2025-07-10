"""
Automation Setup Example for Interchange Python Extension

This example demonstrates how to set up comprehensive automation workflows
including file monitoring, batch processing, and rule-based import management.
"""

import os
import sys
import time
from datetime import datetime

# Add the Python API to the path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))

from python_api.automation.manager import AutomationManager
from python_api.core.translators import TranslatorManager

def setup_basic_automation():
    """Set up basic automation with file monitoring"""
    print("=== Basic Automation Setup ===")
    
    # Create automation manager
    manager = AutomationManager()
    
    # Add watch folders for different asset types
    watch_folders = [
        {"path": "D:/GameAssets/Characters/", "category": "characters"},
        {"path": "D:/GameAssets/Environments/", "category": "environments"},
        {"path": "D:/GameAssets/Props/", "category": "props"}
    ]
    
    print("Adding watch folders...")
    for folder_info in watch_folders:
        folder_path = folder_info["path"]
        category = folder_info["category"]
        
        # Create folder if it doesn't exist (for demo purposes)
        if not os.path.exists(folder_path):
            print(f"⚠️ Folder doesn't exist: {folder_path}")
            print(f"   You can create it or update the path in the script")
        else:
            success = manager.add_watch_folder(folder_path, category)
            if success:
                print(f"✅ Added watch folder: {folder_path} ({category})")
            else:
                print(f"❌ Failed to add watch folder: {folder_path}")
    
    # Display current configuration
    config = manager.config
    print(f"\nCurrent configuration:")
    print(f"  Watch folders: {len(config.get('watch_folders', []))}")
    print(f"  Import rules: {len(config.get('import_rules', {}))}")
    
    return manager

def configure_import_rules():
    """Configure detailed import rules for different asset categories"""
    print("\n=== Configuring Import Rules ===")
    
    manager = AutomationManager()
    
    # Define custom import rules
    custom_rules = {
        "characters": {
            "destination": "/Game/Characters/",
            "translator_options": {
                "EnableDetailedLogging": "true",
                "ValidateInputFiles": "true",
                "CreateBackups": "true"
            },
            "file_patterns": ["*character*", "*char*", "*hero*", "*npc*"],
            "auto_import": True,
            "quality_checks": {
                "max_file_size_mb": 50,
                "required_keywords": ["character", "char"]
            }
        },
        "environments": {
            "destination": "/Game/Environments/",
            "translator_options": {
                "EnableDetailedLogging": "true",
                "ValidateInputFiles": "true"
            },
            "file_patterns": ["*env*", "*environment*", "*level*", "*scene*"],
            "auto_import": True,
            "quality_checks": {
                "max_file_size_mb": 100
            }
        },
        "weapons": {
            "destination": "/Game/Weapons/",
            "translator_options": {
                "EnableDetailedLogging": "true",
                "ValidateInputFiles": "true",
                "CreateBackups": "true"
            },
            "file_patterns": ["*weapon*", "*gun*", "*sword*", "*rifle*"],
            "auto_import": True,
            "quality_checks": {
                "max_file_size_mb": 25
            }
        }
    }
    
    # Update configuration
    manager.config["import_rules"].update(custom_rules)
    manager.save_configuration()
    
    print("Updated import rules:")
    for category, rules in custom_rules.items():
        print(f"  {category}:")
        print(f"    Destination: {rules['destination']}")
        print(f"    Patterns: {rules['file_patterns']}")
        print(f"    Auto-import: {rules['auto_import']}")
    
    return manager

def setup_quality_controls():
    """Configure quality control settings"""
    print("\n=== Setting Up Quality Controls ===")
    
    manager = AutomationManager()
    
    # Configure quality checks
    quality_config = {
        "enabled": True,
        "max_file_size_mb": 100,
        "min_file_size_bytes": 1024,
        "allowed_extensions": [".foo", ".bar", ".custom"],
        "scan_for_viruses": False,
        "validate_file_integrity": True,
        "check_file_permissions": True
    }
    
    manager.config["quality_checks"] = quality_config
    
    # Configure performance settings
    performance_config = {
        "max_concurrent_imports": 4,
        "processing_delay_seconds": 1,
        "batch_size": 15,
        "timeout_seconds": 600,
        "memory_limit_mb": 2048
    }
    
    manager.config["performance"] = performance_config
    manager.save_configuration()
    
    print("Quality control settings:")
    for key, value in quality_config.items():
        print(f"  {key}: {value}")
    
    print("\nPerformance settings:")
    for key, value in performance_config.items():
        print(f"  {key}: {value}")
    
    return manager

def demonstrate_batch_processing():
    """Demonstrate batch processing capabilities"""
    print("\n=== Batch Processing Demo ===")
    
    manager = AutomationManager()
    
    # Create some mock files for demonstration
    mock_files = [
        "C:/MockAssets/character_hero.foo",
        "C:/MockAssets/character_villain.foo",
        "C:/MockAssets/environment_forest.foo",
        "C:/MockAssets/weapon_sword.foo",
        "C:/MockAssets/prop_barrel.foo"
    ]
    
    print(f"Simulating batch import of {len(mock_files)} files...")
    
    # Perform batch import
    results = manager.batch_import_files(
        mock_files, 
        "/Game/BatchImported/",
        "batch_demo"
    )
    
    print("Batch import results:")
    print(f"  Total files: {results['total_files']}")
    print(f"  Successful: {results['successful']}")
    print(f"  Failed: {results['failed']}")
    print(f"  Success rate: {results['success_rate']:.1f}%")
    print(f"  Duration: {results['duration_seconds']:.2f} seconds")
    
    # Show detailed results
    print("\nDetailed results:")
    for detail in results['details'][:5]:  # Show first 5
        status_icon = "✅" if detail['status'] == 'success' else "❌"
        print(f"  {status_icon} {os.path.basename(detail['file'])}: {detail['status']}")

def start_monitoring_demo():
    """Demonstrate file monitoring setup"""
    print("\n=== File Monitoring Demo ===")
    
    manager = AutomationManager()
    
    # Check if watchdog is available
    try:
        from watchdog.observers import Observer
        watchdog_available = True
    except ImportError:
        watchdog_available = False
    
    if not watchdog_available:
        print("⚠️ Watchdog not available - file monitoring will be simulated")
        print("   Install watchdog with: pip install watchdog")
        return manager
    
    # Start monitoring
    print("Starting file monitoring...")
    success = manager.start_monitoring()
    
    if success:
        print("✅ File monitoring started successfully")
        print("📁 Monitoring folders:")
        
        for folder in manager.config.get('watch_folders', []):
            status = "Active" if folder.get('enabled', True) else "Inactive"
            print(f"   {folder['path']} ({folder['category']}) - {status}")
        
        print("\n🔄 Monitoring is now active. Files added to watched folders will be automatically imported.")
        print("   (In a real scenario, you would keep the script running)")
        
        # Simulate some monitoring time
        print("   Simulating 5 seconds of monitoring...")
        time.sleep(5)
        
        # Stop monitoring
        print("🛑 Stopping monitoring...")
        manager.stop_monitoring()
        print("✅ Monitoring stopped")
        
    else:
        print("❌ Failed to start file monitoring")
    
    return manager

def generate_automation_report():
    """Generate and display automation report"""
    print("\n=== Automation Report ===")
    
    manager = AutomationManager()
    
    # Simulate some activity for the report
    manager.statistics.update({
        'total_processed': 25,
        'successful_imports': 22,
        'failed_imports': 3,
        'start_time': datetime.now(),
        'files_per_category': {
            'characters': 8,
            'environments': 6,
            'weapons': 4,
            'props': 7
        },
        'processing_times': [0.5, 0.8, 0.3, 0.6, 0.4, 0.7, 0.9, 0.2]
    })
    
    # Generate report
    report = manager.generate_report()
    print(report)
    
    # Save report to file
    report_file = f"automation_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
    try:
        with open(report_file, 'w') as f:
            f.write(report)
        print(f"\n📄 Report saved to: {report_file}")
    except Exception as e:
        print(f"⚠️ Could not save report: {e}")

def advanced_automation_workflow():
    """Demonstrate an advanced automation workflow"""
    print("\n=== Advanced Automation Workflow ===")
    
    # Step 1: Initialize with custom configuration
    manager = AutomationManager("advanced_automation_config.json")
    
    # Step 2: Set up comprehensive rules
    print("1. Setting up comprehensive automation rules...")
    manager = configure_import_rules()
    
    # Step 3: Configure quality controls
    print("2. Configuring quality controls...")
    manager = setup_quality_controls()
    
    # Step 4: Add multiple watch folders
    print("3. Adding watch folders...")
    advanced_folders = [
        {"path": "D:/GameAssets/Characters/", "category": "characters"},
        {"path": "D:/GameAssets/Environments/", "category": "environments"},
        {"path": "D:/GameAssets/Weapons/", "category": "weapons"},
        {"path": "D:/GameAssets/Props/", "category": "props"},
        {"path": "D:/GameAssets/Vehicles/", "category": "vehicles"}
    ]
    
    for folder_info in advanced_folders:
        if os.path.exists(folder_info["path"]):
            manager.add_watch_folder(folder_info["path"], folder_info["category"])
    
    # Step 5: Test batch processing
    print("4. Testing batch processing...")
    demonstrate_batch_processing()
    
    # Step 6: Start monitoring (briefly)
    print("5. Starting monitoring...")
    start_monitoring_demo()
    
    # Step 7: Generate comprehensive report
    print("6. Generating report...")
    generate_automation_report()
    
    print("\n✅ Advanced automation workflow setup complete!")
    
    return manager

def main():
    """Main function to run all automation examples"""
    print("Interchange Python Extension - Automation Setup Examples")
    print("=" * 70)
    
    try:
        # Run automation setup examples
        setup_basic_automation()
        configure_import_rules()
        setup_quality_controls()
        demonstrate_batch_processing()
        start_monitoring_demo()
        generate_automation_report()
        
        print("\n" + "=" * 70)
        print("🚀 Running Advanced Automation Workflow...")
        print("=" * 70)
        
        advanced_automation_workflow()
        
        print("\n" + "=" * 70)
        print("✅ All automation examples completed successfully!")
        print("\nNext steps:")
        print("1. Update folder paths to match your actual asset directories")
        print("2. Customize import rules for your specific workflow")
        print("3. Install watchdog for file monitoring: pip install watchdog")
        print("4. Run the automation in your production environment")
        
    except Exception as e:
        print(f"\n❌ Error running automation examples: {e}")
        import traceback
        traceback.print_exc()
        return False
    
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
