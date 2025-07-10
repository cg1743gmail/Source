# Unreal Engine 5.5 Interchange Python开发完整指南

## 1. 环境配置与基础设置

### 1.1 Python环境准备
```python
# 必要的Python模块导入
import unreal
import os
import sys
import json
import time
import glob
import threading
from pathlib import Path
from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass
from enum import Enum

# 添加自定义模块路径
sys.path.append(os.path.join(os.path.dirname(__file__), 'utils'))
```

### 1.2 配置管理类
```python
@dataclass
class ImportConfig:
    """导入配置数据类"""
    source_path: str
    destination_path: str
    file_extensions: List[str]
    import_meshes: bool = True
    import_materials: bool = True
    import_textures: bool = True
    import_animations: bool = True
    generate_lightmap_uvs: bool = False
    combine_meshes: bool = False
    auto_save: bool = True
    overwrite_existing: bool = False

class AssetType(Enum):
    """资产类型枚举"""
    MESH = "StaticMesh"
    SKELETAL_MESH = "SkeletalMesh"
    TEXTURE = "Texture2D"
    MATERIAL = "Material"
    ANIMATION = "AnimSequence"
    SOUND = "SoundWave"

class InterchangeHelper:
    """Interchange系统辅助类"""
    
    def __init__(self):
        self.manager = unreal.InterchangeManager.get_interchange_manager_scripted()
        self.asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        self.editor_asset_lib = unreal.EditorAssetLibrary
    
    def is_valid_file(self, file_path: str, extensions: List[str]) -> bool:
        """检查文件是否有效"""
        if not os.path.exists(file_path):
            return False
        
        file_ext = os.path.splitext(file_path)[1].lower()
        return file_ext in [ext.lower() for ext in extensions]
    
    def create_directory_structure(self, path: str) -> bool:
        """创建目录结构"""
        try:
            if not self.editor_asset_lib.does_directory_exist(path):
                return self.editor_asset_lib.make_directory(path)
            return True
        except Exception as e:
            print(f"Failed to create directory {path}: {e}")
            return False
```

## 2. 核心导入功能实现

### 2.1 单文件导入器
```python
class SingleAssetImporter:
    """单个资产导入器"""
    
    def __init__(self, helper: InterchangeHelper):
        self.helper = helper
        self.import_results = []
    
    def import_asset(self, file_path: str, destination_path: str, 
                    config: ImportConfig = None) -> Dict:
        """
        导入单个资产
        
        Args:
            file_path: 源文件路径
            destination_path: 目标路径
            config: 导入配置
            
        Returns:
            导入结果字典
        """
        result = {
            "file_path": file_path,
            "destination_path": destination_path,
            "success": False,
            "error": None,
            "imported_assets": []
        }
        
        try:
            # 验证文件
            if not os.path.exists(file_path):
                result["error"] = f"File not found: {file_path}"
                return result
            
            # 创建目标目录
            if not self.helper.create_directory_structure(destination_path):
                result["error"] = f"Failed to create directory: {destination_path}"
                return result
            
            # 创建源数据
            source_data = unreal.InterchangeManager.create_source_data(file_path)
            if not source_data:
                result["error"] = "Failed to create source data"
                return result
            
            # 执行导入
            import_result = self.helper.manager.import_asset(destination_path, source_data)
            
            if import_result:
                result["success"] = True
                result["imported_assets"] = self._get_imported_assets(destination_path, file_path)
                
                # 应用后处理配置
                if config:
                    self._apply_post_import_settings(result["imported_assets"], config)
                
                print(f"✓ Successfully imported: {file_path}")
            else:
                result["error"] = "Import operation failed"
                print(f"✗ Failed to import: {file_path}")
                
        except Exception as e:
            result["error"] = str(e)
            print(f"✗ Exception during import of {file_path}: {e}")
        
        self.import_results.append(result)
        return result
    
    def _get_imported_assets(self, destination_path: str, source_file: str) -> List[str]:
        """获取导入的资产列表"""
        imported_assets = []
        
        # 基于源文件名查找导入的资产
        base_name = os.path.splitext(os.path.basename(source_file))[0]
        
        # 搜索相关资产
        filter_data = unreal.ARFilter()
        filter_data.package_paths = [destination_path]
        
        assets = self.helper.asset_registry.get_assets(filter_data)
        
        for asset_data in assets:
            asset_name = str(asset_data.asset_name)
            if base_name.lower() in asset_name.lower():
                imported_assets.append(str(asset_data.object_path))
        
        return imported_assets
    
    def _apply_post_import_settings(self, asset_paths: List[str], config: ImportConfig):
        """应用导入后设置"""
        for asset_path in asset_paths:
            try:
                asset = self.helper.editor_asset_lib.load_asset(asset_path)
                if asset:
                    self._configure_asset(asset, config)
            except Exception as e:
                print(f"Failed to configure asset {asset_path}: {e}")
    
    def _configure_asset(self, asset, config: ImportConfig):
        """配置单个资产"""
        asset_class = asset.get_class().get_name()
        
        if asset_class == "StaticMesh" and hasattr(asset, 'get_static_mesh_component'):
            # 静态网格配置
            if config.generate_lightmap_uvs:
                # 这里需要调用相应的API来生成光照贴图UV
                pass
        
        elif asset_class == "Texture2D":
            # 纹理配置
            if hasattr(asset, 'compression_settings'):
                # 设置纹理压缩等
                pass
```

