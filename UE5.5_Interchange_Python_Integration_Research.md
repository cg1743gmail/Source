# Unreal Engine 5.5 Interchange Python集成方案研究

## 1. Python API概述

### 1.1 核心Python模块
Unreal Engine 5.5 提供了完整的Python API来访问Interchange系统：

```python
import unreal

# 核心Interchange类
interchange_manager = unreal.InterchangeManager.get_interchange_manager_scripted()
source_data = unreal.InterchangeManager.create_source_data(file_path)
import_task = unreal.AssetImportTask()
```

### 1.2 主要Python类层次
```
unreal.InterchangeManager
├── get_interchange_manager_scripted()
├── create_source_data()
├── import_asset()
└── import_scene()

unreal.AssetImportTask
├── automated
├── destination_path
├── filename
├── options
└── save

unreal.InterchangeSourceData
├── get_filename()
├── set_filename()
└── get_data()
```

## 2. 基础导入操作

### 2.1 简单资产导入
```python
import unreal
import os

def import_single_asset(file_path, destination_path="/Game/ImportedAssets/"):
    """
    导入单个资产文件
    
    Args:
        file_path (str): 源文件路径
        destination_path (str): 目标路径
    
    Returns:
        bool: 导入是否成功
    """
    if not os.path.exists(file_path):
        print(f"File not found: {file_path}")
        return False
    
    # 获取Interchange管理器
    interchange_manager = unreal.InterchangeManager.get_interchange_manager_scripted()
    
    # 创建源数据
    source_data = unreal.InterchangeManager.create_source_data(file_path)
    if not source_data:
        print(f"Failed to create source data for: {file_path}")
        return False
    
    # 执行导入
    try:
        result = interchange_manager.import_asset(destination_path, source_data)
        if result:
            print(f"Successfully imported: {file_path}")
            return True
        else:
            print(f"Failed to import: {file_path}")
            return False
    except Exception as e:
        print(f"Import error: {e}")
        return False

# 使用示例
success = import_single_asset("C:/Assets/MyModel.fbx", "/Game/Models/")
```

### 2.2 批量资产导入
```python
import unreal
import os
import glob

def batch_import_assets(source_folder, destination_path="/Game/ImportedAssets/", 
                       file_extensions=["*.fbx", "*.obj", "*.gltf"]):
    """
    批量导入资产
    
    Args:
        source_folder (str): 源文件夹路径
        destination_path (str): 目标路径
        file_extensions (list): 支持的文件扩展名
    
    Returns:
        dict: 导入结果统计
    """
    results = {"success": 0, "failed": 0, "files": []}
    
    # 获取所有匹配的文件
    all_files = []
    for extension in file_extensions:
        pattern = os.path.join(source_folder, "**", extension)
        all_files.extend(glob.glob(pattern, recursive=True))
    
    print(f"Found {len(all_files)} files to import")
    
    # 获取Interchange管理器
    interchange_manager = unreal.InterchangeManager.get_interchange_manager_scripted()
    
    for file_path in all_files:
        try:
            # 创建相对路径结构
            rel_path = os.path.relpath(file_path, source_folder)
            dest_dir = os.path.dirname(rel_path).replace("\\", "/")
            full_dest_path = f"{destination_path.rstrip('/')}/{dest_dir}/"
            
            # 创建源数据
            source_data = unreal.InterchangeManager.create_source_data(file_path)
            if not source_data:
                results["failed"] += 1
                results["files"].append({"file": file_path, "status": "failed", "error": "Failed to create source data"})
                continue
            
            # 执行导入
            result = interchange_manager.import_asset(full_dest_path, source_data)
            if result:
                results["success"] += 1
                results["files"].append({"file": file_path, "status": "success", "destination": full_dest_path})
                print(f"✓ Imported: {file_path}")
            else:
                results["failed"] += 1
                results["files"].append({"file": file_path, "status": "failed", "error": "Import failed"})
                print(f"✗ Failed: {file_path}")
                
        except Exception as e:
            results["failed"] += 1
            results["files"].append({"file": file_path, "status": "failed", "error": str(e)})
            print(f"✗ Error importing {file_path}: {e}")
    
    print(f"\nImport completed: {results['success']} success, {results['failed']} failed")
    return results

# 使用示例
results = batch_import_assets("C:/GameAssets/", "/Game/ImportedContent/", ["*.fbx", "*.png", "*.jpg"])
```

