"""
Core API module for Interchange Python Extension

This module provides direct Python wrappers for the core Interchange components:
- Translators: File format translators
- Nodes: Data nodes for asset representation
- Pipelines: Processing pipelines for asset transformation
- Factories: Asset creation factories

All classes in this module provide Python-friendly interfaces to the underlying
C++ Interchange functionality while maintaining full compatibility.
"""

from .translators import CustomFooTranslator
from .nodes import CustomDemoObjectNode, NodeManager
from .pipelines import CustomDemoObjectPipeline, PipelineManager
from .factories import CustomDemoObjectFactory, FactoryManager

__all__ = [
    'CustomFooTranslator',
    'CustomDemoObjectNode',
    'NodeManager', 
    'CustomDemoObjectPipeline',
    'PipelineManager',
    'CustomDemoObjectFactory',
    'FactoryManager'
]