### 2.2 批量导入器
```python
class BatchAssetImporter:
    """批量资产导入器"""
    
    def __init__(self, helper: InterchangeHelper):
        self.helper = helper
        self.single_importer = SingleAssetImporter(helper)
        self.progress_callback = None
        self.cancel_requested = False
    
    def set_progress_callback(self, callback):
        """设置进度回调函数"""
        self.progress_callback = callback
    
    def import_directory(self, source_dir: str, destination_path: str, 
                        config: ImportConfig) -> Dict:
        """
        导入整个目录
        
        Args:
            source_dir: 源目录路径
            destination_path: 目标路径
            config: 导入配置
            
        Returns:
            批量导入结果
        """
        results = {
            "total_files": 0,
            "successful_imports": 0,
            "failed_imports": 0,
            "skipped_files": 0,
            "import_details": []
        }
        
        # 收集所有文件
        all_files = self._collect_files(source_dir, config.file_extensions)
        results["total_files"] = len(all_files)
        
        if results["total_files"] == 0:
            print(f"No files found in {source_dir} with extensions {config.file_extensions}")
            return results
        
        print(f"Found {results['total_files']} files to import")
        
        # 批量导入
        for i, file_path in enumerate(all_files):
            if self.cancel_requested:
                print("Import cancelled by user")
                break
            
            # 计算目标路径
            rel_path = os.path.relpath(file_path, source_dir)
            dest_dir = os.path.dirname(rel_path).replace("\\", "/")
            full_dest_path = f"{destination_path.rstrip('/')}/{dest_dir}/"
            
            # 检查是否需要跳过
            if not config.overwrite_existing and self._asset_exists(file_path, full_dest_path):
                results["skipped_files"] += 1
                print(f"⚠ Skipped (already exists): {file_path}")
                continue
            
            # 执行导入
            import_result = self.single_importer.import_asset(file_path, full_dest_path, config)
            results["import_details"].append(import_result)
            
            if import_result["success"]:
                results["successful_imports"] += 1
            else:
                results["failed_imports"] += 1
            
            # 更新进度
            if self.progress_callback:
                progress = (i + 1) / results["total_files"] * 100
                self.progress_callback(progress, file_path, import_result["success"])
            
            # 自动保存
            if config.auto_save and (i + 1) % 10 == 0:
                self._save_all_assets()
        
        # 最终保存
        if config.auto_save:
            self._save_all_assets()
        
        self._print_summary(results)
        return results
    
    def _collect_files(self, directory: str, extensions: List[str]) -> List[str]:
        """收集指定扩展名的所有文件"""
        all_files = []
        
        for ext in extensions:
            pattern = os.path.join(directory, "**", f"*{ext}")
            files = glob.glob(pattern, recursive=True)
            all_files.extend(files)
        
        return sorted(list(set(all_files)))  # 去重并排序
    
    def _asset_exists(self, source_file: str, destination_path: str) -> bool:
        """检查资产是否已存在"""
        base_name = os.path.splitext(os.path.basename(source_file))[0]
        asset_path = f"{destination_path}{base_name}"
        return self.helper.editor_asset_lib.does_asset_exist(asset_path)
    
    def _save_all_assets(self):
        """保存所有资产"""
        try:
            self.helper.editor_asset_lib.save_directory("/Game/", only_if_is_dirty=True)
            print("Assets saved")
        except Exception as e:
            print(f"Failed to save assets: {e}")
    
    def _print_summary(self, results: Dict):
        """打印导入摘要"""
        print("\n" + "="*50)
        print("IMPORT SUMMARY")
        print("="*50)
        print(f"Total files processed: {results['total_files']}")
        print(f"Successful imports: {results['successful_imports']}")
        print(f"Failed imports: {results['failed_imports']}")
        print(f"Skipped files: {results['skipped_files']}")
        
        if results['failed_imports'] > 0:
            print("\nFailed imports:")
            for detail in results['import_details']:
                if not detail['success']:
                    print(f"  ✗ {detail['file_path']}: {detail['error']}")
    
    def cancel_import(self):
        """取消导入操作"""
        self.cancel_requested = True
```

## 3. 美术资源自动化场景

### 3.1 角色资产导入流水线
```python
class CharacterAssetPipeline:
    """角色资产导入流水线"""
    
    def __init__(self, helper: InterchangeHelper):
        self.helper = helper
        self.batch_importer = BatchAssetImporter(helper)
    
    def import_character_package(self, character_folder: str, character_name: str) -> Dict:
        """
        导入完整的角色资产包
        
        Args:
            character_folder: 角色文件夹路径
            character_name: 角色名称
            
        Returns:
            导入结果
        """
        base_dest_path = f"/Game/Characters/{character_name}/"
        
        # 定义各类资产的子目录和配置
        asset_configs = {
            "Meshes": {
                "extensions": [".fbx", ".obj"],
                "dest_path": f"{base_dest_path}Meshes/",
                "config": ImportConfig(
                    source_path=character_folder,
                    destination_path=f"{base_dest_path}Meshes/",
                    file_extensions=[".fbx", ".obj"],
                    import_meshes=True,
                    import_materials=True,
                    import_animations=True,
                    generate_lightmap_uvs=True
                )
            },
            "Textures": {
                "extensions": [".png", ".jpg", ".tga", ".exr"],
                "dest_path": f"{base_dest_path}Textures/",
                "config": ImportConfig(
                    source_path=character_folder,
                    destination_path=f"{base_dest_path}Textures/",
                    file_extensions=[".png", ".jpg", ".tga", ".exr"],
                    import_textures=True
                )
            },
            "Animations": {
                "extensions": [".fbx"],
                "dest_path": f"{base_dest_path}Animations/",
                "config": ImportConfig(
                    source_path=character_folder,
                    destination_path=f"{base_dest_path}Animations/",
                    file_extensions=[".fbx"],
                    import_animations=True,
                    import_meshes=False,
                    import_materials=False
                )
            }
        }
        
        overall_results = {"categories": {}, "total_success": 0, "total_failed": 0}
        
        # 按类别导入
        for category, config_data in asset_configs.items():
            print(f"\n--- Importing {category} ---")
            
            category_folder = os.path.join(character_folder, category)
            if os.path.exists(category_folder):
                results = self.batch_importer.import_directory(
                    category_folder, 
                    config_data["dest_path"], 
                    config_data["config"]
                )
                overall_results["categories"][category] = results
                overall_results["total_success"] += results["successful_imports"]
                overall_results["total_failed"] += results["failed_imports"]
            else:
                print(f"Category folder not found: {category_folder}")
        
        # 后处理：创建角色蓝图等
        self._create_character_blueprint(character_name, base_dest_path)
        
        return overall_results
    
    def _create_character_blueprint(self, character_name: str, base_path: str):
        """创建角色蓝图（示例）"""
        try:
            # 这里可以添加创建角色蓝图的逻辑
            blueprint_path = f"{base_path}BP_{character_name}"
            print(f"Character blueprint would be created at: {blueprint_path}")
            # 实际实现需要使用UE的蓝图创建API
        except Exception as e:
            print(f"Failed to create character blueprint: {e}")
```