## 3. 高级导入配置

### 3.1 自定义导入管道
```python
import unreal

def import_with_custom_pipeline(file_path, destination_path, pipeline_settings=None):
    """
    使用自定义管道设置导入资产
    
    Args:
        file_path (str): 源文件路径
        destination_path (str): 目标路径
        pipeline_settings (dict): 管道设置
    
    Returns:
        bool: 导入是否成功
    """
    # 默认管道设置
    default_settings = {
        "import_meshes": True,
        "import_materials": True,
        "import_textures": True,
        "generate_lightmap_uvs": False,
        "combine_meshes": False,
        "import_animations": True,
        "import_morph_targets": True,
    }
    
    if pipeline_settings:
        default_settings.update(pipeline_settings)
    
    # 获取Interchange管理器
    interchange_manager = unreal.InterchangeManager.get_interchange_manager_scripted()
    
    # 创建源数据
    source_data = unreal.InterchangeManager.create_source_data(file_path)
    if not source_data:
        return False
    
    # 创建导入任务
    import_task = unreal.AssetImportTask()
    import_task.automated = True
    import_task.destination_path = destination_path
    import_task.filename = file_path
    import_task.save = True
    
    # 应用管道设置（这里需要根据具体的管道类型进行设置）
    # 注意：具体的管道设置API可能因UE版本而异
    try:
        # 执行导入
        result = interchange_manager.import_asset(destination_path, source_data)
        return result is not None
    except Exception as e:
        print(f"Import failed: {e}")
        return False

# 使用示例
pipeline_config = {
    "import_meshes": True,
    "import_materials": False,  # 不导入材质
    "generate_lightmap_uvs": True,  # 生成光照贴图UV
}

success = import_with_custom_pipeline(
    "C:/Assets/Character.fbx", 
    "/Game/Characters/", 
    pipeline_config
)
```

### 3.2 材质和纹理处理
```python
import unreal

def import_with_material_processing(fbx_path, texture_folder, destination_path):
    """
    导入FBX并处理相关材质和纹理
    
    Args:
        fbx_path (str): FBX文件路径
        texture_folder (str): 纹理文件夹路径
        destination_path (str): 目标路径
    """
    interchange_manager = unreal.InterchangeManager.get_interchange_manager_scripted()
    
    # 1. 首先导入纹理
    texture_extensions = ["*.png", "*.jpg", "*.jpeg", "*.tga", "*.bmp"]
    texture_results = batch_import_assets(
        texture_folder, 
        f"{destination_path}/Textures/", 
        texture_extensions
    )
    
    # 2. 导入FBX模型
    fbx_source_data = unreal.InterchangeManager.create_source_data(fbx_path)
    if fbx_source_data:
        model_result = interchange_manager.import_asset(f"{destination_path}/Meshes/", fbx_source_data)
        
        if model_result:
            print("Model imported successfully")
            
            # 3. 后处理：设置材质纹理引用
            # 这里需要使用UE的资产管理API来查找和连接材质
            setup_material_textures(destination_path)
        else:
            print("Model import failed")
    
    return texture_results

def setup_material_textures(base_path):
    """
    设置材质的纹理引用
    """
    # 获取资产注册表
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    
    # 查找材质资产
    material_filter = unreal.ARFilter()
    material_filter.class_names = ["Material"]
    material_filter.package_paths = [base_path]
    
    materials = asset_registry.get_assets(material_filter)
    
    for material_data in materials:
        material_path = material_data.object_path
        print(f"Processing material: {material_path}")
        
        # 加载材质资产
        material = unreal.EditorAssetLibrary.load_asset(str(material_path))
        if material:
            # 这里可以添加具体的材质纹理连接逻辑
            # 需要使用材质编辑器API
            pass
```