### 3.2 环境资产批处理
```python
class EnvironmentAssetProcessor:
    """环境资产处理器"""
    
    def __init__(self, helper: InterchangeHelper):
        self.helper = helper
        self.batch_importer = BatchAssetImporter(helper)
    
    def process_environment_package(self, env_folder: str, env_name: str) -> Dict:
        """
        处理环境资产包
        
        Args:
            env_folder: 环境资产文件夹
            env_name: 环境名称
            
        Returns:
            处理结果
        """
        base_dest_path = f"/Game/Environments/{env_name}/"
        
        # 环境资产分类处理
        categories = {
            "Architecture": {
                "subfolders": ["Buildings", "Structures", "Props"],
                "extensions": [".fbx", ".obj"],
                "generate_lightmap_uvs": True,
                "combine_meshes": False
            },
            "Landscape": {
                "subfolders": ["Terrain", "Vegetation", "Rocks"],
                "extensions": [".fbx", ".obj"],
                "generate_lightmap_uvs": True,
                "combine_meshes": True
            },
            "Materials": {
                "subfolders": ["Textures", "Substances"],
                "extensions": [".png", ".jpg", ".tga", ".exr", ".hdr"],
                "generate_lightmap_uvs": False,
                "combine_meshes": False
            }
        }
        
        results = {}
        
        for category, settings in categories.items():
            category_results = []
            
            for subfolder in settings["subfolders"]:
                subfolder_path = os.path.join(env_folder, category, subfolder)
                if os.path.exists(subfolder_path):
                    config = ImportConfig(
                        source_path=subfolder_path,
                        destination_path=f"{base_dest_path}{category}/{subfolder}/",
                        file_extensions=settings["extensions"],
                        generate_lightmap_uvs=settings["generate_lightmap_uvs"],
                        combine_meshes=settings["combine_meshes"]
                    )
                    
                    result = self.batch_importer.import_directory(
                        subfolder_path,
                        f"{base_dest_path}{category}/{subfolder}/",
                        config
                    )
                    category_results.append(result)
            
            results[category] = category_results
        
        # 创建环境关卡
        self._create_environment_level(env_name, base_dest_path)
        
        return results
    
    def _create_environment_level(self, env_name: str, base_path: str):
        """创建环境关卡"""
        try:
            level_path = f"{base_path}Maps/{env_name}_Level"
            print(f"Environment level would be created at: {level_path}")
            # 实际实现需要使用UE的关卡创建API
        except Exception as e:
            print(f"Failed to create environment level: {e}")
```

### 3.3 动画资产自动化处理
```python
class AnimationAssetProcessor:
    """动画资产处理器"""

    def __init__(self, helper: InterchangeHelper):
        self.helper = helper
        self.single_importer = SingleAssetImporter(helper)

    def process_animation_library(self, anim_folder: str, character_name: str) -> Dict:
        """
        处理动画库

        Args:
            anim_folder: 动画文件夹路径
            character_name: 角色名称

        Returns:
            处理结果
        """
        base_dest_path = f"/Game/Characters/{character_name}/Animations/"

        # 动画分类
        animation_categories = {
            "Locomotion": ["walk", "run", "idle", "jump"],
            "Combat": ["attack", "defend", "dodge", "block"],
            "Interaction": ["pickup", "use", "open", "close"],
            "Emotes": ["wave", "dance", "sit", "laugh"]
        }

        results = {"categories": {}, "total_animations": 0}

        # 收集所有FBX动画文件
        anim_files = glob.glob(os.path.join(anim_folder, "**/*.fbx"), recursive=True)

        for anim_file in anim_files:
            file_name = os.path.basename(anim_file).lower()

            # 根据文件名分类
            category = "Misc"  # 默认分类
            for cat, keywords in animation_categories.items():
                if any(keyword in file_name for keyword in keywords):
                    category = cat
                    break

            # 导入动画
            dest_path = f"{base_dest_path}{category}/"
            config = ImportConfig(
                source_path=anim_file,
                destination_path=dest_path,
                file_extensions=[".fbx"],
                import_animations=True,
                import_meshes=False,
                import_materials=False
            )

            result = self.single_importer.import_asset(anim_file, dest_path, config)

            if category not in results["categories"]:
                results["categories"][category] = []
            results["categories"][category].append(result)

            if result["success"]:
                results["total_animations"] += 1

        # 创建动画蓝图
        self._create_animation_blueprint(character_name, base_dest_path)

        return results

    def _create_animation_blueprint(self, character_name: str, base_path: str):
        """创建动画蓝图"""
        try:
            anim_bp_path = f"{base_path}ABP_{character_name}"
            print(f"Animation Blueprint would be created at: {anim_bp_path}")
            # 实际实现需要使用UE的动画蓝图创建API
        except Exception as e:
            print(f"Failed to create animation blueprint: {e}")

class MaterialAssetProcessor:
    """材质资产处理器"""

    def __init__(self, helper: InterchangeHelper):
        self.helper = helper
        self.single_importer = SingleAssetImporter(helper)

    def process_material_library(self, material_folder: str, library_name: str) -> Dict:
        """
        处理材质库

        Args:
            material_folder: 材质文件夹路径
            library_name: 材质库名称

        Returns:
            处理结果
        """
        base_dest_path = f"/Game/Materials/{library_name}/"

        # 材质类型映射
        material_types = {
            "Diffuse": ["_d", "_diffuse", "_albedo", "_basecolor"],
            "Normal": ["_n", "_normal", "_nrm"],
            "Roughness": ["_r", "_roughness", "_rough"],
            "Metallic": ["_m", "_metallic", "_metal"],
            "Specular": ["_s", "_specular", "_spec"],
            "Emission": ["_e", "_emission", "_emissive"],
            "Opacity": ["_o", "_opacity", "_alpha"],
            "AO": ["_ao", "_ambient", "_occlusion"]
        }

        results = {"material_sets": {}, "total_textures": 0}

        # 收集所有纹理文件
        texture_extensions = [".png", ".jpg", ".jpeg", ".tga", ".exr", ".hdr"]
        texture_files = []

        for ext in texture_extensions:
            pattern = os.path.join(material_folder, f"**/*{ext}")
            texture_files.extend(glob.glob(pattern, recursive=True))

        # 按材质集分组
        material_sets = {}
        for texture_file in texture_files:
            base_name = self._extract_material_base_name(texture_file, material_types)

            if base_name not in material_sets:
                material_sets[base_name] = []
            material_sets[base_name].append(texture_file)

        # 导入每个材质集
        for material_name, textures in material_sets.items():
            material_dest_path = f"{base_dest_path}{material_name}/"

            set_results = []
            for texture_file in textures:
                config = ImportConfig(
                    source_path=texture_file,
                    destination_path=material_dest_path,
                    file_extensions=texture_extensions,
                    import_textures=True
                )

                result = self.single_importer.import_asset(texture_file, material_dest_path, config)
                set_results.append(result)

                if result["success"]:
                    results["total_textures"] += 1

            results["material_sets"][material_name] = set_results

            # 创建材质实例
            self._create_material_instance(material_name, material_dest_path, textures)

        return results

    def _extract_material_base_name(self, texture_path: str, material_types: Dict) -> str:
        """提取材质基础名称"""
        file_name = os.path.splitext(os.path.basename(texture_path))[0].lower()

        # 移除材质类型后缀
        for type_name, suffixes in material_types.items():
            for suffix in suffixes:
                if file_name.endswith(suffix):
                    return file_name[:-len(suffix)]

        return file_name

    def _create_material_instance(self, material_name: str, dest_path: str, textures: List[str]):
        """创建材质实例"""
        try:
            material_path = f"{dest_path}M_{material_name}"
            print(f"Material instance would be created at: {material_path}")
            # 实际实现需要使用UE的材质创建API
        except Exception as e:
            print(f"Failed to create material instance for {material_name}: {e}")
```

## 4. 高级自动化工作流程

### 4.1 项目资产同步器
```python
class ProjectAssetSynchronizer:
    """项目资产同步器"""

    def __init__(self, helper: InterchangeHelper):
        self.helper = helper
        self.batch_importer = BatchAssetImporter(helper)
        self.sync_config_file = "asset_sync_config.json"

    def setup_sync_configuration(self, config: Dict):
        """设置同步配置"""
        with open(self.sync_config_file, 'w') as f:
            json.dump(config, f, indent=2)

    def sync_from_external_source(self, source_root: str) -> Dict:
        """
        从外部源同步资产

        Args:
            source_root: 外部资产根目录

        Returns:
            同步结果
        """
        # 加载同步配置
        if not os.path.exists(self.sync_config_file):
            print("Sync configuration not found. Please run setup_sync_configuration first.")
            return {}

        with open(self.sync_config_file, 'r') as f:
            sync_config = json.load(f)

        sync_results = {"synced_categories": {}, "total_synced": 0, "errors": []}

        for category, settings in sync_config.items():
            try:
                category_source = os.path.join(source_root, settings["source_folder"])
                category_dest = settings["destination_path"]

                if os.path.exists(category_source):
                    # 检查文件修改时间
                    files_to_sync = self._get_modified_files(
                        category_source,
                        category_dest,
                        settings["extensions"]
                    )

                    if files_to_sync:
                        config = ImportConfig(
                            source_path=category_source,
                            destination_path=category_dest,
                            file_extensions=settings["extensions"],
                            **settings.get("import_settings", {})
                        )

                        # 执行同步
                        result = self._sync_files(files_to_sync, category_dest, config)
                        sync_results["synced_categories"][category] = result
                        sync_results["total_synced"] += result["successful_imports"]
                    else:
                        print(f"No modified files found for category: {category}")
                else:
                    error_msg = f"Source folder not found: {category_source}"
                    sync_results["errors"].append(error_msg)
                    print(error_msg)

            except Exception as e:
                error_msg = f"Error syncing category {category}: {e}"
                sync_results["errors"].append(error_msg)
                print(error_msg)

        return sync_results

    def _get_modified_files(self, source_dir: str, dest_dir: str, extensions: List[str]) -> List[str]:
        """获取已修改的文件列表"""
        modified_files = []

        # 收集源文件
        source_files = []
        for ext in extensions:
            pattern = os.path.join(source_dir, f"**/*{ext}")
            source_files.extend(glob.glob(pattern, recursive=True))

        for source_file in source_files:
            # 计算对应的目标资产路径
            rel_path = os.path.relpath(source_file, source_dir)
            asset_name = os.path.splitext(os.path.basename(source_file))[0]
            dest_asset_path = f"{dest_dir}{asset_name}"

            # 检查是否需要更新
            if self._should_update_asset(source_file, dest_asset_path):
                modified_files.append(source_file)

        return modified_files

    def _should_update_asset(self, source_file: str, dest_asset_path: str) -> bool:
        """检查是否应该更新资产"""
        # 检查资产是否存在
        if not self.helper.editor_asset_lib.does_asset_exist(dest_asset_path):
            return True

        # 比较修改时间
        source_mtime = os.path.getmtime(source_file)

        # 获取UE资产的修改时间（这需要特定的API调用）
        # 这里简化处理，实际实现需要获取资产的导入时间戳
        return True  # 简化：总是更新

    def _sync_files(self, files: List[str], dest_path: str, config: ImportConfig) -> Dict:
        """同步文件列表"""
        results = {"successful_imports": 0, "failed_imports": 0, "details": []}

        for file_path in files:
            try:
                result = self.batch_importer.single_importer.import_asset(file_path, dest_path, config)
                results["details"].append(result)

                if result["success"]:
                    results["successful_imports"] += 1
                else:
                    results["failed_imports"] += 1

            except Exception as e:
                results["failed_imports"] += 1
                results["details"].append({
                    "file_path": file_path,
                    "success": False,
                    "error": str(e)
                })

        return results

# 示例同步配置
sync_config_example = {
    "Characters": {
        "source_folder": "Characters",
        "destination_path": "/Game/Characters/",
        "extensions": [".fbx", ".png", ".jpg"],
        "import_settings": {
            "import_meshes": True,
            "import_materials": True,
            "import_animations": True,
            "generate_lightmap_uvs": True
        }
    },
    "Environments": {
        "source_folder": "Environments",
        "destination_path": "/Game/Environments/",
        "extensions": [".fbx", ".obj", ".png", ".jpg", ".hdr"],
        "import_settings": {
            "import_meshes": True,
            "import_materials": True,
            "generate_lightmap_uvs": True,
            "combine_meshes": False
        }
    },
    "Audio": {
        "source_folder": "Audio",
        "destination_path": "/Game/Audio/",
        "extensions": [".wav", ".ogg"],
        "import_settings": {
            "import_audio": True
        }
    }
}
```