## 4. 场景导入和处理

### 4.1 场景层次结构导入
```python
import unreal

def import_scene_with_hierarchy(scene_file, destination_path):
    """
    导入场景并保持层次结构
    
    Args:
        scene_file (str): 场景文件路径（如FBX场景）
        destination_path (str): 目标路径
    """
    interchange_manager = unreal.InterchangeManager.get_interchange_manager_scripted()
    
    # 创建源数据
    source_data = unreal.InterchangeManager.create_source_data(scene_file)
    if not source_data:
        print("Failed to create source data")
        return False
    
    try:
        # 使用import_scene而不是import_asset来保持场景结构
        result = interchange_manager.import_scene(destination_path, source_data)
        
        if result:
            print(f"Scene imported successfully to {destination_path}")
            
            # 后处理：分析导入的场景结构
            analyze_imported_scene(destination_path)
            return True
        else:
            print("Scene import failed")
            return False
            
    except Exception as e:
        print(f"Scene import error: {e}")
        return False

def analyze_imported_scene(scene_path):
    """
    分析导入的场景结构
    """
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    
    # 查找所有相关资产
    filter_data = unreal.ARFilter()
    filter_data.package_paths = [scene_path]
    
    assets = asset_registry.get_assets(filter_data)
    
    asset_types = {}
    for asset_data in assets:
        asset_class = str(asset_data.asset_class)
        if asset_class not in asset_types:
            asset_types[asset_class] = 0
        asset_types[asset_class] += 1
    
    print("Imported scene analysis:")
    for asset_type, count in asset_types.items():
        print(f"  {asset_type}: {count}")
```

## 5. 自动化工作流程

### 5.1 监控文件夹自动导入
```python
import unreal
import os
import time
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

class AssetImportHandler(FileSystemEventHandler):
    """
    文件系统监控处理器，自动导入新文件
    """
    
    def __init__(self, destination_path="/Game/AutoImported/"):
        self.destination_path = destination_path
        self.supported_extensions = ['.fbx', '.obj', '.gltf', '.png', '.jpg', '.jpeg', '.tga']
        self.interchange_manager = unreal.InterchangeManager.get_interchange_manager_scripted()
    
    def on_created(self, event):
        if not event.is_directory:
            file_path = event.src_path
            file_ext = os.path.splitext(file_path)[1].lower()
            
            if file_ext in self.supported_extensions:
                print(f"New file detected: {file_path}")
                # 等待文件写入完成
                time.sleep(2)
                self.import_file(file_path)
    
    def import_file(self, file_path):
        """
        导入单个文件
        """
        try:
            source_data = unreal.InterchangeManager.create_source_data(file_path)
            if source_data:
                result = self.interchange_manager.import_asset(self.destination_path, source_data)
                if result:
                    print(f"✓ Auto-imported: {file_path}")
                else:
                    print(f"✗ Auto-import failed: {file_path}")
            else:
                print(f"✗ Failed to create source data: {file_path}")
        except Exception as e:
            print(f"✗ Auto-import error for {file_path}: {e}")

def start_auto_import_monitor(watch_folder, destination_path="/Game/AutoImported/"):
    """
    启动自动导入监控
    
    Args:
        watch_folder (str): 监控的文件夹路径
        destination_path (str): 导入目标路径
    """
    if not os.path.exists(watch_folder):
        print(f"Watch folder does not exist: {watch_folder}")
        return None
    
    event_handler = AssetImportHandler(destination_path)
    observer = Observer()
    observer.schedule(event_handler, watch_folder, recursive=True)
    observer.start()
    
    print(f"Started monitoring {watch_folder} for auto-import...")
    return observer

# 使用示例
# observer = start_auto_import_monitor("C:/DropFolder/", "/Game/AutoImported/")
# 
# try:
#     while True:
#         time.sleep(1)
# except KeyboardInterrupt:
#     observer.stop()
# observer.join()
```

这个Python集成研究为后续的完整开发文档奠定了基础，展示了如何在Python中有效使用Interchange系统。