### 4.2 实时监控和热重载
```python
import threading
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

class AssetHotReloadHandler(FileSystemEventHandler):
    """资产热重载处理器"""

    def __init__(self, helper: InterchangeHelper, config: ImportConfig):
        self.helper = helper
        self.config = config
        self.single_importer = SingleAssetImporter(helper)
        self.processing_queue = []
        self.processing_lock = threading.Lock()
        self.processing_thread = None
        self.stop_processing = False

        # 启动处理线程
        self.start_processing_thread()

    def on_modified(self, event):
        if not event.is_directory:
            self._queue_file_for_processing(event.src_path)

    def on_created(self, event):
        if not event.is_directory:
            self._queue_file_for_processing(event.src_path)

    def _queue_file_for_processing(self, file_path: str):
        """将文件加入处理队列"""
        file_ext = os.path.splitext(file_path)[1].lower()
        if file_ext in [ext.lower() for ext in self.config.file_extensions]:
            with self.processing_lock:
                if file_path not in self.processing_queue:
                    self.processing_queue.append(file_path)
                    print(f"Queued for hot reload: {file_path}")

    def start_processing_thread(self):
        """启动处理线程"""
        self.processing_thread = threading.Thread(target=self._process_queue)
        self.processing_thread.daemon = True
        self.processing_thread.start()

    def _process_queue(self):
        """处理队列中的文件"""
        while not self.stop_processing:
            files_to_process = []

            with self.processing_lock:
                if self.processing_queue:
                    files_to_process = self.processing_queue.copy()
                    self.processing_queue.clear()

            for file_path in files_to_process:
                try:
                    # 等待文件写入完成
                    time.sleep(1)

                    if os.path.exists(file_path):
                        # 计算目标路径
                        rel_path = os.path.relpath(file_path, self.config.source_path)
                        dest_dir = os.path.dirname(rel_path).replace("\\", "/")
                        full_dest_path = f"{self.config.destination_path.rstrip('/')}/{dest_dir}/"

                        # 执行热重载
                        result = self.single_importer.import_asset(file_path, full_dest_path, self.config)

                        if result["success"]:
                            print(f"🔥 Hot reloaded: {file_path}")
                        else:
                            print(f"❌ Hot reload failed: {file_path} - {result['error']}")

                except Exception as e:
                    print(f"❌ Hot reload error for {file_path}: {e}")

            time.sleep(0.5)  # 短暂休眠

    def stop(self):
        """停止热重载"""
        self.stop_processing = True
        if self.processing_thread:
            self.processing_thread.join()

class AssetMonitoringService:
    """资产监控服务"""

    def __init__(self, helper: InterchangeHelper):
        self.helper = helper
        self.observers = {}
        self.monitoring_configs = {}

    def add_monitoring_path(self, name: str, source_path: str, config: ImportConfig):
        """添加监控路径"""
        if name in self.observers:
            self.remove_monitoring_path(name)

        handler = AssetHotReloadHandler(self.helper, config)
        observer = Observer()
        observer.schedule(handler, source_path, recursive=True)

        self.observers[name] = observer
        self.monitoring_configs[name] = {"path": source_path, "config": config, "handler": handler}

        observer.start()
        print(f"Started monitoring: {source_path}")

    def remove_monitoring_path(self, name: str):
        """移除监控路径"""
        if name in self.observers:
            self.observers[name].stop()
            self.observers[name].join()
            self.monitoring_configs[name]["handler"].stop()

            del self.observers[name]
            del self.monitoring_configs[name]
            print(f"Stopped monitoring: {name}")

    def stop_all_monitoring(self):
        """停止所有监控"""
        for name in list(self.observers.keys()):
            self.remove_monitoring_path(name)
```

## 5. 质量控制和验证

### 5.1 资产质量检查器
```python
class AssetQualityChecker:
    """资产质量检查器"""

    def __init__(self, helper: InterchangeHelper):
        self.helper = helper
        self.quality_rules = self._load_quality_rules()

    def _load_quality_rules(self) -> Dict:
        """加载质量规则"""
        return {
            "StaticMesh": {
                "max_triangles": 50000,
                "min_triangles": 10,
                "require_lightmap_uvs": True,
                "max_materials": 5,
                "require_collision": True
            },
            "SkeletalMesh": {
                "max_triangles": 30000,
                "min_triangles": 100,
                "max_bones": 150,
                "require_physics_asset": True
            },
            "Texture2D": {
                "max_resolution": 4096,
                "min_resolution": 32,
                "power_of_two": True,
                "compression_required": True
            },
            "Material": {
                "max_texture_samples": 16,
                "max_instruction_count": 1000,
                "require_base_color": True
            }
        }

    def check_asset_quality(self, asset_path: str) -> Dict:
        """
        检查单个资产质量

        Args:
            asset_path: 资产路径

        Returns:
            质量检查结果
        """
        result = {
            "asset_path": asset_path,
            "asset_type": None,
            "passed": True,
            "warnings": [],
            "errors": [],
            "metrics": {}
        }

        try:
            # 加载资产
            asset = self.helper.editor_asset_lib.load_asset(asset_path)
            if not asset:
                result["errors"].append("Failed to load asset")
                result["passed"] = False
                return result

            asset_class = asset.get_class().get_name()
            result["asset_type"] = asset_class

            # 根据资产类型执行相应检查
            if asset_class == "StaticMesh":
                self._check_static_mesh_quality(asset, result)
            elif asset_class == "SkeletalMesh":
                self._check_skeletal_mesh_quality(asset, result)
            elif asset_class == "Texture2D":
                self._check_texture_quality(asset, result)
            elif asset_class == "Material":
                self._check_material_quality(asset, result)

        except Exception as e:
            result["errors"].append(f"Quality check failed: {e}")
            result["passed"] = False

        return result

    def _check_static_mesh_quality(self, mesh, result: Dict):
        """检查静态网格质量"""
        rules = self.quality_rules["StaticMesh"]

        try:
            # 获取三角形数量
            if hasattr(mesh, 'get_num_triangles'):
                triangle_count = mesh.get_num_triangles(0)  # LOD 0
                result["metrics"]["triangle_count"] = triangle_count

                if triangle_count > rules["max_triangles"]:
                    result["errors"].append(f"Triangle count ({triangle_count}) exceeds maximum ({rules['max_triangles']})")
                    result["passed"] = False
                elif triangle_count < rules["min_triangles"]:
                    result["warnings"].append(f"Triangle count ({triangle_count}) is very low")

            # 检查光照贴图UV
            if rules["require_lightmap_uvs"]:
                if hasattr(mesh, 'get_num_uv_channels'):
                    uv_channels = mesh.get_num_uv_channels(0)
                    result["metrics"]["uv_channels"] = uv_channels

                    if uv_channels < 2:
                        result["warnings"].append("Missing lightmap UV channel")

            # 检查材质数量
            if hasattr(mesh, 'get_static_materials'):
                materials = mesh.get_static_materials()
                material_count = len(materials)
                result["metrics"]["material_count"] = material_count

                if material_count > rules["max_materials"]:
                    result["warnings"].append(f"Too many materials ({material_count})")

        except Exception as e:
            result["errors"].append(f"Static mesh quality check error: {e}")
            result["passed"] = False

    def _check_texture_quality(self, texture, result: Dict):
        """检查纹理质量"""
        rules = self.quality_rules["Texture2D"]

        try:
            # 获取纹理尺寸
            if hasattr(texture, 'blueprint_get_size_x') and hasattr(texture, 'blueprint_get_size_y'):
                width = texture.blueprint_get_size_x()
                height = texture.blueprint_get_size_y()
                result["metrics"]["resolution"] = f"{width}x{height}"

                # 检查分辨率限制
                max_dimension = max(width, height)
                if max_dimension > rules["max_resolution"]:
                    result["errors"].append(f"Resolution ({width}x{height}) exceeds maximum ({rules['max_resolution']})")
                    result["passed"] = False

                min_dimension = min(width, height)
                if min_dimension < rules["min_resolution"]:
                    result["warnings"].append(f"Resolution ({width}x{height}) is very low")

                # 检查是否为2的幂
                if rules["power_of_two"]:
                    if not (self._is_power_of_two(width) and self._is_power_of_two(height)):
                        result["warnings"].append("Texture resolution is not power of two")

        except Exception as e:
            result["errors"].append(f"Texture quality check error: {e}")
            result["passed"] = False

    def _is_power_of_two(self, n: int) -> bool:
        """检查是否为2的幂"""
        return n > 0 and (n & (n - 1)) == 0

    def batch_quality_check(self, asset_paths: List[str]) -> Dict:
        """批量质量检查"""
        results = {
            "total_assets": len(asset_paths),
            "passed_assets": 0,
            "failed_assets": 0,
            "warnings_count": 0,
            "details": []
        }

        for asset_path in asset_paths:
            check_result = self.check_asset_quality(asset_path)
            results["details"].append(check_result)

            if check_result["passed"]:
                results["passed_assets"] += 1
            else:
                results["failed_assets"] += 1

            results["warnings_count"] += len(check_result["warnings"])

        return results

class AssetValidationPipeline:
    """资产验证管道"""

    def __init__(self, helper: InterchangeHelper):
        self.helper = helper
        self.quality_checker = AssetQualityChecker(helper)

    def validate_import_results(self, import_results: Dict) -> Dict:
        """验证导入结果"""
        validation_results = {
            "import_summary": import_results,
            "quality_checks": {},
            "overall_status": "PASSED"
        }

        # 收集所有导入的资产
        all_imported_assets = []
        for detail in import_results.get("import_details", []):
            if detail["success"]:
                all_imported_assets.extend(detail["imported_assets"])

        if all_imported_assets:
            # 执行质量检查
            quality_results = self.quality_checker.batch_quality_check(all_imported_assets)
            validation_results["quality_checks"] = quality_results

            # 确定整体状态
            if quality_results["failed_assets"] > 0:
                validation_results["overall_status"] = "FAILED"
            elif quality_results["warnings_count"] > 0:
                validation_results["overall_status"] = "WARNING"

        return validation_results

    def generate_validation_report(self, validation_results: Dict, output_file: str = None):
        """生成验证报告"""
        report = []
        report.append("=" * 60)
        report.append("ASSET VALIDATION REPORT")
        report.append("=" * 60)

        # 导入摘要
        import_summary = validation_results["import_summary"]
        report.append(f"Total files processed: {import_summary.get('total_files', 0)}")
        report.append(f"Successful imports: {import_summary.get('successful_imports', 0)}")
        report.append(f"Failed imports: {import_summary.get('failed_imports', 0)}")
        report.append("")

        # 质量检查摘要
        quality_checks = validation_results.get("quality_checks", {})
        if quality_checks:
            report.append("QUALITY CHECK SUMMARY:")
            report.append(f"Total assets checked: {quality_checks.get('total_assets', 0)}")
            report.append(f"Passed quality checks: {quality_checks.get('passed_assets', 0)}")
            report.append(f"Failed quality checks: {quality_checks.get('failed_assets', 0)}")
            report.append(f"Total warnings: {quality_checks.get('warnings_count', 0)}")
            report.append("")

            # 详细问题列表
            if quality_checks.get("details"):
                report.append("QUALITY ISSUES:")
                for detail in quality_checks["details"]:
                    if not detail["passed"] or detail["warnings"]:
                        report.append(f"Asset: {detail['asset_path']}")
                        for error in detail["errors"]:
                            report.append(f"  ❌ ERROR: {error}")
                        for warning in detail["warnings"]:
                            report.append(f"  ⚠️  WARNING: {warning}")
                        report.append("")

        report.append(f"Overall Status: {validation_results['overall_status']}")
        report.append("=" * 60)

        report_text = "\n".join(report)
        print(report_text)

        if output_file:
            with open(output_file, 'w') as f:
                f.write(report_text)
            print(f"Report saved to: {output_file}")

        return report_text
```

## 6. 完整使用示例

### 6.1 游戏项目资产管道示例
```python
def main_asset_pipeline_example():
    """完整的游戏项目资产管道示例"""

    # 初始化系统
    helper = InterchangeHelper()

    # 创建各种处理器
    character_processor = CharacterAssetPipeline(helper)
    environment_processor = EnvironmentAssetProcessor(helper)
    animation_processor = AnimationAssetProcessor(helper)
    material_processor = MaterialAssetProcessor(helper)

    # 创建同步器和监控服务
    synchronizer = ProjectAssetSynchronizer(helper)
    monitoring_service = AssetMonitoringService(helper)
    validation_pipeline = AssetValidationPipeline(helper)

    # 设置项目路径
    project_paths = {
        "source_root": "D:/GameAssets/",
        "characters": "D:/GameAssets/Characters/",
        "environments": "D:/GameAssets/Environments/",
        "animations": "D:/GameAssets/Animations/",
        "materials": "D:/GameAssets/Materials/"
    }

    try:
        print("🚀 Starting Game Asset Pipeline...")

        # 1. 设置同步配置
        sync_config = {
            "Characters": {
                "source_folder": "Characters",
                "destination_path": "/Game/Characters/",
                "extensions": [".fbx", ".png", ".jpg", ".tga"],
                "import_settings": {
                    "import_meshes": True,
                    "import_materials": True,
                    "import_animations": True,
                    "generate_lightmap_uvs": True
                }
            },
            "Environments": {
                "source_folder": "Environments",
                "destination_path": "/Game/Environments/",
                "extensions": [".fbx", ".obj", ".png", ".jpg", ".hdr"],
                "import_settings": {
                    "import_meshes": True,
                    "import_materials": True,
                    "generate_lightmap_uvs": True
                }
            }
        }

        synchronizer.setup_sync_configuration(sync_config)

        # 2. 执行初始同步
        print("📥 Performing initial asset sync...")
        sync_results = synchronizer.sync_from_external_source(project_paths["source_root"])
        print(f"Sync completed: {sync_results['total_synced']} assets synced")

        # 3. 处理特定角色
        if os.path.exists(os.path.join(project_paths["characters"], "MainCharacter")):
            print("👤 Processing main character assets...")
            character_results = character_processor.import_character_package(
                os.path.join(project_paths["characters"], "MainCharacter"),
                "MainCharacter"
            )
            print(f"Character processing: {character_results['total_success']} successful, {character_results['total_failed']} failed")

        # 4. 处理环境资产
        if os.path.exists(os.path.join(project_paths["environments"], "Level01")):
            print("🏞️ Processing environment assets...")
            env_results = environment_processor.process_environment_package(
                os.path.join(project_paths["environments"], "Level01"),
                "Level01"
            )
            print("Environment processing completed")

        # 5. 处理动画库
        if os.path.exists(project_paths["animations"]):
            print("🎭 Processing animation library...")
            anim_results = animation_processor.process_animation_library(
                project_paths["animations"],
                "MainCharacter"
            )
            print(f"Animation processing: {anim_results['total_animations']} animations imported")

        # 6. 处理材质库
        if os.path.exists(project_paths["materials"]):
            print("🎨 Processing material library...")
            material_results = material_processor.process_material_library(
                project_paths["materials"],
                "GameMaterials"
            )
            print(f"Material processing: {material_results['total_textures']} textures imported")

        # 7. 设置实时监控
        print("👁️ Setting up real-time monitoring...")

        for category, config_data in sync_config.items():
            source_path = os.path.join(project_paths["source_root"], config_data["source_folder"])
            if os.path.exists(source_path):
                import_config = ImportConfig(
                    source_path=source_path,
                    destination_path=config_data["destination_path"],
                    file_extensions=config_data["extensions"],
                    **config_data["import_settings"]
                )

                monitoring_service.add_monitoring_path(category, source_path, import_config)

        print("✅ Asset pipeline setup completed successfully!")
        print("🔄 Real-time monitoring is now active. Files will be automatically imported when modified.")

        # 8. 运行质量验证
        print("🔍 Running quality validation...")

        # 收集所有导入的资产进行验证
        all_assets = []
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

        for path in ["/Game/Characters/", "/Game/Environments/", "/Game/Materials/"]:
            filter_data = unreal.ARFilter()
            filter_data.package_paths = [path]
            assets = asset_registry.get_assets(filter_data)
            all_assets.extend([str(asset.object_path) for asset in assets])

        if all_assets:
            quality_checker = AssetQualityChecker(helper)
            quality_results = quality_checker.batch_quality_check(all_assets[:50])  # 限制检查数量

            validation_results = {
                "import_summary": {"total_files": len(all_assets)},
                "quality_checks": quality_results,
                "overall_status": "PASSED" if quality_results["failed_assets"] == 0 else "WARNING"
            }

            validation_pipeline.generate_validation_report(
                validation_results,
                "asset_validation_report.txt"
            )

        return True

    except Exception as e:
        print(f"❌ Pipeline error: {e}")
        return False

    finally:
        # 清理资源
        print("🧹 Cleaning up...")
        monitoring_service.stop_all_monitoring()

# 使用示例
if __name__ == "__main__":
    success = main_asset_pipeline_example()
    if success:
        print("🎉 Asset pipeline completed successfully!")
    else:
        print("💥 Asset pipeline failed!")
```

### 6.2 命令行工具示例
```python
import argparse

def create_command_line_tool():
    """创建命令行工具"""

    parser = argparse.ArgumentParser(description="UE5 Interchange Asset Import Tool")

    # 基本参数
    parser.add_argument("--source", "-s", required=True, help="Source directory or file")
    parser.add_argument("--destination", "-d", required=True, help="Destination path in UE")
    parser.add_argument("--type", "-t", choices=["single", "batch", "character", "environment", "animation", "material"],
                       default="batch", help="Import type")

    # 配置参数
    parser.add_argument("--extensions", nargs="+", default=[".fbx", ".png", ".jpg"],
                       help="File extensions to import")
    parser.add_argument("--overwrite", action="store_true", help="Overwrite existing assets")
    parser.add_argument("--generate-lightmap-uvs", action="store_true", help="Generate lightmap UVs")
    parser.add_argument("--combine-meshes", action="store_true", help="Combine meshes")
    parser.add_argument("--no-materials", action="store_true", help="Skip material import")
    parser.add_argument("--no-animations", action="store_true", help="Skip animation import")

    # 质量控制参数
    parser.add_argument("--quality-check", action="store_true", help="Run quality checks")
    parser.add_argument("--validation-report", help="Output validation report file")

    # 监控参数
    parser.add_argument("--monitor", action="store_true", help="Enable real-time monitoring")
    parser.add_argument("--sync-config", help="Sync configuration file")

    return parser

def execute_command_line_import(args):
    """执行命令行导入"""

    helper = InterchangeHelper()

    # 创建导入配置
    config = ImportConfig(
        source_path=args.source,
        destination_path=args.destination,
        file_extensions=args.extensions,
        import_materials=not args.no_materials,
        import_animations=not args.no_animations,
        generate_lightmap_uvs=args.generate_lightmap_uvs,
        combine_meshes=args.combine_meshes,
        overwrite_existing=args.overwrite
    )

    results = None

    try:
        if args.type == "single":
            # 单文件导入
            importer = SingleAssetImporter(helper)
            results = importer.import_asset(args.source, args.destination, config)

        elif args.type == "batch":
            # 批量导入
            batch_importer = BatchAssetImporter(helper)
            results = batch_importer.import_directory(args.source, args.destination, config)

        elif args.type == "character":
            # 角色导入
            character_processor = CharacterAssetPipeline(helper)
            character_name = os.path.basename(args.source.rstrip("/\\"))
            results = character_processor.import_character_package(args.source, character_name)

        elif args.type == "environment":
            # 环境导入
            env_processor = EnvironmentAssetProcessor(helper)
            env_name = os.path.basename(args.source.rstrip("/\\"))
            results = env_processor.process_environment_package(args.source, env_name)

        elif args.type == "animation":
            # 动画导入
            anim_processor = AnimationAssetProcessor(helper)
            character_name = input("Enter character name for animations: ")
            results = anim_processor.process_animation_library(args.source, character_name)

        elif args.type == "material":
            # 材质导入
            material_processor = MaterialAssetProcessor(helper)
            library_name = os.path.basename(args.source.rstrip("/\\"))
            results = material_processor.process_material_library(args.source, library_name)

        # 质量检查
        if args.quality_check and results:
            print("Running quality checks...")
            validation_pipeline = AssetValidationPipeline(helper)
            validation_results = validation_pipeline.validate_import_results(results)

            if args.validation_report:
                validation_pipeline.generate_validation_report(
                    validation_results,
                    args.validation_report
                )

        # 启动监控
        if args.monitor:
            print("Starting real-time monitoring...")
            monitoring_service = AssetMonitoringService(helper)
            monitoring_service.add_monitoring_path("CommandLine", args.source, config)

            try:
                print("Monitoring active. Press Ctrl+C to stop.")
                while True:
                    time.sleep(1)
            except KeyboardInterrupt:
                print("Stopping monitoring...")
                monitoring_service.stop_all_monitoring()

        return True

    except Exception as e:
        print(f"Import failed: {e}")
        return False

# 命令行入口点
if __name__ == "__main__":
    parser = create_command_line_tool()
    args = parser.parse_args()

    success = execute_command_line_import(args)
    sys.exit(0 if success else 1)
```

## 7. 最佳实践和性能优化

### 7.1 性能优化建议
1. **批量处理**: 尽可能批量导入资产，减少单次操作开销
2. **异步处理**: 利用Python的异步特性处理大量文件
3. **内存管理**: 及时释放不需要的资产引用
4. **缓存机制**: 缓存重复使用的配置和元数据
5. **增量更新**: 只处理修改过的文件

### 7.2 错误处理策略
1. **优雅降级**: 单个文件失败不影响整体流程
2. **详细日志**: 记录详细的错误信息和堆栈跟踪
3. **重试机制**: 对临时性错误实施重试
4. **用户反馈**: 提供清晰的进度和错误反馈

### 7.3 项目组织建议
1. **模块化设计**: 将不同功能分离到独立模块
2. **配置外部化**: 使用配置文件管理导入设置
3. **版本控制**: 对导入脚本进行版本控制
4. **文档完善**: 维护详细的使用文档和API文档

这个Python开发指南提供了完整的美术资源自动化导入解决方案，涵盖了从基础配置到复杂工作流程的各个方面，包括角色、环境、动画、材质等各类资产的专业化处理流程，以及实时监控、质量控制、验证和命令行工具等高级功能，为游戏开发团队提供了一套完整的资产管理自动化解决方案。
